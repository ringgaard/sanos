//
// mkboot.c
//
// Kernel install utility
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

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <os.h>
#include <os/mbr.h>
#include <os/dev.h>
#include <os/dfs.h>
#include <os/tss.h>
#include <os/seg.h>
#include <os/syspage.h>

void usage() {
  fprintf(stderr, "usage: mkboot [ options ]\n\n");
  fprintf(stderr, "  -b <bootsect> Install new boot sector\n");
  fprintf(stderr, "  -d <path>     Filesystem where boot components will be installed\n");
  fprintf(stderr, "  -k <kernel>   Install new kernel\n");
  fprintf(stderr, "  -l <osldr>    Install new boot loader\n");
  fprintf(stderr, "  -o <options>  Kernel options\n");
}

int read_boot_sector(int fd, struct boot_sector *bsect) {
  int rc;

  rc = read(fd, bsect, sizeof(struct boot_sector));
  if (rc < 0) {
    perror("error reading boot sector");
    return -1;
  }
  if (rc != sizeof(struct boot_sector)) {
    fprintf(stderr, "error: Wrong size for boot sector (%d bytes)\n", rc);
    return -1;
  }
  return 0;
}

int install_boot_sector(char *devname, char *bootfile) {
  int dev;
  int fd;
  struct boot_sector bsect;
  unsigned short ldrsize;
  unsigned long ldrstart;
  int rc;

  // Read existing boot sector to get loader parameters
  dev = open(devname, O_RDWR | O_BINARY);
  if (dev < 0) {
    perror(devname);
    return -1;
  }
  if (read_boot_sector(dev, &bsect) < 0) {
    close(dev);
    return -1;
  }
  ldrsize = bsect.ldrsize;
  ldrstart = bsect.ldrstart;

  // Read new bootstrap
  fd = open(bootfile, O_BINARY);
  if (fd < 0) {
    perror(bootfile);
    close(dev);
    return -1;
  }
  if (read_boot_sector(fd, &bsect) < 0) {
    close(dev);
    close(fd);
    return -1;
  }
  close(fd);

  // Check for valid boot sector
  if (bsect.signature != MBR_SIGNATURE) {
    fprintf(stderr, "Invalid boot sector signature\n");
    close(dev);
    return -1; 
  }

  // Patch loader parameters into boot sector
  bsect.ldrsize = ldrsize;
  bsect.ldrstart = ldrstart;

  // Write bootstrap to boot sector
  rc = lseek(dev, 0, SEEK_SET);
  if (rc < 0) {
    perror(devname);
    close(dev);
    return -1;
  }

  rc = write(dev, &bsect, sizeof(struct boot_sector));
  if (rc < 0) {
    printf("Unable to write boot sector\n");
    close(dev);
    return -1;
  }

  close(dev);
  return 0;
}

int install_loader(char *devname, char *ldrfile, char *krnlopts) {
  int dev = -1;
  int ldr = -1;
  char *image = NULL;
  int size;
  int rc;
  int n;
  int blocksize;
  struct boot_sector bsect;
  char ssect[SECTORSIZE];
  struct superblock *super;

  // Open device
  dev = open(devname, O_RDWR | O_BINARY);
  if (dev < 0) {
    perror(devname);
    goto error;
  }

  // Read boot sector from device
  rc = lseek(dev, 0 * SECTORSIZE, SEEK_SET);
  if (rc < 0) {
    perror(devname);
    goto error;
  }
  if (read_boot_sector(dev, &bsect) < 0) goto error;

  // Read loader image
  ldr = open(ldrfile, O_BINARY);
  if (ldr < 0) {
    perror(ldrfile);
    goto error;
  }

  size = filelength(ldr);
  image = (char *) malloc(size);
  if (!image) {
    perror("malloc"); 
    goto error;
  }

  rc = read(ldr, image, size);
  if (rc < 0) {
    perror(ldrfile);
    goto error;
  }

  // Check signature
  if (size < 2 || image[0] != 'M' || image[1] != 'Z') {
    fprintf(stderr, "%s: Invalid boot loader signature\n", ldrfile);
    goto error;
  }

  // Read super block from device
  rc = lseek(dev, 1 * SECTORSIZE, SEEK_SET);
  if (rc < 0) {
    perror(devname);
    goto error;
  }

  rc = read(dev, ssect, SECTORSIZE);
  if (rc < 0) {
    perror(devname);
    goto error;
  }

  super = (struct superblock *) ssect;
  blocksize = 1 << super->log_block_size;

  // Calculate loader start and size in sectors (used by bootstrap)
  bsect.ldrstart = super->first_reserved_block * (blocksize / SECTORSIZE);
  bsect.ldrsize = size / SECTORSIZE;
  if (size > (int) super->reserved_blocks * blocksize) {
    printf("Loader too big (max %d bytes)\n", super->reserved_blocks * blocksize);
    goto error;
  }

  // Patch kernel options into loader
  if (krnlopts) {
    int optspos;
    
    optspos = *(unsigned short *) (image + KRNLOPTS_POSOFS);
    strcpy(image + optspos, krnlopts);
  }

  // Install loader into reserved blocks of file system
  rc = lseek(dev, super->first_reserved_block * blocksize, SEEK_SET);
  if (rc < 0) {
    perror(devname);
    goto error;
  }
  
  for (n = 0; n < size / blocksize; n++) {
    rc = write(dev, image + n * blocksize, blocksize);
    if (rc < 0) {
      perror(devname);
      goto error;
    }
  }

  // Write boot sector with patched loader parameters
  lseek(dev, 0, SEEK_SET);
  write(dev, &bsect, sizeof(struct boot_sector));

  close(ldr);
  close(dev);
  free(image);
  return 0;

error:
  if (dev != -1) close(dev);
  if (ldr != -1) close(ldr);
  if (image) free(image);
  return -1;
}

int install_kernel(char *target, char *krnlfile) {
  int fin;
  int fout;
  int bytes;
  char block[512];
  char targetfn[MAXPATH];

  // Open new kernel file
  fin = open(krnlfile, O_BINARY);
  if (fin < 0) {
    perror(krnlfile);
    return -1;
  }

  // Remove old kernel
  sprintf(targetfn, "%s/boot/krnl.dll", target);
  unlink(targetfn);

  // Install kernel on target using reserved inode
  fout = open(targetfn, O_SPECIAL | (DFS_INODE_KRNL << 24), 0744);
  if (fout < 0) {
    perror(targetfn);
    close(fin);
    return -1;
  }
  fchmod(fout, 0644);

  // Copy kernel
  while ((bytes = read(fin, block, sizeof block)) > 0) {
    write(fout, block, bytes);
  }

  close(fin);
  close(fout);
  return 0;
}

int main(int argc, char *argv[]) {
  char *bootfs = "/";
  char devname[MAXPATH];
  char *bootfile = NULL;
  char *ldrfile = NULL;
  char *krnlfile = NULL;
  char *krnlopts = NULL;
  struct statfs fs;
  int c;
  int rc;

  // Parse command line options
  while ((c = getopt(argc, argv, "b:d:hk:l:o:")) != EOF) {
    switch (c) {
      case 'b':
        bootfile = optarg;
        break;

      case 'd':
        bootfs = optarg;
        break;

      case 'k':
        krnlfile = optarg;
        break;

      case 'l':
        ldrfile = optarg;
        break;

      case 'o':
        krnlopts = optarg;
        break;

      case 'h':
      default:
        usage();
        return 1;
    }
  }

  // Get boot file system information
  if (statfs(bootfs, &fs) < 0) {
    perror(bootfs);
    return 1;
  }
  strcpy(devname, "/dev/");
  strcat(devname, fs.mntfrom);

  // Check boot file system
  if (strcmp(fs.fstype, "dfs") != 0 || strcmp(fs.mntto, bootfs) != 0) {
    printf("%s: cannot install on %s (%s)\n", devname, fs.fstype, fs.mntto);
    return 1;
  }

  // Install new boot sector
  if (bootfile != NULL) {
    printf("Installing boot sector %s on %s\n", bootfile, devname);
    rc = install_boot_sector(devname, bootfile);
    if (rc < 0)  return 1;
  }

  // Install new loader
  if (krnlopts != NULL) {
    if (strlen(krnlopts) > KRNLOPTS_LEN - 1) {
      printf("Kernel options too big\n");
      return 1;
    }
    if (ldrfile == NULL) ldrfile = "/boot/osldr.dll";
  }
  if (ldrfile != NULL) {
    printf("Installing loader %s on %s", ldrfile, devname);
    if (krnlopts) printf(" with options %s", krnlopts);
    printf("\n");
    rc = install_loader(devname, ldrfile, krnlopts);
    if (rc < 0)  return 1;
  }

  // Install new kernel
  if (krnlfile != NULL) {
    printf("Installing kernel %s on %s\n", krnlfile, devname);
    rc = install_kernel(fs.mntto, krnlfile);
    if (rc < 0)  return 1;
  }

  return 0;
}
