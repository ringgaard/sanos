//
// pci.h
//
// PCI bus interface
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

#define PCI_VENDOR_ID(unitcode) (((unitcode) >> 16) & 0xFFFF)
#define PCI_DEVICE_ID(unitcode) ((unitcode) & 0xFFFF)

#define PCI_DEVNO(unitno) ((unitno) >> 3)
#define PCI_FUNCNO(unitno) ((unitno) & 7)

krnlapi unsigned long pci_config_read(int busno, int devno, int funcno, int addr);
krnlapi void pci_config_write(int busno, int devno, int funcno, int addr, unsigned long value);

krnlapi unsigned long pci_unit_read(struct unit *unit, int addr);
krnlapi void pci_unit_write(struct unit *unit, int addr, unsigned long value);

krnlapi void pci_enable_busmastering(struct unit *unit);

void enum_pci_bus(struct bus *bus);
unsigned long get_pci_hostbus_unitcode();

#endif
