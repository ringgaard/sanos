//
// 3c905c.c
//
// Copyright (c) 2002 Søren Thygesen Gjesse. All rights reserved.
//
// 3Com 3C905C NIC network driver
//

#include <os/krnl.h>

// PCI IDs

#define PCI_DEVICE_3C905C       0x9200
#define PCI_VENDOR_3COM         0x10B7

// Commands

#define CMD_RESET               0x0000
#define CMD_SELECT_WINDOW       0x0800

// Non-windowed registeres

#define CMD           0x0E  // this register is actually part of the window region but is shared by all windows
#define STATUS        0x0E  // this register is actually part of the window region but is shared by all windows

// Window 0

#define BIOS_ROM_ADDR     0x04
#define EEPROM_CMD        0x0A
#define EEPROM_DATA       0x0C

#define EEPROM_CMD_SUB    0x0000
#define EEPROM_CMD_WRITE  0x0040
#define EEPROM_CMD_READ   0x0080
#define EEPROM_CMD_ERASE  0x00C0
#define EEPROM_BUSY       0x8000

// Window 1

// Window 2

// Window 3

// Window 4

// Window 5

// Window 6

// Window 7

// EEPROM contents

#define EEPROM_NODE_ADDRESS1      0x00
#define EEPROM_NODE_ADDRESS2      0x01
#define EEPROM_NODE_ADDRESS3      0x02
#define EEPROM_DEVICE_ID          0x03
#define EEPROM_MANUFACT_DATE      0x04
#define EEPROM_MANUFACT_DIVISION  0x05
#define EEPROM_MANUFACT_PRODCODE  0x06
#define EEPROM_MANUFACT_ID        0x07
#define EEPROM_PCI_PARM           0x08
#define EEPROM_ROM_INFO           0x09
#define EEPROM_OEM_NODE_ADDRESS1  0x0A
#define EEPROM_OEM_NODE_ADDRESS2  0x0B
#define EEPROM_OEM_NODE_ADDRESS3  0x0C
#define EEPROM_SOFTWARE_INFO      0x0D
#define EEPROM_COMPAT_WORD        0x0E
#define EEPROM_SOFTWARE_INFO2     0x0F
#define EEPROM_CAPABILITIES_WORD  0x10
#define EEPROM_RESERVED_11        0x11
#define EEPROM_INTERNAL_CONFIG0   0x12
#define EEPROM_INTERNAL_CONFIG1   0x13
#define EEPROM_RESERVED_14        0x14
#define EEPROM_SOFTWARE_INFO3     0x15
#define EEPROM_LANWORKD_DATA1     0x16
#define EEPROM_SUBSYSTEM_VENDOR   0x17
#define EEPROM_SUBSYSTEM_ID       0x18
#define EEPROM_MEDIA_OPTIONS      0x19
#define EEPROM_LANWORKD_DATA2     0x1A
#define EEPROM_SMB_ADDRESS        0x1B
#define EEPROM_PCI_PARM2          0x1C
#define EEPROM_PCI_PARM3          0x1D
#define EEPROM_RESERVED_1E        0x1E
#define EEPROM_RESERVED_1F        0x1F
#define EEPROM_CHECKSUM1          0x20


struct card
{
  unsigned long phys_addr;              // Physical address of this structure
  devno_t devno;                        // Device number

  unsigned short iobase;		// Configured I/O base
  unsigned short irq;		        // Configured IRQ
  unsigned short membase;               // Configured memory base

  struct eth_addr hwaddr;               // MAC address for NIC
};

void select_window(unsigned short addr, int window)
{
  _outpw((unsigned short) (addr + CMD), (unsigned short) (CMD_SELECT_WINDOW + window));
}

int card_transmit(struct dev *dev, struct pbuf *p)
{
  return 0;
}

int card_ioctl(struct dev *dev, int cmd, void *args, size_t size)
{
  return -ENOSYS;
}

int card_attach(struct dev *dev, struct eth_addr *hwaddr)
{
  struct card *card = dev->privdata;
  *hwaddr = card->hwaddr;

  return 0;
}

int card_detach(struct dev *dev)
{
  return 0;
}

struct driver card_driver =
{
  "3c905c",
  DEV_TYPE_PACKET,
  card_ioctl,
  NULL,
  NULL,
  card_attach,
  card_detach,
  card_transmit
};

#define EEPROM_SIZE 0x21

int __declspec(dllexport) install(struct unit *unit)
{
  struct card *card;
  unsigned short eeprom[EEPROM_SIZE];
  unsigned short eeprom_checksum = 0;
  char *chipname = "3c905c";
  int i, j;
  char str[20];

  kprintf("Hello from 3c905c\n");

  // Check for PCI device
  if (unit->bus->bustype != BUSTYPE_PCI) return -EINVAL;

  // Allocate device structure
  card = (struct card *) kmalloc(sizeof(struct card));
  if (!card) return -ENOMEM;
  memset(card, 0, sizeof(struct card));
  card->phys_addr = (unsigned long) virt2phys(card);

  // Setup NIC configuration
  card->iobase = (unsigned short) get_unit_iobase(unit);
  card->irq = (unsigned short) get_unit_irq(unit);
  card->membase = (unsigned short) get_unit_membase(unit);

  // Enable bus mastering
  pci_enable_busmastering(unit);

  // Read the EEPROM
  select_window(card->iobase, 0);
  for (i = 0; i < EEPROM_SIZE; i++)
  {
    unsigned short x;
    kprintf("reading %d", i);
    _outpw((unsigned short) (card->iobase + EEPROM_CMD), (unsigned short) (EEPROM_CMD_READ + i));
    for (j = 0; j < 10; j++)
    {
      usleep(162);
      x = _inpw((unsigned short) (card->iobase + EEPROM_CMD));
      kprintf(" 0x%x (%d)", x, x);
      if ((_inpw((unsigned short) (card->iobase + EEPROM_CMD)) & EEPROM_BUSY) == 0) break;
    }
    eeprom[i] = _inpw((unsigned short) (card->iobase + EEPROM_DATA));
    kprintf(" 0x%x\n", eeprom[i]);
  }

  // Calculate EEPROM checksum
  for (i = 0; i < EEPROM_SIZE - 1; i++) eeprom_checksum ^= eeprom[i];
  eeprom_checksum = (eeprom_checksum ^ (eeprom_checksum >> 8)) & 0xff;
  kprintf("checksum 0x%x 0x%x\n", eeprom_checksum, eeprom[EEPROM_CHECKSUM1]);

  // Get the MAC address from the EEPROM
  for (i = 0; i < ETHER_ADDR_LEN / 2; i++)
    ((unsigned short *) card->hwaddr.addr)[i] = htons(eeprom[EEPROM_OEM_NODE_ADDRESS1 + i]);

  // Set the card MAC address
  select_window(card->iobase, 2);
  for (i = 0; i < ETHER_ADDR_LEN; i++) _outp((unsigned short) (card->iobase + i), card->hwaddr.addr[i]);

  card->devno = dev_make("nic#", &card_driver, unit, card);

  kprintf("%s: 3com %s iobase 0x%x irq %d hwaddr %s\n", device(card->devno)->name, chipname, card->iobase, card->irq, ether2str(&card->hwaddr, str));

  return 0;
}

int __stdcall start(hmodule_t hmod, int reason, void *reserved2)
{
  return 1;
}
