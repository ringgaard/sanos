//
// syscall.h
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// System call numbers
//

#ifndef SYSCALL_H
#define SYSCALL_H

#define SYSCALL_NULL         0
#define SYSCALL_FORMAT       1
#define SYSCALL_MOUNT        2
#define SYSCALL_UNMOUNT      3
#define SYSCALL_OPEN         4
#define SYSCALL_CLOSE        5
#define SYSCALL_FLUSH        6
#define SYSCALL_READ         7
#define SYSCALL_WRITE        8
#define SYSCALL_TELL         9
#define SYSCALL_LSEEK        10
#define SYSCALL_CHSIZE       11
#define SYSCALL_FSTAT        12
#define SYSCALL_STAT         13
#define SYSCALL_MKDIR        14
#define SYSCALL_RMDIR        15
#define SYSCALL_RENAME       16
#define SYSCALL_LINK         17
#define SYSCALL_UNLINK       18
#define SYSCALL_OPENDIR      19
#define SYSCALL_READDIR      20
#define SYSCALL_MMAP         21
#define SYSCALL_MUNMAP       22
#define SYSCALL_MREMAP       23
#define SYSCALL_MPROTECT     24
#define SYSCALL_MLOCK        25
#define SYSCALL_MUNLOCK      26
#define SYSCALL_WAIT         27
#define SYSCALL_MKEVENT      28
#define SYSCALL_EPULSE       29
#define SYSCALL_ESET         30
#define SYSCALL_ERESET       31
#define SYSCALL_SELF         32
#define SYSCALL_EXIT         33
#define SYSCALL_DUP          34
#define SYSCALL_MKTHREAD     35
#define SYSCALL_SUSPEND      36
#define SYSCALL_RESUME       37
#define SYSCALL_ENDTHREAD    38
#define SYSCALL_GETCONTEXT   39
#define SYSCALL_GETPRIO      40
#define SYSCALL_SETPRIO      41
#define SYSCALL_SLEEP        42
#define SYSCALL_TIME         43
#define SYSCALL_GETTIMEOFDAY 44
#define SYSCALL_CLOCK        45
#define SYSCALL_MKSEM        46
#define SYSCALL_SEMREL       47
#define SYSCALL_IOCTL        48
#define SYSCALL_GETFSSTAT    49
#define SYSCALL_FSTATFS      50
#define SYSCALL_STATFS       51
#define SYSCALL_FUTIME       52
#define SYSCALL_UTIME        53
#define SYSCALL_SETTIMEOFDAY 54
#define SYSCALL_ACCEPT       55
#define SYSCALL_BIND         56
#define SYSCALL_CONNECT      57
#define SYSCALL_LISTEN       58
#define SYSCALL_RECV         59
#define SYSCALL_RECVFROM     60
#define SYSCALL_SEND         61
#define SYSCALL_SENDTO       62
#define SYSCALL_SOCKET       63

#define SYSCALL_MAX          63

#endif
