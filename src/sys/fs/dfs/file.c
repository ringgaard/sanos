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

static int open_existing(struct filsys *fs, char *name, int len, struct inode **retval)
{
  struct inode *inode;
  ino_t ino;

  ino = lookup_name(fs, DFS_INODE_ROOT, name, len);
  if (ino == NOINODE) return -ENOENT;
  
  inode = get_inode(fs, ino);
  if (!inode) return -EIO;

  if (inode->desc->flags & DFS_INODE_FLAG_DIRECTORY)
  {
    release_inode(inode);
    return -EISDIR;
  }

  *retval = inode;
  return 0;
}

static int open_always(struct filsys *fs, char *name, int len, struct inode **retval)
{
  struct inode *dir;
  struct inode *inode;
  ino_t ino;
  int rc;

  dir = parse_name(fs, &name, &len);
  if (!dir) return -ENOENT;

  ino = find_dir_entry(dir, name, len);
  if (ino == NOINODE)
  {
    inode = alloc_inode(dir, 0);
    if (!inode) 
    {
      release_inode(dir);
      return -ENOSPC;
    }

    inode->desc->linkcount++;
    mark_inode_dirty(inode);

    rc = add_dir_entry(dir, name, len, inode->ino);
    if (rc < 0)
    {
      unlink_inode(inode);
      release_inode(inode);
      release_inode(dir);
      return rc;
    }
  }
  else
  {
    inode = get_inode(fs, ino);
    if (!inode) 
    {
      release_inode(dir);
      return -EIO;
    }

    if (inode->desc->flags & DFS_INODE_FLAG_DIRECTORY)
    {
      release_inode(dir);
      release_inode(inode);
      return -EISDIR;
    }
  }

  release_inode(dir);
  *retval = inode;
  return 0;
}

static int create_always(struct filsys *fs, char *name, int len, struct inode **retval)
{
  struct inode *dir;
  struct inode *inode;
  struct inode *oldinode;
  ino_t oldino;
  int rc;

  dir = parse_name(fs, &name, &len);
  if (!dir) return -ENOENT;

  inode = alloc_inode(dir, 0);
  if (!inode)
  {
    release_inode(dir);
    return -ENOSPC;
  }

  inode->desc->linkcount++;
  mark_inode_dirty(inode);

  oldino = modify_dir_entry(dir, name, len, inode->ino);
  if (oldino != NOINODE)
  {
    oldinode = get_inode(fs, oldino);
    if (oldinode)
    {
      unlink_inode(oldinode);
      release_inode(oldinode);
    }
  }
  else
  {
    rc = add_dir_entry(dir, name, len, inode->ino);
    if (rc < 0)
    {
      unlink_inode(inode);
      release_inode(inode);
      release_inode(dir);
      return rc;
    }
  }

  release_inode(dir);
  *retval = inode;
  return 0;
}

static int truncate_existing(struct filsys *fs, char *name, int len, struct inode **retval)
{
  struct inode *inode;
  ino_t ino;
  int rc;

  ino = lookup_name(fs, DFS_INODE_ROOT, name, len);
  if (ino == -1) return -ENOENT;
  
  inode = get_inode(fs, ino);
  if (!inode) return -EIO;

  if (inode->desc->flags & DFS_INODE_FLAG_DIRECTORY)
  {
    release_inode(inode);
    return -EISDIR;
  }

  rc = truncate_inode(inode, 0); 
  if (rc < 0)
  {
    release_inode(inode);
    return rc;
  }
  inode->desc->size = 0;
  mark_inode_dirty(inode);

  *retval = inode;
  return 0;
}

static int create_new(struct filsys *fs, char *name, int len, ino_t ino, struct inode **retval)
{
  struct inode *dir;
  struct inode *inode;
  int rc;

  dir = parse_name(fs, &name, &len);
  if (!dir) return -ENOENT;

  if (find_dir_entry(dir, name, len) != NOINODE)
  {
    release_inode(dir);
    return -EEXIST;
  }

  if (ino == NOINODE)
    inode = alloc_inode(dir, 0);
  else
  {
    inode = get_inode(fs, ino);
    if (inode && inode->desc->ctime == 0) inode->desc->ctime = time(NULL);
  }

  if (!inode)
  {
    release_inode(dir);
    return -ENOSPC;
  }

  inode->desc->linkcount++;
  mark_inode_dirty(inode);

  rc = add_dir_entry(dir, name, len, inode->ino);
  if (rc < 0)
  {
    unlink_inode(inode);
    release_inode(inode);
    release_inode(dir);
    return rc;
  }

  release_inode(dir);
  *retval = inode;
  return 0;
}

int dfs_open(struct file *filp, char *name)
{
  struct filsys *fs;
  struct inode *inode;
  int len;
  int rc;

  fs = (struct filsys *) filp->fs->data;
  len = strlen(name);

  switch (filp->flags & (O_CREAT | O_EXCL | O_TRUNC | O_SPECIAL))
  {
    case 0:
    case O_EXCL:
      // Open existing file
      rc = open_existing(fs, name, len, &inode);
      break;

    case O_CREAT:
      // Open file, create new file if it does not exists
      rc = open_always(fs, name, len, &inode);
      break;

    case O_CREAT | O_EXCL:
    case O_CREAT | O_TRUNC | O_EXCL:
      // Create new file, fail if it exists
      rc = create_new(fs, name, len, NOINODE, &inode);
      filp->flags |= F_MODIFIED;
      break;

    case O_TRUNC:
    case O_TRUNC | O_EXCL:
      // Open and truncate existing file
      rc = truncate_existing(fs, name, len, &inode);
      filp->flags |= F_MODIFIED;
      break;

    case O_CREAT | O_TRUNC:
      // Create new file, unlink existing file if it exists
      rc = create_always(fs, name, len, &inode);
      filp->flags |= F_MODIFIED;
      break;

    case O_SPECIAL:
      // Create new file with special inode number
      rc = create_new(fs, name, len, filp->flags >> 24, &inode);
      filp->flags |= F_MODIFIED;
      break;

    default:
      return -EINVAL;
  }

  if (rc < 0) return rc;

  if (filp->flags & O_APPEND) filp->pos = inode->desc->size;
  filp->data = inode;

  return 0;
}

int dfs_close(struct file *filp)
{
  struct inode *inode;

  inode = (struct inode *) filp->data;
  if (filp->flags & F_MODIFIED) 
  {
    inode->desc->mtime = time(NULL);
    mark_inode_dirty(inode);
  }

  release_inode(inode);

  return 0;
}

int dfs_flush(struct file *filp)
{
  int rc;
  struct inode *inode = (struct inode *) filp->data;

  // Flush and sync buffer cache for entire file system
  rc = flush_buffers(inode->fs->cache, 0);
  if (rc < 0) return rc;

  rc = sync_buffers(inode->fs->cache, 0);
  if (rc < 0) return rc;

  return 0;
}

int dfs_read(struct file *filp, void *data, size_t size)
{
  struct inode *inode;
  size_t read;
  size_t count;
  size_t left;
  char *p;
  unsigned int iblock;
  unsigned int start;
  blkno_t blk;
  struct buf *buf;

  inode = (struct inode *) filp->data;
  read = 0;
  p = (char *) data;
  while (filp->pos < inode->desc->size && size > 0)
  {
    iblock = filp->pos / inode->fs->blocksize;
    start = filp->pos % inode->fs->blocksize;

    count = inode->fs->blocksize - start;
    if (count > size) count = size;

    left = inode->desc->size - filp->pos;
    if (count > left) count = left;
    if (count <= 0) break;

    blk = get_inode_block(inode, iblock);
    if (blk == NOBLOCK) return -EIO;

    if (filp->flags & O_DIRECT)
    {
      if (start != 0 || count != inode->fs->blocksize) return read;
      if (dev_read(inode->fs->devno, p, count, blk) != (int) count) return read;
    }
    else
    {
      buf = get_buffer(inode->fs->cache, blk);
      if (!buf) return -EIO;
      memcpy(p, buf->data + start, count);
      release_buffer(inode->fs->cache, buf);
    }

    filp->pos += count;
    p += count;
    read += count;
    size -= count;
  }

  return read;
}

int dfs_write(struct file *filp, void *data, size_t size)
{
  struct inode *inode;
  size_t written;
  size_t count;
  char *p;
  unsigned int iblock;
  unsigned int start;
  blkno_t blk;
  struct buf *buf;

  inode = (struct inode *) filp->data;
  written = 0;
  p = (char *) data;
  while (size > 0)
  {
    iblock = filp->pos / inode->fs->blocksize;
    start = filp->pos % inode->fs->blocksize;

    count = inode->fs->blocksize - start;
    if (count > size) count = size;

    if (iblock < inode->desc->blocks)
    {
      blk = get_inode_block(inode, iblock);
      if (blk == NOBLOCK) return -EIO;
    }
    else if (iblock == inode->desc->blocks)
    {
      blk = expand_inode(inode);
      if (blk == NOBLOCK) return -ENOSPC;
    }
    else
      return written;

    if (filp->flags & O_DIRECT)
    {
      if (start != 0 || count != inode->fs->blocksize) return written;
      if (dev_write(inode->fs->devno, p, count, blk) != (int) count) return written;
    }
    else
    {
      if (count == inode->fs->blocksize)
	buf = alloc_buffer(inode->fs->cache, blk);
      else
	buf = get_buffer(inode->fs->cache, blk);

      if (!buf) return -EIO;

      memcpy(buf->data + start, p, count);

      mark_buffer_updated(inode->fs->cache, buf);
      release_buffer(inode->fs->cache, buf);
    }

    filp->flags |= F_MODIFIED;
    filp->pos += count;
    p += count;
    written += count;
    size -= count;

    if (filp->pos > inode->desc->size)
    {
      inode->desc->size = filp->pos;
      mark_inode_dirty(inode);
    }
  }

  return written;
}

int dfs_ioctl(struct file *filp, int cmd, void *data, size_t size)
{
  return -ENOSYS;
}

loff_t dfs_tell(struct file *filp)
{
  return filp->pos;
}

loff_t dfs_lseek(struct file *filp, loff_t offset, int origin)
{
  struct inode *inode;

  inode = (struct inode *) filp->data;

  switch (origin)
  {
    case SEEK_END:
      offset += inode->desc->size;
      break;

    case SEEK_CUR:
      offset += filp->pos;
  }

  if (offset < 0 || offset > inode->desc->size) return -EINVAL;

  filp->pos = offset;
  return offset;
}

int dfs_chsize(struct file *filp, loff_t size)
{
  struct inode *inode;
  int rc;
  unsigned int blocks;

  inode = (struct inode *) filp->data;

  if (size < 0 || size > inode->desc->size) return -EINVAL;

  inode->desc->size = size;
  mark_inode_dirty(inode);

  filp->flags |= F_MODIFIED;
  if (filp->pos > size) filp->pos = size;

  blocks = (size + inode->fs->blocksize - 1) / inode->fs->blocksize;
  rc = truncate_inode(inode, blocks);
  if (rc < 0) return rc;

  return 0;
}

int dfs_futime(struct file *filp, struct utimbuf *times)
{
  struct inode *inode;

  inode = (struct inode *) filp->data;
  if (times->ctime != -1) inode->desc->ctime = times->ctime;
  if (times->mtime != -1) inode->desc->mtime = times->mtime;
  mark_inode_dirty(inode);
  filp->flags &= ~F_MODIFIED;

  return 0;
}

int dfs_fstat(struct file *filp, struct stat *buffer)
{
  struct inode *inode;
  size_t size;

  inode = (struct inode *) filp->data;
  size = inode->desc->size;
  
  if (buffer)
  {
    buffer->mode = 0;
    if (inode->desc->flags & DFS_INODE_FLAG_DIRECTORY) buffer->mode |= FS_DIRECTORY;

    buffer->ino = inode->ino;
    buffer->nlink = inode->desc->linkcount;
    buffer->devno = NODEV;
    buffer->atime = time(NULL);
    buffer->mtime = inode->desc->mtime;
    buffer->ctime = inode->desc->ctime;
  
    buffer->quad.size_low = inode->desc->size;
    buffer->quad.size_high = 0;
  }

  return size;
}
