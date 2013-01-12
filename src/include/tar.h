//
// tar.h
//
// Extended tar definitions
//
// Copyright (C) 2012 Michael Ringgaard. All rights reserved.
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

#ifndef TAR_H
#define TAR_H

//
// General definitions
//

#define TMAGIC     "ustar"  // ustar plus null byte
#define TMAGLEN    6        // Length of the above
#define TVERSION   "00"     // 00 without a null byte
#define TVERSLEN   2        // Length of the above

//
// Type flags
//

#define REGTYPE    '0'      // Regular file
#define AREGTYPE   '\0'     // Regular file
#define LNKTYPE    '1'      // Link
#define SYMTYPE    '2'      // Symbolic link
#define CHRTYPE    '3'      // Character special
#define BLKTYPE    '4'      // Block special
#define DIRTYPE    '5'      // Directory
#define FIFOTYPE   '6'      // FIFO special
#define CONTTYPE   '7'      // Reserved

//
// Mode field bits
//

#define TSUID      04000    // Set UID on execution
#define TSGID      02000    // Set GID on execution
#define TSVTX      01000    // On directories, restricted deletion flag
#define TUREAD     00400    // Read by owner
#define TUWRITE    00200    // Write by owner special
#define TUEXEC     00100    // Execute/search by owner
#define TGREAD     00040    // Read by group
#define TGWRITE    00020    // Write by group
#define TGEXEC     00010    // Execute/search by group
#define TOREAD     00004    // Read by other
#define TOWRITE    00002    // Write by other
#define TOEXEC     00001    // Execute/search by other

#define TAR_BLKSIZ     512  // Standard tar block size
#define TAR_NAMELEN    100  // Name length
#define TAR_PREFIXLEN  155  // Prefix length
#define TAR_MAXPATHLEN (TAR_NAMELEN + TAR_PREFIXLEN)

//
// Tar header
//

struct tarhdr {
  char name[100];
  char mode[8];
  char uid[8];
  char gid[8];
  char size[12];
  char mtime[12];
  char chksum[8];
  char typeflag;
  char linkname[100];
  char magic[6];
  char version[2];
  char uname[32];
  char gname[32];
  char devmajor[8];
  char devminor[8];
  char prefix[155];
  char padding[12];
};

#endif
