//
// mbr.h
//
// Master Boot Record
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

#ifndef MBR_H
#define MBR_H

#define MBR_SIGNATURE            0xAA55

#define SANOS_BOOT_PARTITION_ID  0xCC

#pragma pack(push, 1)

struct disk_partition {
  unsigned char bootid;   // Bootable?  0=no, 128=yes
  unsigned char beghead;  // Beginning head number
  unsigned char begsect;  // Beginning sector number
  unsigned char begcyl;   // 10 bit nmbr, with high 2 bits put in begsect
  unsigned char systid;   // Operating System type indicator code
  unsigned char endhead;  // Ending head number
  unsigned char endsect;  // Ending sector number
  unsigned char endcyl;   // Also a 10 bit nmbr, with same high 2 bit trick
  unsigned int relsect;   // First sector relative to start of disk
  unsigned int numsect;   // Number of sectors in partition
};

struct master_boot_record  {
  char bootstrap[446];
  struct disk_partition parttab[4];
  unsigned short signature;
};

struct boot_sector {
  char prolog[4];
  char label[8];
  unsigned short ldrsize;
  unsigned long ldrstart;
  char bootstrap[492];
  unsigned short signature;
};

#pragma pack(pop)

#endif
