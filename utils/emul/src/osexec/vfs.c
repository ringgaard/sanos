#include "types.h"
#include <vfs.h>

struct filesystem *fslist = NULL;
struct fs *mountlist = NULL;

int canonicalize(char *path, char *buffer)
{
  char *p;
  int len;

  // Check for maximum filename length
  if (!path) return -1;
  if (strlen(path) >= MAXPATH) return -1;

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
      if (*path > 0 && *path < ' ') return -1;
      
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
      if (p < buffer) return -1;
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
    q = fs->path;
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

int format(devno_t devno, char *type, char *opts)
{
  struct filesystem *fsys;
  int rc;

  // Find file system type
  fsys = fslist;
  while (fsys)
  {
    if (strcmp(type, fsys->name) == 0) break;
    fsys = fsys->next;
  }
  if (!fsys) return -1;

  // Format device for filesystem
  if (!fsys->ops->format) return -1;
  rc = fsys->ops->format(devno, opts);
  return rc;
}

int mount(char *type, char *path, devno_t devno, char *opts)
{
  struct filesystem *fsys;
  struct fs *fs;
  int rc;

  // Check for file system already mounted
  fs = mountlist;
  while (fs)
  {
    if (fnmatch(path, strlen(path), fs->path, strlen(fs->path))) return -1;
    fs = fs->next;
  }

  // Find file system type
  fsys = fslist;
  while (fsys)
  {
    if (strcmp(type, fsys->name) == 0) break;
    fsys = fsys->next;
  }
  if (!fsys) return -1;

  // Allocate new mount point for file system
  fs = (struct fs *) kmalloc(sizeof(struct fs));
  if (!fs) return -1;
  memset(fs, 0, sizeof(struct fs));

  fs->devno = devno;
  strcpy(fs->path, path);
  fs->ops = fsys->ops;
  fs->next = mountlist;

  // Initialize filesystem on device
  rc = fs->ops->mount(fs, opts);
  if (rc != 0)
  {
    kfree(fs);
    return rc;
  }

  mountlist = fs;
  return 0;
}

int unmount_all()
{
  struct fs *fs;
  struct fs *nextfs;

  fs = mountlist;
  while (fs)
  {
    if (fs->ops->unmount) fs->ops->unmount(fs);
    nextfs = fs->next;
    kfree(fs);
    fs = nextfs;
  }

  mountlist = NULL;
  return 0;
}

struct file *open(char *name, int mode)
{
  struct fs *fs;
  struct file *filp;
  int rc;
  char *rest;
  char path[MAXPATH];

  if (canonicalize(name, path) < 0) return NULL;

  fs = fslookup(path, &rest);
  if (!fs) return NULL;
  if (!fs->ops->open) return NULL;

  filp = (struct file *) kmalloc(sizeof(struct file));
  if (!filp) return NULL;
  
  filp->fs = fs;
  filp->flags = mode;
  filp->pos = 0;
  filp->data = NULL;

  rc = fs->ops->open(filp, rest);
  if (rc != 0)
  {
    kfree(filp);
    return NULL;
  }

  return filp;
}

int close(struct file *filp)
{
  int rc;

  if (!filp) return -1;
  if (!filp->fs->ops->close) return -1;
  rc = filp->fs->ops->close(filp);
  kfree(filp);
  return rc;
}

int flush(struct file *filp)
{
  if (!filp) return -1;

  if (!filp->fs->ops->flush) return -1;
  return filp->fs->ops->flush(filp);
}

int read(struct file *filp, void *data, size_t size)
{
  if (!filp) return -1;
  if (!data && size > 0) return -1;
  if (filp->flags & O_WRONLY) return -1;
  
  if (!filp->fs->ops->read) return -1;
  return filp->fs->ops->read(filp, data, size);
}

int write(struct file *filp, void *data, size_t size)
{
  if (!filp) return -1;
  if (!data && size > 0) return -1;
  if (filp->flags & O_RDONLY) return -1;

  if (!filp->fs->ops->write) return -1;
  return filp->fs->ops->write(filp, data, size);
}

loff_t tell(struct file *filp)
{
  if (!filp) return -1;
 
  if (!filp->fs->ops->tell) return -1;
  return filp->fs->ops->tell(filp);
}

loff_t lseek(struct file *filp, loff_t offset, int origin)
{
  if (!filp) return -1;

  if (!filp->fs->ops->lseek) return -1;
  return filp->fs->ops->lseek(filp, offset, origin);
}

int chsize(struct file *filp, loff_t size)
{
  if (!filp) return -1;
  if (filp->flags & O_RDONLY) return -1;

  if (!filp->fs->ops->chsize) return -1;
  return filp->fs->ops->chsize(filp, size);
}

int fstat(struct file *filp, struct stat *buffer)
{
  if (!filp) return -1;
 
  if (!filp->fs->ops->fstat) return -1;
  return filp->fs->ops->fstat(filp, buffer);
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
  if (!fs) return -1;
  
  if (!fs->ops->stat) return -1;
  return fs->ops->stat(fs, rest, buffer);
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
  if (!fs) return -1;

  if (!fs->ops->mkdir) return -1;
  return fs->ops->mkdir(fs, rest);
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
  if (!fs) return -1;
  
  if (!fs->ops->rmdir) return -1;
  return fs->ops->rmdir(fs, rest);
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
  if (!oldfs) return -1;

  rc = canonicalize(newname, newpath);
  if (rc < 0) return rc;

  newfs = fslookup(newpath, &newrest);
  if (!newfs) return -1;

  if (oldfs != newfs) return -1;

  if (!oldfs->ops->rename) return -1;
  return oldfs->ops->rename(oldfs, oldrest, newrest);
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
  if (!oldfs) return -1;

  rc = canonicalize(newname, newpath);
  if (rc < 0) return rc;

  newfs = fslookup(newpath, &newrest);
  if (!newfs) return -1;

  if (oldfs != newfs) return -1;

  if (!oldfs->ops->link) return -1;
  return oldfs->ops->link(oldfs, oldrest, newrest);
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
  if (!fs) return -1;

  if (!fs->ops->unlink) return -1;
  return fs->ops->unlink(fs, rest);
}

struct file *opendir(char *name)
{
  struct fs *fs;
  struct file *filp;
  int rc;
  char *rest;
  char path[MAXPATH];

  if (canonicalize(name, path) < 0) return NULL;

  fs = fslookup(path, &rest);
  if (!fs) return NULL;
  if (!fs->ops->open) return NULL;

  filp = (struct file *) kmalloc(sizeof(struct file));
  if (!filp) return NULL;
  
  filp->fs = fs;
  filp->flags = O_RDONLY | F_DIR;
  filp->pos = 0;
  filp->data = NULL;

  rc = fs->ops->opendir(filp, rest);
  if (rc != 0)
  {
    kfree(filp);
    return NULL;
  }

  return filp;
}

int readdir(struct file *filp, struct dirent *dirp, int count)
{
  if (!filp) return -1;
  if (!dirp) return -1;
  if (!(filp->flags & F_DIR)) return -1;
  
  if (!filp->fs->ops->readdir) return -1;
  return filp->fs->ops->readdir(filp, dirp, count);
}

