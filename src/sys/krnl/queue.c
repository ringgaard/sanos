//
// queue.c
//
// Message queue
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

struct queue *alloc_queue(int size) {
  struct queue *q = (struct queue *) kmalloc(sizeof(struct queue));
  if (!q) return q;

  q->elems = (void **) kmalloc(size * sizeof(void *));
  if (!q->elems) {
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

void free_queue(struct queue *q) {
  kfree(q->elems);
  kfree(q);
}

int enqueue(struct queue *q, void *msg, unsigned int timeout) {
  if (wait_for_object(&q->notfull, timeout) < 0) return -ETIMEOUT;
  q->elems[q->in++] = msg;
  if (q->in == q->size) q->in = 0;
  release_sem(&q->notempty, 1);
  return 0;
}

void *dequeue(struct queue *q, unsigned int timeout) {
  void *msg;

  if (wait_for_object(&q->notempty, timeout) < 0) return NULL;
  msg = q->elems[q->out++];
  if (q->out == q->size) q->out = 0;
  release_sem(&q->notfull, 1);
  return msg;
}
