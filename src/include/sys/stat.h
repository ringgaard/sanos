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

#ifndef STAT_H
#define STAT_H

#ifndef osapi
#define osapi __declspec(dllimport)
#endif

#ifndef _TIME_T_DEFINED
#define _TIME_T_DEFINED
typedef long time_t;
#endif

#ifndef _INO_T_DEFINED
#define _INO_T_DEFINED
typedef unsigned int ino_t;
#endif

#ifndef _DEVNO_T_DEFINED
#define _DEVNO_T_DEFINED
typedef unsigned int devno_t;
#endif

#ifndef _HANDLE_T_DEFINED
#define _HANDLE_T_DEFINED
typedef int handle_t;
#endif

#ifndef _STAT_DEFINED
#define _STAT_DEFINED

struct stat
{
  int mode;
  ino_t ino;
  int nlink;
  devno_t devno;
  time_t atime;
  time_t mtime;
  time_t ctime;
  union
  {
    struct
    {
      unsigned long size_low;
      unsigned long size_high;
    } quad;
    unsigned __int64 size;
  };
};

#endif

#ifndef S_IFMT

#define S_IFMT          0xF000         // File type mask
#define S_IFIFO         0x1000         // Pipe
#define S_IFCHR         0x2000         // Character device
#define S_IFDIR         0x4000         // Directory
#define S_IFBLK         0x6000         // Block device
#define S_IFREG         0x8000         // Regular file

#define S_IREAD         0x0100         // Read permission
#define S_IWRITE        0x0080         // Write permission
#define S_IEXEC         0x0040         // Execute/search permission

#endif

#define S_ISREG(m)	(((m) & S_IFMT) == S_IFREG)
#define S_ISDIR(m)	(((m) & S_IFMT) == S_IFDIR)
#define S_ISCHR(m)	(((m) & S_IFMT) == S_IFCHR)
#define S_ISBLK(m)	(((m) & S_IFMT) == S_IFBLK)
#define S_ISFIFO(m)	(((m) & S_IFMT) == S_IFIFO)

osapi int fstat(handle_t f, struct stat *buffer);
osapi int stat(const char *name, struct stat *buffer);

#endif
