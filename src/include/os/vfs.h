//
// vfs.h
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

#ifndef VFS_H
#define VFS_H

#define VFS_LOCK_TIMEOUT       60000    // Timeout for file system locks

#define F_MODIFIED              0x1000  // File has been modified since it was opened
#define F_DIR                   0x2000  // File is a directory

#define FSOP_FORMAT     0x00000001
#define FSOP_MOUNT      0x00000002
#define FSOP_UMOUNT     0x00000004
#define FSOP_STATFS     0x00000008
#define FSOP_OPEN       0x00000010
#define FSOP_CLOSE      0x00000020
#define FSOP_FLUSH      0x00000040
#define FSOP_READ       0x00000080
#define FSOP_WRITE      0x00000100
#define FSOP_IOCTL      0x00000200
#define FSOP_TELL       0x00000400
#define FSOP_LSEEK      0x00000800
#define FSOP_CHSIZE     0x00001000
#define FSOP_FUTIME     0x00002000
#define FSOP_UTIME      0x00004000
#define FSOP_FSTAT      0x00008000
#define FSOP_STAT       0x00010000
#define FSOP_MKDIR      0x00020000
#define FSOP_RMDIR      0x00040000
#define FSOP_RENAME     0x00080000
#define FSOP_LINK       0x00100000
#define FSOP_UNLINK     0x00200000
#define FSOP_OPENDIR    0x00400000
#define FSOP_READDIR    0x00800000

struct filesystem
{
  char *name;
  struct fsops *ops;
  struct filesystem *next;
};

struct fs
{
  int locks;
  struct mutex exclusive;
  char mntfrom[MAXPATH];
  char mntto[MAXPATH];
  struct fsops *ops;
  struct fs *next;
  struct fs *prev;
  void *data;
  struct filesystem *fsys;
};

struct file
{
  struct object object;

  struct fs *fs;
  int flags;
  loff_t pos;
  void *data;
  char *path;
};

struct fsops
{
  unsigned long reentrant;

  int (*lockfs)(struct fs *fs);
  void (*unlockfs)(struct fs *fs);

  int (*format)(char *devname, char *opts);
  int (*mount)(struct fs *fs, char *opts);
  int (*umount)(struct fs *fs);

  int (*statfs)(struct fs *fs, struct statfs *buf);

  int (*open)(struct file *filp, char *name);
  int (*close)(struct file *filp);
  int (*flush)(struct file *filp);

  int (*read)(struct file *filp, void *data, size_t size);
  int (*write)(struct file *filp, void *data, size_t size);
  int (*ioctl)(struct file *filp, int cmd, void *data, size_t size);

  loff_t (*tell)(struct file *filp);
  loff_t (*lseek)(struct file *filp, loff_t offset, int origin);
  int (*chsize)(struct file *filp, loff_t size);

  int (*futime)(struct file *filp, struct utimbuf *times);
  int (*utime)(struct fs *fs, char *name, struct utimbuf *times);

  int (*fstat)(struct file *filp, struct stat *buffer);
  int (*stat)(struct fs *fs, char *name, struct stat *buffer);

  int (*mkdir)(struct fs *fs, char *name);
  int (*rmdir)(struct fs *fs, char *name);

  int (*rename)(struct fs *fs, char *oldname, char *newname);
  int (*link)(struct fs *fs, char *oldname, char *newname);
  int (*unlink)(struct fs *fs, char *name);
  
  int (*opendir)(struct file *filp, char *name);
  int (*readdir)(struct file *filp, struct dirent *dirp, int count);
};

#ifdef KERNEL

char curdir[MAXPATH];

int init_vfs();
int fnmatch(char *fn1, int len1, char *fn2, int len2);
krnlapi struct filesystem *register_filesystem(char *name, struct fsops *ops);
krnlapi struct fs *fslookup(char *name, char **rest);

krnlapi int format(char *devname, char *type, char *opts);
krnlapi int mount(char *type, char *mntto, char *mntfrom, char *opts);
krnlapi int umount(char *path);
int umount_all();

krnlapi int getfsstat(struct statfs *buf, size_t size);
krnlapi int fstatfs(struct file *filp, struct statfs *buf);
krnlapi int statfs(char *name, struct statfs *buf);

krnlapi int open(char *name, int mode, struct file **retval);
krnlapi int close(struct file *filp);
krnlapi int flush(struct file *filp);

krnlapi int read(struct file *filp, void *data, size_t size);
krnlapi int write(struct file *filp, void *data, size_t size);
krnlapi int ioctl(struct file *filp, int cmd, void *data, size_t size);

krnlapi int readv(struct file *filp, struct iovec *iov, int count);
krnlapi int writev(struct file *filp, struct iovec *iov, int count);

krnlapi loff_t tell(struct file *filp);
krnlapi loff_t lseek(struct file *filp, loff_t offset, int origin);
krnlapi int chsize(struct file *filp, loff_t size);

krnlapi int futime(struct file *filp, struct utimbuf *times);
krnlapi int utime(char *name, struct utimbuf *times);

krnlapi int fstat(struct file *filp, struct stat *buffer);
krnlapi int stat(char *name, struct stat *buffer);

krnlapi int chdir(char *name);
krnlapi int mkdir(char *name);
krnlapi int rmdir(char *name);

krnlapi int rename(char *oldname, char *newname);
krnlapi int link(char *oldname, char *newname);
krnlapi int unlink(char *name);

krnlapi int opendir(char *name, struct file **retval);
krnlapi int readdir(struct file *filp, struct dirent *dirp, int count);

#endif

#endif
