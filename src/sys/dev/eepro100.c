//
// eepro100.c
//
// Intel EtherExpress Pro100 NIC network driver
//
// This driver is for the Intel EtherExpress Pro100 (Speedo3) design.
// It should work with all i82557/558/559 boards.
//
// Copyright (C) 2002 Michael Ringgaard. All rights reserved.
// Portions Copyright (C) 1998-2002 by Donald Becker.
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

// User-configurable values
// The first five are undocumented and spelled per Intel recommendations.

static int congenb = 0;   // Enable congestion control in the DP83840
static int txfifo = 8;    // Tx FIFO threshold in 4 byte units, 0-15
static int rxfifo = 8;    // Rx FIFO threshold, default 32 bytes

// Tx/Rx DMA burst length, 0-127, 0 == no preemption, tx==128 -> disabled
static int txdmacount = 128;
static int rxdmacount = 0;

// Copy breakpoint for the copy-only-tiny-buffer Rx method
// Lower values use more memory, but are faster

static int rx_copybreak = 200;

// Maximum events (Rx packets, etc.) to handle at each interrupt

static int max_interrupt_work = 20;

// Maximum number of multicast addresses to filter (vs. rx-all-multicast)

static int multicast_filter_limit = 64;

// The ring sizes should be a power of two for efficiency

#define TX_RING_SIZE  32    // Effectively 2 entries fewer
#define RX_RING_SIZE  32

// Actual number of TX packets queued, must be <= TX_RING_SIZE - 2

#define TX_QUEUE_LIMIT  12
#define TX_QUEUE_UNFULL 8   // Hysteresis marking queue as no longer full

// Operational parameters that usually are not changed

// Time in ticks before concluding the transmitter is hung

#define TX_TIMEOUT  (2*HZ)

// Size of an pre-allocated Rx buffer: <Ethernet MTU> + slack

#define PKT_BUF_SZ    1536

// Bus+endian portability operations

#define virt_to_le32desc(addr)  cpu_to_le32(virt_to_bus(addr))
#define le32desc_to_virt(addr)  bus_to_virt(le32_to_cpu(addr))

// This table drives the PCI probe routines

enum chip_capability_flags 
{ 
  ResetMII = 1, 
  HasChksum = 2
};

struct board_info
{
  char *vendorname;
  char *productname;
  unsigned long unitcode;
  unsigned long unitmask;
  unsigned long subsystemcode;
  unsigned long subsystemmask;
  unsigned long revisioncode;
  unsigned long revisionmask;
  int flags;
};

static struct board_info board_tbl[] = 
{
  {"Intel", "Intel i82559 rev 8", PCI_UNITCODE(0x8086, 0x1229), 0xffffffff, 0, 0, 0, 0xff, HasChksum},
  {"Intel", "Intel PCI EtherExpress Pro100", PCI_UNITCODE(0x8086, 0x1229), 0xffffffff, 0, 0, 0, 0, 0},
  {"Intel", "Intel EtherExpress Pro/100+ i82559ER", PCI_UNITCODE(0x8086, 0x1209), 0xffffffff, 0, 0, 0, 0, ResetMII},
  {"Intel", "Intel EtherExpress Pro/100 type 1029", PCI_UNITCODE(0x8086, 0x1029), 0xffffffff, 0, 0, 0, 0, 0},
  {"Intel", "Intel EtherExpress Pro/100 type 1030", PCI_UNITCODE(0x8086, 0x1030), 0xffffffff, 0, 0, 0, 0, 0},
  {"Intel", "Intel Pro/100 V Network", PCI_UNITCODE(0x8086, 0x2449), 0xffffffff, 0, 0, 0, 0, 0},
  {"Intel", "Intel Pro/100 VE (type 1031)", PCI_UNITCODE(0x8086, 0x1031), 0xffffffff, 0, 0, 0, 0, 0},
  {"Intel", "Intel Pro/100 VM (type 1038)", PCI_UNITCODE(0x8086, 0x1038), 0xffffffff, 0, 0, 0, 0, 0},
  {"Intel", "Intel Pro/100 VM (type 1039)", PCI_UNITCODE(0x8086, 0x1039), 0xffffffff, 0, 0, 0, 0, 0},
  {"Intel", "Intel Pro/100 VM (type 103a)", PCI_UNITCODE(0x8086, 0x103a), 0xffffffff, 0, 0, 0, 0, 0},
  {"HP", "HP/Compaq D510 Intel Pro/100 VM", PCI_UNITCODE(0x8086, 0x103b), 0xffffffff, PCI_UNITCODE(0x0e11, 0x0012), 0, 0, 0, 0},
  {"Intel", "Intel Pro/100 VM (type 103b)", PCI_UNITCODE(0x8086, 0x103b), 0xffffffff, 0, 0, 0, 0, 0},
  {"Intel", "Intel Pro/100 VM (unknown type series 1030)", PCI_UNITCODE(0x8086, 0x1030), 0xfffffff0, 0, 0, 0, 0, 0},
  {NULL,},
};

// Offsets to the various registers
// All accesses need not be longword aligned

enum speedo_offsets 
{
  SCBStatus  = 0,             // Rx/Command Unit status
  SCBCmd     = 2,             // Rx/Command Unit command
  SCBPointer = 4,             // General purpose pointer
  SCBPort    = 8,             // Misc. commands and operands
  SCBflash   = 12,            // Flash memory control
  SCBeeprom  = 14,            // EEPROM memory control
  SCBCtrlMDI = 16,            // MDI interface control
  SCBEarlyRx = 20,            // Early receive byte count
};

// Commands that can be put in a command list entry

enum commands 
{
  CmdNOp           = 0x00000000, 
  CmdIASetup       = 0x00010000, 
  CmdConfigure     = 0x00020000,
  CmdMulticastList = 0x00030000, 
  CmdTx            = 0x00040000, 
  CmdTDR           = 0x00050000,
  CmdDump          = 0x00060000, 
  CmdDiagnose      = 0x00070000,
  CmdTxFlex        = 0x00080000,   // Use "Flexible mode" for CmdTx command

  CmdIntr          = 0x20000000,   // Interrupt after completion
  CmdSuspend       = 0x40000000,   // Suspend after completion
};

enum SCBCmdBits 
{
  SCBMaskCmdDone      = 0x8000, 
  SCBMaskRxDone       = 0x4000, 
  SCBMaskCmdIdle      = 0x2000,
  SCBMaskRxSuspend    = 0x1000, 
  SCBMaskEarlyRx      = 0x0800, 
  SCBMaskFlowCtl      = 0x0400,
  SCBTriggerIntr      = 0x0200, 
  SCBMaskAll          = 0x0100,
  
  // The rest are Rx and Tx commands
  CUStart             = 0x0010, 
  CUResume            = 0x0020, 
  CUHiPriStart        = 0x0030, 
  CUStatsAddr         = 0x0040,
  CUShowStats         = 0x0050,
  CUCmdBase           = 0x0060,  // CU Base address (set to zero)
  CUDumpStats         = 0x0070,  // Dump then reset stats counters
  CUHiPriResume       = 0x00b0,  // Resume for the high priority Tx queue
  RxStart             = 0x0001, 
  RxResume            = 0x0002, 
  RxAbort             = 0x0004, 
  RxAddrLoad          = 0x0006,
  RxResumeNoResources = 0x0007,
};

enum intr_status_bits 
{
  IntrCmdDone   = 0x8000,  
  IntrRxDone    = 0x4000, 
  IntrCmdIdle   = 0x2000,
  IntrRxSuspend = 0x1000, 
  IntrMIIDone   = 0x0800, 
  IntrDrvrIntr  = 0x0400,
  IntrAllNormal = 0xfc00,
};

enum SCBPort_cmds 
{
  PortReset        = 0, 
  PortSelfTest     = 1, 
  PortPartialReset = 2, 
  PortDump         = 3,
};

// The Speedo3 Rx and Tx frame/buffer descriptors

// A generic descriptor

struct descriptor 
{     
  long cmd_status;            // All command and status fields
  unsigned long link;         // struct descriptor *
  unsigned char params[0];
};

// The Speedo3 Rx and Tx buffer descriptors

// Receive frame descriptor

struct RxFD 
{         
  long status;
  unsigned long link;         // struct RxFD *
  unsigned long rx_buf_addr;  // void *
  unsigned long count;
};

// Selected elements of the Tx/RxFD.status word

enum RxFD_bits 
{
  RxComplete     = 0x8000,
  RxOK           = 0x2000,
  RxErrCRC       = 0x0800, 
  RxErrAlign     = 0x0400, 
  RxErrTooBig    = 0x0200, 
  RxErrSymbol    = 0x0010,
  RxEth2Type     = 0x0020, 
  RxNoMatch      = 0x0004, 
  RxNoIAMatch    = 0x0002,
  TxUnderrun     = 0x1000,  
  StatusComplete = 0x8000,
};

// Transmit frame descriptor set

struct TxFD 
{         
  long status;
  unsigned long link;           // void *
  unsigned long tx_desc_addr;   // Always points to the tx_buf_addr element.
  long count;                   // # of TBD (=1), Tx start thresh., etc.

  // This constitutes two "TBD" entries. Non-zero-copy uses only one
  unsigned long tx_buf_addr0;   // void *, frame to be transmitted. 
  long tx_buf_size0;            // Length of Tx frame.
  unsigned long tx_buf_addr1;   // Used only for zero-copy data section.
  long tx_buf_size1;            // Length of second data buffer (0).
};

// Elements of the dump_statistics block. This block must be lword aligned.

struct speedo_stats 
{
  unsigned long tx_good_frames;
  unsigned long tx_coll16_errs;
  unsigned long tx_late_colls;
  unsigned long tx_underruns;
  unsigned long tx_lost_carrier;
  unsigned long tx_deferred;
  unsigned long tx_one_colls;
  unsigned long tx_multi_colls;
  unsigned long tx_total_colls;
  unsigned long rx_good_frames;
  unsigned long rx_crc_errs;
  unsigned long rx_align_errs;
  unsigned long rx_resource_errs;
  unsigned long rx_overrun_errs;
  unsigned long rx_colls_errs;
  unsigned long rx_runt_errs;
  unsigned long done_marker;
};

struct stats_nic
{
  unsigned long rx_packets;             // Total packets received
  unsigned long tx_packets;             // Total packets transmitted
  unsigned long rx_bytes;               // Total bytes received
  unsigned long tx_bytes;               // Total bytes transmitted
  unsigned long rx_errors;              // Bad packets received
  unsigned long tx_errors;              // Packet transmit problems
  unsigned long rx_dropped;             // Received packets dropped
  unsigned long tx_dropped;             // Transmitted packets dropped
  unsigned long multicast;              // Multicast packets received
  unsigned long collisions;

  // Detailed rx errors
  unsigned long rx_length_errors;
  unsigned long rx_over_errors;         // Receiver ring buff overflow
  unsigned long rx_crc_errors;          // Recved pkt with crc error
  unsigned long rx_frame_errors;        // Recv'd frame alignment error
  unsigned long rx_fifo_errors;         // Recv'r fifo overrun
  unsigned long rx_missed_errors;       // Receiver missed packet

  // Detailed tx errors
  unsigned long tx_aborted_errors;
  unsigned long tx_carrier_errors;
  unsigned long tx_fifo_errors;
  unsigned long tx_heartbeat_errors;
  unsigned long tx_window_errors;

  // Compression
  unsigned long rx_compressed;
  unsigned long tx_compressed;
};

// Do not change the position (alignment) of the first few elements!
// The later elements are grouped for cache locality.

struct nic
{
  struct TxFD tx_ring[TX_RING_SIZE];       // Commands (usually CmdTxPacket)
  struct RxFD *rx_ringp[RX_RING_SIZE];     // Rx descriptor, used as ring
  
  // The addresses of a Tx/Rx-in-place packets/buffers.
  struct pbuf *tx_pbuf[TX_RING_SIZE];
  struct pbuf *rx_pbuf[RX_RING_SIZE];
  struct descriptor *last_cmd;             // Last command sent
  unsigned int cur_tx, dirty_tx;           // The ring entries to be free()ed
  unsigned long tx_threshold;              // The value for txdesc.count
  unsigned long last_cmd_time;
  struct sem tx_sem;                       // Semaphore for Tx ring not full
  
  // Rx control, one cache line.
  struct RxFD *last_rxf;                   // Most recent Rx frame
  unsigned int cur_rx, dirty_rx;           // The next free ring entry
  long last_rx_time;                       // Last Rx, in ticks, to handle Rx hang

  devno_t devno;                           // Device number
  struct dev *dev;                         // Device block
  unsigned short iobase;		   // Configured I/O base
  unsigned short irq;		           // Configured IRQ
  struct interrupt intr;                   // Interrupt object for driver
  struct dpc dpc;                          // DPC for driver
  struct timer timer;                      // Media selection timer
  struct eth_addr hwaddr;                  // MAC address for NIC

  struct stats_nic stats;
  struct speedo_stats lstats;
  int alloc_failures;
  int board_id;
  int flags;
  int mc_setup_frm_len;                    // The length of an allocated...
  struct descriptor *mc_setup_frm;         // ... multicast setup frame
  int mc_setup_busy;                       // Avoid double-use of setup frame
  int in_interrupt;                        // Word-aligned dev->interrupt
  char rx_mode;                            // Current PROMISC/ALLMULTI setting
  int tx_full;                             // The Tx queue is full
  int full_duplex;                         // Full-duplex operation requested
  int flow_ctrl;                           // Use 802.3x flow control
  int rx_bug;                              // Work around receiver hang errata
  int rx_bug10;                            // Receiver might hang at 10mbps
  int rx_bug100;                           // Receiver might hang at 100mbps
  int polling;                             // Hardware blocked interrupt line
  int medialock;                           // The media speed/duplex is fixed
  unsigned short phy[2];                   // PHY media interfaces available
  unsigned short advertising;              // Current PHY advertised caps
  unsigned short partner;                  // Link partner caps
  long last_reset;
};

// The parameters for a CmdConfigure operation.
// There are so many options that it would be difficult to document each bit.
// We mostly use the default or recommended settings

const char i82557_config_cmd[22] = 
{
  22, 0x08, 0, 0,  0, 0, 0x32, 0x03,  1, // 1=Use MII  0=Use AUI
  0, 0x2E, 0,  0x60, 0,
  0xf2, 0x48,   0, 0x40, 0xf2, 0x80,    // 0x40=Force full-duplex
  0x3f, 0x05, 
};

const char i82558_config_cmd[22] = 
{
  22, 0x08, 0, 1,  0, 0, 0x22, 0x03,  1, // 1=Use MII  0=Use AUI
  0, 0x2E, 0,  0x60, 0x08, 0x88,
  0x68, 0, 0x40, 0xf2, 0xBD,    // 0xBD->0xFD=Force full-duplex
  0x31, 0x05, 
};

// PHY media interface chips

static const char *phys[] = 
{
  "None", "i82553-A/B", "i82553-C", "i82503",
  "DP83840", "80c240", "80c24", "i82555",
  "unknown-8", "unknown-9", "DP83840A", "unknown-11",
  "unknown-12", "unknown-13", "unknown-14", "unknown-15"
};

enum phy_chips 
{ 
  NonSuchPhy=0, I82553AB, I82553C, I82503, DP83840, S80C240, S80C24, I82555, DP83840A=10
};

static const char is_mii[] = { 0, 1, 1, 0, 1, 1, 0, 1 };

#define clear_suspend(cmd)   ((char *)(&(cmd)->cmd_status))[3] &= ~0x40

#define EE_READ_CMD   (6)

// How to wait for the command unit to accept a command.
// Typically this takes 0 ticks

__inline static void wait_for_cmd_done(long cmd_ioaddr)
{
  int wait = 0;
  int delayed_cmd;
  
  while (++wait <= 100)
  {
    if (inp(cmd_ioaddr) == 0) return;
  }

  delayed_cmd = inp(cmd_ioaddr);
  
  while (++wait <= 10000)
  {
    if (inp(cmd_ioaddr) == 0) break;
  }

  kprintf("eepro100: Command %2.2x was not immediately accepted, %d ticks!\n", delayed_cmd, wait);
}

// Serial EEPROM section.
// A "bit" grungy, but we work our way through bit-by-bit :->.

//  EEPROM_Ctrl bits

#define EE_SHIFT_CLK  0x01  // EEPROM shift clock.
#define EE_CS         0x02  // EEPROM chip select.
#define EE_DATA_WRITE 0x04  // EEPROM chip data in.
#define EE_DATA_READ  0x08  // EEPROM chip data out.
#define EE_ENB        (0x4800 | EE_CS)
#define EE_WRITE_0    0x4802
#define EE_WRITE_1    0x4806
#define EE_OFFSET     SCBeeprom

// Delay between EEPROM clock transitions.
// The code works with no delay on 33Mhz PCI

#define eeprom_delay(ee_addr) inpw(ee_addr)

static int do_eeprom_cmd(long ioaddr, int cmd, int cmd_len)
{
  unsigned retval = 0;
  long ee_addr = ioaddr + SCBeeprom;

  outpw(ee_addr, EE_ENB | EE_SHIFT_CLK);

  // Shift the command bits out
  do 
  {
    short dataval = (cmd & (1 << cmd_len)) ? EE_WRITE_1 : EE_WRITE_0;
    outpw(ee_addr, dataval);
    eeprom_delay(ee_addr);
    outpw(ee_addr, dataval | EE_SHIFT_CLK);
    eeprom_delay(ee_addr);
    retval = (retval << 1) | ((inpw(ee_addr) & EE_DATA_READ) ? 1 : 0);
  } while (--cmd_len >= 0);
  outpw(ee_addr, EE_ENB);

  // Terminate the EEPROM access.
  outpw(ee_addr, EE_ENB & ~EE_CS);

  return retval;
}

static int mdio_read(long ioaddr, int phy_id, int location)
{
  int val, boguscnt = 64 * 10;    // <64 usec. to complete, typ 27 ticks
  outpd(ioaddr + SCBCtrlMDI, 0x08000000 | (location << 16) | (phy_id << 21));
  do 
  {
    val = inpd(ioaddr + SCBCtrlMDI);
    if (--boguscnt < 0) 
    {
      kprintf("eepro100: mdio_read() timed out with val = %8.8x\n", val);
      break;
    }
  } while (!(val & 0x10000000));
  
  return val & 0xffff;
}

static int mdio_write(long ioaddr, int phy_id, int location, int value)
{
  int val, boguscnt = 64 * 10;    // <64 usec. to complete, typ 27 ticks
  outpd(ioaddr + SCBCtrlMDI, 0x04000000 | (location << 16) | (phy_id << 21) | value);
  do 
  {
    val = inpd(ioaddr + SCBCtrlMDI);
    if (--boguscnt < 0)
    {
      kprintf("eepro100: mdio_write() timed out with val = %8.8x\n", val);
      break;
    }
  } while (!(val & 0x10000000));

  return val & 0xffff;
}

#ifdef xxx

static int do_eeprom_cmd(long ioaddr, int cmd, int cmd_len);
static int mdio_read(long ioaddr, int phy_id, int location);
static int mdio_write(long ioaddr, int phy_id, int location, int value);
static int speedo_open(struct net_device *dev);
static void speedo_resume(struct net_device *dev);
static void speedo_timer(unsigned long data);
static void speedo_init_rx_ring(struct net_device *dev);
static void speedo_tx_timeout(struct net_device *dev);
static int speedo_start_xmit(struct sk_buff *skb, struct net_device *dev);
static int speedo_rx(struct net_device *dev);
static void speedo_interrupt(int irq, void *dev_instance, struct pt_regs *regs);
static int speedo_close(struct net_device *dev);
static struct net_device_stats *speedo_get_stats(struct net_device *dev);
static int speedo_ioctl(struct net_device *dev, struct ifreq *rq, int cmd);
static void set_rx_mode(struct net_device *dev);

static void *speedo_found1(struct pci_dev *pdev, void *init_dev, long ioaddr, int irq, int chip_idx, int card_idx)
{
  struct net_device *dev;
  struct speedo_private *sp;
  int i, option;
  unsigned short eeprom[0x100];
  int acpi_idle_state = 0;

  dev = init_etherdev(init_dev, sizeof(struct speedo_private));

  if (dev->mem_start > 0)
    option = dev->mem_start;
  else if (card_idx >= 0  &&  options[card_idx] >= 0)
    option = options[card_idx];
  else
    option = -1;

  acpi_idle_state = acpi_set_pwr_state(pdev, ACPI_D0);

  // Read the station address EEPROM before doing the reset.
  // Nominally his should even be done before accepting the device, but
  // then we wouldn't have a device name with which to report the error.
  // The size test is for 6 bit vs. 8 bit address serial EEPROMs.
 
  {
    unsigned short sum = 0;
    int j;
    int read_cmd, ee_size;

    if ((do_eeprom_cmd(ioaddr, EE_READ_CMD << 24, 27) & 0xffe0000) == 0xffe0000) 
    {
      ee_size = 0x100;
      read_cmd = EE_READ_CMD << 24;
    } 
    else 
    {
      ee_size = 0x40;
      read_cmd = EE_READ_CMD << 22;
    }

    for (j = 0, i = 0; i < ee_size; i++) 
    {
      unsigned short value = do_eeprom_cmd(ioaddr, read_cmd | (i << 16), 27);
      eeprom[i] = value;
      sum += value;
      if (i < 3) 
      {
        dev->dev_addr[j++] = value;
        dev->dev_addr[j++] = value >> 8;
      }
    }

    if (sum != 0xBABA) kprintf("%s: Invalid EEPROM checksum %#4.4x!\n", dev->name, sum);
  }

  // Reset the chip: stop Tx and Rx processes and clear counters.
  // This takes less than 10usec and will easily finish before the next
  // action.
  outpd(ioaddr + SCBPort, PortReset);

  kprintf("%s: %s%s at %#3lx, ", dev->name, eeprom[3] & 0x0100 ? "OEM " : "", pci_id_tbl[chip_idx].name, ioaddr);
  for (i = 0; i < 5; i++) kprintf("%2.2X:", dev->dev_addr[i]);
  kprintf("%2.2X, IRQ %d.\n", dev->dev_addr[i], irq);

  outpd(ioaddr + SCBPort, PortReset);

  // Return the chip to its original power state.
  acpi_set_pwr_state(pdev, acpi_idle_state);

  // We do a request_region() only to register /proc/ioports info.
  request_region(ioaddr, pci_id_tbl[chip_idx].io_size, dev->name);

  dev->base_addr = ioaddr;
  dev->irq = irq;

  sp = dev->priv;
  if (dev->priv == NULL) 
  {
    void *mem = kmalloc(sizeof(*sp));
    dev->priv = sp = mem;   // Cache align here if kmalloc does not.
    sp->priv_addr = mem;
  }
  memset(sp, 0, sizeof(*sp));
  sp->next_module = root_speedo_dev;
  root_speedo_dev = dev;

  sp->pci_dev = pdev;
  sp->chip_id = chip_idx;
  sp->drv_flags = pci_id_tbl[chip_idx].drv_flags;
  sp->acpi_pwr = acpi_idle_state;

  sp->full_duplex = option >= 0 && (option & 0x220) ? 1 : 0;
  if (card_idx >= 0) 
  {
    if (full_duplex[card_idx] >= 0) sp->full_duplex = full_duplex[card_idx];
  }
  sp->default_port = option >= 0 ? (option & 0x0f) : 0;
  if (sp->full_duplex) sp->medialock = 1;

  sp->phy[0] = eeprom[6];
  sp->phy[1] = eeprom[7];
  sp->rx_bug = (eeprom[3] & 0x03) == 3 ? 0 : 1;

  if (sp->rx_bug) kprintf("  Receiver lock-up workaround activated.\n");

  // The Speedo-specific entries in the device structure.
  dev->open = &speedo_open;
  dev->hard_start_xmit = &speedo_start_xmit;
  dev->stop = &speedo_close;
  dev->get_stats = &speedo_get_stats;
  dev->set_multicast_list = &set_rx_mode;
  dev->do_ioctl = &speedo_ioctl;

  return dev;
}

// Perform a SCB command known to be slow.
// This function checks the status both before and after command execution.

static void do_slow_command(struct net_device *dev, int cmd)
{
  long cmd_ioaddr = dev->base_addr + SCBCmd;
  int wait = 0;

  do
  {
    if (inp(cmd_ioaddr) == 0) break;
  } while(++wait <= 200);

  if (wait > 100)  kprintf("eepro100: Command %4.4x was never accepted (%d polls)!\n", inp(cmd_ioaddr), wait);
  outp(cmd_ioaddrcmd);

  for (wait = 0; wait <= 100; wait++)
  {
    if (inp(cmd_ioaddr) == 0) return;
  }

  for (; wait <= 20000; wait++)
  {
    if (inb(cmd_ioaddr) == 0) 
      return;
    else 
      udelay(1);
  }

  kprintf("eepro100: Command %4.4x was not accepted after %d polls!  Current status %8.8x\n", cmd, wait, inpd(dev->base_addr + SCBStatus));
}

static int speedo_open(struct net_device *dev)
{
  struct speedo_private *sp = (struct speedo_private *) dev->priv;
  long ioaddr = dev->base_addr;

  acpi_set_pwr_state(sp->pci_dev, ACPI_D0);

  kprintf("%s: speedo_open() irq %d.\n", dev->name, dev->irq);

  // Set up the Tx queue early
  sp->cur_tx = 0;
  sp->dirty_tx = 0;
  sp->last_cmd = 0;
  sp->tx_full = 0;
  sp->lock = (spinlock_t) SPIN_LOCK_UNLOCKED;
  sp->polling = sp->in_interrupt = 0;

  dev->if_port = sp->default_port;

  if ((sp->phy[0] & 0x8000) == 0)
  {
    sp->advertising = mdio_read(ioaddr, sp->phy[0] & 0x1f, 4);
  }

  // With some transceivers we must retrigger negotiation to reset
  // power-up errors
  if ((sp->drv_flags & ResetMII) && (sp->phy[0] & 0x8000) == 0) 
  {
    int phy_addr = sp->phy[0] & 0x1f ;
    
    // Use 0x3300 for restarting NWay, other values to force xcvr:
    //   0x0000 10-HD
    //   0x0100 10-FD
    //   0x2000 100-HD
    //   0x2100 100-FD
   
    mdio_write(ioaddr, phy_addr, 0, 0x3300);
  }

  // We can safely take handler calls during init.
  // Doing this after speedo_init_rx_ring() results in a memory leak.
  if (request_irq(dev->irq, &speedo_interrupt, SA_SHIRQ, dev->name, dev)) return -EAGAIN;

  speedo_init_rx_ring(dev);

  // Fire up the hardware
  speedo_resume(dev);
  netif_start_tx_queue(dev);

  // Setup the chip and configure the multicast list
  sp->mc_setup_frm = NULL;
  sp->mc_setup_frm_len = 0;
  sp->mc_setup_busy = 0;
  sp->rx_mode = -1;     // Invalid -> always reset the mode.
  sp->flow_ctrl = sp->partner = 0;
  set_rx_mode(dev);

  kprintf("%s: Done speedo_open(), status %8.8x\n", dev->name, inpw(ioaddr + SCBStatus));

  // Set the timer.  The timer serves a dual purpose:
  // 1) to monitor the media interface (e.g. link beat) and perhaps switch
  //    to an alternate media type
  // 2) to monitor Rx activity, and restart the Rx process if the receiver
  //    hangs.

  init_timer(&sp->timer);
  sp->timer.expires = ticks + 3*HZ;
  sp->timer.data = (unsigned long) dev;
  sp->timer.function = &speedo_timer;
  add_timer(&sp->timer);

  // No need to wait for the command unit to accept here.
  if ((sp->phy[0] & 0x8000) == 0) mdio_read(ioaddr, sp->phy[0] & 0x1f, 0);

  return 0;
}

// Start the chip hardware after a full reset

static void speedo_resume(struct net_device *dev)
{
  struct speedo_private *sp = (struct speedo_private *) dev->priv;
  long ioaddr = dev->base_addr;

  outpw(ioaddr + SCBCmd, SCBMaskAll);

  // Start with a Tx threshold of 256 (0x..20.... 8 byte units)
  sp->tx_threshold = 0x01208000;

  // Set the segment registers to '0'
  wait_for_cmd_done(ioaddr + SCBCmd);
  if (inb(ioaddr + SCBCmd)) 
  {
    outpd(ioaddr + SCBPort, PortPartialReset);
    udelay(10);
  }
  outpd(ioaddr + SCBPointer, 0);
  inpd(ioaddr + SCBPointer);       // Flush to PCI
  udelay(10); // Bogus, but it avoids the bug

  // Note: these next two operations can take a while
  do_slow_command(dev, RxAddrLoad);
  do_slow_command(dev, CUCmdBase);

  // Load the statistics block and rx ring addresses
  outpd(ioaddr + SCBPointer, virt_to_bus(&sp->lstats));
  inpd(ioaddr + SCBPointer);       // Flush to PCI
  outp(ioaddr + SCBCmd, CUStatsAddr);
  sp->lstats.done_marker = 0;
  wait_for_cmd_done(ioaddr + SCBCmd);

  outpd(ioaddr + SCBPointer, virt_to_bus(sp->rx_ringp[sp->cur_rx % RX_RING_SIZE]));
  inpd(ioaddr + SCBPointer);       // Flush to PCI

  // Note: RxStart should complete instantly.
  do_slow_command(dev, RxStart);
  do_slow_command(dev, CUDumpStats);

  // Fill the first command with our physical address
  {
    int entry = sp->cur_tx++ % TX_RING_SIZE;
    struct descriptor *cur_cmd = (struct descriptor *) &sp->tx_ring[entry];

    // Avoid a bug(?!) here by marking the command already completed
    cur_cmd->cmd_status = cpu_to_le32((CmdSuspend | CmdIASetup) | 0xa000);
    cur_cmd->link = virt_to_le32desc(&sp->tx_ring[sp->cur_tx % TX_RING_SIZE]);
    memcpy(cur_cmd->params, dev->dev_addr, 6);
    if (sp->last_cmd) clear_suspend(sp->last_cmd);
    sp->last_cmd = cur_cmd;
  }

  // Start the chip's Tx process and unmask interrupts
  outpd(ioaddr + SCBPointer, virt_to_bus(&sp->tx_ring[sp->dirty_tx % TX_RING_SIZE]));
  outpw(ioaddr + SCBCmd, CUStart);
}

// Media monitoring and control

static void speedo_timer(unsigned long data)
{
  struct net_device *dev = (struct net_device *) data;
  struct speedo_private *sp = (struct speedo_private *) dev->priv;
  long ioaddr = dev->base_addr;
  int phy_num = sp->phy[0] & 0x1f;
  int status = inpw(ioaddr + SCBStatus);

  kprintf("%s: Interface monitor tick, chip status %4.4x\n", dev->name, status);

  // Normally we check every two seconds.
  sp->timer.expires = ticks + 2*HZ;

  if (sp->polling) 
  {
    // Continue to be annoying.
    if (status & 0xfc00) 
    {
      speedo_interrupt(dev->irq, dev, 0);
      if (ticks - sp->last_reset > 10*HZ) 
      {
        kprintf("%s: IRQ %d is still blocked!\n", dev->name, dev->irq);
        sp->last_reset = ticks;
      }
    } else if (ticks - sp->last_reset > 10*HZ)
      sp->polling = 0;
    
    sp->timer.expires = ticks + 2;
  }
  // We have MII and lost link beat
  if ((sp->phy[0] & 0x8000) == 0) 
  {
    int partner = mdio_read(ioaddr, phy_num, 5);
    if (partner != sp->partner) 
    {
      int flow_ctrl = sp->advertising & partner & 0x0400 ? 1 : 0;
      sp->partner = partner;
      if (flow_ctrl != sp->flow_ctrl) 
      {
        sp->flow_ctrl = flow_ctrl;
        sp->rx_mode = -1; // Trigger a reload
      }

      // Clear sticky bit
      mdio_read(ioaddr, phy_num, 1);
      
      // If link beat has returned...
      if (mdio_read(ioaddr, phy_num, 1) & 0x0004)
        netif_link_up(dev);
      else
        netif_link_down(dev);
    }
  }

  // This no longer has a false-trigger window
  if (sp->cur_tx - sp->dirty_tx > 1 && (ticks - dev->trans_start) > TX_TIMEOUT  && (ticks - sp->last_cmd_time) > TX_TIMEOUT) 
  {
    if (status == 0xffff) 
    {
      if (ticks - sp->last_reset > 10*HZ) 
      {
        sp->last_reset = ticks;
        kprintf("%s: The EEPro100 chip is missing!\n", dev->name);
      }
    } 
    else if (status & 0xfc00) 
    {
      // We have a blocked IRQ line.  This should never happen, but we recover as best we can
      if (!sp->polling)
      {
        if (ticks - sp->last_reset > 10*HZ) 
	{
          kprintf("%s: IRQ %d is physically blocked! (%4.4x) Failing back to low-rate polling\n", dev->name, dev->irq, status);
          sp->last_reset = ticks;
        }

        sp->polling = 1;
      }
      speedo_interrupt(dev->irq, dev, 0);
      sp->timer.expires = ticks + 2;  // Avoid 
    } 
    else 
    {
      speedo_tx_timeout(dev);
      sp->last_reset = ticks;
    }
  }

  if (sp->rx_mode < 0 || (sp->rx_bug && ticks - sp->last_rx_time > 2*HZ)) 
  {
    // We haven't received a packet in a Long Time.  We might have been
    // bitten by the receiver hang bug.  This can be cleared by sending
    // a set multicast list command.
    set_rx_mode(dev);
  }

  add_timer(&sp->timer);
}

static void speedo_show_state(struct net_device *dev)
{
  struct speedo_private *sp = (struct speedo_private *) dev->priv;
  long ioaddr = dev->base_addr;
  int phy_num = sp->phy[0] & 0x1f;
  int i;

  // Print a few items for debugging
  int i;
  kprintf("%s: Tx ring dump,  Tx queue %d / %d:\n", dev->name, sp->cur_tx, sp->dirty_tx);
  for (i = 0; i < TX_RING_SIZE; i++)
  {
    kprintf("%s: %c%c%d %8.8x\n", dev->name,
          i == sp->dirty_tx % TX_RING_SIZE ? '*' : ' ',
          i == sp->cur_tx % TX_RING_SIZE ? '=' : ' ',
          i, sp->tx_ring[i].status);
  }

  kprintf("%s: Rx ring dump (next to receive into %d)\n", dev->name, sp->cur_rx);

  for (i = 0; i < RX_RING_SIZE; i++)
  {
    printk("  Rx ring entry %d  %8.8x\n",  i, sp->rx_ringp[i]->status);
  }

  for (i = 0; i < 16; i++) 
  {
    if (i == 6) i = 21;
    kprintf("  PHY index %d register %d is %4.4x.\n", phy_num, i, mdio_read(ioaddr, phy_num, i));
  }
}

// Initialize the Rx and Tx rings

static void speedo_init_rx_ring(struct net_device *dev)
{
  struct speedo_private *sp = (struct speedo_private *) dev->priv;
  struct RxFD *rxf, *last_rxf = NULL;
  int i;

  sp->cur_rx = 0;

  for (i = 0; i < RX_RING_SIZE; i++) 
  {
    struct sk_buff *skb;
    skb = dev_alloc_skb(PKT_BUF_SZ + sizeof(struct RxFD));
    sp->rx_skbuff[i] = skb;
    if (skb == NULL) break;      // OK. Just initially short of Rx bufs
    skb->dev = dev;     // Mark as being used by this device
    rxf = (struct RxFD *)skb->tail;
    sp->rx_ringp[i] = rxf;
    skb_reserve(skb, sizeof(struct RxFD));
    if (last_rxf) last_rxf->link = virt_to_le32desc(rxf);
    last_rxf = rxf;
    rxf->status = cpu_to_le32(0x00000001);  // '1' is flag value only
    rxf->link = 0;            // None yet
    rxf->rx_buf_addr = 0xffffffff;
    rxf->count = cpu_to_le32(PKT_BUF_SZ << 16);
  }
  sp->dirty_rx = (unsigned int)(i - RX_RING_SIZE);

  // Mark the last entry as end-of-list.
  last_rxf->status = cpu_to_le32(0xC0000002); // '2' is flag value only
  sp->last_rxf = last_rxf;
}

static void speedo_tx_timeout(struct net_device *dev)
{
  struct speedo_private *sp = (struct speedo_private *) dev->priv;
  long ioaddr = dev->base_addr;
  int status = inw(ioaddr + SCBStatus);

  kprintf("%s: Transmit timed out: status %4.4x  %4.4x at %d/%d commands %8.8x %8.8x %8.8x\n",
       dev->name, status, inpw(ioaddr + SCBCmd),
       sp->dirty_tx, sp->cur_tx,
       sp->tx_ring[(sp->dirty_tx+0) % TX_RING_SIZE].status,
       sp->tx_ring[(sp->dirty_tx+1) % TX_RING_SIZE].status,
       sp->tx_ring[(sp->dirty_tx+2) % TX_RING_SIZE].status);

  // Trigger a stats dump to give time before the reset
  speedo_get_stats(dev);

  speedo_show_state(dev);

  printk("%s: Restarting the chip...\n", dev->name);

  // Reset the Tx and Rx units.
  outpd(ioaddr + SCBPort, PortReset);
  speedo_show_state(dev);
  udelay(10);
  speedo_resume(dev);

  // Reset the MII transceiver, suggested by Fred Young @ scalable.com
  if ((sp->phy[0] & 0x8000) == 0) 
  {
    int phy_addr = sp->phy[0] & 0x1f;
    int advertising = mdio_read(ioaddr, phy_addr, 4);
    int mii_bmcr = mdio_read(ioaddr, phy_addr, 0);
    mdio_write(ioaddr, phy_addr, 0, 0x0400);
    mdio_write(ioaddr, phy_addr, 1, 0x0000);
    mdio_write(ioaddr, phy_addr, 4, 0x0000);
    mdio_write(ioaddr, phy_addr, 0, 0x8000);
    mdio_read(ioaddr, phy_addr, 0);
    mdio_write(ioaddr, phy_addr, 0, mii_bmcr);
    mdio_write(ioaddr, phy_addr, 4, advertising);
  }

  sp->stats.tx_errors++;
  dev->trans_start = ticks;
}

// Handle the interrupt cases when something unexpected happens

static void speedo_intr_error(struct net_device *dev, int intr_status)
{
  long ioaddr = dev->base_addr;
  struct speedo_private *sp = (struct speedo_private *) dev->priv;

  if (intr_status & IntrRxSuspend) 
  {
    if ((intr_status & 0x003c) == 0x0028)
      // No more Rx buffers
      outp(ioaddr + SCBCmd, RxResumeNoResources);
    else if ((intr_status & 0x003c) == 0x0008) 
    { 
      // No resources (why?!)
      kprintf("%s: Unknown receiver error, status=%#4.4x\n", dev->name, intr_status);

      // No idea of what went wrong.  Restart the receiver
      outpd(ioaddr + SCBPointer, virt_to_bus(sp->rx_ringp[sp->cur_rx % RX_RING_SIZE]));
      outp(ioaddr + SCBCmd, RxStart);
    }

    sp->stats.rx_errors++;
  }
}

static int speedo_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
  struct speedo_private *sp = (struct speedo_private *) dev->priv;
  long ioaddr = dev->base_addr;
  int entry;

  // Block a timer-based transmit from overlapping.  This could better be
  // done with atomic_swap(1, dev->tbusy), but set_bit() works as well.
  // If this ever occurs the queue layer is doing something evil!

  if (netif_pause_tx_queue(dev) != 0) 
  {
    int tickssofar = ticks - dev->trans_start;
    if (tickssofar < TX_TIMEOUT - 2) return 1;
    if (tickssofar < TX_TIMEOUT) 
    {
      // Reap sent packets from the full Tx queue.
      outpw(ioaddr + SCBCmd, SCBTriggerIntr);
      return 1;
    }
    speedo_tx_timeout(dev);
    return 1;
  }

  // Caution: the write order is important here, set the base address
  // with the "ownership" bits last.

  // Calculate the Tx descriptor entry
  entry = sp->cur_tx % TX_RING_SIZE;

  sp->tx_skbuff[entry] = skb;
  // TODO: be a little more clever about setting the interrupt bit
  sp->tx_ring[entry].status = cpu_to_le32(CmdSuspend | CmdTx | CmdTxFlex);
  sp->cur_tx++;
  sp->tx_ring[entry].link = virt_to_le32desc(&sp->tx_ring[sp->cur_tx % TX_RING_SIZE]);
  sp->tx_ring[entry].tx_desc_addr = virt_to_le32desc(&sp->tx_ring[entry].tx_buf_addr0);
  // The data region is always in one buffer descriptor
  sp->tx_ring[entry].count = cpu_to_le32(sp->tx_threshold);
  sp->tx_ring[entry].tx_buf_addr0 = virt_to_le32desc(skb->data);
  sp->tx_ring[entry].tx_buf_size0 = cpu_to_le32(skb->len);
  
  // TODO: perhaps leave the interrupt bit set if the Tx queue is more
  // than half full.  Argument against: we should be receiving packets
  // and scavenging the queue.  Argument for: if so, it shouldn't
  // matter.

  {
    struct descriptor *last_cmd = sp->last_cmd;
    sp->last_cmd = (struct descriptor *)&sp->tx_ring[entry];
    clear_suspend(last_cmd);
  }
  
  if (sp->cur_tx - sp->dirty_tx >= TX_QUEUE_LIMIT) 
  {
    sp->tx_full = 1;
    netif_stop_tx_queue(dev);
  } 
  else
    netif_unpause_tx_queue(dev);
  }

  wait_for_cmd_done(ioaddr + SCBCmd);
  outp(ioaddr + SCBCmd, CUResume);
  dev->trans_start = ticks;

  return 0;
}

// The interrupt handler does all of the Rx thread work and cleans up
// after the Tx thread.

static void speedo_interrupt(int irq, void *dev_instance, struct pt_regs *regs)
{
  struct net_device *dev = (struct net_device *) dev_instance;
  struct speedo_private *sp;
  long ioaddr, boguscnt = max_interrupt_work;
  unsigned short status;

  ioaddr = dev->base_addr;
  sp = (struct speedo_private *) dev->priv;

  while (1)
  {
    status = inpw(ioaddr + SCBStatus);

    if ((status & IntrAllNormal) == 0  ||  status == 0xffff) break;

    // Acknowledge all of the current interrupt sources ASAP
    outw(ioaddr + SCBStatus, status & IntrAllNormal);

    kprintf("%s: interrupt  status=%#4.4x\n", dev->name, status);

    if (status & (IntrRxDone|IntrRxSuspend))
    {
      speedo_rx(dev);
    }

    // The command unit did something, scavenge finished Tx entries
    if (status & (IntrCmdDone | IntrCmdIdle | IntrDrvrIntr)) 
    {
      unsigned int dirty_tx;

      dirty_tx = sp->dirty_tx;
      while (sp->cur_tx - dirty_tx > 0) 
      {
        int entry = dirty_tx % TX_RING_SIZE;
        int status = le32_to_cpu(sp->tx_ring[entry].status);

	kprintf("%s: scavenge candidate %d status %4.4x\n", dev->name, entry, status);

	if ((status & StatusComplete) == 0) 
	{
          // Special case error check: look for descriptor that the chip skipped(?)
          if (sp->cur_tx - dirty_tx > 2  && (sp->tx_ring[(dirty_tx+1) % TX_RING_SIZE].status & cpu_to_le32(StatusComplete))) 
	  {
            kprintf("%s: Command unit failed to mark command %8.8x as complete at %d\n", dev->name, status, dirty_tx);
          } 
	  else
	    // It still hasn't been processed
            break;      
        }

        if ((status & TxUnderrun) && (sp->tx_threshold < 0x01e08000)) 
	{
          sp->tx_threshold += 0x00040000;
          kprintf("%s: Tx threshold increased, %#8.8x\n", dev->name, sp->tx_threshold);
        }

        // Free the original skb
        if (sp->tx_skbuff[entry]) 
	{
	  // Count only user packets
          sp->stats.tx_packets++; 
          sp->stats.tx_bytes += sp->tx_skbuff[entry]->len;
          dev_free_skb_irq(sp->tx_skbuff[entry]);
          sp->tx_skbuff[entry] = 0;
        } 
	else if ((status & 0x70000) == CmdNOp)
          sp->mc_setup_busy = 0;
        
	dirty_tx++;
      }

      sp->dirty_tx = dirty_tx;
      if (sp->tx_full &&  sp->cur_tx - dirty_tx < TX_QUEUE_UNFULL) 
      {
        // The ring is no longer full, clear tbusy
        sp->tx_full = 0;
        netif_resume_tx_queue(dev);
      }
    }

    if (status & IntrRxSuspend)
    {
      speedo_intr_error(dev, status);
    }

    if (--boguscnt < 0) 
    {
      kprintf("%s: Too much work at interrupt, status=0x%4.4x\n", dev->name, status);

      // Clear all interrupt sources.
      outpd(ioaddr + SCBStatus, 0xfc00);
      break;
    }
  }

  kprintf("%s: exiting interrupt, status=%#4.4x\n", dev->name, inpw(ioaddr + SCBStatus));

  clear_bit(0, (void*)&sp->in_interrupt);

  return;
}

static int speedo_rx(struct net_device *dev)
{
  struct speedo_private *sp = (struct speedo_private *) dev->priv;
  int entry = sp->cur_rx % RX_RING_SIZE;
  int status;
  int rx_work_limit = sp->dirty_rx + RX_RING_SIZE - sp->cur_rx;

  kprintf("%s: In speedo_rx()\n", dev->name);

  // If we own the next entry, it's a new packet. Send it up.
  while (sp->rx_ringp[entry] != NULL &&  (status = le32_to_cpu(sp->rx_ringp[entry]->status)) & RxComplete) 
  {
    int desc_count = le32_to_cpu(sp->rx_ringp[entry]->count);
    int pkt_len = desc_count & 0x07ff;

    if (--rx_work_limit < 0) break;

    kprintf("%s: speedo_rx() status %8.8x len %d\n", dev->name, status, pkt_len);

    if ((status & (RxErrTooBig | RxOK | 0x0f90)) != RxOK)
    {
      if (status & RxErrTooBig)
        kprintf("%s: Ethernet frame overran the Rx buffer, status %8.8x!\n", dev->name, status);
      else if (!(status & RxOK)) 
      {
        // There was a fatal error.  This *should* be impossible.
        sp->stats.rx_errors++;
        kprintf("%s: Anomalous event in speedo_rx(), status %8.8x\n", dev->name, status);
      }
    } 
    else 
    {
      struct sk_buff *skb;

      if (sp->drv_flags & HasChksum) pkt_len -= 2;

      // Check if the packet is long enough to just accept without
      // copying to a properly sized skbuff

      if (pkt_len < rx_copybreak && (skb = dev_alloc_skb(pkt_len + 2)) != 0) 
      {
        skb->dev = dev;
        skb_reserve(skb, 2);  // Align IP on 16 byte boundaries
        // 'skb_put()' points to the start of sk_buff data area.
        // Packet is in one chunk -- we can copy + cksum.
        eth_copy_and_sum(skb, sp->rx_skbuff[entry]->tail, pkt_len, 0);
        skb_put(skb, pkt_len);
      } 
      else 
      {
        void *temp;
        // Pass up the already-filled skbuff.
        skb = sp->rx_skbuff[entry];
        if (skb == NULL) 
	{
          kprintf("%s: Inconsistent Rx descriptor chain\n", dev->name);
          break;
        }

        sp->rx_skbuff[entry] = NULL;
        temp = skb_put(skb, pkt_len);

        sp->rx_ringp[entry] = NULL;
      }
      skb->protocol = eth_type_trans(skb, dev);

      netif_rx(skb);
      sp->stats.rx_packets++;
      sp->stats.rx_bytes += pkt_len;
    }
    entry = (++sp->cur_rx) % RX_RING_SIZE;
  }

  // Refill the Rx ring buffers.
  for (; sp->cur_rx - sp->dirty_rx > 0; sp->dirty_rx++) 
  {
    struct RxFD *rxf;
    entry = sp->dirty_rx % RX_RING_SIZE;
    
    if (sp->rx_skbuff[entry] == NULL) 
    {
      struct sk_buff *skb;

      // Get a fresh skbuff to replace the consumed one.
      skb = dev_alloc_skb(PKT_BUF_SZ + sizeof(struct RxFD));
      sp->rx_skbuff[entry] = skb;
      if (skb == NULL) 
      {
        sp->rx_ringp[entry] = NULL;
        sp->alloc_failures++;
        break;      // Better luck next time! 
      }
      rxf = sp->rx_ringp[entry] = (struct RxFD *) skb->tail;
      skb->dev = dev;
      skb_reserve(skb, sizeof(struct RxFD));
      rxf->rx_buf_addr = virt_to_le32desc(skb->tail);
    } 
    else 
    {
      rxf = sp->rx_ringp[entry];
    }

    rxf->status = cpu_to_le32(0xC0000001);  // '1' for driver use only
    rxf->link = 0;      // None yet
    rxf->count = cpu_to_le32(PKT_BUF_SZ << 16);
    sp->last_rxf->link = virt_to_le32desc(rxf);
    sp->last_rxf->status &= cpu_to_le32(~0xC0000000);
    sp->last_rxf = rxf;
  }

  sp->last_rx_time = ticks;
  return 0;
}

static int speedo_close(struct net_device *dev)
{
  long ioaddr = dev->base_addr;
  struct speedo_private *sp = (struct speedo_private *) dev->priv;
  int i;

  netif_stop_tx_queue(dev);

  kprintf("%s: Shutting down ethercard, status was %4.4x. Cumlative allocation failures: %d.\n",
    dev->name, inpw(ioaddr + SCBStatus), sp->alloc_failures);

  // Shut off the media monitoring timer.
  del_timer(&sp->timer);

  // Shutting down the chip nicely fails to disable flow control. So..
  outpd(ioaddr + SCBPort, PortPartialReset);

  free_irq(dev->irq, dev);

  // Free all the skbuffs in the Rx and Tx queues.
  for (i = 0; i < RX_RING_SIZE; i++) 
  {
    struct sk_buff *skb = sp->rx_skbuff[i];
    sp->rx_skbuff[i] = 0;
    if (skb) dev_free_skb(skb);
  }

  for (i = 0; i < TX_RING_SIZE; i++) 
  {
    struct sk_buff *skb = sp->tx_skbuff[i];
    sp->tx_skbuff[i] = 0;
    if (skb) dev_free_skb(skb);
  }

  if (sp->mc_setup_frm) 
  {
    kfree(sp->mc_setup_frm);
    sp->mc_setup_frm_len = 0;
  }

  // Print a few items for debugging.
  speedo_show_state(dev);

  // Alt: acpi_set_pwr_state(pdev, sp->acpi_pwr);
  acpi_set_pwr_state(sp->pci_dev, ACPI_D2);

  return 0;
}

//
// The Speedo-3 has an especially awkward and unusable method of getting
// statistics out of the chip.  It takes an unpredictable length of time
// for the dump-stats command to complete.  To avoid a busy-wait loop we
// update the stats with the previous dump results, and then trigger a
// new dump.
//
// These problems are mitigated by the current /proc implementation, which
// calls this routine first to judge the output length, and then to emit the
// output.
//
// Oh, and incoming frames are dropped while executing dump-stats!
//

static struct net_device_stats *speedo_get_stats(struct net_device *dev)
{
  struct speedo_private *sp = (struct speedo_private *) dev->priv;
  long ioaddr = dev->base_addr;

  // Update only if the previous dump finished.
  if (sp->lstats.done_marker == le32_to_cpu(0xA007)) 
  {
    sp->stats.tx_aborted_errors += le32_to_cpu(sp->lstats.tx_coll16_errs);
    sp->stats.tx_window_errors += le32_to_cpu(sp->lstats.tx_late_colls);
    sp->stats.tx_fifo_errors += le32_to_cpu(sp->lstats.tx_underruns);
    sp->stats.tx_fifo_errors += le32_to_cpu(sp->lstats.tx_lost_carrier);
    //sp->stats.tx_deferred += le32_to_cpu(sp->lstats.tx_deferred);
    sp->stats.collisions += le32_to_cpu(sp->lstats.tx_total_colls);
    sp->stats.rx_crc_errors += le32_to_cpu(sp->lstats.rx_crc_errs);
    sp->stats.rx_frame_errors += le32_to_cpu(sp->lstats.rx_align_errs);
    sp->stats.rx_over_errors += le32_to_cpu(sp->lstats.rx_resource_errs);
    sp->stats.rx_fifo_errors += le32_to_cpu(sp->lstats.rx_overrun_errs);
    sp->stats.rx_length_errors += le32_to_cpu(sp->lstats.rx_runt_errs);
    sp->lstats.done_marker = 0x0000;
    if (netif_running(dev)) {
      wait_for_cmd_done(ioaddr + SCBCmd);
      outp(ioaddr + SCBCmd, CUDumpStats);
    }
  }
  return &sp->stats;
}

// Set or clear the multicast filter for this adaptor.
// This is very ugly with Intel chips -- we usually have to execute an
// entire configuration command, plus process a multicast command.
// This is complicated.  We must put a large configuration command and
// an arbitrarily-sized multicast command in the transmit list.
// To minimize the disruption -- the previous command might have already
// loaded the link -- we convert the current command block, normally a Tx
// command, into a no-op and link it to the new command.

static void set_rx_mode(struct net_device *dev)
{
  struct speedo_private *sp = (struct speedo_private *) dev->priv;
  long ioaddr = dev->base_addr;
  struct descriptor *last_cmd;
  char new_rx_mode;
  unsigned long flags;
  int entry, i;

  if (dev->flags & IFF_PROMISC) 
  {     
    // Set promiscuous.
    new_rx_mode = 3;
  }
  else if ((dev->flags & IFF_ALLMULTI) || dev->mc_count > multicast_filter_limit) 
  {
    new_rx_mode = 1;
  } 
  else
    new_rx_mode = 0;

  if (sp->cur_tx - sp->dirty_tx >= TX_RING_SIZE - 1) 
  {
    // The Tx ring is full -- don't add anything!  Presumably the new mode
    // is in config_cmd_data and will be added anyway, otherwise we wait
    // for a timer tick or the mode to change again.
    sp->rx_mode = -1;
    return;
  }

  if (new_rx_mode != sp->rx_mode) 
  {
    unsigned char *config_cmd_data;

    spin_lock_irqsave(&sp->lock, flags);
    entry = sp->cur_tx % TX_RING_SIZE;
    last_cmd = sp->last_cmd;
    sp->last_cmd = (struct descriptor *)&sp->tx_ring[entry];

    sp->tx_skbuff[entry] = 0;     // Redundant
    sp->tx_ring[entry].status = cpu_to_le32(CmdSuspend | CmdConfigure);
    sp->cur_tx++;
    sp->tx_ring[entry].link = virt_to_le32desc(&sp->tx_ring[(entry + 1) % TX_RING_SIZE]);
    // We may nominally release the lock here

    config_cmd_data = (void *)&sp->tx_ring[entry].tx_desc_addr;
    // Construct a full CmdConfig frame
    memcpy(config_cmd_data, i82558_config_cmd, sizeof(i82558_config_cmd));
    config_cmd_data[1] = (txfifo << 4) | rxfifo;
    config_cmd_data[4] = rxdmacount;
    config_cmd_data[5] = txdmacount + 0x80;
    if (sp->drv_flags & HasChksum) config_cmd_data[9] |= 1;
    config_cmd_data[15] |= (new_rx_mode & 2) ? 1 : 0;
    config_cmd_data[19] = sp->flow_ctrl ? 0xBD : 0x80;
    config_cmd_data[19] |= sp->full_duplex ? 0x40 : 0;
    config_cmd_data[21] = (new_rx_mode & 1) ? 0x0D : 0x05;
    if (sp->phy[0] & 0x8000) 
    {      
      // Use the AUI port instead.
      config_cmd_data[15] |= 0x80;
      config_cmd_data[8] = 0;
    }
    // Trigger the command unit resume.
    wait_for_cmd_done(ioaddr + SCBCmd);
    clear_suspend(last_cmd);
    outp(ioaddr + SCBCmd, CUResume);
    spin_unlock_irqrestore(&sp->lock, flags);
    sp->last_cmd_time = ticks;
  }

  if (new_rx_mode == 0 && dev->mc_count < 4) 
  {
    // The simple case of 0-3 multicast list entries occurs often, and
    // fits within one tx_ring[] entry.
    struct dev_mc_list *mclist;
    unsigned short *setup_params, *eaddrs;

    spin_lock_irqsave(&sp->lock, flags);
    entry = sp->cur_tx % TX_RING_SIZE;
    last_cmd = sp->last_cmd;
    sp->last_cmd = (struct descriptor *)&sp->tx_ring[entry];

    sp->tx_skbuff[entry] = 0;
    sp->tx_ring[entry].status = cpu_to_le32(CmdSuspend | CmdMulticastList);
    sp->cur_tx++;
    sp->tx_ring[entry].link = virt_to_le32desc(&sp->tx_ring[(entry + 1) % TX_RING_SIZE]);
    // We may nominally release the lock here
    sp->tx_ring[entry].tx_desc_addr = 0; // Really MC list count
    setup_params = (unsigned short *)&sp->tx_ring[entry].tx_desc_addr;
    *setup_params++ = cpu_to_le16(dev->mc_count*6);
    
    // Fill in the multicast addresses.
    for (i = 0, mclist = dev->mc_list; i < dev->mc_count; i++, mclist = mclist->next) 
    {
      eaddrs = (unsigned short *) mclist->dmi_addr;
      *setup_params++ = *eaddrs++;
      *setup_params++ = *eaddrs++;
      *setup_params++ = *eaddrs++;
    }

    wait_for_cmd_done(ioaddr + SCBCmd);
    clear_suspend(last_cmd);
    // Immediately trigger the command unit resume
    outp(ioaddr + SCBCmd, CUResume);
    spin_unlock_irqrestore(&sp->lock, flags);
    sp->last_cmd_time = ticks;
  } 
  else if (new_rx_mode == 0) 
  {
    struct dev_mc_list *mclist;
    u16 *setup_params, *eaddrs;
    struct descriptor *mc_setup_frm = sp->mc_setup_frm;
    int i;

    if (sp->mc_setup_frm_len < 10 + dev->mc_count * 6 || sp->mc_setup_frm == NULL) 
    {
      // Allocate a full setup frame, 10bytes + <max addrs>.
      if (sp->mc_setup_frm) kfree(sp->mc_setup_frm);
      sp->mc_setup_busy = 0;
      sp->mc_setup_frm_len = 10 + multicast_filter_limit*6;
      sp->mc_setup_frm = kmalloc(sp->mc_setup_frm_len);
      if (sp->mc_setup_frm == NULL) 
      {
        kprintf("%s: Failed to allocate a setup frame\n", dev->name);
        sp->rx_mode = -1; // We failed, try again.
        return;
      }
    }

    // If we are busy, someone might be quickly adding to the MC list.
    // Try again later when the list updates stop.
    if (sp->mc_setup_busy) 
    {
      sp->rx_mode = -1;
      return;
    }
    mc_setup_frm = sp->mc_setup_frm;

    // Fill the setup frame
    printk(KERN_DEBUG "%s: Constructing a setup frame at %p, %d bytes\n", dev->name, sp->mc_setup_frm, sp->mc_setup_frm_len);
    mc_setup_frm->cmd_status =  cpu_to_le32(CmdSuspend | CmdIntr | CmdMulticastList);
    
    // Link set below
    setup_params = (unsigned short *)&mc_setup_frm->params;
    *setup_params++ = cpu_to_le16(dev->mc_count * 6);
    
    // Fill in the multicast addresses
    for (i = 0, mclist = dev->mc_list; i < dev->mc_count; i++, mclist = mclist->next) 
    {
      eaddrs = (unsigned short *)mclist->dmi_addr;
      *setup_params++ = *eaddrs++;
      *setup_params++ = *eaddrs++;
      *setup_params++ = *eaddrs++;
    }

    // Disable interrupts while playing with the Tx Cmd list
    spin_lock_irqsave(&sp->lock, flags);
    entry = sp->cur_tx % TX_RING_SIZE;
    last_cmd = sp->last_cmd;
    sp->last_cmd = mc_setup_frm;
    sp->mc_setup_busy++;

    // Change the command to a NoOp, pointing to the CmdMulti command
    sp->tx_skbuff[entry] = 0;
    sp->tx_ring[entry].status = cpu_to_le32(CmdNOp);
    sp->cur_tx++;
    sp->tx_ring[entry].link = virt_to_le32desc(mc_setup_frm);
    // We may nominally release the lock here

    // Set the link in the setup frame.
    mc_setup_frm->link = virt_to_le32desc(&(sp->tx_ring[(entry+1) % TX_RING_SIZE]));

    wait_for_cmd_done(ioaddr + SCBCmd);
    clear_suspend(last_cmd);

    // Immediately trigger the command unit resume
    outp(ioaddr + SCBCmd, CUResume);
    spin_unlock_irqrestore(&sp->lock, flags);
    sp->last_cmd_time = ticks;
    kprintf("%s: CmdMCSetup frame length %d in entry %d\n", dev->name, dev->mc_count, entry);
  }

  sp->rx_mode = new_rx_mode;
}

#endif

static int speedo_transmit(struct dev *dev, struct pbuf *p)
{
  return -ENOSYS;
}

static int speedo_ioctl(struct dev *dev, int cmd, void *args, size_t size)
{
  return -ENOSYS;
}

static int speedo_attach(struct dev *dev, struct eth_addr *hwaddr)
{
  struct nic *np = dev->privdata;
  *hwaddr = np->hwaddr;

  return -ENOSYS;
}

static int speedo_detach(struct dev *dev)
{
  return 0;
}

struct driver speedo_driver =
{
  "eepro100",
  DEV_TYPE_PACKET,
  speedo_ioctl,
  NULL,
  NULL,
  speedo_attach,
  speedo_detach,
  speedo_transmit
};

int __declspec(dllexport) install(struct unit *unit, char *opts)
{
  return -ENOSYS;
}

int __stdcall start(hmodule_t hmod, int reason, void *reserved2)
{
  return 1;
}
