//
// procfs.h
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Kernel Information Filesystem
//

#ifndef PROCFS_H
#define PROCFS_H

#define PROC_BLKSIZE    4088
#define PROC_ROOT_INODE 0

struct proc_file;

typedef int (*proc_t)(struct proc_file *f, void *arg);

struct proc_inode
{
  char *name;
  int namelen;
  ino_t ino;
  proc_t proc;
  void *arg;
  size_t size;
  struct proc_inode *next;
};

struct proc_blk
{
  struct proc_blk *next;
  int size;
  char data[PROC_BLKSIZE];
};

struct proc_file
{
  struct proc_inode *inode;
  size_t size;
  struct proc_blk *blkhead;
  struct proc_blk *blktail;
};

void init_procfs();

krnlapi int register_proc_inode(char *name, proc_t proc, void *arg);
krnlapi int proc_write(struct proc_file *pf, void *buffer, size_t size);
krnlapi int pprintf(struct proc_file *pf, const char *fmt, ...);

#endif
