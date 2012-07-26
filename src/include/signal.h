//
// signal.h
//
// Signal handling
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

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef SIGNAL_H
#define SIGNAL_H

#include <sys/types.h>

//
// Signal numbers
//
// Some of these signals are not used in sanos, but are included
// for completeness.
//

#ifndef _NSIG

#define SIGHUP          1       // Hangup (POSIX)
#define SIGINT          2       // Interrupt (ANSI)
#define SIGQUIT         3       // Quit (POSIX)
#define SIGILL          4       // Illegal instruction (ANSI)
#define SIGTRAP         5       // Trace trap (POSIX)
#define SIGABRT         6       // Abort (ANSI)
#define SIGBUS          7       // BUS error (4.2 BSD)
#define SIGFPE          8       // Floating-point exception (ANSI)
#define SIGKILL         9       // Kill, unblockable (POSIX)
#define SIGUSR1         10      // User-defined signal 1 (POSIX)
#define SIGSEGV         11      // Segmentation violation (ANSI)
#define SIGUSR2         12      // User-defined signal 2 (POSIX)
#define SIGPIPE         13      // Broken pipe (POSIX)
#define SIGALRM         14      // Alarm clock (POSIX)
#define SIGTERM         15      // Termination (ANSI)
#define SIGSTKFLT       16      // Stack fault
#define SIGCHLD         17      // Child status has changed (POSIX)
#define SIGCONT         18      // Continue (POSIX)
#define SIGSTOP         19      // Stop, unblockable (POSIX)
#define SIGTSTP         20      // Keyboard stop (POSIX)
#define SIGTTIN         21      // Background read from tty (POSIX)
#define SIGTTOU         22      // Background write to tty (POSIX)
#define SIGURG          23      // Urgent condition on socket (4.2 BSD)
#define SIGXCPU         24      // CPU limit exceeded (4.2 BSD)
#define SIGXFSZ         25      // File size limit exceeded (4.2 BSD)
#define SIGVTALRM       26      // Virtual alarm clock (4.2 BSD)
#define SIGPROF         27      // Profiling alarm clock (4.2 BSD)
#define SIGWINCH        28      // Window size change (4.3 BSD, Sun)
#define SIGIO           29      // I/O now possible (4.2 BSD)
#define SIGPWR          30      // Power failure restart (System V)
#define SIGSYS          31      // Bad system call

#define _NSIG 32

#endif

#define SIGPOLL         SIGIO   // Pollable event occurred (System V)
#define SIGCLD          SIGCHLD // Same as SIGCHLD (System V)
#define SIGIOT          6       // IOT trap (4.2 BSD)

#ifndef _SIGHANDLER_T_DEFINED
#define _SIGHANDLER_T_DEFINED
typedef void (*sighandler_t)(int signum);
#endif

#ifndef SIG_DFL

#define SIG_DFL ((sighandler_t) 0)
#define SIG_IGN ((sighandler_t) 1)
#define SIG_ERR ((sighandler_t) -1)

#endif

struct context;

#ifndef _SIGINFO_T_DEFINED
#define _SIGINFO_T_DEFINED

struct siginfo {
  int si_signo;
  int si_code;

  struct context *si_ctxt;
  void *si_addr;
};

typedef struct siginfo siginfo_t;

#endif

#ifndef _SIGACTION_DEFINED
#define _SIGACTION_DEFINED

struct sigaction {
  union {
    void (*sa_handler)(int signum);
    void (*sa_sigaction)(int signum, siginfo_t *info, void *context);
  };
  sigset_t sa_mask;
  int sa_flags;
};

#endif

//
// Bits in sa_flags
//

#ifndef SA_NOCLDSTOP

#define SA_NOCLDSTOP 0x0001
#define SA_ONSTACK   0x0002
#define SA_RESETHAND 0x0004
#define SA_RESTART   0x0008
#define SA_SIGINFO   0x0010
#define SA_NOCLDWAIT 0x0020
#define SA_NODEFER   0x0030

#endif

//
// Values for sigprocmask
//

#ifndef SIG_BLOCK

#define SIG_BLOCK   1   // Block signals in 'set', other signals unaffected
#define SIG_UNBLOCK 2   // Unblock signals in 'set'
#define SIG_SETMASK 3   // New mask is 'set'

#endif

#ifdef  __cplusplus
extern "C" {
#endif

osapi sighandler_t signal(int signum, sighandler_t handler);
osapi int raise(int signum);
osapi int kill(pid_t pid, int signum);
osapi char *strsignal(int signum);

osapi int sigemptyset(sigset_t *set);
osapi int sigfillset(sigset_t *set);
osapi int sigaddset(sigset_t *set, int signum);
osapi int sigdelset(sigset_t *set, int signum);
osapi int sigismember(sigset_t *set, int signum);

osapi int sigaction(int signum, const struct sigaction *act, struct sigaction *oldact);
osapi int sigprocmask(int how, const sigset_t *set, sigset_t *oldset);
osapi int sigpending(sigset_t *set);
osapi int sigsuspend(const sigset_t *mask);

#ifdef  __cplusplus
}
#endif

#endif
