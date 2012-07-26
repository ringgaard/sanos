//
// buf.c
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

#include <os/krnl.h>

#define SYNC_INTERVAL  10      // Sync interval in seconds
#define BUFWAIT_BOOST  1

struct bufpool *bufpools = NULL;
struct event dirty_buffers;
int lazywriter_started = 0;
struct thread *lazywriter_thread;
int sync_active = 0;

static char *statename[] = {"free", "clean", "dirty", "read", "write", "lock", "upd", "inv", "err"};

//
// dump_pool_stat
//

void dump_pool_stat(struct bufpool *pool) {
  int i;

  kprintf("pool %s: %dx%d, %dK total (", device(pool->devno)->name, pool->poolsize, pool->bufsize, pool->poolsize * pool->bufsize / 1024);
  for (i = 0; i < BUF_STATES; i++)  {
    if (pool->bufcount[i] != 0) {
      kprintf("%s:%d ", statename[i], pool->bufcount[i]);
    }
  }
  kprintf(")\n");
}

//
// bufpools_proc
//

static int bufpools_proc(struct proc_file *pf, void *arg) {
  struct bufpool *pool;
  int i;

  pprintf(pf, "device   bufsize   size  free clean dirty  read write  lock   upd  invl   err\n");
  pprintf(pf, "-------- ------- ------ ----- ----- ----- ----- ----- ----- ----- ----- -----\n");

  pool = bufpools;
  while (pool) {
    pprintf(pf, "%-8s %7d %5dK", device(pool->devno)->name, pool->bufsize, pool->poolsize * pool->bufsize / 1024);
    for (i = 0; i < BUF_STATES; i++) pprintf(pf, "%6d", pool->bufcount[i]);
    pprintf(pf, "\n");
    pool = pool->next;
  }

  return 0;
}

//
// bufstats_proc
//

static int bufstats_proc(struct proc_file *pf, void *arg) {
  struct bufpool *pool;
  int hitratio;

  pprintf(pf, "device      reads   writes   hits%%   alloc    free  update    lazy    sync\n");
  pprintf(pf, "-------- -------- -------- ------- ------- ------- ------- ------- -------\n");

  pool = bufpools;
  while (pool) {
    if (pool->cache_hits + pool->cache_misses == 0) {
      hitratio = 0;
    } else {
      hitratio = pool->cache_hits * 100 / (pool->cache_hits + pool->cache_misses);
    }

    pprintf(pf, "%-8s %8d %8d %6d%% %7d %7d %7d %7d %7d\n", 
      device(pool->devno)->name,
      pool->blocks_read, pool->blocks_written, hitratio,
      pool->blocks_allocated, pool->blocks_freed,
      pool->blocks_updated, pool->blocks_lazywrite, pool->blocks_synched);

    pool = pool->next;
  }

  return 0;
}

//
// bufhash
//

static unsigned long bufhash(blkno_t blkno) {
  return blkno;
}

//
// change_state
//

static void __inline change_state(struct bufpool *pool, struct buf *buf, int newstate) {
  pool->bufcount[buf->state]--;
  pool->bufcount[newstate]++;
  buf->state = newstate;
}

//
// wait_for_buffer
//

static int wait_for_buffer(struct buf *buf) {
  struct thread *t = self();

  t->next_waiter = buf->waiters;
  buf->waiters = t;
  enter_wait(THREAD_WAIT_BUFFER);
  return t->waitkey;
}

//
// release_buffer_waiters
//

static void release_buffer_waiters(struct buf *buf, int waitkey) {
  struct thread *thread = buf->waiters;
  struct thread *next;

  while (thread) {
    buf->locks++;
    next = thread->next_waiter;
    thread->next_waiter = NULL;
    thread->waitkey = waitkey;
    
    mark_thread_ready(thread, 1, BUFWAIT_BOOST);
    thread = next;
  }
  buf->waiters = NULL;
}

//
// insert_into_hashtable
//

static void insert_into_hashtable(struct bufpool *pool, struct buf *buf) {
  int slot;

  slot = bufhash(buf->blkno) % BUFPOOL_HASHSIZE;
  if (pool->hashtable[slot]) pool->hashtable[slot]->bucket.prev = buf;
  buf->bucket.next = pool->hashtable[slot];
  buf->bucket.prev = NULL;
  pool->hashtable[slot] = buf;
}

//
// remove_from_hashtable
//

static void remove_from_hashtable(struct bufpool *pool, struct buf *buf) {
  int slot;

  slot = bufhash(buf->blkno) % BUFPOOL_HASHSIZE;
  if (buf->bucket.next) buf->bucket.next->bucket.prev = buf->bucket.prev;
  if (buf->bucket.prev) buf->bucket.prev->bucket.next = buf->bucket.next;
  if (pool->hashtable[slot] == buf) pool->hashtable[slot] = buf->bucket.next;
  buf->bucket.next = NULL;
  buf->bucket.prev = NULL;
}

//
// lookup_buffer
//

static struct buf *lookup_buffer(struct bufpool *pool, blkno_t blkno) {
  struct buf *buf;

  buf = pool->hashtable[bufhash(blkno) % BUFPOOL_HASHSIZE];
  while (buf && buf->blkno != blkno) buf = buf->bucket.next;
  if (!buf) return NULL;

  switch (buf->state) {
    case BUF_STATE_FREE:
      panic("free buffer in cache");
      break;

    case BUF_STATE_CLEAN:
      // Remove from clean list
      if (buf->chain.next) buf->chain.next->chain.prev = buf->chain.prev;
      if (buf->chain.prev) buf->chain.prev->chain.next = buf->chain.next;
      if (pool->clean.head == buf) pool->clean.head = buf->chain.next;
      if (pool->clean.tail == buf) pool->clean.tail = buf->chain.prev;

      // Set state to locked and add lock
      change_state(pool, buf, BUF_STATE_LOCKED);
      buf->chain.next = NULL;
      buf->chain.prev = NULL;
      buf->locks++;
      break;

    case BUF_STATE_DIRTY:
      // Remove from dirty list
      if (buf->chain.next) buf->chain.next->chain.prev = buf->chain.prev;
      if (buf->chain.prev) buf->chain.prev->chain.next = buf->chain.next;
      if (pool->dirty.head == buf) pool->dirty.head = buf->chain.next;
      if (pool->dirty.tail == buf) pool->dirty.tail = buf->chain.prev;

      // Set state to updated and add lock
      change_state(pool, buf, BUF_STATE_UPDATED);
      buf->chain.next = NULL;
      buf->chain.prev = NULL;
      buf->locks++;
      break;

    case BUF_STATE_READING:
    case BUF_STATE_WRITING:
      // Wait for buffer to become ready. Buffer is locked by reader/writer
      wait_for_buffer(buf);
      break;

    case BUF_STATE_LOCKED:
    case BUF_STATE_UPDATED:
      // Add lock
      buf->locks++;
      break;

    case BUF_STATE_INVALID:
      // Set state to locked, add lock and clear buffer
      change_state(pool, buf, BUF_STATE_LOCKED);
      buf->locks++;
      memset(buf->data, 0, pool->bufsize);
      break;

    case BUF_STATE_ERROR:
      // Buffer is in error, lock buffer and return buffer anyway
      buf->locks++;
      break;

    default:
      panic("invalid buffer state");
  }

  return buf;
}

//
// read_buffer
//

static int read_buffer(struct bufpool *pool, struct buf *buf) {
  int rc;

  // Read block from device into buffer
  change_state(pool, buf, BUF_STATE_READING);
  pool->ioactive = 1;
  rc = dev_read(pool->devno, buf->data, pool->bufsize, buf->blkno * pool->blks_per_buffer, 0);
  if (rc != pool->bufsize) {
    // Set buffer in error state and release all waiters
    kprintf(KERN_ERR "bufpool: error %d reading block %d from %s\n", rc, buf->blkno, device(pool->devno)->name);
    change_state(pool, buf, BUF_STATE_ERROR);
    release_buffer_waiters(buf, rc);
  } else {
    // Lock buffer and release all waiters
    change_state(pool, buf, BUF_STATE_LOCKED);
    release_buffer_waiters(buf, 0);
  }

  pool->blocks_read++;
  return rc;
}

//
// write_buffer
//

static int write_buffer(struct bufpool *pool, struct buf *buf, int flush)
{
  int rc;

  if (buf->state == BUF_STATE_DIRTY) {
    // Remove buffer from dirty list
    if (buf->chain.next) buf->chain.next->chain.prev = buf->chain.prev;
    if (buf->chain.prev) buf->chain.prev->chain.next = buf->chain.next;
    if (pool->dirty.head == buf) pool->dirty.head = buf->chain.next;
    if (pool->dirty.tail == buf) pool->dirty.tail = buf->chain.prev;

    buf->chain.next = NULL;
    buf->chain.prev = NULL;

    // Write block to device from buffer
    change_state(pool, buf, BUF_STATE_WRITING);
    if (!flush) pool->ioactive = 1;
    rc = dev_write(pool->devno, buf->data, pool->bufsize, buf->blkno * pool->blks_per_buffer, 0);
    pool->blocks_written++;
  } else {
    rc = pool->bufsize;
  }

  if (rc != pool->bufsize) {
    // Set buffer in error state and release all waiters
    kprintf(KERN_ERR "bufpool: error %d writing block %d to %s\n", rc, buf->blkno, device(pool->devno)->name);
    change_state(pool, buf, BUF_STATE_ERROR);
    release_buffer_waiters(buf, rc);
  } else {
    // Lock buffer and release all waiters
    change_state(pool, buf, BUF_STATE_LOCKED);
    release_buffer_waiters(buf, 0);
  }

  return rc;
}

//
// get_new_buffer
//

static struct buf *get_new_buffer(struct bufpool *pool) {
  struct buf *buf;

  while (1) {
    // Take buffer from free list if it is not empty
    if (pool->freelist) {
      // Remove buffer from free list
      buf = pool->freelist;
      pool->freelist = buf->chain.next;

      buf->chain.next = NULL;
      buf->chain.prev = NULL;

      return buf;
    }

    // If the clean list is not empty, take the least recently used clean buffer
    if (pool->clean.head) {
      // Remove buffer from clean list
      buf = pool->clean.head;
      if (buf->chain.next) buf->chain.next->chain.prev = NULL;
      pool->clean.head = buf->chain.next;
      if (pool->clean.tail == buf) pool->clean.tail = buf->chain.next;

      buf->chain.next = NULL;
      buf->chain.prev = NULL;

      // Remove buffer from hash table
      remove_from_hashtable(pool, buf);

      return buf;
    }

    // If the dirty list is not empty, write the oldest buffer and try to aquire it
    if (pool->dirty.head) {
      buf = pool->dirty.head;

      // Write the least recently changed buffer to the device
      if (!(write_buffer(pool, buf, 0) < 0)) {
        // Only use the buffer if it has not been locked by other buffer waiters
        if (buf->locks == 0) {
          // Remove buffer from hash table and return buffer
          remove_from_hashtable(pool, buf);

          return buf;
        }
      }
    }

    // Allocation from neither the free, clean or dirty list succeeded, yield and try again
    yield();
  }
}

//
// check_sync
//

static void check_sync() {
  struct bufpool *pool;
  time_t now = time(NULL);

  for (pool = bufpools; pool; pool = pool->next) {
    if (now - pool->last_sync >=  SYNC_INTERVAL) {
      //dump_pool_stat(pool);
      flush_buffers(pool, 0);
      sync_buffers(pool, 0);
    }
  }
}

//
// lazywriter_task
//

static void lazywriter_task(void *arg) {
  struct bufpool *pool;
  int rc;

  while (1) {
    sync_active = 0;
    rc = wait_for_object(&dirty_buffers, SYNC_INTERVAL * 1000);
    sync_active = 1;

    if (rc >= 0) {
      for (pool = bufpools; pool; pool = pool->next) {
        // Wait until we get one second with no activity
        while (pool->ioactive) {
          pool->ioactive = 0;
          msleep(1000);
          check_sync();
        }

        // Flush all dirty buffers
        //dump_pool_stat(pool);
        if (flush_buffers(pool, 1) < 0) set_event(&dirty_buffers);
      }
    }

    check_sync();
  }
}

//
// init_buffer_pool
//

struct bufpool *init_buffer_pool(dev_t devno, int poolsize, int bufsize, void (*sync)(void *arg), void *syncarg) {
  struct bufpool *pool;
  struct buf *buf;
  char *data;
  int i;
  int blksize;

  // Get blocksize from device
  blksize = dev_ioctl(devno, IOCTL_GETBLKSIZE, NULL, 0);
  if (blksize < 0) return  NULL;

  // Allocate and initialize buffer pool structure
  pool = (struct bufpool *) kmalloc(sizeof(struct bufpool));
  if (pool == NULL) return NULL;
  memset(pool, 0, sizeof(struct bufpool));

  pool->devno = devno;
  pool->poolsize = poolsize;
  pool->bufsize = bufsize;
  pool->blks_per_buffer = bufsize / blksize;
  pool->sync = sync;
  pool->syncarg = syncarg;
  pool->last_sync = time(NULL);

  // Allocate buffer headers
  pool->bufbase = (struct buf *) kmalloc(sizeof(struct buf) * poolsize);
  if (pool->bufbase == NULL) {
    kfree(pool);
    return NULL;
  }
  memset(pool->bufbase, 0, sizeof(struct buf) * poolsize);
  
  // Allocate data buffers
  pool->database = (char *) kmalloc_tag(poolsize * bufsize, 'CACH');
  if (pool->database == NULL) {
    kfree(pool->bufbase);
    kfree(pool);
    return NULL;
  }
  memset(pool->database, 0, poolsize * bufsize);

  // Insert all buffers in freelist
  buf = pool->bufbase;
  data = pool->database;
  pool->freelist = pool->bufbase;
  for (i = 0; i < poolsize; i++) {
    buf->data = data;
    if (i == poolsize - 1) {
      buf->chain.next = NULL;
    } else {
      buf->chain.next = buf + 1;
    }

    buf->chain.prev = NULL;

    buf++;
    data += bufsize;
  }
  pool->bufcount[BUF_STATE_FREE] = poolsize;

  // Insert buffer pool in buffer pool list
  pool->next = bufpools;
  pool->prev = NULL;
  if (bufpools) bufpools->prev = pool;
  bufpools = pool;

  // Start lazy writer if not already done
  if (!lazywriter_started) {
    init_event(&dirty_buffers, 0, 0);
    lazywriter_thread = create_kernel_thread(lazywriter_task, NULL, PRIORITY_BELOW_NORMAL, "lazywriter");
    lazywriter_started = 1;

    register_proc_inode("bufpools", bufpools_proc, NULL);
    register_proc_inode("bufstats", bufstats_proc, NULL);
  }

  return pool;
}

//
// free_buffer_pool
//

void free_buffer_pool(struct bufpool *pool) {
  // Wait until sync idle, need to sleep to allow low priority job to finish
  while (sync_active) msleep(100);

  // Remove from buffer pool list
  if (pool->next) pool->next->prev = pool->prev;
  if (pool->prev) pool->prev->next = pool->next;
  if (pool == bufpools) bufpools = pool->next;

  // Deallocate all data
  kfree(pool->database);
  kfree(pool->bufbase);
  kfree(pool);
}

//
// get_buffer
//

struct buf *get_buffer(struct bufpool *pool, blkno_t blkno) {
  struct buf *buf;
  int rc;

  // Try to get buffer from cache
  buf = lookup_buffer(pool, blkno);
  if (buf)  {
    if (buf->state == BUF_STATE_ERROR) {
      release_buffer(pool, buf);
      return NULL;
    } else {
      pool->cache_hits++;
      return buf;
    }
  }

  // Buffer not cached, get new buffer from buffer pool
  buf = get_new_buffer(pool);
  if (!buf) return NULL;

  // Insert buffer into hash table
  buf->blkno = blkno;
  insert_into_hashtable(pool, buf);

  // Add lock on buffer
  buf->locks++;

  // Read block from device
  rc = read_buffer(pool, buf);
  if (rc < 0) {
    release_buffer(pool, buf);
    return NULL;
  }

  pool->cache_misses++;
  return buf;
}

//
// alloc_buffer
//

struct buf *alloc_buffer(struct bufpool *pool, blkno_t blkno) {
  struct buf *buf;

  // Try to get buffer from cache
  buf = lookup_buffer(pool, blkno);
  if (buf) {
    if (buf->state == BUF_STATE_ERROR) {
      release_buffer(pool, buf);
      return NULL;
    } else {
      pool->blocks_allocated++;
      memset(buf->data, 0, pool->bufsize);
      return buf;
    }
  }

  // Buffer not cached, get new buffer from buffer pool
  buf = get_new_buffer(pool);
  if (!buf) return NULL;

  // Insert buffer into hash table
  buf->blkno = blkno;
  insert_into_hashtable(pool, buf);

  // Clear buffer
  memset(buf->data, 0, pool->bufsize);

  // Lock buffer
  change_state(pool, buf, BUF_STATE_LOCKED);
  buf->locks++;

  pool->blocks_allocated++;
  return buf;
}

//
// mark_buffer_updated
//

void mark_buffer_updated(struct bufpool *pool, struct buf *buf) {
  if (buf->state == BUF_STATE_LOCKED || buf->state == BUF_STATE_INVALID) {
    change_state(pool, buf, BUF_STATE_UPDATED);
  }
}

//
// mark_buffer_invalid
//

void mark_buffer_invalid(struct bufpool *pool, struct buf *buf) {
  if (buf->state == BUF_STATE_LOCKED || buf->state == BUF_STATE_UPDATED) {
    change_state(pool, buf, BUF_STATE_INVALID);
  }
}

//
// release_buffer
//

void release_buffer(struct bufpool *pool, struct buf *buf)
{
  // Release lock on buffer
  buf->locks--;
  if (buf->locks > 0) return;

  // Last lock on buffer released
  switch (buf->state) {
    case BUF_STATE_LOCKED:
      // Mark buffer clean and insert in clean list
      change_state(pool, buf, BUF_STATE_CLEAN);
      buf->chain.next = NULL;
      buf->chain.prev = pool->clean.tail;
      if (pool->clean.tail) pool->clean.tail->chain.next = buf;
      pool->clean.tail = buf;
      if (!pool->clean.head) pool->clean.head = buf;
      break;

    case BUF_STATE_UPDATED:
      // Mark buffer dirty and insert in dirty list
      change_state(pool, buf, BUF_STATE_DIRTY);
      buf->chain.next = NULL;
      buf->chain.prev = pool->dirty.tail;
      if (pool->dirty.tail) pool->dirty.tail->chain.next = buf;
      pool->dirty.tail = buf;
      if (!pool->dirty.head) pool->dirty.head = buf;
      set_event(&dirty_buffers);
      pool->blocks_updated++;
      break;

    case BUF_STATE_INVALID:
    case BUF_STATE_ERROR:
      // Remove from hashtable, mark buffer free and insert in free list
      remove_from_hashtable(pool, buf);
      change_state(pool, buf, BUF_STATE_FREE);
      buf->chain.next = pool->freelist;
      buf->chain.prev = NULL;
      pool->freelist = buf;
      pool->blocks_freed++;
      break;

    case BUF_STATE_FREE:
    case BUF_STATE_CLEAN:
    case BUF_STATE_DIRTY:
    case BUF_STATE_READING:
    case BUF_STATE_WRITING:

    default:
      panic("invalid buffer state");
  }
}

//
// invalidate_buffer
//

void invalidate_buffer(struct bufpool *pool, blkno_t blkno) {
  struct buf *buf;

  // If buffer is not in cache we are finished
  buf = lookup_buffer(pool, blkno);
  if (!buf) return;

  // Mark buffer as invalid and release it
  mark_buffer_invalid(pool, buf);
  release_buffer(pool, buf);
}

//
// flush_buffers
//

int flush_buffers(struct bufpool *pool, int interruptable) {
  struct buf *buf;
  int rc;

  // Do not flush if nosync flag is set
  if (pool->nosync) return 0;

  pool->ioactive = 0;
  while (pool->dirty.head) {
    // Check for interrupt
    if (interruptable && pool->ioactive) return -EINTR;

    // Get next buffer from dirty list
    buf = pool->dirty.head;

    // Flush buffer to device
    rc = write_buffer(pool, buf, 1);
    if (rc < 0) return rc;

    pool->blocks_lazywrite++;

    // Move buffer to clean list if it is not locked
    if (buf->locks == 0) {
      change_state(pool, buf, BUF_STATE_CLEAN);
      buf->chain.next = NULL;
      buf->chain.prev = pool->clean.tail;
      if (pool->clean.tail) pool->clean.tail->chain.next = buf;
      pool->clean.tail = buf;
      if (!pool->clean.head) pool->clean.head = buf;
    }
  }

  return 0;
}

//
// sync_buffers
//

int sync_buffers(struct bufpool *pool, int interruptable) {
  struct buf *buf;
  int i;
  int rc;

  // Do not sync if nosync flag is set
  if (pool->nosync) return 0;

  // Call user sync function
  if (pool->sync) pool->sync(pool->syncarg);

  // If there are no updated buffers then there is nothing to do
  if (pool->bufcount[BUF_STATE_UPDATED] == 0) {
    pool->last_sync = time(NULL);
    return 0;
  }

  // Find all updated buffers
  pool->ioactive = 0;
  buf = pool->bufbase;
  for (i = 0; i < pool->poolsize; i++) {
    if (buf->state == BUF_STATE_UPDATED) {
      // Check for interrupt
      if (interruptable && pool->ioactive) return -EINTR;

      // Lock buffer
      buf->locks++;
      
      // Flush buffer to device
      //kprintf("sync block %d\n", buf->blkno);
      rc = dev_write(pool->devno, buf->data, pool->bufsize, buf->blkno * pool->blks_per_buffer, 0);
      if (rc < 0) return rc;
      
      pool->blocks_written++;
      pool->blocks_synched++;

      // Change state from updated to locked
      change_state(pool, buf, BUF_STATE_LOCKED);

      // Release lock
      release_buffer(pool, buf);
    }

    buf++;
  }

  pool->last_sync = time(NULL);
  return 0;
}
