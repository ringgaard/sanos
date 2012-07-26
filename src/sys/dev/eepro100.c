//
// eepro100.c
//
// Intel EtherExpress Pro100 NIC network driver
//
// Written 1998-2002 by Donald Becker.
// Ported to sanos 2002-2004 by Michael Ringgaard.
//
// This software may be used and distributed according to the terms of
// the GNU General Public License (GPL), incorporated herein by reference.
// Drivers based on or derived from this code fall under the GPL and must
// retain the authorship, copyright and license notice.  This driver is not
// a complete program and may only be used when the entire operating
// system is licensed under the GPL.
//
// This driver is for the Intel EtherExpress Pro100 (Speedo3) design.
// It should work with all i82557/558/559 boards.
//
// The author may be reached as becker@scyld.com, or C/O
// Scyld Computing Corporation
// 410 Severn Ave., Suite 210
// Annapolis MD 21403
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

enum chip_capability_flags  { 
  ResetMII = 1, 
  HasChksum = 2
};

static struct board board_tbl[] =  {
  {"Intel", "Intel i82559 rev 8", BUSTYPE_PCI, PCI_UNITCODE(0x8086, 0x1229), 0xffffffff, 0, 0, 8, 0xff, HasChksum},
  {"Intel", "Intel PCI EtherExpress Pro100", BUSTYPE_PCI, PCI_UNITCODE(0x8086, 0x1229), 0xffffffff, 0, 0, 0, 0, 0},
  {"Intel", "Intel EtherExpress Pro/100+ i82559ER", BUSTYPE_PCI, PCI_UNITCODE(0x8086, 0x1209), 0xffffffff, 0, 0, 0, 0, ResetMII},
  {"Intel", "Intel EtherExpress Pro/100 type 1029", BUSTYPE_PCI, PCI_UNITCODE(0x8086, 0x1029), 0xffffffff, 0, 0, 0, 0, 0},
  {"Intel", "Intel EtherExpress Pro/100 type 1030", BUSTYPE_PCI, PCI_UNITCODE(0x8086, 0x1030), 0xffffffff, 0, 0, 0, 0, 0},
  {"Intel", "Intel Pro/100 V Network", BUSTYPE_PCI, PCI_UNITCODE(0x8086, 0x2449), 0xffffffff, 0, 0, 0, 0, 0},
  {"Intel", "Intel Pro/100 VE (type 1031)", BUSTYPE_PCI, PCI_UNITCODE(0x8086, 0x1031), 0xffffffff, 0, 0, 0, 0, 0},
  {"Intel", "Intel Pro/100 VM (type 1038)", BUSTYPE_PCI, PCI_UNITCODE(0x8086, 0x1038), 0xffffffff, 0, 0, 0, 0, 0},
  {"Intel", "Intel Pro/100 VM (type 1039)", BUSTYPE_PCI, PCI_UNITCODE(0x8086, 0x1039), 0xffffffff, 0, 0, 0, 0, 0},
  {"Intel", "Intel Pro/100 VM (type 103a)", BUSTYPE_PCI, PCI_UNITCODE(0x8086, 0x103a), 0xffffffff, 0, 0, 0, 0, 0},
  {"HP", "HP/Compaq D510 Intel Pro/100 VM", BUSTYPE_PCI, PCI_UNITCODE(0x8086, 0x103b), 0xffffffff, PCI_UNITCODE(0x0e11, 0x0012), 0, 0, 0, 0},
  {"Intel", "Intel Pro/100 VM (type 103b)", BUSTYPE_PCI, PCI_UNITCODE(0x8086, 0x103b), 0xffffffff, 0, 0, 0, 0, 0},
  {"Intel", "Intel Pro/100 VM (unknown type series 1030)", BUSTYPE_PCI, PCI_UNITCODE(0x8086, 0x1030), 0xfffffff0, 0, 0, 0, 0, 0},
  {NULL,},
};

// Offsets to the various registers
// All accesses need not be longword aligned

enum speedo_offsets {
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

enum commands {
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

enum SCBCmdBits {
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

enum intr_status_bits {
  IntrCmdDone   = 0x8000,  
  IntrRxDone    = 0x4000, 
  IntrCmdIdle   = 0x2000,
  IntrRxSuspend = 0x1000, 
  IntrMIIDone   = 0x0800, 
  IntrDrvrIntr  = 0x0400,
  IntrAllNormal = 0xfc00,
};

enum SCBPort_cmds {
  PortReset        = 0, 
  PortSelfTest     = 1, 
  PortPartialReset = 2, 
  PortDump         = 3,
};

// The Speedo3 Rx and Tx frame/buffer descriptors

// A generic descriptor

struct descriptor {
  long cmd_status;            // All command and status fields
  unsigned long link;         // struct descriptor *
  unsigned char params[0];
};

// The Speedo3 Rx and Tx buffer descriptors

// Receive frame descriptor

struct RxFD {
  long status;
  unsigned long link;         // struct RxFD *
  unsigned long rx_buf_addr;  // void *
  unsigned long count;
};

// Selected elements of the Tx/RxFD.status word

enum RxFD_bits {
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

struct TxFD {
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

// Elements of the dump_statistics block. This block must be dword aligned.

struct speedo_stats {
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

// Do not change the position (alignment) of the first few elements!
// The later elements are grouped for cache locality.

struct nic {
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

  dev_t devno;                             // Device number
  struct dev *dev;                         // Device block
  unsigned short iobase;                   // Configured I/O base
  unsigned short irq;                      // Configured IRQ
  struct interrupt intr;                   // Interrupt object for driver
  struct dpc dpc;                          // DPC for driver
  struct timer timer;                      // Media selection timer
  struct eth_addr hwaddr;                  // MAC address for NIC
  unsigned int trans_start;

  struct stats_nic stats;
  struct speedo_stats lstats;
  int alloc_failures;
  struct board *board;
  int flags;
  int mc_setup_frm_len;                    // The length of an allocated...
  struct descriptor *mc_setup_frm;         // ... multicast setup frame
  int mc_setup_busy;                       // Avoid double-use of setup frame
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

const unsigned char i82557_config_cmd[22] = {
  22, 0x08, 0, 0,  0, 0, 0x32, 0x03,  1, // 1=Use MII  0=Use AUI
  0, 0x2E, 0,  0x60, 0,
  0xf2, 0x48,   0, 0x40, 0xf2, 0x80,    // 0x40=Force full-duplex
  0x3f, 0x05, 
};

const unsigned char i82558_config_cmd[22] = {
  22, 0x08, 0, 1,  0, 0, 0x22, 0x03,  1, // 1=Use MII  0=Use AUI
  0, 0x2E, 0,  0x60, 0x08, 0x88,
  0x68, 0, 0x40, 0xf2, 0xBD,    // 0xBD->0xFD=Force full-duplex
  0x31, 0x05, 
};

// PHY media interface chips

static const char *phys[] = {
  "None", "i82553-A/B", "i82553-C", "i82503",
  "DP83840", "80c240", "80c24", "i82555",
  "unknown-8", "unknown-9", "DP83840A", "unknown-11",
  "unknown-12", "unknown-13", "unknown-14", "unknown-15"
};

enum phy_chips {
  NonSuchPhy=0, I82553AB, I82553C, I82503, DP83840, S80C240, S80C24, I82555, DP83840A=10
};

static const char is_mii[] = { 0, 1, 1, 0, 1, 1, 0, 1 };

#define clear_suspend(cmd)   ((char *)(&(cmd)->cmd_status))[3] &= ~0x40

#define EE_READ_CMD   (6)

// How to wait for the command unit to accept a command.
// Typically this takes 0 ticks

__inline static void wait_for_cmd_done(long cmd_ioaddr) {
  int wait = 0;
  int delayed_cmd;
  
  while (++wait <= 100) {
    if (inp(cmd_ioaddr) == 0) return;
  }

  delayed_cmd = inp(cmd_ioaddr);
  
  while (++wait <= 10000) {
    if (inp(cmd_ioaddr) == 0) break;
  }

  kprintf(KERN_WARNING "eepro100: Command %2.2x was not immediately accepted, %d ticks!\n", delayed_cmd, wait);
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

static int do_eeprom_cmd(long ioaddr, int cmd, int cmd_len) {
  unsigned retval = 0;
  long ee_addr = ioaddr + SCBeeprom;

  outpw(ee_addr, EE_ENB | EE_SHIFT_CLK);

  // Shift the command bits out
  do {
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

static int mdio_read(long ioaddr, int phy_id, int location) {
  int val, boguscnt = 64 * 10;    // <64 usec. to complete, typ 27 ticks
  outpd(ioaddr + SCBCtrlMDI, 0x08000000 | (location << 16) | (phy_id << 21));
  do {
    val = inpd(ioaddr + SCBCtrlMDI);
    if (--boguscnt < 0) {
      kprintf(KERN_WARNING "eepro100: mdio_read() timed out with val = %8.8x\n", val);
      break;
    }
  } while (!(val & 0x10000000));
  
  return val & 0xffff;
}

static int mdio_write(long ioaddr, int phy_id, int location, int value) {
  int val, boguscnt = 64 * 10;    // <64 usec. to complete, typ 27 ticks
  outpd(ioaddr + SCBCtrlMDI, 0x04000000 | (location << 16) | (phy_id << 21) | value);
  do {
    val = inpd(ioaddr + SCBCtrlMDI);
    if (--boguscnt < 0) {
      kprintf(KERN_WARNING "eepro100: mdio_write() timed out with val = %8.8x\n", val);
      break;
    }
  } while (!(val & 0x10000000));

  return val & 0xffff;
}

// Perform a SCB command known to be slow.
// This function checks the status both before and after command execution.

static void do_slow_command(struct dev *dev, int cmd) {
  struct nic *sp = (struct nic *) dev->privdata;
  long cmd_ioaddr = sp->iobase + SCBCmd;
  int wait = 0;

  while (++wait <= 200) {
    if (inp(cmd_ioaddr) == 0) break;
  }

  if (wait > 100)  kprintf(KERN_WARNING "%s: Command %4.4x was never accepted (%d polls)!\n", dev->name, inp(cmd_ioaddr), wait);
  outp(cmd_ioaddr, cmd);

  for (wait = 0; wait <= 100; wait++) {
    if (inp(cmd_ioaddr) == 0) return;
  }

  for (; wait <= 20000; wait++) {
    if (inp(cmd_ioaddr) == 0) {
      return;
    } else {
      udelay(1);
    }
  }

  kprintf(KERN_WARNING "%s: Command %4.4x was not accepted after %d polls!  Current status %8.8x\n", dev->name, cmd, wait, inpd(sp->iobase + SCBStatus));
}

static int speedo_set_rx_mode(struct dev *dev);
static void speedo_resume(struct dev *dev);
static void speedo_timer(void *arg);
static void speedo_interrupt(struct dev *dev);
static void speedo_tx_timeout(struct dev *dev);

static void speedo_show_state(struct dev *dev) {
  struct nic *sp = (struct nic *) dev->privdata;
  long ioaddr = sp->iobase;
  int phy_num = sp->phy[0] & 0x1f;
  unsigned int i;

  // Print a few items for debugging
  kprintf("%s: Tx ring dump,  Tx queue %d / %d:\n", dev->name, sp->cur_tx, sp->dirty_tx);
  for (i = 0; i < TX_RING_SIZE; i++) {
    kprintf("%s: %c%c%d %8.8x\n", dev->name,
          i == sp->dirty_tx % TX_RING_SIZE ? '*' : ' ',
          i == sp->cur_tx % TX_RING_SIZE ? '=' : ' ',
          i, sp->tx_ring[i].status);
  }

  kprintf("%s: Rx ring dump (next to receive into %d)\n", dev->name, sp->cur_rx);

  for (i = 0; i < RX_RING_SIZE; i++) {
    kprintf("  Rx ring entry %d  %8.8x\n",  i, sp->rx_ringp[i]->status);
  }

  for (i = 0; i < 16; i++) {
    if (i == 6) i = 21;
    kprintf("  PHY index %d register %d is %4.4x.\n", phy_num, i, mdio_read(ioaddr, phy_num, i));
  }
}

// Initialize the Rx and Tx rings

static void speedo_init_rx_ring(struct dev *dev)
{
  struct nic *sp = (struct nic *) dev->privdata;
  struct RxFD *rxf, *last_rxf = NULL;
  int i;

  init_sem(&sp->tx_sem, TX_QUEUE_LIMIT);

  sp->cur_rx = 0;

  for (i = 0; i < RX_RING_SIZE; i++) {
    struct pbuf *p;

    p = pbuf_alloc(PBUF_RAW, PKT_BUF_SZ + sizeof(struct RxFD), PBUF_RW);
    sp->rx_pbuf[i] = p;
    if (p == NULL) break;      // OK. Just initially short of Rx bufs
    rxf = (struct RxFD *) p->payload;
    sp->rx_ringp[i] = rxf;
    pbuf_header(p, - (int) sizeof(struct RxFD));
    if (last_rxf) last_rxf->link = virt2phys(rxf);
    last_rxf = rxf;
    rxf->status = 0x00000001;  // '1' is flag value only
    rxf->link = 0;            // None yet
    rxf->rx_buf_addr = 0xffffffff;
    rxf->count = PKT_BUF_SZ << 16;
  }
  sp->dirty_rx = (unsigned int)(i - RX_RING_SIZE);

  // Mark the last entry as end-of-list.
  last_rxf->status = 0xC0000002; // '2' is flag value only
  sp->last_rxf = last_rxf;
}

static int speedo_open(struct dev *dev) {
  struct nic *sp = (struct nic *) dev->privdata;
  long ioaddr = sp->iobase;

  //kprintf("%s: speedo_open() irq %d.\n", dev->name, sp->irq);

  // Set up the Tx queue early
  sp->cur_tx = 0;
  sp->dirty_tx = 0;
  sp->last_cmd = 0;
  sp->tx_full = 0;
  sp->polling = 0;

  if ((sp->phy[0] & 0x8000) == 0) {
    sp->advertising = mdio_read(ioaddr, sp->phy[0] & 0x1f, 4);
  }

  // With some transceivers we must retrigger negotiation to reset
  // power-up errors
  if ((sp->flags & ResetMII) && (sp->phy[0] & 0x8000) == 0) {
    int phy_addr = sp->phy[0] & 0x1f ;
    
    // Use 0x3300 for restarting NWay, other values to force xcvr:
    //   0x0000 10-HD
    //   0x0100 10-FD
    //   0x2000 100-HD
    //   0x2100 100-FD
   
    mdio_write(ioaddr, phy_addr, 0, 0x3300);
  }

  // We can safely take handler calls during init
  // Doing this after speedo_init_rx_ring() results in a memory leak
  enable_irq(sp->irq);

  // Initialize Rx and Tx rings
  speedo_init_rx_ring(dev);

  // Fire up the hardware
  speedo_resume(dev);

  // Setup the chip and configure the multicast list
  sp->mc_setup_frm = NULL;
  sp->mc_setup_frm_len = 0;
  sp->mc_setup_busy = 0;
  sp->rx_mode = -1;     // Invalid -> always reset the mode.
  sp->flow_ctrl = sp->partner = 0;
  speedo_set_rx_mode(dev);

  //kprintf("%s: Done speedo_open(), status %8.8x\n", dev->name, inpw(ioaddr + SCBStatus));

  // Set the timer.  The timer serves a dual purpose:
  // 1) to monitor the media interface (e.g. link beat) and perhaps switch
  //    to an alternate media type
  // 2) to monitor Rx activity, and restart the Rx process if the receiver
  //    hangs.

  init_timer(&sp->timer, speedo_timer, dev);
  mod_timer(&sp->timer, get_ticks() + 3*HZ);

  // No need to wait for the command unit to accept here.
  if ((sp->phy[0] & 0x8000) == 0) mdio_read(ioaddr, sp->phy[0] & 0x1f, 0);

  return 0;
}

static int speedo_close(struct dev *dev) {
  struct nic *sp = (struct nic *) dev->privdata;
  long ioaddr = sp->iobase;
  int i;

  kprintf("%s: Shutting down ethercard, status was %4.4x. Allocation failures: %d\n",  dev->name, inpw(ioaddr + SCBStatus), sp->alloc_failures);

  // Shut off the media monitoring timer
  del_timer(&sp->timer);

  // Shutting down the chip nicely fails to disable flow control. So..
  outpd(ioaddr + SCBPort, PortPartialReset);

  disable_irq(sp->irq);

  // Free all the pbufs in the Rx and Tx queues
  for (i = 0; i < RX_RING_SIZE; i++) {
    struct pbuf *p = sp->rx_pbuf[i];
    sp->rx_pbuf[i] = NULL;
    if (p) pbuf_free(p);
  }

  for (i = 0; i < TX_RING_SIZE; i++) {
    struct pbuf *p = sp->tx_pbuf[i];
    sp->tx_pbuf[i] = NULL;
    if (p) pbuf_free(p);
  }

  if (sp->mc_setup_frm) {
    kfree(sp->mc_setup_frm);
    sp->mc_setup_frm_len = 0;
  }

  // Print a few items for debugging
  speedo_show_state(dev);

  return 0;
}

// Start the chip hardware after a full reset

static void speedo_resume(struct dev *dev) {
  struct nic *sp = (struct nic *) dev->privdata;
  long ioaddr = sp->iobase;

  outpw(ioaddr + SCBCmd, SCBMaskAll);

  // Start with a Tx threshold of 256 (0x..20.... 8 byte units)
  sp->tx_threshold = 0x01208000;

  // Set the segment registers to '0'
  wait_for_cmd_done(ioaddr + SCBCmd);
  if (inp(ioaddr + SCBCmd)) {
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
  outpd(ioaddr + SCBPointer, virt2phys(&sp->lstats));
  inpd(ioaddr + SCBPointer);       // Flush to PCI
  outp(ioaddr + SCBCmd, CUStatsAddr);
  sp->lstats.done_marker = 0;
  wait_for_cmd_done(ioaddr + SCBCmd);

  outpd(ioaddr + SCBPointer, virt2phys(sp->rx_ringp[sp->cur_rx % RX_RING_SIZE]));
  inpd(ioaddr + SCBPointer);       // Flush to PCI

  // Note: RxStart should complete instantly.
  do_slow_command(dev, RxStart);
  do_slow_command(dev, CUDumpStats);

  // Fill the first command with our physical address
  {
    int entry = sp->cur_tx++ % TX_RING_SIZE;
    struct descriptor *cur_cmd = (struct descriptor *) &sp->tx_ring[entry];

    // Avoid a bug(?!) here by marking the command already completed
    cur_cmd->cmd_status = (CmdSuspend | CmdIASetup) | 0xa000;
    cur_cmd->link = virt2phys(&sp->tx_ring[sp->cur_tx % TX_RING_SIZE]);
    memcpy(cur_cmd->params, sp->hwaddr.addr, 6);
    if (sp->last_cmd) clear_suspend(sp->last_cmd);
    sp->last_cmd = cur_cmd;
  }

  // Start the chip's Tx process and unmask interrupts
  outpd(ioaddr + SCBPointer, virt2phys(&sp->tx_ring[sp->dirty_tx % TX_RING_SIZE]));
  outpw(ioaddr + SCBCmd, CUStart);
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

static struct stats_nic *speedo_get_stats(struct dev *dev) {
  struct nic *sp = (struct nic *) dev->privdata;
  long ioaddr = sp->iobase;

  // Update only if the previous dump finished.
  if (sp->lstats.done_marker == 0xA007) {
    sp->stats.tx_aborted_errors += sp->lstats.tx_coll16_errs;
    sp->stats.tx_window_errors += sp->lstats.tx_late_colls;
    sp->stats.tx_fifo_errors += sp->lstats.tx_underruns;
    sp->stats.tx_fifo_errors += sp->lstats.tx_lost_carrier;
    //sp->stats.tx_deferred += sp->lstats.tx_deferred;
    sp->stats.collisions += sp->lstats.tx_total_colls;
    sp->stats.rx_crc_errors += sp->lstats.rx_crc_errs;
    sp->stats.rx_frame_errors += sp->lstats.rx_align_errs;
    sp->stats.rx_over_errors += sp->lstats.rx_resource_errs;
    sp->stats.rx_fifo_errors += sp->lstats.rx_overrun_errs;
    sp->stats.rx_length_errors += sp->lstats.rx_runt_errs;
    sp->lstats.done_marker = 0x0000;

    wait_for_cmd_done(ioaddr + SCBCmd);
    outp(ioaddr + SCBCmd, CUDumpStats);
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

static int speedo_set_rx_mode(struct dev *dev) {
  struct nic *sp = (struct nic *) dev->privdata;
  long ioaddr = sp->iobase;
  struct descriptor *last_cmd;
  char new_rx_mode;
  int entry, i;

  if (!dev->netif) {
    // Network interface not attached yet -- accept all multicasts
    new_rx_mode = 1;
  } else if (dev->netif->flags & NETIF_PROMISC) {     
    // Set promiscuous.
    new_rx_mode = 3;
  } else if ((dev->netif->flags & NETIF_ALLMULTI) || dev->netif->mccount > multicast_filter_limit) {
    new_rx_mode = 1;
  } else {
    new_rx_mode = 0;
  }

  if (sp->cur_tx - sp->dirty_tx >= TX_RING_SIZE - 1) {
    // The Tx ring is full -- don't add anything!  Presumably the new mode
    // is in config_cmd_data and will be added anyway, otherwise we wait
    // for a timer tick or the mode to change again.
    sp->rx_mode = -1;
    return -EBUSY;
  }

  if (new_rx_mode != sp->rx_mode) {
    unsigned char *config_cmd_data;

    cli();
    entry = sp->cur_tx % TX_RING_SIZE;
    last_cmd = sp->last_cmd;
    sp->last_cmd = (struct descriptor *)&sp->tx_ring[entry];

    sp->tx_pbuf[entry] = 0;     // Redundant
    sp->tx_ring[entry].status = CmdSuspend | CmdConfigure;
    sp->cur_tx++;
    sp->tx_ring[entry].link = virt2phys(&sp->tx_ring[(entry + 1) % TX_RING_SIZE]);
    // We may nominally release the lock here

    config_cmd_data = (void *)&sp->tx_ring[entry].tx_desc_addr;
    // Construct a full CmdConfig frame
    memcpy(config_cmd_data, i82558_config_cmd, sizeof(i82558_config_cmd));
    config_cmd_data[1] = (txfifo << 4) | rxfifo;
    config_cmd_data[4] = rxdmacount;
    config_cmd_data[5] = txdmacount + 0x80;
    if (sp->flags & HasChksum) config_cmd_data[9] |= 1;
    config_cmd_data[15] |= (new_rx_mode & 2) ? 1 : 0;
    config_cmd_data[19] = sp->flow_ctrl ? 0xBD : 0x80;
    config_cmd_data[19] |= sp->full_duplex ? 0x40 : 0;
    config_cmd_data[21] = (new_rx_mode & 1) ? 0x0D : 0x05;
    if (sp->phy[0] & 0x8000) {
      // Use the AUI port instead.
      config_cmd_data[15] |= 0x80;
      config_cmd_data[8] = 0;
    }
    // Trigger the command unit resume.
    wait_for_cmd_done(ioaddr + SCBCmd);
    clear_suspend(last_cmd);
    outp(ioaddr + SCBCmd, CUResume);
    sti();
    sp->last_cmd_time = get_ticks();
  }

  if (new_rx_mode == 0 && dev->netif->mccount < 4) {
    // The simple case of 0-3 multicast list entries occurs often, and
    // fits within one tx_ring[] entry.
    struct mclist *mclist;
    unsigned short *setup_params, *eaddrs;

    cli();
    entry = sp->cur_tx % TX_RING_SIZE;
    last_cmd = sp->last_cmd;
    sp->last_cmd = (struct descriptor *)&sp->tx_ring[entry];

    sp->tx_pbuf[entry] = 0;
    sp->tx_ring[entry].status = CmdSuspend | CmdMulticastList;
    sp->cur_tx++;
    sp->tx_ring[entry].link = virt2phys(&sp->tx_ring[(entry + 1) % TX_RING_SIZE]);
    // We may nominally release the lock here
    sp->tx_ring[entry].tx_desc_addr = 0; // Really MC list count
    setup_params = (unsigned short *) &sp->tx_ring[entry].tx_desc_addr;
    *setup_params++ = dev->netif->mccount * 6;
    
    // Fill in the multicast addresses.
    for (i = 0, mclist = dev->netif->mclist; i < dev->netif->mccount; i++, mclist = mclist->next) {
      eaddrs = (unsigned short *) &mclist->hwaddr;
      *setup_params++ = *eaddrs++;
      *setup_params++ = *eaddrs++;
      *setup_params++ = *eaddrs++;
    }

    wait_for_cmd_done(ioaddr + SCBCmd);
    clear_suspend(last_cmd);
    // Immediately trigger the command unit resume
    outp(ioaddr + SCBCmd, CUResume);
    sti();
    sp->last_cmd_time = get_ticks();
  } else if (new_rx_mode == 0) {
    struct mclist *mclist;
    unsigned short *setup_params, *eaddrs;
    struct descriptor *mc_setup_frm = sp->mc_setup_frm;
    int i;

    if (sp->mc_setup_frm_len < 10 + dev->netif->mccount * 6 || sp->mc_setup_frm == NULL) {
      // Allocate a full setup frame, 10bytes + <max addrs>.
      if (sp->mc_setup_frm) kfree(sp->mc_setup_frm);
      sp->mc_setup_busy = 0;
      sp->mc_setup_frm_len = 10 + multicast_filter_limit*6;
      sp->mc_setup_frm = kmalloc(sp->mc_setup_frm_len);
      if (sp->mc_setup_frm == NULL) {
        kprintf(KERN_WARNING "%s: Failed to allocate a setup frame\n", dev->name);
        sp->rx_mode = -1; // We failed, try again.
        return -ENOMEM;
      }
    }

    // If we are busy, someone might be quickly adding to the MC list.
    // Try again later when the list updates stop.
    if (sp->mc_setup_busy) {
      sp->rx_mode = -1;
      return -EBUSY;
    }
    mc_setup_frm = sp->mc_setup_frm;

    // Fill the setup frame
    kprintf("%s: Constructing a setup frame at %p, %d bytes\n", dev->name, sp->mc_setup_frm, sp->mc_setup_frm_len);
    mc_setup_frm->cmd_status =  CmdSuspend | CmdIntr | CmdMulticastList;
    
    // Link set below
    setup_params = (unsigned short *) &mc_setup_frm->params;
    *setup_params++ = dev->netif->mccount * 6;
    
    // Fill in the multicast addresses
    for (i = 0, mclist = dev->netif->mclist; i < dev->netif->mccount; i++, mclist = mclist->next) {
      eaddrs = (unsigned short *) &mclist->hwaddr;
      *setup_params++ = *eaddrs++;
      *setup_params++ = *eaddrs++;
      *setup_params++ = *eaddrs++;
    }

    // Disable interrupts while playing with the Tx Cmd list
    cli();
    entry = sp->cur_tx % TX_RING_SIZE;
    last_cmd = sp->last_cmd;
    sp->last_cmd = mc_setup_frm;
    sp->mc_setup_busy++;

    // Change the command to a NoOp, pointing to the CmdMulti command
    sp->tx_pbuf[entry] = 0;
    sp->tx_ring[entry].status = CmdNOp;
    sp->cur_tx++;
    sp->tx_ring[entry].link = virt2phys(mc_setup_frm);
    // We may nominally release the lock here

    // Set the link in the setup frame.
    mc_setup_frm->link = virt2phys(&(sp->tx_ring[(entry+1) % TX_RING_SIZE]));

    wait_for_cmd_done(ioaddr + SCBCmd);
    clear_suspend(last_cmd);

    // Immediately trigger the command unit resume
    outp(ioaddr + SCBCmd, CUResume);
    sti();
    sp->last_cmd_time = get_ticks();
    kprintf("%s: CmdMCSetup frame length %d in entry %d\n", dev->name, dev->netif->mccount, entry);
  }

  sp->rx_mode = new_rx_mode;
  return 0;
}

// Media monitoring and control

static void speedo_timer(void *arg) {
  struct dev *dev = (struct dev *) arg;
  struct nic *sp = (struct nic *) dev->privdata;
  long ioaddr = sp->iobase;
  int phy_num = sp->phy[0] & 0x1f;
  int status = inpw(ioaddr + SCBStatus);
  unsigned int expires;

  //kprintf("%s: Interface monitor tick, chip status %4.4x\n", dev->name, status);

  // Normally we check every two seconds.
  expires = get_ticks() + 2*HZ;

  if (sp->polling) {
    // Continue to be annoying.
    if (status & 0xfc00) {
      speedo_interrupt(dev);
      if (get_ticks() - sp->last_reset > 10*HZ) {
        kprintf(KERN_WARNING "%s: IRQ %d is still blocked!\n", dev->name, sp->irq);
        sp->last_reset = get_ticks();
      }
    } else if (get_ticks() - sp->last_reset > 10*HZ) {
      sp->polling = 0;
    }
    
    expires = get_ticks() + 2;
  }

  // We have MII and lost link beat
  if ((sp->phy[0] & 0x8000) == 0) {
    int partner = mdio_read(ioaddr, phy_num, 5);
    if (partner != sp->partner) {
      int flow_ctrl = sp->advertising & partner & 0x0400 ? 1 : 0;
      sp->partner = partner;
      if (flow_ctrl != sp->flow_ctrl) {
        sp->flow_ctrl = flow_ctrl;
        sp->rx_mode = -1; // Trigger a reload
      }

      // Clear sticky bit
      mdio_read(ioaddr, phy_num, 1);
      
      // If link beat has returned...
      //if (mdio_read(ioaddr, phy_num, 1) & 0x0004)
      //  netif_link_up(dev);
      //else
      //  netif_link_down(dev);
    }
  }

  // This no longer has a false-trigger window
  if (sp->cur_tx - sp->dirty_tx > 1 && (get_ticks() - sp->trans_start) > TX_TIMEOUT  && (get_ticks() - sp->last_cmd_time) > TX_TIMEOUT) {
    if (status == 0xffff) {
      if (get_ticks() - sp->last_reset > 10*HZ) {
        sp->last_reset = get_ticks();
        kprintf(KERN_WARNING "%s: The EEPro100 chip is missing!\n", dev->name);
      }
    } else if (status & 0xfc00) {
      // We have a blocked IRQ line.  This should never happen, but we recover as best we can
      if (!sp->polling) {
        if (get_ticks() - sp->last_reset > 10*HZ) {
          kprintf(KERN_WARNING "%s: IRQ %d is physically blocked! (%4.4x) Failing back to low-rate polling\n", dev->name, sp->irq, status);
          sp->last_reset = get_ticks();
        }

        sp->polling = 1;
      }
      speedo_interrupt(dev);
      expires = get_ticks() + 2;  // Avoid 
    } else {
      speedo_tx_timeout(dev);
      sp->last_reset = get_ticks();
    }
  }

  if (sp->rx_mode < 0 || (sp->rx_bug && get_ticks() - sp->last_rx_time > 2*HZ)) {
    // We haven't received a packet in a Long Time.  We might have been
    // bitten by the receiver hang bug.  This can be cleared by sending
    // a set multicast list command.
    speedo_set_rx_mode(dev);
  }

  mod_timer(&sp->timer, expires);
}

static void speedo_tx_timeout(struct dev *dev) {
  struct nic *sp = (struct nic *) dev->privdata;
  long ioaddr = sp->iobase;
  int status = inpw(ioaddr + SCBStatus);

  kprintf(KERN_WARNING "%s: Transmit timed out: status %4.4x  %4.4x at %d/%d commands %8.8x %8.8x %8.8x\n",
       dev->name, status, inpw(ioaddr + SCBCmd),
       sp->dirty_tx, sp->cur_tx,
       sp->tx_ring[(sp->dirty_tx+0) % TX_RING_SIZE].status,
       sp->tx_ring[(sp->dirty_tx+1) % TX_RING_SIZE].status,
       sp->tx_ring[(sp->dirty_tx+2) % TX_RING_SIZE].status);

  // Trigger a stats dump to give time before the reset
  speedo_get_stats(dev);

  speedo_show_state(dev);

  kprintf(KERN_INFO "%s: Restarting the chip...\n", dev->name);

  // Reset the Tx and Rx units.
  outpd(ioaddr + SCBPort, PortReset);
  speedo_show_state(dev);
  udelay(10);
  speedo_resume(dev);

  // Reset the MII transceiver, suggested by Fred Young @ scalable.com
  if ((sp->phy[0] & 0x8000) == 0) {
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
  sp->trans_start = get_ticks();
}

// Handle the interrupt cases when something unexpected happens

static void speedo_intr_error(struct dev *dev, int intr_status) {
  struct nic *sp = (struct nic *) dev->privdata;
  long ioaddr = sp->iobase;

  if (intr_status & IntrRxSuspend) {
    if ((intr_status & 0x003c) == 0x0028)
      // No more Rx buffers
      outp(ioaddr + SCBCmd, RxResumeNoResources);
    else if ((intr_status & 0x003c) == 0x0008) {
      // No resources (why?!)
      kprintf(KERN_ERR "%s: Unknown receiver error, status=%#4.4x\n", dev->name, intr_status);

      // No idea of what went wrong.  Restart the receiver
      outpd(ioaddr + SCBPointer, virt2phys(sp->rx_ringp[sp->cur_rx % RX_RING_SIZE]));
      outp(ioaddr + SCBCmd, RxStart);
    }

    sp->stats.rx_errors++;
  }
}

static int speedo_rx(struct dev *dev) {
  struct nic *sp = (struct nic *) dev->privdata;
  int entry = sp->cur_rx % RX_RING_SIZE;
  int status;
  int rx_work_limit = sp->dirty_rx + RX_RING_SIZE - sp->cur_rx;

  //kprintf("%s: In speedo_rx()\n", dev->name);

  // If we own the next entry, it's a new packet. Send it up.
  while (sp->rx_ringp[entry] != NULL &&  (status = sp->rx_ringp[entry]->status) & RxComplete) {
    int desc_count = sp->rx_ringp[entry]->count;
    int pkt_len = desc_count & 0x07ff;

    if (--rx_work_limit < 0) break;

    //kprintf("%s: speedo_rx() status %8.8x len %d\n", dev->name, status, pkt_len);

    if ((status & (RxErrTooBig | RxOK | 0x0f90)) != RxOK) {
      if (status & RxErrTooBig) {
        kprintf("KERN_WARNING %s: Ethernet frame overran the Rx buffer, status %8.8x!\n", dev->name, status);
      } else if (!(status & RxOK)) {
        // There was a fatal error.  This *should* be impossible.
        sp->stats.rx_errors++;
        kprintf(KERN_WARNING "%s: Anomalous event in speedo_rx(), status %8.8x\n", dev->name, status);
      }
    } else {
      struct pbuf *p;

      if (sp->flags & HasChksum) pkt_len -= 2;

      // Check if the packet is long enough to just accept without
      // copying to a properly sized packet buffer

      if (pkt_len < rx_copybreak && (p = pbuf_alloc(PBUF_RAW, pkt_len, PBUF_RW)) != NULL) {
        memcpy(p->payload, sp->rx_pbuf[entry]->payload, pkt_len);
      } else {
        // Pass up the already-filled pbuf
        p = sp->rx_pbuf[entry];
        if (p == NULL) {
          kprintf(KERN_WARNING "%s: Inconsistent Rx descriptor chain\n", dev->name);
          break;
        }

        sp->rx_pbuf[entry] = NULL;
        sp->rx_ringp[entry] = NULL;

        pbuf_realloc(p, pkt_len);
      }

      // Send packet to upper layer
      if (dev_receive(sp->devno, p) < 0) pbuf_free(p);

      sp->stats.rx_packets++;
      sp->stats.rx_bytes += pkt_len;
    }

    entry = (++sp->cur_rx) % RX_RING_SIZE;
  }

  // Refill the Rx ring buffers
  for (; sp->cur_rx - sp->dirty_rx > 0; sp->dirty_rx++) {
    struct RxFD *rxf;
    entry = sp->dirty_rx % RX_RING_SIZE;
    
    if (sp->rx_pbuf[entry] == NULL) {
      struct pbuf *p;

      // Get a fresh pbuf to replace the consumed one
      p = pbuf_alloc(PBUF_RAW, PKT_BUF_SZ + sizeof(struct RxFD), PBUF_RW);
      sp->rx_pbuf[entry] = p;
      if (p == NULL) {
        sp->rx_ringp[entry] = NULL;
        sp->alloc_failures++;
        break;      // Better luck next time! 
      }
      rxf = sp->rx_ringp[entry] = (struct RxFD *) p->payload;
      pbuf_header(p, - (int) sizeof(struct RxFD));
      rxf->rx_buf_addr = virt2phys(p->payload);
    } else {
      rxf = sp->rx_ringp[entry];
    }

    rxf->status = 0xC0000001;  // '1' for driver use only
    rxf->link = 0;      // None yet
    rxf->count = PKT_BUF_SZ << 16;
    sp->last_rxf->link = virt2phys(rxf);
    sp->last_rxf->status &= ~0xC0000000;
    sp->last_rxf = rxf;
  }

  sp->last_rx_time = get_ticks();
  return 0;
}

// The interrupt handler does all of the Rx thread work and cleans up
// after the Tx thread.

static void speedo_interrupt(struct dev *dev) {
  struct nic *sp = (struct nic *) dev->privdata;
  long ioaddr = sp->iobase;
  long boguscnt = max_interrupt_work;
  unsigned short status;

  while (1) {
    status = inpw(ioaddr + SCBStatus);

    if ((status & IntrAllNormal) == 0  ||  status == 0xffff) break;

    // Acknowledge all of the current interrupt sources ASAP
    outpw(ioaddr + SCBStatus, status & IntrAllNormal);

    //kprintf("%s: interrupt status=%#4.4x\n", dev->name, status);

    if (status & (IntrRxDone | IntrRxSuspend)) {
      speedo_rx(dev);
    }

    // The command unit did something, scavenge finished Tx entries
    if (status & (IntrCmdDone | IntrCmdIdle | IntrDrvrIntr)) {
      unsigned int dirty_tx;
      int entries_freed = 0;

      dirty_tx = sp->dirty_tx;
      while (sp->cur_tx - dirty_tx > 0) {
        int entry = dirty_tx % TX_RING_SIZE;
        int status = sp->tx_ring[entry].status;

        //kprintf("%s: scavenge candidate %d status %4.4x\n", dev->name, entry, status);

        if ((status & StatusComplete) == 0) {
          // Special case error check: look for descriptor that the chip skipped(?)
          if (sp->cur_tx - dirty_tx > 2  && (sp->tx_ring[(dirty_tx + 1) % TX_RING_SIZE].status & StatusComplete)) {
            kprintf(KERN_ERR "%s: Command unit failed to mark command %8.8x as complete at %d\n", dev->name, status, dirty_tx);
          } else {
            // It still hasn't been processed
            break;
          }
        }

        if ((status & TxUnderrun) && (sp->tx_threshold < 0x01e08000)) {
          sp->tx_threshold += 0x00040000;
          kprintf("%s: Tx threshold increased, %#8.8x\n", dev->name, sp->tx_threshold);
        }

        // Free the original pbuf
        if (sp->tx_pbuf[entry]) {
          // Count only user packets
          sp->stats.tx_packets++; 
          sp->stats.tx_bytes += sp->tx_pbuf[entry]->len;
          pbuf_free(sp->tx_pbuf[entry]);
          sp->tx_pbuf[entry] = NULL;
        } else if ((status & 0x70000) == CmdNOp) {
          sp->mc_setup_busy = 0;
        }
        
        dirty_tx++;
        entries_freed++;
      }

      sp->dirty_tx = dirty_tx;
      release_sem(&sp->tx_sem, entries_freed);
    }

    if (status & IntrRxSuspend) {
      speedo_intr_error(dev, status);
    }

    if (--boguscnt < 0) {
      kprintf("%s: Too much work at interrupt, status=0x%4.4x\n", dev->name, status);

      // Clear all interrupt sources.
      outpd(ioaddr + SCBStatus, 0xfc00);
      break;
    }
  }

  //kprintf("%s: exiting interrupt, status=%#4.4x\n", dev->name, inpw(ioaddr + SCBStatus));
}

static void speedo_dpc(void *arg) {
  struct dev *dev = (struct dev *) arg;
  struct nic *sp = (struct nic *) dev->privdata;

  speedo_interrupt(dev);
  eoi(sp->irq);
}

static int speedo_handler(struct context *ctxt, void *arg) {
  struct dev *dev = (struct dev *) arg;
  struct nic *sp = (struct nic *) dev->privdata;

  // Queue DPC to service interrupt
  //kprintf("%s: interrupt\n", dev->name);
  queue_irq_dpc(&sp->dpc, speedo_dpc, dev);

  return 0;
}

static int speedo_transmit(struct dev *dev, struct pbuf *p) {
  struct nic *sp = (struct nic *) dev->privdata;
  long ioaddr = sp->iobase;
  int entry;

  p = pbuf_linearize(PBUF_RAW, p);
  if (!p) return -ENOMEM;

  // Wait for free entry in transmit ring
  if (wait_for_object(&sp->tx_sem, TX_TIMEOUT) < 0) {
    kprintf("%s: transmit timeout, drop packet\n", dev->name);
    sp->stats.tx_dropped++;
    return -ETIMEOUT;
  }

  // Caution: the write order is important here, set the base address
  // with the "ownership" bits last.

  // Calculate the Tx descriptor entry
  entry = sp->cur_tx % TX_RING_SIZE;

  sp->tx_pbuf[entry] = p;
  // TODO: be a little more clever about setting the interrupt bit
  sp->tx_ring[entry].status = CmdSuspend | CmdTx | CmdTxFlex;
  sp->cur_tx++;
  sp->tx_ring[entry].link = virt2phys(&sp->tx_ring[sp->cur_tx % TX_RING_SIZE]);
  sp->tx_ring[entry].tx_desc_addr = virt2phys(&sp->tx_ring[entry].tx_buf_addr0);
  // The data region is always in one buffer descriptor
  sp->tx_ring[entry].count = sp->tx_threshold;
  sp->tx_ring[entry].tx_buf_addr0 = virt2phys(p->payload);
  sp->tx_ring[entry].tx_buf_size0 = p->tot_len;
  
  // TODO: perhaps leave the interrupt bit set if the Tx queue is more
  // than half full.  Argument against: we should be receiving packets
  // and scavenging the queue.  Argument for: if so, it shouldn't
  // matter.

  {
    struct descriptor *last_cmd = sp->last_cmd;
    sp->last_cmd = (struct descriptor *) &sp->tx_ring[entry];
    clear_suspend(last_cmd);
  }
  
  wait_for_cmd_done(ioaddr + SCBCmd);
  outp(ioaddr + SCBCmd, CUResume);
  sp->trans_start = get_ticks();

  return 0;
}

static int speedo_ioctl(struct dev *dev, int cmd, void *args, size_t size) {
  return -ENOSYS;
}

static int speedo_attach(struct dev *dev, struct eth_addr *hwaddr) {
  struct nic *np = dev->privdata;
  *hwaddr = np->hwaddr;

  return 0;
}

static int speedo_detach(struct dev *dev) {
  return 0;
}

struct driver speedo_driver = {
  "eepro100",
  DEV_TYPE_PACKET,
  speedo_ioctl,
  NULL,
  NULL,
  speedo_attach,
  speedo_detach,
  speedo_transmit,
  speedo_set_rx_mode,
};

int __declspec(dllexport) install(struct unit *unit, char *opts) {
  int i;
  struct board *board;
  struct nic *sp;
  struct dev *dev;
  unsigned short ioaddr;
  unsigned short irq;
  unsigned short eeprom[0x100];

  // Check license
  if (license() != LICENSE_GPL) kprintf(KERN_WARNING "notice: eepro100 driver is under GPL license\n");

  // Determine NIC type
  board = lookup_board(board_tbl, unit);
  if (!board) return -EIO;

  unit->vendorname = board->vendorname;
  unit->productname = board->productname;

  // Get NIC PCI configuration
  ioaddr = (unsigned short) get_unit_iobase(unit);
  irq = irq = (unsigned short) get_unit_irq(unit);

  // Enable bus mastering
  pci_enable_busmastering(unit);

  // Allocate private memory
  sp = kmalloc(sizeof(struct nic));
  if (sp == NULL) return -ENOMEM;
  memset(sp, 0, sizeof(struct nic));

  // Read the station address EEPROM before doing the reset.
  // Nominally his should even be done before accepting the device, but
  // then we wouldn't have a device name with which to report the error.
  // The size test is for 6 bit vs. 8 bit address serial EEPROMs.
 
  {
    unsigned short sum = 0;
    int j;
    int read_cmd, ee_size;

    if ((do_eeprom_cmd(ioaddr, EE_READ_CMD << 24, 27) & 0xffe0000) == 0xffe0000) {
      ee_size = 0x100;
      read_cmd = EE_READ_CMD << 24;
    } else {
      ee_size = 0x40;
      read_cmd = EE_READ_CMD << 22;
    }

    for (j = 0, i = 0; i < ee_size; i++) {
      unsigned short value = do_eeprom_cmd(ioaddr, read_cmd | (i << 16), 27);
      eeprom[i] = value;
      sum += value;
      if (i < 3) {
        sp->hwaddr.addr[j++] = (unsigned char) (value % 0xFF);
        sp->hwaddr.addr[j++] = (unsigned char) (value >> 8);
      }
    }

    if (sum != 0xBABA) kprintf(KERN_WARNING "eepro100: Invalid EEPROM checksum %#4.4x!\n", sum);
  }

  // Reset the chip: stop Tx and Rx processes and clear counters.
  // This takes less than 10usec and will easily finish before the next
  // action.
  outpd(ioaddr + SCBPort, PortReset);

  // Create new device
  sp->devno = dev_make("eth#", &speedo_driver, unit, sp);
  if (sp->devno == NODEV) return -ENODEV;
  dev = device(sp->devno);

  init_dpc(&sp->dpc);
  register_interrupt(&sp->intr, IRQ2INTR(irq), speedo_handler, dev);

  sp->dev = dev;
  sp->iobase = ioaddr;
  sp->irq = irq;
  sp->board = board;
  sp->flags = board->flags;

  // Set options
  if (opts) {
    sp->full_duplex = get_num_option(opts, "fullduplex", 0);
  }

  if (sp->full_duplex) sp->medialock = 1;

  sp->phy[0] = eeprom[6];
  sp->phy[1] = eeprom[7];
  sp->rx_bug = (eeprom[3] & 0x03) == 3 ? 0 : 1;


  if (sp->rx_bug) kprintf(KERN_INFO "%s: Receiver lock-up workaround activated\n", dev->name);

  kprintf(KERN_INFO "%s: %s%s iobase 0x%x irq %d mac %la\n", dev->name, eeprom[3] & 0x0100 ? "OEM " : "", unit->productname, ioaddr, irq, &sp->hwaddr);

  return speedo_open(dev);
}

int __stdcall start(hmodule_t hmod, int reason, void *reserved2) {
  return 1;
}
