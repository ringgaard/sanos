//
// mman.h
//
// Memory-mapped files
//
// Copyright (C) 2013 Michael Ringgaard. All rights reserved.
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

#ifndef SYS_MMAN_H
#define SYS_MMAN_H

#include <sys/types.h>

#define MAP_FAILED NULL

//
// Protection options
//

#define PROT_NONE     0     // Page cannot be accessed
#define PROT_READ     1     // Page can be read
#define PROT_WRITE    2     // Page can be written
#define PROT_EXEC     4     // Page can be executed

//
// Sharing flags
//

#define MAP_PRIVATE   1     // Changes are private
#define MAP_SHARED    2     // Share changes
#define MAP_FIXED     4     // Allocate at fixed address
#define MAP_ANONYMOUS 8     // Anonymous mapping that is not backed by any file
#define MAP_FILE      16    // File mapping

#define MAP_ANON      MAP_ANONYMOUS

//
// Synchronization flags
//

#define MS_ASYNC       1     // Perform asynchronous writes
#define MS_SYNC        2     // Perform synchronous writes
#define MS_INVALIDATE  4     // Invalidate mappings

//
// Process memory locking options
//

#define MCL_CURRENT    1     // Lock currently mapped pages
#define MCL_FUTURE     2     // Lock pages that become mapped

#ifdef  __cplusplus
extern "C" {
#endif

void *mmap(void *addr, size_t size, int prot, int flags, handle_t f, off_t offset);
int munmap(void *addr, size_t size);
int msync(void *addr, size_t size, int flags);
int mprotect(void *addr, size_t size, int prot);
int mlock(const void *addr, size_t size);
int munlock(const void *addr, size_t size);
int mlockall(int flags);
int munlockall(void);

#ifdef  __cplusplus
}
#endif

#endif
