//
// iomux.c
//
// I/O multiplexing
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

static void release_waiting_threads(struct iomux *iomux)
{
  struct waitblock *wb;
  struct waitblock *wb_next;
  struct ioobject *iob;

  // Dispatch all ready I/O objects to all ready waiting threads
  wb = iomux->object.waitlist_head;
  iob = iomux->ready_head;
  while (iob && wb)
  {
    wb_next = wb->next_wait;

    if (thread_ready_to_run(wb->thread))
    {
      // Overwrite waitkey for thread with context for object
      wb->thread->waitkey = iob->context;

      // Remove object from iomux
      detach_ioobject(iob);

      // Mark thread ready
      release_thread(wb->thread);

      iob = iomux->ready_head;
    }

    wb = wb_next;
  }
}

void init_iomux(struct iomux *iomux, int flags)
{
  init_object(&iomux->object, OBJECT_IOMUX);
  iomux->flags = flags;
  iomux->ready_head = iomux->ready_tail = NULL;
  iomux->waiting_head = iomux->waiting_tail = NULL;
}

int close_iomux(struct iomux *iomux)
{
  struct ioobject *iob;
  struct ioobject *next;

  // Remove all objects from ready queue
  iob = iomux->ready_head;
  while (iob)
  {
    next = iob->next;
    iob->iomux = NULL;
    iob->next = NULL;
    iob->prev = NULL;
    iob = next;
  }

  // Remove all objects from waiting queue
  iob = iomux->waiting_head;
  while (iob)
  {
    next = iob->next;
    iob->iomux = NULL;
    iob->next = NULL;
    iob->prev = NULL;
    iob = next;
  }

  return 0;
}

int queue_ioobject(struct iomux *iomux, object_t hobj, int events, int context)
{
  struct ioobject *iob = (struct ioobject *) hobj;

  if (iob->object.type != OBJECT_SOCKET) return -EBADF;
  if (!events) return -EINVAL;

  if (iob->iomux) 
  {
    // Do not allow already attached object to attach to another iomux
    if (iob->iomux != iomux) return -EPERM;

    // Update the event monitoring mask
    events |= iob->events_monitored;

    // Detach object, it will be inserted in the appropriate queue further down 
    detach_ioobject(iob);
  }

  iob->iomux = iomux;
  iob->events_monitored = events;
  iob->context = context;
  
  // If some signaled event is monitored insert in ready queue else in waiting queue
  if (iob->events_monitored & iob->events_signaled)
  {
    iob->next = NULL;
    iob->prev = iomux->ready_tail;

    if (iomux->ready_tail) iomux->ready_tail->next = iob;
    iomux->ready_tail = iob;
    if (!iomux->ready_head) iomux->ready_head = iob;

    iomux->object.signaled = 1;
  }
  else
  {
    iob->next = NULL;
    iob->prev = iomux->waiting_tail;

    if (iomux->waiting_tail) iomux->waiting_tail->next = iob;
    iomux->waiting_tail = iob;
    if (!iomux->waiting_head) iomux->waiting_head = iob;
  }

  // If iomux is signaled try to dispatch ready objects to waiting threads
  if (iob->object.signaled) release_waiting_threads(iomux);

  return 0;
}

void init_ioobject(struct ioobject *iob, int type)
{
  init_object(&iob->object, type);
  iob->iomux = NULL;
  iob->context = 0;
  iob->next = iob->prev = NULL;
  iob->events_signaled = iob->events_monitored = 0;
}

void detach_ioobject(struct ioobject *iob)
{
  if (iob->iomux)
  {
    if (iob->next) iob->next->prev = iob->prev;
    if (iob->prev) iob->prev->next = iob->next;

    if (iob->events_monitored & iob->events_signaled)
    {
      // Remove from ready queue
      if (iob->iomux->ready_head == iob) iob->iomux->ready_head = iob->next; 
      if (iob->iomux->ready_tail == iob) iob->iomux->ready_tail = iob->prev; 

      // If ready queue is empty the iomux is no longer signaled
      if (!iob->iomux->ready_head) iob->iomux->object.signaled = 0;
    }
    else
    {
      // Remove object from waiting queue
      if (iob->iomux->waiting_head == iob) iob->iomux->waiting_head = iob->next; 
      if (iob->iomux->waiting_tail == iob) iob->iomux->waiting_tail = iob->prev; 
    }

    iob->iomux = NULL;
    iob->next = iob->prev = NULL;
  }
}

void set_io_event(struct ioobject *iob, int events)
{
  struct iomux *iomux = iob->iomux;
  //
  // If the following is true we must move the object to the ready queue and signal it:
  //  1) object is attached to an iomux
  //  2) object is on the waiting queue
  //  3) the new event(s) are being monitored.
  //

  if (iomux && (iob->events_monitored & iob->events_signaled) == 0 && (iob->events_monitored & events) != 0)
  {
    // Update signaled events
    iob->events_signaled |= events;

    // Remove object from waiting queue
    if (iob->next) iob->next->prev = iob->prev;
    if (iob->prev) iob->prev->next = iob->next;
    if (iomux->waiting_head == iob) iomux->waiting_head = iob->next; 
    if (iomux->waiting_tail == iob) iomux->waiting_tail = iob->prev; 

    // Insert object in the ready queue
    iob->next = NULL;
    iob->prev = iomux->ready_tail;
    if (iomux->ready_tail) iomux->ready_tail->next = iob;
    iomux->ready_tail = iob;
    if (!iomux->ready_head) iomux->ready_head = iob;

    // Signal object
    iob->object.signaled = 1;

    // Try to dispatch ready objects to waiting threads
    release_waiting_threads(iomux);
  }
  else
  {
    // Just update the signaled event for object
    iob->events_signaled |= events;
  }
}

void clear_io_event(struct ioobject *iob, int events)
{
  // Clear events
  iob->events_signaled &= ~events;
}

int dequeue_event_from_iomux(struct iomux *iomux)
{
  struct ioobject *iob;

  // Get first ready object, return error if no objects are ready
  iob = iomux->ready_head;
  if (!iob) return -ENOENT;

  // Detach object from iomux
  detach_ioobject(iob);

  // Return context for object
  return iob->context;
}

int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout)
{
  return -ENOSYS;
}
