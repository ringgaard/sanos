//
// dev.c
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Device Manager
//

#include <os/krnl.h>

struct dev *devtab[MAX_DEVS];
unsigned int num_devs = 0;

struct device *devicetab[MAX_DEVICES];
unsigned int num_devices = 0;

struct device *register_device(int type)
{
  struct device *dv;

  if (num_devices == MAX_DEVICES) panic("too many physical devices");
  dv = (struct device *) kmalloc(sizeof(struct device));
  memset(dv, 0, sizeof(struct device));
  devicetab[num_devices++] = dv;

  dv->type = type;

  return dv;
}

int add_resource(struct device *dv, int type, int flags, unsigned long start, unsigned long len)
{
  if (dv->numres == MAX_RESOURCES) return -ENOMEM;
  dv->res[dv->numres].type = type;
  dv->res[dv->numres].flags = flags;
  dv->res[dv->numres].start = start;
  dv->res[dv->numres].len = len;
  dv->numres++;
  return 0;
}

struct dev *device(devno_t devno)
{
  if (devno < 0 || devno >= num_devs) return NULL;
  return devtab[devno];
}

devno_t dev_make(char *name, struct driver *driver, void *privdata)
{
  struct dev *dev;
  devno_t devno;

  if (num_devs == MAX_DEVS) panic("too many devices");
  devno = num_devs++;

  dev = (struct dev *) kmalloc(sizeof(struct dev));
  devtab[devno] = dev;

  dev->name = name;
  dev->driver = driver;
  dev->privdata = privdata;
  dev->refcnt = 0;

  return devno;
}

devno_t dev_open(char *name)
{
  devno_t devno;

  for (devno = 0; devno < num_devs; devno++)
  {
    if (strcmp(devtab[devno]->name, name) == 0)
    {
      devtab[devno]->refcnt++;
      return devno;
    }
  }

  return NODEV;
}

int dev_close(devno_t devno)
{
  if (devno < 0 || devno >= num_devs) return -ENODEV;
  if (devtab[devno]->refcnt == 0) return -EPERM;
  devtab[devno]->refcnt--;
  return 0;
}

int dev_ioctl(devno_t devno, int cmd, void *args, size_t size)
{
  struct dev *dev;

  if (devno < 0 || devno >= num_devs) return -ENODEV;
  dev = devtab[devno];

  return dev->driver->ioctl(dev, cmd, args, size);
}

int dev_read(devno_t devno, void *buffer, size_t count, blkno_t blkno)
{
  struct dev *dev;

  if (devno < 0 || devno >= num_devs) return -ENODEV;
  dev = devtab[devno];

  return dev->driver->read(dev, buffer, count, blkno);
}

int dev_write(devno_t devno, void *buffer, size_t count, blkno_t blkno)
{
  struct dev *dev;

  if (devno < 0 || devno >= num_devs) return -ENODEV;
  dev = devtab[devno];

  return dev->driver->write(dev, buffer, count, blkno);
}
