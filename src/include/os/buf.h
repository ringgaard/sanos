//
// buf.h
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// I/O buffer cache
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

struct buflist
{
  struct buf *head;
  struct buf *tail;
};

struct buflink
{
  struct buf *next;
  struct buf *prev;
};

struct buf
{
  struct buflink bucket;
  struct buflink chain;
  unsigned short state;
  unsigned short locks;
  struct thread *waiters;
  blkno_t blkno;
  char *data;
};

struct bufpool
{
  devno_t devno;
  int poolsize;
  int bufsize;
  int blks_per_buffer;
  int ioactive;

  void (*sync)(void *arg);
  void *syncarg;
  time_t last_sync;

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

krnlapi struct bufpool *init_buffer_pool(devno_t devno, int poolsize, int bufsize, void (*sync)(void *arg), void *syncarg);
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
