//
// 3c905c.c
//
// Copyright (c) 2002 Søren Thygesen Gjesse. All rights reserved.
//
// 3Com 3C905C NIC network driver
//

#include <os/krnl.h>

// PCI IDs

#define PCI_VENDOR_3COM            0x10B7
#define PCI_DEVICE_3C905C          0x9200

// Commands

#define CMD_RESET                  0x0000
#define CMD_RX_RESET               0x2800
#define CMD_TX_RESET               0x5800
#define CMD_RX_ENABLE              0x2000
#define CMD_TX_ENABLE              0x4800
#define CMD_SET_RX_FILTER          0x8000
#define CMD_REQUEST_INTERRUPT      0x6000
#define CMD_SET_INDICATION_ENABLE  0x7800
#define CMD_SET_INTERRUPT_ENABLE   0x7000
#define CMD_SELECT_WINDOW          0x0800
#define CMD_STATISTICS_ENABLE      0xB000
#define CMD_STATISTICS_DISABLE     0xA800
#define CMD_ACKNOWLEDGE_INTERRUPT  0x6800

// Non-windowed registeres

#define CMD           0x0E  // this register is actually part of the window region but is shared by all windows
#define STATUS        0x0E  // this register is actually part of the window region but is shared by all windows

#define UP_LIST_PTR   0x38

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

#define BAD_SSD               0x0C
#define UPPER_BYTES_OK        0x0D

// Window 5

// Window 6

#define CARRIER_LOST         0x00
#define SQE_ERRORS           0x01
#define MULTIPLE_COLLISIONS  0x02
#define SINGLE_COLLISIONS    0x03
#define LATE_COLLISIONS      0x04
#define RX_OVERRUNS          0x05
#define FRAMES_XMITTED_OK    0x06
#define FRAMES_RCVD_OK       0x07
#define FRAMES_DEFERRED      0x08
#define UPPER_FRAMES_OK      0x09
#define BYTES_RECEIVED_OK    0x0A
#define BYTES_XMITTED_OK     0x0C

#define FIRST_BYTE_STAT      0x00
#define LAST_BYTE_STAT       0x09

// Window 7

// IntStatus flags

#define INTSTATUS_INT_LATCH        0x0001
#define INTSTATUS_HOST_ERROR       0x0002
#define INTSTATUS_TX_COMPLETE      0x0004
#define INTSTATUS_RX_COMPLETE      0x0010
#define INTSTATUS_RX_EARLY         0x0020
#define INTSTATUS_INT_REQUESTED    0x0040
#define INTSTATUS_UPDATE_STATS     0x0080
#define INTSTATUS_LINK_EVENT       0x0100
#define INTSTATUS_DN_COMPLETE      0x0200
#define INTSTATUS_UP_COMPLETE      0x0400
#define INTSTATUS_CMD_IN_PROGRESS  0x1000
#define INTSTATUS_WINDOW_NUMBER    0xE000

// AcknowledgeInterrupt flags

#define INTERRUPT_LATCH_ACK        0x0001
#define LINK_EVENT_ACK             0x0002
#define RX_EARLY_ACK               0x0020
#define INT_REQUESTED_ACK          0x0040
#define DN_COMPLETE_ACK            0x0200
#define UP_COMPLETE_ACK            0x0400

// RxFilter

#define RECEIVE_INDIVIDUAL        0x01
#define RECEIVE_MULTICAST         0x02
#define RECEIVE_BROADCAST         0x04
#define RECEIVE_ALL_FRAMES        0x08
#define RECEIVE_MULTICAST_HASH    0x10

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

struct rx_desc
{
  unsigned long next;
  unsigned long status;
  unsigned long addr;
  unsigned long length;
};

struct nic
{
  unsigned long phys_addr;              // Physical address of this structure
  devno_t devno;                        // Device number

  unsigned short iobase;		// Configured I/O base
  unsigned short irq;		        // Configured IRQ
  unsigned short membase;               // Configured memory base

  struct eth_addr hwaddr;               // MAC address for NIC

  struct dpc dpc;                       // DPC for driver
};

static void dump_dump_status(unsigned short status)
{
  kprintf("3c905c: Status: ");
  if (status & INTSTATUS_INT_LATCH) kprintf(" interruptLatch");
  if (status & INTSTATUS_HOST_ERROR) kprintf(" hostError");
  if (status & INTSTATUS_TX_COMPLETE) kprintf(" txComplete");
  if (status & INTSTATUS_RX_COMPLETE) kprintf(" rxComplete");
  if (status & INTSTATUS_RX_EARLY) kprintf(" rxEarly");
  if (status & INTSTATUS_INT_REQUESTED) kprintf(" intRequested");
  if (status & INTSTATUS_UPDATE_STATS) kprintf(" updateStats");
  if (status & INTSTATUS_LINK_EVENT) kprintf(" linkEvent");
  if (status & INTSTATUS_DN_COMPLETE) kprintf(" dnComplete");
  if (status & INTSTATUS_UP_COMPLETE) kprintf(" upComplete");
  if (status & INTSTATUS_CMD_IN_PROGRESS) kprintf(" cmdInProgress");
  kprintf(" windowNumber: %d", status & INTSTATUS_WINDOW_NUMBER >> 13);
}

void execute_command(unsigned short addr, int cmd, int param)
{
  _outpw((unsigned short) (addr + CMD), (unsigned short) (cmd + param));
}

void execute_command_wait(unsigned short addr, int cmd, int param)
{
  int i = 4000;
  execute_command(addr, cmd, param);
  while (--i > 0)
    if (!(_inpw(addr + STATUS) & INTSTATUS_CMD_IN_PROGRESS))
      return;
  //printk(KERN_ERR "%s: command 0x%04x did not complete! Status=0x%x\n",
  //       dev->name, cmd, inw(dev->base_addr + EL3_STATUS));
}

void select_window(unsigned short addr, int window)
{
  execute_command(addr, CMD_SELECT_WINDOW, window);
}

void clear_statistics(unsigned short addr)
{
  int i;

  select_window(addr, 6);
  for (i = FIRST_BYTE_STAT; i <= LAST_BYTE_STAT; i++) _inp(addr + i);
  _inpw(addr + BYTES_RECEIVED_OK);
  _inpw(addr + BYTES_XMITTED_OK);
  select_window(addr, 4);
  _inp(addr + BAD_SSD);
  _inp(addr + UPPER_BYTES_OK);
}

void update_statistics(struct nic *nic)
{
  int current_window;
  unsigned short rx_frames;
  unsigned short tx_frames;
  unsigned short rx_bytes;
  unsigned short tx_bytes;
  unsigned char upper;
  
  // Read the current window
  current_window = _inpw(nic->iobase + STATUS) >> 13;

  // Read statistics from window 6
  select_window(nic->iobase, 6);

  // frames received/transmitted
  upper = _inp(nic->iobase + UPPER_FRAMES_OK);
  rx_frames = _inp(nic->iobase + FRAMES_RCVD_OK);
  tx_frames = _inp(nic->iobase + FRAMES_XMITTED_OK);
  rx_frames += (upper & 0x0F) << 8;
  tx_frames += (upper & 0xF0) << 4;

  // bytes received/transmitted - upper part added below from window 4
  rx_bytes = _inpw(nic->iobase + BYTES_RECEIVED_OK);
  tx_bytes = _inpw(nic->iobase + BYTES_XMITTED_OK);

  _inp(nic->iobase + CARRIER_LOST);
  _inp(nic->iobase + SQE_ERRORS);
  _inp(nic->iobase + MULTIPLE_COLLISIONS);
  _inp(nic->iobase + SINGLE_COLLISIONS);
  _inp(nic->iobase + LATE_COLLISIONS);
  _inp(nic->iobase + RX_OVERRUNS);
  _inp(nic->iobase + FRAMES_DEFERRED);

  // Read final statistics from window 4
  select_window(nic->iobase, 4);

  // update bytes received/transmitted with upper part
  upper = _inp(nic->iobase + UPPER_BYTES_OK);
  rx_bytes += (upper & 0x0F) << 16;
  tx_bytes += (upper & 0xF0) << 12;

  _inp(nic->iobase + BAD_SSD);

  // update global statistics
  stats.link.recv += rx_frames;
  stats.link.xmit += tx_frames;

  // set the window to its previous value
  select_window(nic->iobase, current_window);
}

int nic_transmit(struct dev *dev, struct pbuf *p)
{
  return 0;
}

int nic_ioctl(struct dev *dev, int cmd, void *args, size_t size)
{
  return -ENOSYS;
}

int nic_attach(struct dev *dev, struct eth_addr *hwaddr)
{
  struct nic *nic = dev->privdata;
  *hwaddr = nic->hwaddr;

  return 0;
}

int nic_detach(struct dev *dev)
{
  return 0;
}

struct driver nic_driver =
{
  "3c905c",
  DEV_TYPE_PACKET,
  nic_ioctl,
  NULL,
  NULL,
  nic_attach,
  nic_detach,
  nic_transmit
};

void nic_dpc(void *arg)
{
  struct nic *nic = (struct nic *) arg;
  unsigned short iobase = nic->iobase;
  unsigned short status;

  // read the status
  status = _inpw(iobase + STATUS);

  // return if no interrupt - caused by shared interrupt
  if (!(status & INTSTATUS_INT_LATCH)) return;

  do
  {
    dump_dump_status(status);

    // Acknowledge the interrupt
    execute_command(iobase, CMD_ACKNOWLEDGE_INTERRUPT, INTERRUPT_LATCH_ACK | INT_REQUESTED_ACK);
  }
  while ((status = _inpw(iobase + STATUS)) & (INTSTATUS_INT_LATCH | INTSTATUS_RX_COMPLETE));

  eoi(nic->irq);
}

void handler(struct context *ctxt, void *arg)
{
  struct nic *nic = (struct nic *) arg;

  // Queue DPC to service interrupt
  queue_irq_dpc(&nic->dpc, nic_dpc, nic);
}


#define EEPROM_SIZE 0x21

int __declspec(dllexport) install(struct unit *unit)
{
  struct nic *nic;
  unsigned short eeprom[EEPROM_SIZE];
  unsigned short eeprom_checksum = 0;
  char *chipname = "3c905c";
  int i, j;
  char str[20];

  kprintf("Hello from 3c905c\n");

  // Check for PCI device
  if (unit->bus->bustype != BUSTYPE_PCI) return -EINVAL;

  // Allocate device structure
  nic = (struct nic *) kmalloc(sizeof(struct nic));
  if (!nic) return -ENOMEM;
  memset(nic, 0, sizeof(struct nic));
  nic->phys_addr = (unsigned long) virt2phys(nic);

  // Setup NIC configuration
  nic->iobase = (unsigned short) get_unit_iobase(unit);
  nic->irq = (unsigned short) get_unit_irq(unit);
  nic->membase = (unsigned short) get_unit_membase(unit);

  // Enable bus mastering
  pci_enable_busmastering(unit);

  // Install interrupt handler
  set_interrupt_handler(IRQ2INTR(nic->irq), handler, nic);
  enable_irq(nic->irq);

  // Read the EEPROM
  select_window(nic->iobase, 0);
  for (i = 0; i < EEPROM_SIZE; i++)
  {
    unsigned short x;
    kprintf("reading %d", i);
    _outpw((unsigned short) (nic->iobase + EEPROM_CMD), (unsigned short) (EEPROM_CMD_READ + i));
    for (j = 0; j < 10; j++)
    {
      usleep(162);
      x = _inpw((unsigned short) (nic->iobase + EEPROM_CMD));
      kprintf(" 0x%x (%d)", x, x);
      if ((_inpw((unsigned short) (nic->iobase + EEPROM_CMD)) & EEPROM_BUSY) == 0) break;
    }
    eeprom[i] = _inpw((unsigned short) (nic->iobase + EEPROM_DATA));
    kprintf(" 0x%x\n", eeprom[i]);
  }

  // Calculate EEPROM checksum
  for (i = 0; i < EEPROM_SIZE - 1; i++) eeprom_checksum ^= eeprom[i];
  eeprom_checksum = (eeprom_checksum ^ (eeprom_checksum >> 8)) & 0xff;
  kprintf("checksum 0x%x 0x%x\n", eeprom_checksum, eeprom[EEPROM_CHECKSUM1]);

  // Get the MAC address from the EEPROM
  for (i = 0; i < ETHER_ADDR_LEN / 2; i++)
    ((unsigned short *) nic->hwaddr.addr)[i] = htons(eeprom[EEPROM_OEM_NODE_ADDRESS1 + i]);

  // Set the card MAC address
  select_window(nic->iobase, 2);
  for (i = 0; i < ETHER_ADDR_LEN; i++) _outp((unsigned short) (nic->iobase + i), nic->hwaddr.addr[i]);

  execute_command_wait(nic->iobase, CMD_TX_RESET, 0);
  execute_command_wait(nic->iobase, CMD_RX_RESET, 0);

  execute_command(nic->iobase, CMD_TX_ENABLE, 0);
  execute_command(nic->iobase, CMD_RX_ENABLE, 0);

  execute_command(nic->iobase, CMD_SET_INDICATION_ENABLE, INTSTATUS_INT_REQUESTED);
  execute_command(nic->iobase, CMD_SET_INTERRUPT_ENABLE, INTSTATUS_INT_REQUESTED);
  execute_command(nic->iobase, CMD_REQUEST_INTERRUPT, 0);

  // TEST
  {
    struct rx_desc *desc;

    execute_command(nic->iobase, CMD_SET_RX_FILTER, RECEIVE_INDIVIDUAL);

    desc = (struct rx_desc *) kmalloc(sizeof(struct rx_desc));
    desc->addr = (unsigned long) virt2phys(kmalloc(1600));
    desc->length = 1600;
    desc->next = 0;
    _outpd((unsigned short) (nic->iobase + UP_LIST_PTR), (unsigned long) virt2phys(desc));
  }

  nic->devno = dev_make("nic#", &nic_driver, unit, nic);

  kprintf("%s: 3com %s iobase 0x%x irq %d hwaddr %s\n", device(nic->devno)->name, chipname, nic->iobase, nic->irq, ether2str(&nic->hwaddr, str));

  return 0;
}

int __stdcall start(hmodule_t hmod, int reason, void *reserved2)
{
  return 1;
}
