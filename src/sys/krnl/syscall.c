//
// syscall.c
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// System call interface
//

#include <os/krnl.h>

struct syscall_entry
{
  char *name;
  char *argfmt;
  int (*func)(char *); 
};

static int lock_buffer(void *buffer, int size)
{
  return 0;
}

static int unlock_buffer(void *buffer, int size)
{
  return 0;
}

static int lock_string(char *s)
{
  return 0;
}

static int unlock_string(char *s)
{
  return 0;
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
  devno_t dev;

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

  dev = dev_open(devname);
  if (dev != NODEV)
  {
    rc = format(dev, type, opts);
    dev_close(dev);
  }
  else
    rc = -ENOENT;

  unlock_string(opts);
  unlock_string(type);
  unlock_string(devname);
  unlock_buffer(params, 12);

  return rc;
}

static int sys_mount(char *params)
{
  char *type;
  char *path;
  char *devname; 
  char *opts;
  int rc;
  devno_t dev;

  if (lock_buffer(params, 16) < 0) return -EFAULT;

  type = *(char **) params;
  path = *(char **) (params + 4);
  devname = *(char **) (params + 8);
  opts = *(char **) (params + 12);

  if (lock_string(type) < 0)
  {
    unlock_buffer(params, 16);
    return -EFAULT;
  }

  if (lock_string(path) < 0)
  {
    unlock_string(type);
    unlock_buffer(params, 16);
    return -EFAULT;
  }

  if (lock_string(devname) < 0)
  {
    unlock_string(path);
    unlock_string(type);
    unlock_buffer(params, 16);
    return -EFAULT;
  }

  if (lock_string(opts) < 0)
  {
    unlock_string(devname);
    unlock_string(path);
    unlock_string(type);
    unlock_buffer(params, 16);
    return -EFAULT;
  }

  dev = dev_open(devname);
  if (dev != NODEV)
    rc = mount(type, path, dev, opts);
  else
    rc = -ENOENT;

  unlock_string(opts);
  unlock_string(devname);
  unlock_string(path);
  unlock_string(type);
  unlock_buffer(params, 16);

  return rc;
}

static int sys_unmount(char *params)
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

  rc = unmount(path);

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

  f = (struct file *) hlock(h, OBJECT_FILE);
  if (!f) 
  {
    unlock_buffer(params, 8);
    return -EBADF;
  }

  if (lock_buffer(buf, sizeof(struct statfs)) < 0)
  {
    hrel(h);
    unlock_buffer(params, 8);
    return -EFAULT;
  }

  rc = fstatfs(f, buf);

  unlock_buffer(buf, sizeof(struct statfs));
  hrel(h);
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
  f = (struct file *) hlock(h, OBJECT_FILE);
  if (!f) 
  {
    unlock_buffer(params, 4);
    return -EBADF;
  }

  rc = flush(f);

  hrel(h);
  unlock_buffer(params, 4);

  return rc;
}

static int sys_read(char *params)
{
  handle_t h;
  struct file *f;
  int rc;
  void *data;
  int size;

  if (lock_buffer(params, 12) < 0) return -EFAULT;

  h = *(handle_t *) params;
  data = *(void **) (params + 4);
  size = *(int *) (params + 8);

  f = (struct file *) hlock(h, OBJECT_FILE);
  if (!f) 
  {
    unlock_buffer(params, 12);
    return -EBADF;
  }

  if (lock_buffer(data, size) < 0)
  {
    hrel(h);
    unlock_buffer(params, 12);
    return -EFAULT;
  }
  
  rc = read(f, data, size);
  
  hrel(h);
  unlock_buffer(params, 12);

  return rc;
}

static int sys_write(char *params)
{
  handle_t h;
  struct file *f;
  int rc;
  void *data;
  int size;

  if (lock_buffer(params, 12) < 0) return -EFAULT;

  h = *(handle_t *) params;
  data = *(void **) (params + 4);
  size = *(int *) (params + 8);

  f = (struct file *) hlock(h, OBJECT_FILE);
  if (!f) 
  {
    unlock_buffer(params, 12);
    return -EBADF;
  }

  if (lock_buffer(data, size) < 0)
  {
    hrel(h);
    unlock_buffer(params, 12);
    return -EFAULT;
  }
  
  rc = write(f, data, size);
  
  hrel(h);
  unlock_buffer(params, 12);

  return rc;
}

static int sys_ioctl(char *params)
{
  handle_t h;
  struct file *f;
  int rc;
  int cmd;
  void *data;
  int size;

  if (lock_buffer(params, 16) < 0) return -EFAULT;

  h = *(handle_t *) params;
  cmd = *(int *) (params + 4);
  data = *(void **) (params + 8);
  size = *(int *) (params + 12);

  f = (struct file *) hlock(h, OBJECT_FILE);
  if (!f) 
  {
    unlock_buffer(params, 16);
    return -EBADF;
  }

  if (lock_buffer(data, size) < 0)
  {
    hrel(h);
    unlock_buffer(params, 16);
    return -EFAULT;
  }
  
  rc = ioctl(f, cmd, data, size);
  
  hrel(h);
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
  f = (struct file *) hlock(h, OBJECT_FILE);
  if (!f) 
  {
    unlock_buffer(params, 4);
    return -EBADF;
  }

  retval = flush(f);

  hrel(h);
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

  f = (struct file *) hlock(h, OBJECT_FILE);
  if (!f) 
  {
    unlock_buffer(params, 12);
    return -EBADF;
  }

  retval = lseek(f, offset, origin);

  hrel(h);
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

  f = (struct file *) hlock(h, OBJECT_FILE);
  if (!f) 
  {
    unlock_buffer(params, 8);
    return -EBADF;
  }

  rc = chsize(f, size);

  hrel(h);
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

  f = (struct file *) hlock(h, OBJECT_FILE);
  if (!f) 
  {
    unlock_buffer(params, 8);
    return -EBADF;
  }

  if (lock_buffer(times, sizeof(struct utimbuf)) < 0)
  {
    hrel(h);
    unlock_buffer(params, 8);
    return -EFAULT;
  }

  rc = futime(f, times);

  unlock_buffer(times, sizeof(struct utimbuf));
  hrel(h);
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

  f = (struct file *) hlock(h, OBJECT_FILE);
  if (!f) 
  {
    unlock_buffer(params, 8);
    return -EBADF;
  }

  if (lock_buffer(buffer, sizeof(struct stat)) < 0)
  {
    hrel(h);
    unlock_buffer(params, 8);
    return -EFAULT;
  }

  rc = fstat(f, buffer);

  unlock_buffer(buffer, sizeof(struct stat));
  hrel(h);
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

  f = (struct file *) hlock(h, OBJECT_FILE);
  if (f == NULL) 
  {
    unlock_buffer(params, 12);
    return -EBADF;
  }

  if (lock_buffer(dirp, count * sizeof(struct dirent)) < 0)
  {
    hrel(h);
    unlock_buffer(params, 12);
    return -EFAULT;
  }
  
  rc = readdir(f, dirp, count);
  
  hrel(h);
  unlock_buffer(params, 12);

  return rc;
}

static int sys_mmap(char *params)
{
  char *addr;
  unsigned long size;
  int type;
  int protect;
  void *retval;

  if (lock_buffer(params, 16) < 0) return -EFAULT;

  addr = *(void **) params;
  size = *(unsigned long *) (params + 4);
  type = *(int *) (params + 8);
  protect = *(int *) (params + 12);

  retval = mmap(addr, size, type, protect);

  unlock_buffer(params, 16);

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
  void *retval;

  if (lock_buffer(params, 20) < 0) return -EFAULT;

  addr = *(void **) params;
  oldsize = *(unsigned long *) (params + 4);
  newsize = *(unsigned long *) (params + 8);
  type = *(int *) (params + 12);
  protect = *(int *) (params + 16);

  retval = mremap(addr, oldsize, newsize, type, protect);

  unlock_buffer(params, 20);

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

  o = (struct object *) hlock(h, OBJECT_ANY);
  if (!o) 
  {
    unlock_buffer(params, 8);
    return -EBADF;
  }

  rc = wait_for_object(o, timeout);

  hrel(h);
  unlock_buffer(params, 8);
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
    kfree(e);
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
  e = (struct event *) hlock(h, OBJECT_EVENT);
  if (!e) 
  {
    unlock_buffer(params, 4);
    return -EBADF;
  }

  pulse_event(e);

  hrel(h);
  unlock_buffer(params, 4);

  return 0;
}

static int sys_eset(char *params)
{
  struct event *e;
  handle_t h;

  if (lock_buffer(params, 4) < 0) return -EFAULT;

  h = *(handle_t *) params;
  e = (struct event *) hlock(h, OBJECT_EVENT);
  if (!e) 
  {
    unlock_buffer(params, 4);
    return -EBADF;
  }

  set_event(e);

  hrel(h);
  unlock_buffer(params, 4);

  return 0;
}

static int sys_ereset(char *params)
{
  struct event *e;
  handle_t h;

  if (lock_buffer(params, 4) < 0) return -EFAULT;

  h = *(handle_t *) params;
  e = (struct event *) hlock(h, OBJECT_EVENT);
  if (!e) 
  {
    unlock_buffer(params, 4);
    return -EBADF;
  }

  reset_event(e);

  hrel(h);
  unlock_buffer(params, 4);

  return 0;
}

static int sys_self(char *params)
{
  return current_thread()->self;
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

  o = hlock(h, OBJECT_ANY);
  if (!o) 
  {
    unlock_buffer(params, 4);
    return -EBADF;
  }

  newh = halloc(o);

  hrel(h);
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

  t = (struct thread *) hlock(h, OBJECT_THREAD);
  if (!t) 
  {
    unlock_buffer(params, 4);
    return -EBADF;
  }

  rc = suspend_thread(t);

  hrel(h);
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

  t = (struct thread *) hlock(h, OBJECT_THREAD);
  if (!t) 
  {
    unlock_buffer(params, 4);
    return -EBADF;
  }

  rc = resume_thread(t);

  hrel(h);
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

static int sys_getcontext(char *params)
{
  handle_t h;
  struct thread *t;
  void *context;

  if (lock_buffer(params, 8) < 0) return -EFAULT;

  h = *(handle_t *) params;
  context = *(void **) (params + 4);

  t = (struct thread *) hlock(h, OBJECT_THREAD);
  if (!t) 
  {
    unlock_buffer(params, 8);
    return -EBADF;
  }

  kprintf("warning: getcontext not implemented\n");

  hrel(h);
  unlock_buffer(params, 8);
  return -ENOSYS;
}

static int sys_getprio(char *params)
{
  handle_t h;
  struct thread *t;
  int priority;

  if (lock_buffer(params, 4) < 0) return -EFAULT;

  h = *(handle_t *) params;

  t = (struct thread *) hlock(h, OBJECT_THREAD);
  if (!t) 
  {
    unlock_buffer(params, 4);
    return -EBADF;
  }

  priority = t->priority;

  hrel(h);
  unlock_buffer(params, 4);
  return priority;
}

static int sys_setprio(char *params)
{
  handle_t h;
  struct thread *t;
  int priority;

  if (lock_buffer(params, 8) < 0) return -EFAULT;

  h = *(handle_t *) params;
  priority = *(int *) (params + 4);

  t = (struct thread *) hlock(h, OBJECT_THREAD);
  if (!t) 
  {
    unlock_buffer(params, 8);
    return -EBADF;
  }

  t->priority = priority;
  // TODO: reschedule thread to immediately reflect priority change

  hrel(h);
  unlock_buffer(params, 8);
  return 0;
}

static int sys_sleep(char *params)
{
  int millisecs;

  if (lock_buffer(params, 4) < 0) return -EFAULT;

  millisecs = *(int *) params;

  sleep(millisecs);

  unlock_buffer(params, 4);
  return 0;
}

static int sys_time(char *params)
{
  return get_time();
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

  tv->tv_usec = systemclock.tv_usec;
  tv->tv_sec = systemclock.tv_sec;

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

  systemclock.tv_usec = tv->tv_usec;
  systemclock.tv_sec = tv->tv_sec;

  unlock_buffer(tv, sizeof(struct timeval));
  unlock_buffer(params, 4);
  return 0;
}

static int sys_clock(char *params)
{
  return get_tick_count() * (1000 / TICKS_PER_SEC);
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
    kfree(s);
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

  s = (struct sem *) hlock(h, OBJECT_SEMAPHORE);
  if (!s) 
  {
    unlock_buffer(params, 8);
    return -EBADF;
  }

  rc = release_sem(s, count);

  hrel(h);
  unlock_buffer(params, 8);
  return rc;
}

struct syscall_entry syscalltab[] =
{
  {"null","", sys_null},
  {"format", "'%s','%s','%s'", sys_format},
  {"mount", "'%s','%s','%s','%s'", sys_mount},
  {"unmount", "'%s'", sys_unmount},
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
  {"statfs", "%s,%p", sys_statfs},
  {"futime", "%d,%p", sys_futime},
  {"utime", "%s,%p", sys_utime},
  {"settimeofday", "%p", sys_settimeofday}
};

int syscall(int syscallno, char *params)
{
  int rc;

#if 0
  {
    char buf[1024];

    vsprintf(buf, syscalltab[syscallno].argfmt, params);
    kprintf("<-%s(%s)\n", syscalltab[syscallno].name, buf);
  }
#endif

  if (syscallno < 0 || syscallno > SYSCALL_MAX) return -ENOSYS;
  rc = syscalltab[syscallno].func(params);

#if 0
  {
    char buf[1024];

    vsprintf(buf, syscalltab[syscallno].argfmt, params);
    kprintf("%s(%s) -> %d\n", syscalltab[syscallno].name, buf, rc);
  }
#endif

  return rc;
}
