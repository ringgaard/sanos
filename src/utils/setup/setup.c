//
// setup.c
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Setup program
//

#include <os.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inifile.h>

#include <os/mbr.h>
#include <os/dfs.h>

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
// format_target
//

int format_target()
{
  char *fstype;
  int blocksize;
  int quick;
  char options[128];
  int rc;

  // Get formatting properties
  fstype = get_property(inst, "format", "fstype", "dfs");
  blocksize = get_numeric_property(inst, "format", "blocksize", 4096);
  quick = get_numeric_property(inst, "format", "quick", 0);

  sprintf(options, "blocksize=%d%s", blocksize, quick ? ",quick" : "");
  printf("Formatting device %s (%s)...\n", devname);

  // Format device
  rc = format(devname, fstype, options);
  if (rc < 0) return rc;
  printf("format complete\n");
  return 0;
}

//
// install_loader
//

int install_loader()
{
  char *loader;
  int n;
  int dev;
  int ldr;
  int rc;
  int blocksize;
  int size;

  // Get properties
  loader = get_property(inst, "install", "loader", "/setup/osldr.dll");

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
  if (size > (int) super->reserved_blocks * blocksize) panic("loader too big");


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

int install_boot_sector()
{
  char *bootstrap;
  int dev;
  int boot;
  int disk;
  int rc;
  int partno;
  int partofs;
  char diskname[16];

  // Get properties
  bootstrap = get_property(inst, "install", "bootstrap", "/setup/boot");

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
    if (partno < 0 || partno > 3) panic("invaid partition");

    // Read master boot record and get partition offset
    sprintf(diskname, "/dev/%c%c%c", devname[0], devname[1], devname[2]);
    disk = open(diskname, 0);
    if (disk < 0) return disk;

    rc = read(disk, msect, SECTORSIZE);
    if (rc < 0) return rc;

    close(disk);

    mbr = (struct master_boot_record *) msect;
    if (mbr->signature != MBR_SIGNATURE) panic("invalid signature in master boot record");

    partofs = mbr->parttab[partno].relsect;
    //printf("Installing on partition %d offset %d\n", partno, partofs);
  }
  else
    partofs = 0;

  // Set boot sector parameters
  bootsect = (struct boot_sector *) bsect;
  if (bootsect->signature != MBR_SIGNATURE) panic("invalid signature in bootstrap");
  
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
// install_kernel
//

int install_kernel()
{
  char *kernel;
  int fin;
  int fout;
  int rc;
  int size;
  int left;
  int bytes;

  // Get properties
  kernel = get_property(inst, "install", "kernel", "/setup/krnl.dll");

  printf("Installing kernel %s\n", kernel);

  // Open source kernel file
  fin = open(kernel, 0);
  if (fin < 0) return fin;

  size = fstat(fin, NULL);

  // Make sure os directory extist on target
  if (stat("/target/os", NULL) < 0)
  {
    rc = mkdir("/target/os");
    if (rc < 0) return rc;
  }

  // Install kernel on target using reserved inode
  fout = open("/target/os/krnl.dll", O_SPECIAL | (DFS_INODE_KRNL << 24));
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
// create_directories
//

int create_directories()
{
  struct section *dirsect;
  struct property *prop;
  char *dirname;
  int rc;

  dirsect = find_section(inst, "dirs");
  if (dirsect != NULL)
  {
    prop = dirsect->properties;
    while (prop)
    {
      dirname = prop->name;
      printf("Creating directory %s\n", dirname);

      rc = mkdir(dirname);
      if (rc < 0) return rc;

      prop = prop->next;
    }
  }

  return 0;
}

//
// install_files
//

int install_files()
{
  struct section *filesect;
  struct property *prop;
  char *srcfn;
  char *dstfn;
  int fin;
  int fout;
  int bytes;
  int rc;

  filesect = find_section(inst, "files");
  if (filesect != NULL)
  {
    prop = filesect->properties;
    while (prop)
    {
      dstfn = prop->name;
      srcfn = prop->value;

      printf("Copying %s to %s\n", srcfn, dstfn);

      fin = open(srcfn, 0);
      if (fin < 0) return rc;

      fout = open(dstfn,  O_CREAT | O_EXCL);
      if (fout < 0) return rc;

      while ((bytes = read(fin , block, 4096)) > 0)
      {
	rc = write(fout, block, bytes);
	if (rc < 0) return rc;
      }

      if (bytes < 0) return bytes;

      close(fin);
      close(fout);

      prop = prop->next;
    }
  }

  return 0;
}

//
// main
//

int __stdcall main(hmodule_t hmod, char *cmdline, void *env)
{
  char *instfn;
  char *prodname;
  char *prodvers;
  int rc;

  if (cmdline && *cmdline)
    instfn = cmdline;
  else
    instfn = "/setup/setup.ini";

  // Get setup properties
  inst = read_properties(instfn);
  if (!inst) 
  {
    printf("Error reading install file, %s\n", instfn);
    return 2;
  }

  prodname = get_property(inst, "setup", "product", "sanos");
  prodvers = get_property(inst, "setup", "version", "1.0");
  devname = get_property(inst, "format", "device", "unknown");

  printf("Installing %s version %s\n", prodname, prodvers);

  // Format target
  rc = format_target();
  if (rc < 0)
  {
    printf("Error %d formatting device\n", rc);
    return rc;
  }

  // Install loader
  rc = install_loader();
  if (rc < 0)
  {
    printf("Error %d installing loader\n", rc);
    return rc;
  }

  // Install boot sector
  rc = install_boot_sector();
  if (rc < 0)
  {
    printf("Error %d installing boot sector\n", rc);
    return rc;
  }

  // Mount target
  printf("Mounting %s on /target\n", devname);
  rc = mount("dfs", "/target", devname, NULL);
  if (rc < 0)
  {
    printf("Error %d mounting target\n", rc);
    return rc;
  }

  // Install kernel
  rc = install_kernel();
  if (rc < 0)
  {
    printf("Error %d installing kernel\n", rc);
    return rc;
  }

  // Create directories
  rc = create_directories();
  if (rc < 0)
  {
    printf("Error %d creating directories\n", rc);
    return rc;
  }

  // Install files
  rc = install_files();
  if (rc < 0)
  {
    printf("Error %d installing files\n", rc);
    return rc;
  }

  // Unmount target
  rc = unmount("/target");
  if (rc < 0)
  {
    printf("Error %d unmounting target\n", rc);
    return rc;
  }

  // Installation successfull
  printf("%s installed\n", prodname);
  free_properties(inst);
  return 0;
}
