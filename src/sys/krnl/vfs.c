//
// vfs.c
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Virtual filesystem
//

#include <os/krnl.h>

struct filesystem *fslist = NULL;
struct fs *mountlist = NULL;

int canonicalize(char *path, char *buffer)
{
  char *p;
  int len;

  // Check for maximum filename length
  if (!path) return -EINVAL;
  if (strlen(path) >= MAXPATH) return -ENAMETOOLONG;

  // Remove drive letter and initial path separator from filename (e.g. c:\)
  if (path[0] != 0 && path[1] == ':') path += 2;
  
  p = buffer;
  while (*path)
  {
    // Parse path separator
    if (*path == PS1 || *path == PS2) path++;
    *p++ = PS1;

    // Parse next name part in path
    len = 0;
    while (*path && *path != PS1 && *path != PS2)
    {
      // We do not allow control characters in filenames
      if (*path > 0 && *path < ' ') return -EINVAL;
      
      *p++ = *path++;
      len++;
    }

    // Handle empty name parts and '.' and '..'
    if (len == 0)
      p--;
    if (len == 1 && path[-1] == '.')
      p -= 2;
    else if (len == 2 && path[-1] == '.' && path[-2] == '.')
    {
      p -= 4;
      if (p < buffer) return -EINVAL;
      while (*p != PS1) p--;
    }
  }

  // Convert empty filename to /
  if (p == buffer) *p++ = PS1;

  // Terminate string
  *p = 0;

  return p - buffer;
}

int fnmatch(char *fn1, int len1, char *fn2, int len2)
{
  if (len1 != len2) return 0;
  while (len1--)
  {
    if (*fn1++ != *fn2++) return 0;
  }

  return 1;
}

struct fs *fslookup(char *name, char **rest)
{
  struct fs *fs;
  char *p;
  char *q;
  int n;
  int m;

  // Remove initial path separator from filename
  if (!name) return NULL;
  p = name;
  if (*p == PS1 || *p == PS2) p++;
  n = strlen(p);

  // Find matching filesystem in mount list
  fs = mountlist;
  while (fs)
  {
    q = fs->mntto;
    if (*q == PS1 || *q == PS2) q++;

    if (!*q)
    {
      if (rest) *rest = p;
      return fs;
    }

    m = strlen(q);
    if (n >= m && fnmatch(p, m, q, m) && (p[m] == 0 || p[m] == PS1 || p[m] == PS2))
    {
      if (rest) *rest = p + m;
      return fs;
    }

    fs = fs->next;
  }

  return NULL;
}

int __inline lock_fs(struct fs *fs, int fsop)
{
  if (fs->ops->reentrant & fsop) return 0;
  if (fs->ops->lockfs)
    return fs->ops->lockfs(fs);
  else
    return wait_for_object(&fs->exclusive, VFS_LOCK_TIMEOUT);
}

void __inline unlock_fs(struct fs *fs, int fsop)
{
  if (fs->ops->reentrant & fsop) return;
  if (fs->ops->unlockfs)
    fs->ops->unlockfs(fs);
  else
    release_mutex(&fs->exclusive);
}

struct filesystem *register_filesystem(char *name, struct fsops *ops)
{
  struct filesystem *fsys;

  fsys = (struct filesystem *) kmalloc(sizeof(struct filesystem));
  if (!fsys) return NULL;

  fsys->name = name;
  fsys->ops = ops;
  fsys->next = fslist;
  fslist = fsys;

  return fsys;
}

int format(char *devname, char *type, char *opts)
{
  struct filesystem *fsys;
  int rc;

  // Check arguments
  if (!type) return -EINVAL;

  // Find file system type
  fsys = fslist;
  while (fsys)
  {
    if (strcmp(type, fsys->name) == 0) break;
    fsys = fsys->next;
  }
  if (!fsys) return -EINVAL;

  // Format device for filesystem
  if (!fsys->ops->format) return -ENODEV;
  rc = fsys->ops->format(devname, opts);
  return rc;
}

int mount(char *type, char *mntto, char *mntfrom, char *opts)
{
  struct filesystem *fsys;
  struct fs *fs;
  struct fs *prevfs;
  int rc;

  // Check parameters
  if (!type) return -EINVAL;
  if (!mntto) return -EINVAL;

  // Check for file system already mounted
  fs = mountlist;
  prevfs = NULL;
  while (fs)
  {
    if (fnmatch(mntto, strlen(mntto), fs->mntto, strlen(fs->mntto))) return -EEXIST;
    if (strlen(mntto) < strlen(fs->mntto)) prevfs = fs;
    fs = fs->next;
  }

  // Find file system type
  fsys = fslist;
  while (fsys)
  {
    if (strcmp(type, fsys->name) == 0) break;
    fsys = fsys->next;
  }
  if (!fsys) return -EINVAL;

  // Allocate new mount point for file system
  fs = (struct fs *) kmalloc(sizeof(struct fs));
  if (!fs) return -ENOMEM;
  memset(fs, 0, sizeof(struct fs));

  strcpy(fs->mntto, mntto);
  strcpy(fs->mntfrom, mntfrom ? mntfrom : "");
  fs->fsys = fsys;
  fs->ops = fsys->ops;
  init_mutex(&fs->exclusive, 0);

  // Initialize filesystem on device
  if (fs->ops->mount)
  {
    rc = fs->ops->mount(fs, opts);
    if (rc != 0)
    {
      kfree(fs);
      return rc;
    }
  }

  // Insert in mount list
  if (prevfs)
  {
    fs->next = prevfs->next;
    fs->prev = prevfs;

    if (prevfs->next) prevfs->next->prev = fs;
    prevfs->next = fs;
  }
  else
  {
    fs->prev = NULL;
    fs->next = mountlist;
    if (mountlist) mountlist->prev = fs;
    mountlist = fs;
  }

  return 0;
}

int umount(char *path)
{
  struct fs *fs;
  int rc;

  // Check parameters
  if (!path) return -EINVAL;

  // Find mounted filesystem
  fs = mountlist;
  while (fs)
  {
    if (fnmatch(path, strlen(path), fs->mntto, strlen(fs->mntto))) break;
    fs = fs->next;
  }
  if (!fs) return -ENOENT;

  // Check for locks
  if (fs->locks > 0) return -EBUSY;

  // Unmount filesystem from device
  if (fs->ops->umount)
  {
    rc = fs->ops->umount(fs);
    if (rc != 0) return rc;
  }

  // Remove mounted filesystem
  if (fs->next) fs->next->prev = fs->prev;
  if (fs->prev) fs->prev->next = fs->next;
  if (mountlist == fs) mountlist = fs->next;
  kfree(fs);

  return 0;
}

int umount_all()
{
  struct fs *fs;
  struct fs *nextfs;

  fs = mountlist;
  while (fs)
  {
    if (fs->ops->umount) fs->ops->umount(fs);
    nextfs = fs->next;
    kfree(fs);
    fs = nextfs;
  }

  mountlist = NULL;
  return 0;
}

static int get_fsstat(struct fs *fs, struct statfs *buf)
{
  int rc;

  strcpy(buf->fstype, fs->fsys->name);
  strcpy(buf->mntto, fs->mntto);
  strcpy(buf->mntfrom, fs->mntfrom);

  if (fs->ops->statfs)
  {
    if (lock_fs(fs, FSOP_STATFS) < 0) return -ETIMEOUT;
    rc = fs->ops->statfs(fs, buf);
    unlock_fs(fs, FSOP_STATFS);
    if (rc < 0) return rc;
  }
  else
  {
    buf->bsize = -1;
    buf->iosize = -1;
    buf->blocks = -1;
    buf->bfree = -1;
    buf->files = -1;
    buf->ffree = -1;
  }

  return 0;
}

int getfsstat(struct statfs *buf, size_t size)
{
  int count;
  struct fs *fs;
  int rc;

  count = 0;
  fs = mountlist;
  while (fs)
  {
    if (buf)
    {
      if (size < sizeof(struct statfs)) break;
      memset(buf, 0, sizeof(struct statfs));
      rc = get_fsstat(fs, buf);
      if (rc < 0) return rc;

      buf++;
      size -= sizeof(struct statfs);
    }

    count++;
    fs = fs->next;
  }

  return count;
}

int fstatfs(struct file *filp, struct statfs *buf)
{
  if (!filp) return -EINVAL;
  if (!buf) return -EINVAL;

  return get_fsstat(filp->fs, buf);
}

int statfs(char *name, struct statfs *buf)
{
  struct fs *fs;
  char *rest;
  int rc;
  char path[MAXPATH];

  rc = canonicalize(name, path);
  if (rc < 0) return rc;

  fs = fslookup(path, &rest);
  if (!fs) return -ENOENT;

  return get_fsstat(fs, buf);
}

int open(char *name, int mode, struct file **retval)
{
  struct fs *fs;
  struct file *filp;
  int rc;
  char *rest;
  char path[MAXPATH];

  rc = canonicalize(name, path);
  if (rc < 0) return rc;

  fs = fslookup(path, &rest);
  if (!fs) return -ENOENT;

  filp = (struct file *) kmalloc(sizeof(struct file));
  if (!filp) return -ENOMEM;
  init_object(&filp->object, OBJECT_FILE);
  
  filp->fs = fs;
  filp->flags = mode;
  filp->pos = 0;
  filp->data = NULL;

  if (fs->ops->open)
  {
    fs->locks++;
    if (lock_fs(fs, FSOP_OPEN) < 0) 
    {
      fs->locks--;
      kfree(filp);
      return -ETIMEOUT;
    }

    rc = fs->ops->open(filp, rest);

    unlock_fs(fs, FSOP_OPEN);

    if (rc != 0)
    {
      fs->locks--;
      kfree(filp);
      return rc;
    }
  }

  *retval = filp;
  return 0;
}

int close(struct file *filp)
{
  int rc;

  if (!filp) return -EINVAL;
  
  if (filp->fs->ops->close)
  {
    if (lock_fs(filp->fs, FSOP_CLOSE) < 0) return -ETIMEOUT;
    rc = filp->fs->ops->close(filp);
    unlock_fs(filp->fs, FSOP_CLOSE);
  }
  else
    rc = 0;

  if (rc == 0) filp->fs->locks--;

  kfree(filp);
  return rc;
}

int flush(struct file *filp)
{
  int rc;

  if (!filp) return -EINVAL;

  if (!filp->fs->ops->flush) return -ENOSYS;

  if (lock_fs(filp->fs, FSOP_FLUSH) < 0) return -ETIMEOUT;
  rc = filp->fs->ops->flush(filp);
  unlock_fs(filp->fs, FSOP_FLUSH);

  return rc;
}

int read(struct file *filp, void *data, size_t size)
{
  int rc;

  if (!filp) return -EINVAL;
  if (!data && size > 0) return -EINVAL;
  if (filp->flags & O_WRONLY) return -EACCES;
  
  if (!filp->fs->ops->read) return -ENOSYS;
  if (lock_fs(filp->fs, FSOP_READ) < 0) return -ETIMEOUT;
  rc = filp->fs->ops->read(filp, data, size);
  unlock_fs(filp->fs, FSOP_READ);
  return rc;
}

int write(struct file *filp, void *data, size_t size)
{
  int rc;

  if (!filp) return -EINVAL;
  if (!data && size > 0) return -EINVAL;
  if (filp->flags & O_RDONLY) return -EACCES;

  if (!filp->fs->ops->write) return -ENOSYS;
  if (lock_fs(filp->fs, FSOP_WRITE) < 0) return -ETIMEOUT;
  rc = filp->fs->ops->write(filp, data, size);
  unlock_fs(filp->fs, FSOP_WRITE);
  return rc;
}

int ioctl(struct file *filp, int cmd, void *data, size_t size)
{
  int rc;

  if (!filp) return -EINVAL;
  if (!data && size > 0) return -EINVAL;

  if (!filp->fs->ops->ioctl) return -ENOSYS;
  if (lock_fs(filp->fs, FSOP_IOCTL) < 0) return -ETIMEOUT;
  rc = filp->fs->ops->ioctl(filp, cmd, data, size);
  unlock_fs(filp->fs, FSOP_IOCTL);
  return rc;
}

int readv(struct file *filp, struct iovec *iov, int count)
{
  return -ENOSYS;
}

int writev(struct file *filp, struct iovec *iov, int count)
{
  return -ENOSYS;
}

loff_t tell(struct file *filp)
{
  int rc;

  if (!filp) return -EINVAL;
 
  if (!filp->fs->ops->tell) return -ENOSYS;
  if (lock_fs(filp->fs, FSOP_TELL) < 0) return -ETIMEOUT;
  rc =  filp->fs->ops->tell(filp);
  unlock_fs(filp->fs, FSOP_TELL);
  return rc;
}

loff_t lseek(struct file *filp, loff_t offset, int origin)
{
  int rc;

  if (!filp) return -EINVAL;

  if (!filp->fs->ops->lseek) return -ENOSYS;
  if (lock_fs(filp->fs, FSOP_LSEEK) < 0) return -ETIMEOUT;
  rc = filp->fs->ops->lseek(filp, offset, origin);
  unlock_fs(filp->fs, FSOP_LSEEK);
  return rc;
}

int chsize(struct file *filp, loff_t size)
{
  int rc;

  if (!filp) return -EINVAL;
  if (filp->flags & O_RDONLY) return -EACCES;

  if (!filp->fs->ops->chsize) return -ENOSYS;
  if (lock_fs(filp->fs, FSOP_CHSIZE) < 0) return -ETIMEOUT;
  rc = filp->fs->ops->chsize(filp, size);
  unlock_fs(filp->fs, FSOP_CHSIZE);
  return rc;
}

int futime(struct file *filp, struct utimbuf *times)
{
  int rc;

  if (!filp) return -EINVAL;
  if (!times) return -EINVAL;
  if (filp->flags & O_RDONLY) return -EACCES;
 
  if (!filp->fs->ops->futime) return -ENOSYS;
  if (lock_fs(filp->fs, FSOP_FUTIME) < 0) return -ETIMEOUT;
  rc = filp->fs->ops->futime(filp, times);
  unlock_fs(filp->fs, FSOP_FUTIME);
  return rc;
}

int utime(char *name, struct utimbuf *times)
{
  struct fs *fs;
  char *rest;
  int rc;
  char path[MAXPATH];

  rc = canonicalize(name, path);
  if (rc < 0) return rc;

  fs = fslookup(path, &rest);
  if (!fs) return -ENOENT;
  
  if (!times) return -EINVAL;

  if (!fs->ops->utime) return -ENOSYS;
  fs->locks++;
  if (lock_fs(fs, FSOP_UTIME) < 0) 
  {
    fs->locks--;
    return -ETIMEOUT;
  }
  rc = fs->ops->utime(fs, rest, times);
  unlock_fs(fs, FSOP_UTIME);
  fs->locks--;
  return rc;
}

int fstat(struct file *filp, struct stat *buffer)
{
  int rc;

  if (!filp) return -EINVAL;
 
  if (!filp->fs->ops->fstat) return -ENOSYS;
  if (lock_fs(filp->fs, FSOP_FSTAT) < 0) return -ETIMEOUT;
  rc = filp->fs->ops->fstat(filp, buffer);
  unlock_fs(filp->fs, FSOP_FSTAT);
  return rc;
}

int stat(char *name, struct stat *buffer)
{
  struct fs *fs;
  char *rest;
  int rc;
  char path[MAXPATH];

  rc = canonicalize(name, path);
  if (rc < 0) return rc;

  fs = fslookup(path, &rest);
  if (!fs) return -ENOENT;
  
  if (!fs->ops->stat) return -ENOSYS;
  fs->locks++;
  if (lock_fs(fs, FSOP_STAT) < 0) 
  {
    fs->locks--;
    return -ETIMEOUT;
  }
  rc = fs->ops->stat(fs, rest, buffer);
  unlock_fs(fs, FSOP_STAT);
  fs->locks--;
  return rc;
}

int mkdir(char *name)
{
  struct fs *fs;
  char *rest;
  int rc;
  char path[MAXPATH];

  rc = canonicalize(name, path);
  if (rc < 0) return rc;

  fs = fslookup(path, &rest);
  if (!fs) return -ENOENT;

  if (!fs->ops->mkdir) return -ENOSYS;
  fs->locks++;
  if (lock_fs(fs, FSOP_MKDIR) < 0) 
  {
    fs->locks--;
    return -ETIMEOUT;
  }
  rc = fs->ops->mkdir(fs, rest);
  unlock_fs(fs, FSOP_MKDIR);
  fs->locks--;
  return rc;
}

int rmdir(char *name)
{
  struct fs *fs;
  char *rest;
  int rc;
  char path[MAXPATH];

  rc = canonicalize(name, path);
  if (rc < 0) return rc;

  fs = fslookup(path, &rest);
  if (!fs) return -ENOENT;
  
  if (!fs->ops->rmdir) return -ENOSYS;
  fs->locks++;
  if (lock_fs(fs, FSOP_RMDIR) < 0) 
  {
    fs->locks--;
    return -ETIMEOUT;
  }
  rc = fs->ops->rmdir(fs, rest);
  unlock_fs(fs, FSOP_RMDIR);
  fs->locks--;
  return rc;
}

int rename(char *oldname, char *newname)
{
  struct fs *oldfs;
  struct fs *newfs;
  char *oldrest;
  char *newrest;
  int rc;
  char oldpath[MAXPATH];
  char newpath[MAXPATH];

  rc = canonicalize(oldname, oldpath);
  if (rc < 0) return rc;

  oldfs = fslookup(oldpath, &oldrest);
  if (!oldfs) return -ENOENT;

  rc = canonicalize(newname, newpath);
  if (rc < 0) return rc;

  newfs = fslookup(newpath, &newrest);
  if (!newfs) return -ENOENT;

  if (oldfs != newfs) return -EXDEV;

  if (!oldfs->ops->rename) return -ENOSYS;
  oldfs->locks++;
  if (lock_fs(oldfs, FSOP_RENAME) < 0) 
  {
    oldfs->locks--;
    return -ETIMEOUT;
  }
  rc = oldfs->ops->rename(oldfs, oldrest, newrest);
  unlock_fs(oldfs, FSOP_RENAME);
  oldfs->locks--;
  return rc;
}

int link(char *oldname, char *newname)
{
  struct fs *oldfs;
  struct fs *newfs;
  char *oldrest;
  char *newrest;
  int rc;
  char oldpath[MAXPATH];
  char newpath[MAXPATH];

  rc = canonicalize(oldname, oldpath);
  if (rc < 0) return rc;

  oldfs = fslookup(oldpath, &oldrest);
  if (!oldfs) return -ENOENT;

  rc = canonicalize(newname, newpath);
  if (rc < 0) return rc;

  newfs = fslookup(newpath, &newrest);
  if (!newfs) return -ENOENT;

  if (oldfs != newfs) return -EXDEV;

  if (!oldfs->ops->link) return -ENOSYS;
  oldfs->locks++;
  if (lock_fs(oldfs, FSOP_LINK) < 0) 
  {
    oldfs->locks--;
    return -ETIMEOUT;
  }
  rc = oldfs->ops->link(oldfs, oldrest, newrest);
  unlock_fs(oldfs, FSOP_LINK);
  oldfs->locks--;
  return rc;
}

int unlink(char *name)
{
  struct fs *fs;
  char *rest;
  int rc;
  char path[MAXPATH];

  rc = canonicalize(name, path);
  if (rc < 0) return rc;

  fs = fslookup(path, &rest);
  if (!fs) return -ENOENT;

  if (!fs->ops->unlink) return -ENOSYS;
  fs->locks++;
  if (lock_fs(fs, FSOP_UNLINK) < 0) 
  {
    fs->locks--;
    return -ETIMEOUT;
  }
  rc = fs->ops->unlink(fs, rest);
  fs->locks--;
  unlock_fs(fs, FSOP_UNLINK);
  return rc;
}

int opendir(char *name, struct file **retval)
{
  struct fs *fs;
  struct file *filp;
  int rc;
  char *rest;
  char path[MAXPATH];

  rc = canonicalize(name, path);
  if (rc < 0) return rc;

  fs = fslookup(path, &rest);
  if (!fs) return -ENOENT;
  if (!fs->ops->opendir) return -ENOSYS;

  filp = (struct file *) kmalloc(sizeof(struct file));
  if (!filp) return -ENOMEM;
  init_object(&filp->object, OBJECT_FILE);
  
  filp->fs = fs;
  filp->flags = O_RDONLY | F_DIR;
  filp->pos = 0;
  filp->data = NULL;

  fs->locks++;
  if (lock_fs(fs, FSOP_OPENDIR) < 0) 
  {
    fs->locks--;
    return -ETIMEOUT;
  }
  rc = fs->ops->opendir(filp, rest);
  unlock_fs(fs, FSOP_OPENDIR);
  if (rc != 0)
  {
    fs->locks--;
    kfree(filp);
    return rc;
  }

  *retval = filp;
  return 0;
}

int readdir(struct file *filp, struct dirent *dirp, int count)
{
  int rc;

  if (!filp) return -EINVAL;
  if (!dirp) return -EINVAL;
  if (!(filp->flags & F_DIR)) return -EINVAL;
  
  if (!filp->fs->ops->readdir) return -ENOSYS;
  if (lock_fs(filp->fs, FSOP_READDIR) < 0) return -ETIMEOUT;
  rc = filp->fs->ops->readdir(filp, dirp, count);
  unlock_fs(filp->fs, FSOP_READDIR);
  return rc;
}
