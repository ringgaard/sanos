//
// ramdisk.c
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// RAM disk driver
//

#include <os/krnl.h>

#define SECTORSIZE              512

struct ramdisk
{
  unsigned int blks;
  char *data;
};

static int ramdisk_ioctl(struct dev *dev, int cmd, void *args, size_t size)
{
  struct ramdisk *rd = (struct ramdisk *) dev->privdata;

  switch (cmd)
  {
    case IOCTL_GETDEVSIZE:
      return rd->blks;

    case IOCTL_GETBLKSIZE:
      return SECTORSIZE;
  }

  return -ENOSYS;
}

static int ramdisk_read(struct dev *dev, void *buffer, size_t count, blkno_t blkno)
{
  struct ramdisk *rd = (struct ramdisk *) dev->privdata;

  if (count == 0) return 0;
  if (blkno + count / SECTORSIZE > rd->blks) return -EFAULT;
  memcpy(buffer, rd->data + blkno * SECTORSIZE, count);
  return count;
}

static int ramdisk_write(struct dev *dev, void *buffer, size_t count, blkno_t blkno)
{
  struct ramdisk *rd = (struct ramdisk *) dev->privdata;

  if (count == 0) return 0;
  if (blkno + count / SECTORSIZE > rd->blks) return -EFAULT;
  memcpy(rd->data + blkno * SECTORSIZE, buffer, count);
  return count;
}

struct driver ramdisk_driver =
{
  "ramdisk",
  DEV_TYPE_BLOCK,
  ramdisk_ioctl,
  ramdisk_read,
  ramdisk_write
};

void init_ramdisk(char *devname, int size)
{
  struct ramdisk *rd;

  rd = kmalloc(sizeof(struct ramdisk));
  rd->blks = size / SECTORSIZE;
  rd->data = kmalloc(size);
  dev_make(devname, &ramdisk_driver, rd);
  kprintf("%s: ramdisk (%d MB)\n", devname, size / M);
}
