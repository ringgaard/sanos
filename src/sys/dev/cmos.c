//
// cmod.c
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// CMOS VMRAM driver
//

#include <os/krnl.h>

#define CMOS_REGISTERS 128

static int cmos_ioctl(struct dev *dev, int cmd, void *args, size_t size)
{
  switch (cmd)
  {
    case IOCTL_GETDEVSIZE:
      return CMOS_REGISTERS;

    case IOCTL_GETBLKSIZE:
      return 1;
  }

  return -ENOSYS;
}

static int cmos_read(struct dev *dev, void *buffer, size_t count, blkno_t blkno)
{
  unsigned int n;

  if (count == 0) return 0;
  if (blkno + count > CMOS_REGISTERS) return -EFAULT;

  for (n = 0; n < count; n++) ((unsigned char *) buffer)[n] = read_cmos_reg(n + blkno);
  return count;
}

static int cmos_write(struct dev *dev, void *buffer, size_t count, blkno_t blkno)
{
  unsigned int n;

  if (count == 0) return 0;
  if (blkno + count > CMOS_REGISTERS) return -EFAULT;

  for (n = 0; n < count; n++) write_cmos_reg(n + blkno, ((unsigned char *) buffer)[n]);
  return count;
}

struct driver cmos_driver =
{
  "cmos",
  DEV_TYPE_BLOCK,
  cmos_ioctl,
  cmos_read,
  cmos_write
};

void init_cmos()
{
  dev_make("cmos", &cmos_driver, NULL);
}
