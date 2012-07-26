//
// dev.c
//
// Device Manager
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

#include <os/krnl.h>

struct unit *units;
struct bus *buses;

struct binding *bindtab;
int num_bindings;

struct dev *devtab[MAX_DEVS];
unsigned int num_devs = 0;

static int units_proc(struct proc_file *pf, void *arg);
static int devices_proc(struct proc_file *pf, void *arg);
static int devstat_proc(struct proc_file *pf, void *arg);

static char *busnames[] = {"HOST", "PCI", "ISA", "?", "?"};
static char *devtypenames[] = {"?", "stream", "block", "packet"};

struct bus *add_bus(struct unit *self, unsigned long bustype, unsigned long busno) {
  struct bus *bus;
  struct bus *b;

  // Create new bus
  bus = (struct bus *) kmalloc(sizeof(struct bus));
  if (!bus) return NULL;
  memset(bus, 0, sizeof(struct bus));

  bus->self = self;
  if (self) bus->parent = self->bus;
  bus->bustype = bustype;
  bus->busno = busno;

  // Add bus as a bridge on the parent bus
  if (bus->parent) {
    if (bus->parent->bridges) {
      b = bus->parent->bridges;
      while (b->sibling) b = b->sibling;
      b->sibling = bus;
    } else {
      bus->parent->bridges = bus;
    }
  }

  // Add bus to bus list
  if (buses) {
    b = buses;
    while (b->next) b = b->next;
    b->next = bus;
  } else {
    buses = bus;
  }

  return bus;
}

struct unit *add_unit(struct bus *bus, unsigned long classcode, unsigned long unitcode, unsigned long unitno) {
  struct unit *unit;
  struct unit *u;

  // Create new unit
  unit = (struct unit *) kmalloc(sizeof(struct unit));
  if (!unit) return NULL;
  memset(unit, 0, sizeof(struct unit));

  unit->bus = bus;
  unit->classcode = classcode;
  unit->unitcode = unitcode;
  unit->unitno = unitno;

  unit->classname = "";
  unit->vendorname = "";
  unit->productname = "";

  // Add unit to bus
  if (bus) {
    if (bus->units) {
      u = bus->units;
      while (u->sibling) u = u->sibling;
      u->sibling = unit;
    } else {
      bus->units = unit;
    }
  }

  // Add unit to unit list
  if (units) {
    u = units;
    while (u->next) u = u->next;
    u->next = unit;
  } else {
    units = unit;
  }

  return unit;
}

struct resource *add_resource(struct unit *unit, unsigned short type, unsigned short flags, unsigned long start, unsigned long len) {
  struct resource *res;
  struct resource *r;

  // Create new resource
  res = (struct resource *) kmalloc(sizeof(struct resource));
  if (!res) return NULL;
  memset(res, 0, sizeof(struct resource));

  res->type = type;
  res->flags = flags;
  res->start = start;
  res->len = len;

  // Add resource to unit resource list
  if (unit->resources) {
    r = unit->resources;
    while (r->next) r = r->next;
    r->next = res;
  } else {
    unit->resources = res;
  }

  return res;
}

struct resource *get_unit_resource(struct unit *unit, int type, int num) {
  struct resource *res = unit->resources;

  while (res) {
    if (res->type == type) {
      if (num == 0) return res;
      num--;
    }
    res = res->next;
  }

  return NULL;
}

int get_unit_irq(struct unit *unit) {
  struct resource *res = get_unit_resource(unit, RESOURCE_IRQ, 0);
  
  if (res) return res->start;
  return -1;
}

int get_unit_iobase(struct unit *unit) {
  struct resource *res = get_unit_resource(unit, RESOURCE_IO, 0);
  
  if (res) return res->start;
  return -1;
}

void *get_unit_membase(struct unit *unit) {
  struct resource *res = get_unit_resource(unit, RESOURCE_MEM, 0);
  
  if (res) return (void *) (res->start);
  return NULL;
}

char *get_unit_name(struct unit *unit) {
  if (unit->productname && *unit->productname) return unit->productname;
  if (unit->classname && *unit->classname) return unit->classname;
  return "unknown";
}

struct unit *lookup_unit(struct unit *start, unsigned long unitcode, unsigned long unitmask) {
  struct unit *unit;
  
  if (start) {
    unit = start->next;
  } else {
    unit = units;
  }

  while (unit) {
    if ((unit->unitcode & unitmask) == unitcode) return unit;
    unit = unit->next;
  }

  return NULL;
}

struct unit *lookup_unit_by_subunit(struct unit *start, unsigned long subunitcode, unsigned long subunitmask) {
  struct unit *unit;
  
  if (start) {
    unit = start->next;
  } else {
    unit = units;
  }

  while (unit) {
    if ((unit->subunitcode & subunitmask) == subunitcode) return unit;
    unit = unit->next;
  }

  return NULL;
}

struct unit *lookup_unit_by_class(struct unit *start, unsigned long classcode, unsigned long classmask) {
  struct unit *unit;
  
  if (start) {
    unit = start->next;
  } else {
    unit = units;
  }

  while (unit) {
    if ((unit->classcode & classmask) == classcode) return unit;
    unit = unit->next;
  }

  return NULL;
}

struct board *lookup_board(struct board *board_tbl, struct unit *unit) {
  int i = 0;

  while (board_tbl[i].vendorname != NULL) {
    if (unit->bus->bustype == board_tbl[i].bustype &&
        (unit->unitcode & board_tbl[i].unitmask) == board_tbl[i].unitcode &&
        (unit->subunitcode & board_tbl[i].subsystemmask) == board_tbl[i].subsystemcode &&
        (unit->revision & board_tbl[i].revisionmask) == board_tbl[i].revisioncode)
      break;

    i++;
  }

  if (board_tbl[i].vendorname == NULL) return NULL;
  return &board_tbl[i];
}

void enum_host_bus() {
  struct bus *host_bus;
  struct unit *pci_host_bridge;
  unsigned long unitcode;
  struct bus *pci_root_bus;
  struct unit *isa_bridge;
  struct bus *isa_bus;

  // Create host bus
  host_bus = add_bus(NULL, BUSTYPE_HOST, 0);

  unitcode = get_pci_hostbus_unitcode();
  if (unitcode) {
    // Enumerate PCI buses
    pci_host_bridge = add_unit(host_bus, PCI_HOST_BRIDGE, unitcode, 0);
    pci_root_bus = add_bus(pci_host_bridge, BUSTYPE_PCI, 0);

    enum_pci_bus(pci_root_bus);
  } else {
    // Enumerate ISA bus using PnP
    isa_bridge = add_unit(host_bus, PCI_ISA_BRIDGE, 0, 0);
    isa_bus = add_bus(isa_bridge, BUSTYPE_ISA, 0);

    enum_isapnp(isa_bus);
  }
}

static void parse_bindings() {
  struct section *sect;
  struct property *prop;
  struct binding *bind;
  char buf[128];
  char *p;
  char *q;
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
  while (prop) {
    bind = &bindtab[n];

    strcpy(buf, prop->name);
    
    p = q = buf;
    while (*q && *q != ' ') q++;
    if (!*q) continue;
    *q++ = 0;

    if (strcmp(p, "pci") == 0) {
      bind->bustype = BUSTYPE_PCI;
    } else if (strcmp(p, "isa") == 0) {
      bind->bustype = BUSTYPE_ISA;
    }

    while (*q == ' ') q++;
    p = q;
    while (*q && *q != ' ') q++;
    if (!*q) continue;
    *q++ = 0;

    if (strcmp(p, "class") == 0) {
      bind->bindtype = BIND_BY_CLASSCODE;
    } else if (strcmp(p, "unit") == 0) {
      bind->bindtype = BIND_BY_UNITCODE;
    } else if (strcmp(p, "subunit") == 0) {
      bind->bindtype = BIND_BY_SUBUNITCODE;
    }

    while (*q == ' ') q++;

    while (*q) {
      unsigned long digit;
      unsigned long mask;

      mask = 0xF;
      if (*q >= '0' && *q <= '9') {
        digit = *q - '0';
      } else if (*q >= 'A' && *q <= 'F') {
        digit = *q - 'A' + 10;
      } else if (*q >= 'a' && *q <= 'f') {
        digit = *q - 'a' + 10;
      } else {
        digit = 0;
        mask = 0;
      }

      bind->code = (bind->code << 4) | digit;
      bind->mask = (bind->mask << 4) | mask;
      q++;
    }

    bind->module = prop->value;

    prop = prop->next;
    n++;
  }
}

static struct binding *find_binding(struct unit *unit) {
  int n;

  for (n = 0; n < num_bindings; n++) {
    struct binding *bind = &bindtab[n];

    if (bind->bustype == unit->bus->bustype) {
      if (bind->bindtype == BIND_BY_CLASSCODE) {
        if ((unit->classcode & bind->mask) == bind->code) return bind;
      }

      if (bind->bindtype == BIND_BY_UNITCODE) {
        if ((unit->unitcode & bind->mask) == bind->code) return bind;
      }

      if (bind->bindtype == BIND_BY_SUBUNITCODE) {
        if ((unit->subunitcode & bind->mask) == bind->code) return bind;
      }
    }
  }

  return NULL;
}

static int initialize_driver(struct unit *unit, char *driverstr) {
  char *buf;
  char *p;
  char *modname;
  char *entryname;
  char *opts;
  hmodule_t hmod;
  int rc;
  int (*entry)(struct unit *unit, char *opts);

  p = buf = kmalloc(strlen(driverstr) + 1);
  if (!buf) return -ENOMEM;
  memcpy(buf, driverstr, strlen(driverstr) + 1);

  modname = p;
  entryname = strchr(p, '!');
  if (entryname) {
    *entryname++ = 0;
    p = entryname;
  } else {
    entryname = "install";
  }

  opts = strchr(p, ':');
  if (opts) {
    *opts++ = 0;
  } else {
    opts = NULL;
  }

  hmod = load(modname, 0);
  if (!hmod) {
    kprintf(KERN_ERR "dev: unable to load module %s\n", modname);
    kfree(buf);
    return -ENOEXEC;
  }

  entry = resolve(hmod, entryname);
  if (!entry) {
    kprintf(KERN_ERR "dev: unable to find entry %s in module %s\n", entryname, modname);
    unload(hmod);
    kfree(buf);
    return -ENOEXEC;
  }

  rc = entry(unit, opts);
  if (rc < 0) {
    kprintf(KERN_ERR "dev: initialization of %s!%s failed with error %d\n", modname, entryname, rc);
    // NB: it is not always safe to unload the driver module after failure
    //unload(hmod);
    kfree(buf);
    return rc;
  }

  kfree(buf);
  return 0;
}

static void install_driver(struct unit *unit, struct binding *bind) {
  int rc;

  rc = initialize_driver(unit, bind->module);
  if (rc < 0) {
    kprintf(KERN_ERR "dev: driver '%s' failed with error %d for unit %08X '%s'\n", bind->module, rc, unit->unitcode, get_unit_name(unit));
  }
}

static void bind_units()
{
  struct unit *unit = units;

  while (unit) {
    if (unit->dev == NULL) {
      struct binding *bind = find_binding(unit);
      if (bind) install_driver(unit, bind);
    }
    unit = unit->next;
  }
}

static void install_legacy_drivers() {
  struct section *sect;
  struct property *prop;
  int buflen;
  char *buf;
  int rc;

  sect = find_section(krnlcfg, "drivers");
  if (!sect) return;

  prop = sect->properties;
  while (prop) {
    buflen = strlen(prop->name) + 1;
    if (prop->value) buflen += strlen(prop->value) + 1;
    buf = kmalloc(buflen);
    
    if (buf) {
      strcpy(buf, prop->name);
      if (prop->value) {
        strcat(buf, ":");
        strcat(buf, prop->value);
      }

      rc = initialize_driver(NULL, buf);
      if (rc < 0) {
        kprintf(KERN_ERR "dev: error %d initializing driver %s\n", rc, prop->name);
      }

      kfree(buf);
    }

    prop = prop->next;
  }
}

void install_drivers() {
  dev_t console;

  // Register /proc/units
  register_proc_inode("units", units_proc, NULL);
  register_proc_inode("devices", devices_proc, NULL);
  register_proc_inode("devstat", devstat_proc, NULL);

  // Parse driver binding database
  parse_bindings();

  // Match bindings to units
  bind_units();

  // Install legacy drivers
  install_legacy_drivers();

  // Make sure we have a console device
  console = dev_open("console");
  if (console == NODEV) {
    initialize_driver(NULL, "krnl.dll!console");
  } else {
    dev_close(console);
  }
}

struct dev *device(dev_t devno) {
  if (devno < 0 || devno >= num_devs) return NULL;
  return devtab[devno];
}

dev_t dev_make(char *name, struct driver *driver, struct unit *unit, void *privdata) {
  struct dev *dev;
  dev_t devno;
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
  if (*p == '#') {
    n = 0;
    while (1) {
      sprintf(p, "%d", n);
      exists = 0;
      for (m = 0; m < num_devs; m++)  {
        if (strcmp(devtab[m]->name, dev->name) == 0) {
          exists = 1;
          break;
        }
      }

      if (!exists) break;
      n++;
    }
  }

  dev->driver = driver;
  dev->unit = unit;
  dev->privdata = privdata;
  dev->refcnt = 0;
  dev->mode = 0600;

  switch (dev->driver->type) {
    case DEV_TYPE_STREAM: dev->mode |= S_IFCHR; break;
    case DEV_TYPE_BLOCK: dev->mode |= S_IFBLK; break;
    case DEV_TYPE_PACKET: dev->mode |= S_IFPKT; break;
  }

  if (unit) unit->dev = dev;

  devno = num_devs++;
  devtab[devno] = dev;
  
  return devno;
}

dev_t devno(char *name) {
  dev_t devno;

  for (devno = 0; devno < num_devs; devno++) {
    if (strcmp(devtab[devno]->name, name) == 0) return devno;
  }
  return NODEV;
}

dev_t dev_open(char *name) {
  dev_t d = devno(name);
  if (d != NODEV) devtab[d]->refcnt++;
  return d;
}

int dev_close(dev_t devno) {
  if (devno < 0 || devno >= num_devs) return -ENODEV;
  if (devtab[devno]->refcnt == 0) return -EPERM;
  devtab[devno]->refcnt--;
  return 0;
}

int dev_ioctl(dev_t devno, int cmd, void *args, size_t size) {
  struct dev *dev;

  if (devno < 0 || devno >= num_devs) return -ENODEV;
  dev = devtab[devno];
  if (!dev->driver->ioctl) return -ENOSYS;

  return dev->driver->ioctl(dev, cmd, args, size);
}

int dev_read(dev_t devno, void *buffer, size_t count, blkno_t blkno, int flags) {
  struct dev *dev;

  if (devno < 0 || devno >= num_devs) return -ENODEV;
  dev = devtab[devno];
  if (!dev->driver->read) return -ENOSYS;
  dev->reads++;
  dev->input += count;

  return dev->driver->read(dev, buffer, count, blkno, flags);
}

int dev_write(dev_t devno, void *buffer, size_t count, blkno_t blkno, int flags) {
  struct dev *dev;

  if (devno < 0 || devno >= num_devs) return -ENODEV;
  dev = devtab[devno];
  if (!dev->driver->read) return -ENOSYS;
  dev->writes++;
  dev->output += count;

  return dev->driver->write(dev, buffer, count, blkno, flags);
}

int dev_attach(dev_t devno, struct netif *netif, int (*receive)(struct netif *netif, struct pbuf *p)) {
  struct dev *dev;

  if (devno < 0 || devno >= num_devs) return -ENODEV;
  dev = devtab[devno];
  if (!dev->driver->attach) return -ENOSYS;

  dev->netif = netif;
  dev->receive = receive;
  return dev->driver->attach(dev, &netif->hwaddr);
}

int dev_detach(dev_t devno) {
  struct dev *dev;
  int rc;

  if (devno < 0 || devno >= num_devs) return -ENODEV;
  dev = devtab[devno];

  if (dev->driver->detach) {
    rc = dev->driver->detach(dev);
  } else {
    rc = 0;
  }

  dev->netif = NULL;
  dev->receive = NULL;

  return rc;
}

int dev_transmit(dev_t devno, struct pbuf *p) {
  struct dev *dev;

  if (devno < 0 || devno >= num_devs) return -ENODEV;
  dev = devtab[devno];
  if (!dev->driver->transmit) return -ENOSYS;
  dev->writes++;
  dev->output += p->tot_len;

  return dev->driver->transmit(dev, p);
}

int dev_receive(dev_t devno, struct pbuf *p) {
  struct dev *dev;

  if (devno < 0 || devno >= num_devs) return -ENODEV;
  dev = devtab[devno];
  if (!dev->receive) return -ENOSYS;
  dev->reads++;
  dev->input += p->tot_len;

  return dev->receive(dev->netif, p);
}

int dev_setevt(dev_t devno, int events) {
  struct dev *dev;

  if (devno < 0 || devno >= num_devs) return -ENODEV;
  dev = devtab[devno];
  devfs_setevt(dev, events);
  return 0;
}

int dev_clrevt(dev_t devno, int events) {
  struct dev *dev;

  if (devno < 0 || devno >= num_devs) return -ENODEV;
  dev = devtab[devno];
  devfs_clrevt(dev, events);
  return 0;
}

static int units_proc(struct proc_file *pf, void *arg) {
  struct unit *unit;
  struct resource *res;
  int bustype;
  int busno;

  unit = units;
  while (unit) {
    if (unit->bus) {
      bustype = unit->bus->bustype;
      busno = unit->bus->busno;
    } else {
      bustype = BUSTYPE_HOST;
      busno = 0;
    }
    
    pprintf(pf, "%s unit %d.%d class %08X code %08X %s:\n", busnames[bustype], busno, unit->unitno,unit->classcode, unit->unitcode, get_unit_name(unit));

    if (unit->subunitcode != 0 || unit ->revision != 0) {
      pprintf(pf, "  subunitcode: %08X revision %d\n", unit->subunitcode, unit->revision);
    }

    res = unit->resources;
    while (res) {
      switch (res->type) {
        case RESOURCE_IO: 
          if (res->len == 1) { 
            pprintf(pf, "  io: 0x%03x", res->start);
          } else {
            pprintf(pf, "  io: 0x%03x-0x%03x", res->start, res->start + res->len - 1);
          }
          break;

        case RESOURCE_MEM:
          if (res->len == 1) {
            pprintf(pf, "  mem: 0x%08x", res->start);
          } else {
            pprintf(pf, "  mem: 0x%08x-0x%08x", res->start, res->start + res->len - 1);
          }
          break;

        case RESOURCE_IRQ:
          if (res->len == 1) {
            pprintf(pf, "  irq: %d", res->start);
          } else {
            pprintf(pf, "  irq: %d-%d", res->start, res->start + res->len - 1);
          }
          break;

        case RESOURCE_DMA:
          if (res->len == 1) {
            pprintf(pf, "  dma: %d", res->start);
          } else {
            pprintf(pf, "  dma: %d-%d", res->start, res->start + res->len - 1);
          }
          break;
      }

      pprintf(pf, "\n");

      res = res->next;
    }

    unit = unit->next;
  }

  return 0;
}

static int devices_proc(struct proc_file *pf, void *arg) {
  dev_t devno;
  struct dev *dev;

  pprintf(pf, "devno name     driver           type   unit\n");
  pprintf(pf, "----- -------- ---------------- ------ ------------------------------\n");

  for (devno = 0; devno < num_devs; devno++) {
    dev = devtab[devno];
    pprintf(pf, "%5d %-8s %-16s %-6s ", devno, dev->name, dev->driver->name, devtypenames[dev->driver->type]);
    if (dev->unit) {
      pprintf(pf, "%s unit %d.%d\n", busnames[dev->unit->bus->bustype], dev->unit->bus->busno, dev->unit->unitno);
    } else {
      pprintf(pf, "<none>\n");
    }
  }

  return 0;
}

static int devstat_proc(struct proc_file *pf, void *arg) {
  dev_t devno;
  struct dev *dev;

  pprintf(pf, "devno name        reads      input   writes     output\n");
  pprintf(pf, "----- -------- -------- ---------- -------- ----------\n");

  for (devno = 0; devno < num_devs; devno++) {
    dev = devtab[devno];
    pprintf(pf, "%5d %-8s%9d%11d%9d%11d\n", devno, dev->name, dev->reads, dev->input, dev->writes, dev->output);
  }

  return 0;
}
