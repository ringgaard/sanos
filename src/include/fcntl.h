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

//
// File open modes
//

#ifndef O_RDONLY

#define O_ACCMODE               0x0003  // Access mode
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

//
// File control commands
//

#define F_DUPFD   1       // Duplicate file handle
#define F_GETFD   2       // Get the file descriptor flags
#define F_SETFD   3       // Set the file descriptor flags
#define F_GETFL   4       // Get the file status flags and file access modes
#define F_SETFL   5       // Set the file status flags

#define F_GETLK   6       // Get the first lock
#define F_SETLK   7       // Set or clear a file segment lock
#define F_SETLKW  8       // Wait and set or clear a file segment lock

//
// File lock
//

#define F_RDLCK   1        // Shared or read lock
#define F_WRLCK   2        // Exclusive or write lock
#define F_UNLCK   3        // Unlock

struct flock {
  short l_type;            // Type of lock
  short l_whence;          // Flag for starting offset
  off_t l_start;           // Relative offset in bytes
  off_t l_len;             // Size; if 0 then until EOF
  pid_t l_pid;             // Process ID of the process holding the lock
};

#ifdef  __cplusplus
extern "C" {
#endif

osapi handle_t open(const char *name, int flags, ...);
osapi handle_t creat(const char *name, int mode);
int fcntl(handle_t f, int cmd, ...);

#ifdef  __cplusplus
}
#endif

#endif
