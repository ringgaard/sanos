//
// null.c
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Null device driver
//

#include <os/krnl.h>

devno_t nulldev = NODEV;

static int null_ioctl(struct dev *dev, int cmd, void *args, size_t size)
{
  switch (cmd)
  {
    case IOCTL_GETDEVSIZE:
      return 0;

    case IOCTL_GETBLKSIZE:
      return 1;
  }
  
  return -1;
}

static int null_read(struct dev *dev, void *buffer, size_t count, blkno_t blkno)
{
  return 0;
}

static int null_write(struct dev *dev, void *buffer, size_t count, blkno_t blkno)
{
  return count;
}

struct driver null_driver =
{
  "null",
  DEV_TYPE_STREAM,
  null_ioctl,
  null_read,
  null_write
};

void init_null()
{
  dev_make("null", &null_driver, NULL);
  nulldev = dev_open("null");
}
