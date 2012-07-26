//
// setup.c
//
// Setup program
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

#include <os/dev.h>
#include <os/mbr.h>
#include <os/dfs.h>
#include <os/seg.h>
#include <os/tss.h>
#include <os/syspage.h>

struct section *inst;
char *devname;
struct superblock *super;
struct master_boot_record *mbr;
struct boot_sector *bootsect;

int ldr_start;
int ldr_size;

char ssect[SECTORSIZE];
char bsect[SECTORSIZE];
char msect[SECTORSIZE];
char block[64 * 1024];
char str[128];

//
// doformat
//

int doformat(struct section *sect) {
  char *devname;
  char *fstype;
  int blocksize;
  int quick;
  int cache;
  char options[128];
  int rc;

  // Get formatting properties
  devname = get_property(inst, sect->name, "device", "<none>");
  fstype = get_property(inst, sect->name, "fstype", "dfs");
  blocksize = get_numeric_property(inst, sect->name, "blocksize", 4096);
  quick = get_numeric_property(inst, sect->name, "quick", 0);
  cache = get_numeric_property(inst, sect->name, "cache", 0);

  sprintf(options, "blocksize=%d,cache=%d%s", blocksize, cache, quick ? ",quick" : "");
  printf("Formatting device %s (%s)...\n", devname, options);

  // Format device
  rc = mkfs(devname, fstype, options);
  if (rc < 0) return -1;
  printf("format complete\n");
  return 0;
}

//
// install_loader
//

int install_loader(char *devname, char *loader, char *krnlopts) {
  int n;
  int dev;
  int ldr;
  int rc;
  int blocksize;
  int size;
  char *image;

  sprintf(str, "/dev/%s", devname);
  printf("Installing loader %s on %s\n", loader, str);

  // Open device
  dev = open(str, O_RDWR | O_BINARY);
  if (dev < 0) return -1;

  // Read loader image
  ldr = open(loader, O_BINARY);
  if (ldr < 0) return -1;

  size = fstat(ldr, NULL);
  image = (char *) malloc(size);
  if (!image) return -1;

  rc = read(ldr, image, size);
  if (rc < 0) return -1;

  close(ldr);

  // Read super block from device
  rc = lseek(dev, 1 * SECTORSIZE, SEEK_SET);
  if (rc < 0) return -1;

  rc = read(dev, ssect, SECTORSIZE);
  if (rc < 0) return -1;

  super = (struct superblock *) ssect;
  blocksize = 1 << super->log_block_size;

  // Calculate loader start and size in sectors (used by bootstrap)
  ldr_start = super->first_reserved_block * (blocksize / SECTORSIZE);
  ldr_size = size / SECTORSIZE;
  if (size > (int) super->reserved_blocks * blocksize) {
    printf("Loader too big\n");
    errno = EIO;
    return -1;
  }

  // Patch kernel options into image
  if (krnlopts) {
    int optspos;

    if (strlen(krnlopts) > KRNLOPTS_LEN - 1) {
      printf("Kernel options too long\n");
      errno = EBUF;
      return -1;
    }
    
    optspos = *(unsigned short *) (image + KRNLOPTS_POSOFS);
    strcpy(image + optspos, krnlopts);
  }

  // Install loader into image
  rc = lseek(dev, super->first_reserved_block * blocksize, SEEK_SET);
  if (rc < 0) return -1;

  for (n = 0; n < size / blocksize; n++) {
    rc = write(dev, image + n * blocksize, blocksize);
    if (rc < 0) return -1;
  }

  close(dev);
  free(image);

  return 0;
}

//
// install_boot_sector
//

int install_boot_sector(char *devname, char *bootstrap) {
  int dev;
  int boot;
  int disk;
  int rc;
  int partno;
  int partofs;
  char diskname[16];

  sprintf(str, "/dev/%s", devname);
  printf("Installing boot sector %s on %s\n", bootstrap, str);

  // Read bootstrap
  boot = open(bootstrap, O_BINARY);
  if (boot < 0) return -1;

  rc = read(boot, bsect, SECTORSIZE);
  if (rc < 0) return -1;

  close(boot);

  // Check for partitioned disk
  if (strlen(devname) == 4) {
    partno = devname[3] - 'a';
    if (partno < 0 || partno > 3) {
      printf("Invaid partition\n");
      errno = EINVAL;
      return -1;
    }

    // Read master boot record and get partition offset
    sprintf(diskname, "/dev/%c%c%c", devname[0], devname[1], devname[2]);
    disk = open(diskname, O_BINARY);
    if (disk < 0) return -1;

    rc = read(disk, msect, SECTORSIZE);
    if (rc < 0) return -1;

    close(disk);

    mbr = (struct master_boot_record *) msect;
    if (mbr->signature != MBR_SIGNATURE) {
      printf("Invalid signature in master boot record\n");
      errno = EIO;
      return -1;
    }

    partofs = mbr->parttab[partno].relsect;
    //printf("Installing on partition %d offset %d\n", partno, partofs);
  } else {
    partofs = 0;
  }

  // Set boot sector parameters
  bootsect = (struct boot_sector *) bsect;
  if (bootsect->signature != MBR_SIGNATURE) {
    printf("Invalid signature in bootstrap");
    errno = EINVAL;
    return -1;
  }
  
  bootsect->ldrstart = ldr_start + partofs;
  bootsect->ldrsize = ldr_size;

  // Write boot sector to target device
  dev = open(str, O_RDWR | O_BINARY);
  if (dev < 0) return -1;

  rc = lseek(dev, 0 * SECTORSIZE, SEEK_SET);
  if (rc < 0) return -1;

  rc = write(dev, bsect, SECTORSIZE);
  if (rc < 0) return -1;

  close(dev);

  return 0;
}

//
// dosysprep
//

int dosysprep(struct section *sect) {
  char *devname;
  char *bootstrap;
  char *loader;
  char *krnlopts;
  int rc;

  // Get properties
  devname = get_property(inst, sect->name, "device", "<none>");
  bootstrap = get_property(inst, sect->name, "bootstrap", "/boot/boot");
  loader = get_property(inst, sect->name, "loader", "/boot/osldr.dll");
  krnlopts = get_property(inst, sect->name, "options", "");

  // Install loader
  rc = install_loader(devname, loader, krnlopts);
  if (rc < 0) return -1;

  // Install boot sector
  rc = install_boot_sector(devname, bootstrap);
  if (rc < 0) return -1;

  return 0;
}

//
// dokernel
//

int dokernel(struct section *sect) {
  char *kernel;
  char *target;
  int fin;
  int fout;
  int rc;
  int size;
  int left;
  int bytes;
  char targetfn[MAXPATH];

  // Get properties
  kernel = get_property(inst, sect->name, "kernel", "/boot/krnl.dll");
  target = get_property(inst, sect->name, "target", "/mnt/boot");

  printf("Installing kernel %s\n", kernel);

  // Open source kernel file
  fin = open(kernel, O_BINARY);
  if (fin < 0) return -1;

  size = fstat(fin, NULL);

  // Make sure /boot directory exists on target
  if (stat(target, NULL) < 0) {
    rc = mkdir(target, 0755);
    if (rc < 0) return -1;
  }

  // Install kernel on target using reserved inode
  sprintf(targetfn, "%s/krnl.dll", target);
  fout = open(targetfn, O_SPECIAL | (DFS_INODE_KRNL << 24));
  if (fout < 0) return -1;
  fchmod(fout, 0644);

  left = size;
  while (left > 0) {
    bytes = read(fin, block, sizeof block);
    if (!bytes) {
      errno = EIO;
      return -1;
    }
    if (bytes < 0) return -1;

    rc = write(fout, block, bytes);
    if (rc < 0) return -1;

    left -= bytes;
  }

  close(fin);
  close(fout);

  return 0;
}

//
// domount
//

int domount(struct section *sect) {
  char *mntfrom;
  char *mntto;
  char *fstype;
  char *opts;
  int rc;

  // Get properties
  mntfrom = get_property(inst, sect->name, "mntfrom", "hd0a");
  mntto = get_property(inst, sect->name, "mntto", "/mnt");
  fstype = get_property(inst, sect->name, "fstype", "dfs");
  opts = get_property(inst, sect->name, "opts", NULL);

  // Mount file system
  printf("Mounting filesystem %s on %s\n", mntfrom, mntto);
  rc = mount(fstype, mntto, mntfrom, opts);
  if (rc < 0) return -1;

  return 0;
}

//
// dounmount
//

int dounmount(struct section *sect) {
  char *path;
  int rc;

  // Get properties
  path = get_property(inst, sect->name, "path", "/mnt");

  // Unmount file system
  printf("Unmounting filesystem %s\n", path);
  rc = umount(path);
  if (rc < 0) return -1;

  return 0;
}

//
// domkdirs
//

int domkdirs(struct section *sect) {
  struct property *prop;
  char *dirname;
  int rc;

  prop = sect->properties;
  while (prop) {
    dirname = prop->name;
    printf("Creating directory %s\n", dirname);

    rc = mkdir(dirname, 0755);
    if (rc < 0) return -1;

    prop = prop->next;
  }

  return 0;
}

//
// copy_file
//

int copy_file(char *srcfn, char *dstfn) {
  int fin;
  int fout;
  int bytes;
  int rc;
  struct stat st;
  struct utimbuf ut;

  fin = open(srcfn, O_BINARY);
  if (fin < 0) return fin;

  rc = fstat(fin, &st);
  if (rc < 0) return -1;

  fout = open(dstfn,  O_CREAT | O_EXCL | O_BINARY, S_IREAD | S_IWRITE);
  if (fout < 0) return -1;

  fchmod(fout, st.st_mode);
  fchown(fout, st.st_uid, st.st_gid);
  
  ut.modtime = st.st_mtime;
  ut.ctime = st.st_ctime;
  ut.actime = st.st_atime;
  futime(fout, &ut);

  while ((bytes = read(fin , block, sizeof block)) > 0) {
    rc = write(fout, block, bytes);
    if (rc < 0) return -1;
  }

  if (bytes < 0) return -1;

  close(fin);
  close(fout);

  return 0;
}

//
// copy_dir
//

int copy_dir(char *srcdir, char *dstdir) {
  struct copyitem {
    char srcdir[MAXPATH];
    char dstdir[MAXPATH];
    struct copyitem *next;
  };

  struct copyitem *head;
  struct copyitem *tail;
  struct copyitem *next;
  int dir;
  struct direntry dirp;
  struct stat64 buf;
  char srcfn[MAXPATH];
  char dstfn[MAXPATH];
  int rc;

  head = tail = (struct copyitem *) malloc(sizeof(struct copyitem));
  if (!head) return -1;
  strcpy(head->srcdir, srcdir);
  strcpy(head->dstdir, dstdir);
  head->next = NULL;
  
  while (head) {
    dir = _opendir(head->srcdir);
    if (dir < 0) return -1;

    while (_readdir(dir, &dirp, 1) > 0) {
      sprintf(srcfn, "%s/%s", head->srcdir, dirp.name);
      sprintf(dstfn, "%s/%s", head->dstdir, dirp.name);

      rc = stat64(srcfn, &buf);
      if (rc < 0) return -1;

      if ((buf.st_mode & S_IFMT) == S_IFDIR) {
        printf("Creating directory %s\n", dstfn);
        rc = mkdir(dstfn, 0755);
        if (rc < 0) return -1;

        tail->next = (struct copyitem *) malloc(sizeof(struct copyitem));
        if (!tail->next) return -1;
        tail = tail->next;
        strcpy(tail->srcdir, srcfn);
        strcpy(tail->dstdir, dstfn);
        tail->next = NULL;
      } else {
        printf("Copying %s to %s\n", srcfn, dstfn);
        rc = copy_file(srcfn, dstfn);
        if (rc < 0) return -1;
      }
    }

    close(dir);

    next = head->next;
    free(head);
    head = next;
  }

  return 0;
}

//
// docopy
//

int docopy(struct section *sect) {
  struct property *prop;
  char *srcfn;
  char *dstfn;
  struct stat64 buf;
  int rc;

  prop = sect->properties;
  while (prop) {
    dstfn = prop->name;
    srcfn = prop->value;

    printf("Installing %s\n", dstfn);
    
    rc = stat64(srcfn, &buf);
    if (rc < 0) return -1;

    if ((buf.st_mode & S_IFMT) == S_IFDIR) {
      printf("Creating directory %s\n", dstfn);
      rc = mkdir(dstfn, 0755);
      if (rc < 0) return -1;

      rc = copy_dir(srcfn, dstfn);
      if (rc < 0) return -1;
    } else {
      rc = copy_file(srcfn, dstfn);
      if (rc < 0) return -1;
    }

    prop = prop->next;
  }

  return 0;
}

//
// runscript
//

int runscript(char *scriptname) {
  struct section *scriptsect;
  struct property *prop;

  scriptsect = find_section(inst, scriptname);
  if (!scriptsect) {
    printf("Unable to find script section %s\n", scriptname);
    return -EINVAL;
  }

  prop = scriptsect->properties;
  while (prop) {
    char *action;
    char *scriptname;
    int rc;
    struct section *scriptblock;
    
    action = prop->name;
    scriptname = prop->value ? prop->value : prop->name;

    scriptblock = find_section(inst, scriptname);
    if (!scriptblock) {
      printf("Unable to find script block %s\n", scriptname);
      errno = EINVAL;
      return -1;
    }

    if (strcmp(action, "format") == 0) {
      rc = doformat(scriptblock);
    } else if (strcmp(action, "sysprep") == 0) {
      rc = dosysprep(scriptblock);
    } else if (strcmp(action, "mount") == 0) {
      rc = domount(scriptblock);
    } else if (strcmp(action, "kernel") == 0) {
      rc = dokernel(scriptblock);
    } else if (strcmp(action, "mkdirs") == 0) {
      rc = domkdirs(scriptblock);
    } else if (strcmp(action, "copy") == 0) {
      rc = docopy(scriptblock);
    } else if (strcmp(action, "unmount") == 0) {
      rc = dounmount(scriptblock);
    } else {
      printf("Unknown action '%s' in script block %s\n", action, prop->name);
      errno = EINVAL;
      return -1;
    }

    if (rc < 0) {
      printf("Error %d (%s) performing %s\n", errno, strerror(errno), prop->name);
      return -1;
    }
    
    prop = prop->next;
  }

  return 0;
}

//
// main
//

int main(int argc, char *argv[]) {
  char *instfn;
  char *prodname;
  char *prodvers;
  char *scriptname;
  int rc;

  if (argc == 2) {
    instfn = argv[1];
  } else {
    instfn = "/etc/setup.ini";
  }

  // Get setup properties
  inst = read_properties(instfn);
  if (!inst) {
    printf("Error reading install file, %s\n", instfn);
    return 2;
  }

  prodname = get_property(inst, "setup", "product", gettib()->peb->osname);
  prodvers = get_property(inst, "setup", "version", "1.0");
  scriptname = get_property(inst, "setup", "actions", "actions");

  printf("Installing %s version %s\n", prodname, prodvers);

  // Perform install script
  rc = runscript(scriptname);
  if (rc < 0) {
    printf("Installation failed\n");
    free_properties(inst);
    return 1;
  }

  // Installation successfull
  printf("%s installed\n", prodname);
  free_properties(inst);

  return 0;
}
