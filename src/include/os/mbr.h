//
// mbr.h
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Master Boot Record
//

#ifndef MBR_H
#define MBR_H

#define MBR_SIGNATURE            0xAA55

#define SANOS_BOOT_PARTITION_ID  6

#pragma pack(push)
#pragma pack(1)

struct disk_partition 
{
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

struct master_boot_record 
{
  char bootstrap[446];
  struct disk_partition parttab[4];
  unsigned short signature;
};

struct boot_sector
{
  char prolog[4];
  char label[8];
  unsigned short ldrsize;
  unsigned long ldrstart;
  char bootstrap[492];
  unsigned short signature;
};

#pragma pack(pop)

#endif
