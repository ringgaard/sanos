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
int num_devices = 0;

struct binding *bindtab;
int num_bindings;

struct device *register_device(int type, unsigned long classcode, unsigned long devicecode)
{
  struct device *dv;

  if (num_devices == MAX_DEVICES) panic("too many physical devices");
  dv = (struct device *) kmalloc(sizeof(struct device));
  memset(dv, 0, sizeof(struct device));
  devicetab[num_devices++] = dv;

  dv->type = type;
  dv->classcode = classcode;
  dv->devicecode = devicecode;

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

static void parse_bindings()
{
  struct section *sect;
  struct property *prop;
  struct binding *bind;
  char *s;
  int n;

  // Parse driver bindings
  sect = find_section(krnlcfg, "bindings");
  if (!sect) return;
  num_bindings = get_section_size(sect);
  if (!num_bindings) return;

  bindtab = (struct binding *) kmalloc(num_bindings * sizeof(struct binding));
  memset(bindtab, 0, num_bindings * sizeof(struct binding));

  n = 0;
  prop = sect->properties;
  while (prop)
  {
    bind = &bindtab[n++];
    s = prop->name;
    bind->type = *s++;
    bind->module = prop->value;

    while (*s)
    {
      unsigned long digit;
      unsigned long mask;

      mask = 0xF;
      if (*s >= '0' && *s <= '9')
	digit = *s - '0';
      else if (*s >= 'A' && *s <= 'F')
	digit = *s - 'A' + 10;
      else if (*s >= 'a' && *s <= 'f')
	digit = *s - 'a' + 10;
      else
      {
	digit = 0;
	mask = 0;
      }

      bind->code = (bind->code << 4) | digit;
      bind->mask = (bind->mask << 4) | mask;
      s++;
    }

    //kprintf("binding %c %08X %08X %s\n", bind->type, bind->code, bind->mask, bind->module);
    prop = prop->next;
  }
}

static struct binding *find_binding(struct device *dv)
{
  int n;

  if (dv->type == DEVICE_TYPE_PCI)
  {
    for (n = 0; n < num_bindings; n++)
    {
      struct binding *bind = &bindtab[n];

      if (bind->type == BIND_PCI_CLASS && (dv->classcode & bind->mask) == bind->code) return bind;
      if (bind->type == BIND_PCI_DEVICE && (dv->devicecode & bind->mask) == bind->code) return bind;
    }
  }
  else if (dv->type == DEVICE_TYPE_PNP)
  {
    for (n = 0; n < num_bindings; n++)
    {
      struct binding *bind = &bindtab[n];

      if (bind->type == BIND_PNP_TYPECODE && (dv->classcode & bind->mask) == bind->code) return bind;
      if (bind->type == BIND_PNP_EISAID && (dv->devicecode & bind->mask) == bind->code) return bind;
    }
  }

  return NULL;
}

void bind_devices()
{
  int n;
  char modfn[MAXPATH];
  char *entryname;
  hmodule_t hmod;
  int (*entry)(struct device *dv);
  int rc;

  // Parse driver binding database
  parse_bindings();

  // Match bindings to devices
  for (n = 0; n < num_devices; n++)
  {
    struct device *dv = devicetab[n];
    struct binding *bind = find_binding(dv);

    // Install driver
    if (bind)
    {
      strcpy(modfn, bind->module);
      entryname = strchr(modfn, ',');
      if (entryname)
	*entryname++ = 0;
      else 
	entryname = "install";

      hmod = load(modfn);
      if (!hmod)
      {
	kprintf("warning: unable to load driver %s for device '%s'\n", modfn, dv->name);
	continue;
      }

      entry = resolve(hmod, entryname);
      if (!entry)
      {
	kprintf("warning: unable to load driver %s entry %s for device '%s'\n", modfn, entryname, dv->name);
	continue;
      }

      //kprintf("install driver %s for device '%s' (entry %p)\n", modfn, dv->name, entry);
      rc = entry(dv);
      if (rc < 0)
      {
	kprintf("warning: error %d loading driver %s for device '%s'\n", rc, modfn, dv->name);
	continue;
      }
    }
  }
}

struct dev *device(devno_t devno)
{
  if (devno < 0 || devno >= num_devs) return NULL;
  return devtab[devno];
}

devno_t dev_make(char *name, struct driver *driver, struct device *device, void *privdata)
{
  struct dev *dev;
  devno_t devno;

  if (num_devs == MAX_DEVS) panic("too many devices");
  devno = num_devs++;

  dev = (struct dev *) kmalloc(sizeof(struct dev));
  devtab[devno] = dev;

  strcpy(dev->name, name);
  dev->driver = driver;
  dev->device = device;
  dev->privdata = privdata;
  dev->refcnt = 0;

  if (device) device->dev = dev;
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
