//
// popen.c
//
// Pipe I/O
//
// Copyright (C) 2011 Michael Ringgaard. All rights reserved.
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

#include <stdio.h>
#include <string.h>

#define SHELL "sh.exe"

FILE *popen(const char *command, const char *mode) {
  char *cmdline;
  int cmdlen;
  int rc;
  int hndl[2];
  int phndl;
  struct tib *tib;
  struct process *proc;
  FILE *f;

  if (!command) {
    errno = EINVAL;
    return NULL;
  }

  cmdlen = strlen(SHELL) + 1 + strlen(command);
  cmdline = malloc(cmdlen + 1);
  if (!cmdline) {
    errno = ENOMEM;
    return NULL;
  }
  strcpy(cmdline, SHELL);
  strcat(cmdline, " ");
  strcat(cmdline, command);

  phndl = spawn(P_SUSPEND, SHELL, cmdline, NULL, &tib);
  free(cmdline);
  if (phndl < 0) return NULL;
  proc = tib->proc;

  rc = pipe(hndl);
  if (rc < 0) return NULL;

  if (*mode == 'w') {
    if (proc->iob[0] != NOHANDLE) close(proc->iob[0]);
    proc->iob[0] = hndl[0];
    f = fdopen(hndl[1], mode);
  } else {
    if (proc->iob[1] != NOHANDLE) close(proc->iob[1]);
    proc->iob[1] = hndl[1];
    if (mode[1] == '2') dup2(hndl[1], proc->iob[2]);
    f = fdopen(hndl[0], mode);
  }

  if (f == NULL) {
    close(phndl);
    return NULL;
  }

  f->phndl = phndl;
  resume(phndl);
  return f;
}

int pclose(FILE *stream) {
  int rc;

  if (stream->flag & _IORD) {
    waitone(stream->phndl, INFINITE);
    close(stream->phndl);
    rc = fclose(stream);
  } else {
    int phndl = stream->phndl;
    rc = fclose(stream);
    waitone(phndl, INFINITE);
    close(phndl);
  }

  return rc;
}
