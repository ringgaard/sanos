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

static void dump_iomux(struct iomux *iomux) {
  struct ioobject *iob;

  kprintf("iomux %c", iomux->object.signaled ? '+' : '-');
  if (iomux->ready_head) {
    kprintf(" r:");
    iob = iomux->ready_head;
    while (iob) {
      kprintf("%d(%x,%x) ", ((((unsigned long) iob) >> 2) & 0xFF), iob->events_monitored, iob->events_signaled);
      iob = iob->next;
    }
  }

  if (iomux->waiting_head) {
    kprintf(" w:");
    iob = iomux->waiting_head;
    while (iob) {
      kprintf("%d(%x,%x) ", ((((unsigned long) iob) >> 2) & 0xFF), iob->events_monitored, iob->events_signaled);
      iob = iob->next;
    }
  }
  kprintf("\n");
}

static void release_waiting_threads(struct iomux *iomux) {
  struct waitblock *wb;
  struct waitblock *wb_next;
  struct ioobject *iob;

  // Dispatch all ready I/O objects to all ready waiting threads
  wb = iomux->object.waitlist_head;
  iob = iomux->ready_head;
  while (iob && wb) {
    wb_next = wb->next_wait;

    if (thread_ready_to_run(wb->thread)) {
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

void init_iomux(struct iomux *iomux, int flags) {
  init_object(&iomux->object, OBJECT_IOMUX);
  iomux->flags = flags;
  iomux->ready_head = iomux->ready_tail = NULL;
  iomux->waiting_head = iomux->waiting_tail = NULL;
}

int close_iomux(struct iomux *iomux) {
  struct ioobject *iob;
  struct ioobject *next;

  // Remove all objects from ready queue
  iob = iomux->ready_head;
  while (iob) {
    next = iob->next;
    iob->iomux = NULL;
    iob->next = NULL;
    iob->prev = NULL;
    iob = next;
  }

  // Remove all objects from waiting queue
  iob = iomux->waiting_head;
  while (iob) {
    next = iob->next;
    iob->iomux = NULL;
    iob->next = NULL;
    iob->prev = NULL;
    iob = next;
  }

  return 0;
}

int queue_ioobject(struct iomux *iomux, object_t hobj, int events, int context) {
  struct ioobject *iob = (struct ioobject *) hobj;

  if (!ISIOOBJECT(iob)) return -EBADF;
  if (!events) return -EINVAL;

  if (iob->iomux) {
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
  if (iob->events_monitored & iob->events_signaled) {
    iob->next = NULL;
    iob->prev = iomux->ready_tail;

    if (iomux->ready_tail) iomux->ready_tail->next = iob;
    iomux->ready_tail = iob;
    if (!iomux->ready_head) iomux->ready_head = iob;

    iomux->object.signaled = 1;
  } else {
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

void init_ioobject(struct ioobject *iob, int type) {
  init_object(&iob->object, type);
  iob->iomux = NULL;
  iob->context = 0;
  iob->next = iob->prev = NULL;
  iob->events_signaled = iob->events_monitored = 0;
}

void detach_ioobject(struct ioobject *iob) {
  if (iob->iomux) {
    if (iob->next) iob->next->prev = iob->prev;
    if (iob->prev) iob->prev->next = iob->next;

    if (iob->events_monitored & iob->events_signaled) {
      // Remove from ready queue
      if (iob->iomux->ready_head == iob) iob->iomux->ready_head = iob->next; 
      if (iob->iomux->ready_tail == iob) iob->iomux->ready_tail = iob->prev; 

      // If ready queue is empty the iomux is no longer signaled
      if (!iob->iomux->ready_head) iob->iomux->object.signaled = 0;
    } else {
      // Remove object from waiting queue
      if (iob->iomux->waiting_head == iob) iob->iomux->waiting_head = iob->next; 
      if (iob->iomux->waiting_tail == iob) iob->iomux->waiting_tail = iob->prev; 
    }

    iob->iomux = NULL;
    iob->next = iob->prev = NULL;
  }
}

void set_io_event(struct ioobject *iob, int events) {
  struct iomux *iomux = iob->iomux;
  if (iomux) {
    // Object is attached to an iomux. If the object is on the waiting queue
    // and new event(s) are being monitored, we must move the object to the 
    // ready queue and signal it.
    if ((iob->events_monitored & iob->events_signaled) == 0 && (iob->events_monitored & events) != 0) {
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

      // Signal iomux
      iomux->object.signaled = 1;

      // Try to dispatch ready objects to waiting threads
      release_waiting_threads(iomux);
    } else {
      // Just update the signaled event(s) for object
      iob->events_signaled |= events;
    }
  } else {
    // Object is not attached to an iomux. Update the signaled events and signal
    // object if data is available.
    iob->events_signaled |= events;
    if (iob->events_signaled & IOEVT_READ) {
      iob->object.signaled = 1;
      release_waiters(&iob->object, 0);
    }
  }
}

void clear_io_event(struct ioobject *iob, int events) {
  // Clear events
  iob->events_signaled &= ~events;
  if (!(iob->events_signaled & IOEVT_READ)) iob->object.signaled = 0;
}

int dequeue_event_from_iomux(struct iomux *iomux) {
  struct ioobject *iob;

  // Get first ready object, return error if no objects are ready
  iob = iomux->ready_head;
  if (!iob) return -ENOENT;

  // Detach object from iomux
  detach_ioobject(iob);

  // Return context for object
  return iob->context;
}

static int check_fds(fd_set *fds, int eventmask) {
  unsigned int n;
  int matches;
  struct ioobject *iob;

  if (!fds) return 0;

  matches = 0;
  for (n = 0; n < fds->count; n++) {
    iob = (struct ioobject *) olock(fds->fd[n], OBJECT_ANY);
    if (!iob) return -EBADF;
    if (!ISIOOBJECT(iob)) {
      orel(iob);
      return -EBADF;
    }

    if (iob->events_signaled & eventmask) fds->fd[matches++] = fds->fd[n];
    orel(iob);
  }

  return matches;
}

static int add_fds_to_iomux(struct iomux *iomux, fd_set *fds, int eventmask) {
  unsigned int n;
  struct ioobject *iob;
  int rc;

  if (!fds) return 0;

  for (n = 0; n < fds->count; n++) {
    iob = (struct ioobject *) olock(fds->fd[n], OBJECT_ANY);
    if (!iob) return -EBADF;
    if (!ISIOOBJECT(iob)) {
      orel(iob);
      return -EBADF;
    }

    rc = queue_ioobject(iomux, iob, eventmask, 0);
    if (rc < 0) return rc;

    orel(iob);
  }

  return 0;
}

static int check_select(fd_set *readfds, fd_set *writefds, fd_set *exceptfds) {
  int numread;
  int numwrite;
  int numexcept;

  numread = check_fds(readfds, IOEVT_READ | IOEVT_ACCEPT | IOEVT_CLOSE);
  if (numread < 0) return numread;

  numwrite = check_fds(writefds, IOEVT_WRITE | IOEVT_CONNECT);
  if (numwrite < 0) return numwrite;

  numexcept = check_fds(exceptfds, IOEVT_ERROR);
  if (numexcept < 0) return numexcept;

  if (numread != 0 || numwrite != 0 || numexcept != 0) {
    if (readfds) readfds->count = numread;
    if (writefds) writefds->count = numwrite;
    if (exceptfds) exceptfds->count = numexcept;

    return numread + numwrite + numexcept;
  }

  return 0;
}

int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout) {
  unsigned int tmo;
  int rc;
  struct iomux iomux;

  rc = check_select(readfds, writefds, exceptfds);
  if (rc != 0) return rc;

  if (!timeout) {
    tmo = INFINITE;
  } else {
    tmo = timeout->tv_sec * 1000 + timeout->tv_usec / 1000;
  }

  if (tmo == 0) return 0;
  if (timeout && !readfds && !writefds && !exceptfds) {
    return msleep(tmo) > 0 ? -EINTR : 0;
  }
  init_iomux(&iomux, 0);

  rc = add_fds_to_iomux(&iomux, readfds, IOEVT_READ | IOEVT_ACCEPT | IOEVT_CLOSE);
  if (rc < 0) {
    close_iomux(&iomux);
    return rc;
  }

  rc = add_fds_to_iomux(&iomux, writefds, IOEVT_WRITE | IOEVT_CONNECT);
  if (rc < 0) {
    close_iomux(&iomux);
    return rc;
  }

  rc = add_fds_to_iomux(&iomux, exceptfds, IOEVT_ERROR);
  if (rc < 0) {
    close_iomux(&iomux);
    return rc;
  }

  rc = wait_for_object(&iomux, tmo);
  if (rc < 0) {
    close_iomux(&iomux);
    if (rc == -ETIMEOUT) rc = 0;
    return rc;
  }

  rc = check_select(readfds, writefds, exceptfds);
  if (rc == 0) {
    if (readfds) readfds->count = 0;
    if (writefds) writefds->count = 0;
    if (exceptfds) exceptfds->count = 0;
  }

  close_iomux(&iomux);
  return rc;
}

static int check_poll(struct pollfd fds[], unsigned int nfds) {
  struct ioobject *iob;
  unsigned int n;
  int ready;
  int revents;
  int mask;

  ready = 0;
  for (n = 0; n < nfds; n++) {
    revents = 0;
    if (fds[n].fd >= 0) {
      iob = (struct ioobject *) olock(fds[n].fd, OBJECT_ANY);
      if (!iob || !ISIOOBJECT(iob)) {
        revents = POLLNVAL;
        if (iob) orel(iob);
      } else {
        mask = IOEVT_ERROR | IOEVT_CLOSE;
        if (fds[n].events & POLLIN) mask |= IOEVT_READ | IOEVT_ACCEPT;
        if (fds[n].events & POLLOUT) mask |= IOEVT_WRITE | IOEVT_CONNECT;

        mask &= iob->events_signaled;
        if (mask != 0) {
          if (mask & (IOEVT_READ | IOEVT_ACCEPT)) revents |= POLLIN;
          if (mask & (IOEVT_WRITE | IOEVT_CONNECT)) revents |= POLLOUT;
          if (mask & IOEVT_ERROR) revents |= POLLERR;
          if (mask & IOEVT_CLOSE) revents |= POLLHUP;
        }
      }

      orel(iob);
    }
    fds[n].revents = revents;
    if (revents != 0) ready++;
  }

  return ready;
}

static int add_fd_to_iomux(struct iomux *iomux, int fd, int events) {
  struct ioobject *iob;
  int rc;
  int mask;

  if (fd < 0) return 0;
  iob = (struct ioobject *) olock(fd, OBJECT_ANY);
  if (!iob) return -EBADF;
  if (!ISIOOBJECT(iob)) {
    orel(iob);
    return -EBADF;
  }

  mask = IOEVT_ERROR | IOEVT_CLOSE;
  if (events & POLLIN) mask |= IOEVT_READ | IOEVT_ACCEPT;
  if (events & POLLOUT) mask |= IOEVT_WRITE | IOEVT_CONNECT;

  rc = queue_ioobject(iomux, iob, mask, 0);
  if (rc < 0) return rc;

  orel(iob);
  return 0;
}

int poll(struct pollfd fds[], unsigned int nfds, int timeout) {
  struct iomux iomux;
  int rc;
  unsigned int n;

  if (nfds == 0) return msleep(timeout) > 0 ? -EINTR : 0;
  if (!fds) return -EINVAL;

  rc = check_poll(fds, nfds);
  if (rc > 0) return rc;
  if (timeout == 0) return 0;

  init_iomux(&iomux, 0);

  for (n = 0; n < nfds; n++) {
    rc = add_fd_to_iomux(&iomux, fds[n].fd, fds[n].events);
    if (rc < 0) {
      close_iomux(&iomux);
      return rc;
    }
  }

  rc = wait_for_object(&iomux, timeout);
  if (rc < 0) {
    close_iomux(&iomux);
    if (rc == -ETIMEOUT) rc = 0;
    return rc;
  }

  rc = check_poll(fds, nfds);
  close_iomux(&iomux);
  return rc;
}
