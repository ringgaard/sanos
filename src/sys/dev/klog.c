//
// klog.c
//
// Kernel logging
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

#define KLOG_SIZE (64 * 1024)

struct klogreq {
  struct klogreq *next;
  struct thread *thread;
  err_t rc;
};

int kprint_enabled = 1;

static char klogbuf[KLOG_SIZE];
static unsigned int klog_start;
static unsigned int klog_end;
static unsigned int klog_size;
static struct klogreq *klog_waiters;
static struct dpc klog_dpc;

static int wait_for_klog() {
  struct klogreq req;

  req.thread = self();
  req.next = klog_waiters;
  klog_waiters = &req;

  enter_wait(THREAD_WAIT_DEVIO);

  return req.rc;
}

static void release_klog_waiters(void *arg) {
  struct klogreq *waiter;

  // Defer scheduling of kernel log waiter if we are in a interrupt handler
  if ((eflags() & EFLAG_IF) == 0)  {
    queue_irq_dpc(&klog_dpc, release_klog_waiters, NULL);
    return;
  }

  waiter = klog_waiters;
  while (waiter) {
    waiter->rc = 0;
    mark_thread_ready(waiter->thread, 1, 2);
    waiter = waiter->next;
  }

  klog_waiters = NULL;
}

static void add_to_klog(char *buf, int size) {
  while (size-- > 0) {
    if (klog_size == KLOG_SIZE) {
      while (klogbuf[klog_start] != '\n' && klog_size > 0) {
        klog_size--;
        klog_start++;
        if (klog_start == KLOG_SIZE) klog_start = 0;
      }

      if (klog_size > 0) {
        klog_size--;
        klog_start++;
        if (klog_start == KLOG_SIZE) klog_start = 0;
      }
    }

    klogbuf[klog_end++] = *buf++;
    if (klog_end == KLOG_SIZE) klog_end = 0;
    klog_size++;
  }

  release_klog_waiters(NULL);
}

void kprintf(const char *fmt,...) {
  va_list args;
  char buffer[1024];
  int len;

  va_start(args, fmt);
  len = vsprintf(buffer, fmt, args);
  va_end(args);
    
  add_to_klog(buffer, len);
  
  //if (debugging) dbg_output(buffer);

  if (kprint_enabled) {
    char *msg = buffer;
    int msglen = len;

    if (msg[0] == '<' && msg[1] >= '0' && msg[1] <= '7' && msg[2] == '>') {
      msg += 3;
      msglen -= 3;
    }

    console_print(msg, msglen);
  }
}

static int klog_ioctl(struct dev *dev, int cmd, void *args, size_t size) {
  switch (cmd) {
    case IOCTL_GETDEVSIZE:
      return klog_size;

    case IOCTL_GETBLKSIZE:
      return 1;

    case IOCTL_KPRINT_ENABLED:
      if (!args || size != 4) return -EINVAL;
      kprint_enabled = *(int *) args;
      return 0;

    case IOCTL_KLOG_WAIT:
      return wait_for_klog();
  }

  return -ENOSYS;
}

static int klog_read(struct dev *dev, void *buffer, size_t count, blkno_t blkno, int flags) {
  char *ptr;
  unsigned int idx;
  unsigned int n;

  blkno = blkno % KLOG_SIZE;
  if (blkno > klog_size) return -EFAULT;
  if (blkno + count > klog_size) count = klog_size - blkno;
  if (count == 0) return 0;
  
  ptr = (char *) buffer;
  idx = (klog_start + blkno) % KLOG_SIZE;
  n = count;
  while (n-- > 0) {
    *ptr++ = klogbuf[idx++];
    if (idx == KLOG_SIZE) idx = 0;
  }

  return count;
}

static int klog_write(struct dev *dev, void *buffer, size_t count, blkno_t blkno, int flags) {
  add_to_klog(buffer, count);
  return count;
}

struct driver klog_driver = {
  "klog",
  DEV_TYPE_BLOCK,
  klog_ioctl,
  klog_read,
  klog_write
};

int __declspec(dllexport) klog(struct unit *unit, char *opts) {
  dev_make("klog", &klog_driver, NULL, NULL);
  return 0;
}
