//
// pci.h
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// PCI bus interface
//

#ifndef PCI_H
#define PCI_H

//
// Ports for access to PCI config space
//

#define PCI_CONFIG_ADDR			0xCF8
#define PCI_CONFIG_DATA			0xCFC

//
// PCI config space register offsets
//

#define PCI_CONFIG_VENDOR		0
#define PCI_CONFIG_CMD_STAT		1
#define PCI_CONFIG_CLASS_REV		2
#define PCI_CONFIG_HDR_TYPE		3
#define PCI_CONFIG_BASE_ADDR_0		4
#define PCI_CONFIG_BASE_ADDR_1		5
#define PCI_CONFIG_BASE_ADDR_2		6
#define PCI_CONFIG_BASE_ADDR_3		7
#define PCI_CONFIG_BASE_ADDR_4		8
#define PCI_CONFIG_BASE_ADDR_5		9
#define PCI_CONFIG_CIS			10
#define PCI_CONFIG_SUBSYSTEM		11
#define PCI_CONFIG_ROM			12
#define PCI_CONFIG_CAPABILITIES		13
#define PCI_CONFIG_INTR			15

//
// PCI device codes
//

#define PCI_CLASS_MASK  	0xFF0000
#define PCI_SUBCLASS_MASK  	0xFFFF00

#define PCI_CLASS_STORAGE_IDE	0x010100

struct pci_bus;

struct pci_dev
{
  struct pci_bus *bus;
  struct pci_dev *next;

  int devno;
  int funcno;

  unsigned short vendorid;
  unsigned short deviceid;

  int classcode;

  unsigned long membase;
  unsigned long iobase;
  
  int intrpin;
  int irq;
};

struct pci_bus
{
  struct pci_bus *parent;
  struct pci_bus *next;
  struct pci_dev *self;

  int busno;
  struct pci_dev *devices;
  struct pci_bus *bridges;
};

extern struct pci_bus *pci_root;

krnlapi unsigned long pci_config_read(int busno, int devno, int funcno, int addr);
krnlapi void pci_config_write(int busno, int devno, int funcno, int addr, unsigned long value);

krnlapi char *get_pci_class_name(int classcode);

krnlapi struct pci_dev *lookup_pci_device(unsigned short vendorid, unsigned short deviceid);
krnlapi struct pci_dev *lookup_pci_device_class(int classcode, int mask);

void init_pci();
void dump_pci_devices();

#endif
