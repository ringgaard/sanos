//
// file.c
//
// Disk filesystem file routines
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

static int open_existing(struct filsys *fs, char *name, struct inode **retval) {
  struct inode *inode;
  int rc;

  rc = namei(fs, name, &inode);
  if (rc < 0) return rc;

  *retval = inode;
  return 0;
}

static int open_always(struct filsys *fs, char *name, int mode, struct inode **retval) {
  struct inode *dir;
  struct inode *inode;
  ino_t ino;
  int rc;
  int len = strlen(name);

  rc = diri(fs, &name, &len, &dir);
  if (rc < 0) return rc;

  rc = find_dir_entry(dir, name, len, &ino);
  if (rc < 0 && rc != -ENOENT) {
    release_inode(dir);
    return rc;
  }

  if (rc == -ENOENT) {
    inode = alloc_inode(dir, S_IFREG | (mode & S_IRWXUGO));
    if (!inode) {
      release_inode(dir);
      return -ENOSPC;
    }

    rc = add_dir_entry(dir, name, len, inode->ino);
    if (rc < 0) {
      unlink_inode(inode);
      release_inode(inode);
      release_inode(dir);
      return rc;
    }
  } else {
    rc = get_inode(fs, ino, &inode);
    if (rc < 0) {
      release_inode(dir);
      return rc;
    }
  }

  release_inode(dir);
  *retval = inode;
  return 0;
}

static int create_always(struct filsys *fs, char *name, int mode, struct inode **retval) {
  struct inode *dir;
  struct inode *inode;
  struct inode *oldinode;
  ino_t oldino;
  int rc;
  int len = strlen(name);

  rc = diri(fs, &name, &len, &dir);
  if (rc < 0) return rc;

  rc = find_dir_entry(dir, name, len, &oldino);
  if (rc == 0) {
    rc = get_inode(fs, oldino, &oldinode);
    if (rc < 0) {
      release_inode(dir);
      return rc;
    }

    if (S_ISDIR(oldinode->desc->mode) && oldinode->desc->linkcount == 1) {
      release_inode(dir);
      return -EISDIR;
    }

    inode = alloc_inode(dir, S_IFREG | (mode & S_IRWXUGO));
    if (!inode) {
      release_inode(dir);
      return -ENOSPC;
    }

    rc = modify_dir_entry(dir, name, len, inode->ino, NULL);
    if (rc < 0) {
      unlink_inode(inode);
      release_inode(inode);
      release_inode(dir);
      return rc;
    }

    unlink_inode(oldinode);
    release_inode(oldinode);
  } else if (rc == -ENOENT) {
    inode = alloc_inode(dir, S_IFREG | (mode & S_IRWXUGO));
    if (!inode) {
      release_inode(dir);
      return -ENOSPC;
    }

    rc = add_dir_entry(dir, name, len, inode->ino);
    if (rc < 0) {
      unlink_inode(inode);
      release_inode(inode);
      release_inode(dir);
      return rc;
    }
  } else {
    release_inode(dir);
    return rc;
  }

  release_inode(dir);
  *retval = inode;
  return 0;
}

static int truncate_existing(struct filsys *fs, char *name, struct inode **retval) {
  struct inode *inode;
  int rc;

  rc = namei(fs, name, &inode);
  if (rc < 0) return rc;

  if (S_ISDIR(inode->desc->mode)) {
    release_inode(inode);
    return -EISDIR;
  }

  rc = truncate_inode(inode, 0); 
  if (rc < 0) {
    release_inode(inode);
    return rc;
  }
  inode->desc->size = 0;
  mark_inode_dirty(inode);

  *retval = inode;
  return 0;
}

static int create_new(struct filsys *fs, char *name, ino_t ino, int mode, struct inode **retval) {
  struct inode *dir;
  struct inode *inode;
  int rc;
  int len = strlen(name);

  rc = diri(fs, &name, &len, &dir);
  if (rc < 0) return rc;

  rc = find_dir_entry(dir, name, len, NULL);
  if (rc != -ENOENT) {
    release_inode(dir);
    return rc >= 0 ? -EEXIST : rc;
  }

  if (ino == NOINODE) {
    inode = alloc_inode(dir, S_IFREG | (mode & S_IRWXUGO));
    if (!inode) rc = -ENOSPC;
  } else {
    rc = get_inode(fs, ino, &inode);
    if (rc < 0) inode = NULL;
    if (inode) {
      inode->desc->ctime = inode->desc->mtime = time(NULL);
      inode->desc->uid = inode->desc->gid = 0;
      inode->desc->mode = S_IFREG | 0700;
      inode->desc->linkcount++;
      mark_inode_dirty(inode);
    }
  }

  if (!inode) {
    release_inode(dir);
    return rc;
  }

  rc = add_dir_entry(dir, name, len, inode->ino);
  if (rc < 0) {
    unlink_inode(inode);
    release_inode(inode);
    release_inode(dir);
    return rc;
  }

  release_inode(dir);
  *retval = inode;
  return 0;
}

int dfs_open(struct file *filp, char *name) {
  struct filsys *fs;
  struct inode *inode;
  int rc;

  fs = (struct filsys *) filp->fs->data;

  switch (filp->flags & (O_CREAT | O_EXCL | O_TRUNC | O_SPECIAL)) {
    case 0:
    case O_EXCL:
      // Open existing file
      rc = open_existing(fs, name, &inode);
      break;

    case O_CREAT:
      // Open file, create new file if it does not exists
      rc = open_always(fs, name, filp->mode, &inode);
      break;

    case O_CREAT | O_EXCL:
    case O_CREAT | O_TRUNC | O_EXCL:
      // Create new file, fail if it exists
      rc = create_new(fs, name, NOINODE, filp->mode, &inode);
      filp->flags |= F_MODIFIED;
      break;

    case O_TRUNC:
    case O_TRUNC | O_EXCL:
      // Open and truncate existing file
      rc = truncate_existing(fs, name, &inode);
      filp->flags |= F_MODIFIED;
      break;

    case O_CREAT | O_TRUNC:
      // Create new file, unlink existing file if it exists
      rc = create_always(fs, name, filp->mode, &inode);
      filp->flags |= F_MODIFIED;
      break;

    case O_SPECIAL:
      // Create new file with special inode number
      rc = create_new(fs, name, filp->flags >> 24, filp->mode, &inode);
      filp->flags |= F_MODIFIED;
      break;

    default:
      return -EINVAL;
  }

  if (rc < 0) return rc;

  if (filp->flags & O_APPEND) filp->pos = inode->desc->size;

  filp->data = inode;
  filp->mode = inode->desc->mode;
  filp->owner = inode->desc->uid;
  filp->group = inode->desc->gid;

  return 0;
}

int dfs_close(struct file *filp) {
  struct inode *inode;

  inode = (struct inode *) filp->data;
  if (filp->flags & F_MODIFIED) {
    inode->desc->mtime = time(NULL);
    mark_inode_dirty(inode);
  }

  if (filp->flags & O_TEMPORARY) unlink(filp->path);

  return 0;
}

int dfs_destroy(struct file *filp) {
  struct inode *inode;

  inode = (struct inode *) filp->data;
  release_inode(inode);

  return 0;
}

int dfs_fsync(struct file *filp) {
  int rc;
  struct inode *inode = (struct inode *) filp->data;

  // Flush and sync buffer cache for entire file system
  rc = flush_buffers(inode->fs->cache, 0);
  if (rc < 0) return rc;

  rc = sync_buffers(inode->fs->cache, 0);
  if (rc < 0) return rc;

  return 0;
}

int dfs_read(struct file *filp, void *data, size_t size, off64_t pos) {
  struct inode *inode;
  size_t read;
  size_t count;
  off64_t left;
  char *p;
  unsigned int iblock;
  unsigned int start;
  blkno_t blk;
  struct buf *buf;

  inode = (struct inode *) filp->data;
  read = 0;
  p = (char *) data;
  while (pos < inode->desc->size && size > 0) {
    if (filp->flags & F_CLOSED) return -EINTR;

    iblock = (unsigned int) (pos / inode->fs->blocksize);
    start = (unsigned int) (pos % inode->fs->blocksize);

    count = inode->fs->blocksize - start;
    if (count > size) count = size;

    left = inode->desc->size - (size_t) pos;
    if (count > left) count = (size_t) left;
    if (count <= 0) break;

    blk = get_inode_block(inode, iblock);
    if (blk == NOBLOCK) return -EIO;

    if (filp->flags & O_DIRECT) {
      if (start != 0 || count != inode->fs->blocksize) return read;
      if (dev_read(inode->fs->devno, p, count, blk, 0) != (int) count) return read;
    } else {
      buf = get_buffer(inode->fs->cache, blk);
      if (!buf) return -EIO;
      memcpy(p, buf->data + start, count);
      release_buffer(inode->fs->cache, buf);
    }

    pos += count;
    p += count;
    read += count;
    size -= count;
  }

  return read;
}

int dfs_write(struct file *filp, void *data, size_t size, off64_t pos) {
  struct inode *inode;
  size_t written;
  size_t count;
  char *p;
  unsigned int iblock;
  unsigned int start;
  blkno_t blk;
  struct buf *buf;
  int rc;

  inode = (struct inode *) filp->data;

  if (filp->flags & O_APPEND) pos = inode->desc->size;
  if (pos + size > DFS_MAXFILESIZE) return -EFBIG;
  if (S_ISDIR(inode->desc->mode)) return -EISDIR;

  if (pos > inode->desc->size) {
    rc = dfs_ftruncate(filp, pos);
    if (rc < 0) return rc;
  }

  written = 0;
  p = (char *) data;
  while (size > 0) {
    if (filp->flags & F_CLOSED) return -EINTR;

    iblock = (unsigned int) pos / inode->fs->blocksize;
    start = (unsigned int) pos % inode->fs->blocksize;

    count = inode->fs->blocksize - start;
    if (count > size) count = size;

    if (iblock < inode->desc->blocks) {
      blk = get_inode_block(inode, iblock);
      if (blk == NOBLOCK) return -EIO;
    } else if (iblock == inode->desc->blocks) {
      blk = expand_inode(inode);
      if (blk == NOBLOCK) return -ENOSPC;
    } else {
      return written;
    }

    if (filp->flags & O_DIRECT) {
      if (start != 0 || count != inode->fs->blocksize) return written;
      if (dev_write(inode->fs->devno, p, count, blk, 0) != (int) count) return written;
    } else {
      if (count == inode->fs->blocksize) {
        buf = alloc_buffer(inode->fs->cache, blk);
      } else {
        buf = get_buffer(inode->fs->cache, blk);
      }
      if (!buf) return -EIO;

      memcpy(buf->data + start, p, count);

      mark_buffer_updated(inode->fs->cache, buf);
      release_buffer(inode->fs->cache, buf);
    }

    filp->flags |= F_MODIFIED;
    pos += count;
    p += count;
    written += count;
    size -= count;

    if (pos > inode->desc->size) {
      inode->desc->size = (loff_t) pos;
      mark_inode_dirty(inode);
    }
  }

  return written;
}

int dfs_ioctl(struct file *filp, int cmd, void *data, size_t size) {
  return -ENOSYS;
}

off64_t dfs_tell(struct file *filp) {
  return filp->pos;
}

off64_t dfs_lseek(struct file *filp, off64_t offset, int origin) {
  struct inode *inode;

  inode = (struct inode *) filp->data;

  switch (origin) {
    case SEEK_END:
      offset += inode->desc->size;
      break;

    case SEEK_CUR:
      offset += filp->pos;
  }

  if (offset < 0) return -EINVAL;

  filp->pos = offset;
  return offset;
}

int dfs_ftruncate(struct file *filp, off64_t size) {
  struct inode *inode;
  int rc;
  unsigned int blocks;
  blkno_t blk;
  struct buf *buf;

  if (size > DFS_MAXFILESIZE) return -EFBIG;

  inode = (struct inode *) filp->data;
  if (S_ISDIR(inode->desc->mode)) return -EISDIR;

  if (size < 0) return -EINVAL;
  if (size == inode->desc->size) return 0;

  blocks = ((size_t) size + inode->fs->blocksize - 1) / inode->fs->blocksize;

  if (size > inode->desc->size) {
    while (blocks < inode->desc->blocks) {
      blk = expand_inode(inode);
      if (blk == NOBLOCK) return -ENOSPC;

      buf = alloc_buffer(inode->fs->cache, blk);
      if (!buf) return -EIO;

      memset(buf->data, 0, inode->fs->blocksize);

      mark_buffer_updated(inode->fs->cache, buf);
      release_buffer(inode->fs->cache, buf);
    }
  } else {
    blocks = ((size_t) size + inode->fs->blocksize - 1) / inode->fs->blocksize;
    rc = truncate_inode(inode, blocks);
    if (rc < 0) return rc;
  }

  inode->desc->size = (loff_t) size;
  mark_inode_dirty(inode);

  filp->flags |= F_MODIFIED;

  return 0;
}

int dfs_futime(struct file *filp, struct utimbuf *times) {
  struct inode *inode;

  inode = (struct inode *) filp->data;
  if (times->ctime != -1) inode->desc->ctime = times->ctime;
  if (times->modtime != -1) inode->desc->mtime = times->modtime;
  filp->flags &= ~F_MODIFIED;
  mark_inode_dirty(inode);

  return 0;
}

int dfs_fstat(struct file *filp, struct stat64 *buffer) {
  struct inode *inode;
  off64_t size;

  inode = (struct inode *) filp->data;
  size = inode->desc->size;
  
  if (buffer) {
    memset(buffer, 0, sizeof(struct stat64));

    buffer->st_mode = inode->desc->mode;
    buffer->st_uid = inode->desc->uid;
    buffer->st_gid = inode->desc->gid;
    buffer->st_ino = inode->ino;
    buffer->st_nlink = inode->desc->linkcount;
    buffer->st_dev = inode->fs->devno;
    buffer->st_atime = time(NULL);
    buffer->st_mtime = inode->desc->mtime;
    buffer->st_ctime = inode->desc->ctime;
    buffer->st_size = inode->desc->size;
  }

  return (int) size;
}

int dfs_fchmod(struct file *filp, int mode) {
  struct thread *thread = self();
  struct inode *inode;

  inode = (struct inode *) filp->data;
  if (thread->euid != 0 && thread->euid != inode->desc->uid) return -EPERM;
  inode->desc->mode = (inode->desc->mode & ~S_IRWXUGO) | (mode & S_IRWXUGO);
  mark_inode_dirty(inode);

  return 0;
}

int dfs_fchown(struct file *filp, int owner, int group) {
  struct thread *thread = self();
  struct inode *inode;

  inode = (struct inode *) filp->data;
  if (thread->euid != 0) return -EPERM;
  if (owner != -1) inode->desc->uid = owner;
  if (group != -1) inode->desc->gid = group;
  mark_inode_dirty(inode);

  return 0;
}
