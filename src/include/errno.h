//
// errno.h
//
// Error codes
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

#ifndef ERRNO_H
#define ERRNO_H

#include <sys/types.h>

#ifndef _ERRORS_DEFINED
#define _ERRORS_DEFINED

#define EPERM           1                // Operation not permitted
#define ENOENT          2                // No such file or directory
#define ESRCH           3                // No such process
#define EINTR           4                // Interrupted system call
#define EIO             5                // Input/output error
#define ENXIO           6                // Device not configured
#define E2BIG           7                // Argument list too long
#define ENOEXEC         8                // Exec format error
#define EBADF           9                // Bad file number
#define ECHILD          10               // No spawned processes
#define EAGAIN          11               // Resource temporarily unavailable
#define ENOMEM          12               // Cannot allocate memory
#define EACCES          13               // Access denied
#define EFAULT          14               // Bad address
#define ENOTBLK         15               // Not block device
#define EBUSY           16               // Device busy
#define EEXIST          17               // File exist
#define EXDEV           18               // Cross-device link
#define ENODEV          19               // Operation not supported by device
#define ENOTDIR         20               // Not a directory
#define EISDIR          21               // Is a directory
#define EINVAL          22               // Invalid argument
#define ENFILE          23               // Too many open files in system
#define EMFILE          24               // Too many files open
#define ENOTTY          25               // Inappropriate ioctl for device
#define ETXTBSY         26               // Unknown error
#define EFBIG           27               // File too large
#define ENOSPC          28               // No space left on device
#define ESPIPE          29               // Illegal seek
#define EROFS           30               // Read-only file system
#define EMLINK          31               // Too many links
#define EPIPE           32               // Broken pipe
#define EDOM            33               // Numerical arg out of domain
#define ERANGE          34               // Result too large
#define EUCLEAN         35               // Structure needs cleaning
#define EDEADLK         36               // Resource deadlock avoided
#define EUNKNOWN        37               // Unknown error
#define ENAMETOOLONG    38               // File name too long
#define ENOLCK          39               // No locks available
#define ENOSYS          40               // Function not implemented
#define ENOTEMPTY       41               // Directory not empty
#define EILSEQ          42               // Invalid multibyte sequence

//
// Sockets errors
//

#define EWOULDBLOCK     45               // Operation would block
#define EINPROGRESS     46               // Operation now in progress
#define EALREADY        47               // Operation already in progress
#define ENOTSOCK        48               // Socket operation on nonsocket
#define EDESTADDRREQ    49               // Destination address required
#define EMSGSIZE        50               // Message too long
#define EPROTOTYPE      51               // Protocol wrong type for socket
#define ENOPROTOOPT     52               // Bad protocol option
#define EPROTONOSUPPORT 53               // Protocol not supported
#define ESOCKTNOSUPPORT 54               // Socket type not supported
#define EOPNOTSUPP      55               // Operation not supported
#define EPFNOSUPPORT    56               // Protocol family not supported
#define EAFNOSUPPORT    57               // Address family not supported 
#define EADDRINUSE      58               // Address already in use
#define EADDRNOTAVAIL   59               // Cannot assign requested address
#define ENETDOWN        60               // Network is down
#define ENETUNREACH     61               // Network is unreachable
#define ENETRESET       62               // Network dropped connection on reset
#define ECONNABORTED    63               // Connection aborted
#define ECONNRESET      64               // Connection reset by peer
#define ENOBUFS         65               // No buffer space available
#define EISCONN         66               // Socket is already connected
#define ENOTCONN        67               // Socket is not connected
#define ESHUTDOWN       68               // Cannot send after socket shutdown
#define ETOOMANYREFS    69               // Too many references
#define ETIMEDOUT       70               // Operation timed out
#define ECONNREFUSED    71               // Connection refused
#define ELOOP           72               // Cannot translate name
#define EWSNAMETOOLONG  73               // Name component or name was too long
#define EHOSTDOWN       74               // Host is down
#define EHOSTUNREACH    75               // No route to host
#define EWSNOTEMPTY     76               // Cannot remove a directory that is not empty
#define EPROCLIM        77               // Too many processes
#define EUSERS          78               // Ran out of quota
#define EDQUOT          79               // Ran out of disk quota
#define ESTALE          80               // File handle reference is no longer available
#define EREMOTE         81               // Item is not available locally

//
// Resolver errors
//

#define EHOSTNOTFOUND   82               // Host not found
#define ETRYAGAIN       83               // Nonauthoritative host not found
#define ENORECOVERY     84               // A nonrecoverable error occured
#define ENODATA         85               // Valid name, no data record of requested type

//
// Misc. error codes
//

#define EPROTO          86               // Protocol error
#define ECHKSUM         87               // Checksum error
#define EBADSLT         88               // Invalid slot
#define EREMOTEIO       89               // Remote I/O error

//
// Error code aliases
//

#define ETIMEOUT        ETIMEDOUT
#define EBUF            ENOBUFS
#define EROUTE          ENETUNREACH
#define ECONN           ENOTCONN
#define ERST            ECONNRESET
#define EABORT          ECONNABORTED

#endif

#ifdef  __cplusplus
extern "C" {
#endif

#ifndef errno
osapi int *_errno();
#define errno (*_errno())
#endif

#ifdef  __cplusplus
}
#endif

#endif
