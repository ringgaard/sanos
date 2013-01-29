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

struct syscall_entry {
  char *name;
  int paramsize;
  char *argfmt;
  int (*func)(char *); 
};

#ifdef SYSCALL_PROFILE
static unsigned long sccnt[SYSCALL_MAX  + 1];
static unsigned long scerr[SYSCALL_MAX  + 1];
#endif

static __inline int lock_buffer(void *buffer, int size, int modify) {
#ifdef SYSCALL_CHECKBUFFER
  if (buffer) {
    if (size < 0) return -EINVAL;
    if (!mem_access(buffer, size, modify ? PT_USER_WRITE : PT_USER_READ)) return -EFAULT;
  }
#endif

  return 0;
}

static __inline void unlock_buffer(void *buffer, int size) {
}

static __inline int lock_string(char *s) {
#ifdef SYSCALL_CHECKBUFFER
  if (s) {
    if (!str_access(s, PT_USER_READ)) return -EFAULT;
  }
#endif

  return 0;
}

static __inline void unlock_string(char *s) {
}

static __inline int lock_iovec(struct iovec *iov, int count, int modify) {
#ifdef SYSCALL_CHECKBUFFER
  int rc;

  rc = check_iovec(iov, count, modify);
  if (rc < 0) return rc;
#endif

  return 0;
}

static __inline void unlock_iovec(struct iovec *iov, int count) {
}

static __inline int lock_fdset(fd_set *fds, int modify) {
#ifdef SYSCALL_CHECKBUFFER
  if (fds) {
    if (!mem_access(fds, sizeof(int), modify ? PT_USER_WRITE : PT_USER_READ)) return -EFAULT;
    if ((void *) fds >= (void *) OSBASE) return -EFAULT;
    if ((char *) fds + (fds->count + 1) * sizeof(int) >= (char *) OSBASE) return -EFAULT;
    if (!mem_access(fds, (fds->count + 1) * sizeof(int), modify ? PT_USER_WRITE : PT_USER_READ)) return -EFAULT;
  }
#endif

  return 0;
}

static __inline void unlock_fdset(fd_set *fds) {
}

static int sys_null(char *params) {
  return 0;
}

static int sys_mkfs(char *params) {
  char *devname; 
  char *type;
  char *opts;
  int rc;

  devname = *(char **) params;
  type = *(char **) (params + 4);
  opts = *(char **) (params + 8);

  if (lock_string(devname) < 0) return -EFAULT;

  if (lock_string(type) < 0) {
    unlock_string(devname);
    return -EFAULT;
  }

  if (lock_string(opts) < 0) {
    unlock_string(type);
    unlock_string(devname);
    return -EFAULT;
  }

  rc = mkfs(devname, type, opts);

  unlock_string(opts);
  unlock_string(type);
  unlock_string(devname);

  return rc;
}

static int sys_mount(char *params) {
  char *type;
  char *mntto;
  char *mntfrom; 
  char *opts;
  int rc;

  type = *(char **) params;
  mntto = *(char **) (params + 4);
  mntfrom = *(char **) (params + 8);
  opts = *(char **) (params + 12);

  if (lock_string(type) < 0) return -EFAULT;

  if (lock_string(mntto) < 0) {
    unlock_string(type);
    return -EFAULT;
  }

  if (lock_string(mntfrom) < 0) {
    unlock_string(mntto);
    unlock_string(type);
    return -EFAULT;
  }

  if (lock_string(opts) < 0) {
    unlock_string(mntfrom);
    unlock_string(mntto);
    unlock_string(type);
    return -EFAULT;
  }

  rc = mount(type, mntto, mntfrom, opts, NULL);

  unlock_string(opts);
  unlock_string(mntfrom);
  unlock_string(mntto);
  unlock_string(type);

  return rc;
}

static int sys_umount(char *params) {
  char *path;
  int rc;

  path = *(char **) params;

  if (lock_string(path) < 0) return -EFAULT;

  rc = umount(path);

  unlock_string(path);

  return rc;
}

static int sys_getfsstat(char *params) {
  struct statfs *buf;
  size_t size;
  int rc;

  buf = *(struct statfs **) params;
  size = *(int *) (params + 4);

  if (lock_buffer(buf, size, 1) < 0) return -EFAULT;

  rc = getfsstat(buf, size);

  unlock_buffer(buf, size);

  return rc;
}

static int sys_fstatfs(char *params) {
  struct file *f;
  handle_t h;
  struct statfs *buf;
  int rc;

  h = *(handle_t *) params;
  buf = *(struct statfs **) (params + 4);

  f = (struct file *) olock(h, OBJECT_FILE);
  if (!f) return -EBADF;

  if (lock_buffer(buf, sizeof(struct statfs), 1) < 0) {
    orel(f);
    return -EFAULT;
  }

  rc = fstatfs(f, buf);

  unlock_buffer(buf, sizeof(struct statfs));
  orel(f);

  return rc;
}

static int sys_statfs(char *params) {
  char *name;
  struct statfs *buf;
  int rc;

  name = *(char **) params;
  buf = *(struct statfs **) (params + 4);

  if (lock_string(name) < 0) return -EFAULT;

  if (lock_buffer(buf, sizeof(struct statfs), 1) < 0) {
    unlock_string(name);
    return -EFAULT;
  }

  rc = statfs(name, buf);

  unlock_buffer(buf, sizeof(struct statfs));
  unlock_string(name);

  return rc;
}

static int sys_open(char *params) {
  struct file *f;
  char *name;
  int flags;
  int mode;
  int rc;

  name = *(char **) params;
  flags = *(int *) (params + 4);
  mode = *(int *) (params + 8);

  if ((flags & O_CREAT) == 0) mode = 0;

  if (lock_string(name) < 0) return -EFAULT;

  rc = open(name, flags, mode, &f);
  if (rc == 0) {
    rc = halloc(&f->iob.object);
    if (rc < 0) close(f);
  }

  unlock_string(name);

  return rc;
}

static int sys_close(char *params) {
  handle_t h;
  int rc;

  h = *(handle_t *) params;
  rc = hfree(h);
  return rc;
}

static int sys_fsync(char *params) {
  struct file *f;
  handle_t h;
  int rc;

  h = *(handle_t *) params;
  f = (struct file *) olock(h, OBJECT_FILE);
  if (!f) return -EBADF;

  rc = fsync(f);

  orel(f);

  return rc;
}

static int sys_read(char *params) {
  handle_t h;
  struct object *o;
  int rc;
  void *data;
  int size;

  h = *(handle_t *) params;
  data = *(void **) (params + 4);
  size = *(int *) (params + 8);

  o = olock(h, OBJECT_ANY);
  if (!o) return -EBADF;
  
  if (lock_buffer(data, size, 1) < 0) {
    orel(o);
    return -EFAULT;
  }

  if (o->type == OBJECT_FILE) {
    rc = read((struct file *) o, data, size);
  } else if (o->type == OBJECT_SOCKET) {
    rc = recv((struct socket *) o, data, size, 0);
  } else {
    rc = -EBADF;
  }

  orel(o);
  unlock_buffer(data, size);

  return rc;
}

static int sys_write(char *params) {
  handle_t h;
  struct object *o;
  int rc;
  void *data;
  int size;

  h = *(handle_t *) params;
  data = *(void **) (params + 4);
  size = *(int *) (params + 8);

  o = olock(h, OBJECT_ANY);
  if (!o) return -EBADF;

  if (lock_buffer(data, size, 0) < 0) {
    orel(o);
    return -EFAULT;
  }

  if (o->type == OBJECT_FILE) {
    rc = write((struct file *) o, data, size);
  } else if (o->type == OBJECT_SOCKET) {
    rc = send((struct socket *) o, data, size, 0);
  } else {
    rc = -EBADF;
  }
  
  orel(o);
  unlock_buffer(data, size);

  return rc;
}

static int sys_ioctl(char *params) {
  handle_t h;
  struct object *o;
  int rc;
  int cmd;
  void *data;
  int size;

  h = *(handle_t *) params;
  cmd = *(int *) (params + 4);
  data = *(void **) (params + 8);
  size = *(int *) (params + 12);

  o = olock(h, OBJECT_ANY);
  if (!o) return -EBADF;

  if (lock_buffer(data, size, 1) < 0) {
    orel(o);
    return -EFAULT;
  }
  
  if (o->type == OBJECT_FILE) {
    rc = ioctl((struct file *) o, cmd, data, size);
  } else if (o->type == OBJECT_SOCKET) {
    rc = ioctlsocket((struct socket *) o, cmd, data, size);
  } else {
    rc = -EBADF;
  }

  orel(o);

  return rc;
}

static int sys_tell(char *params) {
  struct file *f;
  handle_t h;
  off64_t rc;
  off64_t *retval;

  h = *(handle_t *) params;
  retval = *(off64_t **) (params + 4);

  f = (struct file *) olock(h, OBJECT_FILE);
  if (!f) return -EBADF;

  if (lock_buffer(retval, sizeof(off64_t), 1) < 0) {
    orel(f);
    return -EFAULT;
  }

  rc = tell(f);
  if (retval) *retval = rc;

  orel(f);
  unlock_buffer(retval, sizeof(off64_t));

  return rc < 0 ? (int) rc : (int) (rc & 0x7FFFFFFF);
}

static int sys_lseek(char *params) {
  struct file *f;
  handle_t h;
  off64_t offset;
  int origin;
  off64_t rc;
  off64_t *retval;

  h = *(handle_t *) params;
  offset = *(off64_t *) (params + 4);
  origin = *(int *) (params + 12);
  retval = *(off64_t **) (params + 16);

  f = (struct file *) olock(h, OBJECT_FILE);
  if (!f) return -EBADF;

  if (lock_buffer(retval, sizeof(off64_t), 1) < 0) {
    orel(f);
    return -EFAULT;
  }

  rc = lseek(f, offset, origin);
  if (retval) *retval = rc;

  orel(f);
  unlock_buffer(retval, sizeof(off64_t));

  return rc < 0 ? (int) rc : (int) (rc & 0x7FFFFFFF);
}

static int sys_ftruncate(char *params) {
  struct file *f;
  handle_t h;
  off64_t size;
  int rc;

  h = *(handle_t *) params;
  size = *(off64_t *) (params + 4);

  f = (struct file *) olock(h, OBJECT_FILE);
  if (!f) return -EBADF;

  rc = ftruncate(f, size);

  orel(f);

  return rc;
}

static int sys_futime(char *params) {
  struct file *f;
  handle_t h;
  struct utimbuf *times;
  int rc;

  h = *(handle_t *) params;
  times = *(struct utimbuf **) (params + 4);

  f = (struct file *) olock(h, OBJECT_FILE);
  if (!f) return -EBADF;

  if (lock_buffer(times, sizeof(struct utimbuf), 1) < 0) {
    orel(f);
    return -EFAULT;
  }

  rc = futime(f, times);

  unlock_buffer(times, sizeof(struct utimbuf));
  orel(f);

  return rc;
}

static int sys_utime(char *params) {
  char *name;
  struct utimbuf *times;
  int rc;

  name = *(char **) params;
  times = *(struct utimbuf **) (params + 4);

  if (lock_string(name) < 0) return -EFAULT;

  if (lock_buffer(times, sizeof(struct utimbuf), 1) < 0) {
    unlock_string(name);
    return -EFAULT;
  }

  rc = utime(name, times);

  unlock_buffer(times, sizeof(struct utimbuf));
  unlock_string(name);

  return rc;
}

static int sys_fstat(char *params) {
  struct file *f;
  handle_t h;
  struct stat64 *buffer;
  int rc;

  h = *(handle_t *) params;
  buffer = *(struct stat64 **) (params + 4);

  f = (struct file *) olock(h, OBJECT_FILE);
  if (!f) return -EBADF;

  if (lock_buffer(buffer, sizeof(struct stat64), 1) < 0) {
    orel(f);
    return -EFAULT;
  }

  rc = fstat(f, buffer);

  unlock_buffer(buffer, sizeof(struct stat64));
  orel(f);

  if (buffer && rc > 0) rc = 0;
  return rc;
}

static int sys_stat(char *params) {
  char *name;
  struct stat64 *buffer;
  int rc;

  name = *(char **) params;
  buffer = *(struct stat64 **) (params + 4);

  if (lock_string(name) < 0) return -EFAULT;

  if (lock_buffer(buffer, sizeof(struct stat64), 1) < 0) {
    unlock_string(name);
    return -EFAULT;
  }

  rc = stat(name, buffer);

  unlock_buffer(buffer, sizeof(struct stat64));
  unlock_string(name);

  if (buffer && rc > 0) rc = 0;
  return rc;
}

static int sys_mkdir(char *params) {
  char *name;
  int mode;
  int rc;

  name = *(char **) params;
  mode = *(int *) (params + 4);

  if (lock_string(name) < 0) return -EFAULT;

  rc = mkdir(name, mode);

  unlock_string(name);

  return rc;
}

static int sys_rmdir(char *params) {
  char *name;
  int rc;

  name = *(char **) params;

  if (lock_string(name) < 0) return -EFAULT;

  rc = rmdir(name);

  unlock_string(name);

  return rc;
}

static int sys_rename(char *params) {
  char *oldname;
  char *newname;
  int rc;

  oldname = *(char **) params;
  newname = *(char **) (params + 4);

  if (lock_string(oldname) < 0) return -EFAULT;

  if (lock_string(newname) < 0)  {
    unlock_string(oldname);
    return -EFAULT;
  }

  rc = rename(oldname, newname);

  unlock_string(newname);
  unlock_string(oldname);

  return rc;
}

static int sys_link(char *params) {
  char *oldname;
  char *newname;
  int rc;

  oldname = *(char **) params;
  newname = *(char **) (params + 4);

  if (lock_string(oldname) < 0) return -EFAULT;

  if (lock_string(newname) < 0) {
    unlock_string(oldname);
    return -EFAULT;
  }

  rc = link(oldname, newname);

  unlock_string(newname);
  unlock_string(oldname);

  return rc;
}

static int sys_unlink(char *params) {
  char *name;
  int rc;

  name = *(char **) params;

  if (lock_string(name) < 0) return -EFAULT;

  rc = unlink(name);

  unlock_string(name);

  return rc;
}

static int sys_opendir(char *params) {
  struct file *f;
  char *name;
  int rc;

  name = *(char **) params;

  if (lock_string(name) < 0) return -EFAULT;

  rc = opendir(name, &f);
  if (rc == 0) {
    rc = halloc(&f->iob.object);
    if (rc < 0) close(f);
  }

  unlock_string(name);

  return rc;
}

static int sys_readdir(char *params) {
  handle_t h;
  struct file *f;
  int rc;
  struct direntry *dirp;
  int count;

  h = *(handle_t *) params;
  dirp = *(struct direntry **) (params + 4);
  count = *(int *) (params + 8);

  f = (struct file *) olock(h, OBJECT_FILE);
  if (f == NULL) return -EBADF;

  if (lock_buffer(dirp, count * sizeof(struct direntry), 1) < 0) {
    orel(f);
    return -EFAULT;
  }

  rc = readdir(f, dirp, count);

  orel(f);

  return rc;
}

static int sys_vmalloc(char *params) {
  char *addr;
  unsigned long size;
  int type;
  int protect;
  unsigned long tag;
  void *retval;
  int rc;

  addr = *(void **) params;
  size = *(unsigned long *) (params + 4);
  type = *(int *) (params + 8);
  protect = *(int *) (params + 12);
  tag = *(unsigned long *) (params + 16);

  retval = vmalloc(addr, size, type, protect, tag, &rc);
  if (!retval) {
    struct tib *tib = self()->tib;
    if (tib) tib->errnum = -rc;
  }

  return (int) retval;
}

static int sys_vmfree(char *params) {
  char *addr;
  unsigned long size;
  int type;
  int rc;

  addr = *(void **) params;
  size = *(unsigned long *) (params + 4);
  type = *(int *) (params + 8);

  rc = vmfree(addr, size, type);

  return rc;
}

static int sys_vmrealloc(char *params) {
  char *addr;
  unsigned long oldsize;
  unsigned long newsize;
  int type;
  int protect;
  unsigned long tag;
  void *retval;

  addr = *(void **) params;
  oldsize = *(unsigned long *) (params + 4);
  newsize = *(unsigned long *) (params + 8);
  type = *(int *) (params + 12);
  protect = *(int *) (params + 16);
  tag = *(unsigned long *) (params + 20);

  retval = vmrealloc(addr, oldsize, newsize, type, protect, tag);

  return (int) retval;
}

static int sys_vmprotect(char *params) {
  char *addr;
  unsigned long size;
  int protect;
  int rc;

  addr = *(void **) params;
  size = *(unsigned long *) (params + 4);
  protect = *(int *) (params + 8);

  rc = vmprotect(addr, size, protect);

  return rc;
}

static int sys_vmlock(char *params) {
  char *addr;
  unsigned long size;
  int rc;

  addr = *(void **) params;
  size = *(unsigned long *) (params + 4);

  rc = vmlock(addr, size);

  return rc;
}

static int sys_vmunlock(char *params) {
  char *addr;
  unsigned long size;
  int rc;

  addr = *(void **) params;
  size = *(unsigned long *) (params + 4);

  rc = vmunlock(addr, size);

  return rc;
}

static int sys_vmmap(char *params) {
  char *addr;
  unsigned long size;
  int protect;
  handle_t h;
  off64_t offset;
  struct file *f;
  void *retval;
  int rc;

  addr = *(void **) params;
  size = *(unsigned long *) (params + 4);
  protect = *(int *) (params + 8);
  h = *(handle_t *) (params + 12);
  offset = *(off64_t *) (params + 16);

  f = (struct file *) olock(h, OBJECT_FILE);
  if (!f) return -EBADF;

  retval = vmmap(addr, size, protect, f, offset, &rc);
  if (!retval) {
    struct tib *tib = self()->tib;
    if (tib) tib->errnum = -rc;
  }

  orel(f);

  return (int) retval;
}

static int sys_vmsync(char *params) {
  char *addr;
  unsigned long size;
  int rc;

  addr = *(void **) params;
  size = *(unsigned long *) (params + 4);

  rc = vmsync(addr, size);

  return rc;
}

static int sys_waitone(char *params) {
  handle_t h;
  struct object *o;
  unsigned int timeout;
  int rc;

  h = *(handle_t *) params;
  timeout = *(unsigned int *) (params + 4);

  o = (struct object *) olock(h, OBJECT_ANY);
  if (!o) return -EBADF;

  rc = wait_for_one_object(o, timeout, 1);

  orel(o);
  return rc;
}

static int sys_waitall(char *params) {
  handle_t *h;
  int count;
  unsigned int timeout;
  struct object *o[MAX_WAIT_OBJECTS];
  int rc;
  int n;

  h = *(handle_t **) params;
  count = *(int *) (params + 4);
  timeout = *(unsigned int *) (params + 8);

  if (count < 0 || count > MAX_WAIT_OBJECTS) return -EINVAL;

  if ((!h && count > 0) || lock_buffer(h, count * sizeof(handle_t *), 0) < 0) return -EFAULT;

  for (n = 0; n < count; n++) {
    o[n] = olock(h[n], OBJECT_ANY);
    if (!o[n]) {
      while (--n >= 0) orel(o[n]);
      return -EBADF;
    }
  }

  if (count == 1) {
    rc = wait_for_one_object(o[0], timeout, 1);
  } else {
    rc = wait_for_all_objects(o, count, timeout, 1);
  }

  for (n = 0; n < count; n++) orel(o[n]);
  return rc;
}

static int sys_waitany(char *params) {
  handle_t *h;
  int count;
  unsigned int timeout;
  struct object *o[MAX_WAIT_OBJECTS];
  int rc;
  int n;

  h = *(handle_t **) params;
  count = *(int *) (params + 4);
  timeout = *(unsigned int *) (params + 8);

  if (count < 0 || count > MAX_WAIT_OBJECTS) return -EINVAL;

  if ((!h && count > 0) || lock_buffer(h, count * sizeof(handle_t *), 0) < 0) return -EFAULT;

  for (n = 0; n < count; n++) {
    o[n] = olock(h[n], OBJECT_ANY);
    if (!o[n]) {
      while (--n >= 0) orel(o[n]);
      return -EBADF;
    }
  }

  if (count == 1) {
    rc = wait_for_one_object(o[0], timeout, 1);
  } else {
    rc = wait_for_any_object(o, count, timeout, 1);
  }

  for (n = 0; n < count; n++) orel(o[n]);
  return rc;
}

static int sys_mkevent(char *params) {
  int manual_reset;
  int initial_state;
  struct event *e;
  handle_t h;

  manual_reset = *(int *) params;
  initial_state = *(int *) (params + 4);

  e = (struct event *) kmalloc(sizeof(struct event));
  if (!e) return -ENOMEM;

  init_event(e, manual_reset, initial_state);

  h = halloc(&e->object);
  if (h < 0) {
    close_object(&e->object);
    return h;
  }

  return h;
}

static int sys_epulse(char *params) {
  struct event *e;
  handle_t h;

  h = *(handle_t *) params;
  e = (struct event *) olock(h, OBJECT_EVENT);
  if (!e) return -EBADF;

  pulse_event(e);

  orel(e);

  return 0;
}

static int sys_eset(char *params) {
  struct event *e;
  handle_t h;

  h = *(handle_t *) params;
  e = (struct event *) olock(h, OBJECT_EVENT);
  if (!e) return -EBADF;

  set_event(e);

  orel(e);

  return 0;
}

static int sys_ereset(char *params) {
  struct event *e;
  handle_t h;

  h = *(handle_t *) params;
  e = (struct event *) olock(h, OBJECT_EVENT);
  if (!e) return -EBADF;

  reset_event(e);

  orel(e);

  return 0;
}

static int sys_getthreadblock(char *params) {
  handle_t h;
  struct thread *t;
  struct tib *tib;

  h = *(handle_t *) params;
  t = (struct thread *) olock(h, OBJECT_THREAD);
  if (!t) return -EBADF;

  tib = t->tib;

  orel(t);

  return (int) tib;
}

static int sys_exitos(char *params) {
  int mode;

  mode = *(int *) params;

  stop(mode);

  return 0;
}

static int sys_dup(char *params) {
  handle_t h;
  handle_t newh;
  struct object *o;

  h = *(handle_t *) params;

  o = olock(h, OBJECT_ANY);
  if (!o) return -EBADF;

  newh = halloc(o);

  orel(o);
  return newh;
}

static int sys_dup2(char *params) {
  handle_t h1;
  handle_t h2;
  struct object *o;
  int rc;

  h1 = *(handle_t *) params;
  h2 = *(handle_t *) (params + 4);

  o = olock(h1, OBJECT_ANY);
  if (!o) return -EBADF;

  rc = hassign(o, h2);

  orel(o);

  return rc;
}

static int sys_mkthread(char *params) {
  void *entrypoint;
  unsigned long stacksize;
  char *name;
  struct tib **ptib;
  struct thread *t;
  handle_t h;
  int rc;

  entrypoint = *(void **) params;
  stacksize = *(unsigned long *) (params + 4);
  name = *(char **) (params + 8);
  ptib = *(struct tib ***) (params + 12);

  if (entrypoint >= (void *) OSBASE) return -EFAULT;
  if (lock_string(name) < 0) return -EFAULT;

  rc = create_user_thread(entrypoint, stacksize, name, &t);
  if (rc < 0) return rc;

  h = halloc(&t->object);
  if (h < 0) {
    unlock_string(name);
    return h;
  }

  if (ptib) *ptib = t->tib;

  unlock_string(name);

  return h;
}

static int sys_suspend(char *params) {
  handle_t h;
  struct thread *t;
  int rc;

  h = *(handle_t *) params;

  t = (struct thread *) olock(h, OBJECT_THREAD);
  if (!t || t->id == 0) return -EBADF;

  rc = suspend_thread(t);

  orel(t);
  return rc;
}

static int sys_resume(char *params) {
  handle_t h;
  struct thread *t;
  int rc;

  h = *(handle_t *) params;

  t = (struct thread *) olock(h, OBJECT_THREAD);
  if (!t) return -EBADF;

  rc = resume_thread(t);

  orel(t);
  return rc;
}

static int sys_endthread(char *params) {
  int exitcode;

  exitcode = *(int *) params;

  terminate_thread(exitcode);

  return 0;
}

static int sys_setcontext(char *params) {
  handle_t h;
  struct thread *t;
  struct context *context;
  int rc;

  h = *(handle_t *) params;
  context = *(struct context **) (params + 4);

  t = (struct thread *) olock(h, OBJECT_THREAD);
  if (!t) return -EBADF;

  if (lock_buffer(context, sizeof(struct context), 0) < 0) {
    orel(t);
    return -EFAULT;
  }

  rc = set_context(t, context);

  unlock_buffer(context, sizeof(struct context));
  orel(t);

  return rc;
}

static int sys_getcontext(char *params) {
  handle_t h;
  struct thread *t;
  struct context *context;
  int rc;

  h = *(handle_t *) params;
  context = *(struct context **) (params + 4);

  t = (struct thread *) olock(h, OBJECT_THREAD);
  if (!t) return -EBADF;

  if (lock_buffer(context, sizeof(struct context), 1) < 0) {
    orel(t);
    return -EFAULT;
  }

  rc = get_context(t, context);

  unlock_buffer(context, sizeof(struct context));
  orel(t);

  return rc;
}

static int sys_getprio(char *params) {
  handle_t h;
  struct thread *t;
  int priority;

  h = *(handle_t *) params;

  t = (struct thread *) olock(h, OBJECT_THREAD);
  if (!t) return -EBADF;

  priority = get_thread_priority(t);

  orel(t);

  return priority;
}

static int sys_setprio(char *params) {
  handle_t h;
  struct thread *t;
  int priority;
  int rc;

  h = *(handle_t *) params;
  priority = *(int *) (params + 4);

  t = (struct thread *) olock(h, OBJECT_THREAD);
  if (!t) return -EBADF;

  if (priority < 1 || priority > 15) {
    // User mode code can only set priority levels 1-15
    rc = -EINVAL;
  } else if (!t->tib) {
    // User mode code not allowed to set priority for kernel threads
    rc = -EPERM;
  } else {
    // Change thread priority
    rc = set_thread_priority(t, priority);
  }

  orel(t);
  return rc;
}

static int sys_msleep(char *params) {
  int millisecs;
  int rc;

  millisecs = *(int *) params;

  rc = msleep(millisecs);

  return rc;
}

static int sys_time(char *params) {
  time_t *timeptr;
  time_t t;

  timeptr = *(time_t **) params;

  if (lock_buffer(timeptr, sizeof(time_t *), 1) < 0) return -EFAULT;

  t = get_time();
  if (timeptr) *timeptr = t;

  unlock_buffer(timeptr, sizeof(time_t *));

  return t;
}

static int sys_gettimeofday(char *params) {
  struct timeval *tv;
  void *tzp;

  tv = *(struct timeval **) params;
  tzp = *(void **) (params + 4);

  if (!tv) return -EINVAL;
  if (lock_buffer(tv, sizeof(struct timeval), 1) < 0) return -EFAULT;

  tv->tv_sec = systemclock.tv_sec;
  tv->tv_usec = systemclock.tv_usec;

  unlock_buffer(tv, sizeof(struct timeval));
  
  return 0;
}

static int sys_settimeofday(char *params) {
  struct timeval *tv;

  tv = *(struct timeval **) params;

  if (!tv) return -EINVAL;
  if (lock_buffer(tv, sizeof(struct timeval), 0) < 0) return -EFAULT;

  set_time(tv);

  unlock_buffer(tv, sizeof(struct timeval));

  return 0;
}

static int sys_clock(char *params) {
  return clocks;
}

static int sys_mksem(char *params) {
  int initial_count;
  struct sem *s;
  handle_t h;

  initial_count = *(int *) params;

  s = (struct sem *) kmalloc(sizeof(struct sem));
  if (!s) return -ENOMEM;

  init_sem(s, initial_count);

  h = halloc(&s->object);
  if (h < 0) {
    close_object(&s->object);
    return h;
  }

  return h;
}

static int sys_semrel(char *params) {
  handle_t h;
  struct sem *s;
  int count;
  int rc;

  h = *(handle_t *) params;
  count = *(int *) (params + 4);

  s = (struct sem *) olock(h, OBJECT_SEMAPHORE);
  if (!s) return -EBADF;

  rc = release_sem(s, count);

  orel(s);
  return rc;
}

static int sys_accept(char *params) {
  handle_t h;
  struct socket *s;
  struct socket *news;
  int rc;
  struct sockaddr *addr;
  int *addrlen;

  h = *(handle_t *) params;
  addr = *(struct sockaddr **) (params + 4);
  addrlen = *(int **) (params + 8);

  s = (struct socket *) olock(h, OBJECT_SOCKET);
  if (!s) return -EBADF;

  if (lock_buffer(addr, sizeof(struct sockaddr), 1) < 0) {
    orel(s);
    return -EFAULT;
  }
  
  if (lock_buffer(addrlen, sizeof(int), 1) < 0) {
    orel(s);
    unlock_buffer(addr, sizeof(struct sockaddr));
    return -EFAULT;
  }

  rc = accept(s, addr, addrlen, &news);
  if (rc == 0) {
    rc = halloc(&news->iob.object);
    if (rc < 0) closesocket(news);
  }

  orel(s);
  unlock_buffer(addrlen, sizeof(int));
  unlock_buffer(addr, sizeof(struct sockaddr));

  return rc;
}

static int sys_bind(char *params) {
  handle_t h;
  struct socket *s;
  int rc;
  struct sockaddr *name;
  int namelen;

  h = *(handle_t *) params;
  name = *(struct sockaddr **) (params + 4);
  namelen = *(int *) (params + 8);

  s = (struct socket *) olock(h, OBJECT_SOCKET);
  if (!s) return -EBADF;

  if (lock_buffer(name, namelen, 0) < 0) {
    orel(s);
    return -EFAULT;
  }
  
  rc = bind(s, name, namelen);

  orel(s);
  unlock_buffer(name, namelen);

  return rc;
}

static int sys_connect(char *params) {
  handle_t h;
  struct socket *s;
  int rc;
  struct sockaddr *name;
  int namelen;

  h = *(handle_t *) params;
  name = *(struct sockaddr **) (params + 4);
  namelen = *(int *) (params + 8);

  s = (struct socket *) olock(h, OBJECT_SOCKET);
  if (!s) return -EBADF;

  if (lock_buffer(name, namelen, 0) < 0) {
    orel(s);
    return -EFAULT;
  }
  
  rc = connect(s, name, namelen);

  orel(s);
  unlock_buffer(name, namelen);

  return rc;
}

static int sys_getpeername(char *params) {
  handle_t h;
  struct socket *s;
  int rc;
  struct sockaddr *name;
  int *namelen;

  h = *(handle_t *) params;
  name = *(struct sockaddr **) (params + 4);
  namelen = *(int **) (params + 8);

  s = (struct socket *) olock(h, OBJECT_SOCKET);
  if (!s) return -EBADF;

  if (lock_buffer(name, sizeof(struct sockaddr), 1) < 0) {
    orel(s);
    return -EFAULT;
  }
  
  if (lock_buffer(namelen, 4, 1) < 0) {
    orel(s);
    unlock_buffer(name, sizeof(struct sockaddr));
    return -EFAULT;
  }

  rc = getpeername(s, name, namelen);

  orel(s);
  unlock_buffer(namelen, 4);
  unlock_buffer(name, sizeof(struct sockaddr));

  return rc;
}

static int sys_getsockname(char *params) {
  handle_t h;
  struct socket *s;
  int rc;
  struct sockaddr *name;
  int *namelen;

  h = *(handle_t *) params;
  name = *(struct sockaddr **) (params + 4);
  namelen = *(int **) (params + 8);

  s = (struct socket *) olock(h, OBJECT_SOCKET);
  if (!s) return -EBADF;

  if (lock_buffer(name, sizeof(struct sockaddr), 1) < 0) {
    orel(s);
    return -EFAULT;
  }
  
  if (lock_buffer(namelen, 4, 1) < 0) {
    orel(s);
    unlock_buffer(name, sizeof(struct sockaddr));
    return -EFAULT;
  }

  rc = getsockname(s, name, namelen);

  orel(s);
  unlock_buffer(namelen, 4);
  unlock_buffer(name, sizeof(struct sockaddr));

  return rc;
}

static int sys_getsockopt(char *params) {
  handle_t h;
  struct socket *s;
  int rc;
  int level;
  int optname;
  char *optval;
  int *optlen;
  int inoptlen;

  h = *(handle_t *) params;
  level = *(int *) (params + 4);
  optname = *(int *) (params + 8);
  optval = *(char **) (params + 12);
  optlen = *(int **) (params + 16);

  s = (struct socket *) olock(h, OBJECT_SOCKET);
  if (!s) return -EBADF;

  if (!optlen || lock_buffer(optlen, 4, 1) < 0) {
    orel(s);
    return -EFAULT;
  }
  inoptlen = *optlen;

  if (lock_buffer(optval, inoptlen, 1) < 0) {
    orel(s);
    unlock_buffer(optlen, 4);
    return -EFAULT;
  }

  rc = getsockopt(s, level, optname, optval, optlen);

  orel(s);
  unlock_buffer(optval, inoptlen);
  unlock_buffer(optlen, 4);

  return rc;
}

static int sys_listen(char *params) {
  handle_t h;
  struct socket *s;
  int rc;
  int backlog;

  h = *(handle_t *) params;
  backlog = *(int *) (params + 4);

  s = (struct socket *) olock(h, OBJECT_SOCKET);
  if (!s) return -EBADF;

  rc = listen(s, backlog);

  orel(s);

  return rc;
}

static int sys_recv(char *params) {
  handle_t h;
  struct socket *s;
  int rc;
  void *data;
  int size;
  unsigned int flags;

  h = *(handle_t *) params;
  data = *(void **) (params + 4);
  size = *(int *) (params + 8);
  flags = *(unsigned int *) (params + 12);

  s = (struct socket *) olock(h, OBJECT_SOCKET);
  if (!s) return -EBADF;

  if (lock_buffer(data, size, 1) < 0) {
    orel(s);
    return -EFAULT;
  }
  
  rc = recv(s, data, size, flags);

  orel(s);
  unlock_buffer(data, size);

  return rc;
}

static int sys_recvfrom(char *params) {
  handle_t h;
  struct socket *s;
  int rc;
  void *data;
  int size;
  unsigned int flags;
  struct sockaddr *from;
  int *fromlen;

  h = *(handle_t *) params;
  data = *(void **) (params + 4);
  size = *(int *) (params + 8);
  flags = *(unsigned int *) (params + 12);
  from = *(struct sockaddr **) (params + 16);
  fromlen = *(int **) (params + 20);

  s = (struct socket *) olock(h, OBJECT_SOCKET);
  if (!s) return -EBADF;

  if (lock_buffer(data, size, 1) < 0) {
    orel(s);
    return -EFAULT;
  }

  if (lock_buffer(from, sizeof(struct sockaddr), 1) < 0) {
    orel(s);
    unlock_buffer(data, size);
    return -EFAULT;
  }

  if (lock_buffer(fromlen, sizeof(int), 1) < 0) {
    orel(s);
    unlock_buffer(from, sizeof(struct sockaddr));
    unlock_buffer(data, size);
    return -EFAULT;
  }

  rc = recvfrom(s, data, size, flags, from, fromlen);

  orel(s);
  unlock_buffer(fromlen, sizeof(int));
  unlock_buffer(from, sizeof(struct sockaddr));
  unlock_buffer(data, size);

  return rc;
}

static int sys_send(char *params) {
  handle_t h;
  struct socket *s;
  int rc;
  void *data;
  int size;
  unsigned int flags;

  h = *(handle_t *) params;
  data = *(void **) (params + 4);
  size = *(int *) (params + 8);
  flags = *(unsigned int *) (params + 12);

  s = (struct socket *) olock(h, OBJECT_SOCKET);
  if (!s) return -EBADF;

  if (lock_buffer(data, size, 0) < 0) {
    orel(s);
    return -EFAULT;
  }
  
  rc = send(s, data, size, flags);

  orel(s);
  unlock_buffer(data, size);

  return rc;
}

static int sys_sendto(char *params) {
  handle_t h;
  struct socket *s;
  int rc;
  void *data;
  int size;
  unsigned int flags;
  struct sockaddr *to;
  int tolen;

  h = *(handle_t *) params;
  data = *(void **) (params + 4);
  size = *(int *) (params + 8);
  flags = *(unsigned int *) (params + 12);
  to = *(struct sockaddr **) (params + 16);
  tolen = *(int *) (params + 20);

  s = (struct socket *) olock(h, OBJECT_SOCKET);
  if (!s) return -EBADF;

  if (lock_buffer(data, size, 0) < 0) {
    orel(s);
    return -EFAULT;
  }

  if (lock_buffer(to, sizeof(struct sockaddr), 0) < 0) {
    orel(s);
    unlock_buffer(data, size);
    return -EFAULT;
  }

  rc = sendto(s, data, size, flags, to, tolen);

  orel(s);
  unlock_buffer(to, sizeof(struct sockaddr));
  unlock_buffer(data, size);

  return rc;
}

static int sys_setsockopt(char *params) {
  handle_t h;
  struct socket *s;
  int rc;
  int level;
  int optname;
  char *optval;
  int optlen;

  h = *(handle_t *) params;
  level = *(int *) (params + 4);
  optname = *(int *) (params + 8);
  optval = *(char **) (params + 12);
  optlen = *(int *) (params + 16);

  s = (struct socket *) olock(h, OBJECT_SOCKET);
  if (!s) return -EBADF;

  if (lock_buffer(optval, optlen, 0) < 0) {
    orel(s);
    return -EFAULT;
  }

  rc = setsockopt(s, level, optname, optval, optlen);

  orel(s);
  unlock_buffer(optval, optlen);

  return rc;
}

static int sys_shutdown(char *params) {
  handle_t h;
  struct socket *s;
  int rc;
  int how;

  h = *(handle_t *) params;
  how = *(int *) (params + 4);

  s = (struct socket *) olock(h, OBJECT_SOCKET);
  if (!s) return -EBADF;

  rc = shutdown(s, how);

  orel(s);

  return rc;
}

static int sys_socket(char *params) {
  int rc;
  struct socket *s;
  int domain;
  int type;
  int protocol;

  domain = *(int *) params;
  type = *(int *) (params + 4);
  protocol = *(int *) (params + 8);

  rc = socket(domain, type, protocol, &s);
  if (rc == 0) {
    rc = halloc(&s->iob.object);
    if (rc < 0) closesocket(s);
  }

  return rc;
}

static int sys_readv(char *params) {
  handle_t h;
  struct object *o;
  int rc;
  struct iovec *iov;
  int count;

  h = *(handle_t *) params;
  iov = *(struct iovec **) (params + 4);
  count = *(int *) (params + 8);

  o = olock(h, OBJECT_ANY);
  if (!o) return -EBADF;

  if (lock_iovec(iov, count, 1) < 0) {
    orel(o);
    return -EFAULT;
  }

  if (o->type == OBJECT_FILE) {
    rc = readv((struct file *) o, iov, count);
  } else if (o->type == OBJECT_SOCKET) {
    rc = recvv((struct socket *) o, iov, count);
  } else {
    rc = -EBADF;
  }

  unlock_iovec(iov, count);
  orel(o);

  return rc;
}

static int sys_writev(char *params) {
  handle_t h;
  struct object *o;
  int rc;
  struct iovec *iov;
  int count;

  h = *(handle_t *) params;
  iov = *(struct iovec **) (params + 4);
  count = *(int *) (params + 8);

  o = olock(h, OBJECT_ANY);
  if (!o) return -EBADF;
  
  if (lock_iovec(iov, count, 0) < 0) {
    orel(o);
    return -EFAULT;
  }

  if (o->type == OBJECT_FILE) {
    rc = writev((struct file *) o, iov, count);
  } else if (o->type == OBJECT_SOCKET) {
    rc = sendv((struct socket *) o, iov, count);
  } else {
    rc = -EBADF;
  }

  unlock_iovec(iov, count);
  orel(o);

  return rc;
}

static int sys_chdir(char *params) {
  char *name;
  int rc;

  name = *(char **) params;

  if (lock_string(name) < 0) return -EFAULT;

  rc = chdir(name);

  unlock_string(name);

  return rc;
}

static int sys_mkiomux(char *params) {
  int flags;
  struct iomux *iomux;
  handle_t h;

  flags = *(int *) params;

  iomux = (struct iomux *) kmalloc(sizeof(struct iomux));
  if (!iomux) return -ENOMEM;

  init_iomux(iomux, flags);

  h = halloc(&iomux->object);
  if (h < 0) {
    close_object(&iomux->object);
    return h;
  }

  return h;
}

static int sys_dispatch(char *params) {
  handle_t ioh;
  handle_t h;
  struct iomux *iomux;
  struct object *o;
  int events;
  int context;
  int rc;

  ioh = *(handle_t *) params;
  h = *(handle_t *) (params + 4);
  events = *(int *) (params + 8);
  context = *(int *) (params + 12);

  iomux = (struct iomux *) olock(ioh, OBJECT_IOMUX);
  if (!iomux) return -EBADF;

  o = (struct object *) olock(h, OBJECT_ANY);
  if (!o)  {
    orel(iomux);
    return -EBADF;
  }

  rc = queue_ioobject(iomux, o, events, context);

  orel(o);
  orel(iomux);
  return rc;
}

static int sys_recvmsg(char *params) {
  handle_t h;
  struct msghdr *msg;
  unsigned int flags;
  struct socket *s;
  int rc;

  h = *(handle_t *) params;
  msg = *(struct msghdr **) (params + 4);
  flags = *(unsigned int *) (params + 8);

  s = (struct socket *) olock(h, OBJECT_SOCKET);
  if (!s) return -EBADF;

  if (lock_buffer(msg, sizeof(struct msghdr), 1) < 0) {
    orel(s);
    return -EFAULT;
  }
  
  if (lock_iovec(msg->msg_iov, msg->msg_iovlen, 1) < 0) {
    unlock_buffer(msg, sizeof(struct msghdr));
    orel(s);
    return -EFAULT;
  }

  if (lock_buffer(msg->msg_name, msg->msg_namelen, 1) < 0) {
    unlock_iovec(msg->msg_iov, msg->msg_iovlen);
    unlock_buffer(msg, sizeof(struct msghdr));
    orel(s);
    return -EFAULT;
  }

  rc = recvmsg(s, msg, flags);

  unlock_iovec(msg->msg_iov, msg->msg_iovlen);
  unlock_buffer(msg, sizeof(struct msghdr));
  orel(s);

  return rc;
}

static int sys_sendmsg(char *params) {
  handle_t h;
  struct msghdr *msg;
  unsigned int flags;
  struct socket *s;
  int rc;

  h = *(handle_t *) params;
  msg = *(struct msghdr **) (params + 4);
  flags = *(unsigned int *) (params + 8);

  s = (struct socket *) olock(h, OBJECT_SOCKET);
  if (!s) return -EBADF;

  if (lock_buffer(msg, sizeof(struct msghdr), 0) < 0) {
    orel(s);
    return -EFAULT;
  }
  
  if (lock_iovec(msg->msg_iov, msg->msg_iovlen, 0) < 0) {
    unlock_buffer(msg, sizeof(struct msghdr));
    orel(s);
    return -EFAULT;
  }

  if (lock_buffer(msg->msg_name, msg->msg_namelen, 0) < 0) {
    unlock_iovec(msg->msg_iov, msg->msg_iovlen);
    unlock_buffer(msg, sizeof(struct msghdr));
    orel(s);
    return -EFAULT;
  }

  rc = sendmsg(s, msg, flags);

  unlock_iovec(msg->msg_iov, msg->msg_iovlen);
  unlock_buffer(msg, sizeof(struct msghdr));
  orel(s);

  return rc;
}

static int sys_select(char *params) {
  int nfds;
  fd_set *readfds;
  fd_set *writefds;
  fd_set *exceptfds;
  struct timeval *timeout;
  int rc;

  nfds = *(int *) params;
  readfds = *(fd_set **) (params + 4);
  writefds = *(fd_set **) (params + 8);
  exceptfds = *(fd_set **) (params + 12);
  timeout = *(struct timeval **) (params + 16);

  if (lock_fdset(readfds, 1) < 0) return -EFAULT;

  if (lock_fdset(writefds, 1) < 0) {
    unlock_fdset(readfds);
    return -EFAULT;
  }

  if (lock_fdset(exceptfds, 1) < 0) {
    unlock_fdset(writefds);
    unlock_fdset(readfds);
    return -EFAULT;
  }

  if (lock_buffer(timeout, sizeof(struct timeval), 1) < 0) {
    unlock_fdset(exceptfds);
    unlock_fdset(writefds);
    unlock_fdset(readfds);
    return -EFAULT;
  }

  rc = select(nfds, readfds, writefds, exceptfds, timeout);

  unlock_buffer(timeout, sizeof(struct timeval));
  unlock_fdset(exceptfds);
  unlock_fdset(writefds);
  unlock_fdset(readfds);

  return rc;
}

static int sys_pipe(char *params) {
  handle_t *fildes;
  int rc;
  struct file *readpipe;
  struct file *writepipe;

  fildes = *(handle_t **) params;

  if (lock_buffer(fildes, sizeof(handle_t) * 2, 1) < 0) return -EFAULT;

  rc = pipe(&readpipe, &writepipe);

  if (rc == 0) {
    fildes[0] = halloc(&readpipe->iob.object);
    fildes[1] = halloc(&writepipe->iob.object);

    if (fildes[0] < 0 || fildes[1] < 0) {
      if (fildes[0] >= 0) hfree(fildes[0]);
      if (fildes[1] >= 0) hfree(fildes[1]);

      close(readpipe);
      close(writepipe);
      rc = -ENFILE;
    }
  }

  unlock_buffer(fildes, sizeof(handle_t) * 2);

  return rc;
}

static int sys_setmode(char *params) {
  handle_t h;
  struct file *f;
  int rc;
  int mode;

  h = *(handle_t *) params;
  mode = *(int *) (params + 4);

  f = (struct file *) olock(h, OBJECT_FILE);
  if (!f) return -EBADF;
  
  rc = setmode(f, mode);

  orel(f);

  return rc;
}

static int sys_chmod(char *params) {
  char *name;
  int mode;
  int rc;

  name = *(char **) params;
  mode = *(int *) (params + 4);

  if (lock_string(name) < 0) return -EFAULT;

  rc = chmod(name, mode);

  unlock_string(name);

  return rc;
}

static int sys_fchmod(char *params) {
  handle_t h;
  struct file *f;
  int rc;
  int mode;

  h = *(handle_t *) params;
  mode = *(int *) (params + 4);

  f = (struct file *) olock(h, OBJECT_FILE);
  if (!f) return -EBADF;

  rc = fchmod(f, mode);

  orel(f);

  return rc;
}

static int sys_sysinfo(char *params) {
  int rc;
  int cmd;
  void *data;
  int size;

  cmd = *(int *) params;
  data = *(void **) (params + 4);
  size = *(int *) (params + 8);

  if (lock_buffer(data, size, 1) < 0) return -EFAULT;

  switch (cmd) {
    case SYSINFO_CPU:
      if (!data || size < sizeof(struct cpuinfo)) {
        rc = -EFAULT;
      } else {
        rc = cpu_sysinfo((struct cpuinfo *) data);
      }
      break;

    case SYSINFO_MEM:
      if (!data || size < sizeof(struct meminfo)) {
        rc = -EFAULT;
      } else {
        rc = mem_sysinfo((struct meminfo *) data);
      }
      break;

    case SYSINFO_LOAD:
      if (!data || size < sizeof(struct loadinfo)) {
        rc = -EFAULT;
      } else {
        rc = load_sysinfo((struct loadinfo *) data);
      }
      break;

    default:
      rc = -EINVAL;
  }

  unlock_buffer(data, size);

  return rc;
}

static int sys_mkmutex(char *params) {
  int owned;
  struct mutex *m;
  handle_t h;

  owned = *(int *) params;

  m = (struct mutex *) kmalloc(sizeof(struct mutex));
  if (!m) return -ENOMEM;

  init_mutex(m, owned);

  h = halloc(&m->object);
  if (h < 0) {
    close_object(&m->object);
    return h;
  }

  return h;
}

static int sys_mutexrel(char *params) {
  handle_t h;
  struct mutex *m;
  int rc;

  h = *(handle_t *) params;

  m = (struct mutex *) olock(h, OBJECT_MUTEX);
  if (!m) return -EBADF;

  rc = release_mutex(m);

  orel(m);
  return rc;
}

static int sys_pread(char *params) {
  handle_t h;
  struct object *o;
  int rc;
  void *data;
  int size;
  off64_t offset;

  h = *(handle_t *) params;
  data = *(void **) (params + 4);
  size = *(int *) (params + 8);
  offset = *(off64_t *) (params + 12);

  o = olock(h, OBJECT_FILE);
  if (!o) return -EBADF;
  
  if (lock_buffer(data, size, 1) < 0) {
    orel(o);
    return -EFAULT;
  }

  rc = pread((struct file *) o, data, size, offset);

  orel(o);
  unlock_buffer(data, size);

  return rc;
}

static int sys_pwrite(char *params) {
  handle_t h;
  struct object *o;
  int rc;
  void *data;
  int size;
  off64_t offset;

  h = *(handle_t *) params;
  data = *(void **) (params + 4);
  size = *(int *) (params + 8);
  offset = *(off64_t *) (params + 12);

  o = olock(h, OBJECT_FILE);
  if (!o) return -EBADF;

  if (lock_buffer(data, size, 0) < 0) {
    orel(o);
    return -EFAULT;
  }

  rc = pwrite((struct file *) o, data, size, offset);

  orel(o);
  unlock_buffer(data, size);

  return rc;
}

static int sys_getuid(char *params) {
  return getuid();
}

static int sys_setuid(char *params) {
  uid_t uid;

  uid = *(uid_t *) params;

  return setuid(uid);
}

static int sys_getgid(char *params) {
  return getgid();
}

static int sys_setgid(char *params) {
  gid_t gid;

  gid = *(gid_t *) params;

  return setgid(gid);
}

static int sys_geteuid(char *params) {
  return geteuid();
}

static int sys_seteuid(char *params) {
  uid_t uid;

  uid = *(uid_t *) params;

  return seteuid(uid);
}

static int sys_getegid(char *params) {
  return getegid();
}

static int sys_setegid(char *params) {
  gid_t gid;

  gid = *(gid_t *) params;

  return setegid(gid);
}

static int sys_getgroups(char *params) {
  int rc;
  int size;
  gid_t *list;

  size = *(int *) params;
  list = *(gid_t **) (params + 4);

  if (lock_buffer(list, size * sizeof(gid_t), 1) < 0) return -EFAULT;

  rc = getgroups(size, list);

  unlock_buffer(list, size * sizeof(gid_t));

  return rc;
}

static int sys_setgroups(char *params) {
  int rc;
  int size;
  gid_t *list;

  size = *(int *) params;
  list = *(gid_t **) (params + 4);

  if (lock_buffer(list, size * sizeof(gid_t), 0) < 0) return -EFAULT;

  rc = setgroups(size, list);

  unlock_buffer(list, size * sizeof(gid_t));

  return rc;
}

static int sys_chown(char *params) {
  char *name;
  int owner;
  int group;
  int rc;

  name = *(char **) params;
  owner = *(int *) (params + 4);
  group = *(int *) (params + 8);

  if (lock_string(name) < 0) return -EFAULT;

  rc = chown(name, owner, group);

  unlock_string(name);

  return rc;
}

static int sys_fchown(char *params) {
  handle_t h;
  struct file *f;
  int rc;
  int owner;
  int group;

  h = *(handle_t *) params;
  owner = *(int *) (params + 4);
  group = *(int  *) (params + 8);

  f = (struct file *) olock(h, OBJECT_FILE);
  if (!f) return -EBADF;
  
  rc = fchown(f, owner, group);

  orel(f);

  return rc;
}

static int sys_access(char *params) {
  char *name;
  int mode;
  int rc;

  name = *(char **) params;
  mode = *(int *) (params + 4);

  if (lock_string(name) < 0) return -EFAULT;

  rc = access(name, mode);

  unlock_string(name);

  return rc;
}

static int sys_poll(char *params) {
  struct pollfd *fds;
  unsigned int nfds;
  int timeout;
  int rc;

  fds = *(struct pollfd **) params;
  nfds = *(unsigned int *) (params + 4);
  timeout = *(int *) (params + 8);

  if (lock_buffer(fds, nfds * sizeof(struct pollfd), 1) < 0) return -EFAULT;
  
  rc = poll(fds, nfds, timeout);

  unlock_buffer(fds, nfds * sizeof(struct pollfd));

  return rc;
}

static int sys_getcwd(char *params) {
  char *buf;
  int size;
  int rc;

  buf = *(char **) params;
  size = *(unsigned int *) (params + 4);

  if (lock_buffer(buf, size, 1) < 0) return -EFAULT;
  
  rc = getcwd(buf, size);

  unlock_buffer(buf, size);

  return rc;
}

static int sys_sendsig(char *params) {
  handle_t h;
  int signum;
  struct thread *t;
  int rc;

  h = *(handle_t *) params;
  signum = *(int *) (params + 4);

  t = (struct thread *) olock(h, OBJECT_THREAD);
  if (!t) return -EBADF;
  
  rc = send_user_signal(t, signum);

  orel(t);

  return rc;
}

static int sys_sigprocmask(char *params) {
  int how;
  sigset_t *set;
  sigset_t *oldset;
  int rc;

  how = *(int *) params;
  set = *(sigset_t **) (params + 4);
  oldset = *(sigset_t **) (params + 8);

  if (lock_buffer(set, sizeof(sigset_t), 0) < 0) return -EFAULT;
  if (lock_buffer(oldset, sizeof(sigset_t), 1) < 0) {
    unlock_buffer(set, sizeof(sigset_t));
    return -EFAULT;
  }

  rc = set_signal_mask(how, set, oldset);

  unlock_buffer(set, sizeof(sigset_t));
  unlock_buffer(oldset, sizeof(sigset_t));

  return rc;
}

static int sys_sigpending(char *params) {
  sigset_t *set;
  int rc;

  set = *(sigset_t **) params;

  if (lock_buffer(set, sizeof(sigset_t), 1) < 0) return -EFAULT;

  rc = get_pending_signals(set);

  unlock_buffer(set, sizeof(sigset_t));

  return rc;
}

static int sys_alarm(char *params) {
  unsigned int seconds;
  int rc;

  seconds = *(unsigned int *) params;

  rc = schedule_alarm(seconds);

  return rc;
}

struct syscall_entry syscalltab[] = {
  {"null",0, "", sys_null},
  {"mkfs", 12, "'%s','%s','%s'", sys_mkfs},
  {"mount", 16, "'%s','%s','%s','%s'", sys_mount},
  {"umount", 4, "'%s'", sys_umount},
  {"open", 12, "'%s',%x,%x", sys_open},
  {"close", 4, "%d", sys_close},
  {"fsync", 4, "%d", sys_fsync},
  {"read", 12, "%d,%p,%d", sys_read},
  {"write", 12, "%d,%p,%d", sys_write},
  {"tell", 8, "%d,%p", sys_tell},
  {"lseek", 10, "%d,%d-%d,%d,%p", sys_lseek},
  {"ftruncate", 12, "%d,%d-%d", sys_ftruncate},
  {"fstat", 8, "%d,%p", sys_fstat},
  {"stat", 8, "'%s',%p", sys_stat},
  {"mkdir", 8, "'%s',%d", sys_mkdir},
  {"rmdir", 4, "'%s'", sys_rmdir},
  {"rename", 8, "'%s','%s'", sys_rename},
  {"link", 8, "'%s','%s'", sys_link},
  {"unlink", 4, "'%s'", sys_unlink},
  {"opendir", 4, "'%s'", sys_opendir},
  {"readdir", 12, "%d,%p,%d", sys_readdir},
  {"vmalloc", 16, "%p,%d,%x,%x", sys_vmalloc},
  {"vmfree", 12, "%p,%d,%x", sys_vmfree},
  {"vmrealloc", 20, "%p,%d,%d,%x,%x", sys_vmrealloc},
  {"vmprotect", 12, "%p,%d,%x", sys_vmprotect},
  {"vmlock", 8, "%p,%d", sys_vmlock},
  {"vmunlock", 8, "%p,%d", sys_vmunlock},
  {"waitone", 8, "%d,%d", sys_waitone},
  {"mkevent", 8, "%d,%d", sys_mkevent},
  {"epulse", 4, "%d", sys_epulse},
  {"eset", 4, "%d", sys_eset},
  {"ereset", 4, "%d", sys_ereset},
  {"getthreadblock", 4, "%d", sys_getthreadblock},
  {"exitos", 4, "%d", sys_exitos},
  {"dup", 4, "%d", sys_dup},
  {"mkthread", 12, "%p,%d,%p", sys_mkthread},
  {"suspend", 4, "%d", sys_suspend},
  {"resume", 4, "%d", sys_resume},
  {"endthread", 4, "%d", sys_endthread},
  {"setcontext", 8, "%d,%p", sys_setcontext},
  {"getcontext", 8, "%d,%p", sys_getcontext},
  {"getprio", 4, "%d", sys_getprio},
  {"setprio", 8, "%d,%d", sys_setprio},
  {"msleep", 4, "%d", sys_msleep},
  {"time", 0, "", sys_time},
  {"gettimeofday", 8, "%p,%p", sys_gettimeofday},
  {"clock", 0, "", sys_clock},
  {"mksem", 4, "%d", sys_mksem},
  {"semrel", 8, "%p,%d", sys_semrel},
  {"ioctl", 16, "%d,%x,%p,%d", sys_ioctl},
  {"getfsstat", 8, "%p,%d", sys_getfsstat},
  {"fstatfs", 8, "%d,%p", sys_fstatfs},
  {"statfs", 8, "'%s',%p", sys_statfs},
  {"futime", 8, "%d,%p", sys_futime},
  {"utime", 8, "'%s',%p", sys_utime},
  {"settimeofday", 4, "%p", sys_settimeofday},
  {"accept", 12, "%d,%p,%p", sys_accept},
  {"bind", 12, "%d,%p,%d", sys_bind},
  {"connect", 12, "%d,%p,%d", sys_connect},
  {"getpeername", 12, "%d,%p,%p", sys_getpeername},
  {"getsockname", 12, "%d,%p,%p", sys_getsockname},
  {"getsockopt", 20, "%d,%d,%d,%p,%p", sys_getsockopt},
  {"listen", 8, "%d,%d", sys_listen},
  {"recv", 16, "%d,%p,%d,%d", sys_recv},
  {"recvfrom", 24, "%d,%p,%d,%d,%p,%p", sys_recvfrom},
  {"send", 16, "%d,%p,%d,%d", sys_send},
  {"sendto", 24, "%d,%p,%d,%d,%p,%d", sys_sendto},
  {"setsockopt", 20, "%d,%d,%d,%p,%d", sys_setsockopt},
  {"shutdown", 8, "%d,%d", sys_shutdown},
  {"socket", 12, "%d,%d,%d", sys_socket},
  {"waitall", 12, "%p,%d,%d", sys_waitall},
  {"waitany", 12, "%p,%d,%d", sys_waitany},
  {"readv", 12, "%d,%p,%d", sys_readv},
  {"writev", 12, "%d,%p,%d", sys_writev},
  {"chdir", 4, "'%s'", sys_chdir},
  {"mkiomux", 4, "%d", sys_mkiomux},
  {"dispatch", 16, "%d,%d,%d,%d", sys_dispatch},
  {"recvmsg", 12, "%d,%p,%d", sys_recvmsg},
  {"sendmsg", 12, "%d,%p,%d", sys_sendmsg},
  {"select", 20, "%d,%p,%p,%p,%p", sys_select},
  {"pipe", 4, "%p", sys_pipe},
  {"dup2", 8, "%d,%d", sys_dup2},
  {"setmode", 8, "%d,%d", sys_setmode},
  {"chmod", 8, "'%s',%d", sys_chmod},
  {"fchmod", 8, "%d,%d", sys_fchmod},
  {"sysinfo", 12, "%d,%p,%d", sys_sysinfo},
  {"mkmutex", 4, "%d", sys_mkmutex},
  {"mutexrel", 4, "%p", sys_mutexrel},
  {"pread", 20, "%d,%p,%d,%d-%d", sys_pread},
  {"pwrite", 20, "%d,%p,%d,%d-%d", sys_pwrite},
  {"getuid", 0, "", sys_getuid},
  {"setuid", 4, "%d", sys_setuid},
  {"getgid", 0, "", sys_getgid},
  {"setgid", 4, "%d", sys_setgid},
  {"geteuid", 0, "", sys_geteuid},
  {"seteuid", 4, "%d", sys_seteuid},
  {"getegid", 0, "", sys_getegid},
  {"setegid", 4, "%d", sys_setegid},
  {"getgroups", 8, "%d,%p", sys_getgroups},
  {"setgroups", 8, "%d,%p", sys_setgroups},
  {"chown", 12, "'%s',%d,%d", sys_chown},
  {"fchown", 12, "%d,%d,%d", sys_fchown},
  {"access", 8, "'%s',%d", sys_access},
  {"poll", 12, "%p,%d,%d", sys_poll},
  {"getcwd", 8, "%p,%d", sys_getcwd},
  {"sendsig", 8, "%d,%d", sys_sendsig},
  {"sigprocmask", 12, "%d,%p,%p", sys_sigprocmask},
  {"sigpending", 4, "%p", sys_sigpending},
  {"alarm", 4, "%d", sys_alarm},
  {"vmmap", 24, "%p,%d,%x,%d,%d-%d", sys_vmmap},
  {"vmsync", 8, "%p,%d", sys_vmsync},
};

int syscall(int syscallno, char *params, struct context *ctxt) {
  int rc;
  struct thread *t = self();

  t->ctxt = ctxt;
  if (syscallno < 0 || syscallno > SYSCALL_MAX) return -ENOSYS;

#ifdef SYSCALL_LOGENTER
#ifndef SYSCALL_LOGWAIT
  if (syscallno != SYSCALL_WAITONE && syscallno != SYSCALL_WAITALL && syscallno != SYSCALL_WAITANY)
#endif
  {
    char buf[1024];

    vsprintf(buf, syscalltab[syscallno].argfmt, params);
    print_string("<-");
    print_string(syscalltab[syscallno].name);
    print_string("(");
    print_string(buf);
    print_string(")\n");
  }
#endif

#ifdef SYSCALL_PROFILE
  sccnt[syscallno]++;
#endif

  rc = lock_buffer(params, syscalltab[syscallno].paramsize, 0);
  if (rc >= 0) {
    rc = syscalltab[syscallno].func(params);
    unlock_buffer(params, syscalltab[syscallno].paramsize);
  }

  if (rc < 0) {
    struct tib *tib = t->tib;
    if (tib) tib->errnum = -rc;
#ifdef SYSCALL_PROFILE
    scerr[syscallno]++;
#endif
  }

#if defined(SYSCALL_LOGEXIT) || defined(SYSCALL_LOGONLYERRORS)

#ifdef SYSCALL_LOGONLYERRORS

#ifdef SYSCALL_LOGTIMEOUTS
  if (rc < 0)
#else
  if (rc < 0 && rc != -ETIMEOUT)
#endif

#else

#ifndef SYSCALL_LOGWAIT
  if (syscallno != SYSCALL_WAITONE && syscallno != SYSCALL_WAITALL && syscallno != SYSCALL_WAITANY)
#endif

#endif
  {
    char buf[1024];
    char rcstr[16];

    vsprintf(buf, syscalltab[syscallno].argfmt, params);
    sprintf(rcstr, "%d", rc);
    print_string(syscalltab[syscallno].name);
    print_string("(");
    print_string(buf);
    print_string(") -> ");
    print_string(rcstr);
    print_string("\n");
  }
#endif

  check_dpc_queue();
  check_preempt();
  if (signals_ready(t)) deliver_pending_signals(rc);

  t->ctxt = NULL;

  if (rc < 0) return -1;
  return rc;
}

#ifdef SYSCALL_PROFILE
static int syscalls_proc(struct proc_file *pf, void *arg) {
  int i = 0;

  pprintf(pf, "syscall               calls     errors\n");
  pprintf(pf, "---------------- ---------- ----------\n");
  for (i = 0; i < SYSCALL_MAX  + 1; i++) {
    if (sccnt[i] != 0) {
      pprintf(pf, "%-16s %10d %10d\n", syscalltab[i].name, sccnt[i], scerr[i]);
    }
  }

  return 0;
}
#endif

void init_syscall() {
#ifdef SYSCALL_PROFILE
  register_proc_inode("syscalls", syscalls_proc, NULL);
#endif
}
