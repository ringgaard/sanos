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

#define PCI_HOST_BRIDGE         0x060000
#define PCI_BRIDGE              0x060400
#define PCI_ISA_BRIDGE          0x060100

#define PCI_CLASS_STORAGE_IDE	0x010100

#define PCI_UNITCODE(vendorid, deviceid) ((vendorid) << 16 | (deviceid))
#define PCI_UNITNO(devno, funcno) ((devno) << 3 | (funcno))

#define PCI_DEVNO(unitno) ((unitno) >> 3)
#define PCI_FUNCNO(unitno) ((unitno) & 7)

krnlapi unsigned long pci_config_read(int busno, int devno, int funcno, int addr);
krnlapi void pci_config_write(int busno, int devno, int funcno, int addr, unsigned long value);

krnlapi unsigned long pci_unit_read(struct unit *unit, int addr);
krnlapi void pci_unit_write(struct unit *unit, int addr, unsigned long value);

void enum_pci_bus(struct bus *bus);
unsigned long get_pci_hostbus_unitcode();

#endif
