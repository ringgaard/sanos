//
// fcntl.h
//
// File Control
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

#ifndef FCNTL_H
#define FCNTL_H

#include <sys/types.h>

#ifndef O_RDONLY

#define O_RDONLY                0x0000  // Open for reading only
#define O_WRONLY                0x0001  // Open for writing only
#define O_RDWR                  0x0002  // Open for reading and writing
#define O_SPECIAL               0x0004  // Open for special access
#define O_APPEND                0x0008  // Writes done at EOF

#define O_RANDOM                0x0010  // File access is primarily random
#define O_SEQUENTIAL            0x0020  // File access is primarily sequential
#define O_TEMPORARY             0x0040  // Temporary file bit
#define O_NOINHERIT             0x0080  // Child process doesn't inherit file

#define O_CREAT                 0x0100  // Create and open file
#define O_TRUNC                 0x0200  // Truncate file
#define O_EXCL                  0x0400  // Open only if file doesn't already exist
#define O_DIRECT                0x0800  // Do not use cache for reads and writes

#define O_SHORT_LIVED           0x1000  // Temporary storage file, try not to flush
#define O_NONBLOCK              0x2000  // Open in non-blocking mode
#define O_TEXT                  0x4000  // File mode is text (translated)
#define O_BINARY                0x8000  // File mode is binary (untranslated)

#endif

#ifndef R_OK

#define R_OK    4               // Test for read permission
#define W_OK    2               // Test for write permission
#define X_OK    1               // Test for execute permission
#define F_OK    0               // Test for existence

#endif

#ifndef SEEK_SET

#define SEEK_SET                0       // Seek relative to begining of file
#define SEEK_CUR                1       // Seek relative to current file position
#define SEEK_END                2       // Seek relative to end of file

#endif

#ifdef  __cplusplus
extern "C" {
#endif

osapi handle_t open(const char *name, int flags, ...);
osapi handle_t creat(const char *name, int mode);

#ifdef  __cplusplus
}
#endif

#endif
