//
// dfs.c
//
// Disk filesystem routines
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
// 3. Neither the name of Michael Ringgaard nor the names of its contributors
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

struct fsops dfsops =
{
  FSOP_READ | FSOP_WRITE | FSOP_IOCTL | FSOP_TELL | FSOP_LSEEK | FSOP_CHSIZE | 
  FSOP_FUTIME | FSOP_FSTAT,

  NULL,
  NULL,

  dfs_format,
  dfs_mount,
  dfs_umount,

  dfs_statfs,

  dfs_open,
  dfs_close,
  dfs_flush,

  dfs_read,
  dfs_write,
  dfs_ioctl,

  dfs_tell,
  dfs_lseek,
  dfs_chsize,

  dfs_futime,
  dfs_utime,

  dfs_fstat,
  dfs_stat,

  dfs_mkdir,
  dfs_rmdir,

  dfs_rename,
  dfs_link,
  dfs_unlink,

  dfs_opendir,
  dfs_readdir
};

void init_dfs()
{
  register_filesystem("dfs", &dfsops);
}

int dfs_utime(struct fs *fs, char *name, struct utimbuf *times)
{
  ino_t ino;
  struct inode *inode;

  ino = lookup_name((struct filsys *) fs->data, DFS_INODE_ROOT, name, strlen(name));
  if (ino == NOINODE) return -ENOENT;

  inode = get_inode((struct filsys *) fs->data, ino);
  if (!inode) return -EIO;

  if (times->ctime != -1) inode->desc->ctime = times->ctime;
  if (times->mtime != -1) inode->desc->mtime = times->mtime;
  mark_inode_dirty(inode);

  release_inode(inode);
  return 0;
}

int dfs_stat(struct fs *fs, char *name, struct stat *buffer)
{
  ino_t ino;
  struct inode *inode;
  size_t size;

  ino = lookup_name((struct filsys *) fs->data, DFS_INODE_ROOT, name, strlen(name));
  if (ino == NOINODE) return -ENOENT;

  inode = get_inode((struct filsys *) fs->data, ino);
  if (!inode) return -EIO;

  size = inode->desc->size;

  if (buffer)
  {
    buffer->mode = 0;
    if (inode->desc->flags & DFS_INODE_FLAG_DIRECTORY) buffer->mode |= FS_DIRECTORY;

    buffer->ino = ino;
    buffer->nlink = inode->desc->linkcount;
    buffer->devno = NODEV;

    buffer->atime = time(NULL);
    buffer->mtime = inode->desc->mtime;
    buffer->ctime = inode->desc->ctime;
  
    buffer->quad.size_low = inode->desc->size;
    buffer->quad.size_high = 0;
  }

  release_inode(inode);

  return size;
}

int dfs_mkdir(struct fs *fs, char *name)
{
  struct inode *parent;
  struct inode *dir;
  int len;
  int rc;

  len = strlen(name);
  parent = parse_name((struct filsys *) fs->data, &name, &len);
  if (!parent) return -ENOENT;

  if (find_dir_entry(parent, name, len) != NOINODE)
  {
    release_inode(parent);
    return -EEXIST;
  }

  dir = alloc_inode(parent, DFS_INODE_FLAG_DIRECTORY);
  if (!dir)
  {
    release_inode(parent);
    return -ENOSPC;
  }

  dir->desc->linkcount++;
  mark_inode_dirty(dir);

  rc = add_dir_entry(parent, name, len, dir->ino);
  if (rc < 0)
  {
    unlink_inode(dir);
    release_inode(dir);
    release_inode(parent);
    return rc;
  }

  release_inode(dir);
  release_inode(parent);
  return 0;
}

int dfs_rmdir(struct fs *fs, char *name)
{
  struct inode *parent;
  struct inode *dir;
  ino_t ino;
  int len;
  int rc;

  len = strlen(name);
  parent = parse_name((struct filsys *) fs->data, &name, &len);
  if (!parent) return -ENOENT;

  ino = find_dir_entry(parent, name, len);
  if (ino == NOINODE || ino == DFS_INODE_ROOT)
  {
    release_inode(parent);
    return ino == DFS_INODE_ROOT ? -EPERM : -ENOENT;
  }

  dir = get_inode(parent->fs, ino);
  if (!dir)
  {
    release_inode(parent);
    return -EIO;
  }

  if (!(dir->desc->flags & DFS_INODE_FLAG_DIRECTORY))
  {
    release_inode(dir);
    release_inode(parent);
    return -ENOTDIR;
  }

  if (dir->desc->size > 0)
  {
    release_inode(dir);
    release_inode(parent);
    return -ENOTEMPTY;
  }

  rc = delete_dir_entry(parent, name, len); 
  if (rc < 0)
  {
    release_inode(dir);
    release_inode(parent);
    return rc;
  }

  rc = unlink_inode(dir);
  if (rc < 0)
  {
    release_inode(dir);
    release_inode(parent);
    return rc;
  }

  release_inode(dir);
  release_inode(parent);
  return 0;
}

int dfs_rename(struct fs *fs, char *oldname, char *newname)
{
  struct inode *inode;
  struct inode *oldparent;
  struct inode *newparent;
  int oldlen;
  int newlen;
  ino_t ino;
  int rc;

  oldlen = strlen(oldname);
  oldparent = parse_name((struct filsys *) fs->data, &oldname, &oldlen);
  if (!oldparent) return -ENOENT;

  ino = find_dir_entry(oldparent, oldname, oldlen);
  if (ino == NOINODE) 
  {
    release_inode(oldparent);
    return -ENOENT;
  }

  inode = get_inode(oldparent->fs, ino);
  if (!inode)
  {
    release_inode(oldparent);
    return -EIO;
  }

  newlen = strlen(newname);
  newparent = parse_name((struct filsys *) fs->data, &newname, &newlen);
  if (!newparent) 
  {
    release_inode(inode);
    release_inode(oldparent);
    return -ENOENT;
  }

  if (find_dir_entry(newparent, newname, newlen) != NOINODE)
  {
    release_inode(inode);
    release_inode(oldparent);
    release_inode(newparent);
    return -EEXIST;
  }

  rc = add_dir_entry(newparent, newname, newlen, inode->ino); 
  if (rc < 0)
  {
    release_inode(inode);
    release_inode(oldparent);
    release_inode(newparent);
    return rc;
  }

  rc = delete_dir_entry(oldparent, oldname, oldlen);
  if (rc < 0)
  {
    release_inode(inode);
    release_inode(oldparent);
    release_inode(newparent);
    return rc;
  }

  release_inode(inode);
  release_inode(oldparent);
  release_inode(newparent);
  return 0;
}

int dfs_link(struct fs *fs, char *oldname, char *newname)
{
  struct inode *inode;
  struct inode *parent;
  ino_t ino;
  int len;
  int rc;

  ino = lookup_name((struct filsys *) fs->data, DFS_INODE_ROOT, oldname, strlen(oldname));
  if (ino == NOINODE) return -ENOENT;

  inode = get_inode((struct filsys *) fs->data, ino);
  if (!inode) return -EIO;

  len = strlen(newname);
  parent = parse_name((struct filsys *) fs->data, &newname, &len);
  if (!parent) 
  {
    release_inode(inode);
    return -ENOENT;
  }

  if (find_dir_entry(parent, newname, len) != NOINODE)
  {
    release_inode(inode);
    release_inode(parent);
    return -EEXIST;
  }

  inode->desc->linkcount++;
  mark_inode_dirty(inode);

  rc = add_dir_entry(parent, newname, len, inode->ino);
  if (rc < 0)
  {
    unlink_inode(inode);
    release_inode(inode);
    release_inode(parent);
    return rc;
  }

  release_inode(inode);
  release_inode(parent);
  return 0;
}

int dfs_unlink(struct fs *fs, char *name)
{
  struct inode *dir;
  struct inode *inode;
  ino_t ino;
  int len;
  int rc;

  len = strlen(name);
  dir = parse_name((struct filsys *) fs->data, &name, &len);
  if (!dir) return -ENOENT;

  ino = find_dir_entry(dir, name, len);
  if (ino == NOINODE)
  {
    release_inode(dir);
    return -ENOENT;
  }

  inode = get_inode(dir->fs, ino);
  if (!dir)
  {
    release_inode(dir);
    return -EIO;
  }

  if (inode->desc->flags & DFS_INODE_FLAG_DIRECTORY)
  {
    release_inode(inode);
    release_inode(dir);
    return -EISDIR;
  }

  rc = delete_dir_entry(dir, name, len);
  if (rc < 0)
  {
    release_inode(inode);
    release_inode(dir);
    return rc;
  }

  rc = unlink_inode(inode); 
  if (rc < 0)
  {
    release_inode(inode);
    release_inode(dir);
    return rc;
  }

  release_inode(inode);
  release_inode(dir);
  return 0;
}
