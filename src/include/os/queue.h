//
// queue.h
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Message queue
//

#ifndef QUEUE_H
#define QUEUE_H

struct queue
{
  struct sem notempty;
  struct sem notfull;
  int size;
  int in;
  int out;
  void **elems;
};

krnlapi struct queue *alloc_queue(int size);
krnlapi void free_queue(struct queue *q);
krnlapi int enqueue(struct queue *q, void *msg, unsigned int timeout);
krnlapi void *dequeue(struct queue *q, unsigned int timeout);

#endif
