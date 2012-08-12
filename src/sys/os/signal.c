//
// signal.c
//
// Signal and exception handling
//
// Copyright (C) 2002 Michael Ringgaard. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 
// 1. Redistributions of source code must retain the above copyright 
//    notice, this list of conditions and the following disclaimer.  
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.  
// 3. Neither the name of the project nor the names of its contributors
//    may be used to endorse or promote products derived from this software
//    without specific prior written permission. 
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
// OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
// OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF 
// SUCH DAMAGE.
// 

#include <os.h>
#include <string.h>
#include <os/syscall.h>

void dump_stack(struct context *ctxt);

struct sigentry {
  char *label;
  char *name;
  int flags;
  int defaction;
};

#define SIGACT_TERM      0
#define SIGACT_ABORT     1
#define SIGACT_IGN       2
#define SIGACT_STOP      3
#define SIGACT_CONT      4

struct sigentry sigtab[_NSIG] = {
  {"SIGNUL",    "Null", 0, SIGACT_IGN},
  {"SIGHUP",    "Hangup", 0, SIGACT_TERM},
  {"SIGINT",    "Interrupt", 0, SIGACT_TERM},
  {"SIGQUIT",   "Quit", 0, SIGACT_ABORT},
  {"SIGILL",    "Illegal instruction", 0, SIGACT_ABORT},
  {"SIGTRAP",   "Trace trap", 0, SIGACT_ABORT},
  {"SIGABRT",   "Abort", 0, SIGACT_ABORT},
  {"SIGBUS",    "BUS error", 0, SIGACT_TERM},
  {"SIGFPE",    "Floating-point exception", 0, SIGACT_ABORT},
  {"SIGKILL",   "Kill", 0, SIGACT_TERM},
  {"SIGUSR1",   "User-defined signal 1", 0, SIGACT_TERM},
  {"SIGSEGV",   "Segmentation violation", 0, SIGACT_ABORT},
  {"SIGUSR2",   "User-defined signal 2", 0, SIGACT_TERM},
  {"SIGPIPE",   "Broken pipe", 0, SIGACT_TERM},
  {"SIGALRM",   "Alarm clock", 0, SIGACT_TERM},
  {"SIGTERM",   "Termination", 0, SIGACT_TERM},
  {"SIGSTKFLT", "Stack fault", 0, SIGACT_ABORT},
  {"SIGCHLD",   "Child status has changed", 0, SIGACT_IGN},
  {"SIGCONT",   "Continue", 0, SIGACT_CONT},
  {"SIGSTOP",   "Stop", 0, SIGACT_STOP},
  {"SIGTSTP",   "Keyboard stop", 0, SIGACT_STOP},
  {"SIGTTIN",   "Background read from tty", 0, SIGACT_STOP},
  {"SIGTTOU",   "Background write to tty", 0, SIGACT_STOP},
  {"SIGURG",    "Urgent condition on socket", 0, SIGACT_IGN},
  {"SIGXCPU",   "CPU limit exceeded", 0, SIGACT_ABORT},
  {"SIGXFSZ",   "File size limit exceeded", 0, SIGACT_ABORT},
  {"SIGVTALRM", "Virtual alarm clock", 0, SIGACT_TERM},
  {"SIGPROF",   "Profiling alarm clock", 0, SIGACT_TERM},
  {"SIGWINCH",  "Window size change", 0, SIGACT_IGN},
  {"SIGIO",     "I/O now possible", 0, SIGACT_IGN},
  {"SIGPWR",    "Power failure restart", 0, SIGACT_IGN},
  {"SIGSYS",    "Bad system call", 0, SIGACT_ABORT}
};

char *strsignal(int signum) {
  if (signum >= 0 && signum < _NSIG) {
    return sigtab[signum].name;
  } else {
    return "Unknown";
  }
}

void sigexit(struct siginfo *info, int action) {
  __asm {
    mov eax, [action]
    mov ebx, [info]
    int 49
  }
}

int sigemptyset(sigset_t *set) {
  if (!set) {
    errno = EFAULT;
    return -1;
  }

  *set = 0;
  return 0;
}

int sigfillset(sigset_t *set) {
  if (!set) {
    errno = EFAULT;
    return -1;
  }

  *set = 0xFFFFFFFF;
  return 0;
}

int sigaddset(sigset_t *set, int signum) {
  if (!set) {
    errno = EFAULT;
    return -1;
  }

  if (signum < 0 || signum >= _NSIG) {
    errno = EINVAL;
    return -1;
  }

  *set |= (1 << signum);
  return 0;
}

int sigdelset(sigset_t *set, int signum) {
  if (!set) {
    errno = EFAULT;
    return -1;
  }

  if (signum < 0 || signum >= _NSIG) {
    errno = EINVAL;
    return -1;
  }

  *set &= ~(1 << signum);
  return 0;
}

int sigismember(sigset_t *set, int signum) {
  if (!set) {
    errno = EFAULT;
    return -1;
  }

  if (signum < 0 || signum >= _NSIG) {
    errno = EINVAL;
    return -1;
  }

  return (*set & (1 << signum)) != 0;
}

int sigaction(int signum, const struct sigaction *act, struct sigaction *oldact) {
  struct process *proc = gettib()->proc;

  if (signum < 0 || signum >= _NSIG) {
    errno = EINVAL;
    return -1;
  }

  if (oldact) memcpy(oldact, proc->handlers + signum, sizeof(struct sigaction));
  memcpy(proc->handlers + signum, act, sizeof(struct sigaction));

  return 0;
}

sighandler_t signal(int signum, sighandler_t handler) {
  struct sigaction act;
  struct sigaction oldact;

  act.sa_handler = handler;
  act.sa_mask = 0;
  act.sa_flags = 0;

  if (sigaction(signum, &act, &oldact) < 0) return SIG_ERR;
  return oldact.sa_handler;
}

int sigprocmask(int how, const sigset_t *set, sigset_t *oldset) {
  return syscall(SYSCALL_SIGPROCMASK, &how);
}

int sigpending(sigset_t *set) {
  return syscall(SYSCALL_SIGPENDING, &set);
}

int sigsuspend(const sigset_t *mask) {
  int rc;

  if (mask) {
    rc = sigprocmask(SIG_BLOCK, mask, NULL);
    if (rc < 0) return rc;
  }

  rc = waitone(self(), INFINITE);

  if (mask) sigprocmask(SIG_UNBLOCK, mask, NULL);

  return rc;
}

int sendsig(handle_t thread, int signum) {
  return syscall(SYSCALL_SENDSIG, &thread);
}

int kill(pid_t pid, int signum) {
  handle_t h;
  int rc;

  h = getprochandle(pid);
  if (h < 0) return h;

  rc = sendsig(h, signum);
  close(h);

  return rc;
}

int raise(int signum) {
  return sendsig(self(), signum);
}

unsigned alarm(unsigned seconds) {
  return syscall(SYSCALL_ALARM, &seconds);
}

void globalhandler(struct siginfo *info) {
  struct sigaction *act;
  struct sigaction oldact;
  int signum = info->si_signo;

  if (signum < 0 || signum >= _NSIG) return;

  act = gettib()->proc->handlers + signum;
  memcpy(&oldact, act, sizeof(struct sigaction));

  if (oldact.sa_handler == SIG_DFL) {
    switch (sigtab[signum].defaction) {
      case SIGACT_TERM:
        syslog(LOG_ERR, "terminating with signal %d (%s)", signum, strsignal(signum));
        dump_stack(info->si_ctxt);
        exit((signum << 8) | 0x10000);

      case SIGACT_ABORT:
        if (getpeb()->debug) sigexit(info, 1);
        syslog(LOG_ERR, "aborting with signal %d (%s)", signum, strsignal(signum));
        dump_stack(info->si_ctxt);
        exit((signum << 8) | 0x10000);

      case SIGACT_IGN:
        break;

      case SIGACT_STOP:
        syslog(LOG_INFO, "stopped");
        suspend(self());
        break;

      case SIGACT_CONT:
        break;
    }
  } else if (oldact.sa_handler != SIG_IGN) {
    if (oldact.sa_flags & SA_RESETHAND) {
      act->sa_handler = SIG_IGN;
      act->sa_flags &= ~SA_SIGINFO;
    }

    if (oldact.sa_mask) sigprocmask(SIG_BLOCK, &oldact.sa_mask, NULL);

    if (oldact.sa_flags & SA_SIGINFO) {
      oldact.sa_sigaction(signum, info, info->si_ctxt);
    } else {
      oldact.sa_handler(signum);
    }

    if (oldact.sa_mask) sigprocmask(SIG_UNBLOCK, &oldact.sa_mask, NULL);
  }

  sigexit(info, 0);
}
