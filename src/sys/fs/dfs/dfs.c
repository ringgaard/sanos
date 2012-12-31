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

struct fsops dfsops = {
  FSOP_READ | FSOP_WRITE | FSOP_IOCTL | FSOP_TELL | FSOP_LSEEK | FSOP_FTRUNCATE | 
  FSOP_FUTIME | FSOP_FSTAT,

  NULL,
  NULL,

  dfs_mkfs,
  dfs_mount,
  dfs_umount,

  dfs_statfs,

  dfs_open,
  dfs_close,
  dfs_destroy,
  dfs_fsync,

  dfs_read,
  dfs_write,
  dfs_ioctl,

  dfs_tell,
  dfs_lseek,
  dfs_ftruncate,

  dfs_futime,
  dfs_utime,

  dfs_fstat,
  dfs_stat,

  dfs_access,

  dfs_fchmod,
  dfs_chmod,
  dfs_fchown,
  dfs_chown,

  dfs_mkdir,
  dfs_rmdir,

  dfs_rename,
  dfs_link,
  dfs_unlink,

  dfs_opendir,
  dfs_readdir
};

void init_dfs() {
  register_filesystem("dfs", &dfsops);
}

int dfs_utime(struct fs *fs, char *name, struct utimbuf *times) {
  struct inode *inode;
  int rc;

  rc = namei((struct filsys *) fs->data, name, &inode);
  if (rc < 0) return rc;

  rc = checki(inode, S_IWRITE);
  if (rc < 0) {
    release_inode(inode);
    return rc;
  }

  if (times->ctime != -1) inode->desc->ctime = times->ctime;
  if (times->modtime != -1) inode->desc->mtime = times->modtime;
  mark_inode_dirty(inode);

  release_inode(inode);
  return 0;
}

int dfs_stat(struct fs *fs, char *name, struct stat64 *buffer) {
  struct inode *inode;
  off64_t size;
  int rc;

  rc = namei((struct filsys *) fs->data, name, &inode);
  if (rc < 0) return rc;

  size = inode->desc->size;

  if (buffer) {
    memset(buffer, 0, sizeof(struct stat64));

    buffer->st_mode = inode->desc->mode;
    buffer->st_uid = inode->desc->uid;
    buffer->st_gid = inode->desc->gid;
    buffer->st_ino = inode->ino;
    buffer->st_nlink = inode->desc->linkcount;
    buffer->st_dev = ((struct filsys *) fs->data)->devno;

    buffer->st_atime = time(NULL);
    buffer->st_mtime = inode->desc->mtime;
    buffer->st_ctime = inode->desc->ctime;
    buffer->st_size = inode->desc->size;
  }

  release_inode(inode);

  return (int) size;
}

int dfs_access(struct fs *fs, char *name, int mode) {
  struct thread *thread = self();
  struct inode *inode;
  int rc;

  rc = namei((struct filsys *) fs->data, name, &inode);
  if (rc < 0) return rc;

  if (mode != 0) {
    if (thread->euid == 0) {
      rc = mode != 1 || inode->desc->mode & 0111 ? 0 : -EACCES;
    } else {
      if (thread->euid != inode->desc->uid) {
        mode >>= 3;
        if (thread->egid != inode->desc->gid) mode >>= 3;
      }
      if ((mode & inode->desc->mode) == 0) rc = -EACCES;
    }
  }

  release_inode(inode);

  return rc;
}

int dfs_mkdir(struct fs *fs, char *name, int mode) {
  struct inode *parent;
  ino_t ino;
  struct inode *dir;
  int len;
  int rc;

  len = strlen(name);
  if (len == 0) return -EEXIST;
  rc = diri((struct filsys *) fs->data, &name, &len, &parent);
  if (rc < 0) return rc;

  rc = find_dir_entry(parent, name, len, &ino);
  if (rc != -ENOENT) {
    release_inode(parent);
    return rc >= 0 ? -EEXIST : rc;
  }

  dir = alloc_inode(parent, S_IFDIR | (mode & S_IRWXUGO));
  if (!dir) {
    release_inode(parent);
    return -ENOSPC;
  }

  rc = add_dir_entry(parent, name, len, dir->ino);
  if (rc < 0) {
    unlink_inode(dir);
    release_inode(dir);
    release_inode(parent);
    return rc;
  }

  release_inode(dir);
  release_inode(parent);
  return 0;
}

int dfs_rmdir(struct fs *fs, char *name) {
  struct inode *parent;
  struct inode *dir;
  ino_t ino;
  int len;
  int rc;

  len = strlen(name);
  rc = diri((struct filsys *) fs->data, &name, &len, &parent);
  if (rc < 0) return rc;

  rc = find_dir_entry(parent, name, len, &ino);
  if (rc < 0) {
    release_inode(parent);
    return rc;
  }

  if (ino == DFS_INODE_ROOT) {
    release_inode(parent);
    return -EPERM;
  }

  rc = get_inode(parent->fs, ino, &dir);
  if (rc < 0) {
    release_inode(parent);
    return rc;
  }

  if (!S_ISDIR(dir->desc->mode)) {
    release_inode(dir);
    release_inode(parent);
    return -ENOTDIR;
  }

  if (dir->desc->linkcount == 1) {
    if (dir->desc->size > 0) {
      release_inode(dir);
      release_inode(parent);
      return -ENOTEMPTY;
    }

    rc = delete_dir_entry(parent, name, len); 
    if (rc < 0) {
      release_inode(dir);
      release_inode(parent);
      return rc;
    }
  }

  rc = unlink_inode(dir);
  if (rc < 0) {
    release_inode(dir);
    release_inode(parent);
    return rc;
  }

  release_inode(dir);
  release_inode(parent);
  return 0;
}

int dfs_rename(struct fs *fs, char *oldname, char *newname) {
  struct inode *oldparent;
  struct inode *newparent;
  int oldlen;
  int newlen;
  ino_t ino;
  int rc;

  oldlen = strlen(oldname);
  rc = diri((struct filsys *) fs->data, &oldname, &oldlen, &oldparent);
  if (rc < 0) return rc;

  rc = find_dir_entry(oldparent, oldname, oldlen, &ino);
  if (rc < 0) {
    release_inode(oldparent);
    return rc;
  }

  newlen = strlen(newname);
  rc = diri((struct filsys *) fs->data, &newname, &newlen, &newparent);
  if (rc < 0) {
    release_inode(oldparent);
    return rc;
  }

  rc = find_dir_entry(newparent, newname, newlen, NULL);
  if (rc != -ENOENT) {
    release_inode(oldparent);
    release_inode(newparent);
    if (strcmp(oldname, newname) == 0) return 0;
    return rc >= 0 ? -EEXIST : rc;
  }

  if (checki(oldparent, S_IWRITE) < 0 || checki(newparent, S_IWRITE) < 0) {
    release_inode(oldparent);
    release_inode(newparent);
    return -EACCES;
  }

  rc = add_dir_entry(newparent, newname, newlen, ino); 
  if (rc < 0) {
    release_inode(oldparent);
    release_inode(newparent);
    return rc;
  }

  rc = delete_dir_entry(oldparent, oldname, oldlen);
  if (rc < 0) {
    release_inode(oldparent);
    release_inode(newparent);
    return rc;
  }

  release_inode(oldparent);
  release_inode(newparent);
  return 0;
}

int dfs_link(struct fs *fs, char *oldname, char *newname) {
  struct inode *inode;
  struct inode *parent;
  int len;
  int rc;

  rc = namei((struct filsys *) fs->data, oldname, &inode);
  if (rc < 0) return rc;

  len = strlen(newname);
  rc = diri((struct filsys *) fs->data, &newname, &len, &parent);
  if (rc < 0) {
    release_inode(inode);
    return rc;
  }

  rc = find_dir_entry(parent, newname, len, NULL);
  if (rc != -ENOENT) {
    release_inode(inode);
    release_inode(parent);
    return rc >= 0 ? -EEXIST : rc;
  }

  inode->desc->linkcount++;
  mark_inode_dirty(inode);

  rc = add_dir_entry(parent, newname, len, inode->ino);
  if (rc < 0) {
    unlink_inode(inode);
    release_inode(inode);
    release_inode(parent);
    return rc;
  }

  release_inode(inode);
  release_inode(parent);
  return 0;
}

int dfs_unlink(struct fs *fs, char *name) {
  struct inode *dir;
  struct inode *inode;
  ino_t ino;
  int len;
  int rc;

  len = strlen(name);
  rc = diri((struct filsys *) fs->data, &name, &len, &dir);
  if (rc < 0) return rc;

  rc = find_dir_entry(dir, name, len, &ino);
  if (rc < 0) {
    release_inode(dir);
    return rc;
  }

  rc = get_inode(dir->fs, ino, &inode);
  if (rc < 0) {
    release_inode(dir);
    return rc;
  }

  if (S_ISDIR(inode->desc->mode)) {
    release_inode(inode);
    release_inode(dir);
    return -EISDIR;
  }

  rc = delete_dir_entry(dir, name, len);
  if (rc < 0) {
    release_inode(inode);
    release_inode(dir);
    return rc;
  }

  rc = unlink_inode(inode); 
  if (rc < 0) {
    release_inode(inode);
    release_inode(dir);
    return rc;
  }

  release_inode(inode);
  release_inode(dir);
  return 0;
}

int dfs_chmod(struct fs *fs, char *name, int mode) {
  struct thread *thread = self();
  struct inode *inode;
  int rc;

  rc = namei((struct filsys *) fs->data, name, &inode);
  if (rc < 0) return rc;

  if (thread->euid != 0 && thread->euid != inode->desc->uid) {
    release_inode(inode);
    return -EPERM;
  }

  inode->desc->mode = (inode->desc->mode & ~S_IRWXUGO) | (mode & S_IRWXUGO);

  mark_inode_dirty(inode);

  release_inode(inode);
  return 0;
}

int dfs_chown(struct fs *fs, char *name, int owner, int group) {
  struct thread *thread = self();
  struct inode *inode;
  int rc;

  rc = namei((struct filsys *) fs->data, name, &inode);
  if (rc < 0) return rc;

  if (thread->euid != 0) {
    release_inode(inode);
    return -EPERM;
  }

  if (owner != -1) inode->desc->uid = owner;
  if (group != -1) inode->desc->gid = group;

  mark_inode_dirty(inode);

  release_inode(inode);
  return 0;
}
