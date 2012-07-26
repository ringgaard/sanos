//
// loadkrnl.c
//
// Kernel loader
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
#include <os/pdir.h>
#include <os/tss.h>
#include <os/seg.h>
#include <os/syspage.h>
#include <os/mbr.h>
#include <os/dfs.h>
#include <os/pe.h>
#include <os/dev.h>

void kprintf(const char *fmt,...);
void panic(char *msg);

extern unsigned long krnlentry;
extern int bootpart;

int boot_read(void *buffer, size_t count, blkno_t blkno);
char *alloc_heap(int numpages);

char bsect[SECTORSIZE];
char ssect[SECTORSIZE];
char gsect[SECTORSIZE];
char isect[SECTORSIZE];
blkno_t blockdir[1024];

void load_kernel(int bootdrv) {
  struct master_boot_record *mbr;
  struct superblock *sb;
  struct groupdesc *group;
  struct inodedesc *inode;
  int blocksize;
  int blks_per_sect;
  int kernelsize;
  int kernelpages;
  char *kerneladdr;
  struct dos_header *doshdr;
  struct image_header *imghdr;
  char *addr;
  blkno_t blkno;
  int i;
  int j;
  pte_t *pt;
  int imgpages;
  int start;
  char *label;
  struct boot_sector *bootsect;

  //kprintf("Loading kernel");

  // Determine active boot partition if booting from harddisk
  if (bootdrv & 0x80 && (bootdrv & 0xF0) != 0xF0) {
    mbr = (struct master_boot_record *) bsect;
    if (boot_read(mbr, SECTORSIZE, 0) != SECTORSIZE) {
      panic("unable to read master boot record");
    }

    if (mbr->signature != MBR_SIGNATURE) panic("invalid boot signature");

    bootsect = (struct boot_sector *) bsect;
    label = bootsect->label;
    if (label[0] == 'S' && label[1] == 'A' && label[2] == 'N' && label[3] == 'O' && label[4] == 'S') {
      // Disk does not have a partition table
      start = 0;
      bootpart = -1;
    } else {
      // Find active partition
      bootpart = -1;
      for (i = 0; i < 4; i++) {
        if (mbr->parttab[i].bootid == 0x80) {
          bootpart = i;
          start = mbr->parttab[i].relsect;
        }
      }

      if (bootpart == -1) panic("no bootable partition on boot drive");
    }
  } else {
    start = 0;
    bootpart = 0;
  }

  // Read super block from boot device
  sb = (struct superblock *) ssect;
  if (boot_read(sb, SECTORSIZE, 1 + start) != SECTORSIZE) {
    panic("unable to read super block from boot device");
  }

  // Check signature and version
  if (sb->signature != DFS_SIGNATURE) panic("invalid DFS signature");
  if (sb->version != DFS_VERSION) panic("invalid DFS version");
  blocksize = 1 << sb->log_block_size;
  blks_per_sect =  blocksize / SECTORSIZE;

  // Read first group descriptor
  group = (struct groupdesc *) gsect;
  if (boot_read(group, SECTORSIZE, sb->groupdesc_table_block * blks_per_sect + start) != SECTORSIZE) {
    panic("unable to read group descriptor from boot device");
  }

  // Read inode for kernel
  inode = (struct inodedesc *) isect;
  if (boot_read(isect, SECTORSIZE, group->inode_table_block * blks_per_sect + start) != SECTORSIZE) {
    panic("unable to read kernel inode from boot device");
  }
  inode += DFS_INODE_KRNL;

  // Calculate kernel size
  kernelsize = (int) inode->size;
  kernelpages = PAGES(kernelsize);
  //kprintf("Kernel size %d KB\n", kernelsize / 1024);

  // Allocate page table for kernel
  if (kernelpages > PTES_PER_PAGE) panic("kernel too big");
  pt = (pte_t *) alloc_heap(1);
  pdir[PDEIDX(OSBASE)] = (unsigned long) pt | PT_PRESENT | PT_WRITABLE;

  // Allocate pages for kernel
  kerneladdr = alloc_heap(kernelpages);

  // Read kernel from boot device
  if (inode->depth == 0) {
    addr = kerneladdr;
    for (i = 0; i < (int) inode->blocks; i++) {
      if (boot_read(addr, blocksize, inode->blockdir[i] * blks_per_sect + start) != blocksize) {
        panic("error reading kernel from boot device");
      }
      addr += blocksize;
    }
  } else if (inode->depth == 1) {
    addr = kerneladdr;
    blkno = 0;
    for (i = 0; i < DFS_TOPBLOCKDIR_SIZE; i++) {
      if (boot_read(blockdir, blocksize, inode->blockdir[i] * blks_per_sect + start) != blocksize) {
        panic("error reading kernel inode dir from boot device");
      }

      for (j = 0; j < (int) (blocksize / sizeof(blkno_t)); j++) {
        if (boot_read(addr, blocksize, blockdir[j] * blks_per_sect + start) != blocksize) {
          panic("error reading kernel inode dir from boot device");
        }
        
        addr += blocksize;

        blkno++;
        if (blkno == inode->blocks) break;
      }

      if (blkno == inode->blocks) break;
    }
  } else {
    panic("unsupported inode depth");
  }

  // Determine entry point for kernel
  doshdr = (struct dos_header *) kerneladdr;
  imghdr = (struct image_header *) (kerneladdr + doshdr->e_lfanew);
  krnlentry = imghdr->optional.address_of_entry_point + OSBASE;

  // Allocate pages for .data section
  imgpages = PAGES(imghdr->optional.size_of_image);
  alloc_heap(imgpages - kernelpages);

  // Relocate resource data and clear uninitialized data
  if (imghdr->header.number_of_sections == 4) {
    struct image_section_header *data = &imghdr->sections[2];
    struct image_section_header *rsrc = &imghdr->sections[3];
    memcpy(kerneladdr + rsrc->virtual_address, kerneladdr + rsrc->pointer_to_raw_data, rsrc->size_of_raw_data);
    memset(kerneladdr + data->virtual_address + data->size_of_raw_data, 0, data->virtual_size - data->size_of_raw_data);
  }

  // Map kernel into vitual address space
  for (i = 0; i < imgpages; i++) pt[i] = (unsigned long) (kerneladdr + i * PAGESIZE) | PT_PRESENT | PT_WRITABLE;
}
