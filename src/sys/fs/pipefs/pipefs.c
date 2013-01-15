//
// pipefs.c
//
// Pipe Filesystem
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

int pipefs_mount(struct fs *fs, char *opts);
int pipefs_close(struct file *filp);
int pipefs_fsync(struct file *filp);
int pipefs_read(struct file *filp, void *data, size_t size, off64_t pos);
int pipefs_write(struct file *filp, void *data, size_t size, off64_t pos);
int pipefs_ioctl(struct file *filp, int cmd, void *data, size_t size);
off64_t pipefs_lseek(struct file *filp, off64_t offset, int origin);
int pipefs_fstat(struct file *filp, struct stat64 *buffer);

struct fsops pipefsops = {
  FSOP_READ | FSOP_WRITE,

  NULL,
  NULL,

  NULL,
  pipefs_mount,
  NULL,

  NULL,

  NULL,
  pipefs_close,
  NULL,
  pipefs_fsync,

  pipefs_read,
  pipefs_write,
  pipefs_ioctl,

  NULL,
  pipefs_lseek,
  NULL,

  NULL,
  NULL,

  pipefs_fstat,
  NULL,

  NULL,

  NULL,
  NULL,

  NULL,
  NULL,

  NULL,
  NULL,
  NULL,

  NULL,
  NULL
};

struct pipereq {
  struct pipereq *next;
  struct pipe *pipe;
  struct thread *thread;
  char *buffer;
  size_t size;
  int rc;
};

struct pipe {
  struct file *filp;
  struct pipe *peer;
  struct pipereq *waithead;
  struct pipereq *waittail;
};

struct fs *pipefs;

static void release_all_waiters(struct pipe *pipe, int retcode) {
  struct pipereq *req = pipe->waithead;
  struct pipereq *next;

  while (req) {
    next = req->next;
    req->rc = retcode;
    mark_thread_ready(req->thread, 1, 2);
    req = next;
  }

  pipe->waithead = pipe->waittail = NULL;
}

static void cancel_request(struct pipe *pipe, struct pipereq *req) {
  if (pipe->waithead == req) {
    pipe->waithead = req->next;
    if (pipe->waittail == req) pipe->waittail = NULL;
  } else if (pipe->waithead) {
    struct pipereq *r = pipe->waithead;
    while (r->next) {
      if (r->next == req) {
        r->next = req->next;
        if (pipe->waittail == req) pipe->waittail = r;
        return;
      }
      r = r->next;
    }
  }
}

void init_pipefs() {
  register_filesystem("pipefs", &pipefsops);
  mount("pipefs", "", "", NULL, &pipefs);
}

int pipe(struct file **readpipe, struct file **writepipe) {
  struct file *rd;
  struct file *wr;
  struct pipe *rdp;
  struct pipe *wrp;

  if (!pipefs) return -ENOENT;

  rd = newfile(pipefs, NULL, O_RDONLY, S_IREAD);
  wr = newfile(pipefs, NULL, O_WRONLY, S_IWRITE);
  rdp = kmalloc(sizeof(struct pipe));
  wrp = kmalloc(sizeof(struct pipe));
  if (!rd || !wr || !rdp || !wrp) {
    kfree(rd);
    kfree(wr);
    kfree(rdp);
    kfree(wrp);
    return -EMFILE;
  }

  rd->data = rdp;
  rdp->filp = rd;
  rdp->peer = wrp;
  rdp->waithead = rdp->waittail = NULL;

  wr->data = wrp;
  wrp->filp = wr;
  wrp->peer = rdp;
  wrp->waithead = wrp->waittail = NULL;

  *readpipe = rd;
  *writepipe = wr;

  return 0;
}

int pipefs_mount(struct fs *fs, char *opts) {
  // Only allow one instance of pipefs
  if (pipefs) return -EPERM;
  return 0;
}

int pipefs_close(struct file *filp) {
  struct pipe *pipe = (struct pipe *) filp->data;

  set_io_event(&filp->iob, IOEVT_CLOSE);
  release_all_waiters(pipe, -EINTR);

  if (pipe->peer) {
    if (filp->flags & O_WRONLY) {
      set_io_event(&pipe->peer->filp->iob, IOEVT_READ);
      release_all_waiters(pipe->peer, 0);
    } else {
      set_io_event(&pipe->peer->filp->iob, IOEVT_WRITE);
      release_all_waiters(pipe->peer, -EPIPE);
    }
    pipe->peer->peer = NULL;
    pipe->peer = NULL;
  }

  kfree(pipe);

  return 0;
}

int pipefs_fsync(struct file *filp) {
  return 0;
}

int pipefs_read(struct file *filp, void *data, size_t size, off64_t pos) {
  struct pipe *pipe = (struct pipe *) filp->data;
  char *p;
  size_t left;
  size_t count;

  if (size == 0) return 0;
  if (!pipe->peer) return 0;

  p = (char *) data;
  left = size;
  count = 0;
  while (pipe->peer->waithead && left > 0) {
    struct pipereq *req = pipe->peer->waithead;
    size_t bytes = req->size;
    
    if (bytes > left) bytes = left;

    memcpy(p, req->buffer, bytes);
    req->buffer += bytes;
    req->size -= bytes;

    p += bytes;
    left -= bytes;
    count += bytes;

    if (req->size == 0) {
      pipe->peer->waithead = req->next;
      if (pipe->peer->waithead == NULL) pipe->peer->waittail = NULL;
      req->rc = 0;
      mark_thread_ready(req->thread, 1, 2);
    }
  }

  if (pipe->peer->waithead == NULL) clear_io_event(&filp->iob, IOEVT_READ);

  if (count == 0) {
    struct pipereq req;
    int rc;

    set_io_event(&pipe->peer->filp->iob, IOEVT_WRITE);
    if (filp->flags & O_NONBLOCK) return -EAGAIN;

    req.pipe = pipe;
    req.thread = self();
    req.buffer = p;
    req.size = left;
    req.rc = -EINTR;
    req.next = NULL;

    if (pipe->waittail) { 
      pipe->waittail->next = &req;
    } else {
      pipe->waittail = &req;
    }

    if (!pipe->waithead) pipe->waithead = &req;

    rc = enter_alertable_wait(THREAD_WAIT_PIPE);
    if (rc < 0) {
      cancel_request(pipe, &req);
      req.rc = rc;
    }

    if (req.rc < 0) return req.rc;
    count += req.rc;
  }

  return count;
}

int pipefs_write(struct file *filp, void *data, size_t size, off64_t pos) {
  struct pipe *pipe = (struct pipe *) filp->data;
  char *p;
  size_t left;

  if (size == 0) return 0;
  if (!pipe->peer) return -EPIPE;

  p = (char *) data;
  left = size;
  while (pipe->peer->waithead && left > 0) {
    struct pipereq *req = pipe->peer->waithead;
    size_t bytes = req->size;
    
    if (bytes > left) bytes = left;

    memcpy(req->buffer, p, bytes);
    req->rc = bytes;

    p += bytes;
    left -= bytes;

    pipe->peer->waithead = req->next;
    if (pipe->peer->waithead == NULL) pipe->peer->waittail = NULL;
    mark_thread_ready(req->thread, 1, 2);
  }

  if (pipe->peer->waithead == NULL) clear_io_event(&filp->iob, IOEVT_WRITE);

  if (left > 0) {
    struct pipereq req;
    int rc;

    set_io_event(&pipe->peer->filp->iob, IOEVT_READ);
    if (filp->flags & O_NONBLOCK) return size - left;

    req.pipe = pipe;
    req.thread = self();
    req.buffer = p;
    req.size = left;
    req.rc = -EINTR;
    req.next = NULL;

    if (pipe->waittail) { 
      pipe->waittail->next = &req;
    } else {
      pipe->waittail = &req;
    }

    if (!pipe->waithead) pipe->waithead = &req;

    rc = enter_alertable_wait(THREAD_WAIT_PIPE);
    if (rc < 0) {
      cancel_request(pipe, &req);
      req.rc = rc;
    }

    if (req.rc < 0) return req.rc;
  }

  return size;
}

int pipefs_ioctl(struct file *filp, int cmd, void *data, size_t size) {
  if (cmd == FIONBIO) return 0;
  return -ENOSYS;
}

off64_t pipefs_lseek(struct file *filp, off64_t offset, int origin) {
  return -ESPIPE;
}

int pipefs_fstat(struct file *filp, struct stat64 *buffer) {
  // TODO: implement timestamps on create, read and write
  return -ENOSYS;
}
