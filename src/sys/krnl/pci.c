//
// pci.c
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// PCI bus interface
//

#include <os/krnl.h>

#define MAX_PCI_BUSSES 8

#define PCI_BRIDGE 0x60400

struct pci_bus *pci_root;
struct pci_bus *pci_busses[MAX_PCI_BUSSES];
int num_pci_busses;
int num_pci_devices;

struct
{
  int classcode;
  char *name;
} pci_classnames[] =
{
  {0x000000, "Non-VGA unclassified device"},
  {0x000100, "VGA compatible unclassified device"},
  {0x010000, "SCSI storage controller"},
  {0x010100, "IDE interface"},
  {0x010200, "Floppy disk controller"},
  {0x010300, "IPI bus controller"},
  {0x010400, "RAID bus controller"},
  {0x018000, "Unknown mass storage controller"},
  {0x020000, "Ethernet controller"},
  {0x020100, "Token ring network controller"},
  {0x020200, "FDDI network controller"},
  {0x020300, "ATM network controller"},
  {0x020400, "ISDN controller"},
  {0x028000, "Network controller"},
  {0x030000, "VGA controller"},
  {0x030100, "XGA controller"},
  {0x030200, "3D controller"},
  {0x038000, "Display controller"},
  {0x040000, "Multimedia video controller"},
  {0x040100, "Multimedia audio controller"},
  {0x040200, "Computer telephony device"},
  {0x048000, "Multimedia controller"},
  {0x050000, "RAM memory"},
  {0x050100, "FLASH memory"},
  {0x058000, "Memory controller"},
  {0x060000, "Host bridge"},
  {0x060100, "ISA bridge"},
  {0x060200, "EISA bridge"},
  {0x060300, "MicroChannel bridge"},
  {0x060400, "PCI bridge"},
  {0x060500, "PCMCIA bridge"},
  {0x060600, "NuBus bridge"},
  {0x060700, "CardBus bridge"},
  {0x060800, "RACEway bridge"},
  {0x060900, "Semi-transparent PCI-to-PCI bridge"},
  {0x060A00, "InfiniBand to PCI host bridge"},
  {0x068000, "Bridge"},
  {0x070000, "Serial controller"},
  {0x070100, "Parallel controller"},
  {0x070200, "Multiport serial controller"},
  {0x070300, "Modem"},
  {0x078000, "Communication controller"},
  {0x080000, "PIC"},
  {0x080100, "DMA controller"},
  {0x080200, "Timer"},
  {0x080300, "RTC"},
  {0x080400, "PCI Hot-plug controller"},
  {0x088000, "System peripheral"},
  {0x090000, "Keyboard controller"},
  {0x090100, "Digitizer Pen"},
  {0x090200, "Mouse controller"},
  {0x090300, "Scanner controller"},
  {0x090400, "Gameport controller"},
  {0x098000, "Input device controller"},
  {0x0A0000, "Generic Docking Station"},
  {0x0A8000, "Docking Station"},
  {0x0B0000, "386"},
  {0x0B0100, "486"},
  {0x0B0200, "Pentium"},
  {0x0B1000, "Alpha"},
  {0x0B2000, "Power PC"},
  {0x0B3000, "MIPS"},
  {0x0B4000, "Co-processor"},
  {0x0C0000, "FireWire (IEEE 1394)"},
  {0x0C0100, "ACCESS Bus"},
  {0x0C0200, "SSA"},
  {0x0C0300, "USB Controller"},
  {0x0C0400, "Fiber Channel"},
  {0x0C0500, "SMBus"},
  {0x0C0600, "InfiniBand"},
  {0x0D0000, "IRDA controller"},
  {0x0D0100, "Consumer IR controller"},
  {0x0D1000, "RF controller"},
  {0x0D8000, "Wireless controller"},
  {0x0E0000, "I2O"},
  {0x0F0000, "Satellite TV controller"},
  {0x0F0100, "Satellite audio communication controller"},
  {0x0F0300, "Satellite voice communication controller"},
  {0x0F0400, "Satellite data communication controller"},
  {0x100000, "Network and computing encryption device"},
  {0x101000, "Entertainment encryption device"},
  {0x108000, "Encryption controller"},
  {0x110000, "DPIO module"},
  {0x110100, "Performance counters"},
  {0x111000, "Communication synchronizer"},
  {0x118000, "Signal processing controller"},
  {0x000000, NULL}
};

unsigned long pci_config_read(int busno, int devno, int funcno, int addr)
{
  _outpd(PCI_CONFIG_ADDR, ((unsigned long) 0x80000000 | (busno << 16) | (devno << 11) | (funcno << 8) | (addr << 2)));
  return _inpd(PCI_CONFIG_DATA);
}

void pci_config_write(int busno, int devno, int funcno, int addr, unsigned long value)
{
  _outpd(PCI_CONFIG_ADDR, ((unsigned long) 0x80000000 | (busno << 16) | (devno << 11) | (funcno << 8) | (addr << 2)));
  _outpd(PCI_CONFIG_DATA, value);
}

char *get_pci_class_name(int classcode)
{
  int i = 0;

  while (pci_classnames[i].name != NULL)
  {
    if (pci_classnames[i].classcode == classcode) return pci_classnames[i].name;
    if (pci_classnames[i].classcode == (classcode & 0xFFFF00)) return pci_classnames[i].name;
    i++;
  }

  return "Unknown";
}

struct pci_dev *lookup_pci_device(unsigned short vendorid, unsigned short deviceid)
{
  int i;
  struct pci_bus *bus;
  struct pci_dev *dev;

  for (i = 0; i < num_pci_busses; i++)
  {
    bus = pci_busses[i];
    dev = bus->devices;

    while (dev)
    {
      if (dev->vendorid == vendorid && dev->deviceid == deviceid) return dev;
      dev = dev->next;
    }
  }

  return NULL;
}

struct pci_dev *lookup_pci_device_class(int classcode, int mask)
{
  int i;
  struct pci_bus *bus;
  struct pci_dev *dev;

  for (i = 0; i < num_pci_busses; i++)
  {
    bus = pci_busses[i];
    dev = bus->devices;

    while (dev)
    {
      if ((dev->classcode & mask) == classcode) return dev;
      dev = dev->next;
    }
  }

  return NULL;
}

static void scan_pci_bus(struct pci_bus *bus)
{
  int devno;
  int funcno;
  int bar;
  unsigned long value;
  unsigned short vendorid;
  unsigned short deviceid;
  struct pci_dev *dev;
  struct pci_dev *devtail;
  struct pci_bus *bustail;
  struct device *dv;

  devtail = NULL;
  bustail = NULL;

  for (devno = 0; devno < 32; devno++)
  {
    for (funcno = 0; funcno < 8; funcno++)
    {
      // Vendor and device ids
      value = pci_config_read(bus->busno, devno, funcno, PCI_CONFIG_VENDOR);
      vendorid = (unsigned short) (value & 0xFFFF);
      deviceid = (unsigned short) (value >> 16);

      if (vendorid != 0xFFFF && vendorid != 0)
      {
	if (funcno == 0 || devtail == NULL || devtail->devno != devno || devtail->deviceid != deviceid)
	{
	  // Allocate and initialize new PCI device
	  dev = (struct pci_dev *) kmalloc(sizeof(struct pci_dev));
	  memset(dev, 0, sizeof(struct pci_dev));

	  dev->bus = bus;
	  dev->devno = devno;
	  dev->funcno = funcno;
	  dev->vendorid = vendorid;
	  dev->deviceid = deviceid;

	  if (devtail) 
	    devtail->next = dev;
	  else
	    bus->devices = dev;
	  devtail = dev;

	  // Function class code
	  value = pci_config_read(bus->busno, devno, funcno, PCI_CONFIG_CLASS_REV);
	  dev->classcode = value >> 8;

	  // Register new device
	  dv = register_device(DEVICE_TYPE_PCI, dev->classcode, (dev->vendorid << 16) + dev->deviceid);
	  dv->name = get_pci_class_name(dev->classcode);
	  dv->pci = dev;
          num_pci_devices++;

	  if (dev->classcode == PCI_BRIDGE)
	  {
	    struct pci_bus *bridge;

	    // PCI-PCI bridge
	    if (num_pci_busses == MAX_PCI_BUSSES) panic("too many pci busses");

            // Allocate and initialize new PCI bus
	    bridge = (struct pci_bus *) kmalloc(sizeof(struct pci_bus));
            memset(bridge, 0, sizeof(struct pci_bus));
	    pci_busses[num_pci_busses++] = bridge;

	    bridge->parent = bus;
	    bridge->self = dev;

	    if (bustail)
	      bustail->next = bridge;
	    else
	      bus->bridges = bridge;
	    bustail = bridge;

	    // Get secondary bus number for bridge
            value = pci_config_read(bus->busno, devno, funcno, PCI_CONFIG_BASE_ADDR_2);
	    bridge->busno = (value >> 8) & 0xFF;

	    // Scan for devices on secondary bus
	    scan_pci_bus(bridge);
	  }
	  else
	  {
	    // Function i/o and memory base addresses
	    for (bar = 0; bar < 6; bar++)
	    {
  	      value = pci_config_read(bus->busno, devno, funcno, PCI_CONFIG_BASE_ADDR_0 + bar);
	      if (value != 0)
	      {
		unsigned long res = value & 0xFFFFFFFC;

		if (value & 1)
		{
		  add_resource(dv, RESOURCE_IO, 0, res, 1);
		  if (dev->iobase == 0) dev->iobase = res;
		}
		else
		{
		  add_resource(dv, RESOURCE_MEM, 0, res, 1);
		  if (dev->membase == 0) dev->membase = res;
		}
	      }

	    }

	    // Function interrupt line
	    value = pci_config_read(bus->busno, devno, funcno, PCI_CONFIG_INTR);
	    if ((value & 0xFF) > 0 && (value & 0xFF) < 32)
	    {
	      dev->intrpin = (value >> 8) & 0xFF;
	      dev->irq = value & 0xFF;
	      add_resource(dv, RESOURCE_IRQ, 0, dev->irq, 1);
	    }
	  }
	}
      }
    }
  }
}

void init_pci()
{
  pci_root = (struct pci_bus *) kmalloc(sizeof(struct pci_bus));
  memset(pci_root, 0, sizeof(struct pci_bus));
 
  pci_busses[0] = pci_root;
  num_pci_busses = 1;

  scan_pci_bus(pci_root);

  if (num_pci_devices > 0)
  {
    kprintf("pci: %d pci bus%s, %d pci device%s\n", num_pci_busses, num_pci_busses != 1 ? "es" : "", num_pci_devices, num_pci_devices != 1 ? "s" : "");
  }
}

void dump_pci_devices()
{
  int i;
  struct pci_bus *bus;
  struct pci_dev *dev;

  for (i = 0; i < num_pci_busses; i++)
  {
    bus = pci_busses[i];
    dev = bus->devices;

    while (dev)
    {
      kprintf("dev %d.%d.%d vendor %X device %X class %X", dev->bus->busno, dev->devno, dev->funcno, dev->vendorid, dev->deviceid, dev->classcode);

      if (dev->classcode == PCI_BRIDGE)
      {
	kprintf(" bridge\n");
      }
      else
      {
	if (dev->membase != 0) kprintf(" mem %X", dev->membase);
	if (dev->iobase != 0) kprintf(" io %X", dev->iobase);
	if (dev->irq != 0) kprintf(" irq %d", dev->irq);
	kprintf("\n");
      }

      dev = dev->next;
    }
  }
}
