//
// syscall.h
//
// System call numbers
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

#ifndef SYSCALL_H
#define SYSCALL_H

#define SYSCALL_NULL           0
#define SYSCALL_MKFS           1
#define SYSCALL_MOUNT          2
#define SYSCALL_UMOUNT         3
#define SYSCALL_OPEN           4
#define SYSCALL_CLOSE          5
#define SYSCALL_FSYNC          6
#define SYSCALL_READ           7
#define SYSCALL_WRITE          8
#define SYSCALL_TELL           9
#define SYSCALL_LSEEK          10
#define SYSCALL_FTRUNCATE      11
#define SYSCALL_FSTAT          12
#define SYSCALL_STAT           13
#define SYSCALL_MKDIR          14
#define SYSCALL_RMDIR          15
#define SYSCALL_RENAME         16
#define SYSCALL_LINK           17
#define SYSCALL_UNLINK         18
#define SYSCALL_OPENDIR        19
#define SYSCALL_READDIR        20
#define SYSCALL_VMALLOC        21
#define SYSCALL_VMFREE         22
#define SYSCALL_VMREALLOC      23
#define SYSCALL_VMPROTECT      24
#define SYSCALL_VMLOCK         25
#define SYSCALL_VMUNLOCK       26
#define SYSCALL_WAITONE        27
#define SYSCALL_MKEVENT        28
#define SYSCALL_EPULSE         29
#define SYSCALL_ESET           30
#define SYSCALL_ERESET         31
#define SYSCALL_GETTHREADBLOCK 32
#define SYSCALL_EXITOS         33
#define SYSCALL_DUP            34
#define SYSCALL_MKTHREAD       35
#define SYSCALL_SUSPEND        36
#define SYSCALL_RESUME         37
#define SYSCALL_ENDTHREAD      38
#define SYSCALL_SETCONTEXT     39  
#define SYSCALL_GETCONTEXT     40
#define SYSCALL_GETPRIO        41
#define SYSCALL_SETPRIO        42
#define SYSCALL_MSLEEP         43
#define SYSCALL_TIME           44
#define SYSCALL_GETTIMEOFDAY   45
#define SYSCALL_CLOCK          46
#define SYSCALL_MKSEM          47
#define SYSCALL_SEMREL         48
#define SYSCALL_IOCTL          49
#define SYSCALL_GETFSSTAT      50
#define SYSCALL_FSTATFS        51
#define SYSCALL_STATFS         52
#define SYSCALL_FUTIME         53
#define SYSCALL_UTIME          54
#define SYSCALL_SETTIMEOFDAY   55
#define SYSCALL_ACCEPT         56
#define SYSCALL_BIND           57
#define SYSCALL_CONNECT        58
#define SYSCALL_GETPEERNAME    59
#define SYSCALL_GETSOCKNAME    60
#define SYSCALL_GETSOCKOPT     61
#define SYSCALL_LISTEN         62
#define SYSCALL_RECV           63
#define SYSCALL_RECVFROM       64
#define SYSCALL_SEND           65
#define SYSCALL_SENDTO         66
#define SYSCALL_SETSOCKOPT     67
#define SYSCALL_SHUTDOWN       68
#define SYSCALL_SOCKET         69
#define SYSCALL_WAITALL        70
#define SYSCALL_WAITANY        71
#define SYSCALL_READV          72
#define SYSCALL_WRITEV         73
#define SYSCALL_CHDIR          74
#define SYSCALL_MKIOMUX        75
#define SYSCALL_DISPATCH       76
#define SYSCALL_RECVMSG        77
#define SYSCALL_SENDMSG        78
#define SYSCALL_SELECT         79
#define SYSCALL_PIPE           80
#define SYSCALL_DUP2           81
#define SYSCALL_SETMODE        82
#define SYSCALL_CHMOD          83
#define SYSCALL_FCHMOD         84
#define SYSCALL_SYSINFO        85
#define SYSCALL_MKMUTEX        86
#define SYSCALL_MUTEXREL       87
#define SYSCALL_PREAD          88
#define SYSCALL_PWRITE         89
#define SYSCALL_GETUID         90
#define SYSCALL_SETUID         91
#define SYSCALL_GETGID         92
#define SYSCALL_SETGID         93
#define SYSCALL_GETEUID        94
#define SYSCALL_SETEUID        95
#define SYSCALL_GETEGID        96
#define SYSCALL_SETEGID        97
#define SYSCALL_GETGROUPS      98
#define SYSCALL_SETGROUPS      99
#define SYSCALL_CHOWN         100
#define SYSCALL_FCHOWN        101
#define SYSCALL_ACCESS        102
#define SYSCALL_POLL          103
#define SYSCALL_GETCWD        104
#define SYSCALL_SENDSIG       105
#define SYSCALL_SIGPROCMASK   106
#define SYSCALL_SIGPENDING    107
#define SYSCALL_ALARM         108
#define SYSCALL_VMMAP         109
#define SYSCALL_VMSYNC        110

#define SYSCALL_MAX           110

#endif
