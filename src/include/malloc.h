//
// malloc.h
//
// Heap allocation routines
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

#ifndef MALLOC_H
#define MALLOC_H

#include <sys/types.h>

#ifdef  __cplusplus
extern "C" {
#endif

#ifdef USE_LOCAL_HEAP

osapi void *_lmalloc(size_t size);
osapi void *_lrealloc(void *mem, size_t size);
osapi void *_lcalloc(size_t num, size_t size);
osapi void _lfree(void *p);

#define malloc(n) _lmalloc(n)
#define realloc(p, n) _lrealloc((p), (n))
#define calloc(n, s) _lcalloc((n), (s))
#define free(p) _lfree(p)

#else
osapi void *malloc(size_t size);
osapi void *realloc(void *mem, size_t size);
osapi void *calloc(size_t num, size_t size);
osapi void free(void *p);
#endif

void  *_alloca(size_t size);
#define alloca _alloca

#ifdef  __cplusplus
}
#endif

#endif
