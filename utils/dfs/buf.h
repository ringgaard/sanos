#ifndef BUF_H
#define BUF_H

struct thread
{
  void *ready;
  struct thread *next_buffer_waiter;
};

#define BUFPOOL_HASHSIZE 512

#define BUF_STATE_FREE      0
#define BUF_STATE_CLEAN     1
#define BUF_STATE_DIRTY     2
#define BUF_STATE_READING   3
#define BUF_STATE_WRITING   4
#define BUF_STATE_LOCKED    5
#define BUF_STATE_UPDATED   6
#define BUF_STATE_INVALID   7

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
  vfs_blkno_t blkno;
  char *data;
};

struct bufpool
{
  vfs_devno_t devno;
  int poolsize;
  int bufsize;
  int blks_per_buffer;
  
  struct buf *bufbase;
  char *database;

  struct buflist dirty;  // List of dirty buffers (head is least recently changed)
  struct buflist clean;  // List of clean buffers (head is least recently used)
  struct buf *freelist;  // List of free buffers

  struct buf *hashtable[BUFPOOL_HASHSIZE];
};

struct bufpool *init_buffer_pool(vfs_devno_t devno, int poolsize, int bufsize);
void free_buffer_pool(struct bufpool *pool);
struct buf *get_buffer(struct bufpool *pool, vfs_blkno_t blkno);
struct buf *alloc_buffer(struct bufpool *pool, vfs_blkno_t blkno);
void mark_buffer_updated(struct buf *buf);
void mark_buffer_invalid(struct buf *buf);
void release_buffer(struct bufpool *pool, struct buf *buf);
void invalidate_buffer(struct bufpool *pool, vfs_blkno_t blkno);
void flush_buffers(struct bufpool *pool);

#endif
