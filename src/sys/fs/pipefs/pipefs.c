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
int pipefs_flush(struct file *filp);
int pipefs_read(struct file *filp, void *data, size_t size);
int pipefs_write(struct file *filp, void *data, size_t size);
loff_t pipefs_lseek(struct file *filp, loff_t offset, int origin);
int pipefs_fstat(struct file *filp, struct stat *buffer);

struct fsops pipefsops =
{
  FSOP_READ | FSOP_WRITE,

  NULL,
  NULL,

  NULL,
  pipefs_mount,
  NULL,

  NULL,

  NULL,
  pipefs_close,
  pipefs_flush,

  pipefs_read,
  pipefs_write,
  NULL,

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
  NULL
};

struct pipereq
{
  struct pipereq *next;
  struct pipe *pipe;
  struct thread *thread;
  char *buffer;
  size_t size;
  int rc;
};

struct pipe
{
  struct pipe *peer;
  struct pipereq *waithead;
  struct pipereq *waittail;
};

struct fs *pipefs;

void init_pipefs()
{
  register_filesystem("pipefs", &pipefsops);
  mount("pipefs", "", "", NULL, &pipefs);
}

int pipe(struct file **readpipe, struct file **writepipe)
{
  return -ENOSYS;
}

int pipefs_mount(struct fs *fs, char *opts)
{
  // Oly allow one instance of pipefs
  if (pipefs) return -EPERM;
  return 0;
}

int pipefs_close(struct file *filp)
{
  return -ENOSYS;
}

int pipefs_flush(struct file *filp)
{
  return 0;
}

int pipefs_read(struct file *filp, void *data, size_t size)
{
  struct pipe *pipe = (struct pipe *) filp->data;
  char *p;
  size_t left;
  size_t count;

  if (size == 0) return 0;
  if (!pipe->peer) return -EPIPE;

  p = (char *) data;
  left = size;
  count = 0;
  while (pipe->peer->waithead && left > 0)
  {
    struct pipereq *req = pipe->peer->waithead;
    size_t bytes = req->size;
    
    if (bytes > left) bytes = left;

    memcpy(p, req->buffer, bytes);
    req->buffer += bytes;
    req->size -= bytes;

    p += bytes;
    left -= bytes;
    count += bytes;

    if (req->size == 0)
    {
      pipe->peer->waithead = req->next;
      req->rc = 0;
      mark_thread_ready(req->thread);
    }
  }

  if (count == 0)
  {
    struct pipereq req;

    req.pipe = pipe;
    req.thread = self();
    req.buffer = p;
    req.size = left;
    req.rc = -EINTR;
    req.next = NULL;

    if (pipe->waittail) 
      pipe->waittail->next = &req;
    else
      pipe->waittail = &req;

    if (!pipe->waithead) pipe->waithead = &req;

    enter_wait(THREAD_WAIT_PIPE);
    if (req.rc < 0) return req.rc;
    count += req.rc;
  }

  return count;
}

int pipefs_write(struct file *filp, void *data, size_t size)
{
  struct pipe *pipe = (struct pipe *) filp->data;
  char *p;
  size_t left;

  if (size == 0) return 0;
  if (!pipe->peer) return -EPIPE;

  p = (char *) data;
  left = size;
  while (pipe->peer->waithead && left > 0)
  {
    struct pipereq *req = pipe->peer->waithead;
    size_t bytes = req->size;
    
    if (bytes > left) bytes = left;

    memcpy(req->buffer, p, bytes);
    req->rc = bytes;

    p += bytes;
    left -= bytes;

    pipe->peer->waithead = req->next;
    mark_thread_ready(req->thread);
  }

  if (left > 0)
  {
    struct pipereq req;

    req.pipe = pipe;
    req.thread = self();
    req.buffer = p;
    req.size = left;
    req.rc = -EINTR;
    req.next = NULL;

    if (pipe->waittail) 
      pipe->waittail->next = &req;
    else
      pipe->waittail = &req;

    if (!pipe->waithead) pipe->waithead = &req;

    enter_wait(THREAD_WAIT_PIPE);
    if (req.rc < 0) return req.rc;
  }

  return size;
}

loff_t pipefs_lseek(struct file *filp, loff_t offset, int origin)
{
  return -ESPIPE;
}

int pipefs_fstat(struct file *filp, struct stat *buffer)
{
  return -ENOSYS;
}
