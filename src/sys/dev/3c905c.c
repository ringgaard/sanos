//
// 3c905c.c
//
// 3Com 3C905C NIC network driver
//
// Copyright (C) 2002 Michael Ringgaard. All rights reserved.
// Copyright (C) 1999 3Com Corporation. All rights reserved.
// 
// 3Com Network Driver software is distributed as is, without any warranty
// of any kind, either express or implied as further specified in the GNU Public
// License. This software may be used and distributed according to the terms of
// the GNU Public License, located in the file LICENSE.
//
// 3Com and EtherLink are registered trademarks of 3Com Corporation.
// 

#include <os/krnl.h>
#include "3c905c.h"

#define tracenic 0
#define nictrace if (tracenic) kprintf

char *connectorname[] = {"10Base-T", "AUI", "n/a", "BNC", "100Base-TX", "100Base-FX", "MII", "n/a", "Auto", "Ext-MII"};

struct sg_entry {
  unsigned long addr;
  unsigned long length;
};

struct rx_desc {
  unsigned long phys_next;
  unsigned long status;
  struct sg_entry sglist[1];
  struct rx_desc *next;
  struct pbuf *pkt;
  char filler[8];
};

struct tx_desc {
  unsigned long phys_next;
  unsigned long header;
  struct sg_entry sglist[TX_MAX_FRAGS];
  struct tx_desc *next;
  struct tx_desc *prev;
  struct pbuf *pkt;
  unsigned long phys_addr;
  char filler[8];
};

struct nicstat {
  // Transmit statistics.
  unsigned long tx_frames_ok;
  unsigned long tx_bytes_ok;
  unsigned long tx_frames_deferred;
  unsigned long tx_single_collisions;
  unsigned long tx_multiple_collisions;
  unsigned long tx_late_collisions;
  unsigned long tx_carrier_lost;

  unsigned long tx_maximum_collisions;
  unsigned long tx_sqe_errors;
  unsigned long tx_hw_errors;
  unsigned long tx_jabber_error;
  unsigned long tx_unknown_error;

  // Receive statistics.
  unsigned long rx_frames_ok;
  unsigned long rx_bytes_ok;

  unsigned long rx_overruns;
  unsigned long rx_bad_ssd;
  unsigned long rx_alignment_error;
  unsigned long rx_bad_crc_error;
  unsigned long rx_oversize_error;
};

struct nic {
  struct rx_desc rx_ring[RX_RING_SIZE];
  struct tx_desc tx_ring[TX_RING_SIZE];
  struct rx_desc *curr_rx;              // Next entry to receive from rx ring
  struct tx_desc *curr_tx;              // Next entry to reclaim from tx ring
  struct tx_desc *next_tx;              // Next entry to add in tx ring

  struct sem tx_sem;                    // Semaphore for tx ring
  int tx_size;                          // Number of active entries in transmit list

  dev_t devno;                          // Device number
  unsigned long deviceid;               // PCI device id

  unsigned short iobase;                // Configured I/O base
  unsigned short irq;                   // Configured IRQ

  int autoselect;                       // Auto-negotiate
  int connector;                        // Active connector
  int linkspeed;                        // Link speed in mbits/s
  int fullduplex;                       // Full duplex link
  int flowcontrol;                      // Flow control

  struct eth_addr hwaddr;               // MAC address for NIC

  struct interrupt intr;                // Interrupt object for driver
  struct dpc dpc;                       // DPC for driver

  struct nicstat stat;                  // NIC statistics
  unsigned short eeprom[EEPROM_SIZE];   // EEPROM contents
};

struct netstats *netstats;

void clear_statistics(struct nic *nic);
void update_statistics(struct nic *nic);
void update_statistics(struct nic *nic);
int nicstat_proc(struct proc_file *pf, void *arg);

int nic_find_mii_phy(struct nic *nic);
void nic_send_mii_phy_preamble(struct nic *nic);
void nic_write_mii_phy(struct nic *nic, unsigned short reg, unsigned short value);
int nic_read_mii_phy(struct nic *nic, unsigned short reg);

int nic_eeprom_busy(struct nic *nic);
int nic_read_eeprom(struct nic *nic, unsigned short addr);

int nic_transmit(struct dev *dev, struct pbuf *p);
int nic_ioctl(struct dev *dev, int cmd, void *args, size_t size);
int nic_attach(struct dev *dev, struct eth_addr *hwaddr);
int nic_detach(struct dev *dev);
void nic_up_complete(struct nic *nic);
void nic_down_complete(struct nic *nic);
void nic_host_error(struct nic *nic);
void nic_tx_complete(struct nic *nic);
void nic_dpc(void *arg);
int nic_handler(struct context *ctxt, void *arg);

int nic_try_mii(struct nic *nic, unsigned short options);
int nic_negotiate_link(struct nic *nic, unsigned short options);
int nic_program_mii(struct nic *nic, int connector);
int nic_initialize_adapter(struct nic *nic);
int nic_get_link_speed(struct nic *nic);
int nic_restart_receiver(struct nic *nic);
int nic_restart_transmitter(struct nic *nic);
int nic_flow_control(struct nic *nic);
int nic_configure_mii(struct nic *nic, unsigned short media_options);
int nic_check_mii_configuration(struct nic *nic, unsigned short media_options);
int nic_check_mii_auto_negotiation_status(struct nic *nic);
int nic_check_dc_converter(struct nic *nic, int enabled);
int nic_setup_connector(struct nic *nic, int connector);
int nic_setup_media(struct nic *nic);
int nic_setup_buffers(struct nic *nic);

struct driver nic_driver = {
  "3c905c",
  DEV_TYPE_PACKET,
  nic_ioctl,
  NULL,
  NULL,
  nic_attach,
  nic_detach,
  nic_transmit
};

__inline void execute_command(struct nic *nic, int cmd, int param) {
  outpw(nic->iobase + CMD, (unsigned short) (cmd | param));
}

int execute_command_wait(struct nic *nic, int cmd, int param) {
  int i;

  execute_command(nic, cmd, param);
  for (i = 0; i < 100000; i++) {
    if (!(inpw(nic->iobase + STATUS) & INTSTATUS_CMD_IN_PROGRESS)) return 0;
    udelay(10);
  }

  for (i = 0; i < 200; i++) {
    if (!(inpw(nic->iobase + STATUS) & INTSTATUS_CMD_IN_PROGRESS)) return 0;
    msleep(10);
  }

  kprintf(KERN_ERR "nic: command did not complete\n");
  return -ETIMEOUT;
}

__inline void select_window(struct nic *nic, int window) {
  execute_command(nic, CMD_SELECT_WINDOW, window);
}

void clear_statistics(struct nic *nic) {
  int i;

  select_window(nic, 6);
  for (i = FIRST_BYTE_STAT; i <= LAST_BYTE_STAT; i++) inp(nic->iobase + i);
  inpw(nic->iobase + BYTES_RECEIVED_OK);
  inpw(nic->iobase + BYTES_XMITTED_OK);
  select_window(nic, 4);
  inp(nic->iobase + BAD_SSD);
  inp(nic->iobase + UPPER_BYTES_OK);
}

void update_statistics(struct nic *nic) {
  int current_window;
  unsigned long rx_frames;
  unsigned long tx_frames;
  unsigned long rx_bytes;
  unsigned long tx_bytes;
  unsigned char upper;
  
  // Read the current window
  current_window = inpw(nic->iobase + STATUS) >> 13;

  // Read statistics from window 6
  select_window(nic, 6);

  nic->stat.tx_sqe_errors += inp(nic->iobase + SQE_ERRORS) & 0xFF;
  nic->stat.tx_multiple_collisions += inp(nic->iobase + MULTIPLE_COLLISIONS) & 0xFF;
  nic->stat.tx_single_collisions += inp(nic->iobase + SINGLE_COLLISIONS) & 0xFF;
  nic->stat.rx_overruns += inp(nic->iobase + RX_OVERRUNS) & 0xFF;
  nic->stat.tx_carrier_lost += inp(nic->iobase + CARRIER_LOST) & 0xFF;
  nic->stat.tx_late_collisions += inp(nic->iobase + LATE_COLLISIONS) & 0xFF;
  nic->stat.tx_frames_deferred += inp(nic->iobase + FRAMES_DEFERRED) & 0xFF;

  // Frames received/transmitted
  rx_frames = inp(nic->iobase + FRAMES_RECEIVED_OK) & 0xFF;
  tx_frames = inp(nic->iobase + FRAMES_XMITTED_OK) & 0xFF;
  upper = inp(nic->iobase + UPPER_FRAMES_OK) & 0xFF;
  rx_frames += (upper & 0x0F) << 8;
  tx_frames += (upper & 0xF0) << 4;

  nic->stat.rx_frames_ok += rx_frames;
  nic->stat.tx_frames_ok += tx_frames;

  // Bytes received/transmitted - upper part added below from window 4
  rx_bytes = inpw(nic->iobase + BYTES_RECEIVED_OK) & 0xFFFF;
  tx_bytes = inpw(nic->iobase + BYTES_XMITTED_OK) & 0xFFFF;

  // Read final statistics from window 4
  select_window(nic, 4);

  // Update bytes received/transmitted with upper part
  upper = inp(nic->iobase + UPPER_BYTES_OK);
  rx_bytes += (upper & 0x0F) << 16;
  tx_bytes += (upper & 0xF0) << 12;

  nic->stat.rx_bytes_ok += rx_bytes;
  nic->stat.tx_bytes_ok += tx_bytes;

  nic->stat.rx_bad_ssd += inp(nic->iobase + BAD_SSD) & 0xFF;

  // Set the window to its previous value
  select_window(nic, current_window);
}

int nicstat_proc(struct proc_file *pf, void *arg) {
  struct nic *nic = arg;

  update_statistics(nic);

  pprintf(pf, "Frames transmitted... : %lu\n", nic->stat.tx_frames_ok);
  pprintf(pf, "Bytes transmitted.... : %lu\n", nic->stat.tx_bytes_ok);
  pprintf(pf, "Frames deferred...... : %lu\n", nic->stat.tx_frames_deferred);
  pprintf(pf, "Single collisions.... : %lu\n", nic->stat.tx_single_collisions);
  pprintf(pf, "Multiple collisions.. : %lu\n", nic->stat.tx_multiple_collisions);
  pprintf(pf, "Late collisions...... : %lu\n", nic->stat.tx_late_collisions);
  pprintf(pf, "Carrier lost......... : %lu\n", nic->stat.tx_carrier_lost);
  pprintf(pf, "Maximum collisions .. : %lu\n", nic->stat.tx_maximum_collisions);
  pprintf(pf, "SQE errors........... : %lu\n", nic->stat.tx_sqe_errors);
  pprintf(pf, "HW errors............ : %lu\n", nic->stat.tx_hw_errors);
  pprintf(pf, "Jabber errors........ : %lu\n", nic->stat.tx_jabber_error);
  pprintf(pf, "Unknown errors....... : %lu\n", nic->stat.tx_unknown_error);
  pprintf(pf, "Frames received...... : %lu\n", nic->stat.rx_frames_ok);
  pprintf(pf, "Bytes received....... : %lu\n", nic->stat.rx_bytes_ok);
  pprintf(pf, "Overruns............. : %lu\n", nic->stat.rx_overruns);
  pprintf(pf, "Bad SSD.............. : %lu\n", nic->stat.rx_bad_ssd);
  pprintf(pf, "Allignment errors.... : %lu\n", nic->stat.rx_alignment_error);
  pprintf(pf, "CRC errors........... : %lu\n", nic->stat.rx_bad_crc_error);
  pprintf(pf, "Oversized frames..... : %lu\n", nic->stat.rx_oversize_error);

  return 0;
}

int nic_find_mii_phy(struct nic *nic) {
  unsigned short media_options;
  unsigned short phy_management;
  int i;

  // Read the MEDIA OPTIONS to see what connectors are available
  select_window(nic, 3);
  media_options = inpw(nic->iobase + MEDIA_OPTIONS);

  if ((media_options & MEDIA_OPTIONS_MII_AVAILABLE) ||
      (media_options & MEDIA_OPTIONS_100BASET4_AVAILABLE)) {
    // Drop everything, so we are not driving the data, and run the
    // clock through 32 cycles in case the PHY is trying to tell us
    // something. Then read the data line, since the PHY's pull-up
    // will read as a 1 if it's present.
    select_window(nic, 4);
    outpw(nic->iobase + PHYSICAL_MANAGEMENT, 0);

    for (i = 0; i < 32; i++) {
      udelay(1);
      outpw(nic->iobase + PHYSICAL_MANAGEMENT, PHY_CLOCK);

      udelay(1);
      outpw(nic->iobase + PHYSICAL_MANAGEMENT, 0);

    }

    phy_management = inpw(nic->iobase + PHYSICAL_MANAGEMENT);

    if (phy_management & PHY_DATA1) {
      return 0;
    } else {
      return -EIO;
    }
  }

  return 0;
}

void nic_send_mii_phy_preamble(struct nic *nic) {
  int i;

  // Set up and send the preamble, a sequence of 32 "1" bits
  select_window(nic, 4);
  outpw(nic->iobase + PHYSICAL_MANAGEMENT, PHY_WRITE);

  for (i = 0; i < 32; i++) {
    outpw(nic->iobase + PHYSICAL_MANAGEMENT, PHY_WRITE | PHY_DATA1);
    outpw(nic->iobase + PHYSICAL_MANAGEMENT, PHY_WRITE | PHY_DATA1 | PHY_CLOCK);
    udelay(1);
    outpw(nic->iobase + PHYSICAL_MANAGEMENT, PHY_WRITE);
    udelay(1);
  }
}

void nic_write_mii_phy(struct nic *nic, unsigned short reg, unsigned short value) {
  int i,j;
  unsigned short writecmd[2];

  writecmd[0] = MII_PHY_ADDRESS_WRITE;
  writecmd[1] = 0;

  nic_send_mii_phy_preamble(nic);

  // Bits 2..6 of the command word specify the register
  writecmd[0] |= (reg & 0x1F) << 2;
  writecmd[1] = value;

  select_window(nic, 4);

  for (i = 0; i < 2; i++) {
    for (j = 0x8000; j; j >>= 1) {
      if (writecmd[i] & j) {
        outpw(nic->iobase + PHYSICAL_MANAGEMENT, PHY_WRITE | PHY_DATA1);
        outpw(nic->iobase + PHYSICAL_MANAGEMENT, PHY_WRITE | PHY_DATA1 | PHY_CLOCK);
      } else  {
        outpw(nic->iobase + PHYSICAL_MANAGEMENT, PHY_WRITE);
        outpw(nic->iobase + PHYSICAL_MANAGEMENT, PHY_WRITE | PHY_CLOCK);
      }
      udelay(1);
      outpw(nic->iobase + PHYSICAL_MANAGEMENT, PHY_WRITE);
      udelay(1);
    }
  }

  // Now give it a couple of clocks with nobody driving
  outpw(nic->iobase + PHYSICAL_MANAGEMENT, 0);
  for (i = 0; i < 2; i++) {
    outpw(nic->iobase + PHYSICAL_MANAGEMENT, PHY_CLOCK);
    udelay(1);
    outpw(nic->iobase + PHYSICAL_MANAGEMENT, 0);
    udelay(1);
  }
}

int nic_read_mii_phy(struct nic *nic, unsigned short reg) {
  unsigned short  phy_mgmt = 0;
  unsigned short read_cmd;
  unsigned short value;
  int i;

  read_cmd = MII_PHY_ADDRESS_READ;

  nic_send_mii_phy_preamble(nic);
  
  // Bits 2..6 of the command word specify the register
  read_cmd |= (reg & 0x1F) << 2;

  select_window(nic, 4);

  for (i = 0x8000; i > 2; i >>= 1) {
    if (read_cmd & i) {
      outpw(nic->iobase + PHYSICAL_MANAGEMENT, PHY_WRITE | PHY_DATA1);
      outpw(nic->iobase + PHYSICAL_MANAGEMENT, PHY_WRITE | PHY_DATA1 | PHY_CLOCK);
    } else {
      outpw(nic->iobase + PHYSICAL_MANAGEMENT, PHY_WRITE);
      outpw(nic->iobase + PHYSICAL_MANAGEMENT, PHY_WRITE | PHY_CLOCK);
    }

    udelay(1);
    outpw(nic->iobase + PHYSICAL_MANAGEMENT, PHY_WRITE);
    udelay(1);
  }

  // Now run one clock with nobody driving
  outpw(nic->iobase + PHYSICAL_MANAGEMENT, 0);
  outpw(nic->iobase + PHYSICAL_MANAGEMENT, PHY_CLOCK);
  udelay(1);
  outpw(nic->iobase + PHYSICAL_MANAGEMENT, 0);
  udelay(1);

  // Now run one clock, expecting the PHY to be driving a 0 on the data
  // line.  If we read a 1, it has to be just his pull-up, and he's not
  // responding.

  phy_mgmt = inpw(nic->iobase + PHYSICAL_MANAGEMENT);
  if (phy_mgmt & PHY_DATA1) return -EIO;

  // We think we are in sync.  Now we read 16 bits of data from the PHY.
  select_window(nic, 4);
  value = 0;
  for (i = 0x8000; i; i >>= 1) {
    // Shift input up one to make room
    outpw(nic->iobase + PHYSICAL_MANAGEMENT, PHY_CLOCK);
    udelay(1);
    outpw(nic->iobase + PHYSICAL_MANAGEMENT, 0);
    udelay(1);

    phy_mgmt = inpw(nic->iobase + PHYSICAL_MANAGEMENT);

    if (phy_mgmt & PHY_DATA1) value |= i;
  }

  // Now give it a couple of clocks with nobody driving
  outpw(nic->iobase + PHYSICAL_MANAGEMENT, 0);
  for (i = 0; i < 2; i++) {
    outpw(nic->iobase + PHYSICAL_MANAGEMENT, PHY_CLOCK);
    udelay(1);
    outpw(nic->iobase + PHYSICAL_MANAGEMENT, 0);
    udelay(1);
  }

  return value;
}

int nic_eeprom_busy(struct nic *nic) {
  unsigned short status;
  unsigned long timeout;

  timeout = get_ticks() + 1 * TICKS_PER_SEC;
  while (1) {
    status = inpw(nic->iobase + EEPROM_CMD);
    if (!(status & EEPROM_BUSY)) return 0;
    if (time_after(get_ticks(), timeout)) {
      kprintf(KERN_ERR "nic: timeout reading eeprom\n");
      return -ETIMEOUT;
    }

    udelay(10);
  }
}

int nic_read_eeprom(struct nic *nic, unsigned short addr) {
  if (addr > 0x003F)  addr = (addr & 0x003F) | ((addr & 0x03C0) << 2);

  select_window(nic, 0);

  // Check for eeprom busy
  if (nic_eeprom_busy(nic) < 0) return -ETIMEOUT;

  // Issue the read eeprom data command
  outpw(nic->iobase + EEPROM_CMD, (unsigned short) (EEPROM_CMD_READ + addr));

  // Check for eeprom busy
  if (nic_eeprom_busy(nic) < 0) return -ETIMEOUT;

  // Return value read from eeprom
  return inpw(nic->iobase + EEPROM_DATA);
}

int nic_transmit(struct dev *dev, struct pbuf *p) {
  struct nic *nic = dev->privdata;
  struct pbuf *q;
  unsigned long dnlistptr;
  int i;
  struct tx_desc *entry;

  nictrace("nic: transmit packet, %d bytes, %d fragments\n", p->tot_len, pbuf_clen(p));

  // Wait for free entry in transmit ring
  if (wait_for_object(&nic->tx_sem, TX_TIMEOUT) < 0) {
    kprintf(KERN_ERR "nic: transmit timeout, drop packet\n");
    netstats->link.drop++;
    return -ETIMEOUT;
  }

  // Check for over-fragmented packets
  if (pbuf_clen(p) > TX_MAX_FRAGS) {
    p = pbuf_linearize(PBUF_RAW, p);
    netstats->link.memerr++;
    netstats->link.drop++;
    if (!p) return -ENOMEM;
  }

  // Fill tx entry
  entry = nic->next_tx;
  for (q = p, i = 0; q != NULL; q = q->next, i++) {
    entry->sglist[i].addr = virt2phys(q->payload);
    if (!q->next) {
      entry->sglist[i].length = q->len | LAST_FRAG;
    } else {
      entry->sglist[i].length = q->len;
    }
  }

  entry->header = FSH_ROUND_UP_DEFEAT | FSH_DOWN_INDICATE | FSH_TX_INDICATE | FSH_ADD_IP_CHECKSUM | FSH_ADD_TCP_CHECKSUM | FSH_ADD_UDP_CHECKSUM;
  entry->phys_next = 0;
  entry->pkt = p;

  // Move to next entry in tx_ring
  nic->next_tx = entry->next;
  nic->tx_size++;

  // Update download list
  execute_command_wait(nic, CMD_DOWN_STALL, 0);
  dnlistptr = inpd(nic->iobase + DOWN_LIST_POINTER);
  if (dnlistptr == 0) {
    nictrace("nic: set dnlist to %p\n", entry->phys_addr);
    outpd(nic->iobase + DOWN_LIST_POINTER, entry->phys_addr);
  } else {
    nictrace("nic: chaining %p to %p\n", entry->phys_addr, entry->prev->phys_next);
    entry->prev->phys_next = entry->phys_addr;
  }
  execute_command(nic, CMD_DOWN_UNSTALL, 0);

  netstats->link.xmit++;
  return 0;
}

int nic_ioctl(struct dev *dev, int cmd, void *args, size_t size) {
  return -ENOSYS;
}

int nic_attach(struct dev *dev, struct eth_addr *hwaddr) {
  struct nic *nic = dev->privdata;
  *hwaddr = nic->hwaddr;

  dev->netif->flags |= NETIF_IP_TX_CHECKSUM_OFFLOAD | NETIF_IP_RX_CHECKSUM_OFFLOAD;
  dev->netif->flags |= NETIF_UDP_RX_CHECKSUM_OFFLOAD | NETIF_UDP_TX_CHECKSUM_OFFLOAD;
  dev->netif->flags |= NETIF_TCP_RX_CHECKSUM_OFFLOAD | NETIF_TCP_TX_CHECKSUM_OFFLOAD;

  return 0;
}

int nic_detach(struct dev *dev) {
  return 0;
}

void nic_up_complete(struct nic *nic) {
  unsigned long status;
  int length;
  struct pbuf *p, *q;

  while ((status = nic->curr_rx->status) & UP_COMPLETE) {
    length = nic->curr_rx->status & 0x1FFF;

    nictrace("nic: packet received, %d bytes\n", length);

    // Check for errors
    if (status & UP_PACKET_STATUS_ERROR) {
      netstats->link.err++;

      if (status & UP_PACKET_STATUS_RUNT_FRAME) {
        kprintf(KERN_WARNING "nic: runt frame\n");
      }
      if (status & UP_PACKET_STATUS_ALIGNMENT_ERROR) {
        kprintf(KERN_WARNING "nic: alignment error\n");
        nic->stat.rx_alignment_error++;
      }
      if (status & UP_PACKET_STATUS_CRC_ERROR) {
        kprintf(KERN_WARNING "nic: crc error\n");
        nic->stat.rx_bad_crc_error++;
      }
      if (status & UP_PACKET_STATUS_OVERSIZE_FRAME) {
        kprintf(KERN_WARNING "nic: oversize frame\n");
        nic->stat.rx_oversize_error++;
      }

      nic->curr_rx->status = 0;
      nic->curr_rx = nic->curr_rx->next;
      continue;
    }

    // Check for IP checksum error
    if ((nic->curr_rx->status & UP_PACKET_STATUS_IP_CHECKSUM_CHECKED) &&
        (nic->curr_rx->status & UP_PACKET_STATUS_IP_CHECKSUM_ERROR)) {
      //kprintf("nic: ip checksum error\n");
      netstats->ip.chkerr++;
      nic->curr_rx->status = 0;
      nic->curr_rx = nic->curr_rx->next;
      continue;
    }

    // Check for UDP checksum error
    if ((nic->curr_rx->status & UP_PACKET_STATUS_UDP_CHECKSUM_CHECKED) &&
        (nic->curr_rx->status & UP_PACKET_STATUS_UDP_CHECKSUM_ERROR)) {
      //kprintf("nic: udp checksum error\n");
      netstats->udp.chkerr++;
      nic->curr_rx->status = 0;
      nic->curr_rx = nic->curr_rx->next;
      continue;
    }

    // Check for TCP checksum error
    if ((nic->curr_rx->status & UP_PACKET_STATUS_TCP_CHECKSUM_CHECKED) &&
        (nic->curr_rx->status & UP_PACKET_STATUS_TCP_CHECKSUM_ERROR)) {
      //kprintf("nic: tcp checksum error\n");
      netstats->tcp.chkerr++;
      nic->curr_rx->status = 0;
      nic->curr_rx = nic->curr_rx->next;
      continue;
    }

    if (length < RX_COPYBREAK) {
      // Allocate properly sized pbuf and copy
      p = pbuf_alloc(PBUF_RAW, length, PBUF_RW);
      if (p) {
        memcpy(p->payload, nic->curr_rx->pkt->payload, length);
      } else {
        netstats->link.memerr++;
        netstats->link.drop++;
      }
    } else {
      // Allocate new full sized pbuf and replace existing pbuf in rx ring
      q = pbuf_alloc(PBUF_RAW, ETHER_FRAME_LEN, PBUF_RW);
      if (q) {
        p = nic->curr_rx->pkt;
        pbuf_realloc(p, length);

        nic->curr_rx->pkt = q;
        nic->curr_rx->sglist[0].addr = virt2phys(q->payload);
      } else {
        p = NULL;
        netstats->link.memerr++;
        netstats->link.drop++;
      }
    }

    // Send packet to upper layer
    if (p) {
      netstats->link.recv++;
      if (dev_receive(nic->devno, p) < 0) pbuf_free(p);
    }

    // Clear status and move to next packet in rx ring
    nic->curr_rx->status = 0;
    nic->curr_rx = nic->curr_rx->next;
  }

  // Unstall upload engine
  execute_command(nic, CMD_UP_UNSTALL, 0);
}

void nic_down_complete(struct nic *nic) {
  int freed = 0;

  while (nic->tx_size > 0 && (nic->curr_tx->header & FSH_DOWN_COMPLETE)) {
    if (nic->curr_tx->pkt) {
      pbuf_free(nic->curr_tx->pkt);
      nic->curr_tx->pkt = NULL;
    }
    nic->curr_tx->prev->phys_next = 0;

    freed++;
    nic->tx_size--;
    nic->curr_tx = nic->curr_tx->next;
  }

  nictrace("nic: release %d tx entries\n", freed);

  release_sem(&nic->tx_sem, freed);
}

void nic_host_error(struct nic *nic) {
  execute_command_wait(nic, CMD_RESET, 
    GLOBAL_RESET_MASK_NETWORK_RESET |
    GLOBAL_RESET_MASK_TP_AUI_RESET |
    GLOBAL_RESET_MASK_ENDEC_RESET |
    GLOBAL_RESET_MASK_AISM_RESET |
    GLOBAL_RESET_MASK_SMB_RESET |
    GLOBAL_RESET_MASK_VCO_RESET |
    GLOBAL_RESET_MASK_UP_DOWN_RESET);
}

void nic_tx_complete(struct nic *nic) {
  unsigned char txstatus;

  txstatus = inp(nic->iobase + TX_STATUS);
  outp(nic->iobase + TX_STATUS, txstatus);

  if (txstatus & TX_STATUS_HWERROR) {
    kprintf(KERN_ERR "nic: hw error\n");
    nic->stat.tx_hw_errors++;
    nic_restart_transmitter(nic);
  } else if (txstatus & TX_STATUS_JABBER) {
    kprintf(KERN_ERR "nic: jabber error\n");
    nic->stat.tx_jabber_error++;
    nic_restart_transmitter(nic);
  } else if (txstatus & TX_STATUS_MAXIMUM_COLLISION) {
    kprintf(KERN_ERR "nic: maximum collisions\n");
    nic->stat.tx_maximum_collisions++;
    execute_command(nic, CMD_TX_ENABLE, 0);
  } else if (txstatus & 0x3F) {
    kprintf(KERN_ERR "nic: unknown errror in 0x%02X tx complete\n", txstatus);
    nic->stat.tx_unknown_error++;
    execute_command(nic, CMD_TX_ENABLE, 0);
  }
}

void nic_dpc(void *arg) {
  struct nic *nic = (struct nic *) arg;
  unsigned short status;

  // Read the status
  status = inpw(nic->iobase + STATUS);

  // Return if no interrupt - caused by shared interrupt
  if (!(status & INTSTATUS_INT_LATCH)) return;

  while (1) {
    status &= ALL_INTERRUPTS;
    if (!status) break;

    // Handle host error event
    if (status & INTSTATUS_HOST_ERROR) {
      kprintf(KERN_ERR "nic: host error\n");
      nic_host_error(nic);
    }

    // Handle tx complete event
    if (status & INTSTATUS_TX_COMPLETE) {
      nic_tx_complete(nic);
    }

    // Handle rx complete event
    if (status & INTSTATUS_RX_COMPLETE) {
      kprintf("nic: rx complete\n");
    }

    // Handle rx early event
    if (status & INTSTATUS_RX_EARLY) {
      kprintf("nic: rx early\n");
      execute_command(nic, CMD_ACKNOWLEDGE_INTERRUPT, RX_EARLY_ACK);
    }

    // Handle int requested event
    if (status & INTSTATUS_INT_REQUESTED) {
      kprintf("nic: int request\n");
      execute_command(nic, CMD_ACKNOWLEDGE_INTERRUPT, INT_REQUESTED_ACK);
    }

    // Handle update statistics event
    if (status & INTSTATUS_UPDATE_STATS) {
      update_statistics(nic);
    }

    // Handle link event
    if (status & INTSTATUS_LINK_EVENT) {
      kprintf("nic: link event\n");
    }

    // Handle download complete event
    if (status & INTSTATUS_DN_COMPLETE) {
      nic_down_complete(nic);
      execute_command(nic, CMD_ACKNOWLEDGE_INTERRUPT, DN_COMPLETE_ACK);
    }

    // Handle upload complete event
    if (status & INTSTATUS_UP_COMPLETE) {
      nic_up_complete(nic);
      execute_command(nic, CMD_ACKNOWLEDGE_INTERRUPT, UP_COMPLETE_ACK);
    }

    // Acknowledge the interrupt
    execute_command(nic, CMD_ACKNOWLEDGE_INTERRUPT, INTERRUPT_LATCH_ACK);

    // Get next status
    status = inpw(nic->iobase + STATUS);
  }

  eoi(nic->irq);
}

int nic_handler(struct context *ctxt, void *arg) {
  struct nic *nic = (struct nic *) arg;

  // Queue DPC to service interrupt
  queue_irq_dpc(&nic->dpc, nic_dpc, nic);

  return 0;
}

int nic_try_mii(struct nic *nic, unsigned short options) {
  int phy_status;

  nictrace("enter nic_try_mii\n");

  // First see if there's anything connected to the MII
  if (nic_find_mii_phy(nic) < 0) return -ENODEV;

  // Now we can read the status and try to figure out what's out there.
  phy_status = nic_read_mii_phy(nic, MII_PHY_STATUS);
  if (phy_status < 0) return phy_status;

  if ((phy_status & MII_STATUS_AUTO) && (phy_status & MII_STATUS_EXTENDED)) {
    // If it is capable of auto negotiation, see if it has been done already.
    // Check the current MII auto-negotiation state and see if we need to
    // start auto-neg over.
 
    if (nic_check_mii_configuration(nic, options) < 0) return -ENODEV;

    // See if link is up...
    phy_status = nic_read_mii_phy(nic, MII_PHY_STATUS);
    if (phy_status < 0) return phy_status;

    if (phy_status & MII_STATUS_LINK_UP) {
      if (nic_get_link_speed(nic) < 0) return -ENODEV;
      return 0;
    } else  {
      if (phy_status & MII_STATUS_100MB_MASK) {
        nic->linkspeed = 100;
      } else  {
        nic->linkspeed = 10;
      }

      return 0;
    }
  }

  return -ENODEV;
}

int nic_negotiate_link(struct nic *nic, unsigned short options) {
  nictrace("enter nic_negotiate_link\n");
  nic->connector = CONNECTOR_UNKNOWN;

  // Try 100MB Connectors
  if ((options & MEDIA_OPTIONS_100BASETX_AVAILABLE) ||
      (options & MEDIA_OPTIONS_10BASET_AVAILABLE) ||
      (options & MEDIA_OPTIONS_MII_AVAILABLE)) {
    // For 10Base-T and 100Base-TX, select autonegotiation instead of autoselect before calling trymii
    if ((options & MEDIA_OPTIONS_100BASETX_AVAILABLE) || (options & MEDIA_OPTIONS_10BASET_AVAILABLE)) {
      nic->connector = CONNECTOR_AUTONEGOTIATION;
    } else {
      nic->connector = CONNECTOR_MII;
    }

    if (nic_try_mii(nic, options) < 0) nic->connector = CONNECTOR_UNKNOWN;
  }

  // Transceiver available is 100Base-FX
  if ((options & MEDIA_OPTIONS_100BASEFX_AVAILABLE) && nic->connector == CONNECTOR_UNKNOWN) {
    //TODO: try linkbeat
    nic->connector = CONNECTOR_100BASEFX;
    nic->linkspeed = 100;
  }

  // Transceiver available is 10AUI
  if ((options & MEDIA_OPTIONS_10AUI_AVAILABLE) && nic->connector == CONNECTOR_UNKNOWN) {
    nic_setup_connector(nic, CONNECTOR_10AUI);
    //TODO: try test packet
  }

  // Transceiver available is 10Base-2
  if ((options & MEDIA_OPTIONS_10BASE2_AVAILABLE) && nic->connector == CONNECTOR_UNKNOWN) {
    nic_setup_connector(nic, CONNECTOR_10BASE2);
    //TODO: try loopback packet
    execute_command(nic, CMD_DISABLE_DC_CONVERTER, 0);
    nic_check_dc_converter(nic, 0);
  }

  // Nothing left to try!
  if (nic->connector == CONNECTOR_UNKNOWN) {
    kprintf(KERN_WARNING, "nic: no connector found\n");
    nic->connector = CONNECTOR_10BASET;
    nic->linkspeed = 10;
    return -ENODEV;
  }

  nic_setup_connector(nic, nic->connector);
  return 0;
}

int nic_program_mii(struct nic *nic, int connector) {
  int phy_control;
  int phy_status;
  unsigned short mii_type;

  nictrace("enter nic_program_mii\n");

  // First see if there's anything connected to the MII
  if (!nic_find_mii_phy(nic) < 0) return -ENODEV;

  phy_control = nic_read_mii_phy(nic, MII_PHY_CONTROL);
  if (phy_control < 0) return phy_control;

  phy_status = nic_read_mii_phy(nic, MII_PHY_STATUS);
  if (phy_status < 0) return phy_status;

  // Reads the miiSelect field in EEPROM. Program MII as the default.
  mii_type = nic->eeprom[EEPROM_SOFTWARE_INFO3];

  // If an override is present AND the transceiver type is available
  // on the card, that type will be used.
  //rc = nic_mii_media_override(nic, phy_status, &mii_type);
  //if (rc < 0) return rc;

  // If full duplex selected, set it in PhyControl.
  if (nic->fullduplex) {
    phy_control |= MII_CONTROL_FULL_DUPLEX;
  } else {
    phy_control &= ~MII_CONTROL_FULL_DUPLEX;
  }
  phy_control &= ~MII_CONTROL_ENABLE_AUTO;

  if (((mii_type & MIITXTYPE_MASK) == MIISELECT_100BTX) ||
      ((mii_type & MIITXTYPE_MASK) == MIISELECT_100BTX_ANE)) {
    phy_control |= MII_CONTROL_100MB;
    nic_write_mii_phy(nic, MII_PHY_CONTROL, (unsigned short) phy_control);
    msleep(600);
    nic->linkspeed = 100;
    return 0;
  } else if (((mii_type & MIITXTYPE_MASK ) == MIISELECT_10BT) ||
             ((mii_type & MIITXTYPE_MASK ) == MIISELECT_10BT_ANE)) {
    phy_control &= ~MII_CONTROL_100MB;
    nic_write_mii_phy(nic, MII_PHY_CONTROL, (unsigned short) phy_control);
    msleep(600);
    nic->linkspeed = 10;
    return 0;
  }

  phy_control &= ~MII_CONTROL_100MB;
  nic_write_mii_phy(nic, MII_PHY_CONTROL, (unsigned short) phy_control);
  msleep(600);
  nic->linkspeed = 10;

  return 0;
}

int nic_initialize_adapter(struct nic *nic) {
  unsigned short mac_control;
  int i;

  nictrace("enter nic_initialize_adapter\n");

  // TX engine handling
  execute_command_wait(nic, CMD_TX_DISABLE, 0);
  execute_command_wait(nic, CMD_TX_RESET, TX_RESET_MASK_NETWORK_RESET);

  // RX engine handling
  execute_command_wait(nic, CMD_RX_DISABLE, 0);
  execute_command_wait(nic, CMD_RX_RESET, RX_RESET_MASK_NETWORK_RESET);

  // Acknowledge any pending interrupts.
  execute_command(nic, CMD_ACKNOWLEDGE_INTERRUPT, ALL_ACK);
  
  // Clear the statistics from the hardware.
  execute_command(nic, CMD_STATISTICS_DISABLE, 0);
  clear_statistics(nic);

  // Get the MAC address from the EEPROM
  for (i = 0; i < ETHER_ADDR_LEN / 2; i++) {
    ((unsigned short *) nic->hwaddr.addr)[i] = htons(nic->eeprom[EEPROM_OEM_NODE_ADDRESS1 + i]);
  }

  // Set the card MAC address
  select_window(nic, 2);
  for (i = 0; i < ETHER_ADDR_LEN; i++) outp(nic->iobase + i, nic->hwaddr.addr[i]);

  outpw(nic->iobase + 0x6, 0);
  outpw(nic->iobase + 0x8, 0);
  outpw(nic->iobase + 0xA, 0);

  // Enable statistics
  execute_command(nic, CMD_STATISTICS_ENABLE, 0);

  // Clear the mac control register.
  select_window(nic, 3);
  mac_control = inpw(nic->iobase + MAC_CONTROL);
  mac_control &= 0x1;
  outpw(nic->iobase + MAC_CONTROL, mac_control);

  return 0;
}

int nic_get_link_speed(struct nic *nic) {
  int phy_anlpar;
  int phy_aner;
  int phy_anar;
  int phy_status;

  nictrace("enter nic_get_link_speed\n");

  phy_aner = nic_read_mii_phy(nic, MII_PHY_ANER);
  if (phy_aner < 0) return phy_aner;
  
  phy_anlpar = nic_read_mii_phy(nic, MII_PHY_ANLPAR);
  if (phy_anlpar < 0) return phy_anlpar;

  phy_anar = nic_read_mii_phy(nic, MII_PHY_ANAR);
  if (phy_anar < 0) return phy_anar;

  phy_status = nic_read_mii_phy(nic, MII_PHY_STATUS);
  if (phy_status < 0) return phy_status;

  // Check to see if we've completed auto-negotiation.
  if (!(phy_status & MII_STATUS_AUTO_DONE)) return -EBUSY;

  if ((phy_anar & MII_ANAR_100TXFD) && (phy_anlpar & MII_ANLPAR_100TXFD)) {
    //nic->connector = CONNECTOR_100BASETX;
    nic->linkspeed = 100;
    nic->fullduplex = 1;
  } else if ((phy_anar & MII_ANAR_100TX) && (phy_anlpar & MII_ANLPAR_100TX)) {
    //nic->connector = CONNECTOR_100BASETX;
    nic->linkspeed = 100;
    nic->fullduplex = 0;
  } else if ((phy_anar & MII_ANAR_10TFD) && (phy_anlpar & MII_ANLPAR_10TFD)) {
    //nic->connector = CONNECTOR_10BASET;
    nic->linkspeed = 10;
    nic->fullduplex = 1;
  } else if ((phy_anar & MII_ANAR_10T) && (phy_anlpar & MII_ANLPAR_10T)) {
    //nic->connector = CONNECTOR_10BASET;
    nic->linkspeed = 10;
    nic->fullduplex = 0;
  } else if (!(phy_aner & MII_ANER_LPANABLE)) {
    // Link partner is not capable of auto-negotiation. Fall back to 10HD.
    //nic->connector = CONNECTOR_10BASET;
    nic->linkspeed = 10;
    nic->fullduplex = 0;
  } else {
    return -EINVAL;
  }

  return nic->linkspeed;
}

int nic_restart_receiver(struct nic *nic) {
  nictrace("enter nic_restart_receiver\n");

  execute_command_wait(nic, CMD_RX_DISABLE, 0);
  execute_command_wait(nic, CMD_RX_RESET, RX_RESET_MASK_NETWORK_RESET);
  execute_command_wait(nic, CMD_RX_ENABLE, 0);

  return 0;
}

int nic_restart_transmitter(struct nic *nic) {
  unsigned short media_status;
  unsigned long dma_control;
  unsigned long timeout;

  nictrace("enter nic_restart_transmitter\n");

  execute_command(nic, CMD_TX_DISABLE, 0);

  // Wait for the transmit to go quiet.
  select_window(nic, 4);

  media_status = inpw(nic->iobase + MEDIA_STATUS);
  udelay(10);

  if (media_status & MEDIA_STATUS_TX_IN_PROGRESS) {
    timeout = get_ticks() + 1 * TICKS_PER_SEC;
    while (1) {
      media_status = inpw(nic->iobase + MEDIA_STATUS);
      if (!(media_status & MEDIA_STATUS_TX_IN_PROGRESS)) break;
      if (time_after(get_ticks(), timeout)) {
        kprintf(KERN_WARNING "nic: timeout waiting for transmitter to go quiet\n");
        return -ETIMEOUT;
      }
      msleep(10);
    }
  }

  // Wait for download engine to stop
  dma_control = inpd(nic->iobase + DMA_CONTROL);
  udelay(10);

  if (dma_control & DMA_CONTROL_DOWN_IN_PROGRESS) {
    timeout = get_ticks() + 1 * TICKS_PER_SEC;
    while (1) {
      dma_control = inpd(nic->iobase + DMA_CONTROL);
      if (!(dma_control & DMA_CONTROL_DOWN_IN_PROGRESS)) break;
      if (time_after(get_ticks(), timeout)) {
        kprintf(KERN_WARNING "nic: timeout waiting for download engine to stop\n");
        return -ETIMEOUT;
      }
      msleep(10);
    }
  }

  if (execute_command_wait(nic, CMD_TX_RESET, TX_RESET_MASK_DOWN_RESET) < 0) return -ETIMEOUT;
  execute_command(nic, CMD_TX_ENABLE, 0);

  return 0;
}

int nic_flow_control(struct nic *nic) {
  unsigned short phy_anar;
  unsigned short phy_control;

  nictrace("enter nic_flow_control\n");

  phy_anar = nic_read_mii_phy(nic, MII_PHY_ANAR);
  if (phy_anar < 0) return phy_anar;
  phy_anar |= MII_ANAR_FLOWCONTROL;
  nic_write_mii_phy(nic, MII_PHY_ANAR, phy_anar);

  phy_control = nic_read_mii_phy(nic, MII_PHY_CONTROL);
  if (phy_control < 0) return phy_control;
  phy_control |= MII_CONTROL_START_AUTO;
  nic_write_mii_phy(nic, MII_PHY_CONTROL, phy_control);

  return 0;
}

int nic_configure_mii(struct nic *nic, unsigned short media_options) {
  int phy_control;
  int phy_anar;
  int phy_status;
  unsigned long timeout;

  nictrace("enter nic_configure_mii\n");

  phy_control = nic_read_mii_phy(nic, MII_PHY_CONTROL);
  if (phy_control < 0) return phy_control;

  phy_anar = nic_read_mii_phy(nic, MII_PHY_ANAR);
  if (phy_anar < 0) return phy_anar;

  // Set up speed and duplex settings in MII Control and ANAR register.
  phy_anar &= ~(MII_ANAR_100TXFD | MII_ANAR_100TX  | MII_ANAR_10TFD  | MII_ANAR_10T);

  // Set up duplex.
  if (nic->fullduplex) {
    phy_control |= MII_CONTROL_FULL_DUPLEX;
  } else {
    phy_control &= ~(MII_CONTROL_FULL_DUPLEX);
  }

  // Set up flow control.
  if (nic->flowcontrol) {
    phy_anar |= MII_ANAR_FLOWCONTROL;
  } else {
    phy_anar &= ~(MII_ANAR_FLOWCONTROL);
  }

  //
  // Set up the media options. For duplex settings, if we're set to auto-select
  // then enable both half and full-duplex settings. Otherwise, go by what's
  // been enabled for duplex mode.
  //
  
  if (media_options & MEDIA_OPTIONS_100BASETX_AVAILABLE) {
    if (nic->autoselect) {
      phy_anar |= (MII_ANAR_100TXFD | MII_ANAR_100TX);
    } else if (nic->fullduplex) {
      phy_anar |= MII_ANAR_100TXFD;
    } else {
      phy_anar |= MII_ANAR_100TX;
    }
  }

  if (media_options & MEDIA_OPTIONS_10BASET_AVAILABLE) {
    if (nic->autoselect) {
      phy_anar |= (MII_ANAR_10TFD | MII_ANAR_10T);
    } else if (nic->fullduplex) {
      phy_anar |= MII_ANAR_10TFD;
    } else {
      phy_anar |= MII_ANAR_10T;
    }
  }

  // Enable and start auto-negotiation
  phy_control |= (MII_CONTROL_ENABLE_AUTO | MII_CONTROL_START_AUTO);

  // Write the MII registers back.
  nic_write_mii_phy(nic, MII_PHY_ANAR, (unsigned short) phy_anar); 
  nic_write_mii_phy(nic, MII_PHY_CONTROL, (unsigned short) phy_control); 

  // Wait for auto-negotiation to finish.
  phy_status = nic_read_mii_phy(nic, MII_PHY_STATUS);
  if (phy_status < 0) return phy_status;
  udelay(1000);

  if (!(phy_status & MII_STATUS_AUTO_DONE)) {
    timeout = get_ticks() + 3 * TICKS_PER_SEC;
    while (1) {
      phy_status = nic_read_mii_phy(nic, MII_PHY_STATUS);
      if (phy_status & MII_STATUS_AUTO_DONE) break;
      if (time_after(get_ticks(), timeout)) {
        kprintf(KERN_WARNING "nic: timeout waiting for auto-negotiation to finish\n");
        return -ETIMEOUT;
      }
      msleep(10);
    }
  }

  return 0;
}

int nic_check_mii_configuration(struct nic *nic, unsigned short media_options) {
  int phy_control;
  int phy_status;
  int phy_anar;
  int new_anar;
  int rc;

  nictrace("enter nic_check_mii_configuration\n");

  // Check to see if auto-negotiation has completed.
  phy_control = nic_read_mii_phy(nic, MII_PHY_CONTROL);
  if (phy_control < 0) return phy_control;

  phy_status = nic_read_mii_phy(nic, MII_PHY_STATUS);
  if (phy_status < 0) return phy_status;

  if (!((phy_control & MII_CONTROL_ENABLE_AUTO) && (phy_status & MII_STATUS_AUTO_DONE))) {
    // Auto-negotiation did not complete, so start it over using the new settings.
    rc = nic_configure_mii(nic, media_options);
    if (rc < 0) return rc;
  }
  
  //
  // Auto-negotiation has completed. Check the results against the ANAR and ANLPAR
  // registers to see if we need to restart auto-neg.
  //
  
  phy_anar = nic_read_mii_phy(nic, MII_PHY_ANAR);
  if (phy_anar < 0) return phy_anar;

  //
  // Check to see what we negotiated with the link partner. First, let's make
  // sure that the ANAR is set properly based on the media options defined.
  //
  
  new_anar = 0;
  if (media_options & MEDIA_OPTIONS_100BASETX_AVAILABLE) {
    if (nic->autoselect) {
      new_anar |= MII_ANAR_10TFD | MII_ANAR_10T;
    } else if (nic->fullduplex) {
      new_anar |= MII_ANAR_100TXFD;
    } else {
      new_anar |= MII_ANAR_100TX;
    }
  }

  if (media_options & MEDIA_OPTIONS_10BASET_AVAILABLE) {
    if (nic->autoselect) {
      new_anar |= MII_ANAR_100TXFD | MII_ANAR_100TX;
    } else if (nic->fullduplex) {
      new_anar |= MII_ANAR_10TFD;
    } else {
      new_anar |= MII_ANAR_10T;
    }
  }

  if (nic->fullduplex && nic->flowcontrol) new_anar |= MII_ANAR_FLOWCONTROL;

  // If the negotiated configuration hasn't changed return and don't restart auto-negotiation.
  if ((phy_anar & MII_ANAR_MEDIA_MASK) == new_anar) return 0;

  // Check the media settings.
  if (media_options & MEDIA_OPTIONS_100BASETX_AVAILABLE) {
    // Check 100BaseTX settings.
    if ((phy_anar & MII_ANAR_MEDIA_100_MASK) != (new_anar & MII_ANAR_MEDIA_100_MASK)) {
      return nic_configure_mii(nic, media_options);
    }
  }
  
  if (media_options & MEDIA_OPTIONS_10BASET_AVAILABLE) {
    // Check 10BaseT settings.
    if ((phy_anar & MII_ANAR_MEDIA_10_MASK) != (new_anar & MII_ANAR_MEDIA_10_MASK)) {
      return nic_configure_mii(nic, media_options);
    }
  }

  return 0;
}

int nic_check_mii_auto_negotiation_status(struct nic *nic) {
  int phy_status;
  int phy_anar;
  int phy_anlpar;

  nictrace("enter nic_check_mii_auto_negotiation_status\n");

  // Check to see if auto-negotiation has completed.
  phy_status = nic_read_mii_phy(nic, MII_PHY_STATUS);
  if (phy_status < 0) return phy_status;

  // If we have a valid link, so get out!
  if (phy_status & MII_STATUS_LINK_UP) return 0;

  //
  // Check to see why auto-negotiation or parallel detection has failed. We'll do this
  // by comparing the advertisement registers between the NIC and the link partner.
  //

  phy_anar = nic_read_mii_phy(nic, MII_PHY_ANAR);
  if (phy_anar < 0) return phy_anar;

  phy_anlpar = nic_read_mii_phy(nic, MII_PHY_ANLPAR);
  if (phy_anlpar < 0) return phy_anlpar;

  //
  // Now, compare what was advertised between the NIC and it's link partner.
  // If the media bits don't match, then write an error log entry.
  //
  if ((phy_anar & MII_ANAR_MEDIA_MASK) != (phy_anlpar & MII_ANAR_MEDIA_MASK)) {
    kprintf(KERN_ERR "nic: incompatible configuration\n");
    return -EINVAL;
  }

  return 0;
}

int nic_check_dc_converter(struct nic *nic, int enabled) {
  unsigned long timeout;
  unsigned short media_status;

  nictrace("enter nic_check_dc_converter\n");

  media_status = inpw(nic->iobase + MEDIA_STATUS);
  udelay(1000);

  if (enabled && !(media_status & MEDIA_STATUS_DC_CONVERTER_ENABLED) ||
      !enabled && (media_status & MEDIA_STATUS_DC_CONVERTER_ENABLED)) {
    timeout = get_ticks() + 3; // 30 ms
    while (1) {
      media_status = inpw(nic->iobase + MEDIA_STATUS);
      if (enabled && (media_status & MEDIA_STATUS_DC_CONVERTER_ENABLED)) break;
      if (!enabled && !(media_status & MEDIA_STATUS_DC_CONVERTER_ENABLED)) break;
      if (time_after(get_ticks(), timeout)) {
        kprintf(KERN_ERR "nic: timeout waiting for dc converter to go %s\n", enabled ? "on" : "off");
        return -ETIMEOUT;
      }
      msleep(10);
    }
  }

  return 0;
}

int nic_setup_connector(struct nic *nic, int connector) {
  unsigned long internal_config;
  unsigned long old_internal_config;
  unsigned short media_status;

  nictrace("enter nic_setup_connector\n");

  select_window(nic, 3);

  internal_config = inpd(nic->iobase + INTERNAL_CONFIG);
  old_internal_config = internal_config;

  // Program the MII registers if forcing the configuration to 10/100BaseT.
  if (connector == CONNECTOR_10BASET || connector == CONNECTOR_100BASETX) {
    // Clear transceiver type and change to new transceiver type.
    internal_config &= ~(INTERNAL_CONFIG_TRANSCEIVER_MASK);
    internal_config |= (CONNECTOR_AUTONEGOTIATION << 20);

    // Update the internal config register. Only do this if the value has
    // changed to avoid dropping link.
    if (old_internal_config != internal_config) {
      outpd(nic->iobase + INTERNAL_CONFIG, internal_config);
    }

    // Force the MII registers to the correct settings.
    if (nic_check_mii_configuration(nic, (unsigned short) (connector == CONNECTOR_100BASETX ? MEDIA_OPTIONS_100BASETX_AVAILABLE : MEDIA_OPTIONS_10BASET_AVAILABLE)) < 0) {
      // If the forced configuration didn't work, check the results and see why.
      return nic_check_mii_auto_negotiation_status(nic);
    }
  } else {
    // Clear transceiver type and change to new transceiver type
    internal_config = internal_config & (~INTERNAL_CONFIG_TRANSCEIVER_MASK);
    internal_config |= (connector << 20);

    // Update the internal config register. Only do this if the value has
    // changed to avoid dropping link.
    if (old_internal_config != internal_config) {
      outpd(nic->iobase + INTERNAL_CONFIG, internal_config);
    }
  }

  //
  // Determine whether to set enableSQEStats and linkBeatEnable
  // Automatically set JabberGuardEnable in MediaStatus register.
  //
  select_window(nic, 4);

  media_status = inpw(nic->iobase + MEDIA_STATUS);
  media_status &= ~(MEDIA_STATUS_SQE_STATISTICS_ENABLE | MEDIA_STATUS_LINK_BEAT_ENABLE | MEDIA_STATUS_JABBER_GUARD_ENABLE);
  media_status |= MEDIA_STATUS_JABBER_GUARD_ENABLE;

  if (connector == CONNECTOR_10AUI) media_status |= MEDIA_STATUS_SQE_STATISTICS_ENABLE;

  if (connector == CONNECTOR_AUTONEGOTIATION) {
    media_status |= MEDIA_STATUS_LINK_BEAT_ENABLE;
  } else {
    if (connector == CONNECTOR_10BASET || connector == CONNECTOR_100BASETX || connector == CONNECTOR_100BASEFX) {
      if (!(nic->eeprom[EEPROM_SOFTWARE_INFO1] & LINK_BEAT_DISABLE)) {
        media_status |= MEDIA_STATUS_LINK_BEAT_ENABLE;
      }
    }
  }

  outpw(nic->iobase + MEDIA_STATUS, media_status);

  //
  // If configured for coax we must start the internal transceiver.
  // If not, we stop it (in case the configuration changed across a
  // warm boot).
  //
  if (connector == CONNECTOR_10BASE2) {
    execute_command(nic, CMD_ENABLE_DC_CONVERTER, 0);
    nic_check_dc_converter(nic, 1);
  } else {
    execute_command(nic, CMD_DISABLE_DC_CONVERTER, 0);
    nic_check_dc_converter(nic, 0);
  }

  return 0;
}

int nic_setup_media(struct nic *nic) {
  unsigned short options_available;
  unsigned long internal_config;
  unsigned short mac_control;
  int rc;

  nictrace("enter nic_setup_media\n");

  // If this is a 10mb Lightning card, assume that the 10FL bit is
  // set in the media options register
  if (nic->deviceid == 0x900A) {
    options_available = MEDIA_OPTIONS_10BASEFL_AVAILABLE;
  } else {
    // Read the MEDIA OPTIONS to see what connectors are available
    select_window(nic, 3);
    options_available = inpw(nic->iobase + MEDIA_OPTIONS);
  }

  // Get internal config from EEPROM since reset invalidates the normal register value.
  internal_config = nic->eeprom[EEPROM_INTERNAL_CONFIG0] | (nic->eeprom[EEPROM_INTERNAL_CONFIG1] << 16);

  //
  // Read the current value of the InternalConfig register. If it's different
  // from the EEPROM values, than write it out using the EEPROM values.
  // This is done since a global reset may invalidate the register value on
  // some ASICs. Also, writing to InternalConfig may reset the PHY on some ASICs.
  //

  select_window(nic, 3);

  if (internal_config != inpd(nic->iobase + INTERNAL_CONFIG)) {
    outpd(nic->iobase + INTERNAL_CONFIG, internal_config);
  }

  // Get the connector to use.
  if (nic->connector == CONNECTOR_UNKNOWN) {
    nic->connector = (internal_config & INTERNAL_CONFIG_TRANSCEIVER_MASK) >> INTERNAL_CONFIG_TRANSCEIVER_SHIFT;
    if (internal_config & INTERNAL_CONFIG_AUTO_SELECT) nic->autoselect = 1;
  }

  // If auto selection of connector was specified, do it now...
  if (nic->connector == CONNECTOR_AUTONEGOTIATION) {
    execute_command(nic, CMD_STATISTICS_DISABLE, 0);
    nic_negotiate_link(nic, options_available);
  } else {
    // MII connector needs to be initialized and the data rates
    // set up even in the non-autoselect case
    if (nic->connector == CONNECTOR_MII) {
      nic_program_mii(nic, CONNECTOR_MII);
    } else {
      if (nic->connector == CONNECTOR_100BASEFX || nic->connector == CONNECTOR_100BASETX) {
        nic->linkspeed = 100;
      } else {
        nic->linkspeed = 10;
      }
    }

    nic_setup_connector(nic, nic->connector);
  }

  //
  // Check link speed and duplex settings before doing anything else.
  // If the call succeeds, we know the link is up, so we'll update the
  // link state.
  //
  if (nic_get_link_speed(nic) < 0) return -EIO;
  
  // Set up duplex mode
  select_window(nic, 3);
  mac_control = inpw(nic->iobase +  MAC_CONTROL);
  if (nic->fullduplex) {
    // Set Full duplex in MacControl register
    mac_control |= MAC_CONTROL_FULL_DUPLEX_ENABLE;

    // Since we're switching to full duplex, enable flow control.
    mac_control |=  MAC_CONTROL_FLOW_CONTROL_ENABLE;
    nic->flowcontrol = 1;
  } else {
    // Set Half duplex in MacControl register
    mac_control &= ~MAC_CONTROL_FULL_DUPLEX_ENABLE;

    // Since we're switching to half duplex, disable flow control
    mac_control &= ~ MAC_CONTROL_FLOW_CONTROL_ENABLE;
  }
  outpw(nic->iobase + MAC_CONTROL, mac_control);

  // Reset and enable transmitter
  rc = nic_restart_transmitter(nic);
  if (rc < 0) return rc;

  // Reset and enable receiver
  rc = nic_restart_receiver(nic);
  if (rc < 0) return rc;

  //
  // This is for advertisement of flow control.  We only need to
  // call this if the adapter is using flow control, in Autoselect
  // mode and not a Tornado board.
  //
  if (nic->autoselect && nic->flowcontrol && nic->deviceid != 0x9200 && nic->deviceid != 0x9201 && nic->deviceid != 0x9805) {
    nic_flow_control(nic);
  }

  return 0;
}

int nic_software_work(struct nic *nic) {
  unsigned long dma_control;
  unsigned short net_diag;
  int phy_reg;

  nictrace("enter nic_software_work\n");

  if (!(nic->eeprom[EEPROM_SOFTWARE_INFO2] & ENABLE_MWI_WORK)) {
    dma_control = inpd(nic->iobase + DMA_CONTROL);
    outpd(nic->iobase + DMA_CONTROL, dma_control | DMA_CONTROL_DEFEAT_MWI);
  }

  select_window(nic, 4);
  net_diag = inpw(nic->iobase + NETWORK_DIAGNOSTICS);

  if ((((net_diag & NETWORK_DIAGNOSTICS_ASIC_REVISION) >> 4) == 1) &&
      (((net_diag & NETWORK_DIAGNOSTICS_ASIC_REVISION_LOW) >> 1) < 4)) {
    phy_reg = nic_read_mii_phy(nic, 24);
    phy_reg |= 1;
    nic_write_mii_phy(nic, 24, (unsigned short) phy_reg);
  }

  return 0;
}

int nic_setup_buffers(struct nic *nic) {
  int i;

  nictrace("enter nic_setup_buffers\n");

  // Setup the receive ring
  for (i = 0; i < RX_RING_SIZE; i++) {
    if (i == RX_RING_SIZE - 1) {
      nic->rx_ring[i].next = &nic->rx_ring[0];
    } else {
      nic->rx_ring[i].next = &nic->rx_ring[i + 1];
    }

    nic->rx_ring[i].pkt = pbuf_alloc(PBUF_RAW, ETHER_FRAME_LEN, PBUF_RW);
    if (!nic->rx_ring[i].pkt) return -ENOMEM;

    nic->rx_ring[i].phys_next = virt2phys(nic->rx_ring[i].next);
    nic->rx_ring[i].sglist[0].addr = virt2phys(nic->rx_ring[i].pkt->payload);
    nic->rx_ring[i].sglist[0].length = ETHER_FRAME_LEN | LAST_FRAG;
  }
  nic->curr_rx = &nic->rx_ring[0];

  // Setup the transmit ring
  for (i = 0; i < TX_RING_SIZE; i++) {
    if (i == TX_RING_SIZE - 1) {
      nic->tx_ring[i].next = &nic->tx_ring[0];
    } else {
      nic->tx_ring[i].next = &nic->tx_ring[i + 1];
    }

    if (i == 0) {
      nic->tx_ring[i].prev = &nic->tx_ring[TX_RING_SIZE - 1];
    } else {
      nic->tx_ring[i].prev = &nic->tx_ring[i - 1];
    }

    nic->tx_ring[i].phys_addr = virt2phys(&nic->tx_ring[i]);
  }

  nic->next_tx = &nic->tx_ring[0];
  nic->next_tx->header |= FSH_DPD_EMPTY;

  nic->tx_size = 0;
  nic->curr_tx = &nic->tx_ring[0];

  return 0;
}

int nic_start_adapter(struct nic *nic) {
  unsigned short diagnostics;
  unsigned long dma_control;

  nictrace("enter nic_start_adapter\n");

  // Enable upper bytes counting in diagnostics register.
  select_window(nic, 4);

  diagnostics = inpw(nic->iobase + NETWORK_DIAGNOSTICS);
  diagnostics |= NETWORK_DIAGNOSTICS_UPPER_BYTES_ENABLE;
  outpw(nic->iobase + NETWORK_DIAGNOSTICS, diagnostics);

  // Enable counter speed in DMA control.
  dma_control = inpd(nic->iobase + DMA_CONTROL);
  if (nic->linkspeed == 100) dma_control |= DMA_CONTROL_COUNTER_SPEED;
  outpd(nic->iobase + DMA_CONTROL, dma_control);

  // Give receive ring to upload engine
  execute_command_wait(nic, CMD_UP_STALL, 0);
  outpd(nic->iobase + UP_LIST_POINTER, virt2phys(nic->curr_rx));
  execute_command(nic, CMD_UP_UNSTALL, 0);

  // Give transmit ring to download engine
  init_sem(&nic->tx_sem, TX_RING_SIZE);
  execute_command_wait(nic, CMD_DOWN_STALL, 0);
  outpd(nic->iobase + DOWN_LIST_POINTER, 0);
  execute_command(nic, CMD_DOWN_UNSTALL, 0);

  // Set receive filter
  execute_command(nic, CMD_SET_RX_FILTER, RECEIVE_INDIVIDUAL /*| RECEIVE_MULTICAST*/ | RECEIVE_BROADCAST);

  // Enable the statistics back.
  execute_command(nic, CMD_STATISTICS_ENABLE, 0);

  // Acknowledge any pending interrupts.
  execute_command(nic, CMD_ACKNOWLEDGE_INTERRUPT, ALL_ACK);

  // Enable indication for all interrupts.
  execute_command(nic, CMD_SET_INDICATION_ENABLE, ALL_INTERRUPTS);

  // Enable all interrupts to the host.
  execute_command(nic, CMD_SET_INTERRUPT_ENABLE, ALL_INTERRUPTS);

  // Enable the transmit and receive engines.
  execute_command(nic, CMD_RX_ENABLE, 0);
  execute_command(nic, CMD_TX_ENABLE, 0);

  // Delay three seconds, only some switches need this
  //msleep(3000);

  nictrace("exit nic_start_adapter\n");

  return 0;
}

int __declspec(dllexport) install(struct unit *unit) {
  struct nic *nic;
  int i;
  int value;
  int rc;

  // Check for PCI device
  if (unit->bus->bustype != BUSTYPE_PCI) return -EINVAL;

  // Determine chipset
  switch (unit->unitcode) {
    case UNITCODE_3C905B1: 
      unit->vendorname = "3Com";
      unit->productname = "3Com EtherLink 3C905B-1";
      break;

    case UNITCODE_3C905C:
      unit->vendorname = "3Com";
      unit->productname = "3Com EtherLink 3C905C";
      break;

    case UNITCODE_3C9051:
      unit->vendorname = "3Com";
      unit->productname = "3Com EtherLink 3C905-1";
      break;

    default:
      unit->vendorname = "3Com";
      unit->productname = "3Com EtherLink 3C90xC";
  }

  // Allocate device structure
  nic = (struct nic *) kmalloc(sizeof(struct nic));
  if (!nic) return -ENOMEM;
  memset(nic, 0, sizeof(struct nic));

  // Setup NIC configuration
  nic->iobase = (unsigned short) get_unit_iobase(unit);
  nic->irq = (unsigned short) get_unit_irq(unit);
  nic->deviceid = PCI_DEVICE_ID(unit->unitcode);
  nic->connector = CONNECTOR_UNKNOWN;
  nic->devno = dev_make("eth#", &nic_driver, unit, nic);

  // Enable bus mastering
  pci_enable_busmastering(unit);

  // Install interrupt handler
  init_dpc(&nic->dpc);
  register_interrupt(&nic->intr, IRQ2INTR(nic->irq), nic_handler, nic);
  enable_irq(nic->irq);

  // Setup buffers
  rc = nic_setup_buffers(nic);
  if (rc < 0) return rc;

  // Global reset
  rc = execute_command_wait(nic, CMD_RESET, 
         GLOBAL_RESET_MASK_TP_AUI_RESET | 
         GLOBAL_RESET_MASK_ENDEC_RESET | 
         GLOBAL_RESET_MASK_AISM_RESET | 
         GLOBAL_RESET_MASK_SMB_RESET | 
         GLOBAL_RESET_MASK_VCO_RESET);

  if (rc < 0) {
    kprintf(KERN_ERR, "nic: wait asic ready timeout\n");
    return -EIO;
  }

  // Read the EEPROM
  for (i = 0; i < EEPROM_SIZE; i++) {
    value = nic_read_eeprom(nic, (unsigned short) i);
    if (value < 0) return value;
    nic->eeprom[i] = value;
  }

  // Initialize adapter
  rc = nic_initialize_adapter(nic);
  if (rc < 0) return rc;

  // Setup media
  rc = nic_setup_media(nic);
  if (rc < 0) return rc;

  // Software work
  rc = nic_software_work(nic);
  if (rc < 0) return rc;

  // Start adapter
  rc = nic_start_adapter(nic);
  if (rc < 0) return rc;

  register_proc_inode(device(nic->devno)->name, nicstat_proc, nic);

  kprintf(KERN_INFO "%s: %s, iobase 0x%x irq %d mac %la\n", device(nic->devno)->name, unit->productname, nic->iobase, nic->irq, &nic->hwaddr);
  kprintf(KERN_INFO "%s: %d MBits/s, %s-duplex, %s\n", device(nic->devno)->name, nic->linkspeed, nic->fullduplex ? "full" : "half", connectorname[nic->connector]);

  return 0;
}

int __stdcall start(hmodule_t hmod, int reason, void *reserved2) {
  netstats = get_netstats();
  return 1;
}
