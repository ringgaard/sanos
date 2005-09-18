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

char *signame[NSIG] =
{
  NULL,
  NULL,
  "interrupt",
  NULL,
  "illegal instruction",
  NULL,
  NULL,
  NULL,
  "floating point exception",
  NULL,
  NULL,
  "segment violation",
  NULL,
  NULL,
  NULL,
  "term",
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  "break",
  "abort",
  "bus error",
  "debug trap",
  "guard trap",
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL
};

sighandler_t sighandlers[NSIG];

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

  if (signum < 0 || signum >= NSIG) return SIG_ERR;
  prev = sighandlers[signum];
  sighandlers[signum] = handler;
  return prev;
}

int sendsig(int signum, struct siginfo *info)
{
  struct tib *tib;
  sighandler_t handler;
  struct siginfo *prevsig;

  if (signum < 0 || signum >= NSIG) return EINVAL;

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
      syslog(LOG_ERR, "terminating with signal %d (%s)", signum, signame[signum] ? signame[signum] : "unknown");
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
  return sendsig(signum, NULL);
}

struct siginfo *getsiginfo()
{
  return gettib()->cursig;
}

void globalhandler(int signum, struct siginfo *info)
{
  //syslog(LOG_DEBUG, "signal %d received (trap 0x%x at %p)", signum, info->ctxt.traptype, info->ctxt.eip);
  if (sighandlers[signum] == SIG_DFL && peb->debug) sigexit(info, 1);
  sendsig(signum, info);
  sigexit(info, 0);
}
