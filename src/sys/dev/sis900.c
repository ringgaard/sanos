//
// sis900.c
//
// SiS 900/7016 PCI Fast Ethernet driver
//
// Copyright (C) 2002 Michael Ringgaard. All rights reserved.
// Portions Copyright 1999 Silicon Integrated System Corporation
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

#include "sis900.h"

static int max_interrupt_work = 40;
static int multicast_filter_limit = 128;

// Time in ticks before concluding the transmitter is hung

#define TX_TIMEOUT  (4*HZ)

// SiS 900 is capable of 32 bits BM DMA

#define SIS900_DMA_MASK 0xffffffff

enum 
{
  SIS_900 = 0,
  SIS_7016
};

static struct board board_tbl[] = 
{
  {"SiS", "SiS 900 PCI Fast Ethernet", BUSTYPE_PCI, PCI_UNITCODE(0x1039, 0x0900), 0xffffffff, 0},
  {"SiS", "SiS 7016 PCI Fast Ethernet", BUSTYPE_PCI, PCI_UNITCODE(0x1039, 0x7016), 0xffffffff, 0},
  {0,}
};

static void sis900_read_mode(struct dev *dev, int *speed, int *duplex);

#define HOME 0x0001
#define LAN  0x0002
#define MIX  0x0003

struct mii_chip_info 
{
  const char *name;
  unsigned short phy_id0;
  unsigned short phy_id1;
  unsigned char  phy_types;
};

static struct mii_chip_info mii_chip_table[] = 
{
  {"SiS 900 Internal MII PHY", 0x001d, 0x8000, LAN},
  {"SiS 7014 Physical Layer Solution", 0x0016, 0xf830, LAN},
  {"AMD 79C901 10BASE-T PHY", 0x0000, 0x6B70, LAN},
  {"AMD 79C901 HomePNA PHY", 0x0000, 0x6B90, HOME},
  {"ICS LAN PHY", 0x0015, 0xF440, LAN},
  {"NS 83851 PHY", 0x2000, 0x5C20, MIX},
  {"Realtek RTL8201 PHY", 0x0000, 0x8200, LAN},
  {0,},
};

struct mii_phy 
{
  struct mii_phy *next;
  int phy_addr;
  unsigned short phy_id0;
  unsigned short phy_id1;
  unsigned short status;
  unsigned char phy_types;
};

typedef struct bufferdesc
{
  unsigned long link;
  unsigned long cmdsts;
  unsigned long bufptr;
};

struct sis900_private 
{
  devno_t devno;                        // Device number
  struct dev *dev;                      // Device block

  unsigned long iobase;		        // Configured I/O base
  unsigned long irq;		        // Configured IRQ

  struct interrupt intr;                // Interrupt object for driver
  struct dpc dpc;                       // DPC for driver
  struct timer timer;                   // Link status detection timer

  struct eth_addr hwaddr;               // MAC address for NIC

  struct stats_nic stats;

  struct mii_phy *mii;
  struct mii_phy *first_mii;            // Record the first mii structure
  unsigned int cur_phy;

  unsigned char autong_complete;        // Auto-negotiate complete 

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

#if 0

static int sis900_open(struct dev *dev);
static int sis900_mii_probe (struct dev *dev);
static void sis900_init_rxfilter (struct dev *dev);
static void sis900_timer(unsigned long data);
static void sis900_check_mode (struct dev *dev, struct mii_phy *mii_phy);
static void sis900_tx_timeout(struct dev *dev);
static void sis900_init_tx_ring(struct dev *dev);
static void sis900_init_rx_ring(struct dev *dev);
static int sis900_start_xmit(struct pbuf *skb, struct dev *dev);
static int sis900_rx(struct dev *dev);
static void sis900_finish_xmit (struct dev *dev);
static void sis900_interrupt(int irq, void *dev_instance, struct pt_regs *regs);
static int sis900_close(struct dev *dev);
static int mii_ioctl(struct dev *dev, struct ifreq *rq, int cmd);
static struct net_device_stats *sis900_get_stats(struct dev *dev);
static unsigned short sis900_compute_hashtable_index(unsigned char *addr, unsigned char revision);
static void set_rx_mode(struct dev *dev);
static void sis900_reset(struct dev *dev);
static void sis630_set_eq(struct dev *dev, unsigned char revision);
static int sis900_set_config(struct net_device *dev, struct ifmap *map);
static unsigned short sis900_default_phy(struct dev *dev);
static void sis900_set_capability( struct dev *dev ,struct mii_phy *phy);
static unsigned short sis900_reset_phy(struct dev *dev, int phy_addr);
static void sis900_auto_negotiate(struct dev *dev, int phy_addr);
static void sis900_set_mode (long ioaddr, int speed, int duplex);

#endif

//
// sis900_get_mac_addr - Get MAC address for stand alone SiS900 model
//
// Older SiS900 and friends, use EEPROM to store MAC address.
// MAC address is read from read_eeprom() into sis_priv->hwaddr.
//

static sis900_get_mac_addr(struct dev *dev)
{
  struct sis900_private *sis_priv = (struct sis900_private *) dev->privdata;
  long ioaddr = sis_priv->iobase;
  unsigned short signature;
  int i;

  // Check to see if we have sane EEPROM
  signature = (unsigned short) read_eeprom(ioaddr, EEPROMSignature);    
  if (signature == 0xffff || signature == 0x0000) 
  {
    kprintf("%s: Error EERPOM read %x\n", dev->name, signature);
    return 0;
  }

  // Get MAC address from EEPROM
  for (i = 0; i < 3; i++)
  {
    ((unsigned short *)(&sis_priv->hwaddr))[i] = read_eeprom(ioaddr, i + EEPROMMACAddr);
  }

  return 1;
}

//
// sis630e_get_mac_addr - Get MAC address for SiS630E model
//
// SiS630E model, use APC CMOS RAM to store MAC address.
// APC CMOS RAM is accessed through ISA bridge.
// MAC address is read into sis_priv->hwaddr.
//

static int sis630e_get_mac_addr(struct dev *dev)
{
  struct sis900_private *sis_priv = (struct sis900_private *) dev->privdata;
  struct unit *isa_bridge = NULL;
  unsigned char reg;
  int i;

  isa_bridge = lookup_unit(NULL, PCI_UNITCODE(0x1039, 0x0008), PCI_ID_ANY);
  if (isa_bridge == NULL) 
  {
    kprintf("%s: Can not find ISA bridge\n", dev->name);
    return 0;
  }

  pci_read_buffer(isa_bridge, 0x48, &reg, 1);
  reg |= 0x40;
  pci_write_buffer(isa_bridge, 0x48, &reg, 1);

  for (i = 0; i < 6; i++) 
  {
    outp(0x70, 0x09 + i);
    ((unsigned char *)(&sis_priv->hwaddr))[i] = inp(0x71);
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
// sis_priv->hwaddr.
//

static int sis635_get_mac_addr(struct dev *dev)
{
  struct sis900_private *sis_priv = (struct sis900_private *) dev->privdata;
  long ioaddr = sis_priv->iobase;
  unsigned long rfcr_save;
  unsigned long i;

  rfcr_save = inpd(rfcr + ioaddr);

  outpd(ioaddr + cr, rfcr_save | RELOAD);
  outpd(ioaddr + cr, 0);

  // Disable packet filtering before setting filter
  outpd(rfcr + ioaddr, rfcr_save & ~RFEN);

  // Load MAC addr to filter data register
  for (i = 0 ; i < 3 ; i++) 
  {
    outpd(ioaddr + rfcr, (i << RFADDR_shift));
    *(((unsigned short *) &sis_priv->hwaddr) + i) = inpw(ioaddr + rfdr);
  }

  // Enable packet filitering
  outpd(rfcr + ioaddr, rfcr_save | RFEN);

  return 1;
}

//
// sis900_mii_probe - Probe MII PHY for sis900
// 
// Search for total of 32 possible mii phy addresses.
// Identify and set current phy if found one,
// return error if it failed to found.
//

static int sis900_mii_probe(struct dev *dev)
{
  struct sis900_private *sis_priv = (struct sis900_private *) dev->privdata;
  unsigned short poll_bit = MII_STAT_LINK, status = 0;
  unsigned int timeout = ticks + 5 * HZ;
  int phy_addr;

  sis_priv->mii = NULL;

  // Search for total of 32 possible mii phy addresses
  for (phy_addr = 0; phy_addr < 32; phy_addr++)
  {
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
    mii_phy->next = sis_priv->mii;
    sis_priv->mii = mii_phy;
    sis_priv->first_mii = mii_phy;

    for (i = 0; mii_chip_table[i].phy_id1; i++)
    {
      if ((mii_phy->phy_id0 == mii_chip_table[i].phy_id0 ) && ((mii_phy->phy_id1 & 0xFFF0) == mii_chip_table[i].phy_id1))
      {
        mii_phy->phy_types = mii_chip_table[i].phy_types;
        if (mii_chip_table[i].phy_types == MIX)
	{
          mii_phy->phy_types = (mii_status & (MII_STAT_CAN_TX_FDX | MII_STAT_CAN_TX)) ? LAN : HOME;
	}

        kprintf("%s: %s transceiver found at address %d.\n", dev->name, mii_chip_table[i].name, phy_addr);
        break;
      }
    }
      
    if (!mii_chip_table[i].phy_id1)
    {
      kprintf("%s: Unknown PHY transceiver found at address %d.\n", dev->name, phy_addr);
    }
  }
  
  if (sis_priv->mii == NULL) 
  {
    kprintf("%s: No MII transceivers found!\n", dev->name);
    return 0;
  }

  // Select default PHY for mac
  sis_priv->mii = NULL;
  sis900_default_phy(dev);

  // Reset phy if default phy is internal sis900
  if ((sis_priv->mii->phy_id0 == 0x001D) && ((sis_priv->mii->phy_id1 & 0xFFF0) == 0x8000))
  {
    status = sis900_reset_phy(dev, sis_priv->cur_phy);
  }
        
  // Workaround for ICS1893 PHY
  if ((sis_priv->mii->phy_id0 == 0x0015) && ((sis_priv->mii->phy_id1 & 0xFFF0) == 0xF440))
  {
    mdio_write(dev, sis_priv->cur_phy, 0x0018, 0xD200);
  }

  if (status & MII_STAT_LINK)
  {
    while (poll_bit) 
    {
      poll_bit ^= (mdio_read(dev, sis_priv->cur_phy, MII_STATUS) & poll_bit);
      if (ticks >= timeout) 
      {
        kprintf("%s: reset phy and link down now\n", dev->name);
        return -ETIMEOUT;
      }
    }
  }

  if (dev->unit->revision == SIS630E_900_REV) 
  {
    // SiS 630E has some bugs on default value of PHY registers
    mdio_write(dev, sis_priv->cur_phy, MII_ANADV, 0x05e1);
    mdio_write(dev, sis_priv->cur_phy, MII_CONFIG1, 0x22);
    mdio_write(dev, sis_priv->cur_phy, MII_CONFIG2, 0xff00);
    mdio_write(dev, sis_priv->cur_phy, MII_MASK, 0xffc0);
  }

  //if (sis_priv->mii->status & MII_STAT_LINK)
  //  netif_carrier_on(net_dev);
  //else
  //  netif_carrier_off(net_dev);

  return 1;
}

//
// sis900_default_phy - Select default PHY for sis900 mac.
//
// Select first detected PHY with link as default.
// If no one is link on, select PHY whose types is HOME as default.
// If HOME doesn't exist, select LAN.
//

static unsigned short sis900_default_phy(struct dev *dev)
{
  struct sis900_private *sis_priv = (struct sis900_private *) dev->privdata;
  struct mii_phy *phy = NULL, *phy_home = NULL, *default_phy = NULL;
  unsigned short status;

  for (phy = sis_priv->first_mii; phy; phy = phy->next)
  {
    status = mdio_read(dev, phy->phy_addr, MII_STATUS);
    status = mdio_read(dev, phy->phy_addr, MII_STATUS);

    // Link ON & Not select deafalut PHY
    if ((status & MII_STAT_LINK) && !(default_phy))
      default_phy = phy;
    else
    {
      status = mdio_read(dev, phy->phy_addr, MII_CONTROL);
      mdio_write(dev, phy->phy_addr, MII_CONTROL, status | MII_CNTL_AUTO | MII_CNTL_ISOLATE);
      if (phy->phy_types == HOME) phy_home = phy;
     }
  }

  if ((!default_phy) && phy_home)
    default_phy = phy_home;
  else if (!default_phy)
    default_phy = sis_priv->first_mii;

  if (sis_priv->mii != default_phy)
  {
    sis_priv->mii = default_phy;
    sis_priv->cur_phy = default_phy->phy_addr;
    kprintf("%s: Using transceiver found at address %d as default\n", dev->name, sis_priv->cur_phy);
  }
  
  status = mdio_read(dev, sis_priv->cur_phy, MII_CONTROL);
  status &= (~MII_CNTL_ISOLATE);

  mdio_write(dev, sis_priv->cur_phy, MII_CONTROL, status);  
  status = mdio_read(dev, sis_priv->cur_phy, MII_STATUS);
  status = mdio_read(dev, sis_priv->cur_phy, MII_STATUS);

  return status;  
}

//
// sis900_set_capability - set the media capability of network adapter.
//
// Set the media capability of network adapter according to
// mii status register. It's necessary before auto-negotiate.
//
 
static void sis900_set_capability(struct dev *dev , struct mii_phy *phy)
{
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

static unsigned short read_eeprom(long ioaddr, int location)
{
  int i;
  unsigned short retval = 0;
  long ee_addr = ioaddr + mear;
  unsigned long read_cmd = location | EEread;

  outpd(ee_addr, 0);
  eeprom_delay();
  outpd(ee_addr, EECLK);
  eeprom_delay();

  // Shift the read command (9) bits out.
  for (i = 8; i >= 0; i--) 
  {
    unsigned long dataval = (read_cmd & (1 << i)) ? EEDI | EECS : EECS;
    outpd(ee_addr, dataval);
    eeprom_delay();
    outpd(ee_addr, dataval | EECLK);
    eeprom_delay();
  }
  outp(ee_addr, EECS);
  eeprom_delay();

  // Read the 16-bits data in
  for (i = 16; i > 0; i--) 
  {
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
  outpd(ee_addr, EECLK);

  return retval;
}

// Read and write the MII management registers using software-generated
// serial MDIO protocol. Note that the command bits and data bits are
// send out seperately

#define mdio_delay()    inpd(mdio_addr)

static void mdio_idle(long mdio_addr)
{
  outpd(mdio_addr, MDIO | MDDIR);
  mdio_delay();
  outpd(mdio_addr, MDIO | MDDIR | MDC);
}

// Syncronize the MII management interface by shifting 32 one bits out.

static void mdio_reset(long mdio_addr)
{
  int i;

  for (i = 31; i >= 0; i--) 
  {
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

static unsigned short mdio_read(struct dev *dev, int phy_id, int location)
{
  struct sis900_private *sis_priv = (struct sis900_private *) dev->privdata;
  long mdio_addr = sis_priv->iobase + mear;
  int mii_cmd = MIIread | (phy_id << MIIpmdShift) | (location << MIIregShift);
  unsigned short retval = 0;
  int i;

  mdio_reset(mdio_addr);
  mdio_idle(mdio_addr);

  for (i = 15; i >= 0; i--) 
  {
    int dataval = (mii_cmd & (1 << i)) ? MDDIR | MDIO : MDDIR;
    outpd(mdio_addr, dataval);
    mdio_delay();
    outpd(mdio_addr, dataval | MDC);
    mdio_delay();
  }

  // Read the 16 data bits.
  for (i = 16; i > 0; i--) 
  {
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

static void mdio_write(struct dev *dev, int phy_id, int location, int value)
{
  struct sis900_private *sis_priv = (struct sis900_private *) dev->privdata;
  long mdio_addr = sis_priv->iobase + mear;
  int mii_cmd = MIIwrite | (phy_id << MIIpmdShift) | (location << MIIregShift);
  int i;

  mdio_reset(mdio_addr);
  mdio_idle(mdio_addr);

  // Shift the command bits out.
  for (i = 15; i >= 0; i--) 
  {
    int dataval = (mii_cmd & (1 << i)) ? MDDIR | MDIO : MDDIR;
    outp(mdio_addr, dataval);
    mdio_delay();
    outp(mdio_addr, dataval | MDC);
    mdio_delay();
  }
  mdio_delay();

  // Shift the value bits out.
  for (i = 15; i >= 0; i--) 
  {
    int dataval = (value & (1 << i)) ? MDDIR | MDIO : MDDIR;
    outpd(mdio_addr, dataval);
    mdio_delay();
    outpd(mdio_addr, dataval | MDC);
    mdio_delay();
  }
  mdio_delay();

  // Clear out extra bits.
  for (i = 2; i > 0; i--) 
  {
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

static unsigned short sis900_reset_phy(struct dev *dev, int phy_addr)
{
  int i = 0;
  unsigned short status;

  while (i++ < 2) status = mdio_read(dev, phy_addr, MII_STATUS);
  mdio_write(dev, phy_addr, MII_CONTROL, MII_CNTL_RESET);
  
  return status;
}

#if 0

//
// sis900_open - open sis900 device
//
// Do some initialization and start net interface.
// enable interrupts and set sis900 timer.
//

static int sis900_open(struct dev *dev)
{
  struct sis900_private *sis_priv = net_dev->priv;
  long ioaddr = net_dev->base_addr;
  unsigned char revision;
  int ret;

  // Soft reset the chip.
  sis900_reset(net_dev);

  // Equalizer workaround Rule
  pci_read_config_byte(sis_priv->pci_dev, PCI_CLASS_REVISION, &revision);
  sis630_set_eq(net_dev, revision);

  ret = request_irq(net_dev->irq, &sis900_interrupt, SA_SHIRQ, net_dev->name, net_dev);
  if (ret) return ret;

  sis900_init_rxfilter(net_dev);

  sis900_init_tx_ring(net_dev);
  sis900_init_rx_ring(net_dev);

  set_rx_mode(net_dev);

  netif_start_queue(net_dev);

  // Workaround for EDB
  sis900_set_mode(ioaddr, HW_SPEED_10_MBPS, FDX_CAPABLE_HALF_SELECTED);

  // Enable all known interrupts by setting the interrupt mask.
  outl((RxSOVR | RxORN | RxERR | RxOK | TxURN | TxERR | TxIDLE), ioaddr + imr);
  outl(RxENA | inl(ioaddr + cr), ioaddr + cr);
  outl(IE, ioaddr + ier);

  sis900_check_mode(net_dev, sis_priv->mii);

  // Set the timer to switch to check for link beat and perhaps switch
  // to an alternate media type.
  init_timer(&sis_priv->timer);
  sis_priv->timer.expires = ticks + HZ;
  sis_priv->timer.data = (unsigned long) net_dev;
  sis_priv->timer.function = &sis900_timer;
  add_timer(&sis_priv->timer);

  return 0;
}

//
// sis900_init_rxfilter - Initialize the Rx filter
//
// Set receive filter address to our MAC address
// and enable packet filtering.
//

static void sis900_init_rxfilter(struct dev *dev)
{
  long ioaddr = net_dev->base_addr;
  unsigned long rfcr_save;
  unsigned long i;

  rfcr_save = inl(rfcr + ioaddr);

  // Disable packet filtering before setting filter
  outl(rfcr_save & ~RFEN, rfcr + ioaddr);

  // load MAC addr to filter data register
  for (i = 0 ; i < 3 ; i++) 
  {
    unsigned long w;

    w = (unsigned long) *((unsigned short *)(net_dev->dev_addr) + i);
    outl((i << RFADDR_shift), ioaddr + rfcr);
    outl(w, ioaddr + rfdr);

    printk(KERN_INFO "%s: Receive Filter Addrss[%d]=%x\n", net_dev->name, i, inl(ioaddr + rfdr));
  }

  // enable packet filitering
  outl(rfcr_save | RFEN, rfcr + ioaddr);
}

//
// sis900_init_tx_ring - Initialize the Tx descriptor ring
//
// Initialize the Tx descriptor ring, 
//

static void sis900_init_tx_ring(struct dev *dev)
{
  struct sis900_private *sis_priv = net_dev->priv;
  long ioaddr = net_dev->base_addr;
  int i;

  sis_priv->tx_full = 0;
  sis_priv->dirty_tx = sis_priv->cur_tx = 0;

  for (i = 0; i < NUM_TX_DESC; i++) 
  {
    sis_priv->tx_pbuf[i] = NULL;

    sis_priv->tx_ring[i].link = sis_priv->tx_ring_dma + ((i + 1) % NUM_TX_DESC) * sizeof(struct bufferdesc);
    sis_priv->tx_ring[i].cmdsts = 0;
    sis_priv->tx_ring[i].bufptr = 0;
  }

  // Load Transmit Descriptor Register
  outl(sis_priv->tx_ring_dma, ioaddr + txdp);
  printk(KERN_INFO "%s: TX descriptor register loaded with: %8.8x\n", net_dev->name, inl(ioaddr + txdp));
}

//
// sis900_init_rx_ring - Initialize the Rx descriptor ring
//
// Initialize the Rx descriptor ring, 
// and pre-allocate recevie buffers (socket buffer)
//

static void sis900_init_rx_ring(struct dev *dev)
{
  struct sis900_private *sis_priv = net_dev->priv;
  long ioaddr = net_dev->base_addr;
  int i;

  sis_priv->cur_rx = 0;
  sis_priv->dirty_rx = 0;

  // Init RX descriptor
  for (i = 0; i < NUM_RX_DESC; i++) 
  {
    sis_priv->rx_pbuf[i] = NULL;

    sis_priv->rx_ring[i].link = sis_priv->rx_ring_dma + ((i + 1) % NUM_RX_DESC) * sizeof(struct bufferdesc);
    sis_priv->rx_ring[i].cmdsts = 0;
    sis_priv->rx_ring[i].bufptr = 0;
  }

  // Allocate sock buffers
  for (i = 0; i < NUM_RX_DESC; i++) 
  {
    struct pbuf *skb;

    if ((skb = dev_alloc_skb(RX_BUF_SIZE)) == NULL) 
    {
      // Not enough memory for pbuf, this makes a "hole"
      // on the buffer ring, it is not clear how the
      // hardware will react to this kind of degenerated
      // buffer
      break;
    }
    skb->dev = net_dev;
    sis_priv->rx_pbuf[i] = skb;
    sis_priv->rx_ring[i].cmdsts = RX_BUF_SIZE;
    sis_priv->rx_ring[i].bufptr = pci_map_single(sis_priv->pci_dev, skb->tail, RX_BUF_SIZE, PCI_DMA_FROMDEVICE);
  }
  sis_priv->dirty_rx = (unsigned int) (i - NUM_RX_DESC);

  // Load Receive Descriptor Register
  outl(sis_priv->rx_ring_dma, ioaddr + rxdp);
  printk(KERN_INFO "%s: RX descriptor register loaded with: %8.8x\n", net_dev->name, inl(ioaddr + rxdp));
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
// 5 <= max <= 14 --> set equalizer to max+1 or set equalizer to max+2 if max == min
// max >= 15      --> set equalizer to max+5 or set equalizer to max+6 if max == min

static void sis630_set_eq(struct dev *dev, unsigned char revision)
{
  struct sis900_private *sis_priv = net_dev->priv;
  unsigned short reg14h, eq_value = 0, max_value = 0, min_value = 0;
  unsigned char host_bridge_rev;
  int i, maxcount=10;
  struct pci_dev *dev = NULL;

  if (!(revision == SIS630E_900_REV || revision == SIS630EA1_900_REV ||
        revision == SIS630A_900_REV || revision ==  SIS630ET_900_REV))
    return;

  dev = pci_find_device(PCI_VENDOR_ID_SI, PCI_DEVICE_ID_SI_630, dev);
  if (dev) pci_read_config_byte(dev, PCI_CLASS_REVISION, &host_bridge_rev);

  if (netif_carrier_ok(net_dev))
  {
    reg14h = mdio_read(net_dev, sis_priv->cur_phy, MII_RESV);
    mdio_write(net_dev, sis_priv->cur_phy, MII_RESV, (0x2200 | reg14h) & 0xBFFF);
    for (i = 0; i < maxcount; i++)
    {
      eq_value = (0x00F8 & mdio_read(net_dev, sis_priv->cur_phy, MII_RESV)) >> 3;
      if (i == 0) max_value = min_value = eq_value;
      max_value = (eq_value > max_value) ? eq_value : max_value;
      min_value = (eq_value < min_value) ? eq_value : min_value;
    }

    // 630E rule to determine the equalizer value
    if (revision == SIS630E_900_REV || revision == SIS630EA1_900_REV || revision == SIS630ET_900_REV) 
    {
      if (max_value < 5)
        eq_value = max_value;
      else if (max_value >= 5 && max_value < 15)
        eq_value = (max_value == min_value) ? max_value + 2 : max_value + 1;
      else if (max_value >= 15)
        eq_value =(max_value == min_value) ? max_value + 6 : max_value + 5;
    }

    // 630B0&B1 rule to determine the equalizer value
    if (revision == SIS630A_900_REV && (host_bridge_rev == SIS630B0 || host_bridge_rev == SIS630B1))
    {
      if (max_value == 0)
        eq_value = 3;
      else
        eq_value= (max_value + min_value + 1) / 2;
    }

    // Write equalizer value and setting
    reg14h = mdio_read(net_dev, sis_priv->cur_phy, MII_RESV);
    reg14h = (reg14h & 0xFF07) | ((eq_value << 3) & 0x00F8);
    reg14h = (reg14h | 0x6000) & 0xFDFF;
    mdio_write(net_dev, sis_priv->cur_phy, MII_RESV, reg14h);
  }
  else 
  {
    reg14h = mdio_read(net_dev, sis_priv->cur_phy, MII_RESV);
    if (revision == SIS630A_900_REV && (host_bridge_rev == SIS630B0 || host_bridge_rev == SIS630B1))
      mdio_write(net_dev, sis_priv->cur_phy, MII_RESV, (reg14h | 0x2200) & 0xBFFF);
    else
      mdio_write(net_dev, sis_priv->cur_phy, MII_RESV, (reg14h | 0x2000) & 0xBFFF);
  }
}

//
// sis900_timer - sis900 timer routine
//
// On each timer ticks we check two things, 
// link status (ON/OFF) and link mode (10/100/Full/Half)
//

static void sis900_timer(unsigned long data)
{
  struct dev *dev = (struct net_device *) data;
  struct sis900_private *sis_priv = net_dev->priv;
  struct mii_phy *mii_phy = sis_priv->mii;
  static int next_tick = 5*HZ;
  unsigned short status;
  unsigned char revision;

  if (!sis_priv->autong_complete)
  {
    int speed, duplex = 0;

    sis900_read_mode(net_dev, &speed, &duplex);
    if (duplex)
    {
      sis900_set_mode(net_dev->base_addr, speed, duplex);
      pci_read_config_byte(sis_priv->pci_dev, PCI_CLASS_REVISION, &revision);
      sis630_set_eq(net_dev, revision);
      netif_start_queue(net_dev);
    }

    sis_priv->timer.expires = ticks + HZ;
    add_timer(&sis_priv->timer);
    return;
  }

  status = mdio_read(net_dev, sis_priv->cur_phy, MII_STATUS);
  status = mdio_read(net_dev, sis_priv->cur_phy, MII_STATUS);

  if (!netif_carrier_ok(net_dev)) 
  {
    // Link OFF -> ON
look_for_link:
    // Search for new PHY
    status = sis900_default_phy(net_dev);
    mii_phy = sis_priv->mii;

    if (status & MII_STAT_LINK)
    {
      sis900_check_mode(net_dev, mii_phy);
      netif_carrier_on(net_dev);
    }
  }
  else 
  {
    // Link ON -> OFF
    if (!(status & MII_STAT_LINK))
    {
      netif_carrier_off(net_dev);
      printk(KERN_INFO "%s: Media Link Off\n", net_dev->name);

      // Change mode issue
      if ((mii_phy->phy_id0 == 0x001D) && ((mii_phy->phy_id1 & 0xFFF0) == 0x8000))
      {
        sis900_reset_phy(net_dev,  sis_priv->cur_phy);
      }

      pci_read_config_byte(sis_priv->pci_dev, PCI_CLASS_REVISION, &revision);
      sis630_set_eq(net_dev, revision);

      goto look_for_link;
    }
  }

  sis_priv->timer.expires = ticks + next_tick;
  add_timer(&sis_priv->timer);
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

static void sis900_check_mode(struct dev *dev, struct mii_phy *mii_phy)
{
  struct sis900_private *sis_priv = net_dev->priv;
  long ioaddr = net_dev->base_addr;
  int speed, duplex;

  if (mii_phy->phy_types == LAN)
  {
    outl(~EXD & inl(ioaddr + cfg), ioaddr + cfg);
    sis900_set_capability(net_dev , mii_phy);
    sis900_auto_negotiate(net_dev, sis_priv->cur_phy);
  }
  else
  {
    outl(EXD | inl(ioaddr + cfg), ioaddr + cfg);
    speed = HW_SPEED_HOME;
    duplex = FDX_CAPABLE_HALF_SELECTED;
    sis900_set_mode(ioaddr, speed, duplex);
    sis_priv->autong_complete = 1;
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

static void sis900_set_mode(long ioaddr, int speed, int duplex)
{
  unsigned long tx_flags = 0, rx_flags = 0;

  if(inl(ioaddr + cfg) & EDB_MASTER_EN)
  {
    tx_flags = TxATP | (DMA_BURST_64 << TxMXDMA_shift) | (TX_FILL_THRESH << TxFILLT_shift);
    rx_flags = DMA_BURST_64 << RxMXDMA_shift;
  }
  else
  {
    tx_flags = TxATP | (DMA_BURST_512 << TxMXDMA_shift) | (TX_FILL_THRESH << TxFILLT_shift);
    rx_flags = DMA_BURST_512 << RxMXDMA_shift;
  }

  if (speed == HW_SPEED_HOME || speed == HW_SPEED_10_MBPS ) 
  {
    rx_flags |= (RxDRNT_10 << RxDRNT_shift);
    tx_flags |= (TxDRNT_10 << TxDRNT_shift);
  }
  else 
  {
    rx_flags |= (RxDRNT_100 << RxDRNT_shift);
    tx_flags |= (TxDRNT_100 << TxDRNT_shift);
  }

  if (duplex == FDX_CAPABLE_FULL_SELECTED) 
  {
    tx_flags |= (TxCSI | TxHBI);
    rx_flags |= RxATX;
  }

  outl(tx_flags, ioaddr + txcfg);
  outl(rx_flags, ioaddr + rxcfg);
}

//
// sis900_auto_negotiate:  Set the Auto-Negotiation Enable/Reset bit.
//
// If the adapter is link-on, set the auto-negotiate enable/reset bit.
// autong_complete should be set to 0 when starting auto-negotiation.
// autong_complete should be set to 1 if we didn't start auto-negotiation.
// sis900_timer will wait for link on again if autong_complete = 0.
//

static void sis900_auto_negotiate(struct dev *dev, int phy_addr)
{
  struct sis900_private *sis_priv = net_dev->priv;
  int i = 0;
  unsigned long status;
  
  while (i++ < 2) status = mdio_read(net_dev, phy_addr, MII_STATUS);

  if (!(status & MII_STAT_LINK))
  {
    printk(KERN_INFO "%s: Media Link Off\n", net_dev->name);
    sis_priv->autong_complete = 1;
    netif_carrier_off(net_dev);
    return;
  }

  // (Re)start AutoNegotiate
  mdio_write(net_dev, phy_addr, MII_CONTROL, MII_CNTL_AUTO | MII_CNTL_RST_AUTO);
  sis_priv->autong_complete = 0;
}

//
// sis900_read_mode - read media mode for sis900 internal phy
//
// The capability of remote end will be put in mii register autorec
// after auto-negotiation. Use AND operation to get the upper bound
// of speed and duplex between two ends.
//

static void sis900_read_mode(struct dev *dev, int *speed, int *duplex)
{
  struct sis900_private *sis_priv = net_dev->priv;
  struct mii_phy *phy = sis_priv->mii;
  int phy_addr = sis_priv->cur_phy;
  unsigned long status;
  unsigned short autoadv, autorec;
  int i = 0;

  while (i++ < 2) status = mdio_read(net_dev, phy_addr, MII_STATUS);

  if (!(status & MII_STAT_LINK)) return;

  // AutoNegotiate completed
  autoadv = mdio_read(net_dev, phy_addr, MII_ANADV);
  autorec = mdio_read(net_dev, phy_addr, MII_ANLPAR);
  status = autoadv & autorec;
  
  *speed = HW_SPEED_10_MBPS;
  *duplex = FDX_CAPABLE_HALF_SELECTED;

  if (status & (MII_NWAY_TX | MII_NWAY_TX_FDX)) *speed = HW_SPEED_100_MBPS;
  if (status & ( MII_NWAY_TX_FDX | MII_NWAY_T_FDX)) *duplex = FDX_CAPABLE_FULL_SELECTED;
  
  sis_priv->autong_complete = 1;

  // Workaround for Realtek RTL8201 PHY issue
  if((phy->phy_id0 == 0x0000) && ((phy->phy_id1 & 0xFFF0) == 0x8200))
  {
    if (mdio_read(net_dev, phy_addr, MII_CONTROL) & MII_CNTL_FDX) *duplex = FDX_CAPABLE_FULL_SELECTED;
    if (mdio_read(net_dev, phy_addr, 0x0019) & 0x01) *speed = HW_SPEED_100_MBPS;
  }

  printk(KERN_INFO "%s: Media Link On %s %s-duplex \n", net_dev->name, *speed == HW_SPEED_100_MBPS ? "100mbps" : "10mbps", *duplex == FDX_CAPABLE_FULL_SELECTED ? "full" : "half");
}

//
// sis900_tx_timeout - sis900 transmit timeout routine
// @net_dev: the net device to transmit
//
// Print transmit timeout status
// Disable interrupts and do some tasks
//

static void sis900_tx_timeout(struct dev *dev)
{
  struct sis900_private *sis_priv = net_dev->priv;
  long ioaddr = net_dev->base_addr;
  unsigned long flags;
  int i;

  printk(KERN_INFO "%s: Transmit timeout, status %8.8x %8.8x \n", net_dev->name, inl(ioaddr + cr), inl(ioaddr + isr));

  // Disable interrupts by clearing the interrupt mask.
  outl(0x0000, ioaddr + imr);

  // Use spinlock to prevent interrupt handler accessing buffer ring
  spin_lock_irqsave(&sis_priv->lock, flags);

  // Discard unsent packets
  sis_priv->dirty_tx = sis_priv->cur_tx = 0;
  for (i = 0; i < NUM_TX_DESC; i++)
  {
    struct pbuf *skb = sis_priv->tx_pbuf[i];

    if (skb) 
    {
      pci_unmap_single(sis_priv->pci_dev, sis_priv->tx_ring[i].bufptr, skb->len, PCI_DMA_TODEVICE);
      dev_kfree_skb(skb);
      sis_priv->tx_pbuf[i] = 0;
      sis_priv->tx_ring[i].cmdsts = 0;
      sis_priv->tx_ring[i].bufptr = 0;
      sis_priv->stats.tx_dropped++;
    }
  }
  sis_priv->tx_full = 0;
  netif_wake_queue(net_dev);

  spin_unlock_irqrestore(&sis_priv->lock, flags);

  net_dev->trans_start = ticks;

  // FIXME: Should we restart the transmission thread here  ??
  outl(TxENA | inl(ioaddr + cr), ioaddr + cr);

  // Enable all known interrupts by setting the interrupt mask.
  outl((RxSOVR | RxORN | RxERR | RxOK | TxURN | TxERR | TxIDLE), ioaddr + imr);
}

//
// sis900_start_xmit - sis900 start transmit routine
//
// Set the transmit buffer descriptor, 
// and write TxENA to enable transimt state machine.
// Tell upper layer if the buffer is full
//

static int sis900_start_xmit(struct pbuf *skb, struct dev *dev)
{
  struct sis900_private *sis_priv = net_dev->priv;
  long ioaddr = net_dev->base_addr;
  unsigned int  entry;
  unsigned long flags;

  // Don't transmit data before the complete of auto-negotiation
  if (!sis_priv->autong_complete)
  {
    netif_stop_queue(net_dev);
    return 1;
  }

  spin_lock_irqsave(&sis_priv->lock, flags);

  // Calculate the next Tx descriptor entry.
  entry = sis_priv->cur_tx % NUM_TX_DESC;
  sis_priv->tx_pbuf[entry] = skb;

  // Set the transmit buffer descriptor and enable Transmit State Machine
  sis_priv->tx_ring[entry].bufptr = pci_map_single(sis_priv->pci_dev, skb->data, skb->len, PCI_DMA_TODEVICE);
  sis_priv->tx_ring[entry].cmdsts = (OWN | skb->len);
  outl(TxENA | inl(ioaddr + cr), ioaddr + cr);

  if (++sis_priv->cur_tx - sis_priv->dirty_tx < NUM_TX_DESC) 
  {
    // Typical path, tell upper layer that more transmission is possible
    netif_start_queue(net_dev);
  } 
  else 
  {
    // Buffer full, tell upper layer no more transmission
    sis_priv->tx_full = 1;
    netif_stop_queue(net_dev);
  }

  spin_unlock_irqrestore(&sis_priv->lock, flags);

  net_dev->trans_start = ticks;

  printk(KERN_INFO "%s: Queued Tx packet at %p size %d to slot %d.\n",  net_dev->name, skb->data, (int) skb->len, entry);

  return 0;
}

//
// sis900_interrupt - sis900 interrupt handler
//
// The interrupt handler does all of the Rx thread work, 
// and cleans up after the Tx thread
//

static void sis900_interrupt(int irq, void *dev_instance, struct pt_regs *regs)
{
  struct dev *dev = dev_instance;
  struct sis900_private *sis_priv = net_dev->priv;
  int boguscnt = max_interrupt_work;
  long ioaddr = net_dev->base_addr;
  unsigned long status;

  spin_lock (&sis_priv->lock);

  while (1)
  {
    status = inl(ioaddr + isr);
    if ((status & (HIBERR | TxURN | TxERR | TxIDLE | RxORN | RxERR | RxOK)) == 0) break;

    // Why dow't we break after Tx/Rx case ?? keyword: full-duplex
    
    if (status & (RxORN | RxERR | RxOK)) sis900_rx(net_dev);

    if (status & (TxURN | TxERR | TxIDLE)) sis900_finish_xmit(net_dev);

    // Something strange happened !!!
    if (status & HIBERR) 
    {
      printk(KERN_INFO "%s: Abnormal interrupt, status %#8.8x.\n", net_dev->name, status);
      break;
    }

    if (--boguscnt < 0) 
    {
      printk(KERN_INFO "%s: Too much work at interrupt, interrupt status = %#8.8x.\n", net_dev->name, status);
      break;
    }
  }

  printk(KERN_INFO "%s: exiting interrupt, interrupt status = 0x%#8.8x.\n", net_dev->name, inl(ioaddr + isr));
  
  spin_unlock (&sis_priv->lock);
}

//
// sis900_rx - sis900 receive routine
//
// Process receive interrupt events, 
// put buffer to higher layer and refill buffer pool
// Note: This fucntion is called by interrupt handler, 
// don't do "too much" work here
//

static int sis900_rx(struct dev *dev)
{
  struct sis900_private *sis_priv = net_dev->priv;
  long ioaddr = net_dev->base_addr;
  unsigned int entry = sis_priv->cur_rx % NUM_RX_DESC;
  unsigned long rx_status = sis_priv->rx_ring[entry].cmdsts;

  printk(KERN_INFO "sis900_rx, cur_rx:%4.4d, dirty_rx:%4.4d status:0x%8.8x\n", sis_priv->cur_rx, sis_priv->dirty_rx, rx_status);

  while (rx_status & OWN) 
  {
    unsigned int rx_size;

    rx_size = (rx_status & DSIZE) - CRC_SIZE;

    if (rx_status & (ABORT | OVERRUN | TOOLONG | RUNT | RXISERR | CRCERR | FAERR))
    {
      // Corrupted packet received
      printk(KERN_INFO "%s: Corrupted packet received, buffer status = 0x%8.8x.\n", net_dev->name, rx_status);

      sis_priv->stats.rx_errors++;
      if (rx_status & OVERRUN) sis_priv->stats.rx_over_errors++;
      if (rx_status & (TOOLONG | RUNT)) sis_priv->stats.rx_length_errors++;
      if (rx_status & (RXISERR | FAERR)) sis_priv->stats.rx_frame_errors++;
      if (rx_status & CRCERR) sis_priv->stats.rx_crc_errors++;
      
      // Reset buffer descriptor state
      sis_priv->rx_ring[entry].cmdsts = RX_BUF_SIZE;
    } 
    else 
    {
      struct pbuf *skb;

      // This situation should never happen, but due to
      // some unknow bugs, it is possible that
      // we are working on NULL pbuf :-(

      if (sis_priv->rx_pbuf[entry] == NULL) 
      {
        printk(KERN_INFO "%s: NULL pointer encountered in Rx ring, skipping\n", net_dev->name);
        break;
      }

      pci_dma_sync_single(sis_priv->pci_dev, sis_priv->rx_ring[entry].bufptr, RX_BUF_SIZE, PCI_DMA_FROMDEVICE);
      pci_unmap_single(sis_priv->pci_dev, sis_priv->rx_ring[entry].bufptr, RX_BUF_SIZE, PCI_DMA_FROMDEVICE);

      // Give the socket buffer to upper layers
      skb = sis_priv->rx_pbuf[entry];
      skb_put(skb, rx_size);
      skb->protocol = eth_type_trans(skb, net_dev);
      netif_rx(skb);

      // Some network statistics
      if ((rx_status & BCAST) == MCAST) sis_priv->stats.multicast++;
      net_dev->last_rx = ticks;
      sis_priv->stats.rx_bytes += rx_size;
      sis_priv->stats.rx_packets++;

      // Refill the Rx buffer, what if there is not enought memory for
      // new socket buffer ??
      if ((skb = dev_alloc_skb(RX_BUF_SIZE)) == NULL) 
      {
        // Not enough memory for pbuf, this makes a "hole"
        // on the buffer ring, it is not clear how the
        // hardware will react to this kind of degenerated
        // buffer
        printk(KERN_INFO "%s: Memory squeeze, deferring packet.\n", net_dev->name);
        sis_priv->rx_pbuf[entry] = NULL;

	// Reset buffer descriptor state
        sis_priv->rx_ring[entry].cmdsts = 0;
        sis_priv->rx_ring[entry].bufptr = 0;
        sis_priv->stats.rx_dropped++;
        break;
      }

      skb->dev = net_dev;
      sis_priv->rx_pbuf[entry] = skb;
      sis_priv->rx_ring[entry].cmdsts = RX_BUF_SIZE;
      sis_priv->rx_ring[entry].bufptr = pci_map_single(sis_priv->pci_dev, skb->tail, RX_BUF_SIZE, PCI_DMA_FROMDEVICE);
      sis_priv->dirty_rx++;
    }
   
    sis_priv->cur_rx++;
    entry = sis_priv->cur_rx % NUM_RX_DESC;
    rx_status = sis_priv->rx_ring[entry].cmdsts;
  }

  // Refill the Rx buffer, what if the rate of refilling is slower than 
  // consuming
  for (;sis_priv->cur_rx - sis_priv->dirty_rx > 0; sis_priv->dirty_rx++) 
  {
    struct pbuf *skb;

    entry = sis_priv->dirty_rx % NUM_RX_DESC;

    if (sis_priv->rx_pbuf[entry] == NULL) 
    {
      if ((skb = dev_alloc_skb(RX_BUF_SIZE)) == NULL) 
      {
        // Not enough memory for pbuf, this makes a "hole"
        // on the buffer ring, it is not clear how the 
        // hardware will react to this kind of degenerated 
        // buffer
        printk(KERN_INFO "%s: Memory squeeze, deferring packet.\n", net_dev->name);
        sis_priv->stats.rx_dropped++;
        break;
      }

      skb->dev = net_dev;
      sis_priv->rx_pbuf[entry] = skb;
      sis_priv->rx_ring[entry].cmdsts = RX_BUF_SIZE;
      sis_priv->rx_ring[entry].bufptr = pci_map_single(sis_priv->pci_dev, skb->tail, RX_BUF_SIZE, PCI_DMA_FROMDEVICE);
    }
  }

  // Re-enable the potentially idle receive state matchine
  outl(RxENA | inl(ioaddr + cr), ioaddr + cr);

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

static void sis900_finish_xmit(struct dev *dev)
{
  struct sis900_private *sis_priv = net_dev->priv;

  for (; sis_priv->dirty_tx < sis_priv->cur_tx; sis_priv->dirty_tx++) 
  {
    struct pbuf *skb;
    unsigned int entry;
    unsigned long tx_status;

    entry = sis_priv->dirty_tx % NUM_TX_DESC;
    tx_status = sis_priv->tx_ring[entry].cmdsts;

    if (tx_status & OWN) 
    {
      // The packet is not transmitted yet (owned by hardware) !
      // Note: the interrupt is generated only when Tx Machine
      // is idle, so this is an almost impossible case
      break;
    }

    if (tx_status & (ABORT | UNDERRUN | OWCOLL)) 
    {
      // Packet unsuccessfully transmitted
      printk(KERN_INFO "%s: Transmit error, Tx status %8.8x.\n", net_dev->name, tx_status);

      sis_priv->stats.tx_errors++;
      if (tx_status & UNDERRUN) sis_priv->stats.tx_fifo_errors++;
      if (tx_status & ABORT) sis_priv->stats.tx_aborted_errors++;
      if (tx_status & NOCARRIER) sis_priv->stats.tx_carrier_errors++;
      if (tx_status & OWCOLL) sis_priv->stats.tx_window_errors++;
    } 
    else 
    {
      // packet successfully transmitted
      sis_priv->stats.collisions += (tx_status & COLCNT) >> 16;
      sis_priv->stats.tx_bytes += tx_status & DSIZE;
      sis_priv->stats.tx_packets++;
    }

    // Free the original skb
    skb = sis_priv->tx_pbuf[entry];
    pci_unmap_single(sis_priv->pci_dev, sis_priv->tx_ring[entry].bufptr, skb->len, PCI_DMA_TODEVICE);
    dev_kfree_skb_irq(skb);
    sis_priv->tx_pbuf[entry] = NULL;
    sis_priv->tx_ring[entry].bufptr = 0;
    sis_priv->tx_ring[entry].cmdsts = 0;
  }

  if (sis_priv->tx_full && netif_queue_stopped(net_dev) && sis_priv->cur_tx - sis_priv->dirty_tx < NUM_TX_DESC - 4) 
  {
    // The ring is no longer full, clear tx_full and schedule more transmission
    // by netif_wake_queue(net_dev)
    sis_priv->tx_full = 0;
    netif_wake_queue (net_dev);
  }
}

//
// sis900_close - close sis900 device 
//
// Disable interrupts, stop the Tx and Rx Status Machine 
// free Tx and RX socket buffer
//

static int sis900_close(struct dev *dev)
{
  long ioaddr = net_dev->base_addr;
  struct sis900_private *sis_priv = net_dev->priv;
  struct pbuf *skb;
  int i;

  netif_stop_queue(net_dev);

  // Disable interrupts by clearing the interrupt mask.
  outl(0x0000, ioaddr + imr);
  outl(0x0000, ioaddr + ier);

  // Stop the chip's Tx and Rx Status Machine
  outl(RxDIS | TxDIS | inl(ioaddr + cr), ioaddr + cr);

  del_timer(&sis_priv->timer);

  free_irq(net_dev->irq, net_dev);

  // Free Tx and RX pbuf
  for (i = 0; i < NUM_RX_DESC; i++) 
  {
    skb = sis_priv->rx_pbuf[i];
    if (skb) 
    {
      pci_unmap_single(sis_priv->pci_dev, sis_priv->rx_ring[i].bufptr, RX_BUF_SIZE, PCI_DMA_FROMDEVICE);
      dev_kfree_skb(skb);
      sis_priv->rx_pbuf[i] = 0;
    }
  }
  for (i = 0; i < NUM_TX_DESC; i++) 
  {
    skb = sis_priv->tx_pbuf[i];
    if (skb) 
    {
      pci_unmap_single(sis_priv->pci_dev, sis_priv->tx_ring[i].bufptr, skb->len, PCI_DMA_TODEVICE);
      dev_kfree_skb(skb);
      sis_priv->tx_pbuf[i] = 0;
    }
  }

  // Green! Put the chip in low-power mode.

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

static unsigned short sis900_compute_hashtable_index(unsigned char *addr, unsigned char revision)
{
  unsigned long crc = ether_crc(6, addr);

  // leave 8 or 7 most siginifant bits
  if ((revision == SIS635A_900_REV) || (revision == SIS900B_900_REV))
    return (int)(crc >> 24);
  else
    return (int)(crc >> 25);
}

//
// set_rx_mode - Set SiS900 receive mode 
//
// Set SiS900 receive mode for promiscuous, multicast, or broadcast mode.
// And set the appropriate multicast filter.
// Multicast hash table changes from 128 to 256 bits for 635M/B & 900B0.
//

static void set_rx_mode(struct dev *dev)
{
  long ioaddr = net_dev->base_addr;
  struct sis900_private *sis_priv = net_dev->priv;
  unsigned short mc_filter[16] = {0};  // 256/128 bits multicast hash table
  int i, table_entries;
  unsigned long rx_mode;
  unsigned char revision;

  // 635 Hash Table entires = 256(2^16)
  pci_read_config_byte(sis_priv->pci_dev, PCI_CLASS_REVISION, &revision);
  if ((revision == SIS635A_900_REV) || (revision == SIS900B_900_REV))
    table_entries = 16;
  else
    table_entries = 8;

  if (net_dev->flags & IFF_PROMISC) 
  {
    // Accept any kinds of packets
    rx_mode = RFPromiscuous;
    for (i = 0; i < table_entries; i++) mc_filter[i] = 0xffff;
  } 
  else if ((net_dev->mc_count > multicast_filter_limit) || (net_dev->flags & IFF_ALLMULTI)) 
  {
    // too many multicast addresses or accept all multicast packet
    rx_mode = RFAAB | RFAAM;
    for (i = 0; i < table_entries; i++) mc_filter[i] = 0xffff;
  } 
  else 
  {
    // Accept Broadcast packet, destination address matchs our MAC address,
    // use Receive Filter to reject unwanted MCAST packet
    struct dev_mc_list *mclist;
    rx_mode = RFAAB;
    for (i = 0, mclist = net_dev->mc_list; mclist && i < net_dev->mc_count; i++, mclist = mclist->next)
    {
      set_bit(sis900_compute_hashtable_index(mclist->dmi_addr, revision), mc_filter);
    }
  }

  // Update Multicast Hash Table in Receive Filter
  for (i = 0; i < table_entries; i++) 
  {
    // Why plus 0x04 ??, That makes the correct value for hash table.
    outl((unsigned long)(0x00000004 + i) << RFADDR_shift, ioaddr + rfcr);
    outl(mc_filter[i], ioaddr + rfdr);
  }

  outl(RFEN | rx_mode, ioaddr + rfcr);

  // sis900 is capatable of looping back packet at MAC level for debugging purpose
  if (net_dev->flags & IFF_LOOPBACK) 
  {
    unsigned long cr_saved;
  
    // We must disable Tx/Rx before setting loopback mode
    cr_saved = inl(ioaddr + cr);
    outl(cr_saved | TxDIS | RxDIS, ioaddr + cr);
    
    // Enable loopback
    outl(inl(ioaddr + txcfg) | TxMLB, ioaddr + txcfg);
    outl(inl(ioaddr + rxcfg) | RxATX, ioaddr + rxcfg);
    
    // Restore cr
    outl(cr_saved, ioaddr + cr);
  }

  return;
}

//
// sis900_reset - Reset sis900 MAC 
//
// Reset sis900 MAC and wait until finished
// reset through command register
// Change backoff algorithm for 900B0 & 635 M/B


static void sis900_reset(struct dev *dev)
{
  struct sis900_private *sis_priv = net_dev->priv;
  long ioaddr = net_dev->base_addr;
  int i = 0;
  unsigned long status = TxRCMP | RxRCMP;
  unsigned char  revision;

  outl(0, ioaddr + ier);
  outl(0, ioaddr + imr);
  outl(0, ioaddr + rfcr);

  outl(RxRESET | TxRESET | RESET | inl(ioaddr + cr), ioaddr + cr);
  
  // Check that the chip has finished the reset.
  while (status && (i++ < 1000)) status ^= (inl(isr + ioaddr) & status);

  pci_read_config_byte(sis_priv->pci_dev, PCI_CLASS_REVISION, &revision);
  if ((revision == SIS635A_900_REV) || (revision == SIS900B_900_REV))
    outl(PESEL | RND_CNT, ioaddr + cfg);
  else
    outl(PESEL, ioaddr + cfg);
}

//
// sis900_remove - Remove sis900 device 
//
// remove and release SiS900 net device
//

static void sis900_remove(struct pci_dev *pci_dev)
{
  struct dev *dev = pci_get_drvdata(pci_dev);
  struct sis900_private *sis_priv = net_dev->priv;
  struct mii_phy *phy = NULL;

  while (sis_priv->first_mii)
  {
    phy = sis_priv->first_mii;
    sis_priv->first_mii = phy->next;
    kfree(phy);
  }

  pci_free_consistent(pci_dev, RX_TOTAL_SIZE, sis_priv->rx_ring, sis_priv->rx_ring_dma);
  pci_free_consistent(pci_dev, TX_TOTAL_SIZE, sis_priv->tx_ring, sis_priv->tx_ring_dma);
  unregister_netdev(net_dev);
  kfree(net_dev);
  pci_release_regions(pci_dev);
  pci_set_drvdata(pci_dev, NULL);
}
#endif

static void sis900_dpc(void *arg)
{
  struct dev *dev = (struct dev *) arg;
  struct sis900_private *sis_priv = (struct sis900_private *) dev->privdata;

  eoi(sis_priv->irq);
}


static int sis900_handler(struct context *ctxt, void *arg)
{
  struct dev *dev = (struct dev *) arg;
  struct sis900_private *sis_priv = (struct sis900_private *) dev->privdata;

  // Queue DPC to service interrupt
  kprintf("%s: interrupt\n", dev->name);
  queue_irq_dpc(&sis_priv->dpc, sis900_dpc, dev);

  return 0;
}

static int sis900_transmit(struct dev *dev, struct pbuf *p)
{
  return -ENOSYS;
}

static int sis900_set_rx_mode(struct dev *dev)
{
  return -ENOSYS;
}

static int sis900_ioctl(struct dev *dev, int cmd, void *args, size_t size)
{
  return -ENOSYS;
}

static int sis900_attach(struct dev *dev, struct eth_addr *hwaddr)
{
  //struct nic *np = dev->privdata;
  //*hwaddr = np->hwaddr;

  return -ENOSYS;
}

static int sis900_detach(struct dev *dev)
{
  return 0;
}

struct driver sis900_driver =
{
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

int __declspec(dllexport) install(struct unit *unit, char *opts)
{
  struct sis900_private *sis_priv;
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
  irq = irq = get_unit_irq(unit);

  // Enable bus mastering
  pci_enable_busmastering(unit);

  // Allocate private memory
  sis_priv = kmalloc(sizeof(struct sis900_private));
  if (sis_priv == NULL) return -ENOMEM;
  memset(sis_priv, 0, sizeof(struct sis900_private));

  // Create new device
  sis_priv->devno = dev_make("eth#", &sis900_driver, unit, sis_priv);
  if (sis_priv->devno == NODEV) return -ENODEV;
  dev = device(sis_priv->devno);

  sis_priv->dev = dev;
  sis_priv->iobase = ioaddr;
  sis_priv->irq = irq;

  init_dpc(&sis_priv->dpc);
  register_interrupt(&sis_priv->intr, IRQ2INTR(irq), sis900_handler, dev);

  ring_space = kmalloc(TX_TOTAL_SIZE);
  if (!ring_space)  return -ENOMEM;
  sis_priv->tx_ring = (struct bufferdesc *) ring_space;
  sis_priv->tx_ring_dma = virt2phys(ring_space);

  ring_space = kmalloc(RX_TOTAL_SIZE);
  if (!ring_space) return -ENOMEM;
  sis_priv->rx_ring = (struct bufferdesc *) ring_space;
  sis_priv->rx_ring_dma = virt2phys(ring_space);
    
  // Get Mac address according to the chip revision
  if (unit->revision == SIS630E_900_REV)
    rc = sis630e_get_mac_addr(dev);
  else if (unit->revision > 0x81 && unit->revision <= 0x90)
    rc = sis635_get_mac_addr(dev);
  else
    rc = sis900_get_mac_addr(dev);

  if (rc == 0) return -ENODEV;
  
  // 630ET : set the mii access mode as software-mode
  if (unit->revision == SIS630ET_900_REV) outpd(ioaddr + cr, ACCESSMODE | inpd(ioaddr + cr));

  // probe for mii transceiver
  if (sis900_mii_probe(dev) == 0) return -ENODEV;

  // print some information about our NIC
  kprintf("%s: %s at %#lx, IRQ %d, %la\n", dev->name, board->productname, ioaddr, irq, &sis_priv->hwaddr);

  //return sis900_open(dev);

  return 0;
}

int __stdcall start(hmodule_t hmod, int reason, void *reserved2)
{
  return 1;
}
