#include <windows.h>

#include "types.h"
#include "buf.h"

unsigned long threadtls;

void panic(char *reason);
int dev_read(devno_t devno, void *buffer, size_t count, blkno_t blkno);
int dev_write(devno_t devno, void *buffer, size_t count, blkno_t blkno);

struct thread *get_current_thread()
{
  struct thread *thread;

  thread = TlsGetValue(threadtls);
  if (thread == NULL)
  {
    thread = (struct thread *) kmalloc(sizeof(struct thread));

    thread->ready = CreateEvent(NULL, TRUE, FALSE, NULL);
    thread->next_buffer_waiter = NULL;
    TlsSetValue(threadtls, thread);
  }

  return thread;
}

static void yield()
{
  Sleep(0);
}

static unsigned long bufhash(blkno_t blkno)
{
  return blkno;
}

static void wait_for_buffer(struct buf *buf)
{
  struct thread *self = get_current_thread();
  self->next_buffer_waiter = buf->waiters;
  buf->waiters = self;
  ResetEvent(self->ready);
  WaitForSingleObject(self->ready, INFINITE);
}

static void mark_ready(struct thread *thread)
{
  SetEvent(thread->ready);
}

static void insert_into_hashtable(struct bufpool *pool, struct buf *buf)
{
  int slot;

  slot = bufhash(buf->blkno) % BUFPOOL_HASHSIZE;
  if (pool->hashtable[slot]) pool->hashtable[slot]->bucket.prev = buf;
  buf->bucket.next = pool->hashtable[slot];
  buf->bucket.prev = NULL;
  pool->hashtable[slot] = buf;
}

static void remove_from_hashtable(struct bufpool *pool, struct buf *buf)
{
  int slot;

  slot = bufhash(buf->blkno) % BUFPOOL_HASHSIZE;
  if (buf->bucket.next) buf->bucket.next->bucket.prev = buf->bucket.prev;
  if (buf->bucket.prev) buf->bucket.prev->bucket.next = buf->bucket.next;
  if (pool->hashtable[slot] == buf) pool->hashtable[slot] = buf->bucket.next;
  buf->bucket.next = NULL;
  buf->bucket.prev = NULL;
}

static struct buf *lookup_buffer(struct bufpool *pool, blkno_t blkno)
{
  struct buf *buf;

  buf = pool->hashtable[bufhash(blkno) % BUFPOOL_HASHSIZE];
  while (buf && buf->blkno != blkno) buf = buf->bucket.next;
  if (!buf) return NULL;

  switch (buf->state)
  {
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
      buf->chain.next = NULL;
      buf->chain.prev = NULL;
      buf->state = BUF_STATE_LOCKED;
      buf->locks++;
      break;

    case BUF_STATE_DIRTY:
      // Remove from dirty list
      if (buf->chain.next) buf->chain.next->chain.prev = buf->chain.prev;
      if (buf->chain.prev) buf->chain.prev->chain.next = buf->chain.next;
      if (pool->dirty.head == buf) pool->dirty.head = buf->chain.next;
      if (pool->dirty.tail == buf) pool->dirty.tail = buf->chain.prev;

      // Set state to updated and add lock
      buf->chain.next = NULL;
      buf->chain.prev = NULL;
      buf->state = BUF_STATE_UPDATED;
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
      buf->state = BUF_STATE_LOCKED;
      buf->locks++;
      memset(buf->data, 0, pool->bufsize);
      break;

    default:
      panic("invalid buffer state");
  }

  return buf;
}

static void read_buffer(struct bufpool *pool, struct buf *buf)
{
  struct thread *thread;
  struct thread *next;

  // Read block from device into buffer
  buf->state = BUF_STATE_READING;
  dev_read(pool->devno, buf->data, pool->bufsize, buf->blkno * pool->blks_per_buffer);

  // Lock buffer and release all waiters
  buf->state = BUF_STATE_LOCKED;
  thread = buf->waiters;
  while (thread)
  {
    buf->locks++;
    next = thread->next_buffer_waiter;
    thread->next_buffer_waiter = NULL;

    mark_ready(thread);

    thread = next;
  }
  buf->waiters = NULL;
}

static void write_buffer(struct bufpool *pool, struct buf *buf)
{
  struct thread *thread;
  struct thread *next;

  if (buf->state == BUF_STATE_DIRTY)
  {
    // Remove buffer from dirty list
    if (buf->chain.next) buf->chain.next->chain.prev = buf->chain.prev;
    if (buf->chain.prev) buf->chain.prev->chain.next = buf->chain.next;
    if (pool->dirty.head == buf) pool->dirty.head = buf->chain.next;
    if (pool->dirty.tail == buf) pool->dirty.tail = buf->chain.prev;

    buf->chain.next = NULL;
    buf->chain.prev = NULL;

    // Write block to device from buffer
    buf->state = BUF_STATE_WRITING;
    dev_write(pool->devno, buf->data, pool->bufsize, buf->blkno * pool->blks_per_buffer);
  }

  // Lock buffer and release all waiters
  buf->state = BUF_STATE_LOCKED;
  thread = buf->waiters;
  while (thread)
  {
    buf->locks++;
    next = thread->next_buffer_waiter;
    thread->next_buffer_waiter = NULL;
    
    SetEvent(thread->ready);
    thread = next;
  }
  buf->waiters = NULL;
}

static struct buf *get_new_buffer(struct bufpool *pool)
{
  struct buf *buf;

  while (1)
  {
    // Take buffer from free list if it is not empty
    if (pool->freelist)
    {
      // Remove buffer from free list
      buf = pool->freelist;
      pool->freelist = buf->chain.next;

      buf->chain.next = NULL;
      buf->chain.prev = NULL;

      return buf;
    }

    // If the clean list is not empty, take the least recently used clean buffer
    if (pool->clean.head)
    {
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
    if (pool->dirty.head)
    {
      buf = pool->dirty.head;

      // Write the least recently changed buffer to the device
      write_buffer(pool, buf);

      // Only use the buffer if it has not been locked by other buffer waiters
      if (buf->locks == 0)
      {
	// Remove buffer from hash table and return buffer
	remove_from_hashtable(pool, buf);

	return buf;
      }
    }

    // Allocation from neither the free, clean or dirty list succeeded, yield and try again
    yield();
  }
}

struct bufpool *init_buffer_pool(devno_t devno, int poolsize, int bufsize)
{
  struct bufpool *pool;
  struct buf *buf;
  char *data;
  int i;

  pool = (struct bufpool *) kmalloc(sizeof(struct bufpool));
  if (pool == NULL) return NULL;
  memset(pool, 0, sizeof(struct bufpool));

  pool->devno = devno;
  pool->poolsize = poolsize;
  pool->bufsize = bufsize;
  pool->blks_per_buffer = bufsize / 512; // TODO: get blksize from device

  pool->bufbase = (struct buf *) kmalloc(sizeof(struct buf) * poolsize);
  if (pool->bufbase == NULL)
  {
    kfree(pool);
    return NULL;
  }
  memset(pool->bufbase, 0, sizeof(struct buf) * poolsize);
  
  pool->database = (char *) kmalloc(poolsize * bufsize);
  if (pool->database == NULL)
  {
    kfree(pool->bufbase);
    kfree(pool);
    return NULL;
  }
  memset(pool->database, 0, poolsize * bufsize);

  buf = pool->bufbase;
  data = pool->database;

  pool->freelist = pool->bufbase;
  for (i = 0; i < poolsize; i++)
  {
    buf->data = data;
    if (i == poolsize - 1)
      buf->chain.next = NULL;
    else
      buf->chain.next = buf + 1;

    buf->chain.prev = NULL;

    buf++;
    data += bufsize;
  }

  threadtls = TlsAlloc();
  return pool;
}

void free_buffer_pool(struct bufpool *pool)
{
  kfree(pool->database);
  kfree(pool->bufbase);
  kfree(pool);
}

struct buf *get_buffer(struct bufpool *pool, blkno_t blkno)
{
  struct buf *buf;

  // Try to get buffer from cache
  buf = lookup_buffer(pool, blkno);
  if (buf) return buf;

  // Buffer not cached, get new buffer from buffer pool
  buf = get_new_buffer(pool);

  // Insert buffer into hash table
  buf->blkno = blkno;
  insert_into_hashtable(pool, buf);

  // Read block from device
  read_buffer(pool, buf);

  // Add lock on buffer
  buf->locks++;

  return buf;
}

struct buf *alloc_buffer(struct bufpool *pool, blkno_t blkno)
{
  struct buf *buf;

  // Try to get buffer from cache
  buf = lookup_buffer(pool, blkno);
  if (buf) 
  {
    memset(buf->data, 0, pool->bufsize);
    return buf;
  }

  // Buffer not cached, get new buffer from buffer pool
  buf = get_new_buffer(pool);

  // Insert buffer into hash table
  buf->blkno = blkno;
  insert_into_hashtable(pool, buf);

  // Clear buffer
  memset(buf->data, 0, pool->bufsize);

  // Lock buffer
  buf->state = BUF_STATE_LOCKED;
  buf->locks++;

  return buf;
}

void mark_buffer_updated(struct buf *buf)
{
  if (buf->state == BUF_STATE_LOCKED || buf->state == BUF_STATE_INVALID)
  {
    buf->state = BUF_STATE_UPDATED;
  }
}

void mark_buffer_invalid(struct buf *buf)
{
  if (buf->state == BUF_STATE_LOCKED || buf->state == BUF_STATE_UPDATED)
  {
    buf->state = BUF_STATE_INVALID;
  }
}

void release_buffer(struct bufpool *pool, struct buf *buf)
{
  // Release lock on buffer
  buf->locks--;
  if (buf->locks > 0) return;

  // Last lock on buffer released
  switch (buf->state)
  {
    case BUF_STATE_LOCKED:
      // Mark buffer clean and insert in clean list
      buf->state = BUF_STATE_CLEAN;
      buf->chain.next = NULL;
      buf->chain.prev = pool->clean.tail;
      if (pool->clean.tail) pool->clean.tail->chain.next = buf;
      pool->clean.tail = buf;
      if (!pool->clean.head) pool->clean.head = buf;
      break;

    case BUF_STATE_UPDATED:
      // Mark buffer dirty and insert in dirty list
      buf->state = BUF_STATE_DIRTY;
      buf->chain.next = NULL;
      buf->chain.prev = pool->dirty.tail;
      if (pool->dirty.tail) pool->dirty.tail->chain.next = buf;
      pool->dirty.tail = buf;
      if (!pool->dirty.head) pool->dirty.head = buf;
      break;

    case BUF_STATE_INVALID:
      // Mark buffer free and insert in free list
      buf->state = BUF_STATE_FREE;
      buf->chain.next = pool->freelist;
      buf->chain.prev = NULL;
      pool->freelist = buf;
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

void flush_buffers(struct bufpool *pool)
{
  struct buf *buf;

  while (pool->dirty.head)
  {
    // Get next buffer from dirty list
    buf = pool->dirty.head;

    // Flush buffer to device
    write_buffer(pool, buf);

    // Move buffer to clean list if it is not locked
    if (buf->locks == 0)
    {
      buf->state = BUF_STATE_CLEAN;
      buf->chain.next = NULL;
      buf->chain.prev = pool->clean.tail;
      if (pool->clean.tail) pool->clean.tail->chain.next = buf;
      pool->clean.tail = buf;
      if (!pool->clean.head) pool->clean.head = buf;
    }
  }
}

