//
// rtl8139.c
//
// RealTek RTL8129/RTL8139 PCI NIC network driver
//
// Copyright (C) 2002 Michael Ringgaard. All rights reserved.
// Portions Copyright (C) 1997-2002 by Donald Becker.
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

// The user-configurable values

// Maximum events (Rx packets, etc.) to handle at each interrupt

static int max_interrupt_work = 20;

// Maximum number of multicast addresses to filter (vs. Rx-all-multicast).
// The RTL chips use a 64 element hash table based on the Ethernet CRC.  It
// is efficient to update the hardware filter, but recalculating the table
// for a long filter list is painful

static int multicast_filter_limit = 32;

// Operational parameters that are set at compile time

// Maximum size of the in-memory receive ring (smaller if no memory)

#define RX_BUF_LEN_IDX  2     // 0=8K, 1=16K, 2=32K, 3=64K

// Size of the Tx bounce buffers -- must be at least (mtu+14+4)

#define TX_BUF_SIZE 1536

// PCI Tuning Parameters
// Threshold is bytes transferred to chip before transmission starts

#define TX_FIFO_THRESH 256  // In bytes, rounded down to 32 byte units

// The following settings are log_2(bytes)-4:  0 = 16 bytes .. 6 = 1024

#define RX_FIFO_THRESH  4   // Rx buffer level before first PCI xfer
#define RX_DMA_BURST    4   // Maximum PCI burst, '4' is 256 bytes
#define TX_DMA_BURST    4   // Calculate as 16 << val

// Operational parameters that usually are not changed
// Time in ticks before concluding the transmitter is hung

#define TX_TIMEOUT  (6*HZ)

// Allocation size of Rx buffers with full-sized Ethernet frames.
// This is a cross-driver value that is not a limit,
// but a way to keep a consistent allocation size among drivers.

#define PKT_BUF_SZ    1536

static void *rtl8139_probe1(struct pci_dev *pdev, void *init_dev, long ioaddr, int irq, int chip_idx, int find_cnt);
static int rtl_pwr_event(void *dev_instance, int event);

enum chip_capability_flags 
{
  HAS_MII_XCVR    = 0x01, 
  HAS_CHIP_XCVR   = 0x02,
  HAS_LNK_CHNG    = 0x04, 
  HAS_DESC        = 0x08
};

#define RTL8129_CAPS  HAS_MII_XCVR
#define RTL8139_CAPS  HAS_CHIP_XCVR | HAS_LNK_CHNG
#define RTL8139D_CAPS  HAS_CHIP_XCVR | HAS_LNK_CHNG | HAS_DESC

struct chip_info
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

static struct chip_info chip_tbl[] = 
{
  {"RealTek", "RealTek RTL8139+", PCI_UNITCODE(0x10ec, 0x8139), 0xffffffff, 0,0, 0x20, 0xff, RTL8139D_CAPS},
  {"RealTek", "RealTek RTL8139C Fast Ethernet", PCI_UNITCODE(0x10ec, 0x8139), 0xffffffff, 0,0, 0x10, 0xff, RTL8139_CAPS},
  {"RealTek", "RealTek RTL8129 Fast Ethernet", PCI_UNITCODE(0x10ec, 0x8129), 0xffffffff, 0, 0, 0, 0, RTL8129_CAPS},
  {"RealTek", "RealTek RTL8139 Fast Ethernet", PCI_UNITCODE(0x10ec, 0x8139), 0xffffffff, 0, 0, 0, 0, RTL8139_CAPS},
  {"RealTek", "RealTek RTL8139B PCI",  PCI_UNITCODE(0x10ec, 0x8138), 0xffffffff, 0, 0, 0, 0, RTL8139_CAPS},
  {"SMC", "SMC1211TX EZCard 10/100 (RealTek RTL8139)", PCI_UNITCODE(0x1113, 0x1211), 0xffffffff, 0, 0, 0, 0, RTL8139_CAPS},
  {"Accton", "Accton MPX5030 (RTL8139)", PCI_UNITCODE(0x1113, 0x1211), 0xffffffff, 0, 0, 0, 0, RTL8139_CAPS},
  {"D-Link", "D-Link DFE-538TX (RTL8139)", PCI_UNITCODE(0x1186, 0x1300), 0xffffffff, 0, 0, 0, 0, RTL8139_CAPS},
  {"LevelOne", "LevelOne FPC-0106Tx (RTL8139)", PCI_UNITCODE(0x018a, 0x0106), 0xffffffff, 0, 0, 0, 0, RTL8139_CAPS},
  {"Compaq", "Compaq HNE-300 (RTL8139c)", PCI_UNITCODE(0x021b, 0x8139), 0xffffffff, 0, 0, 0, 0, RTL8139_CAPS},
  {"Generic", "Generic RTL8139", 0, 0, 0, 0, 0, 0, RTL8139_CAPS},
  {NULL,},
};

// Number of Tx descriptor registers

#define NUM_TX_DESC 4

// Symbolic offsets to registers

enum RTL8129_registers 
{
  MAC0             = 0x00,       // Ethernet hardware address
  MAR0             = 0x08,       // Multicast filter
  TxStatus0        = 0x10,       // Transmit status (Four 32bit registers)
  TxAddr0          = 0x20,       // Tx descriptors (also four 32bit)
  RxBuf            = 0x30, 
  RxEarlyCnt       = 0x34, 
  RxEarlyStatus    = 0x36,
  ChipCmd          = 0x37,
  RxBufPtr         = 0x38,
  RxBufAddr        = 0x3A,
  IntrMask         = 0x3C,
  IntrStatus       = 0x3E,
  TxConfig         = 0x40,
  RxConfig         = 0x44,
  Timer            = 0x48,        // A general-purpose counter
  RxMissed         = 0x4C,        // 24 bits valid, write clears
  Cfg9346          = 0x50, 
  Config0          = 0x51, 
  Config1          = 0x52,
  FlashReg         = 0x54, 
  GPPinData        = 0x58, 
  GPPinDir         = 0x59, 
  MII_SMI          = 0x5A, 
  HltClk           = 0x5B,
  MultiIntr        = 0x5C, 
  TxSummary        = 0x60,
  MII_BMCR         = 0x62, 
  MII_BMSR         = 0x64, 
  NWayAdvert       = 0x66, 
  NWayLPAR         = 0x68,
  NWayExpansion    = 0x6A,
  
  // Undocumented registers, but required for proper operation
  FIFOTMS          = 0x70,        // FIFO Control and test
  CSCR             = 0x74,        // Chip Status and Configuration Register
  PARA78           = 0x78, 
  PARA7c           = 0x7c,        // Magic transceiver parameter register
};

enum ChipCmdBits 
{
  CmdReset   = 0x10,
  CmdRxEnb   = 0x08,
  CmdTxEnb   = 0x04,
  RxBufEmpty = 0x01,
};

// Interrupt register bits

enum IntrStatusBits 
{
  PCIErr     = 0x8000, 
  PCSTimeout = 0x4000,
  RxFIFOOver = 0x40, 
  RxUnderrun = 0x20, 
  RxOverflow = 0x10,
  TxErr      = 0x08,
  TxOK       = 0x04, 
  RxErr      = 0x02, 
  RxOK       = 0x01,
};

enum TxStatusBits 
{
  TxHostOwns    = 0x2000,
  TxUnderrun    = 0x4000,
  TxStatOK      = 0x8000,
  TxOutOfWindow = 0x20000000,
  TxAborted     = 0x40000000,
  TxCarrierLost = 0x80000000,
};

enum RxStatusBits 
{
  RxMulticast = 0x8000, 
  RxPhysical  = 0x4000, 
  RxBroadcast = 0x2000,
  RxBadSymbol = 0x0020, 
  RxRunt      = 0x0010, 
  RxTooLong   = 0x0008, 
  RxCRCErr    = 0x0004,
  RxBadAlign  = 0x0002, 
  RxStatusOK  = 0x0001,
};

// Bits in RxConfig

enum RxConfigBits
{
  AcceptErr       = 0x20, 
  AcceptRunt      = 0x10, 
  AcceptBroadcast = 0x08,
  AcceptMulticast = 0x04, 
  AcceptMyPhys    = 0x02, 
  AcceptAllPhys   = 0x01,
};

enum CSCRBits 
{
  CSCR_LinkOKBit      = 0x0400, 
  CSCR_LinkChangeBit  = 0x0800,
  CSCR_LinkStatusBits = 0x0f000, 
  CSCR_LinkDownOffCmd = 0x003c0,
  CSCR_LinkDownCmd    = 0x0f3c0,
};

// Twister tuning parameters from RealTek.
// Completely undocumented, but required to tune bad links.

#define PARA78_default  0x78fa8388
#define PARA7c_default  0xcb38de43      /* param[0][3] */
#define PARA7c_xxx      0xcb38de43

unsigned long param[4][4] = 
{
  {0xcb39de43, 0xcb39ce43, 0xfb38de03, 0xcb38de43},
  {0xcb39de43, 0xcb39ce43, 0xcb39ce83, 0xcb39ce83},
  {0xcb39de43, 0xcb39ce43, 0xcb39ce83, 0xcb39ce83},
  {0xbb39de43, 0xbb39ce43, 0xbb39ce83, 0xbb39ce83}
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

struct nic 
{
  devno_t devno;                        // Device number
  struct dev *dev;                      // Device block

  unsigned short iobase;		// Configured I/O base
  unsigned short irq;		        // Configured IRQ

  struct interrupt intr;                // Interrupt object for driver
  struct dpc dpc;                       // DPC for driver
  struct timer timer;                   // Media selection timer

  struct eth_addr hwaddr;               // MAC address for NIC

  int chip_id;
  int flags;
  struct stats_nic stats;
  int msg_level;
  int max_interrupt_work;

  // Receive state
  unsigned char *rx_ring;
  unsigned int cur_rx;                  // Index into the Rx buffer of next Rx pkt.
  unsigned int rx_buf_len;              // Size (8K 16K 32K or 64KB) of the Rx ring

  // Transmit state
  unsigned int cur_tx, dirty_tx, tx_flag;
  unsigned long tx_full;                // The Tx queue is full
  struct pbuf *tx_pbuf[NUM_TX_DESC];    // The saved address of a sent-in-place packet
  unsigned char *tx_buf[NUM_TX_DESC];   // Tx bounce buffers
  unsigned char *tx_bufs;               // Tx bounce buffer region

  // Receive filter state
  unsigned int rx_config;
  unsigned long mc_filter[2];           // Multicast hash filter
  int cur_rx_mode;
  int multicast_filter_limit;

  // Transceiver state
  char phys[4];                       // MII device addresses
  unsigned short advertising;         // NWay media advertisement
  char twistie, twist_row, twist_col; // Twister tune state
  unsigned char config1;
  unsigned char full_duplex;          // Full-duplex operation requested
  unsigned char duplex_lock;
  unsigned char link_speed;
  unsigned char media2;               // Secondary monitored media port
  unsigned char medialock;            // Don't sense media type
  unsigned char mediasense;           // Media sensing in progress
};

static int rtl8129_open(struct dev *dev);

static void rtl_hw_start(struct dev *dev);
static int read_eeprom(long ioaddr, int location, int addr_len);
static int mdio_read(struct dev *dev, int phy_id, int location);
static void mdio_write(struct dev *dev, int phy_id, int location, int val);
static void rtl8129_timer(unsigned long data);
static void rtl8129_tx_timeout(struct dev *dev);
static void rtl8129_init_ring(struct dev *dev);
static int rtl8129_start_xmit(struct sk_buff *skb, struct dev *dev);
static int rtl8129_rx(struct dev *dev);
static void rtl8129_interrupt(int irq, void *dev_instance, struct pt_regs *regs);
static void rtl_error(struct dev *dev, int status, int link_status);
static int rtl8129_close(struct dev *dev);
static int mii_ioctl(struct dev *dev, struct ifreq *rq, int cmd);
static struct net_device_stats *rtl8129_get_stats(struct dev *dev);
static unsigned long ether_crc(int length, unsigned char *data);
static void set_rx_mode(struct dev *dev);

//
// Serial EEPROM section
//

//  EEPROM_Ctrl bits

#define EE_SHIFT_CLK  0x04  // EEPROM shift clock
#define EE_CS         0x08  // EEPROM chip select
#define EE_DATA_WRITE 0x02  // EEPROM chip data in
#define EE_WRITE_0    0x00
#define EE_WRITE_1    0x02
#define EE_DATA_READ  0x01  // EEPROM chip data out
#define EE_ENB       (0x80 | EE_CS)

// Delay between EEPROM clock transitions.
// No extra delay is needed with 33Mhz PCI, but 66Mhz may change this.

#define eeprom_delay()  inpd(ee_addr)

// The EEPROM commands include the alway-set leading bit

#define EE_WRITE_CMD  (5)
#define EE_READ_CMD   (6)
#define EE_ERASE_CMD  (7)

static int read_eeprom(long ioaddr, int location, int addr_len)
{
  int i;
  unsigned retval = 0;
  long ee_addr = ioaddr + Cfg9346;
  int read_cmd = location | (EE_READ_CMD << addr_len);

  outp(ee_addr, EE_ENB & ~EE_CS);
  outp(ee_addr, EE_ENB);

  // Shift the read command bits out
  for (i = 4 + addr_len; i >= 0; i--) 
  {
    int dataval = (read_cmd & (1 << i)) ? EE_DATA_WRITE : 0;
    outp(ee_addr, EE_ENB | dataval);
    eeprom_delay();
    outp(ee_addr, EE_ENB | dataval | EE_SHIFT_CLK);
    eeprom_delay();
  }
  outp(ee_addr, EE_ENB);
  eeprom_delay();

  for (i = 16; i > 0; i--) 
  {
    outp(ee_addr, EE_ENB | EE_SHIFT_CLK);
    eeprom_delay();
    retval = (retval << 1) | ((inp(ee_addr) & EE_DATA_READ) ? 1 : 0);
    outp(ee_addr, EE_ENB);
    eeprom_delay();
  }

  // Terminate the EEPROM access
  outp(ee_addr, ~EE_CS);
  return retval;
}

// MII serial management

// Read and write the MII management registers using software-generated
// serial MDIO protocol.
// The maximum data clock rate is 2.5 Mhz.  The minimum timing is usually
// met by back-to-back PCI I/O cycles, but we insert a delay to avoid
// "overclocking" issues

#define MDIO_DIR      0x80
#define MDIO_DATA_OUT 0x04
#define MDIO_DATA_IN  0x02
#define MDIO_CLK      0x01
#define MDIO_WRITE0   (MDIO_DIR)
#define MDIO_WRITE1   (MDIO_DIR | MDIO_DATA_OUT)

#define mdio_delay(mdio_addr) inpd(mdio_addr)

static char mii_2_8139_map[8] = 
{
  MII_BMCR, MII_BMSR, 0, 0, NWayAdvert, NWayLPAR, NWayExpansion, 0 
};

// Syncronize the MII management interface by shifting 32 one bits out

static void mdio_sync(long mdio_addr)
{
  int i;

  for (i = 32; i >= 0; i--) 
  {
    outp(mdio_addr, MDIO_WRITE1);
    mdio_delay(mdio_addr);
    outp(mdio_addr, MDIO_WRITE1 | MDIO_CLK);
    mdio_delay(mdio_addr);
  }
}

static int mdio_read(struct dev *dev, int phy_id, int location)
{
  struct nic *np = (struct nic *) dev->privdata;
  long mdio_addr = np->iobase + MII_SMI;
  int mii_cmd = (0xf6 << 10) | (phy_id << 5) | location;
  int retval = 0;
  int i;

  if (phy_id > 31) 
  {  
    // Really a 8139.  Use internal registers
    return location < 8 && mii_2_8139_map[location] ? inpw(np->iobase + mii_2_8139_map[location]) : 0;
  }

  mdio_sync(mdio_addr);

  // Shift the read command bits out
  for (i = 15; i >= 0; i--) 
  {
    int dataval = (mii_cmd & (1 << i)) ? MDIO_DATA_OUT : 0;

    outp(mdio_addr, MDIO_DIR | dataval);
    mdio_delay(mdio_addr);
    outp(mdio_addr, MDIO_DIR | dataval | MDIO_CLK);
    mdio_delay(mdio_addr);
  }

  // Read the two transition, 16 data, and wire-idle bits
  for (i = 19; i > 0; i--) 
  {
    outp(mdio_addr, 0);
    mdio_delay(mdio_addr);
    retval = (retval << 1) | ((inp(mdio_addr) & MDIO_DATA_IN) ? 1 : 0);
    outp(mdio_addr, MDIO_CLK);
    mdio_delay(mdio_addr);
  }

  return (retval >> 1) & 0xffff;
}

static void mdio_write(struct dev *dev, int phy_id, int location, int value)
{
  struct nic *np = (struct nic *) dev->privdata;
  long mdio_addr = np->iobase + MII_SMI;
  int mii_cmd = (0x5002 << 16) | (phy_id << 23) | (location << 18) | value;
  int i;

  if (phy_id > 31) 
  {  
    /* Really a 8139.  Use internal registers. */
    long ioaddr = np->iobase;
    if (location == 0) 
    {
      outp(ioaddr + Cfg9346, 0xC0);
      outpw(ioaddr + MII_BMCR, value);
      outp(ioaddr + Cfg9346, 0x00);
    } 
    else if (location < 8  &&  mii_2_8139_map[location])
    {
      outpw(ioaddr + mii_2_8139_map[location], value);
    }
  }
  else
  {
    mdio_sync(mdio_addr);

    // Shift the command bits out
    for (i = 31; i >= 0; i--) 
    {
      int dataval = (mii_cmd & (1 << i)) ? MDIO_WRITE1 : MDIO_WRITE0;
      outp(mdio_addr, dataval);
      mdio_delay(mdio_addr);
      outp(mdio_addr, dataval | MDIO_CLK);
      mdio_delay(mdio_addr);
    }

    // Clear out extra bits
    for (i = 2; i > 0; i--) 
    {
      outp(mdio_addr, 0);
      mdio_delay(mdio_addr);
      outp(mdio_addr, MDIO_CLK);
      mdio_delay(mdio_addr);
    }
  }
}

#ifdef xxx

static void rtl8129_tx_timeout(struct net_device *dev)
{
  struct nic *tp = (struct nic *) dev->priv;
  long ioaddr = dev->base_addr;
  int status = inw(ioaddr + IntrStatus);
  int mii_reg, i;

  kprintf("%s: Transmit timeout, status %2.2x %4.4x media %2.2x\n",
    dev->name, inp(ioaddr + ChipCmd), status, inp(ioaddr + GPPinData));

  if (status & (TxOK | RxOK)) 
  {
    kprintf("%s: RTL8139 Interrupt line blocked, status %x\n", dev->name, status);
  }

  // Disable interrupts by clearing the interrupt mask
  outpw(ioaddr + IntrMask, 0x0000);

  // Emit info to figure out what went wrong
  kprintf("%s: Tx queue start entry %d  dirty entry %d%s\n", dev->name, tp->cur_tx, tp->dirty_tx, tp->tx_full ? ", full" : "");
  
  for (i = 0; i < NUM_TX_DESC; i++)
    kprintf("%s:  Tx descriptor %d is %8.8x.%s\n",
         dev->name, i, (int)inpd(ioaddr + TxStatus0 + i * 4),
         i == tp->dirty_tx % NUM_TX_DESC ? " (queue head)" : "");

  kprintf("%s: MII #%d registers are:", dev->name, tp->phys[0]);
  for (mii_reg = 0; mii_reg < 8; mii_reg++)
    kprintf(" %4.4x", mdio_read(dev, tp->phys[0], mii_reg));
  printk("\n");

  // Stop a shared interrupt from scavenging while we are
  tp->dirty_tx = tp->cur_tx = 0;

  // Dump the unsent Tx packets
  for (i = 0; i < NUM_TX_DESC; i++) 
  {
    if (tp->tx_skbuff[i]) 
    {
      dev_free_skb(tp->tx_skbuff[i]);
      tp->tx_skbuff[i] = 0;
      tp->stats.tx_dropped++;
    }
  }

  rtl_hw_start(dev);
  netif_unpause_tx_queue(dev);
  tp->tx_full = 0;
}

static int rtl8129_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
  struct nic *tp = (struct nic *) dev->priv;
  long ioaddr = dev->base_addr;
  int entry;

  if (netif_pause_tx_queue(dev) != 0) 
  {
    // This watchdog code is redundant with the media monitor timer. */
    if (ticks - dev->trans_start > TX_TIMEOUT) rtl8129_tx_timeout(dev);
    return -EIO;
  }

  // Calculate the next Tx descriptor entry
  entry = tp->cur_tx % NUM_TX_DESC;

  tp->tx_skbuff[entry] = skb;
  if ((long)skb->data & 3) 
  {
    // Must use alignment buffer
    memcpy(tp->tx_buf[entry], skb->data, skb->len);
    outpd(ioaddr + TxAddr0 + entry * 4, virt_to_bus(tp->tx_buf[entry]));
  } 
  else
    outl(ioaddr + TxAddr0 + entry * 4, virt_to_bus(skb->data));
  
  // Note: the chip doesn't have auto-pad! */
  outpd(ioaddr + TxStatus0 + entry * 4, tp->tx_flag | (skb->len >= ETH_ZLEN ? skb->len : ETH_ZLEN));

  // There is a race condition here -- we might read dirty_tx, take an
  // interrupt that clears the Tx queue, and only then set tx_full.
  // So we do this in two phases

  if (++tp->cur_tx - tp->dirty_tx >= NUM_TX_DESC)
  {
    set_bit(0, &tp->tx_full);

    if (tp->cur_tx - (volatile unsigned int) tp->dirty_tx < NUM_TX_DESC) 
    {
      clear_bit(0, &tp->tx_full);
      netif_unpause_tx_queue(dev);
    } 
    else
      netif_stop_tx_queue(dev);
  } 
  else
    netif_unpause_tx_queue(dev);

  dev->trans_start = ticks;
  if (tp->msg_level & NETIF_MSG_TX_QUEUED)
  {
    kprintf("%s: Queued Tx packet at %p size %d to slot %d\n", dev->name, skb->data, (int) skb->len, entry);
  }

  return 0;
}

// The interrupt handler does all of the Rx thread work and cleans up after the Tx thread

static void rtl8129_interrupt(int irq, void *dev_instance, struct pt_regs *regs)
{
  struct net_device *dev = (struct net_device *) dev_instance;
  struct nic *np = (struct nic *) dev->priv;
  struct nic *tp = np;
  int boguscnt = np->max_interrupt_work;
  long ioaddr = dev->base_addr;
  int link_changed = 0;

  while (1)
  {
    int status = inpw(ioaddr + IntrStatus);
    
    // Acknowledge all of the current interrupt sources ASAP, but
    // first get an additional status bit from CSCR
    if (status & RxUnderrun) link_changed = inpw(ioaddr + CSCR) & CSCR_LinkChangeBit;
    outpw(ioaddr + IntrStatus, status);

    if (tp->msg_level & NETIF_MSG_INTR)
    {
      kprintf("%s: interrupt status=%#4.4x new intstat=%#4.4x\n", dev->name, status, inpw(ioaddr + IntrStatus));
    }

    if ((status & (PCIErr | PCSTimeout | RxUnderrun | RxOverflow | RxFIFOOver | TxErr | TxOK | RxErr | RxOK)) == 0) break;

    if (status & (RxOK | RxUnderrun | RxOverflow | RxFIFOOver))
    {
      // Rx interrupt
      rtl8129_rx(dev);
    }

    if (status & (TxOK | TxErr)) 
    {
      unsigned int dirty_tx = tp->dirty_tx;

      while (tp->cur_tx - dirty_tx > 0) 
      {
        int entry = dirty_tx % NUM_TX_DESC;
        int txstatus = inpd(ioaddr + TxStatus0 + entry * 4);

        if (!(txstatus & (TxStatOK | TxUnderrun | TxAborted))) break; // It still hasn't been Txed

        // Note: TxCarrierLost is always asserted at 100mbps
        if (txstatus & (TxOutOfWindow | TxAborted))
	{
          // There was an major error, log it
          if (tp->msg_level & NETIF_MSG_TX_ERR)
	  {
            kprintf("%s: Transmit error, Tx status %8.8x\n", dev->name, txstatus);
	  }
          tp->stats.tx_errors++;
          if (txstatus & TxAborted) 
	  {
            tp->stats.tx_aborted_errors++;
            outpd(ioaddr + TxConfig, TX_DMA_BURST << 8);
          }
          if (txstatus & TxCarrierLost) tp->stats.tx_carrier_errors++;
          if (txstatus & TxOutOfWindow) tp->stats.tx_window_errors++;
          if ((txstatus & 0x0f000000) == 0x0f000000) tp->stats.collisions16++;
        }
	else 
	{
          if (tp->msg_level & NETIF_MSG_TX_DONE)
	  {
            kprintf("%s: Transmit done, Tx status %8.8x\n", dev->name, txstatus);
	  }

          if (txstatus & TxUnderrun) 
	  {
            // Add 64 to the Tx FIFO threshold
            if (tp->tx_flag <  0x00300000) tp->tx_flag += 0x00020000;
            tp->stats.tx_fifo_errors++;
          }
          tp->stats.collisions += (txstatus >> 24) & 15;
          tp->stats.tx_bytes += txstatus & 0x7ff;
          tp->stats.tx_packets++;
        }

        // Free the original skb
        dev_free_skb_irq(tp->tx_skbuff[entry]);
        tp->tx_skbuff[entry] = 0;
        if (test_bit(0, &tp->tx_full)) 
	{
          // The ring is no longer full, clear tbusy
          clear_bit(0, &tp->tx_full);
          netif_resume_tx_queue(dev);
        }
        dirty_tx++;
      }

      tp->dirty_tx = dirty_tx;
    }

    // Check uncommon events with one test
    if (status & (PCIErr | PCSTimeout | RxUnderrun | RxOverflow | RxFIFOOver | TxErr | RxErr))
    {
      if (status == 0xffff) break; // Missing chip!
      rtl_error(dev, status, link_changed);
    }

    if (--boguscnt < 0) 
    {
      kprintf("%s: Too much work at interrupt, IntrStatus=0x%4.4x\n", dev->name, status);
      
      // Clear all interrupt sources
      outpw(ioaddr + IntrStatus, 0xffff);
      break;
    }
  }

  if (tp->msg_level & NETIF_MSG_INTR)
  {
    kprintf("%s: exiting interrupt, intr_status=%#4.4x\n", dev->name, inpw(ioaddr + IntrStatus));
  }
}

// Receive packet from nic

static int rtl8129_rx(struct net_device *dev)
{
  struct nic *tp = (struct nic *) dev->priv;
  long ioaddr = dev->base_addr;
  unsigned char *rx_ring = tp->rx_ring;
  unsigned short cur_rx = tp->cur_rx;

  if (tp->msg_level & NETIF_MSG_RX_STATUS)
  {
    kprintf("%s: In rtl8129_rx(), current %4.4x BufAddr %4.4x, free to %4.4x, Cmd %2.2x\n",
      dev->name, cur_rx, inpw(ioaddr + RxBufAddr),
      inpw(ioaddr + RxBufPtr), inp(ioaddr + ChipCmd));
  }

  while ((inp(ioaddr + ChipCmd) & RxBufEmpty) == 0) 
  {
    int ring_offset = cur_rx % tp->rx_buf_len;
    unsigned long rx_status = le32_to_cpu(*(unsigned long *)(rx_ring + ring_offset));
    int rx_size = rx_status >> 16;        // Includes the CRC

    if (tp->msg_level & NETIF_MSG_RX_STATUS) 
    {
      int i;
      kprintf("%s:  rtl8129_rx() status %4.4x, size %4.4x, cur %4.4x\n", dev->name, rx_status, rx_size, cur_rx);
      kprintf("%s: Frame contents ", dev->name);
      for (i = 0; i < 70; i++) printk(" %2.2x", rx_ring[ring_offset + i]);
      kprintf("\n");
    }

    if (rx_status & (RxBadSymbol | RxRunt | RxTooLong | RxCRCErr | RxBadAlign)) 
    {
      if (tp->msg_level & NETIF_MSG_RX_ERR)
      {
        kprintf("%s: Ethernet frame had errors, status %8.8x\n", dev->name, rx_status);
      }

      if (rx_status == 0xffffffff) 
      {
        kprintf("%s: Invalid receive status at ring offset %4.4x\n", dev->name, ring_offset);
        rx_status = 0;
      }

      if (rx_status & RxTooLong) 
      {
        if (tp->msg_level & NETIF_MSG_DRV)
	{
          kprintf("%s: Oversized Ethernet frame, status %4.4x!\n", dev->name, rx_status);
	}

        // The chip hangs here.
        // This should never occur, which means that we are screwed when it does
      }

      tp->stats.rx_errors++;
      if (rx_status & (RxBadSymbol | RxBadAlign)) tp->stats.rx_frame_errors++;
      if (rx_status & (RxRunt | RxTooLong)) tp->stats.rx_length_errors++;
      if (rx_status & RxCRCErr) tp->stats.rx_crc_errors++;
      
      // Reset the receiver, based on RealTek recommendation. (Bug?)
      tp->cur_rx = 0;
      outp(ioaddr + ChipCmd, CmdTxEnb);
      
      // Reset the multicast list
      set_rx_mode(dev);
      outb(ioaddr + ChipCmd, CmdRxEnb | CmdTxEnb);
    } 
    else 
    {
      // Malloc up new buffer
      // Omit the four octet CRC from the length
      struct sk_buff *skb;
      int pkt_size = rx_size - 4;

      // Allocate a common-sized skbuff if we are close
      skb = dev_alloc_skb(1400 < pkt_size && pkt_size < PKT_BUF_SZ-2 ? PKT_BUF_SZ : pkt_size + 2);
      if (skb == NULL) 
      {
        kprintf("%s: Memory squeeze, deferring packet.\n", dev->name);
        
	// We should check that some rx space is free.
        // If not, free one and mark stats->rx_dropped
        tp->stats.rx_dropped++;
        break;
      }
      
      skb->dev = dev;
      skb_reserve(skb, 2);  // 16 byte align the IP fields
      if (ring_offset + rx_size > tp->rx_buf_len) 
      {
        int semi_count = tp->rx_buf_len - ring_offset - 4;
        
	// This could presumably use two calls to copy_and_sum()?
        memcpy(skb_put(skb, semi_count), &rx_ring[ring_offset + 4], semi_count);
        memcpy(skb_put(skb, pkt_size-semi_count), rx_ring, pkt_size - semi_count);
        if (tp->msg_level & NETIF_MSG_PKTDATA) 
	{
          int i;
        
	  kprintf("%s:  Frame wrap @%d", dev->name, semi_count);
          for (i = 0; i < 16; i++) kprintf(" %2.2x", rx_ring[i]);
          kprintf("\n");
          memset(rx_ring, 0xcc, 16);
        }
      } 
      else 
      {
        eth_copy_and_sum(skb, &rx_ring[ring_offset + 4], pkt_size, 0);
        skb_put(skb, pkt_size);
      }
      skb->protocol = eth_type_trans(skb, dev);
      netif_rx(skb);
      tp->stats.rx_bytes += pkt_size;
      tp->stats.rx_packets++;
    }

    cur_rx = (cur_rx + rx_size + 4 + 3) & ~3;
    outpw(ioaddr + RxBufPtr, cur_rx - 16);
  }

  if (tp->msg_level & NETIF_MSG_RX_STATUS)
  {
    kprintf("%s: Done rtl8129_rx(), current %4.4x BufAddr %4.4x, free to %4.4x, Cmd %2.2x\n",
      dev->name, cur_rx, inpw(ioaddr + RxBufAddr),
      inpw(ioaddr + RxBufPtr), inp(ioaddr + ChipCmd));
  }

  tp->cur_rx = cur_rx;
  return 0;
}

// Error and abnormal or uncommon events handlers

static void rtl_error(struct net_device *dev, int status, int link_changed)
{
  struct nic *tp = (struct nic *) dev->priv;
  long ioaddr = dev->base_addr;

  if (tp->msg_level & NETIF_MSG_LINK)
  {
    kprintf("%s: Abnormal interrupt, status %8.8x\n", dev->name, status);
  }

  // Update the error count
  tp->stats.rx_missed_errors += inpd(ioaddr + RxMissed);
  outpd(ioaddr + RxMissed, 0);

  if (status & RxUnderrun)
  {
    // This might actually be a link change event
    if ((tp->drv_flags & HAS_LNK_CHNG) && link_changed)
    {
      // Really link-change on new chips
      int lpar = inpw(ioaddr + NWayLPAR);
      int duplex = (lpar&0x0100) || (lpar & 0x01C0) == 0x0040 || tp->duplex_lock;
      
      // Do not use MII_BMSR as that clears sticky bit
      if (inpw(ioaddr + GPPinData) & 0x0004) 
      {
        netif_link_down(dev);
      } 
      else
        netif_link_up(dev);

      if (tp->msg_level & NETIF_MSG_LINK)
      {
        kprintf("%s: Link changed, link partner %4.4x new duplex %d\n", dev->name, lpar, duplex);
      }

      tp->full_duplex = duplex;

      // Only count as errors with no link change
      status &= ~RxUnderrun;
    } 
    else 
    {
      // If this does not work, we will do rtl_hw_start
      outp(ioaddr + ChipCmd, CmdTxEnb);
      set_rx_mode(dev); // Reset the multicast list
      outb(ioaddr + ChipCmd, CmdRxEnb | CmdTxEnb);

      tp->stats.rx_errors++;
      tp->stats.rx_fifo_errors++;
    }
  }
  
  if (status & (RxOverflow | RxErr | RxFIFOOver)) tp->stats.rx_errors++;
  if (status & (PCSTimeout)) tp->stats.rx_length_errors++;
  if (status & RxFIFOOver) tp->stats.rx_fifo_errors++;
  if (status & RxOverflow) 
  {
    tp->stats.rx_over_errors++;
    tp->cur_rx = inpw(ioaddr + RxBufAddr) % tp->rx_buf_len;
    outpw(ioaddr + RxBufPtr, tp->cur_rx - 16);
  }

  if (status & PCIErr) 
  {
    unsigned long pci_cmd_status;
    pci_read_config_dword(tp->pci_dev, PCI_COMMAND, &pci_cmd_status);
    kprintf("%s: PCI Bus error %4.4x.\n", dev->name, pci_cmd_status);
  }
}

static struct net_device_stats *rtl8129_get_stats(struct net_device *dev)
{
  struct nic *tp = (struct nic *)dev->priv;
  long ioaddr = dev->base_addr;

  if (netif_running(dev)) 
  {
    tp->stats.rx_missed_errors += inpd(ioaddr + RxMissed);
    outl(ioaddr + RxMissed, 0);
  }

  return &tp->stats;
}

// Set or clear the multicast filter

static unsigned const ethernet_polynomial = 0x04c11db7U;

static inline unsigned long ether_crc(int length, unsigned char *data)
{
  int crc = -1;

  while (--length >= 0) 
  {
    unsigned char current_octet = *data++;
    int bit;
    for (bit = 0; bit < 8; bit++, current_octet >>= 1)
      crc = (crc << 1) ^
        ((crc < 0) ^ (current_octet & 1) ? ethernet_polynomial : 0);
  }

  return crc;
}

static void set_rx_mode(struct net_device *dev)
{
  struct nic *tp = (struct nic *) dev->priv;
  long ioaddr = dev->base_addr;
  unsigned long mc_filter[2];    // Multicast hash filter
  int i, rx_mode;

  if (tp->msg_level & NETIF_MSG_RXFILTER)
  {
    kprintf("%s: set_rx_mode(%4.4x) done -- Rx config %8.8x\n", dev->name, dev->flags, (int)inpd(ioaddr + RxConfig));
  }

  if (dev->flags & IFF_PROMISC)
  {
    // Unconditionally log net taps
    kprintf("%s: Promiscuous mode enabled\n", dev->name);

    rx_mode = AcceptBroadcast | AcceptMulticast | AcceptMyPhys | AcceptAllPhys;
    mc_filter[1] = mc_filter[0] = 0xffffffff;
  } 
  else if ((dev->mc_count > tp->multicast_filter_limit) || (dev->flags & IFF_ALLMULTI)) 
  {
    // Too many to filter perfectly -- accept all multicasts
    rx_mode = AcceptBroadcast | AcceptMulticast | AcceptMyPhys;
    mc_filter[1] = mc_filter[0] = 0xffffffff;
  } 
  else 
  {
    struct dev_mc_list *mclist;
    rx_mode = AcceptBroadcast | AcceptMulticast | AcceptMyPhys;
    mc_filter[1] = mc_filter[0] = 0;
    for (i = 0, mclist = dev->mc_list; mclist && i < dev->mc_count; i++, mclist = mclist->next)
    {
      set_bit(ether_crc(ETH_ALEN, mclist->dmi_addr) >> 26, mc_filter);
    }
  }
  
  // We can safely update without stopping the chip
  outpd(ioaddr + RxConfig, tp->rx_config | rx_mode);
  tp->mc_filter[0] = mc_filter[0];
  tp->mc_filter[1] = mc_filter[1];
  outpd(ioaddr + MAR0 + 0, mc_filter[0]);
  outpd(ioaddr + MAR0 + 4, mc_filter[1]);
}

#endif

void rtl8139_timer(void *arg);

// Initialize the Rx and Tx rings

static void rtl8129_init_ring(struct dev *dev)
{
  struct nic *tp = (struct nic *) dev->privdata;
  int i;

  tp->tx_full = 0;
  tp->dirty_tx = tp->cur_tx = 0;

  for (i = 0; i < NUM_TX_DESC; i++) 
  {
    tp->tx_pbuf[i] = 0;
    tp->tx_buf[i] = &tp->tx_bufs[i * TX_BUF_SIZE];
  }
}

// Start the hardware at open or resume

static void rtl_hw_start(struct dev *dev)
{
  struct nic *tp = (struct nic *) dev->privdata;
  long ioaddr = tp->iobase;
  int i;

  // Soft reset the chip
  outp(ioaddr + ChipCmd, CmdReset);

  // Check that the chip has finished the reset
  for (i = 1000; i > 0; i--)
  {
    if ((inp(ioaddr + ChipCmd) & CmdReset) == 0) break;
  }

  // Restore our idea of the MAC address
  outp(ioaddr + Cfg9346, 0xC0);
  outpd(ioaddr + MAC0 + 0, *(unsigned long *)(tp->hwaddr.addr + 0));
  outpd(ioaddr + MAC0 + 4, *(unsigned long *)(tp->hwaddr.addr + 4));

  // Hmmm, do these belong here?
  tp->cur_rx = 0;

  // Must enable Tx/Rx before setting transfer thresholds!
  outp(ioaddr + ChipCmd, CmdRxEnb | CmdTxEnb);
  outpd(ioaddr + RxConfig, tp->rx_config);

  // Check this value: the documentation contradicts ifself.  Is the
  // IFG correct with bit 28:27 zero, or with |0x03000000 ?
  outpd(ioaddr + TxConfig, (TX_DMA_BURST << 8));

  // This is check_duplex()
  if (tp->phys[0] >= 0 || (tp->flags & HAS_MII_XCVR)) 
  {
    unsigned short mii_reg5 = mdio_read(dev, tp->phys[0], 5);
    if (mii_reg5 != 0xffff)
    {
      if ((mii_reg5 & 0x0100) == 0x0100 || (mii_reg5 & 0x00C0) == 0x0040)
      {
        tp->full_duplex = 1;
      }
    }

    kprintf("%s: Setting %s%s-duplex based on auto-negotiated partner ability %4.4x\n", 
      dev->name, 
      mii_reg5 == 0 ? "" : (mii_reg5 & 0x0180) ? "100mbps " : "10mbps ",
      tp->full_duplex ? "full" : "half", mii_reg5);
  }

  if (tp->flags & HAS_MII_XCVR)
  {
    // RTL8129 chip
    outp(ioaddr + Config1, tp->full_duplex ? 0x60 : 0x20);
  }
  outp(ioaddr + Cfg9346, 0x00);

  outpd(ioaddr + RxBuf, (unsigned long) virt2phys(tp->rx_ring));

  // Start the chip's Tx and Rx process
  outpd(ioaddr + RxMissed, 0);
  //set_rx_mode(dev);
  outp(ioaddr + ChipCmd, CmdRxEnb | CmdTxEnb);
  
  // Enable all known interrupts by setting the interrupt mask
  outpw(ioaddr + IntrMask, PCIErr | PCSTimeout | RxUnderrun | RxOverflow | RxFIFOOver | TxErr | TxOK | RxErr | RxOK);
}

static int rtl8129_open(struct dev *dev)
{
  struct nic *tp = (struct nic *) dev->privdata;
  long ioaddr = tp->iobase;
  int rx_buf_len_idx;

  enable_irq(tp->irq);

  // The Rx ring allocation size is 2^N + delta, which is worst-case for
  // the kernel binary-buddy allocation.  We allocate the Tx bounce buffers
  // at the same time to use some of the otherwise wasted space.
  // The delta of +16 is required for dribble-over because the receiver does
  // not wrap when the packet terminates just beyond the end of the ring

  rx_buf_len_idx = RX_BUF_LEN_IDX;
  do 
  {
    tp->rx_buf_len = 8192 << rx_buf_len_idx;
    tp->rx_ring = kmalloc(tp->rx_buf_len + 16 + (TX_BUF_SIZE * NUM_TX_DESC));
  } while (tp->rx_ring == NULL && --rx_buf_len_idx >= 0);

  if (tp->rx_ring == NULL) return -ENOMEM;
  tp->tx_bufs = tp->rx_ring + tp->rx_buf_len + 16;

  rtl8129_init_ring(dev);
  tp->full_duplex = tp->duplex_lock;
  tp->tx_flag = (TX_FIFO_THRESH << 11) & 0x003f0000;
  tp->rx_config = (RX_FIFO_THRESH << 13) | (rx_buf_len_idx << 11) | (RX_DMA_BURST << 8);

  rtl_hw_start(dev);
  //netif_start_tx_queue(dev);

  kprintf("%s: rtl8129_open() ioaddr %#lx IRQ %d GP Pins %2.2x %s-duplex\n",
        dev->name, ioaddr, tp->irq, inp(ioaddr + GPPinData),
        tp->full_duplex ? "full" : "half");

  // Set the timer to switch to check for link beat and perhaps switch
  // to an alternate media type
  init_timer(&tp->timer, rtl8139_timer, dev);
  mod_timer(&tp->timer, ticks + 3*HZ);

  return 0;
}

static int rtl8129_close(struct dev *dev)
{
  struct nic *tp = (struct nic *) dev->privdata;
  long ioaddr = tp->iobase;
  int i;

  //netif_stop_tx_queue(dev);

  kprintf("%s: Shutting down ethercard, status was 0x%4.4x\n", dev->name, inpw(ioaddr + IntrStatus));

  // Disable interrupts by clearing the interrupt mask
  outpw(ioaddr + IntrMask, 0x0000);

  // Stop the chip's Tx and Rx DMA processes
  outp(ioaddr + ChipCmd, 0x00);

  // Update the error counts
  tp->stats.rx_missed_errors += inpd(ioaddr + RxMissed);
  outpd(ioaddr + RxMissed, 0);

  del_timer(&tp->timer);

  disable_irq(tp->irq);

  for (i = 0; i < NUM_TX_DESC; i++) 
  {
    if (tp->tx_pbuf[i]) pbuf_free(tp->tx_pbuf[i]);
    tp->tx_pbuf[i] = 0;
  }
  kfree(tp->rx_ring);
  tp->rx_ring = NULL;

  // Green! Put the chip in low-power mode
  outp(ioaddr + Cfg9346, 0xC0);
  outp(ioaddr + Config1, tp->config1 | 0x03);
  outp(ioaddr + HltClk, 'H');   // 'R' would leave the clock running

  return 0;
}

int rtl8139_transmit(struct dev *dev, struct pbuf *p)
{
  return -ENOSYS;
}

int rtl8139_ioctl(struct dev *dev, int cmd, void *args, size_t size)
{
  return -ENOSYS;
}

int rtl8139_attach(struct dev *dev, struct eth_addr *hwaddr)
{
  struct nic *np = dev->privdata;
  *hwaddr = np->hwaddr;

  return 0;
}

int rtl8139_detach(struct dev *dev)
{
  return 0;
}

void rtl8139_dpc(void *arg)
{
  struct dev *dev = (struct dev *) arg;
  struct nic *np = (struct nic *) dev->privdata;
}

int rtl8139_handler(struct context *ctxt, void *arg)
{
  struct dev *dev = (struct dev *) arg;
  struct nic *np = (struct nic *) dev->privdata;

  // Queue DPC to service interrupt
  queue_irq_dpc(&np->dpc, rtl8139_dpc, dev);

  return 0;
}

void rtl8139_timer(void *arg)
{
  struct dev *dev = (struct dev *) arg;
  struct nic *np = (struct nic *) dev->privdata;
  long ioaddr = np->iobase;
  int next_tick = 60 * HZ;
  int mii_reg5 = mdio_read(dev, np->phys[0], 5);

  if (!np->duplex_lock && mii_reg5 != 0xffff) 
  {
    int duplex = (mii_reg5 & 0x0100) || (mii_reg5 & 0x01C0) == 0x0040;
    if (np->full_duplex != duplex) 
    {
      np->full_duplex = duplex;
      
      kprintf("%s: Using %s-duplex based on MII #%d link partner ability of %4.4x\n", 
	dev->name, np->full_duplex ? "full" : "half", np->phys[0], mii_reg5);

      if (np->flags & HAS_MII_XCVR) 
      {
        outp(ioaddr + Cfg9346, 0xC0);
        outp(ioaddr + Config1, np->full_duplex ? 0x60 : 0x20);
        outp(ioaddr + Cfg9346, 0x00);
      }
    }
  }

#if 0
  // Check for bogusness
  if (inw(ioaddr + IntrStatus) & (TxOK | RxOK)) 
  {
    int status = inpw(ioaddr + IntrStatus);      // Double check
    if (status & (TxOK | RxOK) && ! dev->interrupt) 
    {
      kprintf("%s: RTL8139 Interrupt line blocked, status %x\n", dev->name, status);
      rtl8129_interrupt(dev->irq, dev, 0);
    }
  }
  if (ticks - dev->trans_start >= 2 * TX_TIMEOUT)
  {
    rtl8129_tx_timeout(dev);
  }
#endif

#define RTL_TUNE_TWISTER
#if defined(RTL_TUNE_TWISTER)
  // This is a complicated state machine to configure the "twister" for
  // impedance/echos based on the cable length.
  // All of this is magic and undocumented.

  if (np->twistie) 
  {
    switch(np->twistie) 
    {
      case 1: 
	if (inpw(ioaddr + CSCR) & CSCR_LinkOKBit) 
	{
	  // We have link beat, let us tune the twister
	  outpw(ioaddr + CSCR, CSCR_LinkDownOffCmd);
	  np->twistie = 2;
	  next_tick = HZ / 10;
	} 
	else 
	{
	  // Just put in some reasonable defaults for when beat returns
	  outpw(ioaddr + CSCR, CSCR_LinkDownCmd);
	  outpd(ioaddr + FIFOTMS, 0x20);  // Turn on cable test mode
	  outpd(ioaddr + PARA78, PARA78_default);
	  outpd(ioaddr + PARA7c, PARA7c_default);
	  np->twistie = 0;  // Bail from future actions
	}
        break;

      case 2: 
      {
	// Read how long it took to hear the echo
	int linkcase = inpw(ioaddr + CSCR) & CSCR_LinkStatusBits;
	if (linkcase == 0x7000) 
	  np->twist_row = 3;
	else if (linkcase == 0x3000) 
	  np->twist_row = 2;
	else if (linkcase == 0x1000) 
	  np->twist_row = 1;
	else 
	  np->twist_row = 0;
	
	np->twist_col = 0;
	np->twistie = 3;
	next_tick = HZ / 10;
	break;
      }

      case 3: 
	// Put out four tuning parameters, one per 100msec
	if (np->twist_col == 0) outpw(ioaddr + FIFOTMS, 0);
	outpd(ioaddr + PARA7c, param[(int) np->twist_row][(int) np->twist_col]);
	next_tick = HZ / 10;
	if (++np->twist_col >= 4) 
	{
	  // For short cables we are done. For long cables (row == 3) check for mistune
	  np->twistie = (np->twist_row == 3) ? 4 : 0;
	}
	break;

      case 4: 
	// Special case for long cables: check for mistune
	if ((inpw(ioaddr + CSCR) & CSCR_LinkStatusBits) == 0x7000) 
	{
	  np->twistie = 0;
	} 
	else 
	{
	  outpd(0xfb38de03, ioaddr + PARA7c);
	  np->twistie = 5;
	  next_tick = HZ / 10;
	}
	break;

      case 5: 
	// Retune for shorter cable (column 2)
	outpd(ioaddr + FIFOTMS, 0x20);
	outpd(ioaddr + PARA78, PARA78_default);
	outpd(ioaddr + PARA7c, PARA7c_default);
	outpd(ioaddr + FIFOTMS, 0x00);
	np->twist_row = 2;
	np->twist_col = 0;
	np->twistie = 3;
	next_tick = HZ / 10;
	break;
    }
  }
#endif

  if (np->flags & HAS_MII_XCVR)
    kprintf("%s: Media selection tick, GP pins %2.2x\n", dev->name, inp(ioaddr + GPPinData));
  else
    kprintf("%s: Media selection tick, Link partner %4.4x\n", dev->name, inpw(ioaddr + NWayLPAR));

  kprintf("%s:  Other registers are IntMask %4.4x IntStatus %4.4x RxStatus %4.4x\n",
        dev->name, inpw(ioaddr + IntrMask), inpw(ioaddr + IntrStatus), (int) inpd(ioaddr + RxEarlyStatus));

  kprintf("%s:  Chip config %2.2x %2.2x\n", dev->name, inp(ioaddr + Config0), inp(ioaddr + Config1));

  mod_timer(&np->timer, ticks + next_tick);
}

struct driver rtl8139_driver =
{
  "rtl8139",
  DEV_TYPE_PACKET,
  rtl8139_ioctl,
  NULL,
  NULL,
  rtl8139_attach,
  rtl8139_detach,
  rtl8139_transmit
};

int __declspec(dllexport) install(struct unit *unit, char *opts)
{
  int chip_idx;
  unsigned short ioaddr;
  unsigned short irq;
  struct dev *dev;
  struct nic *np;
  int i;
  int config1;

  // Determine NIC type
  chip_idx = 0;
  while (chip_tbl[chip_idx].vendorname != NULL)
  {
    if ((unit->unitcode & chip_tbl[chip_idx].unitmask) == chip_tbl[chip_idx].unitcode &&
        (unit->subunitcode & chip_tbl[chip_idx].subsystemmask) == chip_tbl[chip_idx].subsystemcode &&
        (unit->revision & chip_tbl[chip_idx].revisionmask) == chip_tbl[chip_idx].revisioncode)
      break;

    chip_idx++;
  }

  if (chip_tbl[chip_idx].vendorname == NULL) return -EIO;

  unit->vendorname = chip_tbl[chip_idx].vendorname;
  unit->productname = chip_tbl[chip_idx].productname;

  // Get NIC PCI configuration
  ioaddr = (unsigned short) get_unit_iobase(unit);
  irq = irq = (unsigned short) get_unit_irq(unit);

  // Allocate private memory (must be 16 byte aligned)
  np = kmalloc(sizeof(struct nic));
  if (np == NULL) return -ENOMEM;
  memset(np, 0, sizeof(struct nic));

  // Enable bus mastering
  pci_enable_busmastering(unit);

  // Create new device
  np->devno = dev_make("eth#", &rtl8139_driver, unit, np);
  if (np->devno == NODEV) return -ENODEV;
  dev = device(np->devno);

  init_dpc(&np->dpc);
  register_interrupt(&np->intr, IRQ2INTR(irq), rtl8139_handler, dev);

  // Bring the chip out of low-power mode
  config1 = inp(ioaddr + Config1);
  if (chip_tbl[chip_idx].flags & HAS_MII_XCVR)
  {
    // RTL8129 chip
    outp(ioaddr + Config1, config1 & ~0x03);
  }

  {
    int addr_len = read_eeprom(ioaddr, 0, 8) == 0x8129 ? 8 : 6;
    for (i = 0; i < 3; i++)
    {
      ((unsigned short *)(&np->hwaddr))[i] = read_eeprom(ioaddr, i + 7, addr_len);
    }
  }

  kprintf("%s: %s iobase 0x%x irq %d hwaddr %la\n", np->dev->name, unit->productname, ioaddr, irq, &np->hwaddr);

  np->dev = dev;
  np->iobase = ioaddr;
  np->irq = irq;
  np->chip_id = chip_idx;
  np->flags = chip_tbl[chip_idx].flags;
  np->max_interrupt_work = max_interrupt_work;
  np->multicast_filter_limit = multicast_filter_limit;
  np->config1 = 0;

  // Find the connected MII xcvrs.
  if (np->flags & HAS_MII_XCVR) 
  {
    int phy, phy_idx = 0;

    for (phy = 0; phy < 32 && phy_idx < sizeof(np->phys); phy++) 
    {
      int mii_status = mdio_read(dev, phy, 1);
      if (mii_status != 0xffff && mii_status != 0x0000) 
      {
        np->phys[phy_idx++] = phy;
        np->advertising = mdio_read(dev, phy, 4);
        kprintf("%s: MII transceiver %d status 0x%4.4x advertising %4.4x\n", dev->name, phy, mii_status, np->advertising);
      }
    }

    if (phy_idx == 0) 
    {
      kprintf("%s: No MII transceivers found!  Assuming SYM transceiver\n", dev->name);
      np->phys[0] = 32;
    }
  } 
  else
    np->phys[0] = 32;

  // Put the chip into low-power mode
  outp(ioaddr + Cfg9346, 0xC0);
  if (np->flags & HAS_MII_XCVR)
  {
    // RTL8129 chip
    outp(ioaddr + Config1, 0x03);
  }

  outp(ioaddr + HltClk, 'H');   // 'R' would leave the clock running

  // Set options
  if (opts) 
  {
    np->full_duplex = get_num_option(opts, "fullduplex", 0);
    np->link_speed = get_num_option(opts, "linkspeed", 0);
  }

  if (np->full_duplex) 
  {
    // Changing the MII-advertised media might prevent re-connection
    np->duplex_lock = 1;

    kprintf("%s: Media type forced to Full Duplex\n", dev->name);    
  }

  if (np->link_speed)
  {
    np->medialock = 1;

    kprintf("%s: Forcing %dMBits/s %s-duplex operation\n", dev->name, np->link_speed, np->full_duplex ? "full" : "half");

    mdio_write(dev, np->phys[0], 0,
           ((np->link_speed == 100) ? 0x2000 : 0) |  // 100mbps?
           (np->full_duplex ? 0x0100 : 0));  // Full duplex?
  }

  return rtl8129_open(dev);
}

int __stdcall start(hmodule_t hmod, int reason, void *reserved2)
{
  return 1;
}
