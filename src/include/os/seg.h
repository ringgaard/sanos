//
// seg.h
//
// Segment descriptors
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

#ifndef SEG_H
#define SEG_H

// Each descriptor should have exactly one of next 8 codes to define the type of descriptor

#define D_LDT   0x02  // LDT segment
#define D_TASK  0x05  // Task gate
#define D_TSS   0x09  // TSS
#define D_CALL  0x0C  // 386 call gate
#define D_INT   0x0E  // 386 interrupt gate
#define D_TRAP  0x0F  // 386 trap gate

#define D_DATA  0x10  // Data segment
#define D_CODE  0x18  // Code segment

// Descriptors may include the following as appropriate:

#define D_DPL3         0x60   // DPL3 or mask for DPL
#define D_DPL2         0x40   // DPL2 or mask for DPL
#define D_DPL1         0x20   // DPL1 or mask for DPL
#define D_DPL0         0x00   // DPL0 or mask for DPL

#define D_PRESENT      0x80   // Present

// Segment descriptors (not gates) may include:

#define D_ACCESSED 0x1  // Accessed (data or code)

#define D_WRITE    0x2  // Writable (data segments only)
#define D_EXDOWN   0x4  // Expand down (data segments only)

#define D_READ     0x2  // Readable (code segments only)
#define D_CONFORM  0x4  // Conforming (code segments only)

#define D_BUSY     0x2  // Busy (TSS only)

// Granularity flags

#define D_BIG      0x4    // Default to 32 bit mode
#define D_BIG_LIM  0x8    // Limit is in 4K units

// Structures for descriptors and gates

#pragma pack(push, 1)

struct segment {
  unsigned short limit_low;      // Limit 0..15
  unsigned short base_low;       // Base  0..15
  unsigned char base_med;        // Base  16..23
  unsigned char access;          // Access byte
  unsigned char limit_high;      // Limit 16..19 + Granularity << 4
  unsigned char base_high;       // Base 24..31
};

struct gate {
  unsigned short offset_low;   // Offset 0..15
  unsigned short selector;     // Selector
  unsigned char notused;
  unsigned char access;        // Access flags
  unsigned short offset_high;  // Offset 16..31
};

struct desc {
  unsigned long low;
  unsigned long high;
};

union dte {
  struct segment segment;
  struct gate gate;
  struct desc desc;
};

struct selector {
  unsigned short limit;
  void *dt;
};

struct fullptr {
  unsigned long offset;
  unsigned short segment;
};

void __inline seginit(struct segment *seg, unsigned long addr, unsigned long size, int access, int granularity) {
  seg->base_low = (unsigned short) (addr & 0xFFFF);
  seg->base_med = (unsigned char) ((addr >> 16) & 0xFF);
  seg->base_high = (unsigned char) ((addr >> 24) & 0xFF);
  seg->limit_low = (unsigned short) ((size - 1) & 0xFFFF);
  seg->limit_high = (unsigned char) ((((size - 1) >> 16) & 0xF) | (granularity << 4));
  seg->access = access;
}

#pragma pack(pop)

#endif
