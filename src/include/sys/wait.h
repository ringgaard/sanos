//
// wait.h
//
// Wait for child process termination
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

#ifndef SYS_WAIT_H
#define SYS_WAIT_H

#include <sys/types.h>

#define WNOHANG 1

//
// Exit status format:
//
// +------------------------------------------------------------------------+
// |    reserved                | CONT  | STOP  |  SIG  | signum | exitcode |
// |    13 bits                 | 1 bit | 1 bit | 1 bit | 8 bits |  8 bits  |
// +------------------------------------------------------------------------+
//

#define WIFEXITED(status)    (((status) & 0x10000) == 0)
#define WEXITSTATUS(status)  ((status) & 0xFF)
#define WIFSIGNALED(status)  ((status) & 0x10000)
#define WTERMSIG(status)     (((status) >> 8) & 0xFF)
#define WIFSTOPPED(status)   ((status) & 0x20000)
#define WSTOPSIG(status)     (((status) >> 8) & 0xFF)
#define WIFCONTINUED(status) ((status) & 0x40000)

#ifdef  __cplusplus
extern "C" {
#endif

pid_t wait(int *stat_loc);
pid_t waitpid(pid_t pid, int *stat_loc, int options);

#ifdef  __cplusplus
}
#endif

#endif
