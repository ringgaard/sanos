//
// seg.h
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
// 
// Segment descriptors
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

#define D_ACCESSED 0x1     // Accessed (data or code)

#define D_WRITE    0x2  // Writable (data segments only)
#define D_EXDOWN   0x4  // Expand down (data segments only)

#define D_READ     0x2  // Readable (code segments only)
#define D_CONFORM  0x4  // Conforming (code segments only)

#define D_BUSY     0x2  // Busy (TSS only)

// Granularity flags

#define D_BIG      0x4	  // Default to 32 bit mode
#define D_BIG_LIM  0x8	  // Limit is in 4K units

// Structures for descriptors and gates

#pragma pack(push)
#pragma pack(1)

struct segment
{
  unsigned short limit_low;      // Limit 0..15
  unsigned short base_low;       // Base  0..15
  unsigned char base_med;        // Base  16..23
  unsigned char access;          // Access byte
  unsigned char limit_high:4;    // Limit 16..19
  unsigned char granularity:4;   // Granularity
  unsigned char base_high;       // Base 24..31
};

struct gate
{
  unsigned short offset_low;   // Offset 0..15
  unsigned short selector;     // Selector
  unsigned char notused;
  unsigned char access;        // Access flags
  unsigned short offset_high;  // Offset 16..31
};

union dte
{
  struct segment segment;
  struct gate gate;
};

struct selector
{ 
  unsigned short limit;
  void *dt;
};

void __inline seginit(struct segment *seg, unsigned long addr, unsigned long size, int access, int granularity)
{
  seg->base_low = (unsigned short)(addr & 0xFFFF);
  seg->base_med = (unsigned char)((addr >> 16) & 0xFF);
  seg->base_high = (unsigned char)((addr >> 24) & 0xFF);
  seg->limit_low = (unsigned short) ((size - 1) & 0xFFFF);
  seg->limit_high = (unsigned char) (((size - 1) >> 16) & 0xF);
  seg->access = access;
  seg->granularity = granularity;
}

#pragma pack(pop)

#endif
