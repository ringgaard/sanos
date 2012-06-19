#include <time.h>
#include <string.h>

#include "types.h"
#include "buf.h"
#include "vfs.h"
#include "dfs.h"

static struct inode *open_existing(struct filsys *fs, char *name, int len)
{
  struct inode *inode;
  vfs_ino_t ino;

  ino = lookup_name(fs, DFS_INODE_ROOT, name, len);
  if (ino == -1) return NULL;
  
  inode = get_inode(fs, ino);
  if (!inode) return NULL;

  if (VFS_S_ISDIR(inode->desc->mode))
  {
    release_inode(inode);
    return NULL;
  }

  return inode;
}

static struct inode *open_always(struct filsys *fs, char *name, int len, int mode)
{
  struct inode *dir;
  struct inode *inode;
  vfs_ino_t ino;

  dir = parse_name(fs, &name, &len);
  if (!dir) return NULL;

  ino = find_dir_entry(dir, name, len);
  if (ino == -1)
  {
    inode = alloc_inode(dir, VFS_S_IFREG | mode);
    if (!inode) 
    {
      release_inode(dir);
      return NULL;
    }

    inode->desc->linkcount++;
    mark_inode_dirty(inode);

    if (add_dir_entry(dir, name, len, inode->ino) < 0)
    {
      unlink_inode(inode);
      release_inode(inode);
      release_inode(dir);
      return NULL;
    }
  }
  else
  {
    inode = get_inode(fs, ino);
    if (!inode) 
    {
      release_inode(dir);
      return NULL;
    }

    if (VFS_S_ISDIR(inode->desc->mode))
    {
      release_inode(dir);
      release_inode(inode);
      return NULL;
    }
  }

  release_inode(dir);
  return inode;
}

static struct inode *create_new(struct filsys *fs, char *name, int len, int mode)
{
  struct inode *dir;
  struct inode *inode;
  struct inode *oldinode;
  vfs_ino_t oldino;

  dir = parse_name(fs, &name, &len);
  if (!dir) return NULL;

  inode = alloc_inode(dir, VFS_S_IFREG | mode);
  if (!inode)
  {
    release_inode(dir);
    return NULL;
  }

  inode->desc->linkcount++;
  mark_inode_dirty(inode);

  oldino = modify_dir_entry(dir, name, len, inode->ino);
  if (oldino != -1)
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
    if (add_dir_entry(dir, name, len, inode->ino) < 0)
    {
      unlink_inode(inode);
      release_inode(inode);
      release_inode(dir);
      return NULL;
    }
  }

  release_inode(dir);
  return inode;
}

static struct inode *truncate_existing(struct filsys *fs, char *name, int len)
{
  struct inode *inode;
  vfs_ino_t ino;

  ino = lookup_name(fs, DFS_INODE_ROOT, name, len);
  if (ino == -1) return NULL;
  
  inode = get_inode(fs, ino);
  if (!inode) return NULL;

  if (VFS_S_ISDIR(inode->desc->mode))
  {
    release_inode(inode);
    return NULL;
  }

  if (truncate_inode(inode, 0) < 0)
  {
    release_inode(inode);
    return NULL;
  }
  inode->desc->size = 0;
  mark_inode_dirty(inode);

  return inode;
}

static struct inode *create_always(struct filsys *fs, char *name, int len, vfs_ino_t ino, int mode)
{
  struct inode *dir;
  struct inode *inode;

  dir = parse_name(fs, &name, &len);
  if (!dir) return NULL;

  if (find_dir_entry(dir, name, len) != -1)
  {
    release_inode(dir);
    return NULL;
  }

  if (ino == -1)
    inode = alloc_inode(dir, VFS_S_IFREG | mode);
  else
  {
    inode = get_inode(fs, ino);
    if (inode->desc->ctime == 0) inode->desc->ctime = time(NULL);
    inode->desc->mode = VFS_S_IFREG | mode;
  }

  if (!inode)
  {
    release_inode(dir);
    return NULL;
  }

  inode->desc->linkcount++;
  mark_inode_dirty(inode);

  if (add_dir_entry(dir, name, len, inode->ino) < 0)
  {
    unlink_inode(inode);
    release_inode(inode);
    release_inode(dir);
    return NULL;
  }

  release_inode(dir);
  return inode;
}

int dfs_open(struct file *filp, char *name, int mode)
{
  struct filsys *fs;
  struct inode *inode;
  int len;

  fs = (struct filsys *) filp->fs->data;
  len = strlen(name);

  switch (filp->flags & (VFS_O_CREAT | VFS_O_EXCL | VFS_O_TRUNC | VFS_O_SPECIAL))
  {
    case 0:
    case VFS_O_EXCL:
      // Open existing file
      inode = open_existing(fs, name, len);
      break;

    case VFS_O_CREAT:
      // Open file, create new file if it does not exists
      inode = open_always(fs, name, len, mode);
      break;

    case VFS_O_CREAT | VFS_O_EXCL:
    case VFS_O_CREAT | VFS_O_TRUNC | VFS_O_EXCL:
      // Create new file, unlink existing file if it exists
      inode = create_new(fs, name, len, mode);
      break;

    case VFS_O_TRUNC:
    case VFS_O_TRUNC | VFS_O_EXCL:
      // Open and truncate existing file
      inode = truncate_existing(fs, name, len);
      filp->flags |= F_MODIFIED;
      break;

    case VFS_O_CREAT | VFS_O_TRUNC:
      // Create new file, fail if it exists
      inode = create_always(fs, name, len, -1, mode);
      break;

    case VFS_O_SPECIAL:
      // Create new file with special inode number
      inode = create_always(fs, name, len, filp->flags >> 24, mode);
      break;

    default:
      return -1;
  }

  if (!inode) return -1;

  if (filp->flags & VFS_O_APPEND) filp->pos = (int) inode->desc->size;
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
  vfs_blkno_t blk;
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

    left = (int) inode->desc->size - filp->pos;
    if (count > left) count = left;

    blk = get_inode_block(inode, iblock);
    if (blk == -1) return read;

    buf = get_buffer(inode->fs->cache, blk);
    if (!buf) return read;

    memcpy(p, buf->data + start, count);

    release_buffer(inode->fs->cache, buf);

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
  vfs_blkno_t blk;
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
      blk = get_inode_block(inode, iblock);
    else if (iblock == inode->desc->blocks)
      blk = expand_inode(inode);
    else
      return written;

    if (blk == -1) return written;

    if (count == inode->fs->blocksize)
      buf = alloc_buffer(inode->fs->cache, blk);
    else
      buf = get_buffer(inode->fs->cache, blk);

    if (!buf) return written;

    memcpy(buf->data + start, p, count);
    filp->flags |= F_MODIFIED;

    mark_buffer_updated(buf);
    release_buffer(inode->fs->cache, buf);

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

vfs_loff_t dfs_tell(struct file *filp)
{
  return filp->pos;
}

vfs_loff_t dfs_lseek(struct file *filp, vfs_loff_t offset, int origin)
{
  struct inode *inode;

  inode = (struct inode *) filp->data;

  switch (origin)
  {
    case VFS_SEEK_END:
      offset += (int) inode->desc->size;
      break;

    case VFS_SEEK_CUR:
      offset += filp->pos;
  }

  if (offset < 0 || offset > inode->desc->size) return -1;

  filp->pos = offset;
  return offset;
}

int dfs_chsize(struct file *filp, vfs_loff_t size)
{
  struct inode *inode;
  int rc;
  unsigned int blocks;

  inode = (struct inode *) filp->data;

  if (size < 0 || size > inode->desc->size) return -1;

  inode->desc->size = size;
  mark_inode_dirty(inode);

  filp->flags |= F_MODIFIED;
  if (filp->pos > size) filp->pos = size;

  blocks = (size + inode->fs->blocksize - 1) / inode->fs->blocksize;
  rc = truncate_inode(inode, blocks);
  if (rc < 0) return rc;

  return 0;
}

int dfs_futime(struct file *filp, struct vfs_utimbuf *times)
{
  struct inode *inode;

  inode = (struct inode *) filp->data;
  if (times->ctime != -1) inode->desc->ctime = times->ctime;
  if (times->mtime != -1) inode->desc->mtime = times->mtime;
  mark_inode_dirty(inode);
  filp->flags &= ~F_MODIFIED;

  return 0;
}

int dfs_fstat(struct file *filp, struct vfs_stat *buffer)
{
  struct inode *inode;

  inode = (struct inode *) filp->data;
  
  buffer->mode = inode->desc->mode;
  buffer->ino = inode->ino;
  buffer->atime = time(NULL);
  buffer->mtime = inode->desc->mtime;
  buffer->ctime = inode->desc->ctime;
  
  buffer->quad.size_low = (int) inode->desc->size;
  buffer->quad.size_high = 0;

  return 0;
}
