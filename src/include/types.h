//
// types.h
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Basic type definitions
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

#define ULONG_MAX   0xffffffffUL
#define LONG_MIN    (-2147483647L - 1)
#define LONG_MAX    2147483647L

#endif
