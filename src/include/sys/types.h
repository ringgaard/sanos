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

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef TYPES_H
#define TYPES_H

#ifndef _SIZE_T_DEFINED
#define _SIZE_T_DEFINED
typedef unsigned int size_t;
#endif

#ifndef _TIME_T_DEFINED
#define _TIME_T_DEFINED
typedef long time_t;
#endif

#ifndef _CLOCK_T_DEFINED
#define _CLOCK_T_DEFINED
typedef long clock_t;
#endif

#ifndef _INO_T_DEFINED
#define _INO_T_DEFINED
typedef unsigned int ino_t;
#endif

#ifndef _DEV_T_DEFINED
#define _DEV_T_DEFINED
typedef unsigned int dev_t;
#endif

#ifndef _BLKNO_T_DEFINED
#define _BLKNO_T_DEFINED
typedef unsigned int blkno_t;
#endif

#ifndef _LOFF_T_DEFINED
#define _LOFF_T_DEFINED
typedef long loff_t;
#endif

#ifndef _OFF64_T_DEFINED
#define _OFF64_T_DEFINED
typedef __int64 off64_t;
#endif

#ifndef _HANDLE_T_DEFINED
#define _HANDLE_T_DEFINED
typedef int handle_t;
#endif

#ifndef _TID_T_DEFINED
#define _TID_T_DEFINED
typedef unsigned long tid_t;
#endif

#ifndef _PID_T_DEFINED
#define _PID_T_DEFINED
typedef unsigned long pid_t;
#endif

#ifndef _HMODULE_T_DEFINED
#define _HMODULE_T_DEFINED
typedef void *hmodule_t;
#endif

#ifndef _TLS_T_DEFINED
#define _TLS_T_DEFINED
typedef unsigned long tls_t;
#endif

#ifndef _WCHAR_T_DEFINED
#define _WCHAR_T_DEFINED
typedef unsigned short wchar_t;
#endif

typedef int port_t;
typedef int err_t;

typedef __int64 systime_t;

#ifndef NULL
#define NULL ((void *) 0)
#endif

#define K 1024
#define M (K * K)

#define FALSE 0
#define TRUE  1

#ifndef NOHANDLE
#define NOHANDLE ((handle_t) -1)
#endif

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;
typedef unsigned __int64 QWORD;

#endif
