//
// 3c905c.c
//
// Copyright (c) 2002 Søren Thygesen Gjesse. All rights reserved.
//
// 3Com 3C905C NIC network driver
//

#include <os/krnl.h>

#define ETHER_FRAME_LEN         1544
#define EEPROM_SIZE             0x21

//
// PCI IDs
//

#define UNITCODE_3C905B1           PCI_UNITCODE(0x10B7, 0x9055)
#define UNITCODE_3C905C            PCI_UNITCODE(0x10B7, 0x9200)

//
// Commands
//

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
#define CMD_UP_STALL		   0x3000
#define CMD_UP_UNSTALL		   0x3001
#define CMD_DOWN_STALL		   0x3002
#define CMD_DOWN_UNSTALL	   0x3003

//
// Command arguments
//

#define ACKNOWLEDGE_ALL_INTERRUPTS	0x7FF   

//
// Non-windowed registers
//

#define CMD                   0x0E 
#define STATUS                0x0E

#define TIMER	              0x1A
#define TX_STATUS	      0x1B
#define INT_STATUS_AUTO       0x1E
#define DMA_CONTROL  	      0x20
#define DOWN_LIST_POINTER     0x24
#define DOWN_POLL  	      0x2D
#define UP_PACKET_STATUS      0x30
#define FREE_TIMER  	      0x34
#define COUNTDOWN 	      0x36
#define UP_LIST_POINTER       0x38
#define UP_POLL	              0x3D
#define REAL_TIME_COUNTER     0x40
#define CONFIG_ADDRESS 	      0x44
#define CONFIG_DATA   	      0x48
#define DEBUG_DATA 	      0x70
#define DEBUG_CONTROL 	      0x74

//
// Window 0
//

#define BIOS_ROM_ADDR         0x04
#define BIOS_ROM_DATA         0x08
#define EEPROM_CMD            0x0A
#define EEPROM_DATA           0x0C

#define EEPROM_CMD_SUB    0x0000
#define EEPROM_CMD_WRITE  0x0040
#define EEPROM_CMD_READ   0x0080
#define EEPROM_CMD_ERASE  0x00C0
#define EEPROM_BUSY       0x8000

//
// Window 1
//

//
// Window 2
//

#define STATION_ADDRESS_LOW   0x00
#define STATION_ADDRESS_MID   0x02
#define STATION_ADDRESS_HIGH  0x04

//
// Window 3
//

#define INTERNAL_CONFIG       0x00
#define MAXIMUM_PACKET_SIZE   0x04
#define MAC_CONTROL	      0x06
#define MEDIA_OPTIONS  	      0x08
#define RX_FREE	  	      0x0A
#define TX_FREE		      0x0C

//
// Window 4
//

#define NETWORK_DIAGNOSTICS   0x06
#define PHYSICAL_MANAGEMENT   0x08
#define MEDIA_STATUS          0x0A
#define BAD_SSD               0x0C
#define UPPER_BYTES_OK        0x0D

//
// Window 5
//

#define RX_FILTER  	      0x08
#define INTERRUPT_ENABLE      0x0A
#define INDICATION_ENABLE     0x0C

//
// Window 6
//

#define CARRIER_LOST          0x00
#define SQE_ERRORS            0x01
#define MULTIPLE_COLLISIONS   0x02
#define SINGLE_COLLISIONS     0x03
#define LATE_COLLISIONS       0x04
#define RX_OVERRUNS           0x05
#define FRAMES_XMITTED_OK     0x06
#define FRAMES_RCVD_OK        0x07
#define FRAMES_DEFERRED       0x08
#define UPPER_FRAMES_OK       0x09
#define BYTES_RECEIVED_OK     0x0A
#define BYTES_XMITTED_OK      0x0C

#define FIRST_BYTE_STAT       0x00
#define LAST_BYTE_STAT        0x09

//
// Window 7
//

//
// IntStatus flags
//

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

#define ALL_INTERRUPTS		   0x06EE

//
// AcknowledgeInterrupt flags
//

#define INTERRUPT_LATCH_ACK        0x0001
#define LINK_EVENT_ACK             0x0002
#define RX_EARLY_ACK               0x0020
#define INT_REQUESTED_ACK          0x0040
#define DN_COMPLETE_ACK            0x0200
#define UP_COMPLETE_ACK            0x0400

//
// RxFilter
//

#define RECEIVE_INDIVIDUAL        0x01
#define RECEIVE_MULTICAST         0x02
#define RECEIVE_BROADCAST         0x04
#define RECEIVE_ALL_FRAMES        0x08
#define RECEIVE_MULTICAST_HASH    0x10

//
// EEPROM contents
//

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

#define TX_RING_SIZE	          16
#define RX_RING_SIZE	          32
#define TX_MAX_FRAGS              16

#define LAST_FRAG 	          0x80000000  // Last Addr/Len pair in descriptor
#define DN_COMPLETE	          0x00010000  // This packet has been downloaded

struct sg_entry
{
  unsigned long addr;
  unsigned long length;
};

struct rx_desc
{
  unsigned long phys_next;
  unsigned long status;
  struct sg_entry sglist[1];
  struct rx_desc *next;
  struct pbuf *pkt;
};

struct tx_desc
{
  unsigned long phys_next;
  unsigned long framehdr;
  struct sg_entry sglist[TX_MAX_FRAGS];
  struct tx_desc *next;
  struct pbuf *pkt;
};

struct nic
{
  struct rx_desc rx_ring[RX_RING_SIZE];
  struct tx_desc tx_ring[TX_RING_SIZE];
  struct rx_desc *curr_rx;
  struct tx_desc *curr_tx;

  devno_t devno;                        // Device number

  unsigned short iobase;		// Configured I/O base
  unsigned short irq;		        // Configured IRQ

  struct eth_addr hwaddr;               // MAC address for NIC

  struct dpc dpc;                       // DPC for driver

  unsigned short eeprom[EEPROM_SIZE];   // EEPROM contents
};

static void dump_dump_status(unsigned short status)
{
  kprintf("nic: Status: ");
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

__inline void execute_command(struct nic *nic, int cmd, int param)
{
  _outpw(nic->iobase + CMD, (unsigned short) (cmd + param));
}

void execute_command_wait(struct nic *nic, int cmd, int param)
{
  int i = 4000;
  execute_command(nic, cmd, param);
  while (--i > 0)
    if (!(_inpw(nic->iobase + STATUS) & INTSTATUS_CMD_IN_PROGRESS))
      return;

  kprintf("nic: command did not complete\n");
}

__inline void select_window(struct nic *nic, int window)
{
  execute_command(nic, CMD_SELECT_WINDOW, window);
}

static void print_bits(struct proc_file *pf, unsigned long value)
{
  int i;

  i = 0;
  while (value)
  {
    if (value & 1) pprintf(pf, " b%d", i);
    i++;
    value >>= 1;
  }
}

static int nicstat_proc(struct proc_file *pf, void *arg)
{
  struct nic *nic = arg;

  int current_window;
  int i;
  unsigned short rx_frames;
  unsigned short tx_frames;
  unsigned long rx_bytes;
  unsigned long tx_bytes;
  unsigned char upper;
  
  // Read the current window
  current_window = _inpw(nic->iobase + STATUS) >> 13;

  // Read statistics from window 6
  select_window(nic, 6);

  // Frames received/transmitted
  upper = _inp(nic->iobase + UPPER_FRAMES_OK);
  rx_frames = _inp(nic->iobase + FRAMES_RCVD_OK);
  tx_frames = _inp(nic->iobase + FRAMES_XMITTED_OK);
  rx_frames += (upper & 0x0F) << 8;
  tx_frames += (upper & 0xF0) << 4;

  // Bytes received/transmitted - upper part added below from window 4
  rx_bytes = _inpw(nic->iobase + BYTES_RECEIVED_OK) & 0xFFFF;
  tx_bytes = _inpw(nic->iobase + BYTES_XMITTED_OK) & 0xFFFF;

  pprintf(pf, "carrier lost: %lu\n", _inp(nic->iobase + CARRIER_LOST));
  pprintf(pf, "sqe errors: %lu\n", _inp(nic->iobase + SQE_ERRORS));
  pprintf(pf, "multiple collisions: %lu\n", _inp(nic->iobase + MULTIPLE_COLLISIONS));
  pprintf(pf, "single collisions: %lu\n", _inp(nic->iobase + SINGLE_COLLISIONS));
  pprintf(pf, "late collisions: %lu\n", _inp(nic->iobase + LATE_COLLISIONS));
  pprintf(pf, "rx overruns: %lu\n", _inp(nic->iobase + RX_OVERRUNS));
  pprintf(pf, "frames deferred: %lu\n", _inp(nic->iobase + FRAMES_DEFERRED));

  // Read final statistics from window 4
  select_window(nic, 4);

  // Update bytes received/transmitted with upper part
  upper = _inp(nic->iobase + UPPER_BYTES_OK);
  rx_bytes += (upper & 0x0F) << 16;
  tx_bytes += (upper & 0xF0) << 12;

  _inp(nic->iobase + BAD_SSD);

  pprintf(pf, "rx frames: %lu\n", rx_frames);
  pprintf(pf, "rx bytes: %lu\n", rx_bytes);
  pprintf(pf, "tx frames: %lu\n", tx_frames);
  pprintf(pf, "tx bytes: %lu\n", tx_bytes);

  pprintf(pf, "eeprom:\n");
  for (i = 0; i < EEPROM_SIZE; i++) 
  {
    pprintf(pf, " %02X: %04X ", i, nic->eeprom[i]);
    print_bits(pf, nic->eeprom[i]);
    pprintf(pf, "\n");
  }

  select_window(nic, 3);
  pprintf(pf, "media options: ");
  print_bits(pf, _inpw(nic->iobase + MEDIA_OPTIONS));
  pprintf(pf, "\n");

  select_window(nic, 4);
  pprintf(pf, "media status: ");
  print_bits(pf, _inpw(nic->iobase + MEDIA_STATUS));
  pprintf(pf, "\n");

  // Set the window to its previous value
  select_window(nic, current_window);

  return 0;
}

void clear_statistics(struct nic *nic)
{
  int i;

  select_window(nic, 6);
  for (i = FIRST_BYTE_STAT; i <= LAST_BYTE_STAT; i++) _inp(nic->iobase + i);
  _inpw(nic->iobase + BYTES_RECEIVED_OK);
  _inpw(nic->iobase + BYTES_XMITTED_OK);
  select_window(nic, 4);
  _inp(nic->iobase + BAD_SSD);
  _inp(nic->iobase + UPPER_BYTES_OK);
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
  select_window(nic, 6);

  // Frames received/transmitted
  upper = _inp(nic->iobase + UPPER_FRAMES_OK);
  rx_frames = _inp(nic->iobase + FRAMES_RCVD_OK);
  tx_frames = _inp(nic->iobase + FRAMES_XMITTED_OK);
  rx_frames += (upper & 0x0F) << 8;
  tx_frames += (upper & 0xF0) << 4;

  // Bytes received/transmitted - upper part added below from window 4
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
  select_window(nic, 4);

  // Update bytes received/transmitted with upper part
  upper = _inp(nic->iobase + UPPER_BYTES_OK);
  rx_bytes += (upper & 0x0F) << 16;
  tx_bytes += (upper & 0xF0) << 12;

  _inp(nic->iobase + BAD_SSD);

  // Update global statistics
  stats.link.recv += rx_frames;
  stats.link.xmit += tx_frames;

  // Set the window to its previous value
  select_window(nic, current_window);
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
  unsigned short status;

  kprintf("nic: dpc\n");
  // Read the status
  status = _inpw(nic->iobase + STATUS);

  // Return if no interrupt - caused by shared interrupt
  if (!(status & INTSTATUS_INT_LATCH)) return;

  while (1)
  {
    dump_dump_status(status);

    status &= ALL_INTERRUPTS;
    if (!status) break;

    // Handle upload complete event
    if (status & INTSTATUS_UP_COMPLETE)
    {
      kprintf("nic: packet received\n");
      execute_command(nic, CMD_ACKNOWLEDGE_INTERRUPT, UP_COMPLETE_ACK);
    }

    // Acknowledge the interrupt
    execute_command(nic, CMD_ACKNOWLEDGE_INTERRUPT, INTERRUPT_LATCH_ACK);

    // Get next status
    status = _inpw(nic->iobase + STATUS);
  }

  eoi(nic->irq);
}

void handler(struct context *ctxt, void *arg)
{
  struct nic *nic = (struct nic *) arg;

  // Queue DPC to service interrupt
  kprintf("nic: intr\n");
  queue_irq_dpc(&nic->dpc, nic_dpc, nic);
}

int __declspec(dllexport) install(struct unit *unit)
{
  struct nic *nic;
  unsigned short eeprom_checksum = 0;
  int i, j;

  // Check for PCI device
  if (unit->bus->bustype != BUSTYPE_PCI) return -EINVAL;

  // Determine chipset
  switch (unit->unitcode)
  {
    case UNITCODE_3C905B1: 
      unit->vendorname = "3Com";
      unit->productname = "3Com EtherLink 3C905B-1";
      break;

    case UNITCODE_3C905C:
      unit->vendorname = "3Com";
      unit->productname = "3Com EtherLink 3C905C";
      break;

    default:
      unit->vendorname = "3Com";
      unit->productname = "3Com EtherLink 3c90xC";
  }

  // Allocate device structure
  nic = (struct nic *) kmalloc(sizeof(struct nic));
  if (!nic) return -ENOMEM;
  memset(nic, 0, sizeof(struct nic));
  //nic->phys_addr = (unsigned long) virt2phys(nic);

  // Setup NIC configuration
  nic->iobase = (unsigned short) get_unit_iobase(unit);
  nic->irq = (unsigned short) get_unit_irq(unit);

  // Enable bus mastering
  pci_enable_busmastering(unit);

  // Install interrupt handler
  set_interrupt_handler(IRQ2INTR(nic->irq), handler, nic);
  enable_irq(nic->irq);

  // Read the EEPROM
  select_window(nic, 0);
  for (i = 0; i < EEPROM_SIZE; i++)
  {
    unsigned short x;
    _outpw(nic->iobase + EEPROM_CMD, (unsigned short) (EEPROM_CMD_READ + i));
    for (j = 0; j < 10; j++)
    {
      usleep(162);
      x = _inpw(nic->iobase + EEPROM_CMD);
      if ((_inpw(nic->iobase + EEPROM_CMD) & EEPROM_BUSY) == 0) break;
    }
    nic->eeprom[i] = _inpw(nic->iobase + EEPROM_DATA);
  }

  // Calculate EEPROM checksum
  for (i = 0; i < EEPROM_SIZE - 1; i++) eeprom_checksum ^= nic->eeprom[i];
  eeprom_checksum = (eeprom_checksum ^ (eeprom_checksum >> 8)) & 0xff;
  if (eeprom_checksum != nic->eeprom[EEPROM_CHECKSUM1])
  {
    kprintf("warning: nic eeprom checksum error (0x%x,0x%x)\n", eeprom_checksum, nic->eeprom[EEPROM_CHECKSUM1]);
  }

  // Get the MAC address from the EEPROM
  for (i = 0; i < ETHER_ADDR_LEN / 2; i++)
    ((unsigned short *) nic->hwaddr.addr)[i] = htons(nic->eeprom[EEPROM_OEM_NODE_ADDRESS1 + i]);

  // Set the card MAC address
  select_window(nic, 2);
  for (i = 0; i < ETHER_ADDR_LEN; i++) _outp(nic->iobase + i, nic->hwaddr.addr[i]);

  // Setup the receive ring
  for (i = 0; i < RX_RING_SIZE; i++)
  {
    if (i == RX_RING_SIZE - 1)
      nic->rx_ring[i].next = &nic->rx_ring[0];
    else
      nic->rx_ring[i].next = &nic->rx_ring[i + 1];

    nic->rx_ring[i].pkt = pbuf_alloc(PBUF_RAW, ETHER_FRAME_LEN, PBUF_RW);
    if (!nic->rx_ring[i].pkt) return -ENOMEM;

    nic->rx_ring[i].phys_next = (unsigned long) virt2phys(nic->rx_ring[i].next);
    nic->rx_ring[i].sglist[0].addr = (unsigned long) virt2phys(nic->rx_ring[i].pkt->payload);
    nic->rx_ring[i].sglist[0].length = ETHER_FRAME_LEN | LAST_FRAG;
  }
  nic->curr_rx = &nic->rx_ring[0];

  execute_command_wait(nic, CMD_UP_STALL, 0);
  _outpd(nic->iobase + UP_LIST_POINTER, (unsigned long) virt2phys(nic->curr_rx));
  execute_command(nic, CMD_UP_UNSTALL, 0);

  // Select 10BASE-T 
  select_window(nic, 3);
  _outpd(nic->iobase + INTERNAL_CONFIG, _inpd(nic->iobase + INTERNAL_CONFIG) & 0xFFF0FFFF);

  execute_command_wait(nic, CMD_TX_RESET, 0);
  execute_command_wait(nic, CMD_RX_RESET, 0);

  execute_command(nic, CMD_TX_ENABLE, 0);
  execute_command(nic, CMD_RX_ENABLE, 0);

  execute_command(nic, CMD_SET_INDICATION_ENABLE, ALL_INTERRUPTS);
  execute_command(nic, CMD_SET_INTERRUPT_ENABLE, ALL_INTERRUPTS);

  execute_command(nic, CMD_SET_RX_FILTER, RECEIVE_INDIVIDUAL | RECEIVE_MULTICAST | RECEIVE_BROADCAST);

  select_window(nic, 7);

  nic->devno = dev_make("nic#", &nic_driver, unit, nic);

  kprintf("%s: %s iobase 0x%x irq %d hwaddr %la\n", device(nic->devno)->name, unit->productname, nic->iobase, nic->irq, &nic->hwaddr);

  register_proc_inode("nicstat", nicstat_proc, nic);
  return 0;
}

int __stdcall start(hmodule_t hmod, int reason, void *reserved2)
{
  return 1;
}
