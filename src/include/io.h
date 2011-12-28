//
// io.h
//
// Low-level file handling and I/O functions
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

#ifndef IO_H
#define IO_H

#include <sys/types.h>

#ifdef  __cplusplus
extern "C" {
#endif

osapi handle_t open(const char *name, int flags, ...);
osapi handle_t sopen(const char *name, int flags, int shflags, ...);
osapi handle_t creat(const char *name, int mode);
osapi int close(handle_t h);

osapi int read(handle_t f, void *data, size_t size);
osapi int write(handle_t f, const void *data, size_t size);

osapi handle_t dup(handle_t h);
osapi handle_t dup2(handle_t h1, handle_t h2);

osapi loff_t tell(handle_t f);
osapi loff_t lseek(handle_t f, loff_t offset, int origin);
osapi loff_t filelength(handle_t f);
osapi off64_t filelength64(handle_t f);

osapi int access(const char *name, int mode);
osapi int umask(int mode);
osapi int eof(handle_t f);
osapi int setmode(handle_t f, int mode);

#ifdef  __cplusplus
}
#endif

#endif
