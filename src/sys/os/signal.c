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

sighandler_t sighandlers[NSIG];

static void sigexit(struct siginfo *info, int action)
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

void raise(int signum, struct siginfo *info)
{
  sighandler_t handler;

  if (signum < 0 || signum >= NSIG) return;
  handler = sighandlers[signum];
  if (handler == SIG_IGN) return;
  if (handler == SIG_DFL) dbgbreak();
  handler(signum, info);
}

void globalhandler(int signum, struct siginfo *info)
{
  //syslog(LOG_DEBUG, "signal %d received (trap 0x%x)\n", signum, info->ctxt.traptype);
  if (sighandlers[signum] == SIG_DFL) sigexit(info, 1);
  raise(signum, info);
  sigexit(info, 0);
}
