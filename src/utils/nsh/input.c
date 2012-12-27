//
// input.c
//
// Shell input
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

#include "sh.h"

int pushfile(struct inputfile **file, int fd) {
  struct inputfile *inp;

  if (fd == -1) return 0;
  inp = (struct inputfile *) malloc(sizeof(struct inputfile));
  if (!inp) return -1;

  inp->prev = *file;
  inp->lineno = 1;
  inp->fd = fd;
  inp->buf = inp->ptr = inp->end = NULL;

  *file = inp;
  return 0;
}

int pushstr(struct inputfile **file, char *str) {
  struct inputfile *inp;
  char *newstr;

  if (!str) return 0;
  newstr = strdup(str);
  if (!newstr) return -1;

  inp = (struct inputfile *) malloc(sizeof(struct inputfile));
  if (!inp) return -1;

  inp->prev = *file;
  inp->lineno = 1;
  inp->fd = -1;
  inp->buf = inp->ptr = newstr;
  inp->end = inp->buf + strlen(newstr);

  *file = inp;
  return 0;
}

int popfile(struct inputfile **file) {
  struct inputfile *inp;

  inp = *file;
  if (!inp) return 0;
  if (inp->fd != -1) close(inp->fd);
  if (inp->buf) free(inp->buf);

  *file = inp->prev;
  free(inp);

  return 0;
}

void popallfiles(struct inputfile **file) {
  while (*file) popfile(file);
}

int pgetc(struct inputfile **file, int peek) {
  while (*file) {
    struct inputfile *inp = *file;
    
    if (inp->ptr != inp->end) {
      if (peek) {
        return *inp->ptr;
      } else {
        int ch = *(inp->ptr)++;
        if (ch == '\n') inp->lineno++;
        return ch;
      }
    }

    if (inp->fd != -1) {
      int rc;

      if (!inp->buf) inp->buf = inp->ptr = inp->end = malloc(FILEBUFSIZ);
      if (!inp->buf) return -1;

      rc = read(inp->fd, inp->buf, FILEBUFSIZ);
      if (rc < 0) {
        return -1;
      } else if (rc == 0){
        if (popfile(file) < 0) return -1;
      } else {
        inp->ptr = inp->buf;
        inp->end = inp->buf + rc;
      }
    } else {
      if (popfile(file) < 0) return -1;
    }
  }

  return -1;
}
