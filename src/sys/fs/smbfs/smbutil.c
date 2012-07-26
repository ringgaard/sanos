//
// smbutil.c
//
// SMB utility functions
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

#include <os/krnl.h>
#include "smb.h"

int recv_fully(struct socket *s, char *buf, int size, int flags) {
  int bytes;
  int left;

  left = size;
  while (left > 0) {
    bytes = recv(s, buf, left, flags);
    if (bytes <= 0) return bytes;

    buf += bytes;
    left -= bytes;
  }

  return size;
}

char *addstr(char *p, char *s) {
  while (*s) *p++ = *s++;
  return p;
}

char *addstrz(char *p, char *s) {
  while (*s) *p++ = *s++;
  *p++ = 0;
  return p;
}

char *addpathz(char *p, char *s) {
  while (*s) {
    *p = *s == '/' ? '\\' : *s;
    p++;
    s++;
  }
  *p++ = 0;
  return p;
}

time_t ft2time(smb_time filetime) {
  return (time_t) ((filetime - EPOC) / SECTIMESCALE);
}

smb_time time2ft(time_t time) {
  return (smb_time) time * SECTIMESCALE + EPOC;
}

int smb_convert_filename(char *name) {
  char *p;

  for (p = name; *p; p++)  if (*p == '/') *p = '\\';
  return 0;
}

int smb_errno(struct smb *smb) {
  int errcls = smb->error_class;
  int error  = smb->error;

  //kprintf("smb: error %d class: %d\n", error, errcls);

  if (errcls == SMB_ERRDOS) {
    switch (error) {
      case ERRbadfunc: return -EINVAL;
      case ERRbadfile: return -ENOENT;
      case ERRbadpath: return -ENOENT;
      case ERRnofids: return -EMFILE;
      case ERRnoaccess: return -EACCES;
      case ERRbadfid: return -EBADF;
      case ERRbadmcb: return -EREMOTEIO;
      case ERRnomem: return -ENOMEM;
      case ERRbadmem: return -EFAULT;
      case ERRbadenv: return -EREMOTEIO;
      case ERRbadformat: return -EREMOTEIO;
      case ERRbadaccess: return -EACCES;
      case ERRbaddata: return -E2BIG;
      case ERRbaddrive: return -ENXIO;
      case ERRremcd: return -EREMOTEIO;
      case ERRdiffdevice: return -EXDEV;
      case ERRnofiles: return 0;
      case ERRbadshare: return -ETXTBSY;
      case ERRlock: return -EDEADLK;
      case ERRfilexists: return -EEXIST;
      case 87: return 0;
      case 123: return -ENOENT;
      case 145: return -ENOTEMPTY;
      case 183: return -EEXIST;
      default: return -EIO;
    }
  } else if (errcls == SMB_ERRSRV) {
    switch (error) {
      case ERRerror: return -ENFILE;
      case ERRbadpw: return -EINVAL;
      case ERRbadtype: return -EIO;
      case ERRaccess: return -EACCES;
      case ERRinvnid: return -EBADSLT;
      default: return -EIO;
    }
  } else if (errcls == SMB_ERRHRD) {
    switch (error) {
      case ERRnowrite: return -EROFS;
      case ERRbadunit: return -ENODEV;
      case ERRnotready: return -EUCLEAN;
      case ERRbadcmd: return -EIO;
      case ERRdata: return -EIO;
      case ERRbadreq: return -ERANGE;
      case ERRbadshare: return -ETXTBSY;
      case ERRlock: return -EDEADLK;
      default: return -EIO;
    }
  } else if (errcls == SMB_ERRCMD) {
    return -EIO;
  }

  return -EIO;
}
