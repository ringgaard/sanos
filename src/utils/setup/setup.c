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

#include <os/mbr.h>
#include <os/dfs.h>
#include <os/version.h>

#define SECTORSIZE 512

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
char block[4096];
char str[128];

//
// doformat
//

int doformat(struct section *sect)
{
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
  rc = format(devname, fstype, options);
  if (rc < 0) return rc;
  printf("format complete\n");
  return 0;
}

//
// install_loader
//

int install_loader(char *devname, char *loader)
{
  int n;
  int dev;
  int ldr;
  int rc;
  int blocksize;
  int size;

  sprintf(str, "/dev/%s", devname);
  printf("Installing loader %s on %s\n", loader, str);

  // Open device
  dev = open(str, O_RDWR);
  if (dev < 0) return dev;

  // Open loader and get loader size
  ldr = open(loader, 0);
  if (ldr < 0) return ldr;

  size = fstat(ldr, NULL);

  // Read super block from device
  rc = lseek(dev, 1 * SECTORSIZE, SEEK_SET);
  if (rc < 0) return rc;

  rc = read(dev, ssect, SECTORSIZE);
  if (rc < 0) return rc;

  super = (struct superblock *) ssect;
  blocksize = 1 << super->log_block_size;

  // Calculate loader start and size in sectors (used by bootstrap)
  ldr_start = super->first_reserved_block * (blocksize / SECTORSIZE);
  ldr_size = size / SECTORSIZE;
  if (size > (int) super->reserved_blocks * blocksize) 
  {
    printf("Loader too big\n");
    return -EIO;
  }

  // Install loader
  rc = lseek(dev, super->first_reserved_block * blocksize, SEEK_SET);
  if (rc < 0) return rc;

  for (n = 0; n < size / blocksize; n++)
  {
    rc = read(ldr, block, blocksize);
    if (rc < 0) return rc;

    rc = write(dev, block, blocksize);
    if (rc < 0) return rc;
  }

  close(dev);
  close(ldr);

  return 0;
}

//
// install_boot_sector
//

int install_boot_sector(char *devname, char *bootstrap)
{
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
  boot = open(bootstrap, 0);
  if (boot < 0) return boot;

  rc = read(boot, bsect, SECTORSIZE);
  if (rc < 0) return rc;

  close(boot);

  // Check for partitioned disk
  if (strlen(devname) == 4)
  {
    partno = devname[3] - 'a';
    if (partno < 0 || partno > 3) 
    {
      printf("Invaid partition\n");
      return -EINVAL;
    }

    // Read master boot record and get partition offset
    sprintf(diskname, "/dev/%c%c%c", devname[0], devname[1], devname[2]);
    disk = open(diskname, 0);
    if (disk < 0) return disk;

    rc = read(disk, msect, SECTORSIZE);
    if (rc < 0) return rc;

    close(disk);

    mbr = (struct master_boot_record *) msect;
    if (mbr->signature != MBR_SIGNATURE) 
    {
      printf("Invalid signature in master boot record\n");
      return -EIO;
    }

    partofs = mbr->parttab[partno].relsect;
    //printf("Installing on partition %d offset %d\n", partno, partofs);
  }
  else
    partofs = 0;

  // Set boot sector parameters
  bootsect = (struct boot_sector *) bsect;
  if (bootsect->signature != MBR_SIGNATURE) 
  {
    printf("Invalid signature in bootstrap");
    return -EINVAL;
  }
  
  bootsect->ldrstart = ldr_start + partofs;
  bootsect->ldrsize = ldr_size;

  // Write boot sector to target device
  dev = open(str, O_RDWR);
  if (dev < 0) return dev;

  rc = lseek(dev, 0 * SECTORSIZE, SEEK_SET);
  if (rc < 0) return rc;

  rc = write(dev, bsect, SECTORSIZE);
  if (rc < 0) return rc;

  close(dev);

  return 0;
}

//
// dosysprep
//

int dosysprep(struct section *sect)
{
  char *devname;
  char *bootstrap;
  char *loader;
  int rc;

  // Get properties
  devname = get_property(inst, sect->name, "device", "<none>");
  bootstrap = get_property(inst, sect->name, "bootstrap", "/setup/boot");
  loader = get_property(inst, sect->name, "loader", "/setup/osldr.dll");

  // Install loader
  rc = install_loader(devname, loader);
  if (rc < 0) return rc;

  // Install boot sector
  rc = install_boot_sector(devname, bootstrap);
  if (rc < 0) return rc;

  return 0;
}

//
// dokernel
//

int dokernel(struct section *sect)
{
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
  kernel = get_property(inst, sect->name, "kernel", "/setup/krnl.dll");
  target = get_property(inst, sect->name, "target", "/mnt/bin");

  printf("Installing kernel %s\n", kernel);

  // Open source kernel file
  fin = open(kernel, 0);
  if (fin < 0) return fin;

  size = fstat(fin, NULL);

  // Make sure os directory extist on target
  if (stat(target, NULL) < 0)
  {
    rc = mkdir(target);
    if (rc < 0) return rc;
  }

  // Install kernel on target using reserved inode
  sprintf(targetfn, "%s/krnl.dll", target);
  fout = open(targetfn, O_SPECIAL | (DFS_INODE_KRNL << 24));
  if (fout < 0) return rc;

  left = size;
  while (left > 0)
  {
    bytes = read(fin, block, 4096);
    if (!bytes) return -EIO;
    if (bytes < 0) return bytes;

    rc = write(fout, block, bytes);
    if (rc < 0) return rc;

    left -= bytes;
  }

  close(fin);
  close(fout);

  return 0;
}

//
// domount
//

int domount(struct section *sect)
{
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

  return rc;
}

//
// dounmount
//

int dounmount(struct section *sect)
{
  char *path;
  int rc;

  // Get properties
  path = get_property(inst, sect->name, "path", "/mnt");

  // Unmount file system
  printf("Unmounting filesystem %s\n", path);
  rc = umount(path);

  return rc;
}

//
// domkdirs
//

int domkdirs(struct section *sect)
{
  struct property *prop;
  char *dirname;
  int rc;

  prop = sect->properties;
  while (prop)
  {
    dirname = prop->name;
    printf("Creating directory %s\n", dirname);

    rc = mkdir(dirname);
    if (rc < 0) return rc;

    prop = prop->next;
  }

  return 0;
}

//
// copy_file
//

int copy_file(char *srcfn, char *dstfn)
{
  int fin;
  int fout;
  int bytes;
  int rc;

  fin = open(srcfn, 0);
  if (fin < 0) return fin;

  fout = open(dstfn,  O_CREAT | O_EXCL, S_IREAD | S_IWRITE);
  if (fout < 0) return fout;

  while ((bytes = read(fin , block, 4096)) > 0)
  {
    rc = write(fout, block, bytes);
    if (rc < 0) return rc;
  }

  if (bytes < 0) return bytes;

  close(fin);
  close(fout);

  return 0;
}

//
// copy_dir
//

int copy_dir(char *srcdir, char *dstdir)
{
  struct copyitem
  {
    char srcdir[MAXPATH];
    char dstdir[MAXPATH];
    struct copyitem *next;
  };

  struct copyitem *head;
  struct copyitem *tail;
  struct copyitem *next;
  int dir;
  struct dirent dirp;
  struct stat buf;
  char srcfn[MAXPATH];
  char dstfn[MAXPATH];
  int rc;

  head = tail = (struct copyitem *) malloc(sizeof(struct copyitem));
  if (!head) return -ENOMEM;
  strcpy(head->srcdir, srcdir);
  strcpy(head->dstdir, dstdir);
  head->next = NULL;
  
  while (head)
  {
    dir = opendir(head->srcdir);
    if (dir < 0) return dir;

    while (readdir(dir, &dirp, 1) > 0)
    {
      sprintf(srcfn, "%s/%s", head->srcdir, dirp.name);
      sprintf(dstfn, "%s/%s", head->dstdir, dirp.name);

      rc = stat(srcfn, &buf);
      if (rc < 0) return rc;

      if ((buf.mode & S_IFMT) == S_IFDIR)
      {
        printf("Creating directory %s\n", dstfn);
	rc = mkdir(dstfn);
	if (rc < 0) return rc;

	tail->next = (struct copyitem *) malloc(sizeof(struct copyitem));
	if (!tail->next) return -ENOMEM;
	tail = tail->next;
	strcpy(tail->srcdir, srcfn);
	strcpy(tail->dstdir, dstfn);
	tail->next = NULL;
      }
      else
      {
        printf("Copying %s to %s\n", srcfn, dstfn);
        rc = copy_file(srcfn, dstfn);
        if (rc < 0) return rc;
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

int docopy(struct section *sect)
{
  struct property *prop;
  char *srcfn;
  char *dstfn;
  struct stat buf;
  int rc;

  prop = sect->properties;
  while (prop)
  {
    dstfn = prop->name;
    srcfn = prop->value;

    printf("Installing %s\n", dstfn);
    
    rc = stat(srcfn, &buf);
    if (rc < 0) return rc;

    if ((buf.mode & S_IFMT) == S_IFDIR)
    {
      printf("Creating directory %s\n", dstfn);
      rc = mkdir(dstfn);
      if (rc < 0) return rc;

      rc = copy_dir(srcfn, dstfn);
      if (rc < 0) return rc;
    }
    else
    {
      rc = copy_file(srcfn, dstfn);
      if (rc < 0) return rc;
    }

    prop = prop->next;
  }

  return 0;
}

//
// runscript
//

int runscript(char *scriptname)
{
  struct section *scriptsect;
  struct property *prop;

  scriptsect = find_section(inst, scriptname);
  if (!scriptsect)
  {
    printf("Unable to find script section %s\n", scriptname);
    return -EINVAL;
  }

  prop = scriptsect->properties;
  while (prop)
  {
    char *action;
    char *scriptname;
    int rc;
    struct section *scriptblock;
    
    action = prop->name;
    scriptname = prop->value ? prop->value : prop->name;

    scriptblock = find_section(inst, scriptname);
    if (!scriptblock)
    {
      printf("Unable to find script block %s\n", scriptname);
      return -EINVAL;
    }

    if (strcmp(action, "format") == 0)
      rc = doformat(scriptblock);
    else if (strcmp(action, "sysprep") == 0)
      rc = dosysprep(scriptblock);
    else if (strcmp(action, "mount") == 0)
      rc = domount(scriptblock);
    else if (strcmp(action, "kernel") == 0)
      rc = dokernel(scriptblock);
    else if (strcmp(action, "mkdirs") == 0)
      rc = domkdirs(scriptblock);
    else if (strcmp(action, "copy") == 0)
      rc = docopy(scriptblock);
    else if (strcmp(action, "unmount") == 0)
      rc = dounmount(scriptblock);
    else
    {
      printf("Unknown action '%s' in script block %s\n", action, prop->name);
      return -EINVAL;
    }

    if (rc < 0)
    {
      printf("Error %d (%s) performing %s\n", -rc, strerror(rc), prop->name);
      return rc;
    }
    
    prop = prop->next;
  }

  return 0;
}

//
// main
//

int main(int argc, char *argv[])
{
  char *instfn;
  char *prodname;
  char *prodvers;
  char *scriptname;
  int rc;

  if (argc == 2)
    instfn = argv[1];
  else
    instfn = "/setup/setup.ini";

  // Get setup properties
  inst = read_properties(instfn);
  if (!inst) 
  {
    printf("Error reading install file, %s\n", instfn);
    return 2;
  }

  prodname = get_property(inst, "setup", "product", OSNAME);
  prodvers = get_property(inst, "setup", "version", OSVERSION);
  scriptname = get_property(inst, "setup", "actions", "actions");

  printf("Installing %s version %s\n", prodname, prodvers);

  // Perform install script
  rc = runscript(scriptname);
  if (rc < 0)
  {
    printf("Installation failed\n");
    free_properties(inst);
    return rc;
  }

  // Installation successfull
  printf("%s installed\n", prodname);
  free_properties(inst);

  return 0;
}
