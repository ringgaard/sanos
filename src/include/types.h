//
// types.h
//
// Basic type definitions
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

#ifndef TYPES_H
#define TYPES_H

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;
typedef unsigned __int64 QWORD;

typedef long time_t;
typedef unsigned int size_t;
typedef unsigned int ino_t;
typedef unsigned int devno_t;
typedef unsigned int blkno_t;
typedef unsigned int loff_t;

typedef int port_t;
typedef int err_t;

typedef int handle_t;
typedef void *hmodule_t;
typedef unsigned long tls_t;

typedef __int64 systime_t;
typedef unsigned long clock_t;

typedef unsigned long tid_t;
typedef unsigned long pid_t;

#ifndef NULL
#define NULL ((void *) 0)
#endif

#define K 1024
#define M (K * K)

#define FALSE 0
#define TRUE  1

#define NOHANDLE ((handle_t) -1)

#define ULONG_MAX   0xFFFFFFFFUL
#define LONG_MIN    (-2147483647L - 1)
#define LONG_MAX    2147483647L

#endif
