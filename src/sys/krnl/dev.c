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

static void *get_driver_entry(char *module, char *defentry)
{
  char modfn[MAXPATH];
  char *entryname;
  hmodule_t hmod;

  strcpy(modfn, module);
  entryname = strchr(modfn, '!');
  if (entryname)
    *entryname++ = 0;
  else 
    entryname = defentry;

  hmod = load(modfn);
  if (!hmod) return NULL;

  return resolve(hmod, entryname);
}

static void bind_devices()
{
  int n;
  int (*entry)(struct device *dv);
  int rc;

  for (n = 0; n < num_devices; n++)
  {
    struct device *dv = devicetab[n];
    struct binding *bind = find_binding(dv);

    // Install driver
    if (bind)
    {
      entry = get_driver_entry(bind->module, "install");
      if (!entry)
      {
	kprintf("warning: unable to load driver %s for device '%s'\n", bind->module, dv->name);
	continue;
      }

      //kprintf("install driver %s for device '%s' (entry %p)\n", modfn, dv->name, entry);
      rc = entry(dv);
      if (rc < 0)
      {
	kprintf("warning: error %d installing driver %s for device '%s'\n", rc, bind->module, dv->name);
	continue;
      }
    }
  }
}

static void install_nonpnp_drivers()
{
  struct section *sect;
  struct property *prop;
  int (*entry)(char *opts);
  int rc;

  sect = find_section(krnlcfg, "drivers");
  if (!sect) return;

  prop = sect->properties;
  while (prop)
  {
    entry = get_driver_entry(prop->name, "install");
    if (entry)
    {
      rc = entry(prop->value);
      if (rc < 0) kprintf("warning: error %d installing driver %s\n", rc, prop->name);      
    }
    else
      kprintf("warning: unable to load driver %s\n", prop->name);

    prop = prop->next;
  }
}

void install_drivers()
{
  // Parse driver binding database
  parse_bindings();

  // Match bindings to devices
  bind_devices();

  // Install non-PnP drivers
  install_nonpnp_drivers();
}

struct dev *device(devno_t devno)
{
  if (devno < 0 || devno >= num_devs) return NULL;
  return devtab[devno];
}

devno_t dev_make(char *name, struct driver *driver, struct device *dv, void *privdata)
{
  struct dev *dev;
  devno_t devno;
  char *p;
  unsigned int n, m;
  int exists;

  if (num_devs == MAX_DEVS) panic("too many devices");

  dev = (struct dev *) kmalloc(sizeof(struct dev));
  if (!dev) return NODEV;
  memset(dev, 0, sizeof(struct dev));

  strcpy(dev->name, name);
  
  p = dev->name;
  while (p[0] && p[1]) p++;
  if (*p == '#')
  {
    n = 0;
    while (1)
    {
      sprintf(p, "%d", n);
      exists = 0;
      for (m = 0; m < num_devs; m++) 
      {
	if (strcmp(devtab[m]->name, dev->name) == 0) exists = 1;
	break;
      }

      if (!exists) break;
    }
  }

  dev->driver = driver;
  dev->device = dv;
  dev->privdata = privdata;
  dev->refcnt = 0;

  if (dv) dv->dev = dev;

  devno = num_devs++;
  devtab[devno] = dev;
  
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
  if (!dev->driver->ioctl) return -ENOSYS;

  return dev->driver->ioctl(dev, cmd, args, size);
}

int dev_read(devno_t devno, void *buffer, size_t count, blkno_t blkno)
{
  struct dev *dev;

  if (devno < 0 || devno >= num_devs) return -ENODEV;
  dev = devtab[devno];
  if (!dev->driver->read) return -ENOSYS;

  return dev->driver->read(dev, buffer, count, blkno);
}

int dev_write(devno_t devno, void *buffer, size_t count, blkno_t blkno)
{
  struct dev *dev;

  if (devno < 0 || devno >= num_devs) return -ENODEV;
  dev = devtab[devno];
  if (!dev->driver->read) return -ENOSYS;

  return dev->driver->write(dev, buffer, count, blkno);
}

int dev_attach(devno_t devno, struct netif *netif, int (*receive)(struct netif *netif, struct pbuf *p))
{
  struct dev *dev;

  if (devno < 0 || devno >= num_devs) return -ENODEV;
  dev = devtab[devno];
  if (!dev->driver->attach) return -ENOSYS;

  dev->netif = netif;
  dev->receive = receive;
  return dev->driver->attach(dev, &netif->hwaddr);
}

int dev_detach(devno_t devno)
{
  struct dev *dev;
  int rc;

  if (devno < 0 || devno >= num_devs) return -ENODEV;
  dev = devtab[devno];

  if (dev->driver->detach) 
    rc = dev->driver->detach(dev);
  else
    rc = 0;

  dev->netif = NULL;
  dev->receive = NULL;

  return rc;
}

int dev_transmit(devno_t devno, struct pbuf *p)
{
  struct dev *dev;

  if (devno < 0 || devno >= num_devs) return -ENODEV;
  dev = devtab[devno];
  if (!dev->driver->transmit) return -ENOSYS;

  return dev->driver->transmit(dev, p);
}

int dev_receive(devno_t devno, struct pbuf *p)
{
  struct dev *dev;

  if (devno < 0 || devno >= num_devs) return -ENODEV;
  dev = devtab[devno];
  if (!dev->receive) return -ENOSYS;

  return dev->receive(dev->netif, p);
}
