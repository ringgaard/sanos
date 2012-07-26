//
// stat.h
//
// Defines structure used by stat() and fstat()
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

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef SYS_STAT_H
#define SYS_STAT_H

#include <sys/types.h>

#ifndef _STAT_DEFINED
#define _STAT_DEFINED

struct stat {
  dev_t st_dev;
  ino_t st_ino;
  mode_t st_mode;
  nlink_t st_nlink;
  uid_t st_uid;
  gid_t st_gid;
  dev_t st_rdev;
  loff_t st_size;
  time_t st_atime;
  time_t st_mtime;
  time_t st_ctime;
};

struct stat64 {
  dev_t st_dev;
  ino_t st_ino;
  mode_t st_mode;
  nlink_t st_nlink;
  uid_t st_uid;
  gid_t st_gid;
  dev_t st_rdev;
  off64_t st_size;
  time_t st_atime;
  time_t st_mtime;
  time_t st_ctime;
};

#endif

#ifndef S_IFMT

#define S_IFMT         0170000         // File type mask
#define S_IFPKT        0160000         // Packet device
#define S_IFSOCK       0140000         // Socket
#define S_IFLNK        0120000         // Symbolic link
#define S_IFREG        0100000         // Regular file
#define S_IFBLK        0060000         // Block device
#define S_IFDIR        0040000         // Directory
#define S_IFCHR        0020000         // Character device
#define S_IFIFO        0010000         // Pipe

#define S_IREAD        0000400         // Read permission, owner
#define S_IWRITE       0000200         // Write permission, owner
#define S_IEXEC        0000100         // Execute/search permission, owner

#define S_ISLNK(m)      (((m) & S_IFMT) == S_IFLNK)
#define S_ISREG(m)      (((m) & S_IFMT) == S_IFREG)
#define S_ISDIR(m)      (((m) & S_IFMT) == S_IFDIR)
#define S_ISCHR(m)      (((m) & S_IFMT) == S_IFCHR)
#define S_ISBLK(m)      (((m) & S_IFMT) == S_IFBLK)
#define S_ISFIFO(m)     (((m) & S_IFMT) == S_IFIFO)
#define S_ISSOCK(m)     (((m) & S_IFMT) == S_IFSOCK)
#define S_ISPKT(m)      (((m) & S_IFMT) == S_IFPKT)

#define S_IRWXU 00700
#define S_IRUSR 00400
#define S_IWUSR 00200
#define S_IXUSR 00100

#define S_IRWXG 00070
#define S_IRGRP 00040
#define S_IWGRP 00020
#define S_IXGRP 00010

#define S_IRWXO 00007
#define S_IROTH 00004
#define S_IWOTH 00002
#define S_IXOTH 00001

#define S_IRWXUGO 00777

#endif

#ifdef  __cplusplus
extern "C" {
#endif

#ifdef LARGEFILES
#define fstat fstat64
#define stat stat64
#define lstat lstat64
#else
osapi int fstat(handle_t f, struct stat *buffer);
osapi int stat(const char *name, struct stat *buffer);
osapi int lstat(const char *name, struct stat *buffer);
#endif

osapi int fstat64(handle_t f, struct stat64 *buffer);
osapi int stat64(const char *name, struct stat64 *buffer);
osapi int lstat64(const char *name, struct stat64 *buffer);

osapi int chmod(const char *name, int mode);
osapi int fchmod(handle_t f, int mode);

osapi int mkdir(const char *name, int mode);

#ifdef  __cplusplus
}
#endif

#endif
