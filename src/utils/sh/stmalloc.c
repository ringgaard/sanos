//
// stmalloc.c
//
// Stacked memory allocation
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

void pushstkmark(struct stkmark *oldmark, struct stkmark *newmark) {
  if (oldmark) {
    newmark->blk = oldmark->blk;
    newmark->ptr = oldmark->ptr;
    newmark->txt = oldmark->txt;
    newmark->end = oldmark->end;
  } else {
    newmark->blk = NULL;
    newmark->ptr = NULL;
    newmark->txt = NULL;
    newmark->end = NULL;
  }
  newmark->prev = oldmark;
}

void popstkmark(struct stkmark *mark) {
  struct stkblk *blk = mark->blk;
  struct stkblk *prev_mark_blk = mark->prev ? mark->prev->blk : NULL;
  while (blk && blk != prev_mark_blk) {
    struct stkblk *prev = blk->prev;
    free(blk);
    blk = prev;
  }
}

void *stalloc(struct stkmark *mark, int size) {
  char *ptr;

  if (mark->txt) printf("=== alloc with active string\n");

  if (size > mark->end - mark->ptr) {
    int blksize;
    struct stkblk *blk;

    blksize = size;
    if (blksize < STKBLKMIN) blksize = STKBLKMIN;

    //printf("==== malloc new block (%d bytes)\n", size);
    blk = (struct stkblk *) malloc(sizeof(struct stkblk) - STKBLKMIN + blksize);
    if (!blk) return NULL;

    blk->prev = mark->blk;
    mark->blk = blk;
    mark->ptr = blk->space;
    mark->end = mark->ptr + blksize;
  }

  ptr = mark->ptr;
  memset(ptr, 0, size);
  mark->ptr += size;

  return ptr;
}

static int streserve(struct stkmark *mark, int size) {
  int txtlen;

  if (!mark->txt) mark->txt = mark->ptr;
  if (mark->txt + size <= mark->end) return 0;
  
  txtlen = mark->txt - mark->ptr;

  if (mark->blk && mark->ptr == mark->blk->space && (!mark->prev || mark->blk != mark->prev->blk)) {
    int blksize = mark->end - mark->ptr;
    int minsize = txtlen + size;
    while (blksize < minsize) blksize *= 2;
    //printf("==== realloc string block (%d bytes)\n", blksize);
    mark->blk = (struct stkblk *) realloc(mark->blk, blksize);
    if (!mark->blk) return -1;
    mark->ptr = mark->blk->space;
    mark->end = mark->ptr + blksize;
    mark->txt = mark->ptr + txtlen;
  } else {
    struct stkblk *blk;
    int blksize = txtlen + size;
    if (blksize < STKBLKMIN) blksize = STKBLKMIN;

    //printf("==== malloc new string block %d\n", blksize);
    blk = (struct stkblk *) malloc(sizeof(struct stkblk) - STKBLKMIN + blksize);
    if (!blk) return -1;

    if (mark->txt) {
      //printf("==== copying %d string bytes\n", txtlen);
      memcpy(blk->space, mark->ptr, txtlen);
      mark->txt = blk->space + txtlen;
    }

    blk->prev = mark->blk;
    mark->blk = blk;
    mark->ptr = blk->space;
    mark->end = mark->ptr + blksize;
  }

  if (!mark->txt) mark->txt = mark->ptr;
  return 0;
}

int stputbuf(struct stkmark *mark, char *data, int len) {
  if (streserve(mark, len) < 0) return -1;
  memcpy(mark->txt, data, len);
  mark->txt += len;
  return 0;
}

int stputstr(struct stkmark *mark, char *str) {
  return stputbuf(mark, str, strlen(str));
}

int stputc(struct stkmark *mark, int ch) {
  if (streserve(mark, 1) < 0) return -1;
  *(mark->txt)++ = ch;
  return 0;
}

char *ststrdup(struct stkmark *mark, char *str) {
  int len;
  char *s;
  
  if (!str) return NULL;
  len = strlen(str);
  s = stalloc(mark, len + 1);
  memcpy(s, str, len + 1);
  return s;
}

char *ststr(struct stkmark *mark) {
  char *str;

  if (!mark->txt) return NULL;
  if (stputc(mark, 0) < 0) return NULL;
  str = mark->ptr;
  mark->ptr = mark->txt;
  mark->txt = NULL;
  return str;
}

int ststrlen(struct stkmark *mark) {
  if (!mark->txt) return -1;
  return mark->txt - mark->ptr;
}

char *ststrptr(struct stkmark *mark) {
  if (!mark->txt) return NULL;
  return mark->ptr;
}

int stfreestr(struct stkmark *mark, char *str) {
  if (!str) return 0;
  if (mark->blk && mark->blk->space <= str && str <= mark->ptr) {
    mark->ptr = str;
    return 0;
  } else {
    return -1;
  }
}
