//
// syscall.c
//
// System call interface
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

#define SYSCALL_PROFILE
//#define SYSCALL_LOGENTER
//#define SYSCALL_LOGEXIT
//#define SYSCALL_LOGONLYERRORS
//#define SYSCALL_LOGTIMEOUTS
//#define SYSCALL_LOGWAIT

#define SYSCALL_CHECKBUFFER

struct syscall_entry
{
  char *name;
  char *argfmt;
  int (*func)(char *); 
};

#ifdef SYSCALL_PROFILE
static unsigned long sccnt[SYSCALL_MAX  + 1];
static unsigned long scerr[SYSCALL_MAX  + 1];
#endif

static __inline int lock_buffer(void *buffer, int size)
{
#ifdef SYSCALL_CHECKBUFFER
  if (buffer)
  {
    if (buffer >= (void *) OSBASE) return -EFAULT;
    if ((char *) buffer + size >= (char *) OSBASE) return -EFAULT;
    if (!mem_mapped(buffer, size)) return -EFAULT;
  }
#endif

  return 0;
}

static __inline void unlock_buffer(void *buffer, int size)
{
}

static __inline int lock_string(char *s)
{
#ifdef SYSCALL_CHECKBUFFER
  if (s)
  {
    if (s >= (char *) OSBASE) return -EFAULT;
    if (!str_mapped(s)) return -EFAULT;
  }
#endif

  return 0;
}

static __inline void unlock_string(char *s)
{
}

static __inline int lock_iovec(struct iovec *iov, int count)
{
#ifdef SYSCALL_CHECKBUFFER
  int rc;

  rc = check_iovec(iov, count);
  if (rc < 0) return rc;
#endif

  return 0;
}

static __inline void unlock_iovec(struct iovec *iov, int count)
{
}

static __inline int lock_fdset(fd_set *fds)
{
#ifdef SYSCALL_CHECKBUFFER
  if (fds)
  {
    if (!mem_mapped(fds, sizeof(int))) return -EFAULT;
    if ((void *) fds >= (void *) OSBASE) return -EFAULT;
    if ((char *) fds + (fds->count + 1) * sizeof(int) >= (char *) OSBASE) return -EFAULT;
    if (!mem_mapped(fds, (fds->count + 1) * sizeof(int))) return -EFAULT;
  }
#endif

  return 0;
}

static __inline void unlock_fdset(fd_set *fds)
{
}

static int sys_null(char *params)
{
  return 0;
}

static int sys_format(char *params)
{
  char *devname; 
  char *type;
  char *opts;
  int rc;

  if (lock_buffer(params, 12) < 0) return -EFAULT;

  devname = *(char **) params;
  type = *(char **) (params + 4);
  opts = *(char **) (params + 8);

  if (lock_string(devname) < 0)
  {
    unlock_buffer(params, 12);
    return -EFAULT;
  }

  if (lock_string(type) < 0)
  {
    unlock_string(devname);
    unlock_buffer(params, 12);
    return -EFAULT;
  }

  if (lock_string(opts) < 0)
  {
    unlock_string(type);
    unlock_string(devname);
    unlock_buffer(params, 12);
    return -EFAULT;
  }

  rc = format(devname, type, opts);

  unlock_string(opts);
  unlock_string(type);
  unlock_string(devname);
  unlock_buffer(params, 12);

  return rc;
}

static int sys_mount(char *params)
{
  char *type;
  char *mntto;
  char *mntfrom; 
  char *opts;
  int rc;

  if (lock_buffer(params, 16) < 0) return -EFAULT;

  type = *(char **) params;
  mntto = *(char **) (params + 4);
  mntfrom = *(char **) (params + 8);
  opts = *(char **) (params + 12);

  if (lock_string(type) < 0)
  {
    unlock_buffer(params, 16);
    return -EFAULT;
  }

  if (lock_string(mntto) < 0)
  {
    unlock_string(type);
    unlock_buffer(params, 16);
    return -EFAULT;
  }

  if (lock_string(mntfrom) < 0)
  {
    unlock_string(mntto);
    unlock_string(type);
    unlock_buffer(params, 16);
    return -EFAULT;
  }

  if (lock_string(opts) < 0)
  {
    unlock_string(mntfrom);
    unlock_string(mntto);
    unlock_string(type);
    unlock_buffer(params, 16);
    return -EFAULT;
  }

  rc = mount(type, mntto, mntfrom, opts, NULL);

  unlock_string(opts);
  unlock_string(mntfrom);
  unlock_string(mntto);
  unlock_string(type);
  unlock_buffer(params, 16);

  return rc;
}

static int sys_umount(char *params)
{
  char *path;
  int rc;

  if (lock_buffer(params, 4) < 0) return -EFAULT;

  path = *(char **) params;

  if (lock_string(path) < 0)
  {
    unlock_buffer(params, 4);
    return -EFAULT;
  }

  rc = umount(path);

  unlock_string(path);
  unlock_buffer(params, 4);

  return rc;
}

static int sys_getfsstat(char *params)
{
  struct statfs *buf;
  size_t size;
  int rc;

  if (lock_buffer(params, 8) < 0) return -EFAULT;

  buf = *(struct statfs **) params;
  size = *(int *) (params + 4);

  if (lock_buffer(buf, size) < 0)
  {
    unlock_buffer(params, 8);
    return -EFAULT;
  }

  rc = getfsstat(buf, size);

  unlock_buffer(buf, size);
  unlock_buffer(params, 8);

  return rc;
}

static int sys_fstatfs(char *params)
{
  struct file *f;
  handle_t h;
  struct statfs *buf;
  int rc;

  if (lock_buffer(params, 8) < 0) return -EFAULT;

  h = *(handle_t *) params;
  buf = *(struct statfs **) (params + 4);

  f = (struct file *) olock(h, OBJECT_FILE);
  if (!f) 
  {
    unlock_buffer(params, 8);
    return -EBADF;
  }

  if (lock_buffer(buf, sizeof(struct statfs)) < 0)
  {
    orel(f);
    unlock_buffer(params, 8);
    return -EFAULT;
  }

  rc = fstatfs(f, buf);

  unlock_buffer(buf, sizeof(struct statfs));
  orel(f);
  unlock_buffer(params, 8);

  return rc;
}

static int sys_statfs(char *params)
{
  char *name;
  struct statfs *buf;
  int rc;

  if (lock_buffer(params, 8) < 0) return -EFAULT;

  name = *(char **) params;
  buf = *(struct statfs **) (params + 4);

  if (lock_string(name) < 0) 
  {
    unlock_buffer(params, 8);
    return -EFAULT;
  }

  if (lock_buffer(buf, sizeof(struct statfs)) < 0)
  {
    unlock_string(name);
    unlock_buffer(params, 8);
    return -EFAULT;
  }

  rc = statfs(name, buf);

  unlock_buffer(buf, sizeof(struct statfs));
  unlock_string(name);
  unlock_buffer(params, 8);

  return rc;
}

static int sys_open(char *params)
{
  struct file *f;
  char *name;
  int mode;
  int rc;

  if (lock_buffer(params, 8) < 0) return -EFAULT;

  name = *(char **) params;
  mode = *(int *) (params + 4);

  if (lock_string(name) < 0)
  {
    unlock_buffer(params, 8);
    return -EFAULT;
  }

  rc = open(name, mode, &f);
  if (rc == 0)
  {
    rc = halloc(&f->object);
    if (rc < 0) close(f);
  }

  unlock_string(name);
  unlock_buffer(params, 8);

  return rc;
}

static int sys_close(char *params)
{
  handle_t h;
  int rc;

  if (lock_buffer(params, 4) < 0) return -EFAULT;

  h = *(handle_t *) params;

  rc = hfree(h);

  unlock_buffer(params, 4);

  return rc;
}

static int sys_flush(char *params)
{
  struct file *f;
  handle_t h;
  int rc;

  if (lock_buffer(params, 4) < 0) return -EFAULT;

  h = *(handle_t *) params;
  f = (struct file *) olock(h, OBJECT_FILE);
  if (!f) 
  {
    unlock_buffer(params, 4);
    return -EBADF;
  }

  rc = flush(f);

  orel(f);
  unlock_buffer(params, 4);

  return rc;
}

static int sys_read(char *params)
{
  handle_t h;
  struct object *o;
  int rc;
  void *data;
  int size;

  if (lock_buffer(params, 12) < 0) return -EFAULT;

  h = *(handle_t *) params;
  data = *(void **) (params + 4);
  size = *(int *) (params + 8);

  o = olock(h, OBJECT_ANY);
  if (!o) 
  {
    unlock_buffer(params, 12);
    return -EBADF;
  }
  
  if (lock_buffer(data, size) < 0)
  {
    orel(o);
    unlock_buffer(params, 12);
    return -EFAULT;
  }

  if (o->type == OBJECT_FILE)
    rc = read((struct file *) o, data, size);
  else if (o->type == OBJECT_SOCKET)
    rc = recv((struct socket *) o, data, size, 0);
  else
    rc = -EBADF;

  orel(o);
  unlock_buffer(data, size);
  unlock_buffer(params, 12);

  return rc;
}

static int sys_write(char *params)
{
  handle_t h;
  struct object *o;
  int rc;
  void *data;
  int size;

  if (lock_buffer(params, 12) < 0) return -EFAULT;

  h = *(handle_t *) params;
  data = *(void **) (params + 4);
  size = *(int *) (params + 8);

  o = olock(h, OBJECT_ANY);
  if (!o) 
  {
    unlock_buffer(params, 12);
    return -EBADF;
  }

  if (lock_buffer(data, size) < 0)
  {
    orel(o);
    unlock_buffer(params, 12);
    return -EFAULT;
  }
  
  if (o->type == OBJECT_FILE)
    rc = write((struct file *) o, data, size);
  else if (o->type == OBJECT_SOCKET)
    rc = send((struct socket *) o, data, size, 0);
  else
    rc = -EBADF;
  
  orel(o);
  unlock_buffer(data, size);
  unlock_buffer(params, 12);

  return rc;
}

static int sys_ioctl(char *params)
{
  handle_t h;
  struct object *o;
  int rc;
  int cmd;
  void *data;
  int size;

  if (lock_buffer(params, 16) < 0) return -EFAULT;

  h = *(handle_t *) params;
  cmd = *(int *) (params + 4);
  data = *(void **) (params + 8);
  size = *(int *) (params + 12);

  o = olock(h, OBJECT_ANY);
  if (!o) 
  {
    unlock_buffer(params, 16);
    return -EBADF;
  }

  if (lock_buffer(data, size) < 0)
  {
    orel(o);
    unlock_buffer(params, 16);
    return -EFAULT;
  }
  
  if (o->type == OBJECT_FILE)
    rc = ioctl((struct file *) o, cmd, data, size);
  else if (o->type == OBJECT_SOCKET)
    rc = ioctlsocket((struct socket *) o, cmd, data, size);
  else
    rc = -EBADF;
  
  orel(o);
  unlock_buffer(params, 16);

  return rc;
}

static int sys_tell(char *params)
{
  struct file *f;
  handle_t h;
  loff_t retval;

  if (lock_buffer(params, 4) < 0) return -EFAULT;

  h = *(handle_t *) params;
  f = (struct file *) olock(h, OBJECT_FILE);
  if (!f) 
  {
    unlock_buffer(params, 4);
    return -EBADF;
  }

  retval = flush(f);

  orel(f);
  unlock_buffer(params, 4);

  return (int) retval;
}

static int sys_lseek(char *params)
{
  struct file *f;
  handle_t h;
  loff_t offset;
  int origin;
  loff_t retval;

  if (lock_buffer(params, 12) < 0) return -EFAULT;

  h = *(handle_t *) params;
  offset = *(loff_t *) (params + 4);
  origin = *(int *) (params + 8);

  f = (struct file *) olock(h, OBJECT_FILE);
  if (!f) 
  {
    unlock_buffer(params, 12);
    return -EBADF;
  }

  retval = lseek(f, offset, origin);

  orel(f);
  unlock_buffer(params, 12);

  return (int) retval;
}

static int sys_chsize(char *params)
{
  struct file *f;
  handle_t h;
  loff_t size;
  int rc;

  if (lock_buffer(params, 8) < 0) return -EFAULT;

  h = *(handle_t *) params;
  size = *(loff_t *) (params + 4);

  f = (struct file *) olock(h, OBJECT_FILE);
  if (!f) 
  {
    unlock_buffer(params, 8);
    return -EBADF;
  }

  rc = chsize(f, size);

  orel(f);
  unlock_buffer(params, 8);

  return rc;
}

static int sys_futime(char *params)
{
  struct file *f;
  handle_t h;
  struct utimbuf *times;
  int rc;

  if (lock_buffer(params, 8) < 0) return -EFAULT;

  h = *(handle_t *) params;
  times = *(struct utimbuf **) (params + 4);

  f = (struct file *) olock(h, OBJECT_FILE);
  if (!f) 
  {
    unlock_buffer(params, 8);
    return -EBADF;
  }

  if (lock_buffer(times, sizeof(struct utimbuf)) < 0)
  {
    orel(f);
    unlock_buffer(params, 8);
    return -EFAULT;
  }

  rc = futime(f, times);

  unlock_buffer(times, sizeof(struct utimbuf));
  orel(f);
  unlock_buffer(params, 8);

  return rc;
}

static int sys_utime(char *params)
{
  char *name;
  struct utimbuf *times;
  int rc;

  if (lock_buffer(params, 8) < 0) return -EFAULT;

  name = *(char **) params;
  times = *(struct utimbuf **) (params + 4);

  if (lock_string(name) < 0) 
  {
    unlock_buffer(params, 8);
    return -EFAULT;
  }

  if (lock_buffer(times, sizeof(struct utimbuf)) < 0)
  {
    unlock_string(name);
    unlock_buffer(params, 8);
    return -EFAULT;
  }

  rc = utime(name, times);

  unlock_buffer(times, sizeof(struct utimbuf));
  unlock_string(name);
  unlock_buffer(params, 8);

  return rc;
}

static int sys_fstat(char *params)
{
  struct file *f;
  handle_t h;
  struct stat *buffer;
  int rc;

  if (lock_buffer(params, 8) < 0) return -EFAULT;

  h = *(handle_t *) params;
  buffer = *(struct stat **) (params + 4);

  f = (struct file *) olock(h, OBJECT_FILE);
  if (!f) 
  {
    unlock_buffer(params, 8);
    return -EBADF;
  }

  if (lock_buffer(buffer, sizeof(struct stat)) < 0)
  {
    orel(f);
    unlock_buffer(params, 8);
    return -EFAULT;
  }

  rc = fstat(f, buffer);

  unlock_buffer(buffer, sizeof(struct stat));
  orel(f);
  unlock_buffer(params, 8);

  return rc;
}

static int sys_stat(char *params)
{
  char *name;
  struct stat *buffer;
  int rc;

  if (lock_buffer(params, 8) < 0) return -EFAULT;

  name = *(char **) params;
  buffer = *(struct stat **) (params + 4);

  if (lock_string(name) < 0) 
  {
    unlock_buffer(params, 8);
    return -EFAULT;
  }

  if (lock_buffer(buffer, sizeof(struct stat)) < 0)
  {
    unlock_string(name);
    unlock_buffer(params, 8);
    return -EFAULT;
  }

  rc = stat(name, buffer);

  unlock_buffer(buffer, sizeof(struct stat));
  unlock_string(name);
  unlock_buffer(params, 8);

  return rc;
}

static int sys_mkdir(char *params)
{
  char *name;
  int rc;

  if (lock_buffer(params, 4) < 0) return -EFAULT;

  name = *(char **) params;

  if (lock_string(name) < 0) 
  {
    unlock_buffer(params, 4);
    return -EFAULT;
  }

  rc = mkdir(name);

  unlock_string(name);
  unlock_buffer(params, 4);

  return rc;
}

static int sys_rmdir(char *params)
{
  char *name;
  int rc;

  if (lock_buffer(params, 4) < 0) return -EFAULT;

  name = *(char **) params;

  if (lock_string(name) < 0) 
  {
    unlock_buffer(params, 4);
    return -EFAULT;
  }

  rc = rmdir(name);

  unlock_string(name);
  unlock_buffer(params, 4);

  return rc;
}

static int sys_rename(char *params)
{
  char *oldname;
  char *newname;
  int rc;

  if (lock_buffer(params, 8) < 0) return -EFAULT;

  oldname = *(char **) params;
  newname = *(char **) (params + 4);

  if (lock_string(oldname) < 0) 
  {
    unlock_buffer(params, 8);
    return -EFAULT;
  }

  if (lock_string(newname) < 0) 
  {
    unlock_string(oldname);
    unlock_buffer(params, 8);
    return -EFAULT;
  }

  rc = rename(oldname, newname);

  unlock_string(newname);
  unlock_string(oldname);
  unlock_buffer(params, 8);

  return rc;
}

static int sys_link(char *params)
{
  char *oldname;
  char *newname;
  int rc;

  if (lock_buffer(params, 8) < 0) return -EFAULT;

  oldname = *(char **) params;
  newname = *(char **) (params + 4);

  if (lock_string(oldname) < 0) 
  {
    unlock_buffer(params, 8);
    return -EFAULT;
  }

  if (lock_string(newname) < 0) 
  {
    unlock_string(oldname);
    unlock_buffer(params, 8);
    return -EFAULT;
  }

  rc = link(oldname, newname);

  unlock_string(newname);
  unlock_string(oldname);
  unlock_buffer(params, 8);

  return rc;
}

static int sys_unlink(char *params)
{
  char *name;
  int rc;

  if (lock_buffer(params, 4) < 0) return -EFAULT;

  name = *(char **) params;

  if (lock_string(name) < 0) 
  {
    unlock_buffer(params, 4);
    return -EFAULT;
  }

  rc = unlink(name);

  unlock_string(name);
  unlock_buffer(params, 4);

  return rc;
}

static int sys_opendir(char *params)
{
  struct file *f;
  char *name;
  int rc;

  if (lock_buffer(params, 4) < 0) return -EFAULT;

  name = *(char **) params;

  if (lock_string(name) < 0)
  {
    unlock_buffer(params, 4);
    return -EFAULT;
  }

  rc = opendir(name, &f);
  if (rc == 0)
  {
    rc = halloc(&f->object);
    if (rc < 0) close(f);
  }

  unlock_string(name);
  unlock_buffer(params, 4);

  return rc;
}

static int sys_readdir(char *params)
{
  handle_t h;
  struct file *f;
  int rc;
  struct dirent *dirp;
  int count;

  if (lock_buffer(params, 12) < 0) return -EFAULT;

  h = *(handle_t *) params;
  dirp = *(struct dirent **) (params + 4);
  count = *(int *) (params + 8);

  f = (struct file *) olock(h, OBJECT_FILE);
  if (f == NULL) 
  {
    unlock_buffer(params, 12);
    return -EBADF;
  }

  if (lock_buffer(dirp, count * sizeof(struct dirent)) < 0)
  {
    orel(f);
    unlock_buffer(params, 12);
    return -EFAULT;
  }
  
  rc = readdir(f, dirp, count);
  
  orel(f);
  unlock_buffer(params, 12);

  return rc;
}

static int sys_mmap(char *params)
{
  char *addr;
  unsigned long size;
  int type;
  int protect;
  unsigned long tag;
  void *retval;

  if (lock_buffer(params, 20) < 0) return -EFAULT;

  addr = *(void **) params;
  size = *(unsigned long *) (params + 4);
  type = *(int *) (params + 8);
  protect = *(int *) (params + 12);
  tag = *(unsigned long *) (params + 16);

  retval = mmap(addr, size, type, protect, tag);

  unlock_buffer(params, 20);

  return (int) retval;
}

static int sys_munmap(char *params)
{
  char *addr;
  unsigned long size;
  int type;
  int rc;

  if (lock_buffer(params, 12) < 0) return -EFAULT;

  addr = *(void **) params;
  size = *(unsigned long *) (params + 4);
  type = *(int *) (params + 8);

  rc = munmap(addr, size, type);

  unlock_buffer(params, 12);

  return rc;
}

static int sys_mremap(char *params)
{
  char *addr;
  unsigned long oldsize;
  unsigned long newsize;
  int type;
  int protect;
  unsigned long tag;
  void *retval;

  if (lock_buffer(params, 24) < 0) return -EFAULT;

  addr = *(void **) params;
  oldsize = *(unsigned long *) (params + 4);
  newsize = *(unsigned long *) (params + 8);
  type = *(int *) (params + 12);
  protect = *(int *) (params + 16);
  tag = *(unsigned long *) (params + 20);

  retval = mremap(addr, oldsize, newsize, type, protect, tag);

  unlock_buffer(params, 24);

  return (int) retval;
}

static int sys_mprotect(char *params)
{
  char *addr;
  unsigned long size;
  int protect;
  int rc;

  if (lock_buffer(params, 12) < 0) return -EFAULT;

  addr = *(void **) params;
  size = *(unsigned long *) (params + 4);
  protect = *(int *) (params + 8);

  rc = mprotect(addr, size, protect);

  unlock_buffer(params, 12);

  return rc;
}

static int sys_mlock(char *params)
{
  char *addr;
  unsigned long size;
  int rc;

  if (lock_buffer(params, 8) < 0) return -EFAULT;

  addr = *(void **) params;
  size = *(unsigned long *) (params + 4);

  rc = mlock(addr, size);

  unlock_buffer(params, 8);

  return rc;
}

static int sys_munlock(char *params)
{
  char *addr;
  unsigned long size;
  int rc;

  if (lock_buffer(params, 8) < 0) return -EFAULT;

  addr = *(void **) params;
  size = *(unsigned long *) (params + 4);

  rc = munlock(addr, size);

  unlock_buffer(params, 8);

  return rc;
}

static int sys_wait(char *params)
{
  handle_t h;
  struct object *o;
  unsigned int timeout;
  int rc;

  if (lock_buffer(params, 8) < 0) return -EFAULT;

  h = *(handle_t *) params;
  timeout = *(unsigned int *) (params + 4);

  o = (struct object *) olock(h, OBJECT_ANY);
  if (!o) 
  {
    unlock_buffer(params, 8);
    return -EBADF;
  }

  rc = wait_for_object(o, timeout);

  orel(o);
  unlock_buffer(params, 8);
  return rc;
}

static int sys_waitall(char *params)
{
  handle_t *h;
  int count;
  unsigned int timeout;
  struct object *o[MAX_WAIT_OBJECTS];
  int rc;
  int n;

  if (lock_buffer(params, 12) < 0) return -EFAULT;

  h = *(handle_t **) params;
  count = *(int *) (params + 4);
  timeout = *(unsigned int *) (params + 8);

  if (count < 0 || count > MAX_WAIT_OBJECTS) return -EINVAL;

  if ((!h && count > 0) || lock_buffer(h, count * sizeof(handle_t *)) < 0)
  {
    unlock_buffer(params, 12);
    return -EFAULT;
  }

  for (n = 0; n < count; n++)
  {
    o[n] = olock(h[n], OBJECT_ANY);
    if (!o[n])
    {
      while (--n >= 0) orel(o[n]);
      unlock_buffer(params, 12);
      return -EBADF;
    }
  }

  rc = wait_for_all_objects(o, count, timeout);

  for (n = 0; n < count; n++) orel(o[n]);
  unlock_buffer(params, 12);
  return rc;
}

static int sys_waitany(char *params)
{
  handle_t *h;
  int count;
  unsigned int timeout;
  struct object *o[MAX_WAIT_OBJECTS];
  int rc;
  int n;

  if (lock_buffer(params, 12) < 0) return -EFAULT;

  h = *(handle_t **) params;
  count = *(int *) (params + 4);
  timeout = *(unsigned int *) (params + 8);

  if (count < 0 || count > MAX_WAIT_OBJECTS) return -EINVAL;

  if ((!h && count > 0) || lock_buffer(h, count * sizeof(handle_t *)) < 0)
  {
    unlock_buffer(params, 12);
    return -EFAULT;
  }

  for (n = 0; n < count; n++)
  {
    o[n] = olock(h[n], OBJECT_ANY);
    if (!o[n])
    {
      while (--n >= 0) orel(o[n]);
      unlock_buffer(params, 12);
      return -EBADF;
    }
  }

  rc = wait_for_any_object(o, count, timeout);

  for (n = 0; n < count; n++) orel(o[n]);
  unlock_buffer(params, 12);
  return rc;
}

static int sys_mkevent(char *params)
{
  int manual_reset;
  int initial_state;
  struct event *e;
  handle_t h;

  lock_buffer(params, 8);

  manual_reset = *(int *) params;
  initial_state = *(int *) (params + 4);

  e = (struct event *) kmalloc(sizeof(struct event));
  if (!e) 
  {
    unlock_buffer(params, 8);
    return -ENOMEM;
  }

  init_event(e, manual_reset, initial_state);

  h = halloc(&e->object);
  if (h < 0)
  {
    close_object(&e->object);
    unlock_buffer(params, 8);
    return h;
  }

  unlock_buffer(params, 8);

  return h;
}

static int sys_epulse(char *params)
{
  struct event *e;
  handle_t h;

  if (lock_buffer(params, 4) < 0) return -EFAULT;

  h = *(handle_t *) params;
  e = (struct event *) olock(h, OBJECT_EVENT);
  if (!e) 
  {
    unlock_buffer(params, 4);
    return -EBADF;
  }

  pulse_event(e);

  orel(e);
  unlock_buffer(params, 4);

  return 0;
}

static int sys_eset(char *params)
{
  struct event *e;
  handle_t h;

  if (lock_buffer(params, 4) < 0) return -EFAULT;

  h = *(handle_t *) params;
  e = (struct event *) olock(h, OBJECT_EVENT);
  if (!e) 
  {
    unlock_buffer(params, 4);
    return -EBADF;
  }

  set_event(e);

  orel(e);
  unlock_buffer(params, 4);

  return 0;
}

static int sys_ereset(char *params)
{
  struct event *e;
  handle_t h;

  if (lock_buffer(params, 4) < 0) return -EFAULT;

  h = *(handle_t *) params;
  e = (struct event *) olock(h, OBJECT_EVENT);
  if (!e) 
  {
    unlock_buffer(params, 4);
    return -EBADF;
  }

  reset_event(e);

  orel(e);
  unlock_buffer(params, 4);

  return 0;
}

static int sys_self(char *params)
{
  return self()->hndl;
}

static int sys_exit(char *params)
{
  int status;

  if (lock_buffer(params, 4) < 0) return -EFAULT;

  status = *(int *) params;

  exit(status);

  unlock_buffer(params, 4);
  return 0;
}

static int sys_dup(char *params)
{
  handle_t h;
  handle_t newh;
  struct object *o;

  if (lock_buffer(params, 4) < 0) return -EFAULT;

  h = *(handle_t *) params;

  o = olock(h, OBJECT_ANY);
  if (!o) 
  {
    unlock_buffer(params, 4);
    return -EBADF;
  }

  newh = halloc(o);

  orel(o);
  unlock_buffer(params, 4);
  return newh;
}

static int sys_mkthread(char *params)
{
  void *entrypoint;
  unsigned long stacksize;
  struct tib **ptib;
  struct thread *t;
  handle_t h;
  int rc;

  if (lock_buffer(params, 12) < 0) return -EFAULT;

  entrypoint = *(void **) params;
  stacksize = *(unsigned long *) (params + 4);
  ptib = *(struct tib ***) (params + 8);

  rc = create_user_thread(entrypoint, stacksize, &t);
  if (rc < 0)
  {
    unlock_buffer(params, 12);
    return rc;
  }

  h = halloc(&t->object);
  if (h < 0)
  {
    //destroy_thread(t);
    unlock_buffer(params, 12);
    return h;
  }

  if (ptib) *ptib = t->tib;

  unlock_buffer(params, 12);
  return h;
}

static int sys_suspend(char *params)
{
  handle_t h;
  struct thread *t;
  int rc;

  if (lock_buffer(params, 4) < 0) return -EFAULT;

  h = *(handle_t *) params;

  t = (struct thread *) olock(h, OBJECT_THREAD);
  if (!t || t->id == 0) 
  {
    unlock_buffer(params, 4);
    return -EBADF;
  }

  rc = suspend_thread(t);

  orel(t);
  unlock_buffer(params, 4);
  return rc;
}

static int sys_resume(char *params)
{
  handle_t h;
  struct thread *t;
  int rc;

  if (lock_buffer(params, 4) < 0) return -EFAULT;

  h = *(handle_t *) params;

  t = (struct thread *) olock(h, OBJECT_THREAD);
  if (!t) 
  {
    unlock_buffer(params, 4);
    return -EBADF;
  }

  rc = resume_thread(t);

  orel(t);
  unlock_buffer(params, 4);
  return rc;
}

static int sys_endthread(char *params)
{
  int exitcode;

  if (lock_buffer(params, 4) < 0) return -EFAULT;
  exitcode = *(int *) params;
  unlock_buffer(params, 4);

  terminate_thread(exitcode);

  return 0;
}

static int sys_setcontext(char *params)
{
  handle_t h;
  struct thread *t;
  struct context *context;
  int rc;

  if (lock_buffer(params, 8) < 0) return -EFAULT;

  h = *(handle_t *) params;
  context = *(struct context **) (params + 4);

  t = (struct thread *) olock(h, OBJECT_THREAD);
  if (!t) 
  {
    unlock_buffer(params, 8);
    return -EBADF;
  }

  if (lock_buffer(context, sizeof(struct context)) < 0)
  {
    orel(t);
    unlock_buffer(params, 8);
    return -EFAULT;
  }

  rc = set_context(t, context);

  unlock_buffer(context, sizeof(struct context));
  orel(t);
  unlock_buffer(params, 8);

  return rc;
}

static int sys_getcontext(char *params)
{
  handle_t h;
  struct thread *t;
  struct context *context;
  int rc;

  if (lock_buffer(params, 8) < 0) return -EFAULT;

  h = *(handle_t *) params;
  context = *(struct context **) (params + 4);

  t = (struct thread *) olock(h, OBJECT_THREAD);
  if (!t) 
  {
    unlock_buffer(params, 8);
    return -EBADF;
  }

  if (lock_buffer(context, sizeof(struct context)) < 0)
  {
    orel(t);
    unlock_buffer(params, 8);
    return -EFAULT;
  }

  rc = get_context(t, context);

  unlock_buffer(context, sizeof(struct context));
  orel(t);
  unlock_buffer(params, 8);

  return rc;
}

static int sys_getprio(char *params)
{
  handle_t h;
  struct thread *t;
  int priority;

  if (lock_buffer(params, 4) < 0) return -EFAULT;

  h = *(handle_t *) params;

  t = (struct thread *) olock(h, OBJECT_THREAD);
  if (!t) 
  {
    unlock_buffer(params, 4);
    return -EBADF;
  }

  priority = get_thread_priority(t);

  orel(t);
  unlock_buffer(params, 4);
  return priority;
}

static int sys_setprio(char *params)
{
  handle_t h;
  struct thread *t;
  int priority;
  int rc;

  if (lock_buffer(params, 8) < 0) return -EFAULT;

  h = *(handle_t *) params;
  priority = *(int *) (params + 4);

  t = (struct thread *) olock(h, OBJECT_THREAD);
  if (!t) 
  {
    unlock_buffer(params, 8);
    return -EBADF;
  }

  rc = set_thread_priority(t, priority);

  orel(t);
  unlock_buffer(params, 8);
  return rc;
}

static int sys_sleep(char *params)
{
  int millisecs;
  int rc;

  if (lock_buffer(params, 4) < 0) return -EFAULT;

  millisecs = *(int *) params;

  rc = sleep(millisecs);

  unlock_buffer(params, 4);
  return rc;
}

static int sys_time(char *params)
{
  time_t *timeptr;
  time_t t;

  if (lock_buffer(params, 4) < 0) return -EFAULT;

  timeptr = *(time_t **) params;

  if (lock_buffer(timeptr, sizeof(time_t *)) < 0)
  {
    unlock_buffer(params, 4);
    return -EFAULT;
  }

  t = get_time();
  if (timeptr) *timeptr = t;

  unlock_buffer(timeptr, sizeof(time_t *));
  unlock_buffer(params, 4);

  return t;
}

static int sys_gettimeofday(char *params)
{
  struct timeval *tv;

  if (lock_buffer(params, 4) < 0) return -EFAULT;

  tv = *(struct timeval **) params;

  if (!tv) return -EINVAL;
  if (lock_buffer(tv, sizeof(struct timeval)) < 0)
  {
    unlock_buffer(params, 4);
    return -EFAULT;
  }

  tv->tv_sec = systemclock.tv_sec;
  tv->tv_usec = systemclock.tv_usec;

  unlock_buffer(tv, sizeof(struct timeval));
  unlock_buffer(params, 4);
  
  return 0;
}

static int sys_settimeofday(char *params)
{
  struct timeval *tv;

  if (lock_buffer(params, 4) < 0) return -EFAULT;

  tv = *(struct timeval **) params;

  if (!tv) return -EINVAL;
  if (lock_buffer(tv, sizeof(struct timeval)) < 0)
  {
    unlock_buffer(params, 4);
    return -EFAULT;
  }

  set_time(tv);

  unlock_buffer(tv, sizeof(struct timeval));
  unlock_buffer(params, 4);

  return 0;
}

static int sys_clock(char *params)
{
  return clocks;
}

static int sys_mksem(char *params)
{
  int initial_count;
  struct sem *s;
  handle_t h;

  lock_buffer(params, 4);

  initial_count = *(int *) params;

  s = (struct sem *) kmalloc(sizeof(struct sem));
  if (!s) 
  {
    unlock_buffer(params, 4);
    return -ENOMEM;
  }

  init_sem(s, initial_count);

  h = halloc(&s->object);
  if (h < 0)
  {
    close_object(&s->object);
    unlock_buffer(params, 4);
    return h;
  }

  unlock_buffer(params, 4);

  return h;
}

static int sys_semrel(char *params)
{
  handle_t h;
  struct sem *s;
  int count;
  int rc;

  if (lock_buffer(params, 8) < 0) return -EFAULT;

  h = *(handle_t *) params;
  count = *(int *) (params + 4);

  s = (struct sem *) olock(h, OBJECT_SEMAPHORE);
  if (!s) 
  {
    unlock_buffer(params, 8);
    return -EBADF;
  }

  rc = release_sem(s, count);

  orel(s);
  unlock_buffer(params, 8);
  return rc;
}

static int sys_accept(char *params)
{
  handle_t h;
  struct socket *s;
  struct socket *news;
  int rc;
  struct sockaddr *addr;
  int *addrlen;

  if (lock_buffer(params, 12) < 0) return -EFAULT;

  h = *(handle_t *) params;
  addr = *(struct sockaddr **) (params + 4);
  addrlen = *(int **) (params + 8);

  s = (struct socket *) olock(h, OBJECT_SOCKET);
  if (!s) 
  {
    unlock_buffer(params, 12);
    return -EBADF;
  }

  if (lock_buffer(addr, sizeof(struct sockaddr)) < 0)
  {
    orel(s);
    unlock_buffer(params, 12);
    return -EFAULT;
  }
  
  if (lock_buffer(addrlen, sizeof(int)) < 0)
  {
    orel(s);
    unlock_buffer(addr, sizeof(struct sockaddr));
    unlock_buffer(params, 12);
    return -EFAULT;
  }

  rc = accept(s, addr, addrlen, &news);
  if (rc == 0)
  {
    rc = halloc(&news->iob.object);
    if (rc < 0) closesocket(news);
  }

  orel(s);
  unlock_buffer(addrlen, sizeof(int));
  unlock_buffer(addr, sizeof(struct sockaddr));
  unlock_buffer(params, 12);

  return rc;
}

static int sys_bind(char *params)
{
  handle_t h;
  struct socket *s;
  int rc;
  struct sockaddr *name;
  int namelen;

  if (lock_buffer(params, 12) < 0) return -EFAULT;

  h = *(handle_t *) params;
  name = *(struct sockaddr **) (params + 4);
  namelen = *(int *) (params + 8);

  s = (struct socket *) olock(h, OBJECT_SOCKET);
  if (!s) 
  {
    unlock_buffer(params, 12);
    return -EBADF;
  }

  if (lock_buffer(name, namelen) < 0)
  {
    orel(s);
    unlock_buffer(params, 12);
    return -EFAULT;
  }
  
  rc = bind(s, name, namelen);

  orel(s);
  unlock_buffer(name, namelen);
  unlock_buffer(params, 12);

  return rc;
}

static int sys_connect(char *params)
{
  handle_t h;
  struct socket *s;
  int rc;
  struct sockaddr *name;
  int namelen;

  if (lock_buffer(params, 12) < 0) return -EFAULT;

  h = *(handle_t *) params;
  name = *(struct sockaddr **) (params + 4);
  namelen = *(int *) (params + 8);

  s = (struct socket *) olock(h, OBJECT_SOCKET);
  if (!s) 
  {
    unlock_buffer(params, 12);
    return -EBADF;
  }

  if (lock_buffer(name, namelen) < 0)
  {
    orel(s);
    unlock_buffer(params, 12);
    return -EFAULT;
  }
  
  rc = connect(s, name, namelen);

  orel(s);
  unlock_buffer(name, namelen);
  unlock_buffer(params, 12);

  return rc;
}

static int sys_getpeername(char *params)
{
  handle_t h;
  struct socket *s;
  int rc;
  struct sockaddr *name;
  int *namelen;

  if (lock_buffer(params, 12) < 0) return -EFAULT;

  h = *(handle_t *) params;
  name = *(struct sockaddr **) (params + 4);
  namelen = *(int **) (params + 8);

  s = (struct socket *) olock(h, OBJECT_SOCKET);
  if (!s) 
  {
    unlock_buffer(params, 12);
    return -EBADF;
  }

  if (lock_buffer(name, sizeof(struct sockaddr)) < 0)
  {
    orel(s);
    unlock_buffer(params, 12);
    return -EFAULT;
  }
  
  if (lock_buffer(namelen, 4) < 0)
  {
    orel(s);
    unlock_buffer(name, sizeof(struct sockaddr));
    unlock_buffer(params, 12);
    return -EFAULT;
  }

  rc = getpeername(s, name, namelen);

  orel(s);
  unlock_buffer(namelen, 4);
  unlock_buffer(name, sizeof(struct sockaddr));
  unlock_buffer(params, 12);

  return rc;
}

static int sys_getsockname(char *params)
{
  handle_t h;
  struct socket *s;
  int rc;
  struct sockaddr *name;
  int *namelen;

  if (lock_buffer(params, 12) < 0) return -EFAULT;

  h = *(handle_t *) params;
  name = *(struct sockaddr **) (params + 4);
  namelen = *(int **) (params + 8);

  s = (struct socket *) olock(h, OBJECT_SOCKET);
  if (!s) 
  {
    unlock_buffer(params, 12);
    return -EBADF;
  }

  if (lock_buffer(name, sizeof(struct sockaddr)) < 0)
  {
    orel(s);
    unlock_buffer(params, 12);
    return -EFAULT;
  }
  
  if (lock_buffer(namelen, 4) < 0)
  {
    orel(s);
    unlock_buffer(name, sizeof(struct sockaddr));
    unlock_buffer(params, 12);
    return -EFAULT;
  }

  rc = getsockname(s, name, namelen);

  orel(s);
  unlock_buffer(namelen, 4);
  unlock_buffer(name, sizeof(struct sockaddr));
  unlock_buffer(params, 12);

  return rc;
}

static int sys_getsockopt(char *params)
{
  handle_t h;
  struct socket *s;
  int rc;
  int level;
  int optname;
  char *optval;
  int *optlen;
  int inoptlen;

  if (lock_buffer(params, 20) < 0) return -EFAULT;

  h = *(handle_t *) params;
  level = *(int *) (params + 4);
  optname = *(int *) (params + 8);
  optval = *(char **) (params + 12);
  optlen = *(int **) (params + 16);

  s = (struct socket *) olock(h, OBJECT_SOCKET);
  if (!s) 
  {
    unlock_buffer(params, 20);
    return -EBADF;
  }

  if (!optlen || lock_buffer(optlen, 4) < 0)
  {
    orel(s);
    unlock_buffer(params, 20);
    return -EFAULT;
  }
  inoptlen = *optlen;

  if (lock_buffer(optval, inoptlen) < 0)
  {
    orel(s);
    unlock_buffer(optlen, 4);
    unlock_buffer(params, 20);
    return -EFAULT;
  }
  
  rc = getsockopt(s, level, optname, optval, optlen);

  orel(s);
  unlock_buffer(optval, inoptlen);
  unlock_buffer(optlen, 4);
  unlock_buffer(params, 20);

  return rc;
}

static int sys_listen(char *params)
{
  handle_t h;
  struct socket *s;
  int rc;
  int backlog;

  if (lock_buffer(params, 8) < 0) return -EFAULT;

  h = *(handle_t *) params;
  backlog = *(int *) (params + 4);

  s = (struct socket *) olock(h, OBJECT_SOCKET);
  if (!s) 
  {
    unlock_buffer(params, 8);
    return -EBADF;
  }
  
  rc = listen(s, backlog);

  orel(s);
  unlock_buffer(params, 8);

  return rc;
}

static int sys_recv(char *params)
{
  handle_t h;
  struct socket *s;
  int rc;
  void *data;
  int size;
  unsigned int flags;

  if (lock_buffer(params, 16) < 0) return -EFAULT;

  h = *(handle_t *) params;
  data = *(void **) (params + 4);
  size = *(int *) (params + 8);
  flags = *(unsigned int *) (params + 12);

  s = (struct socket *) olock(h, OBJECT_SOCKET);
  if (!s) 
  {
    unlock_buffer(params, 16);
    return -EBADF;
  }

  if (lock_buffer(data, size) < 0)
  {
    orel(s);
    unlock_buffer(params, 16);
    return -EFAULT;
  }
  
  rc = recv(s, data, size, flags);

  orel(s);
  unlock_buffer(data, size);
  unlock_buffer(params, 16);

  return rc;
}

static int sys_recvfrom(char *params)
{
  handle_t h;
  struct socket *s;
  int rc;
  void *data;
  int size;
  unsigned int flags;
  struct sockaddr *from;
  int *fromlen;

  if (lock_buffer(params, 24) < 0) return -EFAULT;

  h = *(handle_t *) params;
  data = *(void **) (params + 4);
  size = *(int *) (params + 8);
  flags = *(unsigned int *) (params + 12);
  from = *(struct sockaddr **) (params + 16);
  fromlen = *(int **) (params + 20);

  s = (struct socket *) olock(h, OBJECT_SOCKET);
  if (!s) 
  {
    unlock_buffer(params, 24);
    return -EBADF;
  }

  if (lock_buffer(data, size) < 0)
  {
    orel(s);
    unlock_buffer(params, 24);
    return -EFAULT;
  }
  
  if (lock_buffer(from, sizeof(struct sockaddr)) < 0)
  {
    orel(s);
    unlock_buffer(data, size);
    unlock_buffer(params, 24);
    return -EFAULT;
  }

  if (lock_buffer(fromlen, sizeof(int)) < 0)
  {
    orel(s);
    unlock_buffer(from, sizeof(struct sockaddr));
    unlock_buffer(data, size);
    unlock_buffer(params, 24);
    return -EFAULT;
  }

  rc = recvfrom(s, data, size, flags, from, fromlen);

  orel(s);
  unlock_buffer(fromlen, sizeof(int));
  unlock_buffer(from, sizeof(struct sockaddr));
  unlock_buffer(data, size);
  unlock_buffer(params, 24);

  return rc;
}

static int sys_send(char *params)
{
  handle_t h;
  struct socket *s;
  int rc;
  void *data;
  int size;
  unsigned int flags;

  if (lock_buffer(params, 16) < 0) return -EFAULT;

  h = *(handle_t *) params;
  data = *(void **) (params + 4);
  size = *(int *) (params + 8);
  flags = *(unsigned int *) (params + 12);

  s = (struct socket *) olock(h, OBJECT_SOCKET);
  if (!s) 
  {
    unlock_buffer(params, 16);
    return -EBADF;
  }

  if (lock_buffer(data, size) < 0)
  {
    orel(s);
    unlock_buffer(params, 16);
    return -EFAULT;
  }
  
  rc = send(s, data, size, flags);

  orel(s);
  unlock_buffer(data, size);
  unlock_buffer(params, 16);

  return rc;
}

static int sys_sendto(char *params)
{
  handle_t h;
  struct socket *s;
  int rc;
  void *data;
  int size;
  unsigned int flags;
  struct sockaddr *to;
  int tolen;

  if (lock_buffer(params, 24) < 0) return -EFAULT;

  h = *(handle_t *) params;
  data = *(void **) (params + 4);
  size = *(int *) (params + 8);
  flags = *(unsigned int *) (params + 12);
  to = *(struct sockaddr **) (params + 16);
  tolen = *(int *) (params + 20);

  s = (struct socket *) olock(h, OBJECT_SOCKET);
  if (!s) 
  {
    unlock_buffer(params, 24);
    return -EBADF;
  }

  if (lock_buffer(data, size) < 0)
  {
    orel(s);
    unlock_buffer(params, 24);
    return -EFAULT;
  }
  
  if (lock_buffer(to, sizeof(struct sockaddr)) < 0)
  {
    orel(s);
    unlock_buffer(data, size);
    unlock_buffer(params, 24);
    return -EFAULT;
  }

  rc = sendto(s, data, size, flags, to, tolen);

  orel(s);
  unlock_buffer(to, sizeof(struct sockaddr));
  unlock_buffer(data, size);
  unlock_buffer(params, 24);

  return rc;
}

static int sys_setsockopt(char *params)
{
  handle_t h;
  struct socket *s;
  int rc;
  int level;
  int optname;
  char *optval;
  int optlen;

  if (lock_buffer(params, 20) < 0) return -EFAULT;

  h = *(handle_t *) params;
  level = *(int *) (params + 4);
  optname = *(int *) (params + 8);
  optval = *(char **) (params + 12);
  optlen = *(int *) (params + 16);

  s = (struct socket *) olock(h, OBJECT_SOCKET);
  if (!s) 
  {
    unlock_buffer(params, 20);
    return -EBADF;
  }

  if (lock_buffer(optval, optlen) < 0)
  {
    orel(s);
    unlock_buffer(params, 20);
    return -EFAULT;
  }
  
  rc = setsockopt(s, level, optname, optval, optlen);

  orel(s);
  unlock_buffer(optval, optlen);
  unlock_buffer(params, 20);

  return rc;
}

static int sys_shutdown(char *params)
{
  handle_t h;
  struct socket *s;
  int rc;
  int how;

  if (lock_buffer(params, 8) < 0) return -EFAULT;

  h = *(handle_t *) params;
  how = *(int *) (params + 4);

  s = (struct socket *) olock(h, OBJECT_SOCKET);
  if (!s) 
  {
    unlock_buffer(params, 8);
    return -EBADF;
  }
  
  rc = shutdown(s, how);

  orel(s);
  unlock_buffer(params, 8);

  return rc;
}

static int sys_socket(char *params)
{
  int rc;
  struct socket *s;
  int domain;
  int type;
  int protocol;

  if (lock_buffer(params, 12) < 0) return -EFAULT;

  domain = *(int *) params;
  type = *(int *) (params + 4);
  protocol = *(int *) (params + 8);

  rc = socket(domain, type, protocol, &s);
  if (rc == 0)
  {
    rc = halloc(&s->iob.object);
    if (rc < 0) closesocket(s);
  }

  unlock_buffer(params, 12);

  return rc;
}

static int sys_readv(char *params)
{
  handle_t h;
  struct object *o;
  int rc;
  struct iovec *iov;
  int count;

  if (lock_buffer(params, 12) < 0) return -EFAULT;

  h = *(handle_t *) params;
  iov = *(struct iovec **) (params + 4);
  count = *(int *) (params + 8);

  o = olock(h, OBJECT_ANY);
  if (!o) 
  {
    unlock_buffer(params, 12);
    return -EBADF;
  }
  
  if (lock_iovec(iov, count) < 0)
  {
    orel(o);
    unlock_buffer(params, 12);
    return -EFAULT;
  }

  if (o->type == OBJECT_FILE)
    rc = readv((struct file *) o, iov, count);
  else if (o->type == OBJECT_SOCKET)
    rc = recvv((struct socket *) o, iov, count);
  else
    rc = -EBADF;

  unlock_iovec(iov, count);
  orel(o);
  unlock_buffer(params, 12);

  return rc;
}

static int sys_writev(char *params)
{
  handle_t h;
  struct object *o;
  int rc;
  struct iovec *iov;
  int count;

  if (lock_buffer(params, 12) < 0) return -EFAULT;

  h = *(handle_t *) params;
  iov = *(struct iovec **) (params + 4);
  count = *(int *) (params + 8);

  o = olock(h, OBJECT_ANY);
  if (!o) 
  {
    unlock_buffer(params, 12);
    return -EBADF;
  }
  
  if (lock_iovec(iov, count) < 0)
  {
    orel(o);
    unlock_buffer(params, 12);
    return -EFAULT;
  }

  if (o->type == OBJECT_FILE)
    rc = writev((struct file *) o, iov, count);
  else if (o->type == OBJECT_SOCKET)
    rc = sendv((struct socket *) o, iov, count);
  else
    rc = -EBADF;

  lock_iovec(iov, count);
  orel(o);
  unlock_buffer(params, 12);

  return rc;
}

static int sys_chdir(char *params)
{
  char *name;
  int rc;

  if (lock_buffer(params, 4) < 0) return -EFAULT;

  name = *(char **) params;

  if (lock_string(name) < 0) 
  {
    unlock_buffer(params, 4);
    return -EFAULT;
  }

  rc = chdir(name);

  unlock_string(name);
  unlock_buffer(params, 4);

  return rc;
}

static int sys_mkiomux(char *params)
{
  int flags;
  struct iomux *iomux;
  handle_t h;

  lock_buffer(params, 4);

  flags = *(int *) params;

  iomux = (struct iomux *) kmalloc(sizeof(struct iomux));
  if (!iomux) 
  {
    unlock_buffer(params, 4);
    return -ENOMEM;
  }

  init_iomux(iomux, flags);

  h = halloc(&iomux->object);
  if (h < 0)
  {
    close_object(&iomux->object);
    unlock_buffer(params, 4);
    return h;
  }

  unlock_buffer(params, 4);

  return h;
}

static int sys_dispatch(char *params)
{
  handle_t ioh;
  handle_t h;
  struct iomux *iomux;
  struct object *o;
  int events;
  int context;
  int rc;

  if (lock_buffer(params, 16) < 0) return -EFAULT;

  ioh = *(handle_t *) params;
  h = *(handle_t *) (params + 4);
  events = *(int *) (params + 8);
  context = *(int *) (params + 12);

  iomux = (struct iomux *) olock(ioh, OBJECT_IOMUX);
  if (!iomux)
  {
    unlock_buffer(params, 16);
    return -EBADF;
  }

  o = (struct object *) olock(h, OBJECT_ANY);
  if (!o) 
  {
    orel(iomux);
    unlock_buffer(params, 16);
    return -EBADF;
  }

  rc = queue_ioobject(iomux, o, events, context);

  orel(o);
  orel(iomux);
  unlock_buffer(params, 16);
  return rc;
}

static int sys_recvmsg(char *params)
{
  handle_t h;
  struct msghdr *msg;
  unsigned int flags;
  struct socket *s;
  int rc;

  if (lock_buffer(params, 12) < 0) return -EFAULT;

  h = *(handle_t *) params;
  msg = *(struct msghdr **) (params + 4);
  flags = *(unsigned int *) (params + 8);

  s = (struct socket *) olock(h, OBJECT_SOCKET);
  if (!s) 
  {
    unlock_buffer(params, 12);
    return -EBADF;
  }

  if (lock_buffer(msg, sizeof(struct msghdr)) < 0)
  {
    orel(s);
    unlock_buffer(params, 12);
    return -EFAULT;
  }
  
  if (lock_iovec(msg->iov, msg->iovlen) < 0)
  {
    unlock_buffer(msg, sizeof(struct msghdr));
    orel(s);
    unlock_buffer(params, 12);
    return -EFAULT;
  }

  if (lock_buffer(msg->name, msg->namelen) < 0)
  {
    unlock_iovec(msg->iov, msg->iovlen);
    unlock_buffer(msg, sizeof(struct msghdr));
    orel(s);
    unlock_buffer(params, 12);
    return -EFAULT;
  }

  rc = recvmsg(s, msg, flags);

  unlock_iovec(msg->iov, msg->iovlen);
  unlock_buffer(msg, sizeof(struct msghdr));
  orel(s);
  unlock_buffer(params, 12);

  return rc;
}

static int sys_sendmsg(char *params)
{
  handle_t h;
  struct msghdr *msg;
  unsigned int flags;
  struct socket *s;
  int rc;

  if (lock_buffer(params, 12) < 0) return -EFAULT;

  h = *(handle_t *) params;
  msg = *(struct msghdr **) (params + 4);
  flags = *(unsigned int *) (params + 8);

  s = (struct socket *) olock(h, OBJECT_SOCKET);
  if (!s) 
  {
    unlock_buffer(params, 12);
    return -EBADF;
  }

  if (lock_buffer(msg, sizeof(struct msghdr)) < 0)
  {
    orel(s);
    unlock_buffer(params, 12);
    return -EFAULT;
  }
  
  if (lock_iovec(msg->iov, msg->iovlen) < 0)
  {
    unlock_buffer(msg, sizeof(struct msghdr));
    orel(s);
    unlock_buffer(params, 12);
    return -EFAULT;
  }

  if (lock_buffer(msg->name, msg->namelen) < 0)
  {
    unlock_iovec(msg->iov, msg->iovlen);
    unlock_buffer(msg, sizeof(struct msghdr));
    orel(s);
    unlock_buffer(params, 12);
    return -EFAULT;
  }

  rc = sendmsg(s, msg, flags);

  unlock_iovec(msg->iov, msg->iovlen);
  unlock_buffer(msg, sizeof(struct msghdr));
  orel(s);
  unlock_buffer(params, 12);

  return rc;
}

static int sys_select(char *params)
{
  int nfds;
  fd_set *readfds;
  fd_set *writefds;
  fd_set *exceptfds;
  struct timeval *timeout;
  int rc;

  if (lock_buffer(params, 20) < 0) return -EFAULT;

  nfds = *(int *) params;
  readfds = *(fd_set **) (params + 4);
  writefds = *(fd_set **) (params + 8);
  exceptfds = *(fd_set **) (params + 12);
  timeout = *(struct timeval **) (params + 16);

  if (lock_fdset(readfds) < 0)
  {
    unlock_buffer(params, 20);
    return -EFAULT;
  }

  if (lock_fdset(writefds) < 0)
  {
    unlock_fdset(readfds);
    unlock_buffer(params, 20);
    return -EFAULT;
  }

  if (lock_fdset(exceptfds) < 0)
  {
    unlock_fdset(writefds);
    unlock_fdset(readfds);
    unlock_buffer(params, 20);
    return -EFAULT;
  }

  if (lock_buffer(timeout, sizeof(struct timeval)) < 0)
  {
    unlock_fdset(exceptfds);
    unlock_fdset(writefds);
    unlock_fdset(readfds);
    unlock_buffer(params, 20);
    return -EFAULT;
  }

  rc = select(nfds, readfds, writefds, exceptfds, timeout);

  unlock_buffer(timeout, sizeof(struct timeval));
  unlock_fdset(exceptfds);
  unlock_fdset(writefds);
  unlock_fdset(readfds);
  unlock_buffer(params, 20);

  return rc;
}

static int sys_pipe(char *params)
{
  handle_t *fildes;
  int rc;
  struct file *readpipe;
  struct file *writepipe;

  if (lock_buffer(params, 4) < 0) return -EFAULT;

  fildes = *(handle_t **) params;

  if (lock_buffer(fildes, sizeof(handle_t) * 2) < 0)
  {
    unlock_buffer(params, 4);
    return -EFAULT;
  }

  rc = pipe(&readpipe, &writepipe);

  if (rc == 0)
  {
    fildes[0] = halloc(&readpipe->object);
    fildes[1] = halloc(&writepipe->object);

    if (fildes[0] < 0 || fildes[1] < 0)
    {
      if (fildes[0] >= 0) hfree(fildes[0]);
      if (fildes[1] >= 0) hfree(fildes[1]);

      close(readpipe);
      close(writepipe);
      rc = -ENFILE;
    }
  }
  
  unlock_buffer(fildes, sizeof(handle_t) * 2);
  unlock_buffer(params, 4);

  return rc;
}

struct syscall_entry syscalltab[] =
{
  {"null","", sys_null},
  {"format", "'%s','%s','%s'", sys_format},
  {"mount", "'%s','%s','%s','%s'", sys_mount},
  {"umount", "'%s'", sys_umount},
  {"open", "'%s',%x", sys_open},
  {"close", "%d", sys_close},
  {"flush", "%d", sys_flush},
  {"read", "%d,%p,%d", sys_read},
  {"write", "%d,%p,%d", sys_write},
  {"tell", "%d", sys_tell},
  {"lseek", "%d,%d,%d", sys_lseek},
  {"chsize", "%d,%d", sys_chsize},
  {"fstat", "%d,%p", sys_fstat},
  {"stat", "'%s',%p", sys_stat},
  {"mkdir", "'%s'", sys_mkdir},
  {"rmdir", "'%s'", sys_rmdir},
  {"rename", "'%s','%s'", sys_rename},
  {"link", "'%s','%s'", sys_link},
  {"unlink", "'%s'", sys_unlink},
  {"opendir", "'%s'", sys_opendir},
  {"readdir", "%d,%p,%d", sys_readdir},
  {"mmap", "%p,%d,%x,%x", sys_mmap},
  {"munmap", "%p,%d,%x", sys_munmap},
  {"mremap", "%p,%d,%d,%x,%x", sys_mremap},
  {"mprotect", "%p,%d,%x", sys_mprotect},
  {"mlock", "%p,%d", sys_mlock},
  {"munlock", "%p,%d", sys_munlock},
  {"wait", "%d,%d", sys_wait},
  {"mkevent", "%d,%d", sys_mkevent},
  {"epulse", "%d", sys_epulse},
  {"eset", "%d", sys_eset},
  {"ereset", "%d", sys_ereset},
  {"self", "", sys_self},
  {"exit", "%d", sys_exit},
  {"dup", "%d", sys_dup},
  {"mkthread", "%p,%d,%p", sys_mkthread},
  {"suspend", "%d", sys_suspend},
  {"resume", "%d", sys_resume},
  {"endthread", "%d", sys_endthread},
  {"setcontext", "%d,%p", sys_setcontext},
  {"getcontext", "%d,%p", sys_getcontext},
  {"getprio", "%d", sys_getprio},
  {"setprio", "%d,%d", sys_setprio},
  {"sleep", "%d", sys_sleep},
  {"time", "", sys_time},
  {"gettimeofday", "%p", sys_gettimeofday},
  {"clock", "", sys_clock},
  {"mksem", "%d", sys_mksem},
  {"semrel", "%p,%d", sys_semrel},
  {"ioctl", "%d,%d,%p,%d", sys_ioctl},
  {"getfsstat", "%p,%d", sys_getfsstat},
  {"fstatfs", "%d,%p", sys_fstatfs},
  {"statfs", "'%s',%p", sys_statfs},
  {"futime", "%d,%p", sys_futime},
  {"utime", "'%s',%p", sys_utime},
  {"settimeofday", "%p", sys_settimeofday},
  {"accept", "%d,%p,%p", sys_accept},
  {"bind", "%d,%p,%d", sys_bind},
  {"connect", "%d,%p,%d", sys_connect},
  {"getpeername", "%d,%p,%p", sys_getpeername},
  {"getsockname", "%d,%p,%p", sys_getsockname},
  {"getsockopt", "%d,%d,%d,%p,%p", sys_getsockopt},
  {"listen", "%d,%d", sys_listen},
  {"recv", "%d,%p,%d,%d", sys_recv},
  {"recvfrom", "%d,%p,%d,%d,%p,%p", sys_recvfrom},
  {"send", "%d,%p,%d,%d", sys_send},
  {"sendto", "%d,%p,%d,%d,%p,%d", sys_sendto},
  {"setsockopt", "%d,%d,%d,%p,%d", sys_setsockopt},
  {"shutdown", "%d,%d", sys_shutdown},
  {"socket", "%d,%d,%d", sys_socket},
  {"waitall", "%p,%d,%d", sys_waitall},
  {"waitany", "%p,%d,%d", sys_waitany},
  {"readv", "%d,%p,%d", sys_readv},
  {"writev", "%d,%p,%d", sys_writev},
  {"chdir", "'%s'", sys_chdir},
  {"mkiomux", "%d", sys_mkiomux},
  {"dispatch", "%d,%d,%d,%d", sys_dispatch},
  {"recvmsg", "%d,%p,%d", sys_recvmsg},
  {"sendmsg", "%d,%p,%d", sys_sendmsg},
  {"select", "%d,%p,%p,%p,%p", sys_select},
  {"pipe", "%p", sys_pipe},
};

int syscall(int syscallno, char *params)
{
  int rc;
  struct thread *t = self();

  struct syscall_context *sysctxt = (struct syscall_context *) &syscallno;

  t->uctxt = (char *) sysctxt + offsetof(struct syscall_context, traptype);
  if (syscallno < 0 || syscallno > SYSCALL_MAX) return -ENOSYS;

#ifdef SYSCALL_LOGENTER
#ifndef SYSCALL_LOGWAIT
  if (syscallno != SYSCALL_WAIT && syscallno != SYSCALL_WAITALL && syscallno != SYSCALL_WAITANY)
#endif
  {

    char buf[1024];

    vsprintf(buf, syscalltab[syscallno].argfmt, params);
    kprintf("<-%s(%s)\n", syscalltab[syscallno].name, buf);
  }
#endif

#ifdef SYSCALL_PROFILE
  sccnt[syscallno]++;
#endif

  rc = syscalltab[syscallno].func(params);

  if (rc < 0)
  {
    struct tib *tib = self()->tib;
    if (tib) tib->err = rc;
  }

#ifdef SYSCALL_PROFILE
  if (rc < 0) scerr[syscallno]++;
#endif

#if defined(SYSCALL_LOGEXIT) || defined(SYSCALL_LOGONLYERRORS)

#ifdef SYSCALL_LOGONLYERRORS

#ifdef SYSCALL_LOGTIMEOUTS
  if (rc < 0)
#else
  if (rc < 0 && rc != -ETIMEOUT)
#endif

#else

#ifndef SYSCALL_LOGWAIT
  if (syscallno != SYSCALL_WAIT && syscallno != SYSCALL_WAITALL && syscallno != SYSCALL_WAITANY)
#endif

#endif
  {
    char buf[1024];

    vsprintf(buf, syscalltab[syscallno].argfmt, params);
    kprintf("%s(%s) -> %d\n", syscalltab[syscallno].name, buf, rc);
  }
#endif

  check_dpc_queue();
  check_preempt();

  t->uctxt = NULL;

  return rc;
}

#ifdef SYSCALL_PROFILE
static int syscalls_proc(struct proc_file *pf, void *arg)
{
  int i = 0;

  pprintf(pf, "syscall               calls     errors\n");
  pprintf(pf, "---------------- ---------- ----------\n");
  for (i = 0; i < SYSCALL_MAX  + 1; i++)
  {
    if (sccnt[i] != 0) 
    {
      pprintf(pf, "%-16s %10d %10d\n", syscalltab[i].name, sccnt[i], scerr[i]);
    }
  }

  return 0;
}
#endif

void init_syscall()
{
#ifdef SYSCALL_PROFILE
  register_proc_inode("syscalls", syscalls_proc, NULL);
#endif
}
