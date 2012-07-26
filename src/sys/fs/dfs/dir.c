//
// dir.c
//
// Disk filesystem directory routines
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

#define NAME_ALIGN_LEN(l) (((l) + 3) & ~3)

int find_dir_entry(struct inode *dir, char *name, int len, ino_t *retval) {
  unsigned int block;
  blkno_t blk;
  struct buf *buf;
  char *p;
  struct dentry *de;
  ino_t ino;

  if (len <= 0) return -EINVAL;
  if (len >= MAXPATH) return -ENAMETOOLONG;
  if (!S_ISDIR(dir->desc->mode)) return -ENOTDIR;
  if (checki(dir, S_IEXEC) < 0) return -EACCES;

  for (block = 0; block < dir->desc->blocks; block++) {
    blk = get_inode_block(dir, block);
    if (blk == NOBLOCK) return -EIO;

    buf = get_buffer(dir->fs->cache, blk);
    if (!buf) return -EIO;

    p = buf->data;
    while (p < buf->data + dir->fs->blocksize) {
      de = (struct dentry *) p;

      if (fnmatch(name, len, de->name, de->namelen)) {
        ino = de->ino;
        release_buffer(dir->fs->cache, buf);

        if (retval) *retval = ino;
        return 0;
      }

      p += de->reclen;
    }

    release_buffer(dir->fs->cache, buf);
  }

  return -ENOENT;
}

int add_dir_entry(struct inode *dir, char *name, int len, ino_t ino) {
  unsigned int block;
  blkno_t blk;
  struct buf *buf;
  char *p;
  struct dentry *de;
  struct dentry *newde;
  unsigned int minlen;
  unsigned int newlen;

  if (len <= 0) return -EINVAL;
  if (len >= MAXPATH) return -ENAMETOOLONG;
  if (!S_ISDIR(dir->desc->mode)) return -ENOTDIR;
  if (checki(dir, S_IWRITE) < 0) return -EACCES;

  newlen = sizeof(struct dentry) + NAME_ALIGN_LEN(len);
  for (block = 0; block < dir->desc->blocks; block++) {
    blk = get_inode_block(dir, block);
    if (blk == NOBLOCK) return -EIO;

    buf = get_buffer(dir->fs->cache, blk);
    if (!buf) return -EIO;

    p = buf->data;
    while (p < buf->data + dir->fs->blocksize) {
      de = (struct dentry *) p;
      minlen = sizeof(struct dentry) + NAME_ALIGN_LEN(de->namelen);

      if (de->reclen >= minlen + newlen) {
        newde = (struct dentry *) (p + minlen);

        newde->ino = ino;
        newde->reclen = de->reclen - minlen;
        newde->namelen = len;
        memcpy(newde->name, name, len);

        de->reclen = minlen;

        mark_buffer_updated(dir->fs->cache, buf);
        release_buffer(dir->fs->cache, buf);

        dir->desc->mtime = time(NULL);
        mark_inode_dirty(dir);

        return 0;
      }

      p += de->reclen;
    }

    release_buffer(dir->fs->cache, buf);
  }

  blk = expand_inode(dir);
  if (blk == NOBLOCK) return -ENOSPC;

  buf = alloc_buffer(dir->fs->cache, blk);
  if (!buf) return -ENOMEM;

  dir->desc->size += dir->fs->blocksize;
  dir->desc->mtime = time(NULL);
  mark_inode_dirty(dir);

  newde = (struct dentry *) (buf->data);
  newde->ino = ino;
  newde->reclen = dir->fs->blocksize;
  newde->namelen = len;
  memcpy(newde->name, name, len);

  mark_buffer_updated(dir->fs->cache, buf);
  release_buffer(dir->fs->cache, buf);

  return 0;
}

int modify_dir_entry(struct inode *dir, char *name, int len, ino_t ino, ino_t *oldino) {
  unsigned int block;
  blkno_t blk;
  struct buf *buf;
  char *p;
  struct dentry *de;

  if (len <= 0) return -EINVAL;
  if (len >= MAXPATH) return -ENAMETOOLONG;
  if (!S_ISDIR(dir->desc->mode)) return -ENOTDIR;
  if (checki(dir, S_IWRITE) < 0) return -EACCES;

  for (block = 0; block < dir->desc->blocks; block++) {
    blk = get_inode_block(dir, block);
    if (blk == NOBLOCK) return -EIO;

    buf = get_buffer(dir->fs->cache, blk);
    if (!buf) return -EIO;

    p = buf->data;
    while (p < buf->data + dir->fs->blocksize) {
      de = (struct dentry *) p;

      if (fnmatch(name, len, de->name, de->namelen)) {
        if (oldino) *oldino = de->ino;
        de->ino = ino;
        mark_buffer_updated(dir->fs->cache, buf);
        release_buffer(dir->fs->cache, buf);

        return 0;
      }

      p += de->reclen;
    }

    release_buffer(dir->fs->cache, buf);
  }

  return -ENOENT;
}

int delete_dir_entry(struct inode *dir, char *name, int len) {
  unsigned int block;
  blkno_t blk;
  blkno_t lastblk;
  struct buf *buf;
  char *p;
  struct dentry *de;
  struct dentry *prevde;
  struct dentry *nextde;

  if (len <= 0 || len >= MAXPATH) return -ENAMETOOLONG;
  if (!S_ISDIR(dir->desc->mode)) return -ENOTDIR;
  if (checki(dir, S_IWRITE) < 0) return -EACCES;

  for (block = 0; block < dir->desc->blocks; block++) {
    blk = get_inode_block(dir, block);
    if (blk == NOBLOCK) return -EIO;

    buf = get_buffer(dir->fs->cache, blk);
    if (!buf) return -EIO;

    p = buf->data;
    prevde = NULL;
    while (p < buf->data + dir->fs->blocksize) {
      de = (struct dentry *) p;

      if (fnmatch(name, len, de->name, de->namelen)) {
        if (prevde) {
          // Merge entry with previous entry
          prevde->reclen += de->reclen;
          memset(de, 0, de->reclen);
          mark_buffer_updated(dir->fs->cache, buf);
        } else if (de->reclen == dir->fs->blocksize) {
          // Block is empty, swap this block with last block and truncate
          if (block != dir->desc->blocks - 1) {
            lastblk = get_inode_block(dir, dir->desc->blocks - 1);
            if (lastblk == NOBLOCK) return -EIO;
            set_inode_block(dir, block, lastblk);
            set_inode_block(dir, dir->desc->blocks - 1, buf->blkno);
          }

          truncate_inode(dir, dir->desc->blocks - 1);
          dir->desc->size -= dir->fs->blocksize;
          mark_buffer_invalid(dir->fs->cache, buf);
        } else {
          // Merge with next entry
          nextde = (struct dentry *) (p + de->reclen);
          de->ino = nextde->ino;
          de->reclen += nextde->reclen;
          de->namelen = nextde->namelen;
          memmove(de->name, nextde->name, nextde->namelen);
          mark_buffer_updated(dir->fs->cache, buf);
        }

        release_buffer(dir->fs->cache, buf);

        dir->desc->mtime = time(NULL);
        mark_inode_dirty(dir);

        return 0;
      }

      prevde = de;
      p += de->reclen;
    }

    release_buffer(dir->fs->cache, buf);
  }

  return -ENOENT;
}

static int lookup_name(struct filsys *fs, ino_t ino, char *name, int len, ino_t *retval) {
  char *p;
  int l;
  struct inode *inode;
  int rc;

  while (1) {
    // Skip path separator
    if (*name == PS1 || *name == PS2) {
      name++;
      len--;
    }
    
    if (len == 0) {
      *retval = ino;
      return 0;
    }

    // Find next part of name
    p = name;
    l = 0;
    while (l < len && *p != PS1 && *p != PS2) {
      l++;
      p++;
    }

    // Find inode for next name part
    rc = get_inode(fs, ino, &inode);
    if (rc < 0) return rc;
    
    rc = find_dir_entry(inode, name, l, &ino);
    release_inode(inode);
    if (rc < 0) return rc;

    // If we have parsed the whole name return the inode number
    if (l == len) {
      *retval = ino;
      return 0;
    }

    // Prepare for next name part
    name = p;
    len -= l;
  }
}

int diri(struct filsys *fs, char **name, int *len, struct inode **inode) {
  char *start;
  char *p;
  ino_t ino;
  struct inode *dir;
  int rc;

  if (*len == 0) return get_inode(fs, DFS_INODE_ROOT, inode);

  start = *name;
  p = start + *len - 1;
  while (p > start && *p != PS1 && *p != PS2) p--;

  if (p == start) {
    if (*p == PS1 || *p == PS2) {
      (*name)++;
      (*len)--;
    }

    return get_inode(fs, DFS_INODE_ROOT, inode);
  }

  rc = lookup_name(fs, DFS_INODE_ROOT, start, p - start, &ino);
  if (rc < 0) return rc;

  rc = get_inode(fs, ino, &dir);
  if (rc < 0) return rc;

  if (!S_ISDIR(dir->desc->mode)) {
    release_inode(dir);
    return -ENOTDIR;
  }

  *name = p + 1;
  *len -= p - start + 1;
  *inode = dir;

  return 0;
}

int namei(struct filsys *fs, char *name, struct inode **inode) {
  ino_t ino;
  int rc;
  
  rc = lookup_name(fs, DFS_INODE_ROOT, name, strlen(name), &ino);
  if (rc < 0) return rc;

  rc = get_inode(fs, ino, inode);
  if (rc < 0) return rc;

  return 0;
}

int dfs_opendir(struct file *filp, char *name) {
  struct inode *inode;
  int rc;

  rc = namei((struct filsys *) filp->fs->data, name, &inode);
  if (rc < 0) return rc;

  if (!S_ISDIR(inode->desc->mode)) {
    release_inode(inode);
    return -ENOTDIR;
  }

  filp->data = inode;
  filp->mode = inode->desc->mode;
  filp->owner = inode->desc->uid;
  filp->group = inode->desc->gid;

  return 0;
}

int dfs_readdir(struct file *filp, struct direntry *dirp, int count) {
  unsigned int iblock;
  unsigned int start;
  struct inode *inode;
  blkno_t blk;
  struct buf *buf;
  struct dentry *de;

  inode = (struct inode *) filp->data;
  if (count != 1) return -EINVAL;
  if (filp->pos == inode->desc->size) return 0;
  if (filp->pos > inode->desc->size) return 0;

  iblock = (unsigned int) filp->pos / inode->fs->blocksize;
  start = (unsigned int) filp->pos % inode->fs->blocksize;

  blk = get_inode_block(inode, iblock);
  if (blk == NOBLOCK) return -EIO;

  buf = get_buffer(inode->fs->cache, blk);
  if (!buf) return -EIO;

  de = (struct dentry *) (buf->data + start);
  if (de->reclen + start > inode->fs->blocksize || de->namelen <= 0 || de->namelen >= MAXPATH) {
    release_buffer(inode->fs->cache, buf);
    return 0;
  }

  dirp->ino = de->ino;
  dirp->reclen = de->reclen;
  dirp->namelen = de->namelen;
  memcpy(dirp->name, de->name, de->namelen);
  dirp->name[de->namelen] = 0;
  
  filp->pos += de->reclen;

  release_buffer(inode->fs->cache, buf);
  return 1;
}
