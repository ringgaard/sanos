//
// procfs.c
//
// Kernel Information Filesystem
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

struct proc_inode *proc_list_head;
struct proc_inode *proc_list_tail;
ino_t next_procino = PROC_ROOT_INODE + 1;

int procfs_open(struct file *filp, char *name);
int procfs_close(struct file *filp);

int procfs_read(struct file *filp, void *data, size_t size, off64_t pos);

off64_t procfs_tell(struct file *filp);
off64_t procfs_lseek(struct file *filp, off64_t offset, int origin);

int procfs_fstat(struct file *filp, struct stat64 *buffer);
int procfs_stat(struct fs *fs, char *name, struct stat64 *buffer);

int procfs_opendir(struct file *filp, char *name);
int procfs_readdir(struct file *filp, struct direntry *dirp, int count);

struct fsops procfsops = {
  FSOP_OPEN | FSOP_CLOSE | FSOP_READ | FSOP_TELL | FSOP_LSEEK | FSOP_STAT | FSOP_FSTAT | FSOP_OPENDIR | FSOP_READDIR,

  NULL,
  NULL,

  NULL,
  NULL,
  NULL,

  NULL,

  procfs_open,
  procfs_close,
  NULL,
  NULL,

  procfs_read,
  NULL,
  NULL,

  procfs_tell,
  procfs_lseek,
  NULL,

  NULL,
  NULL,

  procfs_fstat,
  procfs_stat,

  NULL,

  NULL,
  NULL,
  NULL,
  NULL,

  NULL,
  NULL,

  NULL,
  NULL,
  NULL,

  procfs_opendir,
  procfs_readdir
};

static struct proc_inode *find_proc(char *name) {
  struct proc_inode *inode = proc_list_head;
  int namelen = strlen(name);

  while (inode) {
    if (fnmatch(name, namelen, inode->name, inode->namelen)) return inode;
    inode = inode->next;
  }

  return NULL;
}

static void free_proc_file(struct proc_file *pf) {
  struct proc_blk *blk;
  struct proc_blk *next;

  if (!pf) return;
  blk = pf->blkhead;
  while (blk) {
    next = blk->next;
    kfree(blk);
    blk = next;
  }

  kfree(pf);
}

void init_procfs() {
  register_filesystem("procfs", &procfsops);
}

int register_proc_inode(char *name, proc_t proc, void *arg) {
  struct proc_inode *inode;

  inode = (struct proc_inode *) kmalloc(sizeof(struct proc_inode));
  if (!inode) return -ENOMEM;

  inode->ino = next_procino++;
  inode->name = name;
  inode->namelen = strlen(name);
  inode->proc = proc;
  inode->arg = arg;
  inode->size = 0;

  if (proc_list_tail) {
    proc_list_tail->next = inode;
    proc_list_tail = inode;
  } else {
    proc_list_head = proc_list_tail = inode;
  }

  return 0;
}

int proc_write(struct proc_file *pf, void *buffer, size_t size) {
  char *ptr = (char *) buffer;
  size_t left = size;
  struct proc_blk *blk;
  size_t count;

  while (left > 0) {
    if (!pf->blktail || pf->blktail->size == PROC_BLKSIZE) {
      blk = (struct proc_blk *) kmalloc(sizeof(struct proc_blk));
      if (!blk) return -ENOMEM;

      blk->next = NULL;
      blk->size = 0;

      if (pf->blktail) {
        pf->blktail->next = blk;
        pf->blktail = blk;
      } else {
        pf->blkhead = pf->blktail = blk;
      }
    } else {
      blk = pf->blktail;
    }

    count = blk->size + left > PROC_BLKSIZE ? (size_t) (PROC_BLKSIZE - blk->size) : left;

    memcpy(blk->data + blk->size, ptr, count);
    blk->size += count;
    ptr += count;
    left -= count;
    pf->size += count;
  }

  return size;
}

int pprintf(struct proc_file *pf, const char *fmt, ...) {
  va_list args;
  char buffer[1024];
  int len;

  va_start(args, fmt);
  len = vsprintf(buffer, fmt, args);
  va_end(args);
    
  return proc_write(pf, buffer, len);
}

int procfs_open(struct file *filp, char *name) {
  struct proc_inode *inode;
  struct proc_file *pf;
  int rc;

  if (*name == PS1 || *name == PS2) name++;
  inode = find_proc(name);
  if (!inode) return -ENOENT;

  pf = (struct proc_file *) kmalloc(sizeof(struct proc_file));
  if (!pf) return -ENOMEM;

  pf->inode = inode;
  pf->blkhead = pf->blktail = NULL;
  pf->size = 0;

  rc = inode->proc(pf, inode->arg);
  if (rc < 0) {
    free_proc_file(pf);
    return rc;
  }

  inode->size = pf->size;
  filp->data = pf;
  filp->mode = S_IFCHR | S_IRUSR | S_IRGRP | S_IROTH;
  return 0;
}

int procfs_close(struct file *filp) {
  if (!(filp->flags & F_DIR)) {
    struct proc_file *pf = filp->data;
    free_proc_file(pf);
  }

  return 0;
}

int procfs_read(struct file *filp, void *data, size_t size, off64_t pos) {
  struct proc_file *pf = filp->data;
  struct proc_blk *blk;
  size_t start = 0;
  char *ptr = (char *) data;
  int left = size;
  int count;
  int offset;

  if (filp->flags & F_DIR) return -EINVAL;
  if (!size) return 0;

  blk = pf->blkhead;
  while (blk && start + blk->size <= pos) {
    start += blk->size;
    blk = blk->next;
  }

  if (blk) {
    offset = (int) pos - start;

    if (offset < 0 || offset >= PROC_BLKSIZE) {
      kprintf("invalid proc blk offset %d\n", offset);
      dbg_break();
    }

    if (left < blk->size - offset) {
      count = left;
    } else {
      count = blk->size - offset;
    }

    memcpy(ptr, blk->data + offset, count);
    ptr += count;
    left -= count;
    blk = blk->next;
  }

  while (left > 0 && blk) {
    if (left < blk->size) {
      count = left;
    } else {
      count = blk->size;
    }

    memcpy(ptr, blk->data, count);
    ptr += count;
    left -= count;
    blk = blk->next;
  }

  return size - left;
}

off64_t procfs_tell(struct file *filp) {
  return filp->pos;
}

off64_t procfs_lseek(struct file *filp, off64_t offset, int origin) {
  struct proc_file *pf = filp->data;

  if (filp->flags & F_DIR) return -EINVAL;

  switch (origin) {
    case SEEK_END:
      offset += pf->size;
      break;

    case SEEK_CUR:
      offset += filp->pos;
  }

  if (offset < 0 || offset > pf->size) return -EINVAL;

  filp->pos = offset;
  return offset;
}

int procfs_fstat(struct file *filp, struct stat64 *buffer) {
  struct proc_file *pf = filp->data;

  if (filp->flags & F_DIR) {
    if (buffer) {
      memset(buffer, 0, sizeof(struct stat64));
      buffer->st_mode = S_IFDIR | S_IRUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;
      buffer->st_ino = PROC_ROOT_INODE;
      buffer->st_nlink = 1;
      buffer->st_dev = NODEV;
      buffer->st_atime = buffer->st_mtime = buffer->st_ctime = get_time();
      buffer->st_size = 0;
    }

    return 0;
  }

  if (buffer) {
    memset(buffer, 0, sizeof(struct stat64));
    buffer->st_mode = S_IFCHR | S_IRUSR | S_IRGRP | S_IROTH;

    buffer->st_ino = pf->inode->ino;
    buffer->st_nlink = 1;
    buffer->st_dev = NODEV;

    buffer->st_atime = buffer->st_mtime = buffer->st_ctime = get_time();
    buffer->st_size = pf->size;
  }

  return pf->size;
}

int procfs_stat(struct fs *fs, char *name, struct stat64 *buffer) {
  struct proc_inode *inode;

  if (*name == PS1 || *name == PS2) name++;

  if (!*name) {
    if (buffer) {
      memset(buffer, 0, sizeof(struct stat64));
      buffer->st_mode = S_IFDIR | S_IRUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;
      buffer->st_ino = PROC_ROOT_INODE;
      buffer->st_nlink = 1;
      buffer->st_dev = NODEV;
      buffer->st_atime = buffer->st_mtime = buffer->st_ctime = get_time();
      buffer->st_size = 0;
    }

    return 0;
  }

  inode = find_proc(name);
  if (!inode) return -ENOENT;

  if (buffer) {
    memset(buffer, 0, sizeof(struct stat64));
    buffer->st_mode = S_IFCHR | S_IRUSR | S_IRGRP | S_IROTH;

    buffer->st_ino = inode->ino;
    buffer->st_nlink = 1;
    buffer->st_dev = NODEV;

    buffer->st_atime = buffer->st_mtime = buffer->st_ctime = get_time();
    buffer->st_size = inode->size;
  }

  return inode->size;
}

int procfs_opendir(struct file *filp, char *name) {
  if (*name == PS1 || *name == PS2) name++;
  if (*name) return -ENOENT;

  filp->data = proc_list_head;
  return 0;
}

int procfs_readdir(struct file *filp, struct direntry *dirp, int count) {
  struct proc_inode *inode = filp->data;

  if (!inode) return 0;

  dirp->ino = inode->ino;
  dirp->namelen = inode->namelen;
  dirp->reclen = sizeof(struct direntry) + dirp->namelen + 1;
  memcpy(dirp->name, inode->name, inode->namelen + 1);

  filp->pos++;
  filp->data = inode->next;
  return 1;
}
