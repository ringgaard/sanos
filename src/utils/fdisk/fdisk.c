//
// fdisk.c
//
// Disk Partition Editor
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
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inifile.h>

#include <os/mbr.h>
#include <os/dev.h>

#define ALIGN(n, size) ((((n) + (size) - 1) / (size)) * (size))

char *devname;
int hdev;
struct master_boot_record mbr;
struct geometry geom;

unsigned char bootrecord[512] = {
  0x33, 0xC0, 0x8E, 0xD0, 0xBC, 0x00, 0x7C, 0xFB, 0x50, 0x07, 0x50, 0x1F, 0xFC, 0xBE, 0x1B, 0x7C,
  0xBF, 0x1B, 0x06, 0x50, 0x57, 0xB9, 0xE5, 0x01, 0xF3, 0xA4, 0xCB, 0xBD, 0xBE, 0x07, 0xB1, 0x04,
  0x38, 0x6E, 0x00, 0x7C, 0x09, 0x75, 0x13, 0x83, 0xC5, 0x10, 0xE2, 0xF4, 0xCD, 0x18, 0x8B, 0xF5,
  0x83, 0xC6, 0x10, 0x49, 0x74, 0x19, 0x38, 0x2C, 0x74, 0xF6, 0xA0, 0xB5, 0x07, 0xB4, 0x07, 0x8B,
  0xF0, 0xAC, 0x3C, 0x00, 0x74, 0xFC, 0xBB, 0x07, 0x00, 0xB4, 0x0E, 0xCD, 0x10, 0xEB, 0xF2, 0x88,
  0x4E, 0x10, 0xE8, 0x46, 0x00, 0x73, 0x2A, 0xFE, 0x46, 0x10, 0x80, 0x7E, 0x04, 0x0B, 0x74, 0x0B,
  0x80, 0x7E, 0x04, 0x0C, 0x74, 0x05, 0xA0, 0xB6, 0x07, 0x75, 0xD2, 0x80, 0x46, 0x02, 0x06, 0x83,
  0x46, 0x08, 0x06, 0x83, 0x56, 0x0A, 0x00, 0xE8, 0x21, 0x00, 0x73, 0x05, 0xA0, 0xB6, 0x07, 0xEB,
  0xBC, 0x81, 0x3E, 0xFE, 0x7D, 0x55, 0xAA, 0x74, 0x0B, 0x80, 0x7E, 0x10, 0x00, 0x74, 0xC8, 0xA0,
  0xB7, 0x07, 0xEB, 0xA9, 0x8B, 0xFC, 0x1E, 0x57, 0x8B, 0xF5, 0xCB, 0xBF, 0x05, 0x00, 0x8A, 0x56,
  0x00, 0xB4, 0x08, 0xCD, 0x13, 0x72, 0x23, 0x8A, 0xC1, 0x24, 0x3F, 0x98, 0x8A, 0xDE, 0x8A, 0xFC,
  0x43, 0xF7, 0xE3, 0x8B, 0xD1, 0x86, 0xD6, 0xB1, 0x06, 0xD2, 0xEE, 0x42, 0xF7, 0xE2, 0x39, 0x56,
  0x0A, 0x77, 0x23, 0x72, 0x05, 0x39, 0x46, 0x08, 0x73, 0x1C, 0xB8, 0x01, 0x02, 0xBB, 0x00, 0x7C,
  0x8B, 0x4E, 0x02, 0x8B, 0x56, 0x00, 0xCD, 0x13, 0x73, 0x51, 0x4F, 0x74, 0x4E, 0x32, 0xE4, 0x8A,
  0x56, 0x00, 0xCD, 0x13, 0xEB, 0xE4, 0x8A, 0x56, 0x00, 0x60, 0xBB, 0xAA, 0x55, 0xB4, 0x41, 0xCD,
  0x13, 0x72, 0x36, 0x81, 0xFB, 0x55, 0xAA, 0x75, 0x30, 0xF6, 0xC1, 0x01, 0x74, 0x2B, 0x61, 0x60,
  0x6A, 0x00, 0x6A, 0x00, 0xFF, 0x76, 0x0A, 0xFF, 0x76, 0x08, 0x6A, 0x00, 0x68, 0x00, 0x7C, 0x6A,
  0x01, 0x6A, 0x10, 0xB4, 0x42, 0x8B, 0xF4, 0xCD, 0x13, 0x61, 0x61, 0x73, 0x0E, 0x4F, 0x74, 0x0B,
  0x32, 0xE4, 0x8A, 0x56, 0x00, 0xCD, 0x13, 0xEB, 0xD6, 0x61, 0xF9, 0xC3, 0x49, 0x6E, 0x76, 0x61,
  0x6C, 0x69, 0x64, 0x20, 0x70, 0x61, 0x72, 0x74, 0x69, 0x74, 0x69, 0x6F, 0x6E, 0x20, 0x74, 0x61,
  0x62, 0x6C, 0x65, 0x00, 0x45, 0x72, 0x72, 0x6F, 0x72, 0x20, 0x6C, 0x6F, 0x61, 0x64, 0x69, 0x6E,
  0x67, 0x20, 0x6F, 0x70, 0x65, 0x72, 0x61, 0x74, 0x69, 0x6E, 0x67, 0x20, 0x73, 0x79, 0x73, 0x74,
  0x65, 0x6D, 0x00, 0x4D, 0x69, 0x73, 0x73, 0x69, 0x6E, 0x67, 0x20, 0x6F, 0x70, 0x65, 0x72, 0x61,
  0x74, 0x69, 0x6E, 0x67, 0x20, 0x73, 0x79, 0x73, 0x74, 0x65, 0x6D, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0xAA
};

//
// ask
//

int ask(char *question, char *choices) {
  char ch;
  char *s;

  printf("%s", question);
  while (1) {
    if (read(fdin, &ch, 1) == 1) {
      s = choices;
      while (*s) {
        if (*s == ch)  {
          printf("%c\n", ch);
          return ch;
        }
        s++;
      }
    }
  }
}

//
// lba_to_chs
//

void lba_to_chs(int blkno, int *cyl, int *head, int *sect) {
  *cyl  = blkno / (geom.heads * geom.spt);
  *head = (blkno / geom.spt) % geom.heads;
  *sect = blkno % geom.spt + 1;
}

//
// read_mbr
//

int read_mbr() {
  int rc;

  lseek(hdev, 0, SEEK_SET);
  rc = read(hdev, &mbr, sizeof(mbr));
  if (rc < 0) return rc;

  if (mbr.signature != MBR_SIGNATURE) {
    errno = EINVAL;
    return -1;
  }

  return 0;
}

//
// add_partition
//

void add_partition() {
  int off, noff, poff;
  int size, psize;
  int i, partno, type;
  int cyl, head, sect;
  int ch;
  char str[128];

  // Get partition number
  ch = ask("partition number (0-3)? ", "0123\n");
  if (ch == '\n') return;
  partno = ch - '0';
  if (mbr.parttab[partno].systid != 0) {
    printf("error: partition already used\n");
    return;
  }

  // Get partition size
  printf("partition size (KB) (*=max size)? ");
  gets(str);
  size = *str == '*' ? -1 : atoi(str) * 1024 / geom.sectorsize;

  // Get partition type
  printf("partition type (default %x)? ", SANOS_BOOT_PARTITION_ID);
  gets(str);
  type = *str ? strtol(str, NULL, 16) : SANOS_BOOT_PARTITION_ID;

  // Adjust size to be a multiple of the sectors per track
  if (size > 0) {
    size = ALIGN(size, geom.spt);
    if (size < geom.spt) {
      printf("partition size must be at least %d sectors\n", geom.spt);
      return;
    }
  }

  // Search for the next partition in the table
  for (i = partno + 1; i < 4; i++) {
    if (mbr.parttab[i].systid != 0) break;
  }

  if (i >= 4) {
    noff = -1;
  } else {
    noff = mbr.parttab[i].relsect;
  }

  // Search for the previous partition in the table
  for (i = partno - 1; i >= 0; i--) {
    if (mbr.parttab[i].systid != 0) break;
  }

  if (i < 0) {
    poff = -1;
  } else {
    poff = mbr.parttab[i].relsect;
    psize = mbr.parttab[i].numsect;
  }

  // Compute offset of new partition
  if (poff < 0) {
    off = geom.spt;
  } else {
    off = ALIGN(poff + psize, geom.spt);
  }

  // Check whether specified partition will fit, or calculate maximum partition size
  if (size < 0) {
    size = (noff < 0 ? geom.sectors : noff) - off;
  } else {
    if (off + size > (noff < 0 ? geom.sectors : noff)) {
      printf("partition size too large\n");
      return;
    }
  }
  printf("add partition %d offset %d size %d KB\n", partno, off, size / (1024 / geom.sectorsize));

  mbr.parttab[partno].relsect = off;
  mbr.parttab[partno].numsect = size;
  mbr.parttab[partno].systid = type;

  lba_to_chs(off, &cyl, &head, &sect);
  mbr.parttab[partno].begcyl = cyl & 0xFF;
  mbr.parttab[partno].beghead = head;
  mbr.parttab[partno].begsect = sect | ((cyl >> 8) << 6);

  lba_to_chs(off + size - 1, &cyl, &head, &sect);
  mbr.parttab[partno].endcyl = cyl & 0xFF;
  mbr.parttab[partno].endhead = head;
  mbr.parttab[partno].endsect = sect | ((cyl >> 8) << 6);
}

//
// delete_partition
//

void delete_partition() {
  int ch;
  int partno;

  // Get partition number
  ch = ask("delete partition number (0-3)? ", "0123\n");
  if (ch == '\n') return;
  partno = ch - '0';
  if (mbr.parttab[partno].systid == 0) {
    printf("error: partition not used\n");
    return;
  }

  // Confirm
  ch = ask("\nARE YOU SURE (uppercase 'Y' to confirm)? ", "YNn\n");
  if (ch != 'Y') return;

  // Delete partition
  memset(mbr.parttab + partno, 0, sizeof(struct disk_partition));
  printf("partition %d deleted\n", partno);
}

//
// set_boot_part
//

void set_boot_part() {
  int ch;
  int partno;
  int i;

  // Get partition number
  ch = ask("boot partition number (0-3)? ", "0123\n");
  if (ch == '\n') return;
  partno = ch - '0';

  // Mark partition as active boot partition
  for (i = 0; i < 4; i++) mbr.parttab[i].bootid = 0;
  mbr.parttab[partno].bootid = 0x80;
  printf("partition %d is now boot partition\n", partno);
}

//
// list_partitions
//

void list_partitions() {
  int i;

  printf("device %s: %d KB CHS=%d/%d/%d\n\n", devname, geom.sectors / (1024 / geom.sectorsize), geom.cyls, geom.heads, geom.spt);

  printf("     ------start------ -------end-------\n");
  printf("part   cyl head sector   cyl head sector   offset         size type\n");

  for (i = 0; i < 4; i++) {
    struct disk_partition *p = &mbr.parttab[i];
    
    printf("%c", p->bootid == 0x80 ? '*' : ' ');
    printf("%3d  %4u  %3u     %2u  %4u  %3u     %2u %8u %12u  %02x\n",
           i, 
           p->begcyl + ((p->begsect >> 6) << 8), p->beghead, p->begsect & 0x3F, 
           p->endcyl + ((p->endsect >> 6) << 8), p->endhead, p->endsect & 0x3F, 
           p->relsect, p->numsect / (1024 / geom.sectorsize), p->systid);
  }
}

//
// commit_mbr
//

void commit_mbr() {
  int rc;

  if (ask("save partition table (y/n)? ", "yn") == 'y') {
    lseek(hdev, 0, SEEK_SET);
    rc = write(hdev, &mbr, sizeof(mbr));
    if (rc < 0) {
      printf("%s: error %d writing master boot record\n", devname, errno);
      return;
    }

    rc = ioctl(hdev, IOCTL_REVALIDATE, NULL, 0);
    if (rc < 0) {
      printf("%s: error %d revalidating partitions\n", devname, errno);
      return;
    }

    printf("partition tables saved\n");
  }
}

//
// clear_mbr
//

void clear_mbr() {
  if (ask("create new master boot record (y/n)? ", "yn") == 'y') {
    memcpy(&mbr, bootrecord, sizeof(mbr));
    printf("new master boot record created\n");
  }
}

//
// help
//

void help() {
  printf("  (a)dd     add new partition\n");
  printf("  (b)oot    set boot partition\n");
  printf("  (c)ommit  save partition table in master boot record\n");
  printf("  (d)elete  delete partition\n");
  printf("  (l)ist    list partitions\n");
  printf("  (m)br     reinitialize master boot record and clear partition table\n");
  printf("  (h)elp    this help\n");
  printf("  e(x)it    exit fdisk (discarding uncommitted changes)\n");
}

//
// main
//

int main(int argc, char *argv[]) {
  int rc;
  int cmd;
  int done = 0;

  // Check arguments
  if (argc == 1) {
    devname = "/dev/hd0";
  } else if (argc == 2) {
    devname = argv[1];
  } else {
    printf("usage: fdisk <device>\n");
    return 1;
  }

  // Open device
  hdev = open(devname, O_RDWR | O_BINARY);
  if (hdev < 0) {
    printf("%s: error %d opening device\n", devname, errno);
    return 1;
  }

  // Get disk geometry
  rc = ioctl(hdev, IOCTL_GETGEOMETRY, &geom , sizeof(struct geometry));
  if (rc < 0) {
    printf("%s: error %d determining disk geometry\n", devname, errno);
    close(hdev);
    return 1;
  }

  // Read master boot record
  rc = read_mbr();
  if (rc < 0 && errno != EINVAL) {
    printf("%s: error %d reading master boot record\n", devname, errno);
    close(hdev);
    return 1;
  }

  // Ask to create new master boot record if the existing is invalid
  if (rc < 0 && errno == EINVAL) {
    printf("%s: invalid master boot record\n", devname);
    if (ask("create new master boot record (y/n)? ", "yn") == 'y') {
      memcpy(&mbr, bootrecord, sizeof(mbr));
    }
  }

  // Read commands
  printf("(a)dd (b)oot (c)ommit (d)elete (l)ist (m)br (h)elp e(x)it\n");
  while (!done) {
    cmd = ask("fdisk> ", "abcdlmhx?");

    switch (cmd) {
      case 'a':
        add_partition();
        break;

      case 'b':
        set_boot_part();
        break;

      case 'c':
        commit_mbr();
        break;

      case 'd':
        delete_partition();
        break;

      case 'l':
        list_partitions();
        break;

      case 'm':
        clear_mbr();
        break;

      case 'h':
      case '?':
        help();
        break;

      case 'x':
        done = 1;
        break;
    }
  }

  // Close device
  close(hdev);
  return 0;
}
