//
// sis900.c
//
// SiS 900/7016 PCI Fast Ethernet driver
//
// Copyright (C) 2002 Michael Ringgaard. All rights reserved.
// Copyright (C) 1999 Silicon Integrated System Corporation.
//
// Modified from the driver which is originally written by Donald Becker.
//
// This software may be used and distributed according to the terms
// of the GNU General Public License (GPL), incorporated herein by reference.
// Drivers based on this skeleton fall under the GPL and must retain
// the authorship (implicit copyright) notice.
//

#include <os/krnl.h>
#include <bitops.h>

#include "sis900.h"

static int max_interrupt_work = 40;
static int multicast_filter_limit = 128;

// Time in ticks before concluding the transmitter is hung

#define TX_TIMEOUT  (4*HZ)

// SiS 900 is capable of 32 bits BM DMA

#define SIS900_DMA_MASK 0xffffffff

enum  {
  SIS_900 = 0,
  SIS_7016
};

static struct board board_tbl[] =  {
  {"SiS", "SiS 900 PCI Fast Ethernet", BUSTYPE_PCI, PCI_UNITCODE(0x1039, 0x0900), 0xffffffff, 0},
  {"SiS", "SiS 7016 PCI Fast Ethernet", BUSTYPE_PCI, PCI_UNITCODE(0x1039, 0x7016), 0xffffffff, 0},
  {0,}
};

#define HOME 0x0001
#define LAN  0x0002
#define MIX  0x0003

struct mii_chip_info {
  const char *name;
  unsigned short phy_id0;
  unsigned short phy_id1;
  unsigned char  phy_types;
};

static struct mii_chip_info mii_chip_table[] = {
  {"SiS 900 Internal MII PHY", 0x001d, 0x8000, LAN},
  {"SiS 7014 Physical Layer Solution", 0x0016, 0xf830, LAN},
  {"AMD 79C901 10BASE-T PHY", 0x0000, 0x6B70, LAN},
  {"AMD 79C901 HomePNA PHY", 0x0000, 0x6B90, HOME},
  {"ICS LAN PHY", 0x0015, 0xF440, LAN},
  {"NS 83851 PHY", 0x2000, 0x5C20, MIX},
  {"Realtek RTL8201 PHY", 0x0000, 0x8200, LAN},
  {0,},
};

struct mii_phy {
  struct mii_phy *next;
  int phy_addr;
  unsigned short phy_id0;
  unsigned short phy_id1;
  unsigned short status;
  unsigned char phy_types;
};

typedef struct bufferdesc {
  unsigned long link;
  unsigned long cmdsts;
  unsigned long bufptr;
};

struct sis900_private {
  dev_t devno;                          // Device number
  struct dev *dev;                      // Device block

  unsigned long iobase;                 // Configured I/O base
  unsigned long irq;                    // Configured IRQ

  struct interrupt intr;                // Interrupt object for driver
  struct dpc dpc;                       // DPC for driver
  struct timer timer;                   // Link status detection timer

  struct eth_addr hwaddr;               // MAC address for NIC

  struct stats_nic stats;

  struct mii_phy *mii;
  struct mii_phy *first_mii;            // Record the first mii structure
  unsigned int cur_phy;

  unsigned char autong_complete;        // Auto-negotiate complete 
  unsigned char carrier_ok;
  struct event link_up;

  struct sem tx_sem;                    // Semaphore for Tx ring not full
  unsigned int cur_rx, dirty_rx;        // Producer/comsumer pointers for Tx/Rx ring
  unsigned int cur_tx, dirty_tx;

  // The saved address of a sent/receive-in-place packet buffer

  struct pbuf *tx_pbuf[NUM_TX_DESC];
  struct pbuf *rx_pbuf[NUM_RX_DESC];
  struct bufferdesc *tx_ring;
  struct bufferdesc *rx_ring;

  unsigned long tx_ring_dma;
  unsigned long rx_ring_dma;

  unsigned int tx_full;                 // The Tx queue is full
};

static unsigned short read_eeprom(long ioaddr, int location);
static unsigned short mdio_read(struct dev *dev, int phy_id, int location);
static void mdio_write(struct dev *dev, int phy_id, int location, int val);
static unsigned short sis900_default_phy(struct dev *dev);
static unsigned short sis900_reset_phy(struct dev *dev, int phy_addr);
static void sis900_check_mode(struct dev *dev, struct mii_phy *mii_phy);
static void sis900_set_mode(long ioaddr, int speed, int duplex);
static void sis900_auto_negotiate(struct dev *dev, int phy_addr);
static void sis900_init_rxfilter (struct dev *dev);
static void sis900_timer(void *arg);
static void sis900_init_tx_ring(struct dev *dev);
static void sis900_init_rx_ring(struct dev *dev);
static void sis630_set_eq(struct dev *dev);
static void sis900_reset(struct dev *dev);
static int sis900_set_rx_mode(struct dev *dev);
static int sis900_rx(struct dev *dev);
static void sis900_finish_xmit(struct dev *dev);
static void sis900_read_mode(struct dev *dev, int *speed, int *duplex);

//
// sis900_get_mac_addr - Get MAC address for stand alone SiS900 model
//
// Older SiS900 and friends, use EEPROM to store MAC address.
// MAC address is read from read_eeprom() into sp->hwaddr.
//

static sis900_get_mac_addr(struct dev *dev) {
  struct sis900_private *sp = dev->privdata;
  long ioaddr = sp->iobase;
  unsigned short signature;
  int i;

  // Check to see if we have sane EEPROM
  signature = (unsigned short) read_eeprom(ioaddr, EEPROMSignature);    
  if (signature == 0xffff || signature == 0x0000) {
    kprintf(KERN_ERR "%s: Error EEPROM read %x\n", dev->name, signature);
    return 0;
  }

  // Get MAC address from EEPROM
  for (i = 0; i < 3; i++) {
    ((unsigned short *)(&sp->hwaddr))[i] = read_eeprom(ioaddr, i + EEPROMMACAddr);
  }

  return 1;
}

//
// sis630e_get_mac_addr - Get MAC address for SiS630E model
//
// SiS630E model, use APC CMOS RAM to store MAC address.
// APC CMOS RAM is accessed through ISA bridge.
// MAC address is read into sp->hwaddr.
//

static int sis630e_get_mac_addr(struct dev *dev) {
  struct sis900_private *sp = dev->privdata;
  struct unit *isa_bridge = NULL;
  unsigned char reg;
  int i;

  isa_bridge = lookup_unit(NULL, PCI_UNITCODE(0x1039, 0x0008), PCI_ID_ANY);
  if (isa_bridge == NULL) {
    kprintf("%s: Can not find ISA bridge\n", dev->name);
    return 0;
  }

  pci_read_buffer(isa_bridge, 0x48, &reg, 1);
  reg |= 0x40;
  pci_write_buffer(isa_bridge, 0x48, &reg, 1);

  for (i = 0; i < 6; i++) {
    outp(0x70, 0x09 + i);
    ((unsigned char *)(&sp->hwaddr))[i] = inp(0x71);
  }

  reg &= ~0x40;
  pci_write_buffer(isa_bridge, 0x48, &reg, 1);

  return 1;
}

//
// sis635_get_mac_addr - Get MAC address for SIS635 model
//
// SiS635 model, set MAC Reload Bit to load Mac address from APC
// to rfdr. rfdr is accessed through rfcr. MAC address is read into 
// sp->hwaddr.
//

static int sis635_get_mac_addr(struct dev *dev) {
  struct sis900_private *sp = dev->privdata;
  long ioaddr = sp->iobase;
  unsigned long rfcr_save;
  unsigned long i;

  rfcr_save = inpd(rfcr + ioaddr);

  outpd(ioaddr + cr, rfcr_save | RELOAD);
  outpd(ioaddr + cr, 0);

  // Disable packet filtering before setting filter
  outpd(rfcr + ioaddr, rfcr_save & ~RFEN);

  // Load MAC addr to filter data register
  for (i = 0 ; i < 3 ; i++) {
    outpd(ioaddr + rfcr, (i << RFADDR_shift));
    *(((unsigned short *) &sp->hwaddr) + i) = inpw(ioaddr + rfdr);
  }

  // Enable packet filitering
  outpd(rfcr + ioaddr, rfcr_save | RFEN);

  return 1;
}

//
// sis96x_get_mac_addr: - Get MAC address for SiS962 or SiS963 model
//
// SiS962 or SiS963 model, use EEPROM to store MAC address. And EEPROM 
// is shared by LAN and 1394. When access EEPROM, send EEREQ signal to 
// hardware first and wait for EEGNT. If EEGNT is ON, EEPROM is permitted 
// to be access by LAN, otherwise is not. After MAC address is read from 
// EEPROM, send EEDONE signal to refuse EEPROM access by LAN. 
// The EEPROM map of SiS962 or SiS963 is different to SiS900. 
// The signature field in SiS962 or SiS963 spec is meaningless. 
// MAC address is read into sp->hwaddr.
//

static int sis96x_get_mac_addr(struct dev *dev) {
  struct sis900_private *sp = dev->privdata;
  long ioaddr = sp->iobase;
  long ee_addr = ioaddr + mear;
  unsigned long waittime = 0;
  int i;
  
  outpd(ee_addr, EEREQ);
  while (waittime < 2000) {
    if (inpd(ee_addr) & EEGNT) {
      // Fet MAC address from EEPROM
      for (i = 0; i < 3; i++) {
        ((unsigned short *)(&sp->hwaddr))[i] = read_eeprom(ioaddr, i + EEPROMMACAddr);
      }

      outpd(ee_addr, EEDONE);
      return 1;
    } else {
      udelay(1);
      waittime++;
    }
  }

  outpd(ee_addr, EEDONE);
  return 0;
}

//
// sis900_mii_probe - Probe MII PHY for sis900
// 
// Search for total of 32 possible mii phy addresses.
// Identify and set current phy if found one,
// return error if it failed to found.
//

static int sis900_mii_probe(struct dev *dev) {
  struct sis900_private *sp = dev->privdata;
  unsigned short poll_bit = MII_STAT_LINK, status = 0;
  unsigned int timeout = get_ticks() + 5 * HZ;
  int phy_addr;

  sp->mii = NULL;

  // Search for total of 32 possible mii phy addresses
  for (phy_addr = 0; phy_addr < 32; phy_addr++) {
    struct mii_phy *mii_phy = NULL;
    unsigned short mii_status;
    int i;

    mii_phy = NULL;
    for (i = 0; i < 2; i++) mii_status = mdio_read(dev, phy_addr, MII_STATUS);

    if (mii_status == 0xffff || mii_status == 0x7fff || mii_status == 0x0000) continue;
    
    mii_phy = kmalloc(sizeof(struct mii_phy));
    if (!mii_phy) return -ENOMEM;
    
    mii_phy->phy_id0 = mdio_read(dev, phy_addr, MII_PHY_ID0);
    mii_phy->phy_id1 = mdio_read(dev, phy_addr, MII_PHY_ID1);    
    mii_phy->phy_addr = phy_addr;
    mii_phy->status = mii_status;
    mii_phy->next = sp->mii;
    sp->mii = mii_phy;
    sp->first_mii = mii_phy;

    for (i = 0; mii_chip_table[i].phy_id1; i++) {
      if ((mii_phy->phy_id0 == mii_chip_table[i].phy_id0 ) && ((mii_phy->phy_id1 & 0xFFF0) == mii_chip_table[i].phy_id1)) {
        mii_phy->phy_types = mii_chip_table[i].phy_types;
        if (mii_chip_table[i].phy_types == MIX) {
          mii_phy->phy_types = (mii_status & (MII_STAT_CAN_TX_FDX | MII_STAT_CAN_TX)) ? LAN : HOME;
        }

        kprintf("%s: %s transceiver found at address %d.\n", dev->name, mii_chip_table[i].name, phy_addr);
        break;
      }
    }
      
    if (!mii_chip_table[i].phy_id1) {
      kprintf("%s: Unknown PHY transceiver found at address %d.\n", dev->name, phy_addr);
    }
  }
  
  if (sp->mii == NULL) {
    kprintf("%s: No MII transceivers found!\n", dev->name);
    return 0;
  }

  // Select default PHY for mac
  sp->mii = NULL;
  sis900_default_phy(dev);

  // Reset phy if default phy is internal sis900
  if ((sp->mii->phy_id0 == 0x001D) && ((sp->mii->phy_id1 & 0xFFF0) == 0x8000)) {
    status = sis900_reset_phy(dev, sp->cur_phy);
  }
        
  // Workaround for ICS1893 PHY
  if ((sp->mii->phy_id0 == 0x0015) && ((sp->mii->phy_id1 & 0xFFF0) == 0xF440)) {
    mdio_write(dev, sp->cur_phy, 0x0018, 0xD200);
  }

  if (status & MII_STAT_LINK) {
    while (poll_bit) {
      poll_bit ^= (mdio_read(dev, sp->cur_phy, MII_STATUS) & poll_bit);
      if (get_ticks() >= timeout) {
        kprintf(KERN_WARNING "%s: reset phy and link down now\n", dev->name);
        return -ETIMEOUT;
      }
    }
  }

  if (dev->unit->revision == SIS630E_900_REV) {
    // SiS 630E has some bugs on default value of PHY registers
    mdio_write(dev, sp->cur_phy, MII_ANADV, 0x05e1);
    mdio_write(dev, sp->cur_phy, MII_CONFIG1, 0x22);
    mdio_write(dev, sp->cur_phy, MII_CONFIG2, 0xff00);
    mdio_write(dev, sp->cur_phy, MII_MASK, 0xffc0);
  }

  if (sp->mii->status & MII_STAT_LINK) {
    sp->carrier_ok = 1;
  } else {
    sp->carrier_ok = 0;
  }

  return 1;
}

//
// sis900_default_phy - Select default PHY for sis900 mac.
//
// Select first detected PHY with link as default.
// If no one is link on, select PHY whose types is HOME as default.
// If HOME doesn't exist, select LAN.
//

static unsigned short sis900_default_phy(struct dev *dev) {
  struct sis900_private *sp = dev->privdata;
  struct mii_phy *phy = NULL, *phy_home = NULL, *default_phy = NULL;
  unsigned short status;

  for (phy = sp->first_mii; phy; phy = phy->next) {
    status = mdio_read(dev, phy->phy_addr, MII_STATUS);
    status = mdio_read(dev, phy->phy_addr, MII_STATUS);

    // Link ON & Not select default PHY
    if ((status & MII_STAT_LINK) && !(default_phy)) {
      default_phy = phy;
    } else {
      status = mdio_read(dev, phy->phy_addr, MII_CONTROL);
      mdio_write(dev, phy->phy_addr, MII_CONTROL, status | MII_CNTL_AUTO | MII_CNTL_ISOLATE);
      if (phy->phy_types == HOME) phy_home = phy;
     }
  }

  if ((!default_phy) && phy_home) {
    default_phy = phy_home;
  } else if (!default_phy) {
    default_phy = sp->first_mii;
  }

  if (sp->mii != default_phy) {
    sp->mii = default_phy;
    sp->cur_phy = default_phy->phy_addr;
    kprintf("%s: Using transceiver found at address %d as default\n", dev->name, sp->cur_phy);
  }
  
  status = mdio_read(dev, sp->cur_phy, MII_CONTROL);
  status &= (~MII_CNTL_ISOLATE);

  mdio_write(dev, sp->cur_phy, MII_CONTROL, status);  
  status = mdio_read(dev, sp->cur_phy, MII_STATUS);
  status = mdio_read(dev, sp->cur_phy, MII_STATUS);

  return status;  
}

//
// sis900_set_capability - set the media capability of network adapter.
//
// Set the media capability of network adapter according to
// mii status register. It's necessary before auto-negotiate.
//
 
static void sis900_set_capability(struct dev *dev , struct mii_phy *phy) {
  unsigned short cap;
  unsigned short status;
  
  status = mdio_read(dev, phy->phy_addr, MII_STATUS);
  status = mdio_read(dev, phy->phy_addr, MII_STATUS);
  
  cap = MII_NWAY_CSMA_CD |
    ((phy->status & MII_STAT_CAN_TX_FDX)? MII_NWAY_TX_FDX : 0) |
    ((phy->status & MII_STAT_CAN_TX)    ? MII_NWAY_TX : 0) |
    ((phy->status & MII_STAT_CAN_T_FDX) ? MII_NWAY_T_FDX : 0)|
    ((phy->status & MII_STAT_CAN_T)     ? MII_NWAY_T : 0);

  mdio_write(dev, phy->phy_addr, MII_ANADV, cap);
}

// Delay between EEPROM clock transitions.

#define eeprom_delay()  inpd(ee_addr)

//
// read_eeprom - Read Serial EEPROM
//
// Read Serial EEPROM through EEPROM Access Register.
// Note that location is in word (16 bits) unit
//

static unsigned short read_eeprom(long ioaddr, int location) {
  int i;
  unsigned short retval = 0;
  long ee_addr = ioaddr + mear;
  unsigned long read_cmd = location | EEread;

  outpd(ee_addr, 0);
  eeprom_delay();
  outpd(ee_addr, EECS);
  eeprom_delay();

  // Shift the read command (9) bits out.
  for (i = 8; i >= 0; i--) {
    unsigned long dataval = (read_cmd & (1 << i)) ? EEDI | EECS : EECS;
    outpd(ee_addr, dataval);
    eeprom_delay();
    outpd(ee_addr, dataval | EECLK);
    eeprom_delay();
  }
  outpd(ee_addr, EECS);
  eeprom_delay();

  // Read the 16-bits data in
  for (i = 16; i > 0; i--) {
    outpd(ee_addr, EECS);
    eeprom_delay();
    outpd(ee_addr, EECS | EECLK);
    eeprom_delay();
    retval = (retval << 1) | ((inpd(ee_addr) & EEDO) ? 1 : 0);
    eeprom_delay();
  }

  // Terminate the EEPROM access.
  outpd(ee_addr, 0);
  eeprom_delay();

  return retval;
}

// Read and write the MII management registers using software-generated
// serial MDIO protocol. Note that the command bits and data bits are
// send out seperately

#define mdio_delay()    inpd(mdio_addr)

static void mdio_idle(long mdio_addr) {
  outpd(mdio_addr, MDIO | MDDIR);
  mdio_delay();
  outpd(mdio_addr, MDIO | MDDIR | MDC);
}

// Syncronize the MII management interface by shifting 32 one bits out.

static void mdio_reset(long mdio_addr) {
  int i;

  for (i = 31; i >= 0; i--) {
    outpd(mdio_addr, MDDIR | MDIO);
    mdio_delay();
    outpd(mdio_addr, MDDIR | MDIO | MDC);
    mdio_delay();
  }
}

//
// mdio_read - read MII PHY register
//
// Read MII registers through MDIO and MDC
// using MDIO management frame structure and protocol(defined by ISO/IEC).
// Please see SiS7014 or ICS spec
//

static unsigned short mdio_read(struct dev *dev, int phy_id, int location) {
  struct sis900_private *sp = dev->privdata;
  long mdio_addr = sp->iobase + mear;
  int mii_cmd = MIIread | (phy_id << MIIpmdShift) | (location << MIIregShift);
  unsigned short retval = 0;
  int i;

  mdio_reset(mdio_addr);
  mdio_idle(mdio_addr);

  for (i = 15; i >= 0; i--) {
    int dataval = (mii_cmd & (1 << i)) ? MDDIR | MDIO : MDDIR;
    outpd(mdio_addr, dataval);
    mdio_delay();
    outpd(mdio_addr, dataval | MDC);
    mdio_delay();
  }

  // Read the 16 data bits.
  for (i = 16; i > 0; i--) {
    outpd(mdio_addr, 0);
    mdio_delay();
    retval = (retval << 1) | ((inpd(mdio_addr) & MDIO) ? 1 : 0);
    outpd(mdio_addr, MDC);
    mdio_delay();
  }
  outpd(mdio_addr, 0x00);

  return retval;
}

//
// mdio_write - write MII PHY register
//
// Write MII registers with @value through MDIO and MDC
// using MDIO management frame structure and protocol(defined by ISO/IEC)
// please see SiS7014 or ICS spec
//

static void mdio_write(struct dev *dev, int phy_id, int location, int value) {
  struct sis900_private *sp = dev->privdata;
  long mdio_addr = sp->iobase + mear;
  int mii_cmd = MIIwrite | (phy_id << MIIpmdShift) | (location << MIIregShift);
  int i;

  mdio_reset(mdio_addr);
  mdio_idle(mdio_addr);

  // Shift the command bits out.
  for (i = 15; i >= 0; i--) {
    int dataval = (mii_cmd & (1 << i)) ? MDDIR | MDIO : MDDIR;
    outp(mdio_addr, dataval);
    mdio_delay();
    outp(mdio_addr, dataval | MDC);
    mdio_delay();
  }
  mdio_delay();

  // Shift the value bits out.
  for (i = 15; i >= 0; i--) {
    int dataval = (value & (1 << i)) ? MDDIR | MDIO : MDDIR;
    outpd(mdio_addr, dataval);
    mdio_delay();
    outpd(mdio_addr, dataval | MDC);
    mdio_delay();
  }
  mdio_delay();

  // Clear out extra bits.
  for (i = 2; i > 0; i--) {
    outp(mdio_addr, 0);
    mdio_delay();
    outp(mdio_addr, MDC);
    mdio_delay();
  }
  outpd(mdio_addr, 0x00);
}

//
// sis900_reset_phy - reset sis900 mii phy.
//
// Some specific phy can't work properly without reset.
// This function will be called during initialization and
// link status change from ON to DOWN.
//

static unsigned short sis900_reset_phy(struct dev *dev, int phy_addr) {
  int i = 0;
  unsigned short status;

  while (i++ < 2) status = mdio_read(dev, phy_addr, MII_STATUS);
  mdio_write(dev, phy_addr, MII_CONTROL, MII_CNTL_RESET);
  
  return status;
}

//
// sis900_open - open sis900 device
//
// Do some initialization and start net interface.
// enable interrupts and set sis900 timer.
//

static int sis900_open(struct dev *dev) {
  struct sis900_private *sp = dev->privdata;
  long ioaddr = sp->iobase;

  // Soft reset the chip.
  sis900_reset(dev);

  // Equalizer workaround Rule
  sis630_set_eq(dev);

  sis900_init_rxfilter(dev);

  sis900_init_tx_ring(dev);
  sis900_init_rx_ring(dev);

  sis900_set_rx_mode(dev);

  // Workaround for EDB
  sis900_set_mode(ioaddr, HW_SPEED_10_MBPS, FDX_CAPABLE_HALF_SELECTED);

  // Enable all known interrupts by setting the interrupt mask.
  outpd(ioaddr + imr, (RxSOVR | RxORN | RxERR | RxOK | TxURN | TxERR | TxIDLE));
  outpd(ioaddr + cr, RxENA | inpd(ioaddr + cr));
  outpd(ioaddr + ier, IE);

  sis900_check_mode(dev, sp->mii);

  // Set the timer to switch to check for link beat and perhaps switch
  // to an alternate media type.
  init_timer(&sp->timer, sis900_timer, dev);
  mod_timer(&sp->timer, get_ticks() + HZ);

  return 0;
}

//
// sis900_init_rxfilter - Initialize the Rx filter
//
// Set receive filter address to our MAC address
// and enable packet filtering.
//

static void sis900_init_rxfilter(struct dev *dev) {
  struct sis900_private *sp = dev->privdata;
  long ioaddr = sp->iobase;
  unsigned long rfcr_save;
  unsigned long i;

  rfcr_save = inpd(rfcr + ioaddr);

  // Disable packet filtering before setting filter
  outpd(rfcr + ioaddr, rfcr_save & ~RFEN);

  // Load MAC addr to filter data register
  for (i = 0 ; i < 3 ; i++) {
    unsigned long w;

    w = (unsigned long) *((unsigned short *)(&sp->hwaddr) + i);
    outpd(ioaddr + rfcr, (i << RFADDR_shift));
    outpd(ioaddr + rfdr, w);

    //kprintf("%s: Receive Filter Address[%d]=%x\n", dev->name, i, inpd(ioaddr + rfdr));
  }

  // Enable packet filitering
  outpd(rfcr + ioaddr, rfcr_save | RFEN);
}

//
// sis900_init_tx_ring - Initialize the Tx descriptor ring
//
// Initialize the Tx descriptor ring, 
//

static void sis900_init_tx_ring(struct dev *dev) {
  struct sis900_private *sp = dev->privdata;
  long ioaddr = sp->iobase;
  int i;

  sp->tx_full = 0;
  sp->dirty_tx = sp->cur_tx = 0;

  for (i = 0; i < NUM_TX_DESC; i++) {
    sp->tx_pbuf[i] = NULL;

    sp->tx_ring[i].link = sp->tx_ring_dma + ((i + 1) % NUM_TX_DESC) * sizeof(struct bufferdesc);
    sp->tx_ring[i].cmdsts = 0;
    sp->tx_ring[i].bufptr = 0;
  }

  // Load Transmit Descriptor Register
  outpd(ioaddr + txdp, sp->tx_ring_dma);
  //kprintf("%s: TX descriptor register loaded with: %8.8x\n", dev->name, inpd(ioaddr + txdp));
}

//
// sis900_init_rx_ring - Initialize the Rx descriptor ring
//
// Initialize the Rx descriptor ring, 
// and pre-allocate recevie buffers (socket buffer)
//

static void sis900_init_rx_ring(struct dev *dev) {
  struct sis900_private *sp = dev->privdata;
  long ioaddr = sp->iobase;
  int i;

  sp->cur_rx = 0;
  sp->dirty_rx = 0;

  // Init RX descriptor
  for (i = 0; i < NUM_RX_DESC; i++) {
    sp->rx_pbuf[i] = NULL;

    sp->rx_ring[i].link = sp->rx_ring_dma + ((i + 1) % NUM_RX_DESC) * sizeof(struct bufferdesc);
    sp->rx_ring[i].cmdsts = 0;
    sp->rx_ring[i].bufptr = 0;
  }

  // Allocate packet buffers
  for (i = 0; i < NUM_RX_DESC; i++) {
    struct pbuf *p;

    p = pbuf_alloc(PBUF_RAW, RX_BUF_SIZE, PBUF_RW);
    if (!p) {
      // Not enough memory for pbuf, this makes a "hole"
      // on the buffer ring, it is not clear how the
      // hardware will react to this kind of degenerated
      // buffer
      break;
    }
    sp->rx_pbuf[i] = p;
    sp->rx_ring[i].cmdsts = RX_BUF_SIZE;
    sp->rx_ring[i].bufptr = virt2phys(p->payload);
  }
  sp->dirty_rx = (unsigned int) (i - NUM_RX_DESC);

  // Load Receive Descriptor Register
  outpd(ioaddr + rxdp, sp->rx_ring_dma);
  //kprintf("%s: RX descriptor register loaded with: %8.8x\n", dev->name, inpd(ioaddr + rxdp));
}

//
// sis630_set_eq - set phy equalizer value for 630 LAN
//
// 630E equalizer workaround rule (Cyrus Huang 08/15)
// PHY register 14h(Test)
// Bit 14: 0 -- Automatically dectect (default)
//   1 -- Manually set Equalizer filter
// Bit 13: 0 -- (Default)
//   1 -- Speed up convergence of equalizer setting
// Bit 9 : 0 -- (Default)
//   1 -- Disable Baseline Wander
// Bit 3~7   -- Equalizer filter setting
// Link ON: Set Bit 9, 13 to 1, Bit 14 to 0
// Then calculate equalizer value
// Then set equalizer value, and set Bit 14 to 1, Bit 9 to 0
// Link Off:Set Bit 13 to 1, Bit 14 to 0
// Calculate Equalizer value:
// When Link is ON and Bit 14 is 0, SIS900PHY will auto-dectect proper equalizer value.
// When the equalizer is stable, this value is not a fixed value. It will be within
// a small range(eg. 7~9). Then we get a minimum and a maximum value(eg. min=7, max=9)
// 0 <= max <= 4  --> set equalizer to max
// 5 <= max <= 14 --> set equalizer to max + 1 or set equalizer to max + 2 if max == min
// max >= 15      --> set equalizer to max + 5 or set equalizer to max + 6 if max == min

static void sis630_set_eq(struct dev *dev) {
  struct sis900_private *sp = dev->privdata;
  unsigned short reg14h, eq_value = 0, max_value = 0, min_value = 0;
  int host_bridge_rev = 0;
  int revision;
  int i, maxcount = 10;
  struct unit *host_bridge;

  revision = dev->unit->revision;
  if (!(revision == SIS630E_900_REV || revision == SIS630EA1_900_REV ||
        revision == SIS630A_900_REV || revision ==  SIS630ET_900_REV))
    return;

  host_bridge = lookup_unit(NULL, PCI_UNITCODE(0x1039, 0x0630), PCI_ID_ANY);
  if (host_bridge) host_bridge_rev = host_bridge->revision;

  if (sp->carrier_ok) {
    reg14h = mdio_read(dev, sp->cur_phy, MII_RESV);
    mdio_write(dev, sp->cur_phy, MII_RESV, (0x2200 | reg14h) & 0xBFFF);
    for (i = 0; i < maxcount; i++) {
      eq_value = (0x00F8 & mdio_read(dev, sp->cur_phy, MII_RESV)) >> 3;
      if (i == 0) max_value = min_value = eq_value;
      max_value = (eq_value > max_value) ? eq_value : max_value;
      min_value = (eq_value < min_value) ? eq_value : min_value;
    }

    // 630E rule to determine the equalizer value
    if (revision == SIS630E_900_REV || revision == SIS630EA1_900_REV || revision == SIS630ET_900_REV) {
      if (max_value < 5) {
        eq_value = max_value;
      } else if (max_value >= 5 && max_value < 15) {
        eq_value = (max_value == min_value) ? max_value + 2 : max_value + 1;
      } else if (max_value >= 15) {
        eq_value = (max_value == min_value) ? max_value + 6 : max_value + 5;
      }
    }

    // 630B0&B1 rule to determine the equalizer value
    if (revision == SIS630A_900_REV && (host_bridge_rev == SIS630B0 || host_bridge_rev == SIS630B1)) {
      if (max_value == 0) {
        eq_value = 3;
      } else {
        eq_value = (max_value + min_value + 1) / 2;
      }
    }

    // Write equalizer value and setting
    reg14h = mdio_read(dev, sp->cur_phy, MII_RESV);
    reg14h = (reg14h & 0xFF07) | ((eq_value << 3) & 0x00F8);
    reg14h = (reg14h | 0x6000) & 0xFDFF;
    mdio_write(dev, sp->cur_phy, MII_RESV, reg14h);
  } else {
    reg14h = mdio_read(dev, sp->cur_phy, MII_RESV);
    if (revision == SIS630A_900_REV && (host_bridge_rev == SIS630B0 || host_bridge_rev == SIS630B1)) {
      mdio_write(dev, sp->cur_phy, MII_RESV, (reg14h | 0x2200) & 0xBFFF);
    } else {
      mdio_write(dev, sp->cur_phy, MII_RESV, (reg14h | 0x2000) & 0xBFFF);
    }
  }
}

//
// sis900_timer - sis900 timer routine
//
// On each timer ticks we check two things, 
// link status (ON/OFF) and link mode (10/100/Full/Half)
//

static void sis900_timer(void *arg) {
  struct dev *dev = arg;
  struct sis900_private *sp = dev->privdata;
  struct mii_phy *mii_phy = sp->mii;
  unsigned short status;

  if (!sp->autong_complete) {
    int speed, duplex = 0;

    sis900_read_mode(dev, &speed, &duplex);
    if (duplex) {
      sis900_set_mode(sp->iobase, speed, duplex);
      sis630_set_eq(dev);
      //sp->carrier_ok = 1; // MRI
    }

    mod_timer(&sp->timer, get_ticks() + HZ);
    return;
  }

  status = mdio_read(dev, sp->cur_phy, MII_STATUS);
  status = mdio_read(dev, sp->cur_phy, MII_STATUS);

  if (!sp->carrier_ok) {
    // Link OFF -> ON
look_for_link:

    // Search for new PHY
    status = sis900_default_phy(dev);
    mii_phy = sp->mii;

    if (status & MII_STAT_LINK) {
      sis900_check_mode(dev, mii_phy);
      sp->carrier_ok = 1;
    }
  } else {
    // Link ON -> OFF
    if (!(status & MII_STAT_LINK)) {
      sp->carrier_ok = 0;
      kprintf(KERN_WARNING "%s: Media Link Off\n", dev->name);

      // Change mode issue
      if ((mii_phy->phy_id0 == 0x001D) && ((mii_phy->phy_id1 & 0xFFF0) == 0x8000)) {
        sis900_reset_phy(dev,  sp->cur_phy);
      }

      sis630_set_eq(dev);

      goto look_for_link;
    }
  }

  mod_timer(&sp->timer, get_ticks() + 5*HZ);
}

//
// sis900_check_mode - check the media mode for sis900
//
// Older driver gets the media mode from mii status output
// register. Now we set our media capability and auto-negotiate
// to get the upper bound of speed and duplex between two ends.
// If the types of mii phy is HOME, it doesn't need to auto-negotiate
// and autong_complete should be set to 1.
//

static void sis900_check_mode(struct dev *dev, struct mii_phy *mii_phy) {
  struct sis900_private *sp = dev->privdata;
  long ioaddr = sp->iobase;
  int speed, duplex;

  if (mii_phy->phy_types == LAN) {
    outpd(ioaddr + cfg, ~EXD & inpd(ioaddr + cfg));
    sis900_set_capability(dev , mii_phy);
    sis900_auto_negotiate(dev, sp->cur_phy);
  } else {
    outpd(ioaddr + cfg, EXD | inpd(ioaddr + cfg));
    speed = HW_SPEED_HOME;
    duplex = FDX_CAPABLE_HALF_SELECTED;
    sis900_set_mode(ioaddr, speed, duplex);
    sp->autong_complete = 1;
  }
}

//
// sis900_set_mode - Set the media mode of mac register.
//
// Set the media mode of mac register txcfg/rxcfg according to
// speed and duplex of phy. Bit EDB_MASTER_EN indicates the EDB
// bus is used instead of PCI bus. When this bit is set 1, the
// Max DMA Burst Size for TX/RX DMA should be no larger than 16
// double words.
//

static void sis900_set_mode(long ioaddr, int speed, int duplex) {
  unsigned long tx_flags = 0, rx_flags = 0;

  if (inpd(ioaddr + cfg) & EDB_MASTER_EN) {
    tx_flags = TxATP | (DMA_BURST_64 << TxMXDMA_shift) | (TX_FILL_THRESH << TxFILLT_shift);
    rx_flags = DMA_BURST_64 << RxMXDMA_shift;
  } else {
    tx_flags = TxATP | (DMA_BURST_512 << TxMXDMA_shift) | (TX_FILL_THRESH << TxFILLT_shift);
    rx_flags = DMA_BURST_512 << RxMXDMA_shift;
  }

  if (speed == HW_SPEED_HOME || speed == HW_SPEED_10_MBPS)
  {
    rx_flags |= (RxDRNT_10 << RxDRNT_shift);
    tx_flags |= (TxDRNT_10 << TxDRNT_shift);
  } else {
    rx_flags |= (RxDRNT_100 << RxDRNT_shift);
    tx_flags |= (TxDRNT_100 << TxDRNT_shift);
  }

  if (duplex == FDX_CAPABLE_FULL_SELECTED) {
    tx_flags |= (TxCSI | TxHBI);
    rx_flags |= RxATX;
  }

  outpd(ioaddr + txcfg, tx_flags);
  outpd(ioaddr + rxcfg, rx_flags);
}

//
// sis900_auto_negotiate:  Set the Auto-Negotiation Enable/Reset bit.
//
// If the adapter is link-on, set the auto-negotiate enable/reset bit.
// autong_complete should be set to 0 when starting auto-negotiation.
// autong_complete should be set to 1 if we didn't start auto-negotiation.
// sis900_timer will wait for link on again if autong_complete = 0.
//

static void sis900_auto_negotiate(struct dev *dev, int phy_addr) {
  struct sis900_private *sp = dev->privdata;
  int i = 0;
  unsigned long status;
  
  while (i++ < 2) status = mdio_read(dev, phy_addr, MII_STATUS);

  if (!(status & MII_STAT_LINK)) {
    kprintf(KERN_WARNING "%s: Media Link Off\n", dev->name);
    sp->autong_complete = 1;
    sp->carrier_ok = 0;
    return;
  }

  // (Re)start AutoNegotiate
  mdio_write(dev, phy_addr, MII_CONTROL, MII_CNTL_AUTO | MII_CNTL_RST_AUTO);
  sp->autong_complete = 0;
}

//
// sis900_read_mode - read media mode for sis900 internal phy
//
// The capability of remote end will be put in mii register autorec
// after auto-negotiation. Use AND operation to get the upper bound
// of speed and duplex between two ends.
//

static void sis900_read_mode(struct dev *dev, int *speed, int *duplex) {
  struct sis900_private *sp = dev->privdata;
  struct mii_phy *phy = sp->mii;
  int phy_addr = sp->cur_phy;
  unsigned long status;
  unsigned short autoadv, autorec;
  int i = 0;

  while (i++ < 2) status = mdio_read(dev, phy_addr, MII_STATUS);

  if (!(status & MII_STAT_LINK)) return;

  // AutoNegotiate completed
  autoadv = mdio_read(dev, phy_addr, MII_ANADV);
  autorec = mdio_read(dev, phy_addr, MII_ANLPAR);
  status = autoadv & autorec;
  
  *speed = HW_SPEED_10_MBPS;
  *duplex = FDX_CAPABLE_HALF_SELECTED;

  if (status & (MII_NWAY_TX | MII_NWAY_TX_FDX)) *speed = HW_SPEED_100_MBPS;
  if (status & (MII_NWAY_TX_FDX | MII_NWAY_T_FDX)) *duplex = FDX_CAPABLE_FULL_SELECTED;
  
  sp->autong_complete = 1;

  // Workaround for Realtek RTL8201 PHY issue
  if ((phy->phy_id0 == 0x0000) && ((phy->phy_id1 & 0xFFF0) == 0x8200)) {
    if (mdio_read(dev, phy_addr, MII_CONTROL) & MII_CNTL_FDX) *duplex = FDX_CAPABLE_FULL_SELECTED;
    if (mdio_read(dev, phy_addr, 0x0019) & 0x01) *speed = HW_SPEED_100_MBPS;
  }

  kprintf(KERN_INFO "%s: Media Link On %s %s-duplex \n", dev->name, *speed == HW_SPEED_100_MBPS ? "100mbps" : "10mbps", *duplex == FDX_CAPABLE_FULL_SELECTED ? "full" : "half");

  // Signal autonegotiate complete and link up
  set_event(&sp->link_up);
}

//
// sis900_tx_timeout - sis900 transmit timeout routine
//
// Print transmit timeout status
// Disable interrupts and do some tasks
//

static void sis900_tx_timeout(struct dev *dev) {
  struct sis900_private *sp = dev->privdata;
  long ioaddr = sp->iobase;
  int i;

  kprintf(KERN_WARNING "%s: Transmit timeout, status %8.8x %8.8x \n", dev->name, inpd(ioaddr + cr), inpd(ioaddr + isr));

  // Disable interrupts by clearing the interrupt mask.
  outpd(ioaddr + imr, 0x0000);

  // Discard unsent packets
  sp->dirty_tx = sp->cur_tx = 0;
  for (i = 0; i < NUM_TX_DESC; i++) {
    struct pbuf *p = sp->tx_pbuf[i];

    if (p) {
      pbuf_free(p);
      sp->tx_pbuf[i] = 0;
      sp->tx_ring[i].cmdsts = 0;
      sp->tx_ring[i].bufptr = 0;
      sp->stats.tx_dropped++;
    }
  }
  sp->tx_full = 0;

  // Load Transmit Descriptor Register
  outpd(ioaddr + txdp, sp->tx_ring_dma);

  // Enable all known interrupts by setting the interrupt mask.
  outpd(ioaddr + imr, (RxSOVR | RxORN | RxERR | RxOK | TxURN | TxERR | TxIDLE));
}

//
// sis900_transmit - sis900 start transmit routine
//
// Set the transmit buffer descriptor, 
// and write TxENA to enable transimt state machine.
// Tell upper layer if the buffer is full
//

static int sis900_transmit(struct dev *dev, struct pbuf *p) {
  struct sis900_private *sp = dev->privdata;
  long ioaddr = sp->iobase;
  unsigned int entry;
  //unsigned int index_cur_tx, index_dirty_tx;
  //unsigned int count_dirty_tx;

  // Don't transmit data before the complete of auto-negotiation
  if (!sp->autong_complete) return -EBUSY;

  // Wait for free entry in transmit ring
  if (wait_for_object(&sp->tx_sem, TX_TIMEOUT) < 0) {
    kprintf(KERN_WARNING "%s: transmit timeout, drop packet\n", dev->name);
    sp->stats.tx_dropped++;
    return -ETIMEOUT;
  }

  // Make sure the packet buffer is not fragmented
  p = pbuf_linearize(PBUF_RAW, p);

  // Calculate the next Tx descriptor entry.
  entry = sp->cur_tx % NUM_TX_DESC;
  sp->tx_pbuf[entry] = p;

  // Set the transmit buffer descriptor and enable Transmit State Machine
  sp->tx_ring[entry].bufptr = virt2phys(p->payload);
  sp->tx_ring[entry].cmdsts = (OWN | p->tot_len);
  outpd(ioaddr + cr, TxENA | inpd(ioaddr + cr));

  sp->cur_tx++;

  //index_cur_tx = sp->cur_tx;
  //index_dirty_tx = sp->dirty_tx;

  //for (count_dirty_tx = 0; index_cur_tx != index_dirty_tx; index_dirty_tx++) count_dirty_tx++;

  //if (index_cur_tx == index_dirty_tx) 
  //{
  //  // Dirty_tx is met in the cycle of cur_tx, buffer full
  //  sp->tx_full = 1;
  //  //netif_stop_queue(net_dev);
  //} 
  //else if (count_dirty_tx < NUM_TX_DESC) 
  //{ 
  //  // Typical path, tell upper layer that more transmission is possible
  //  //netif_start_queue(net_dev);
  //} else 
  //{
  //  // Buffer full, tell upper layer no more transmission
  //  sp->tx_full = 1;
  //  //netif_stop_queue(net_dev);
  //}

  //kprintf("%s: Queued Tx packet at %p size %d to slot %d.\n",  dev->name, p->payload, p->tot_len, entry);

  return 0;
}

//
// sis900_dpc - sis900 interrupt handler
//
// The dpc handler does all of the Rx thread work, 
// and cleans up after the Tx thread
//

static void sis900_dpc(void *arg) {
  struct dev *dev = arg;
  struct sis900_private *sp = dev->privdata;
  long ioaddr = sp->iobase;
  int boguscnt = max_interrupt_work;
  unsigned long status;

  while (1) {
    status = inpd(ioaddr + isr);
    if ((status & (HIBERR | TxURN | TxERR | TxIDLE | RxORN | RxERR | RxOK)) == 0) break;

    // Why dow't we break after Tx/Rx case ?? keyword: full-duplex
    
    if (status & (RxORN | RxERR | RxOK)) sis900_rx(dev);

    if (status & (TxURN | TxERR | TxIDLE)) sis900_finish_xmit(dev);

    // Something strange happened !!!
    if (status & HIBERR) {
      kprintf("%s: Abnormal interrupt, status 0x%08x.\n", dev->name, status);
      break;
    }

    if (--boguscnt < 0) {
      kprintf("%s: Too much work at interrupt, interrupt status = 0x%08x.\n", dev->name, status);
      break;
    }
  }

  //kprintf("%s: exiting interrupt, interrupt status = 0x%08x.\n", dev->name, inpd(ioaddr + isr));
  
  eoi(sp->irq);
}

//
// sis900_rx - sis900 receive routine
//
// Process receive interrupt events, 
// put buffer to higher layer and refill buffer pool
// Note: This fucntion is called by interrupt handler, 
// don't do "too much" work here
//

static int sis900_rx(struct dev *dev) {
  struct sis900_private *sp = dev->privdata;
  long ioaddr = sp->iobase;
  unsigned int entry = sp->cur_rx % NUM_RX_DESC;
  unsigned long rx_status = sp->rx_ring[entry].cmdsts;

  //kprintf("sis900_rx: cur_rx:%4.4d, dirty_rx:%4.4d status:0x%08x\n", sp->cur_rx, sp->dirty_rx, rx_status);

  while (rx_status & OWN) {
    unsigned int rx_size;

    rx_size = (rx_status & DSIZE) - CRC_SIZE;

    if (rx_status & (ABORT | OVERRUN | TOOLONG | RUNT | RXISERR | CRCERR | FAERR)) {
      // Corrupted packet received
      kprintf("%s: Corrupted packet received, buffer status = 0x%08x.\n", dev->name, rx_status);

      sp->stats.rx_errors++;
      if (rx_status & OVERRUN) sp->stats.rx_over_errors++;
      if (rx_status & (TOOLONG | RUNT)) sp->stats.rx_length_errors++;
      if (rx_status & (RXISERR | FAERR)) sp->stats.rx_frame_errors++;
      if (rx_status & CRCERR) sp->stats.rx_crc_errors++;
      
      // Reset buffer descriptor state
      sp->rx_ring[entry].cmdsts = RX_BUF_SIZE;
    } else {
      struct pbuf *p;

      // This situation should never happen, but due to
      // some unknow bugs, it is possible that
      // we are working on NULL pbuf :-(

      if (sp->rx_pbuf[entry] == NULL) {
        kprintf("%s: NULL pointer encountered in Rx ring, skipping\n", dev->name);
        break;
      }

      // Give the packet buffer to upper layers
      p = sp->rx_pbuf[entry];
      pbuf_realloc(p, rx_size);
      if (dev_receive(sp->devno, p) < 0) pbuf_free(p);

      // Some network statistics
      if ((rx_status & BCAST) == MCAST) sp->stats.multicast++;
      sp->stats.rx_bytes += rx_size;
      sp->stats.rx_packets++;

      // Refill the Rx buffer, what if there is not enought memory for
      // new socket buffer ??
      p = pbuf_alloc(PBUF_RAW, RX_BUF_SIZE, PBUF_RW);
      if (!p) {
        // Not enough memory for packet buffer , this makes a "hole"
        // on the buffer ring, it is not clear how the
        // hardware will react to this kind of degenerated
        // buffer
        kprintf("%s: Memory squeeze, deferring packet.\n", dev->name);
        sp->rx_pbuf[entry] = NULL;

        // Reset buffer descriptor state
        sp->rx_ring[entry].cmdsts = 0;
        sp->rx_ring[entry].bufptr = 0;
        sp->stats.rx_dropped++;
        break;
      }

      sp->rx_pbuf[entry] = p;
      sp->rx_ring[entry].cmdsts = RX_BUF_SIZE;
      sp->rx_ring[entry].bufptr = virt2phys(p->payload);
      sp->dirty_rx++;
    }
   
    sp->cur_rx++;
    entry = sp->cur_rx % NUM_RX_DESC;
    rx_status = sp->rx_ring[entry].cmdsts;
  }

  // Refill the Rx buffer, what if the rate of refilling is slower than 
  // consuming
  for (;sp->cur_rx - sp->dirty_rx > 0; sp->dirty_rx++) {
    struct pbuf *p;

    entry = sp->dirty_rx % NUM_RX_DESC;

    if (sp->rx_pbuf[entry] == NULL) {
      p = pbuf_alloc(PBUF_RAW, RX_BUF_SIZE, PBUF_RW);
      if (!p) {
        // Not enough memory for packet buffer, this makes a "hole"
        // on the buffer ring, it is not clear how the 
        // hardware will react to this kind of degenerated 
        // buffer
        kprintf("%s: Memory squeeze, deferring packet.\n", dev->name);
        sp->stats.rx_dropped++;
        break;
      }

      sp->rx_pbuf[entry] = p;
      sp->rx_ring[entry].cmdsts = RX_BUF_SIZE;
      sp->rx_ring[entry].bufptr = virt2phys(p->payload);
    }
  }

  // Re-enable the potentially idle receive state matchine
  outpd(ioaddr + cr, RxENA | inpd(ioaddr + cr));

  return 0;
}

//
// sis900_finish_xmit - finish up transmission of packets
//
// Check for error condition and free socket buffer etc 
// schedule for more transmission as needed
// Note: This fucntion is called by interrupt handler, 
// don't do "too much" work here
//

static void sis900_finish_xmit(struct dev *dev) {
  struct sis900_private *sp = dev->privdata;
  int freed = 0;

  for (; sp->dirty_tx != sp->cur_tx; sp->dirty_tx++) {
    struct pbuf *p;
    unsigned int entry;
    unsigned long tx_status;

    entry = sp->dirty_tx % NUM_TX_DESC;
    tx_status = sp->tx_ring[entry].cmdsts;

    if (tx_status & OWN) {
      // The packet is not transmitted yet (owned by hardware) !
      // Note: the interrupt is generated only when Tx Machine
      // is idle, so this is an almost impossible case
      break;
    }

    if (tx_status & (ABORT | UNDERRUN | OWCOLL)) {
      // Packet unsuccessfully transmitted
      kprintf("%s: Transmit error, Tx status %8.8x.\n", dev->name, tx_status);

      sp->stats.tx_errors++;
      if (tx_status & UNDERRUN) sp->stats.tx_fifo_errors++;
      if (tx_status & ABORT) sp->stats.tx_aborted_errors++;
      if (tx_status & NOCARRIER) sp->stats.tx_carrier_errors++;
      if (tx_status & OWCOLL) sp->stats.tx_window_errors++;
    } else {
      // packet successfully transmitted
      sp->stats.collisions += (tx_status & COLCNT) >> 16;
      sp->stats.tx_bytes += tx_status & DSIZE;
      sp->stats.tx_packets++;
    }

    // Free the original packet buffer
    p = sp->tx_pbuf[entry];
    pbuf_free(p);
    sp->tx_pbuf[entry] = NULL;
    sp->tx_ring[entry].bufptr = 0;
    sp->tx_ring[entry].cmdsts = 0;

    freed++;
  }

  //if (sp->tx_full && netif_queue_stopped(net_dev) && sp->cur_tx - sp->dirty_tx < NUM_TX_DESC - 4) 
  //{
  //  // The ring is no longer full, clear tx_full and schedule more transmission
  //  // by netif_wake_queue(net_dev)
  //  sp->tx_full = 0;
  //  netif_wake_queue (net_dev);
  //}

  //kprintf("%s: sis900_finish_xmit released %d packets from tx ring\n", dev->name, freed);

  release_sem(&sp->tx_sem, freed);
}

//
// sis900_close - close sis900 device 
//
// Disable interrupts, stop the Tx and Rx Status Machine 
// free Tx and RX socket buffer
//

static int sis900_close(struct dev *dev) {
  struct sis900_private *sp = dev->privdata;
  long ioaddr = sp->iobase;
  struct pbuf *p;
  int i;

  // Disable interrupts by clearing the interrupt mask.
  outpd(ioaddr + imr, 0x0000);
  outpd(ioaddr + ier, 0x0000);

  // Stop the chip's Tx and Rx Status Machine
  outpd(ioaddr + cr, RxDIS | TxDIS | inpd(ioaddr + cr));

  del_timer(&sp->timer);

  // Free Tx and RX pbuf
  for (i = 0; i < NUM_RX_DESC; i++) {
    p = sp->rx_pbuf[i];
    if (p) {
      pbuf_free(p);
      sp->rx_pbuf[i] = 0;
    }
  }

  for (i = 0; i < NUM_TX_DESC; i++) {
    p = sp->tx_pbuf[i];
    if (p) {
      pbuf_free(p);
      sp->tx_pbuf[i] = 0;
    }
  }

  return 0;
}

//
// sis900_compute_hashtable_index - compute hashtable index 
//
// SiS 900 uses the most sigificant 7 bits to index a 128 bits multicast
// hash table, which makes this function a little bit different from other drivers
// SiS 900 B0 & 635 M/B uses the most significat 8 bits to index 256 bits
// multicast hash table. 
//

static unsigned short sis900_compute_hashtable_index(unsigned char *addr, unsigned long revision) {
  unsigned long crc = ether_crc(6, addr);

  // leave 8 or 7 most siginifant bits
  if (revision >= SIS635A_900_REV || revision == SIS900B_900_REV) {
    return (unsigned short) (crc >> 24);
  } else {
    return (unsigned short) (crc >> 25);
  }
}

//
// sis900_set_rx_mode - Set SiS900 receive mode 
//
// Set SiS900 receive mode for promiscuous, multicast, or broadcast mode.
// And set the appropriate multicast filter.
// Multicast hash table changes from 128 to 256 bits for 635M/B & 900B0.
//

static int sis900_set_rx_mode(struct dev *dev) {
  struct sis900_private *sp = dev->privdata;
  long ioaddr = sp->iobase;
  unsigned short mc_filter[16] = {0};  // 256/128 bits multicast hash table
  int i, table_entries;
  unsigned long rx_mode;

  // 635 Hash Table entires = 256(2^16)
  if (dev->unit->revision >= SIS635A_900_REV || dev->unit->revision == SIS900B_900_REV) {
    table_entries = 16;
  } else {
    table_entries = 8;
  }

  if (dev->netif == NULL) {
    // Net interface not yet ready, accept all multicast packet
    rx_mode = RFAAB | RFAAM;
    for (i = 0; i < table_entries; i++) mc_filter[i] = 0xffff;
  } else if (dev->netif->flags & NETIF_PROMISC) {
    // Accept any kinds of packets
    rx_mode = RFPromiscuous;
    for (i = 0; i < table_entries; i++) mc_filter[i] = 0xffff;
  } else if ((dev->netif->mccount > multicast_filter_limit) || (dev->netif->flags & NETIF_ALLMULTI)) {
    // Too many multicast addresses or accept all multicast packet
    rx_mode = RFAAB | RFAAM;
    for (i = 0; i < table_entries; i++) mc_filter[i] = 0xffff;
  } else {
    // Accept Broadcast packet, destination address matchs our MAC address,
    // use Receive Filter to reject unwanted MCAST packet
    struct mclist *mclist;
    rx_mode = RFAAB;
    for (i = 0, mclist = dev->netif->mclist; mclist && i < dev->netif->mccount; i++, mclist = mclist->next) {
      set_bit(mc_filter, sis900_compute_hashtable_index((unsigned char *) &mclist->hwaddr, dev->unit->revision));
    }
  }

  // Update Multicast Hash Table in Receive Filter
  for (i = 0; i < table_entries; i++) {
    // Why plus 0x04 ??, That makes the correct value for hash table.
    outpd(ioaddr + rfcr, (unsigned long) (0x00000004 + i) << RFADDR_shift);
    outpd(ioaddr + rfdr, mc_filter[i]);
  }

  outpd(ioaddr + rfcr, RFEN | rx_mode);

  // sis900 is capatable of looping back packet at MAC level for debugging purpose
  if (dev->netif && dev->netif->flags & NETIF_LOOPBACK) {
    unsigned long cr_saved;
  
    // We must disable Tx/Rx before setting loopback mode
    cr_saved = inpd(ioaddr + cr);
    outpd(ioaddr + cr, cr_saved | TxDIS | RxDIS);
    
    // Enable loopback
    outpd(ioaddr + txcfg, inpd(ioaddr + txcfg) | TxMLB);
    outpd(ioaddr + rxcfg, inpd(ioaddr + rxcfg) | RxATX);
    
    // Restore cr
    outpd(ioaddr + cr, cr_saved);
  }

  return 0;
}

//
// sis900_reset - Reset sis900 MAC 
//
// Reset sis900 MAC and wait until finished
// reset through command register
// Change backoff algorithm for 900B0 & 635 M/B
//

static void sis900_reset(struct dev *dev) {
  struct sis900_private *sp = dev->privdata;
  long ioaddr = sp->iobase;
  int i = 0;
  unsigned long status = TxRCMP | RxRCMP;

  outpd(ioaddr + ier, 0);
  outpd(ioaddr + imr, 0);
  outpd(ioaddr + rfcr, 0);

  outpd(ioaddr + cr, RxRESET | TxRESET | RESET | inpd(ioaddr + cr));
  
  // Check that the chip has finished the reset.
  while (status && (i++ < 1000)) status ^= (inpd(isr + ioaddr) & status);

  if (dev->unit->revision >= SIS635A_900_REV || dev->unit->revision == SIS900B_900_REV) {
    outpd(ioaddr + cfg, PESEL | RND_CNT);
  } else {
    outpd(ioaddr + cfg, PESEL);
  }
}

static int sis900_handler(struct context *ctxt, void *arg) {
  struct dev *dev = arg;
  struct sis900_private *sp = dev->privdata;

  // Queue DPC to service interrupt
  //kprintf("%s: interrupt\n", dev->name);
  queue_irq_dpc(&sp->dpc, sis900_dpc, dev);

  return 0;
}

static int sis900_ioctl(struct dev *dev, int cmd, void *args, size_t size) {
  return -ENOSYS;
}

static int sis900_attach(struct dev *dev, struct eth_addr *hwaddr) {
  struct sis900_private *sp = dev->privdata;
  *hwaddr = sp->hwaddr;
  sis900_set_rx_mode(dev);

  return 0;
}

static int sis900_detach(struct dev *dev) {
  return 0;
}

struct driver sis900_driver = {
  "sis900",
  DEV_TYPE_PACKET,
  sis900_ioctl,
  NULL,
  NULL,
  sis900_attach,
  sis900_detach,
  sis900_transmit,
  sis900_set_rx_mode,
};

int __declspec(dllexport) install(struct unit *unit, char *opts) {
  struct sis900_private *sp;
  struct dev *dev;
  struct board *board;
  void *ring_space;
  long ioaddr;
  long irq;
  int rc;

  // Determine NIC type
  board = lookup_board(board_tbl, unit);
  if (!board) return -EIO;

  unit->vendorname = board->vendorname;
  unit->productname = board->productname;

  // Get NIC PCI configuration
  ioaddr = get_unit_iobase(unit);
  irq = get_unit_irq(unit);

  // Enable device and bus mastering
  pci_enable_busmastering(unit);

  // Allocate private memory
  sp = kmalloc(sizeof(struct sis900_private));
  if (sp == NULL) return -ENOMEM;
  memset(sp, 0, sizeof(struct sis900_private));

  // Create new device
  sp->devno = dev_make("eth#", &sis900_driver, unit, sp);
  if (sp->devno == NODEV) return -ENODEV;
  dev = device(sp->devno);

  sp->dev = dev;
  sp->iobase = ioaddr;
  sp->irq = irq;

  ring_space = kmalloc(TX_TOTAL_SIZE);
  if (!ring_space)  return -ENOMEM;
  sp->tx_ring = (struct bufferdesc *) ring_space;
  sp->tx_ring_dma = virt2phys(ring_space);

  ring_space = kmalloc(RX_TOTAL_SIZE);
  if (!ring_space) return -ENOMEM;
  sp->rx_ring = (struct bufferdesc *) ring_space;
  sp->rx_ring_dma = virt2phys(ring_space);
    
  // Get Mac address according to the chip revision
  if (unit->revision == SIS630E_900_REV) {
    rc = sis630e_get_mac_addr(dev);
  } else if (unit->revision > 0x81 && unit->revision <= 0x90) {
    rc = sis635_get_mac_addr(dev);
  } else if (unit->revision == SIS96x_900_REV) {
    rc = sis96x_get_mac_addr(dev);
  } else {
    rc = sis900_get_mac_addr(dev);
  }
  if (rc == 0) return -ENODEV;

  // Initialize interrupt handler
  init_dpc(&sp->dpc);
  register_interrupt(&sp->intr, IRQ2INTR(irq), sis900_handler, dev);
  enable_irq(irq);

  init_sem(&sp->tx_sem, NUM_TX_DESC);
  init_event(&sp->link_up, 1, 0);

  // 630ET: set the mii access mode as software-mode
  if (unit->revision == SIS630ET_900_REV) outpd(ioaddr + cr, ACCESSMODE | inpd(ioaddr + cr));

  // Probe for mii transceiver
  if (sis900_mii_probe(dev) == 0) return -ENODEV;

  // Print some information about our NIC
  kprintf(KERN_INFO "%s: %s iobase %#lx irq %d mac %la\n", dev->name, board->productname, ioaddr, irq, &sp->hwaddr);

  // Initialize NIC
  rc = sis900_open(dev);
  if (rc < 0) return rc;

  // Wait for link up and autonegotiation complete
  wait_for_object(&sp->link_up, 5000);

  return 0;
}

int __stdcall start(hmodule_t hmod, int reason, void *reserved2) {
  return 1;
}
