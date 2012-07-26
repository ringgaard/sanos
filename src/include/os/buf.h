//
// buf.h
//
// I/O buffer cache
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

#ifndef BUF_H
#define BUF_H

#define BUFPOOL_HASHSIZE 512

#define BUF_STATE_FREE      0
#define BUF_STATE_CLEAN     1
#define BUF_STATE_DIRTY     2
#define BUF_STATE_READING   3
#define BUF_STATE_WRITING   4
#define BUF_STATE_LOCKED    5
#define BUF_STATE_UPDATED   6
#define BUF_STATE_INVALID   7
#define BUF_STATE_ERROR     8

#define BUF_STATES          9

struct thread;
struct buf;

struct buflist {
  struct buf *head;
  struct buf *tail;
};

struct buflink {
  struct buf *next;
  struct buf *prev;
};

struct buf {
  struct buflink bucket;
  struct buflink chain;
  unsigned short state;
  unsigned short locks;
  struct thread *waiters;
  blkno_t blkno;
  char *data;
};

struct bufpool {
  dev_t devno;
  int poolsize;
  int bufsize;
  int blks_per_buffer;
  int ioactive;

  void (*sync)(void *arg);
  void *syncarg;
  time_t last_sync;
  int nosync;

  int blocks_read;
  int cache_hits;
  int cache_misses;

  int blocks_allocated;
  int blocks_freed;

  int blocks_updated;
  int blocks_written;
  int blocks_lazywrite;
  int blocks_synched;

  struct bufpool *next;
  struct bufpool *prev;

  struct buf *bufbase;
  char *database;

  struct buflist dirty;  // List of dirty buffers (head is least recently changed)
  struct buflist clean;  // List of clean buffers (head is least recently used)
  struct buf *freelist;  // List of free buffers

  int bufcount[BUF_STATES];

  struct buf *hashtable[BUFPOOL_HASHSIZE];
};

krnlapi struct bufpool *init_buffer_pool(dev_t devno, int poolsize, int bufsize, void (*sync)(void *arg), void *syncarg);
krnlapi void free_buffer_pool(struct bufpool *pool);
krnlapi struct buf *get_buffer(struct bufpool *pool, blkno_t blkno);
krnlapi struct buf *alloc_buffer(struct bufpool *pool, blkno_t blkno);
krnlapi void mark_buffer_updated(struct bufpool *pool, struct buf *buf);
krnlapi void mark_buffer_invalid(struct bufpool *pool, struct buf *buf);
krnlapi void release_buffer(struct bufpool *pool, struct buf *buf);
krnlapi void invalidate_buffer(struct bufpool *pool, blkno_t blkno);
krnlapi int flush_buffers(struct bufpool *pool, int interruptable);
krnlapi int sync_buffers(struct bufpool *pool, int interruptable);

#endif
