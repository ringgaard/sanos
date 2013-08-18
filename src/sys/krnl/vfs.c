//
// vfs.c
//
// Virtual filesystem
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

struct filesystem *fslist = NULL;
struct fs *mountlist = NULL;
char pathsep = '/';

#define LFBUFSIZ 1025
#define CR '\r'
#define LF '\n'

int canonicalize(char *path, char *buffer) {
  char *p;
  char *end;
  int len;

  // Check arguments
  if (!path) return -EINVAL;

  // Remove drive letter from filename (e.g. c:)
  if (path[0] != 0 && path[1] == ':') path += 2;

  // Initialize buffer
  p = buffer;
  end = buffer + MAXPATH;

  // Add current directory to filename if relative path
  if (*path != PS1 && *path != PS2) {
    struct thread *t = self();

    // Do not add current directory if it is root directory
    len = strlen(t->curdir);
    if (len > 1) {
      memcpy(p, t->curdir, len);
      p += len;
    }
  }

  while (*path) {
    // Parse path separator
    if (*path == PS1 || *path == PS2) path++;
    if (p == end) return -ENAMETOOLONG;
    *p++ = pathsep;

    // Parse next name part in path
    len = 0;
    while (*path && *path != PS1 && *path != PS2) {
      // We do not allow control characters in filenames
      if (*path > 0 && *path < ' ') return -EINVAL;
      
      if (p == end) return -ENAMETOOLONG;
      *p++ = *path++;
      len++;
    }

    // Handle empty name parts and '.' and '..'
    if (len == 0) {
      p--;
    } else if (len == 1 && path[-1] == '.') {
      p -= 2;
    } else if (len == 2 && path[-1] == '.' && path[-2] == '.') {
      p -= 4;
      if (p < buffer) return -EINVAL;
      while (*p != pathsep) p--;
    }
  }

  // Convert empty filename to /
  if (p == buffer) *p++ = pathsep;

  // Terminate string
  if (p == end) return -ENAMETOOLONG;
  *p = 0;

  return p - buffer;
}

int fnmatch(char *fn1, int len1, char *fn2, int len2) {
  if (len1 != len2) return 0;
  while (len1--) if (*fn1++ != *fn2++) return 0;
  return 1;
}

int fslookup(char *name, int full, struct fs **mntfs, char **rest) {
  struct fs *fs;
  char *p;
  char *q;
  int n;
  int m;
  int rc;

  // Remove initial path separator from filename
  if (!name) return -EINVAL;
  p = name;
  if (*p == PS1 || *p == PS2) p++;
  n = strlen(p);

  // Find matching filesystem in mount list
  fs = mountlist;
  while (fs) {
    q = fs->mntto;
    if (*q) {
      if (*q == PS1 || *q == PS2) q++;

      if (!*q) {
        rc = check(fs->mode, fs->uid, fs->gid, S_IEXEC);
        if (rc < 0) return rc;

        if (rest) *rest = p;
        *mntfs = fs;

        return 0;
      }

      m = strlen(q);
      if (n >= m && fnmatch(p, m, q, m) && (p[m] == PS1 || p[m] == PS2 || full && p[m] == 0)) {
        rc = check(fs->mode, fs->uid, fs->gid, S_IEXEC);
        if (rc < 0) return rc;

        if (rest) *rest = p + m;
        *mntfs = fs;

        return 0;
      }
    }

    fs = fs->next;
  }

  return -ENOENT;
}

int __inline lock_fs(struct fs *fs, int fsop) {
  if (fs->ops->reentrant & fsop) return 0;
  if (fs->ops->lockfs) {
    return fs->ops->lockfs(fs);
  } else {
    return wait_for_object(&fs->exclusive, VFS_LOCK_TIMEOUT);
  }
}

void __inline unlock_fs(struct fs *fs, int fsop) {
  if (fs->ops->reentrant & fsop) return;
  if (fs->ops->unlockfs) {
    fs->ops->unlockfs(fs);
  } else {
    release_mutex(&fs->exclusive);
  }
}

static int files_proc(struct proc_file *pf, void *arg) {
  int h;
  struct object *o;
  struct file *filp;
  char perm[11];

  pprintf(pf, "handle    flags mode       uid gid type     path\n");
  pprintf(pf, "------ -------- ---------- --- --- -------- ----------------------------------\n");
  for (h = 0; h < htabsize; h++) {
    if (!HUSED(htab[h])) continue;
    o = HOBJ(htab[h]);
    if (o->type != OBJECT_FILE) continue;

    filp = (struct file *) o;

    strcpy(perm, " ---------");
    switch (filp->mode & S_IFMT) {
      case S_IFREG: perm[0] = '-'; break;
      case S_IFLNK: perm[0] = 'l'; break;
      case S_IFDIR: perm[0] = 'd'; break;
      case S_IFBLK: perm[0] = 'b'; break;
      case S_IFCHR: perm[0] = 'c'; break;
      case S_IFPKT: perm[0] = 'p'; break;
    }

    if (filp->mode & 0400) perm[1] = 'r';
    if (filp->mode & 0200) perm[2] = 'w';
    if (filp->mode & 0100) perm[3] = 'x';
    if (filp->mode & 0040) perm[4] = 'r';
    if (filp->mode & 0020) perm[5] = 'w';
    if (filp->mode & 0010) perm[6] = 'x';
    if (filp->mode & 0004) perm[7] = 'r';
    if (filp->mode & 0002) perm[8] = 'w';
    if (filp->mode & 0001) perm[9] = 'x';

    pprintf(pf, "%6d %08X %s %3d %3d %-8s %s\n", h, filp->flags, perm, filp->owner, filp->group, filp->fs->fsys->name, filp->path ? filp->path : "<no name>");
  }

  return 0;
}

int init_vfs() {
  pathsep = PS1;
  self()->curdir[0] = PS1;
  self()->curdir[1] = 0;

  if (!peb) panic("peb not initialized in vfs");
  peb->pathsep = pathsep;
  register_proc_inode("files", files_proc, NULL);
  return 0;
}

struct filesystem *register_filesystem(char *name, struct fsops *ops) {
  struct filesystem *fsys;

  fsys = (struct filesystem *) kmalloc(sizeof(struct filesystem));
  if (!fsys) return NULL;

  fsys->name = name;
  fsys->ops = ops;
  fsys->next = fslist;
  fslist = fsys;

  return fsys;
}

struct file *newfile(struct fs *fs, char *path, int flags, int mode) {
  struct thread *thread = self();
  struct file *filp;
  int umaskval = 0;
  int fmodeval = 0;

  if (peb) {
    umaskval = peb->umaskval;
    fmodeval = peb->fmodeval;
  }

  filp = (struct file *) kmalloc(sizeof(struct file));
  if (!filp) return NULL;
  init_ioobject(&filp->iob, OBJECT_FILE);
  
  if ((flags & (O_TEXT | O_BINARY)) == 0) flags |= fmodeval;

  filp->fs = fs;
  filp->flags = flags;
  filp->mode = mode & ~umaskval;
  filp->owner = thread->euid; 
  filp->group = thread->egid;
  filp->pos = 0;
  filp->data = NULL;
  filp->path = strdup(path);
  filp->chbuf = LF;

  return filp;
}

int mkfs(char *devname, char *type, char *opts) {
  struct filesystem *fsys;
  int rc;

  // Check arguments
  if (!type) return -EINVAL;

  // Find file system type
  fsys = fslist;
  while (fsys) {
    if (strcmp(type, fsys->name) == 0) break;
    fsys = fsys->next;
  }
  if (!fsys) return -EINVAL;

  // Format device for filesystem
  if (!fsys->ops->mkfs) return -ENODEV;
  rc = fsys->ops->mkfs(devname, opts);
  return rc;
}

int mount(char *type, char *mntto, char *mntfrom, char *opts, struct fs **newfs) {
  struct filesystem *fsys;
  struct fs *fs;
  struct fs *prevfs;
  int rc;
  char path[MAXPATH];
  struct stat64 st;
  
  // Check parameters
  if (!type) return -EINVAL;
  if (!mntto) return -EINVAL;

  if (*mntto) {
    rc = canonicalize(mntto, path);
    if (rc < 0) return rc;
  } else {
    *path = 0;
  }

  // Check for file system already mounted
  fs = mountlist;
  prevfs = NULL;
  while (fs) {
    if (fnmatch(path, strlen(path), fs->mntto, strlen(fs->mntto))) return -EEXIST;
    if (strlen(path) < strlen(fs->mntto)) prevfs = fs;
    fs = fs->next;
  }

  // Check that mount point exists
  if (path[0] == 0 || (path[0] == PS1 || path[0] == PS2) &&  path[1] == 0) {
    st.st_uid = 0;
    st.st_gid = 0;
    st.st_mode = 0755;
  } else {
    rc = stat(path, &st);
    if (rc < 0) return rc;
  }

  // Find file system type
  fsys = fslist;
  while (fsys) {
    if (strcmp(type, fsys->name) == 0) break;
    fsys = fsys->next;
  }
  if (!fsys) return -EINVAL;

  // Allocate new mount point for file system
  fs = (struct fs *) kmalloc(sizeof(struct fs));
  if (!fs) return -ENOMEM;
  memset(fs, 0, sizeof(struct fs));

  strcpy(fs->mntto, path);
  strcpy(fs->mntfrom, mntfrom ? mntfrom : "");

  fs->fsys = fsys;
  fs->ops = fsys->ops;

  fs->uid = st.st_uid;
  fs->gid = st.st_gid;
  fs->mode = st.st_mode;

  init_mutex(&fs->exclusive, 0);

  // Initialize filesystem on device
  if (fs->ops->mount) {
    rc = fs->ops->mount(fs, opts);
    if (rc != 0) {
      kfree(fs);
      return rc;
    }
  }

  // Insert in mount list
  if (prevfs) {
    fs->next = prevfs->next;
    fs->prev = prevfs;

    if (prevfs->next) prevfs->next->prev = fs;
    prevfs->next = fs;
  } else {
    fs->prev = NULL;
    fs->next = mountlist;
    if (mountlist) mountlist->prev = fs;
    mountlist = fs;
  }

  if (newfs) *newfs = fs;
  return 0;
}

int umount(char *name) {
  struct fs *fs;
  int rc;
  char path[MAXPATH];

  // Check parameters
  if (!name) return -EINVAL;

  rc = canonicalize(name, path);
  if (rc < 0) return rc;

  // Find mounted filesystem
  fs = mountlist;
  while (fs) {
    if (fnmatch(path, strlen(path), fs->mntto, strlen(fs->mntto))) break;
    fs = fs->next;
  }
  if (!fs) return -ENOENT;

  // Check for locks
  if (fs->locks > 0) return -EBUSY;

  // Unmount filesystem from device
  if (fs->ops->umount) {
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

int umount_all() {
  struct fs *fs;
  struct fs *nextfs;

  fs = mountlist;
  while (fs) {
    if (fs->ops->umount) fs->ops->umount(fs);
    nextfs = fs->next;
    kfree(fs);
    fs = nextfs;
  }

  mountlist = NULL;
  return 0;
}

static int get_fsstat(struct fs *fs, struct statfs *buf) {
  int rc;

  strcpy(buf->fstype, fs->fsys->name);
  strcpy(buf->mntto, fs->mntto);
  strcpy(buf->mntfrom, fs->mntfrom);

  if (fs->ops->statfs) {
    if (lock_fs(fs, FSOP_STATFS) < 0) return -ETIMEOUT;
    rc = fs->ops->statfs(fs, buf);
    unlock_fs(fs, FSOP_STATFS);
    if (rc < 0) return rc;
  } else {
    buf->bsize = -1;
    buf->iosize = -1;
    buf->blocks = -1;
    buf->bfree = -1;
    buf->files = -1;
    buf->ffree = -1;
  }

  return 0;
}

int getfsstat(struct statfs *buf, size_t size) {
  int count;
  struct fs *fs;
  int rc;

  count = 0;
  fs = mountlist;
  while (fs) {
    if (buf) {
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

int fstatfs(struct file *filp, struct statfs *buf) {
  if (!filp) return -EINVAL;
  if (!buf) return -EINVAL;

  return get_fsstat(filp->fs, buf);
}

int statfs(char *name, struct statfs *buf) {
  struct fs *fs;
  char *rest;
  int rc;
  char path[MAXPATH];

  rc = canonicalize(name, path);
  if (rc < 0) return rc;

  rc = fslookup(path, 1, &fs, &rest);
  if (rc < 0) return rc;

  return get_fsstat(fs, buf);
}

int open(char *name, int flags, int mode, struct file **retval) {
  struct fs *fs;
  struct file *filp;
  int rc;
  char *rest;
  char path[MAXPATH];

  rc = canonicalize(name, path);
  if (rc < 0) return rc;

  rc = fslookup(path, 0, &fs, &rest);
  if (rc < 0) return rc;

  filp = newfile(fs, path, flags, mode);
  if (!filp) return -EMFILE;

  if (fs->ops->open) {
    fs->locks++;
    if (lock_fs(fs, FSOP_OPEN) < 0)  {
      fs->locks--;
      kfree(filp->path);
      kfree(filp);
      return -ETIMEOUT;
    }

    rc = fs->ops->open(filp, rest);
    if (rc == 0) {
      int access;

      if (filp->flags & O_RDWR) {
        access = S_IREAD | S_IWRITE;
      } else if (filp->flags & O_WRONLY) {
        access = S_IWRITE;
      } else {
        access = S_IREAD;
      }

      rc = check(filp->mode, filp->owner, filp->group, access);
    }

    unlock_fs(fs, FSOP_OPEN);

    if (rc != 0) {
      fs->locks--;
      kfree(filp->path);
      kfree(filp);
      return rc;
    }
  }

  *retval = filp;
  return 0;
}

int close(struct file *filp) {
  int rc;

  if (!filp) return -EINVAL;
  
  if (filp->fs->ops->close) {
    if (lock_fs(filp->fs, FSOP_CLOSE) < 0) return -ETIMEOUT;
    rc = filp->fs->ops->close(filp);
    unlock_fs(filp->fs, FSOP_CLOSE);
  } else {
    rc = 0;
  }

  if (rc == 0) filp->fs->locks--;
  
  detach_ioobject(&filp->iob);
  kfree(filp->path);
  filp->path = NULL;
  filp->flags |= F_CLOSED;

  return rc;
}

int destroy(struct file *filp) {
  int rc;

  if (!filp) return -EINVAL;

  if (filp->fs->ops->destroy) {
    rc = filp->fs->ops->destroy(filp);
  } else {
    rc = 0;
  }

  kfree(filp);
  return rc;
}

int fsync(struct file *filp) {
  int rc;

  if (!filp) return -EINVAL;

  if (!filp->fs->ops->fsync) return -ENOSYS;

  if (lock_fs(filp->fs, FSOP_FSYNC) < 0) return -ETIMEOUT;
  rc = filp->fs->ops->fsync(filp);
  unlock_fs(filp->fs, FSOP_FSYNC);

  return rc;
}

int setmode(struct file *filp, int mode) {
  int oldmode;

  if (mode & ~(O_TEXT | O_BINARY | O_APPEND | O_NOINHERIT)) return -EINVAL;
  oldmode = filp->mode & (O_TEXT | O_BINARY | O_APPEND | O_NOINHERIT);
  filp->mode = (filp->mode & ~(O_TEXT | O_BINARY | O_APPEND | O_NOINHERIT)) | mode;
  return oldmode;
}

int read_translated(struct file *filp, void *data, size_t size) {
  char *buf = (char *) data;
  char *p = buf;
  char *q = buf;
  int bytes = 0;
  int rc;
  char peekch;

  if (size == 0) return 0;
  p = q = buf;

  // Read data including lookahead char if present
  if (filp->chbuf != LF) {
    buf[0] = filp->chbuf;
    filp->chbuf = LF;
    if (size == 1) return 1;

    rc = filp->fs->ops->read(filp, buf + 1, size - 1, filp->pos);
    if (rc > 0) filp->pos += rc;
    if (rc < 0) return rc;
    bytes = rc + 1;
  } else {
    rc = filp->fs->ops->read(filp, buf, size, filp->pos);
    if (rc > 0) filp->pos += rc;
    if (rc < 0) return rc;
    bytes = rc;
  }

  // Translate CR/LF to LF in the buffer
  while (p < buf + bytes)  {
    if (*p != CR) {
      *q++ = *p++;
    } else {
      // *p is CR, so must check next char for LF
      if (p < buf + bytes - 1) {
        if (*(p + 1) == LF) {
          // convert CR/LF to LF
          p += 2;
          *q++ = LF;
        } else {
          *q++ = *p++;
        }
      } else {
        // We found a CR at end of buffer. 
        // We must peek ahead to see if next char is an LF.
        p++;

        rc = filp->fs->ops->read(filp, &peekch, 1, filp->pos);
        if (rc > 0) filp->pos += rc;
        if (rc <= 0) {
          // Couldn't read ahead, store CR
          *q++ = CR;
        } else {
          // peekch now has the extra character. If char is LF store LF
          // else store CR and put char in lookahead buffer.
          if (peekch == LF) {
            *q++ = LF;
          } else {
            *q++ = CR;
            filp->chbuf = peekch;
          }
        }
      }
    }
  }

  // Return number of bytes in buffer
  return q - buf;
}

int read(struct file *filp, void *data, size_t size) {
  int rc;

  if (!filp) return -EINVAL;
  if (!data && size > 0) return -EINVAL;
  if (filp->flags & O_WRONLY) return -EACCES;
  
  if (!filp->fs->ops->read) return -ENOSYS;
  if (lock_fs(filp->fs, FSOP_READ) < 0) return -ETIMEOUT;
  if (filp->flags & O_TEXT) {
    rc = read_translated(filp, data, size);
  } else {
    rc = filp->fs->ops->read(filp, data, size, filp->pos);
    if (rc > 0) filp->pos += rc;
  }
  unlock_fs(filp->fs, FSOP_READ);
  return rc;
}

int pread(struct file *filp, void *data, size_t size, off64_t offset) {
  int rc;

  if (!filp) return -EINVAL;
  if (!data && size > 0 || offset < 0) return -EINVAL;
  if (filp->flags & O_WRONLY) return -EACCES;
  if (filp->flags & O_TEXT) return -ENXIO;
  
  if (!filp->fs->ops->read) return -ENOSYS;
  if (lock_fs(filp->fs, FSOP_READ) < 0) return -ETIMEOUT;
  rc = filp->fs->ops->read(filp, data, size, offset);
  unlock_fs(filp->fs, FSOP_READ);
  return rc;
}

static int write_translated(struct file *filp, void *data, size_t size) {
  char *buf;
  char *p, *q;
  int rc;
  int lfcnt;
  int bytes;
  char lfbuf[LFBUFSIZ];

  // Translate LF to CR/LF on output
  buf = (char *) data;
  p = buf;
  bytes = lfcnt = 0;

  while ((unsigned) (p - buf) < size) {
    // Fill the buffer, except maybe last char
    q = lfbuf;
    while ((unsigned) (q - lfbuf) < LFBUFSIZ - 1 && (unsigned) (p - buf) < size) {
      char ch = *p++;
      if (ch == LF) {
        lfcnt++;
        *q++ = CR;
      }
      *q++ = ch;
    }

    // Write the buffer and update total
    rc = filp->fs->ops->write(filp, lfbuf, q - lfbuf, filp->pos);
    if (rc > 0) filp->pos += rc;
    if (rc < 0) return rc;
    bytes += rc;
    if (rc < q - lfbuf) break;
  }

  return bytes - lfcnt;
}

int write(struct file *filp, void *data, size_t size) {
  int rc;

  if (!filp) return -EINVAL;
  if (!data && size > 0) return -EINVAL;
  if (filp->flags == O_RDONLY) return -EACCES;

  if (!filp->fs->ops->write) return -ENOSYS;
  if (lock_fs(filp->fs, FSOP_WRITE) < 0) return -ETIMEOUT;
  if (filp->flags & O_TEXT) {
    rc = write_translated(filp, data, size);
  } else {
    rc = filp->fs->ops->write(filp, data, size, filp->pos);
    if (rc > 0) filp->pos += rc;
  }
  unlock_fs(filp->fs, FSOP_WRITE);
  return rc;
}

int pwrite(struct file *filp, void *data, size_t size, off64_t offset) {
  int rc;

  if (!filp) return -EINVAL;
  if (!data && size > 0 || offset < 0) return -EINVAL;
  if (filp->flags == O_RDONLY) return -EACCES;
  if (filp->flags & O_TEXT) return -ENXIO;

  if (!filp->fs->ops->write) return -ENOSYS;
  if (lock_fs(filp->fs, FSOP_WRITE) < 0) return -ETIMEOUT;
  rc = filp->fs->ops->write(filp, data, size, offset);
  unlock_fs(filp->fs, FSOP_WRITE);
  return rc;
}

int ioctl(struct file *filp, int cmd, void *data, size_t size) {
  int rc;

  if (!filp) return -EINVAL;
  if (!data && size > 0) return -EINVAL;

  if (cmd == FIONBIO) {
    if (size != sizeof(int)) return -EINVAL;
    if (*(int *) data) {
      filp->flags |= O_NONBLOCK;
    } else {
      filp->flags &= ~O_NONBLOCK;
    }
  } else if (cmd == IOCTL_SET_TTY) {
    if (size != sizeof(int)) return -EINVAL;
    if (*(int *) data) {
      filp->flags |= F_TTY;
    } else {
      filp->flags &= ~F_TTY;
    }
    return 0;
  } else if (cmd == IOCTL_GET_TTY) {
    if (size != sizeof(int)) return -EINVAL;
    *(int *) data = filp->flags & F_TTY ? 1 : 0;
    return 0;
  }

  if (!filp->fs->ops->ioctl) return -ENOSYS;
  if (lock_fs(filp->fs, FSOP_IOCTL) < 0) return -ETIMEOUT;
  rc = filp->fs->ops->ioctl(filp, cmd, data, size);
  unlock_fs(filp->fs, FSOP_IOCTL);
  return rc;
}

int readv(struct file *filp, struct iovec *iov, int count) {
  int i;
  int rc;
  int total = 0;

  for (i = 0; i < count; i++) {
    rc = read(filp, iov[i].iov_base, iov[i].iov_len);
    if (rc < 0) return rc;

    total += rc;
    if (rc < (int) iov[i].iov_len) return total;
  }

  return total;
}

int writev(struct file *filp, struct iovec *iov, int count) {
  int i;
  int rc;
  int total = 0;

  for (i = 0; i < count; i++) {
    rc = write(filp, iov[i].iov_base, iov[i].iov_len);
    if (rc < 0) return rc;

    total += rc;
    if (rc < (int) iov[i].iov_len) return total;
  }

  return total;
}

off64_t tell(struct file *filp) {
  off64_t rc;

  if (!filp) return -EINVAL;
 
  if (!filp->fs->ops->tell) return -ENOSYS;
  if (lock_fs(filp->fs, FSOP_TELL) < 0) return -ETIMEOUT;
  rc =  filp->fs->ops->tell(filp);
  unlock_fs(filp->fs, FSOP_TELL);
  return rc;
}

off64_t lseek(struct file *filp, off64_t offset, int origin) {
  off64_t rc;

  if (!filp) return -EINVAL;

  if (!filp->fs->ops->lseek) return -ENOSYS;
  if (lock_fs(filp->fs, FSOP_LSEEK) < 0) return -ETIMEOUT;
  rc = filp->fs->ops->lseek(filp, offset, origin);
  unlock_fs(filp->fs, FSOP_LSEEK);
  return rc;
}

int ftruncate(struct file *filp, off64_t size) {
  int rc;

  if (!filp) return -EINVAL;
  if (filp->flags & O_RDONLY) return -EACCES;

  if (!filp->fs->ops->ftruncate) return -ENOSYS;
  if (lock_fs(filp->fs, FSOP_FTRUNCATE) < 0) return -ETIMEOUT;
  rc = filp->fs->ops->ftruncate(filp, size);
  unlock_fs(filp->fs, FSOP_FTRUNCATE);
  return rc;
}

int futime(struct file *filp, struct utimbuf *times) {
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

int utime(char *name, struct utimbuf *times) {
  struct fs *fs;
  char *rest;
  int rc;
  char path[MAXPATH];

  rc = canonicalize(name, path);
  if (rc < 0) return rc;

  rc = fslookup(path, 0, &fs, &rest);
  if (rc < 0) return rc;
  
  if (!times) return -EINVAL;

  if (!fs->ops->utime) return -ENOSYS;
  fs->locks++;
  if (lock_fs(fs, FSOP_UTIME) < 0) {
    fs->locks--;
    return -ETIMEOUT;
  }
  rc = fs->ops->utime(fs, rest, times);
  unlock_fs(fs, FSOP_UTIME);
  fs->locks--;
  return rc;
}

int fstat(struct file *filp, struct stat64 *buffer) {
  int rc;

  if (!filp) return -EINVAL;
 
  if (!filp->fs->ops->fstat) return -ENOSYS;
  if (lock_fs(filp->fs, FSOP_FSTAT) < 0) return -ETIMEOUT;
  rc = filp->fs->ops->fstat(filp, buffer);
  unlock_fs(filp->fs, FSOP_FSTAT);
  return rc;  
}

int stat(char *name, struct stat64 *buffer) {
  struct fs *fs;
  char *rest;
  int rc;
  char path[MAXPATH];

  rc = canonicalize(name, path);
  if (rc < 0) return rc;

  rc = fslookup(path, 0, &fs, &rest);
  if (rc < 0) return rc;

  if (!fs->ops->stat) return -ENOSYS;
  fs->locks++;
  if (lock_fs(fs, FSOP_STAT) < 0) {
    fs->locks--;
    return -ETIMEOUT;
  }
  rc = fs->ops->stat(fs, rest, buffer);
  unlock_fs(fs, FSOP_STAT);
  fs->locks--;
  return rc;
}

int access(char *name, int mode) {
  struct fs *fs;
  char *rest;
  int rc;
  char path[MAXPATH];

  rc = canonicalize(name, path);
  if (rc < 0) return rc;

  rc = fslookup(path, 0, &fs, &rest);
  if (rc < 0) return rc;

  if (!fs->ops->access) {
    struct thread *thread = self();
    struct stat64 buf;

    rc = stat(name, &buf);
    if (rc < 0) return rc;
    if (mode == 0) return 0;

    if (thread->euid == 0) {
      if (mode == X_OK) {
        return buf.st_mode & 0111 ? 0 : -EACCES;
      } else {
        return 0;
      }
    }

    if (thread->euid == buf.st_uid) {
      mode <<= 6;
    } else if (thread->egid == buf.st_gid) {
      mode <<= 3;
    }

    if ((mode && buf.st_mode) == 0) return -EACCES;
    return 0;
  }

  fs->locks++;
  if (lock_fs(fs, FSOP_ACCESS) < 0) {
    fs->locks--;
    return -ETIMEOUT;
  }
  rc = fs->ops->access(fs, rest, mode);
  unlock_fs(fs, FSOP_ACCESS);
  fs->locks--;
  return rc;
}

int fchmod(struct file *filp, int mode) {
  int rc;

  if (!filp) return -EINVAL;
  if (filp->flags & O_RDONLY) return -EACCES;
 
  if (!filp->fs->ops->fchmod) return -ENOSYS;
  if (lock_fs(filp->fs, FSOP_FCHMOD) < 0) return -ETIMEOUT;
  rc = filp->fs->ops->fchmod(filp, mode);
  unlock_fs(filp->fs, FSOP_FCHMOD);
  return rc;
}

int chmod(char *name, int mode) {
  struct fs *fs;
  char *rest;
  int rc;
  char path[MAXPATH];

  rc = canonicalize(name, path);
  if (rc < 0) return rc;

  rc = fslookup(path, 0, &fs, &rest);
  if (rc < 0) return rc;

  if (!fs->ops->chmod) return -ENOSYS;
  fs->locks++;
  if (lock_fs(fs, FSOP_CHMOD) < 0) {
    fs->locks--;
    return -ETIMEOUT;
  }
  rc = fs->ops->chmod(fs, rest, mode);
  unlock_fs(fs, FSOP_CHMOD);
  fs->locks--;
  return rc;
}

int fchown(struct file *filp, int owner, int group) {
  int rc;

  if (!filp) return -EINVAL;
  if (filp->flags & O_RDONLY) return -EACCES;

  if (!filp->fs->ops->fchown) return -ENOSYS;
  if (lock_fs(filp->fs, FSOP_FCHOWN) < 0) return -ETIMEOUT;
  rc = filp->fs->ops->fchown(filp, owner, group);
  unlock_fs(filp->fs, FSOP_FCHOWN);
  return rc;
}

int chown(char *name, int owner, int group) {
  struct fs *fs;
  char *rest;
  int rc;
  char path[MAXPATH];

  rc = canonicalize(name, path);
  if (rc < 0) return rc;

  rc = fslookup(path, 0, &fs, &rest);
  if (rc < 0) return rc;

  if (!fs->ops->chmod) return -ENOSYS;
  fs->locks++;
  if (lock_fs(fs, FSOP_CHOWN) < 0) {
    fs->locks--;
    return -ETIMEOUT;
  }
  rc = fs->ops->chown(fs, rest, owner, group);
  unlock_fs(fs, FSOP_CHOWN);
  fs->locks--;
  return rc;
}

int chdir(char *name) {
  struct fs *fs;
  char *rest;
  int rc;
  char path[MAXPATH];
  char newdir[MAXPATH];
  struct stat64 buffer;

  rc = canonicalize(name, path);
  if (rc < 0) return rc;
  strcpy(newdir, path);

  rc = fslookup(path, 0, &fs, &rest);
  if (rc < 0) return rc;
  
  if (fs->ops->stat) {
    fs->locks++;
    if (lock_fs(fs, FSOP_STAT) < 0) {
      fs->locks--;
      return -ETIMEOUT;
    }
    rc = fs->ops->stat(fs, rest, &buffer);
    unlock_fs(fs, FSOP_STAT);
    fs->locks--;

    if (rc < 0) return rc;
    if ((buffer.st_mode & S_IFMT) != S_IFDIR) return -ENOTDIR;
  }

  strcpy(self()->curdir, newdir);

  return 0;
}

int getcwd(char *buf, size_t size) {
  if (!buf || size == 0) return -EINVAL;
  if (size <= strlen(self()->curdir)) return -ERANGE;
  strcpy(buf, self()->curdir);

  return 0;
}

int mkdir(char *name, int mode) {
  struct fs *fs;
  char *rest;
  int rc;
  char path[MAXPATH];

  rc = canonicalize(name, path);
  if (rc < 0) return rc;

  rc = fslookup(path, 0, &fs, &rest);
  if (rc < 0) return rc;

  if (!fs->ops->mkdir) return -ENOSYS;
  fs->locks++;
  if (lock_fs(fs, FSOP_MKDIR) < 0) {
    fs->locks--;
    return -ETIMEOUT;
  }
  rc = fs->ops->mkdir(fs, rest, mode & ~(peb ? peb->umaskval : 0));
  unlock_fs(fs, FSOP_MKDIR);
  fs->locks--;
  return rc;
}

int rmdir(char *name) {
  struct fs *fs;
  char *rest;
  int rc;
  char path[MAXPATH];

  rc = canonicalize(name, path);
  if (rc < 0) return rc;

  rc = fslookup(path, 0, &fs, &rest);
  if (rc < 0) return rc;
  
  if (!fs->ops->rmdir) return -ENOSYS;
  fs->locks++;
  if (lock_fs(fs, FSOP_RMDIR) < 0) {
    fs->locks--;
    return -ETIMEOUT;
  }
  rc = fs->ops->rmdir(fs, rest);
  unlock_fs(fs, FSOP_RMDIR);
  fs->locks--;
  return rc;
}

int rename(char *oldname, char *newname) {
  struct fs *oldfs;
  struct fs *newfs;
  char *oldrest;
  char *newrest;
  int rc;
  char oldpath[MAXPATH];
  char newpath[MAXPATH];

  rc = canonicalize(oldname, oldpath);
  if (rc < 0) return rc;

  rc = fslookup(oldpath, 0, &oldfs, &oldrest);
  if (rc < 0) return rc;

  rc = canonicalize(newname, newpath);
  if (rc < 0) return rc;

  rc = fslookup(newpath, 0, &newfs, &newrest);
  if (rc < 0) return rc;

  if (oldfs != newfs) return -EXDEV;

  if (!oldfs->ops->rename) return -ENOSYS;
  oldfs->locks++;
  if (lock_fs(oldfs, FSOP_RENAME) < 0) {
    oldfs->locks--;
    return -ETIMEOUT;
  }
  rc = oldfs->ops->rename(oldfs, oldrest, newrest);
  unlock_fs(oldfs, FSOP_RENAME);
  oldfs->locks--;
  return rc;
}

int link(char *oldname, char *newname) {
  struct fs *oldfs;
  struct fs *newfs;
  char *oldrest;
  char *newrest;
  int rc;
  char oldpath[MAXPATH];
  char newpath[MAXPATH];

  rc = canonicalize(oldname, oldpath);
  if (rc < 0) return rc;

  rc = fslookup(oldpath, 0, &oldfs, &oldrest);
  if (rc < 0) return rc;

  rc = canonicalize(newname, newpath);
  if (rc < 0) return rc;

  rc = fslookup(newpath, 0, &newfs, &newrest);
  if (rc < 0) return rc;

  if (oldfs != newfs) return -EXDEV;

  if (!oldfs->ops->link) return -ENOSYS;
  oldfs->locks++;
  if (lock_fs(oldfs, FSOP_LINK) < 0) {
    oldfs->locks--;
    return -ETIMEOUT;
  }
  rc = oldfs->ops->link(oldfs, oldrest, newrest);
  unlock_fs(oldfs, FSOP_LINK);
  oldfs->locks--;
  return rc;
}

int unlink(char *name) {
  struct fs *fs;
  char *rest;
  int rc;
  char path[MAXPATH];

  rc = canonicalize(name, path);
  if (rc < 0) return rc;

  rc = fslookup(path, 0, &fs, &rest);
  if (rc < 0) return rc;

  if (!fs->ops->unlink) return -ENOSYS;
  fs->locks++;
  if (lock_fs(fs, FSOP_UNLINK) < 0) {
    fs->locks--;
    return -ETIMEOUT;
  }
  rc = fs->ops->unlink(fs, rest);
  fs->locks--;
  unlock_fs(fs, FSOP_UNLINK);
  return rc;
}

int opendir(char *name, struct file **retval) {
  struct fs *fs;
  struct file *filp;
  int rc;
  char *rest;
  char path[MAXPATH];

  rc = canonicalize(name, path);
  if (rc < 0) return rc;

  rc = fslookup(path, 1, &fs, &rest);
  if (rc < 0) return rc;

  if (!fs->ops->opendir) return -ENOSYS;

  filp = (struct file *) kmalloc(sizeof(struct file));
  if (!filp) return -ENOMEM;
  init_ioobject(&filp->iob, OBJECT_FILE);
  
  filp->fs = fs;
  filp->flags = O_RDONLY | F_DIR;
  filp->pos = 0;
  filp->data = NULL;
  filp->path = strdup(path);

  fs->locks++;
  if (lock_fs(fs, FSOP_OPENDIR) < 0) {
    fs->locks--;
    kfree(filp->path);
    kfree(filp);
    return -ETIMEOUT;
  }
  rc = fs->ops->opendir(filp, rest);
  unlock_fs(fs, FSOP_OPENDIR);
  if (rc != 0) {
    fs->locks--;
    kfree(filp->path);
    kfree(filp);
    return rc;
  }

  *retval = filp;
  return 0;
}

int readdir(struct file *filp, struct direntry *dirp, int count) {
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
