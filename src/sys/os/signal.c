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
#include <os/pdir.h>

char *signame[_NSIG] =
{
  NULL,
  "Hangup",
  "Interrupt",
  "Quit",
  "Illegal instruction",
  "Trace trap",
  "Abort",
  "BUS error",
  "Floating-point exception",
  "Kill",
  "User-defined signal 1",
  "Segmentation violation",
  "User-defined signal 2",
  "Broken pipe",
  "Alarm clock",
  "Termination",
  "Stack fault",
  "Child status has changed",
  "Continue",
  "Stop, unblockable",
  "Keyboard stop",
  "Background read from tty",
  "Background write to tty",
  "Urgent condition on socket",
  "CPU limit exceeded",
  "File size limit exceeded",
  "Virtual alarm clock",
  "Profiling alarm clock",
  "Window size change",
  "I/O now possible",
  "Power failure restart",
  "Bad system call"
};

sighandler_t sighandlers[_NSIG];

char *strsignal(int signum)
{
  if (signum >= 0 && signum < _NSIG && signame[signum]) 
    return signame[signum];
  else
    return "Unknown";
}

void sigexit(struct siginfo *info, int action)
{
  __asm
  {
    mov eax, [action]
    mov ebx, [info]
    int 49
  }
}

sighandler_t signal(int signum, sighandler_t handler)
{
  sighandler_t prev;

  if (signum < 0 || signum >= _NSIG) return SIG_ERR;
  prev = sighandlers[signum];
  sighandlers[signum] = handler;
  return prev;
}

int sendsig(struct siginfo *info)
{
  struct tib *tib;
  sighandler_t handler;
  struct siginfo *prevsig;
  int signum = info->si_signo;

  if (signum < 0 || signum >= _NSIG) return EINVAL;

  tib = gettib();
  prevsig = tib->cursig;
  tib->cursig = info;

  handler = sighandlers[signum];

  if (handler == SIG_DFL)
  {
    if (peb->debug)
      dbgbreak();
    else
    {
      syslog(LOG_ERR, "terminating with signal %d (%s)", signum, strsignal(signum));
      exit(signum);
    }
  }
  else if (handler != SIG_IGN)
    handler(signum);

  tib->cursig = prevsig;
  return 0;
}

int raise(int signum)
{
  struct siginfo info;
  memset(&info, 0, sizeof(struct siginfo));
  info.si_signo = signum;
  return sendsig(&info);
}

struct siginfo *getsiginfo()
{
  return gettib()->cursig;
}

int sigemptyset(sigset_t *set)
{
  if (!set)
  {
    errno = EFAULT;
    return -1;
  }

  *set = 0;
  return 0;
}

int sigfillset(sigset_t *set)
{
  if (!set)
  {
    errno = EFAULT;
    return -1;
  }

  *set = 0xFFFFFFFF;
  return 0;
}

int sigaddset(sigset_t *set, int signum)
{
  if (!set)
  {
    errno = EFAULT;
    return -1;
  }

  if (signum < 0 || signum >= _NSIG)
  {
    errno = EINVAL;
    return -1;
  }

  *set |= (1 << signum);
  return 0;
}

int sigdelset(sigset_t *set, int signum)
{
  if (!set)
  {
    errno = EFAULT;
    return -1;
  }

  if (signum < 0 || signum >= _NSIG)
  {
    errno = EINVAL;
    return -1;
  }

  *set &= ~(1 << signum);
  return 0;
}

int sigismember(sigset_t *set, int signum)
{
  if (!set)
  {
    errno = EFAULT;
    return -1;
  }

  if (signum < 0 || signum >= _NSIG)
  {
    errno = EINVAL;
    return -1;
  }

  return (*set & (1 << signum)) != 0;
}

int sigaction(int signum, const struct sigaction *act, struct sigaction *oldact)
{
  errno = ENOSYS;
  return -1;
}

int sigprocmask(int how, const sigset_t *set, sigset_t *oldset)
{
  errno = ENOSYS;
  return -1;
}

int sigpending(sigset_t *set)
{
  errno = ENOSYS;
  return -1;
}

int sigsuspend(const sigset_t *mask)
{
  errno = ENOSYS;
  return -1;
}

void globalhandler(struct siginfo *info)
{
  //syslog(LOG_DEBUG, "signal %d received (trap 0x%x at %p)", signum, info->ctxt.traptype, info->ctxt.eip);
  if (sighandlers[info->si_signo] == SIG_DFL && peb->debug) sigexit(info, 1);
  sendsig(info);
  sigexit(info, 0);
}
