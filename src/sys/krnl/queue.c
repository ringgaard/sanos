//
// queue.c
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Message queue
//

#include <os/krnl.h>

struct queue *alloc_queue(int size)
{
  struct queue *q = (struct queue *) kmalloc(sizeof(struct queue));
  if (!q) return q;

  q->elems = (void **) kmalloc(size * sizeof(void *));
  if (!q->elems)
  {
    kfree(q);
    return NULL;
  }

  init_sem(&q->notfull, size);
  init_sem(&q->notempty, 0);

  q->size = size;
  q->in = 0;
  q->out = 0;

  return q;
}

void free_queue(struct queue *q)
{
  kfree(q->elems);
  kfree(q);
}

int enqueue(struct queue *q, void *msg, unsigned int timeout)
{
  if (wait_for_object(&q->notfull, timeout) < 0) return -ETIMEOUT;
  q->elems[q->in++] = msg;
  if (q->in == q->size) q->in = 0;
  release_sem(&q->notempty, 1);
  return 0;
}

void *dequeue(struct queue *q, unsigned int timeout)
{
  void *msg;

  if (wait_for_object(&q->notempty, timeout) < 0) return NULL;
  msg = q->elems[q->out++];
  if (q->out == q->size) q->out = 0;
  release_sem(&q->notfull, 1);
  return msg;
}
