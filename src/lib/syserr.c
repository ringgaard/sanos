//
// syserr.c
//
// Standard library functions
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

#include <os.h>

char *sys_errlist[] =
{
  /*  0                 */  "No error",
  /*  1 EPERM           */  "Operation not permitted",
  /*  2 ENOENT          */  "No such file or directory",
  /*  3 ESRCH           */  "No such process",
  /*  4 EINTR           */  "Interrupted function call",
  /*  5 EIO             */  "Input/output error",
  /*  6 ENXIO           */  "No such device or address",
  /*  7 E2BIG           */  "Arg list too long",
  /*  8 ENOEXEC         */  "Exec format error",
  /*  9 EBADF           */  "Bad file descriptor",
  /* 10 ECHILD          */  "No child processes",
  /* 11 EAGAIN          */  "Resource temporarily unavailable",
  /* 12 ENOMEM          */  "Not enough space",
  /* 13 EACCES          */  "Permission denied",
  /* 14 EFAULT          */  "Bad address",
  /* 15 ENOTBLK         */  "Unknown error",
  /* 16 EBUSY           */  "Resource device",
  /* 17 EEXIST          */  "File exists",
  /* 18 EXDEV           */  "Improper link",
  /* 19 ENODEV          */  "No such device",
  /* 20 ENOTDIR         */  "Not a directory",
  /* 21 EISDIR          */  "Is a directory",
  /* 22 EINVAL          */  "Invalid argument",
  /* 23 ENFILE          */  "Too many open files in system",
  /* 24 EMFILE          */  "Too many open files",
  /* 25 ENOTTY          */  "Inappropriate I/O control operation",
  /* 26 ETXTBSY         */  "Unknown error",
  /* 27 EFBIG           */  "File too large",
  /* 28 ENOSPC          */  "No space left on device",
  /* 29 ESPIPE          */  "Invalid seek",
  /* 30 EROFS           */  "Read-only file system",
  /* 31 EMLINK          */  "Too many links",
  /* 32 EPIPE           */  "Broken pipe",
  /* 33 EDOM            */  "Domain error",
  /* 34 ERANGE          */  "Result too large",
  /* 35 EUCLEAN         */  "Structure needs cleaning",
  /* 36 EDEADLK         */  "Resource deadlock avoided",
  /* 37 UNKNOWN         */  "Unknown error",
  /* 38 ENAMETOOLONG    */  "Filename too long",
  /* 39 ENOLCK          */  "No locks available",
  /* 40 ENOSYS          */  "Function not implemented",
  /* 41 ENOTEMPTY       */  "Directory not empty",
  /* 42 EILSEQ          */  "Illegal byte sequence",

  /* 43 ETIMEOUT        */  "Timeout",
  /* 44 EBUF            */  "Buffer error",
  /* 45 EROUTE          */  "No route",
  /* 46 ECONN           */  "Not connected",
  /* 47 ERST            */  "Connection reset",
  /* 48 EABORT          */  "Connction aborted",
  /* 49 EUSED           */  "Address in use",
  /* 50 EPROTONOSUPPORT */  "Protocol not supported",
  /* 51 EMSGSIZE        */  "Message too long",
  /* 52 ECONNREFUSED    */  "Connection refused",
  /* 53 EHOSTUNREACH    */  "Host unreachable",
  /* 54 ENETUNREACH     */  "Network unrechable",
  /* 55 EHOST           */  "Host not found",
  /* 56 EPROTO          */  "Protocol error",
  /* 57 ECHKSUM         */  "Checksum error",
  /* 58 EDESTUNREACH    */  "Destination unreachable",
  /* 59 EBADSLT         */  "Invalid slot",
  /* 60 EREMOTEIO       */  "Remote I/O error",
};

int sys_nerr = sizeof(sys_errlist) / sizeof(sys_errlist[0]);

char *strerror(int errnum)
{
  if (errnum < 0) errnum = -errnum;

  if (errnum >= sys_nerr)
    return "Unknown error";
  else
    return sys_errlist[errnum];
}
