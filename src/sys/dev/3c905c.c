//
// 3c905c.c
//
// Copyright (c) 2002 Søren Thygesen Gjesse. All rights reserved.
//
// 3Com 3C905C NIC network driver
//

#include <os/krnl.h>
#include "3c905c.h"

char *connectorname[] = {"10Base-T", "AUI", "n/a", "BNC", "100Base-TX", "100Base-FX", "MII", "n/a", "Auto", "Ext-MII"};

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
  char filler[8];
};

struct tx_desc
{
  unsigned long phys_next;
  unsigned long header;
  struct sg_entry sglist[TX_MAX_FRAGS];
  struct tx_desc *next;
  struct tx_desc *prev;
  struct pbuf *pkt;
  unsigned long phys_addr;
  char filler[8];
};

struct nicstat
{
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

struct nic
{
  struct rx_desc rx_ring[RX_RING_SIZE];
  struct tx_desc tx_ring[TX_RING_SIZE];
  struct rx_desc *curr_rx;              // Next entry to receive from rx ring
  struct tx_desc *curr_tx;              // Next entry to reclaim from tx ring
  struct tx_desc *next_tx;              // Next entry to add in tx ring

  struct sem tx_sem;                    // Semaphore for tx ring
  int tx_size;                          // Number of active entries in transmit list

  devno_t devno;                        // Device number
  unsigned long deviceid;               // PCI device id

  unsigned short iobase;		// Configured I/O base
  unsigned short irq;		        // Configured IRQ

  int autoselect;                       // Auto-negotiate
  int connector;                        // Active connector
  int linkspeed;                        // Link speed in mbits/s
  int fullduplex;                       // Full duplex link
  int flowcontrol;                      // Flow control

  struct eth_addr hwaddr;               // MAC address for NIC

  struct dpc dpc;                       // DPC for driver

  struct nicstat stat;                  // NIC statistics
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
  kprintf(" windowNumber: %d\n", status & INTSTATUS_WINDOW_NUMBER >> 13);
}

__inline void execute_command(struct nic *nic, int cmd, int param)
{
  _outpw(nic->iobase + CMD, (unsigned short) (cmd + param));
}

int execute_command_wait(struct nic *nic, int cmd, int param)
{
  int i;

  execute_command(nic, cmd, param);
  for (i = 0; i < 100000; i++)
  {
    if (!(_inpw(nic->iobase + STATUS) & INTSTATUS_CMD_IN_PROGRESS)) return 0;
    usleep(10);
  }

  for (i = 0; i < 200; i++)
  {
    if (!(_inpw(nic->iobase + STATUS) & INTSTATUS_CMD_IN_PROGRESS)) return 0;
    sleep(10);
  }

  kprintf("nic: command did not complete\n");
  return -ETIMEOUT;
}

__inline void select_window(struct nic *nic, int window)
{
  execute_command(nic, CMD_SELECT_WINDOW, window);
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
  unsigned long rx_frames;
  unsigned long tx_frames;
  unsigned long rx_bytes;
  unsigned long tx_bytes;
  unsigned char upper;
  
  // Read the current window
  current_window = _inpw(nic->iobase + STATUS) >> 13;

  // Read statistics from window 6
  select_window(nic, 6);

  nic->stat.tx_sqe_errors += _inp(nic->iobase + SQE_ERRORS);
  nic->stat.tx_multiple_collisions += _inp(nic->iobase + MULTIPLE_COLLISIONS);
  nic->stat.tx_single_collisions += _inp(nic->iobase + SINGLE_COLLISIONS);
  nic->stat.rx_overruns += _inp(nic->iobase + RX_OVERRUNS);
  nic->stat.tx_carrier_lost += _inp(nic->iobase + CARRIER_LOST);
  nic->stat.tx_late_collisions += _inp(nic->iobase + LATE_COLLISIONS);
  nic->stat.tx_frames_deferred += _inp(nic->iobase + FRAMES_DEFERRED);

  // Frames received/transmitted
  rx_frames = _inp(nic->iobase + FRAMES_RECEIVED_OK);
  tx_frames = _inp(nic->iobase + FRAMES_XMITTED_OK);
  upper = _inp(nic->iobase + UPPER_FRAMES_OK);
  rx_frames += (upper & 0x0F) << 8;
  tx_frames += (upper & 0xF0) << 4;

  nic->stat.rx_frames_ok += rx_frames;
  nic->stat.tx_frames_ok += tx_frames;

  // Bytes received/transmitted - upper part added below from window 4
  rx_bytes = _inpw(nic->iobase + BYTES_RECEIVED_OK);
  tx_bytes = _inpw(nic->iobase + BYTES_XMITTED_OK);

  // Read final statistics from window 4
  select_window(nic, 4);

  // Update bytes received/transmitted with upper part
  upper = _inp(nic->iobase + UPPER_BYTES_OK);
  rx_bytes += (upper & 0x0F) << 16;
  tx_bytes += (upper & 0xF0) << 12;

  nic->stat.rx_bytes_ok += rx_bytes;
  nic->stat.tx_bytes_ok += tx_bytes;

  nic->stat.rx_bad_ssd += _inp(nic->iobase + BAD_SSD);

  // Set the window to its previous value
  select_window(nic, current_window);
}

int nicstat_proc(struct proc_file *pf, void *arg)
{
  struct nic *nic = arg;

  update_statistics(nic);

  pprintf(pf, "Frames transmitted... : %ul\n", nic->stat.tx_frames_ok);
  pprintf(pf, "Bytes transmitted.... : %ul\n", nic->stat.tx_bytes_ok);
  pprintf(pf, "Frames deferred...... : %ul\n", nic->stat.tx_frames_deferred);
  pprintf(pf, "Single collisions.... : %ul\n", nic->stat.tx_single_collisions);
  pprintf(pf, "Multiple collisions.. : %ul\n", nic->stat.tx_multiple_collisions);
  pprintf(pf, "Late collisions...... : %ul\n", nic->stat.tx_late_collisions);
  pprintf(pf, "Carrier lost......... : %ul\n", nic->stat.tx_carrier_lost);
  pprintf(pf, "Maximum collisions .. : %ul\n", nic->stat.tx_maximum_collisions);
  pprintf(pf, "SQE errors........... : %ul\n", nic->stat.tx_sqe_errors);
  pprintf(pf, "HW errors............ : %ul\n", nic->stat.tx_hw_errors);
  pprintf(pf, "Jabber errors........ : %ul\n", nic->stat.tx_jabber_error);
  pprintf(pf, "Unknown errors....... : %ul\n", nic->stat.tx_unknown_error);
  pprintf(pf, "Frames received...... : %ul\n", nic->stat.rx_frames_ok);
  pprintf(pf, "Bytes received....... : %ul\n", nic->stat.rx_bytes_ok);
  pprintf(pf, "Overruns............. : %ul\n", nic->stat.rx_overruns);
  pprintf(pf, "Bad SSD.............. : %ul\n", nic->stat.rx_bad_ssd);
  pprintf(pf, "Allignment errors.... : %ul\n", nic->stat.rx_alignment_error);
  pprintf(pf, "CRC errors........... : %ul\n", nic->stat.rx_bad_crc_error);
  pprintf(pf, "Oversized frames..... : %ul\n", nic->stat.rx_oversize_error);

  return 0;
}

void nic_send_mii_phy_preamble(struct nic *nic)
{
  int i;

  // Set up and send the preamble, a sequence of 32 "1" bits
  select_window(nic, 4);
  _outpw(nic->iobase + PHYSICAL_MANAGEMENT, PHY_WRITE);

  for (i = 0; i < 32; i++) 
  {
    _outpw(nic->iobase + PHYSICAL_MANAGEMENT, PHY_WRITE | PHY_DATA1);
    _outpw(nic->iobase + PHYSICAL_MANAGEMENT, PHY_WRITE | PHY_DATA1 | PHY_CLOCK);
    usleep(1);
    _outpw(nic->iobase + PHYSICAL_MANAGEMENT, PHY_WRITE);
    usleep(1);
  }
}

void nic_write_mii_phy(struct nic *nic, unsigned short reg, unsigned short value)
{
  int i,j;
  unsigned short writecmd[2];

  writecmd[0] = MII_PHY_ADDRESS_WRITE;
  writecmd[1] = 0;

  nic_send_mii_phy_preamble(nic);

  // Bits 2..6 of the command word specify the register
  writecmd[0] |= (reg & 0x1F) << 2;
  writecmd[1] = value;

  select_window(nic, 4);

  for (i = 0; i < 2; i++) 
  {
    for (j = 0x8000; j; j >>= 1) 
    {

      if (writecmd[i] & j) 
      {
        _outpw(nic->iobase + PHYSICAL_MANAGEMENT, PHY_WRITE | PHY_DATA1);
        _outpw(nic->iobase + PHYSICAL_MANAGEMENT, PHY_WRITE | PHY_DATA1 | PHY_CLOCK);
      }
      else 
      {
        _outpw(nic->iobase + PHYSICAL_MANAGEMENT, PHY_WRITE);
        _outpw(nic->iobase + PHYSICAL_MANAGEMENT, PHY_WRITE | PHY_CLOCK);
      }
      usleep(1);
      _outpw(nic->iobase + PHYSICAL_MANAGEMENT, PHY_WRITE);
      usleep(1);
    }
  }

  // Now give it a couple of clocks with nobody driving
  _outpw(nic->iobase + PHYSICAL_MANAGEMENT, 0);
  for (i = 0; i < 2; i++) 
  {
    _outpw(nic->iobase + PHYSICAL_MANAGEMENT, PHY_CLOCK);
    usleep(1);
    _outpw(nic->iobase + PHYSICAL_MANAGEMENT, 0);
    usleep(1);
  }
}

int nic_read_mii_phy(struct nic *nic, unsigned short reg)
{
  unsigned short  phy_mgmt = 0;
  unsigned short read_cmd;
  unsigned short value;
  int i;

  read_cmd = MII_PHY_ADDRESS_READ;

  nic_send_mii_phy_preamble(nic);
  
  // Bits 2..6 of the command word specify the register
  read_cmd |= (reg & 0x1F) << 2;

  select_window(nic, 4);

  for (i = 0x8000; i > 2; i >>= 1) 
  {
    if (read_cmd & i) 
    {
      _outpw(nic->iobase + PHYSICAL_MANAGEMENT, PHY_WRITE | PHY_DATA1);
      _outpw(nic->iobase + PHYSICAL_MANAGEMENT, PHY_WRITE | PHY_DATA1 | PHY_CLOCK);
    }
    else 
    {
      _outpw(nic->iobase + PHYSICAL_MANAGEMENT, PHY_WRITE);
      _outpw(nic->iobase + PHYSICAL_MANAGEMENT, PHY_WRITE | PHY_CLOCK);
    }

    usleep(1);
    _outpw(nic->iobase + PHYSICAL_MANAGEMENT, PHY_WRITE);
    usleep(1);
  }

  // Now run one clock with nobody driving
  _outpw(nic->iobase + PHYSICAL_MANAGEMENT, 0);
  _outpw(nic->iobase + PHYSICAL_MANAGEMENT, PHY_CLOCK);
  usleep(1);
  _outpw(nic->iobase + PHYSICAL_MANAGEMENT, 0);
  usleep(1);

  // Now run one clock, expecting the PHY to be driving a 0 on the data
  // line.  If we read a 1, it has to be just his pull-up, and he's not
  // responding.

  phy_mgmt = _inpw(nic->iobase + PHYSICAL_MANAGEMENT);
  if (phy_mgmt & PHY_DATA1) return -EIO;

  // We think we are in sync.  Now we read 16 bits of data from the PHY.
  select_window(nic, 4);
  value = 0;
  for (i = 0x8000; i; i >>= 1) 
  {
    // Shift input up one to make room
    _outpw(nic->iobase + PHYSICAL_MANAGEMENT, PHY_CLOCK);
    usleep(1);
    _outpw(nic->iobase + PHYSICAL_MANAGEMENT, 0);
    usleep(1);

    phy_mgmt = _inpw(nic->iobase + PHYSICAL_MANAGEMENT);

    if (phy_mgmt & PHY_DATA1) value |= i;
  }

  // Now give it a couple of clocks with nobody driving
  _outpw(nic->iobase + PHYSICAL_MANAGEMENT, 0);
  for (i = 0; i < 2; i++) 
  {
    _outpw(nic->iobase + PHYSICAL_MANAGEMENT, PHY_CLOCK);
    usleep(1);
    _outpw(nic->iobase + PHYSICAL_MANAGEMENT, 0);
    usleep(1);
  }

  return value;
}

int nic_eeprom_busy(struct nic *nic)
{
  unsigned short status;
  unsigned long timeout;

  timeout = ticks + 1 * TICKS_PER_SEC;
  while (1)
  {
    status = _inpw(nic->iobase + EEPROM_CMD);
    if (!(status & EEPROM_BUSY)) return 0;
    if (time_after(ticks, timeout))
    {
      kprintf("nic: timeout reading eeprom\n");
      return -ETIMEOUT;
    }

    usleep(10);
  }
}

int nic_read_eeprom(struct nic *nic, unsigned short addr)
{
  if (addr > 0x003F)  addr = (addr & 0x003F) | ((addr & 0x03C0) << 2);

  select_window(nic, 0);

  // Check for eeprom busy
  if (nic_eeprom_busy(nic) < 0) return -ETIMEOUT;

  // Issue the read eeprom data command
  _outpw(nic->iobase + EEPROM_CMD, (unsigned short) (EEPROM_CMD_READ + addr));

  // Check for eeprom busy
  if (nic_eeprom_busy(nic) < 0) return -ETIMEOUT;

  // Return value read from eeprom
  return _inpw(nic->iobase + EEPROM_DATA);
}

int nic_transmit(struct dev *dev, struct pbuf *p)
{
  struct nic *nic = dev->privdata;
  struct pbuf *q;
  unsigned long dnlistptr;
  int i;
  struct tx_desc *entry;

  //kprintf("nic: transmit packet, %d bytes, %d fragments\n", p->tot_len, pbuf_clen(p));

  // Wait for free entry in transmit ring
  if (wait_for_object(&nic->tx_sem, TX_TIMEOUT) < 0)
  {
    kprintf("nic: transmit timeout, drop packet\n");
    stats.link.drop++;
    return -ETIMEOUT;
  }

  // Check for over-fragmented packets
  if (pbuf_clen(p) > TX_MAX_FRAGS)
  {
    p = pbuf_linearize(PBUF_RAW, p);
    stats.link.memerr++;
    stats.link.drop++;
    if (!p) return -ENOMEM;
  }

  // Fill tx entry
  entry = nic->next_tx;
  for (q = p, i = 0; q != NULL; q = q->next, i++) 
  {
    entry->sglist[i].addr = (unsigned long) virt2phys(q->payload);
    if (!q->next) 
      entry->sglist[i].length = q->len | LAST_FRAG;
    else
      entry->sglist[i].length = q->len;
  }

  entry->header = FSH_ROUND_UP_DEFEAT | FSH_DOWN_INDICATE | FSH_TX_INDICATE | FSH_ADD_IP_CHECKSUM | FSH_ADD_TCP_CHECKSUM | FSH_ADD_UDP_CHECKSUM;
  entry->phys_next = 0;
  entry->pkt = p;

  // Move to next entry in tx_ring
  nic->next_tx = entry->next;
  nic->tx_size++;

  // Update download list
  execute_command_wait(nic, CMD_DOWN_STALL, 0);
  dnlistptr = _inpd(nic->iobase + DOWN_LIST_POINTER);
  if (dnlistptr == 0)
    _outpd(nic->iobase + DOWN_LIST_POINTER, entry->phys_addr);
  else
    entry->prev->phys_next = entry->phys_addr;
  execute_command(nic, CMD_DOWN_UNSTALL, 0);

  stats.link.xmit++;
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

  dev->netif->flags |= NETIF_IP_TX_CHECKSUM_OFFLOAD | NETIF_IP_RX_CHECKSUM_OFFLOAD;
  dev->netif->flags |= NETIF_UDP_RX_CHECKSUM_OFFLOAD | NETIF_UDP_TX_CHECKSUM_OFFLOAD;
  dev->netif->flags |= NETIF_TCP_RX_CHECKSUM_OFFLOAD | NETIF_TCP_TX_CHECKSUM_OFFLOAD;

  return 0;
}

int nic_detach(struct dev *dev)
{
  return 0;
}

void nic_up_complete(struct nic *nic)
{
  unsigned long status;
  int length;
  struct pbuf *p, *q;

  while ((status = nic->curr_rx->status) & UP_COMPLETE)
  {
    length = nic->curr_rx->status & 0x1FFF;

    //kprintf("nic: packet received, %d bytes\n", length);

    // Check for errors
    if (status & UP_PACKET_STATUS_ERROR)
    {
      stats.link.err++;

      if (status & UP_PACKET_STATUS_RUNT_FRAME) 
      {
	kprintf("nic: runt frame\n");
      }
      if (status & UP_PACKET_STATUS_ALIGNMENT_ERROR) 
      {
	kprintf("nic: alignment error\n");
	nic->stat.rx_alignment_error++;
      }
      if (status & UP_PACKET_STATUS_CRC_ERROR) 
      {
	kprintf("nic: crc error\n");
	nic->stat.rx_bad_crc_error++;
      }
      if (status & UP_PACKET_STATUS_OVERSIZE_FRAME)
      {
	kprintf("nic: oversize frame\n");
	nic->stat.rx_oversize_error++;
      }

      nic->curr_rx->status = 0;
      nic->curr_rx = nic->curr_rx->next;
      continue;
    }

    // Check for IP checksum error
    if ((nic->curr_rx->status & UP_PACKET_STATUS_IP_CHECKSUM_CHECKED) &&
        (nic->curr_rx->status & UP_PACKET_STATUS_IP_CHECKSUM_ERROR))
    {
      kprintf("nic: ip checksum error\n");
      stats.ip.chkerr++;
      nic->curr_rx->status = 0;
      nic->curr_rx = nic->curr_rx->next;
      continue;
    }

    // Check for UDP checksum error
    if ((nic->curr_rx->status & UP_PACKET_STATUS_UDP_CHECKSUM_CHECKED) &&
        (nic->curr_rx->status & UP_PACKET_STATUS_UDP_CHECKSUM_ERROR))
    {
      kprintf("nic: udp checksum error\n");
      stats.udp.chkerr++;
      nic->curr_rx->status = 0;
      nic->curr_rx = nic->curr_rx->next;
      continue;
    }

    // Check for TCP checksum error
    if ((nic->curr_rx->status & UP_PACKET_STATUS_TCP_CHECKSUM_CHECKED) &&
        (nic->curr_rx->status & UP_PACKET_STATUS_TCP_CHECKSUM_ERROR))
    {
      kprintf("nic: tcp checksum error\n");
      stats.tcp.chkerr++;
      nic->curr_rx->status = 0;
      nic->curr_rx = nic->curr_rx->next;
      continue;
    }

    if (length < RX_COPYBREAK)
    {
      // Allocate properly sized pbuf and copy
      p = pbuf_alloc(PBUF_RAW, length, PBUF_RW);
      if (p)
      {
	memcpy(p->payload, nic->curr_rx->pkt->payload, length);
      }
      else
      {
	stats.link.memerr++;
	stats.link.drop++;
      }
    }
    else
    {
      // Allocate new full sized pbuf and existing replace pbuf in rx ring
      q = pbuf_alloc(PBUF_RAW, ETHER_FRAME_LEN, PBUF_RW);
      if (q)
      {
	p = nic->curr_rx->pkt;
	pbuf_realloc(p, length);

	nic->curr_rx->pkt = q;
	nic->curr_rx->sglist[0].addr = (unsigned long) virt2phys(q->payload);
      }
      else
      {
	p = NULL;
	stats.link.memerr++;
	stats.link.drop++;
      }
    }

    // Send packet to upper layer
    if (p)
    {
      stats.link.recv++;
      if (dev_receive(nic->devno, p) < 0) pbuf_free(p);
    }

    // Clear status and move to next packet in rx ring
    nic->curr_rx->status = 0;
    nic->curr_rx = nic->curr_rx->next;
  }

  // Unstall upload engine
  execute_command(nic, CMD_UP_UNSTALL, 0);
}

void nic_down_complete(struct nic *nic)
{
  int freed = 0;

  while (nic->tx_size > 0 && (nic->curr_tx->header & FSH_DOWN_COMPLETE))
  {
    if (nic->curr_tx->pkt)
    {
      pbuf_free(nic->curr_tx->pkt);
      nic->curr_tx->pkt = NULL;
    }
    nic->curr_tx->prev->phys_next = 0;

    freed++;
    nic->tx_size--;
    nic->curr_tx = nic->curr_tx->next;
  }

  //kprintf("nic: release %d tx entries\n", freed);

  release_sem(&nic->tx_sem, freed);
}

void nic_host_error(struct nic *nic)
{
  execute_command_wait(nic, CMD_RESET, 
    GLOBAL_RESET_MASK_NETWORK_RESET |
    GLOBAL_RESET_MASK_TP_AUI_RESET |
    GLOBAL_RESET_MASK_ENDEC_RESET |
    GLOBAL_RESET_MASK_AISM_RESET |
    GLOBAL_RESET_MASK_SMB_RESET |
    GLOBAL_RESET_MASK_VCO_RESET |
    GLOBAL_RESET_MASK_UP_DOWN_RESET);
}

void nic_tx_complete(struct nic *nic)
{
  unsigned char txstatus;

  txstatus = _inp(nic->iobase + TX_STATUS);
  _outp(nic->iobase + TX_STATUS, txstatus);

  if (txstatus & TX_STATUS_HWERROR) 
  {
    kprintf("nic: hw error\n");
    nic->stat.tx_hw_errors++;
    //TODO: reset and enable transmitter
  }
  else if (txstatus & TX_STATUS_JABBER) 
  {
    kprintf("nic: jabber error\n");
    nic->stat.tx_jabber_error++;
    //TODO: reset and enable transmitter
  }
  else if (txstatus & TX_STATUS_MAXIMUM_COLLISION)
  {
    kprintf("nic: maximum collisions\n");
    nic->stat.tx_maximum_collisions++;
    execute_command(nic, CMD_TX_ENABLE, 0);
  }
  else if (txstatus & 0x3F)
  {
    kprintf("nic: unknown errror in 0x%02X tx complete\n", txstatus);
    nic->stat.tx_unknown_error++;
    execute_command(nic, CMD_TX_ENABLE, 0);
  }
}

void nic_dpc(void *arg)
{
  struct nic *nic = (struct nic *) arg;
  unsigned short status;

  // Read the status
  status = _inpw(nic->iobase + STATUS);

  // Return if no interrupt - caused by shared interrupt
  if (!(status & INTSTATUS_INT_LATCH)) return;

  while (1)
  {
    //dump_dump_status(status);

    status &= ALL_INTERRUPTS;
    if (!status) break;

    // Handle host error event
    if (status & INTSTATUS_HOST_ERROR)
    {
      kprintf("nic: host error\n");
      nic_host_error(nic);
    }

    // Handle tx complete event
    if (status & INTSTATUS_TX_COMPLETE)
    {
      nic_tx_complete(nic);
    }

    // Handle rx complete event
    if (status & INTSTATUS_RX_COMPLETE)
    {
      kprintf("nic: rx complete\n");
    }

    // Handle rx early event
    if (status & INTSTATUS_RX_EARLY)
    {
      kprintf("nic: rx early\n");
      execute_command(nic, CMD_ACKNOWLEDGE_INTERRUPT, RX_EARLY_ACK);
    }

    // Handle int requested event
    if (status & INTSTATUS_INT_REQUESTED)
    {
      kprintf("nic: int request\n");
      execute_command(nic, CMD_ACKNOWLEDGE_INTERRUPT, INT_REQUESTED_ACK);
    }

    // Handle update statistics event
    if (status & INTSTATUS_UPDATE_STATS)
    {
      update_statistics(nic);
    }

    // Handle link event
    if (status & INTSTATUS_LINK_EVENT)
    {
      kprintf("nic: link event\n");
    }

    // Handle download complete event
    if (status & INTSTATUS_DN_COMPLETE)
    {
      nic_down_complete(nic);
      execute_command(nic, CMD_ACKNOWLEDGE_INTERRUPT, DN_COMPLETE_ACK);
    }

    // Handle upload complete event
    if (status & INTSTATUS_UP_COMPLETE)
    {
      nic_up_complete(nic);
      execute_command(nic, CMD_ACKNOWLEDGE_INTERRUPT, UP_COMPLETE_ACK);
    }

    // Acknowledge the interrupt
    execute_command(nic, CMD_ACKNOWLEDGE_INTERRUPT, INTERRUPT_LATCH_ACK);

    // Get next status
    status = _inpw(nic->iobase + STATUS);
  }

  eoi(nic->irq);
}

void nic_handler(struct context *ctxt, void *arg)
{
  struct nic *nic = (struct nic *) arg;

  // Queue DPC to service interrupt
  queue_irq_dpc(&nic->dpc, nic_dpc, nic);
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

int nic_negotiate_link(struct nic *nic, unsigned short options_available)
{
  unsigned long internal_config;
  int connector;
  int control;
  int status;
  int anar;
  int anlpar;
  int i;
  unsigned short mac_control;

  select_window(nic, 3);
  internal_config = _inpd(nic->iobase + INTERNAL_CONFIG);
  connector = (internal_config & INTERNAL_CONFIG_TRANSCEIVER_MASK) >> INTERNAL_CONFIG_TRANSCEIVER_SHIFT;

  if (connector != CONNECTOR_AUTONEGOTIATION)
  {
    kprintf("nic: not configured for auto-negotiate, connector %d\n", connector);
    nic->connector = CONNECTOR_10BASET;
    nic->fullduplex = 0;
    nic->linkspeed = 10;
    return 0;
  }

  status = nic_read_mii_phy(nic, MII_PHY_STATUS);
  if (status < 0)
  {
    kprintf("nic: mii not responding\n");
    return -EIO;
  }

  control = nic_read_mii_phy(nic, MII_PHY_CONTROL);
  anar = nic_read_mii_phy(nic, MII_PHY_ANAR);

  control |= MII_CONTROL_ENABLE_AUTO | MII_CONTROL_START_AUTO;

  nic_write_mii_phy(nic, MII_PHY_ANAR, (unsigned short) anar);
  nic_write_mii_phy(nic, MII_PHY_CONTROL, (unsigned short) control);

  status = nic_read_mii_phy(nic, MII_PHY_STATUS);
  if (!(status & MII_STATUS_AUTO_DONE))
  {
    for (i = 0; i < 30; i++)
    {
      sleep(100);
      status = nic_read_mii_phy(nic, MII_PHY_STATUS);
      if (status & MII_STATUS_AUTO_DONE) break;
    }

    if (!(status & MII_STATUS_AUTO_DONE))
    {
      kprintf("nic: timeout wait for auto-negotiation to complete\n");
      return -ETIMEOUT;
    }
  }

  anar = nic_read_mii_phy(nic, MII_PHY_ANAR);
  anlpar = nic_read_mii_phy(nic, MII_PHY_ANLPAR);

  if ((anar & MII_ANAR_100TXFD) && (anlpar & MII_ANLPAR_100TXFD))
  {
    nic->connector = CONNECTOR_100BASETX;
    nic->fullduplex = 1;
    nic->linkspeed = 100;
  }
  else if ((anar & MII_ANAR_100TX) && (anlpar & MII_ANLPAR_100TX))
  {
    nic->connector = CONNECTOR_100BASETX;
    nic->fullduplex = 0;
    nic->linkspeed = 100;
  }
  else if ((anar & MII_ANAR_10TFD) && (anlpar & MII_ANLPAR_10TFD))
  {
    nic->connector = CONNECTOR_10BASET;
    nic->fullduplex = 1;
    nic->linkspeed = 10;
  }
  else if ((anar & MII_ANAR_10T) && (anlpar & MII_ANLPAR_10T))
  {
    nic->connector = CONNECTOR_10BASET;
    nic->fullduplex = 0;
    nic->linkspeed = 10;
  }
  else
  {
    kprintf("nic: unable to determine negotiated link\n");
    return -EIO;
  }

  if (nic->fullduplex)
  {
    select_window(nic, 3);
    mac_control = _inpw(nic->iobase + MAC_CONTROL);
    mac_control |= MAC_CONTROL_FULL_DUPLEX_ENABLE;
    //mac_control |=  MAC_CONTROL_FLOW_CONTROL_ENABLE;
    _outpw(nic->iobase + MAC_CONTROL, mac_control);
  }

  return 0;
}

int nic_program_mmi(struct nic *nic, int connector)
{
  return -ENOSYS;
}

int nic_initialize_adapter(struct nic *nic)
{
  unsigned short mac_control;
  int i;

  // TX engine handling
  execute_command_wait(nic, CMD_TX_DISABLE, 0);
  execute_command_wait(nic, CMD_TX_RESET, TX_RESET_MASK_NETWORK_RESET);

  // RX engine handling
  execute_command_wait(nic, CMD_RX_DISABLE, 0);
  execute_command_wait(nic, CMD_TX_RESET, RX_RESET_MASK_NETWORK_RESET);

  // Acknowledge any pending interrupts.
  execute_command(nic, CMD_ACKNOWLEDGE_INTERRUPT, ALL_ACK);
  
  // Clear the statistics from the hardware.
  execute_command(nic, CMD_STATISTICS_DISABLE, 0);
  clear_statistics(nic);

  // Get the MAC address from the EEPROM
  for (i = 0; i < ETHER_ADDR_LEN / 2; i++)
    ((unsigned short *) nic->hwaddr.addr)[i] = htons(nic->eeprom[EEPROM_OEM_NODE_ADDRESS1 + i]);

  // Set the card MAC address
  select_window(nic, 2);
  for (i = 0; i < ETHER_ADDR_LEN; i++) _outp(nic->iobase + i, nic->hwaddr.addr[i]);

  _outpw(nic->iobase + 0x6, 0);
  _outpw(nic->iobase + 0x8, 0);
  _outpw(nic->iobase + 0xA, 0);

  // Enable statistics
  execute_command(nic, CMD_STATISTICS_ENABLE, 0);

  // Clear the mac control register.
  select_window(nic, 3);
  mac_control = _inpw(nic->iobase + MAC_CONTROL);
  mac_control &= 0x1;
  _outpw(nic->iobase + MAC_CONTROL, mac_control);

  return 0;
}

int nic_get_link_speed(struct nic *nic)
{
  int phy_anlpar;
  int phy_aner;
  int phy_anar;
  int phy_status;

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

  if ((phy_anar & MII_ANAR_100TXFD) && (phy_anlpar & MII_ANLPAR_100TXFD)) 
  {
    //pAdapter->Hardware.MIIPhyUsed = MII_100TXFD;
    nic->linkspeed = 100;
    nic->fullduplex = 1;
  }
  else if ((phy_anar & MII_ANAR_100TX) && (phy_anlpar & MII_ANLPAR_100TX)) 
  {
    //pAdapter->Hardware.MIIPhyUsed = MII_100TX ;
    nic->linkspeed = 100;
    nic->fullduplex = 0;
  }
  else if ((phy_anar & MII_ANAR_10TFD) && (phy_anlpar & MII_ANLPAR_10TFD)) 
  {
    //pAdapter->Hardware.MIIPhyUsed = MII_10TFD ;
    nic->linkspeed = 10;
    nic->fullduplex = 1;
  }
  else if ((phy_anar & MII_ANAR_10T) && (phy_anlpar & MII_ANLPAR_10T)) 
  {
    //pAdapter->Hardware.MIIPhyUsed = MII_10T;
    nic->linkspeed = 10;
    nic->fullduplex = 0;
  }
  else if (!(phy_aner & MII_ANER_LPANABLE))
  {
    // Link partner is not capable of auto-negotiation. Fall back to 10HD.
    
    //pAdapter->Hardware.MIIPhyUsed = MII_10T ;
    nic->linkspeed = 10;
    nic->fullduplex = 0;
  }
  else
    return -EINVAL;

  return nic->linkspeed;
}

int nic_restart_receiver(struct nic *nic)
{
  execute_command_wait(nic, CMD_RX_DISABLE, 0);
  execute_command_wait(nic, CMD_RX_RESET, RX_RESET_MASK_NETWORK_RESET);
  execute_command_wait(nic, CMD_RX_ENABLE, 0);

  return 0;
}

int nic_restart_transmitter(struct nic *nic)
{
  unsigned short media_status;
  unsigned long dma_control;
  unsigned long timeout;

  execute_command(nic, CMD_TX_DISABLE, 0);

  // Wait for the transmit to go quiet.
  select_window(nic, 4);

  media_status = _inpw(nic->iobase + MEDIA_STATUS);
  usleep(10);

  if (media_status & MEDIA_STATUS_TX_IN_PROGRESS)
  {
    timeout = ticks + 1 * TICKS_PER_SEC;
    while (1)
    {
      media_status = _inpw(nic->iobase + MEDIA_STATUS);
      if (!(media_status & MEDIA_STATUS_TX_IN_PROGRESS)) break;
      if (time_after(ticks, timeout))
      {
	kprintf("nic: timeout waiting for transmitter to go quiet\n");
	return -ETIMEOUT;
      }
      sleep(10);
    }
  }

  // Wait for download engine to stop
  dma_control = _inpd(nic->iobase + DMA_CONTROL);
  usleep(10);

  if (dma_control & DMA_CONTROL_DOWN_IN_PROGRESS)
  {
    timeout = ticks + 1 * TICKS_PER_SEC;
    while (1)
    {
      dma_control = _inpd(nic->iobase + DMA_CONTROL);
      if (!(dma_control & DMA_CONTROL_DOWN_IN_PROGRESS)) break;
      if (time_after(ticks, timeout))
      {
	kprintf("nic: timeout waiting for download engine to stop\n");
	return -ETIMEOUT;
      }
      sleep(10);
    }
  }

  if (execute_command_wait(nic, CMD_TX_RESET, TX_RESET_MASK_DOWN_RESET) < 0) return -ETIMEOUT;
  execute_command(nic, CMD_TX_ENABLE, 0);

  return 0;
}

int nic_flow_control(struct nic *nic)
{
  unsigned short phy_anar;
  unsigned short phy_control;

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

int nic_configure_mii(struct nic *nic, unsigned short media_options)
{
  return -ENOSYS;
}

int nic_check_mii_configuration(struct nic *nic, unsigned short media_options)
{
#if 0
  int phy_control;
  int phy_status;
  int phy_anar;
  int temp_anar;
  int rc;

  //
  // Check to see if auto-negotiation has completed. Check the results
  // in the control and status registers.
  //

  phy_control = nic_read_mii_phy(nic, MII_PHY_CONTROL);
  if (phy_control < 0) return phy_control;

  phy_status = nic_read_mii_phy(nic, MII_PHY_STATUS);
  if (phy_status < 0) return phy_status;

  if (!((phy_control & MII_CONTROL_ENABLE_AUTO) && (phy_status & MII_STATUS_AUTO_DONE))) 
  {
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
  
  temp_anar = 0;
  if (media_options & MEDIA_OPTIONS_100BASETX_AVAILABLE) 
  {
    if (pAdapter->Hardware.AutoSelect) 
    {
	    tempAnar |= MII_ANAR_100TXFD | MII_ANAR_100TX;
    }
    else 
    {
	    if (pAdapter->Hardware.FullDuplexEnable)
		    tempAnar |= MII_ANAR_100TXFD;
	    else
		    tempAnar |= MII_ANAR_100TX;
    }
  }

  if (MediaOptions & MEDIA_OPTIONS_10BASET_AVAILABLE) 
  {
    if (pAdapter->Hardware.AutoSelect)
	    tempAnar |= MII_ANAR_10TFD | MII_ANAR_10T;
    else 
    {
	    if (pAdapter->Hardware.FullDuplexEnable)
		    tempAnar |= MII_ANAR_10TFD;
	    else
		    tempAnar |= MII_ANAR_10T;
    }
  }

  if ( pAdapter->Hardware.FullDuplexEnable && pAdapter->Hardware.FlowControlSupported ) tempAnar |= MII_ANAR_FLOWCONTROL;

  if ((PhyAnar & MII_ANAR_MEDIA_MASK) == tempAnar) 
  {
    //
    // The negotiated configuration hasn't changed.
    // So, return and don't restart auto-negotiation.
    //
    return TRUE;
  }

  //
  // Check the media settings.
  //
  if (MediaOptions & MEDIA_OPTIONS_100BASETX_AVAILABLE) 
  {
    //
    // Check 100BaseTX settings.
    //
    if ((PhyAnar & MII_ANAR_MEDIA_100_MASK) != (tempAnar & MII_ANAR_MEDIA_100_MASK)) {
    DBGPRINT_INITIALIZE(("CheckMIIConfiguration: Re-Initiating autonegotiation...\n"));
	    return ConfigureMII(pAdapter,MediaOptions);
    }
  }
  
  if (MediaOptions & MEDIA_OPTIONS_10BASET_AVAILABLE) 
  {
    //
    // Check 10BaseT settings.
    //
    if ((PhyAnar & MII_ANAR_MEDIA_10_MASK) != (tempAnar & MII_ANAR_MEDIA_10_MASK)) {
    DBGPRINT_INITIALIZE(("CheckMIIConfiguration: Re-Initiating autonegotiation...\n"));
	    return ConfigureMII(pAdapter,MediaOptions);
    }
  }

  return TRUE;
#endif
  return -ENOSYS;
}

int nic_setup_connector(struct nic *nic, int connector)
{
#if 0
  unsigned long internal_config;
  unsigned long old_internal_config;
  unsigned short media_status;

  select_window(nic, 3);

  internal_config = _inpd(nic->iobase + INTERNAL_CONFIG);
  old_internal_config = internal_config;

  // Program the MII registers if forcing the configuration to 10/100BaseT.
  if (connector == CONNECTOR_10BASET) || connector == CONNECTOR_100BASETX)
  {
    // Clear transceiver type and change to new transceiver type.
    internal_config &= ~(INTERNAL_CONFIG_TRANSCEIVER_MASK);
    internal_config |= (CONNECTOR_AUTONEGOTIATION << 20);

    // Update the internal config register. Only do this if the value has
    // changed to avoid dropping link.
    if (old_internal_config != internal_config)
    {
      _outpd(nic->iobase + INTERNAL_CONFIG, internal_config);
    }

    // Force the MII registers to the correct settings.
    if (nic_check_mii_configuration(nic, (unsigned short) (connector == CONNECTOR_100BASETX ? MEDIA_OPTIONS_100BASETX_AVAILABLE : MEDIA_OPTIONS_10BASET_AVAILABLE) < 0) 
    {
      // If the forced configuration didn't work, check the results and see why.
      nic_check_mii__auto_negotiation_status(nic);
      return 0;
    }
  }
  else 
  {
    // Clear transceiver type and change to new transceiver type
    internal_config = internal_config & (~INTERNAL_CONFIG_TRANSCEIVER_MASK);
    internal_config |= (connector << 20);

    // Update the internal config register. Only do this if the value has
    // changed to avoid dropping link.
    if (old_internal_config != internal_config)
    {
      _outpd(nic->iobase + INTERNAL_CONFIG, internal_config);
    }
  }

  //
  // Determine whether to set enableSQEStats and linkBeatEnable
  // Automatically set JabberGuardEnable in MediaStatus register.
  //
  select_window(nic, 4);

  media_status = _inpw(nic->iobase + MEDIA_STATUS);
  media_status &= ~(MEDIA_STATUS_SQE_STATISTICS_ENABLE | MEDIA_STATUS_LINK_BEAT_ENABLE | MEDIA_STATUS_JABBER_GUARD_ENABLE);
  media_status |= MEDIA_STATUS_JABBER_GUARD_ENABLE;

  if (connector == CONNECTOR_10AUI) media_status |= MEDIA_STATUS_SQE_STATISTICS_ENABLE;

  if (connector == CONNECTOR_AUTONEGOTIATION)
    MediaStatus |= MEDIA_STATUS_LINK_BEAT_ENABLE;
  else 
  {
    if (connector == CONNECTOR_10BASET || connector == CONNECTOR_100BASETX || connector == CONNECTOR_100BASEFX)
    {
      if (nic->eeprom[EEPROM_SOFTWARE_INFO1] & 
	    if (!pAdapter->Hardware.LinkBeatDisable)
		    MediaStatus |= MEDIA_STATUS_LINK_BEAT_ENABLE;
    }
  }
  NIC_WRITE_PORT_USHORT(pAdapter, MEDIA_STATUS_REGISTER, MediaStatus);

  DBGPRINT_INITIALIZE((
	  "tc90x_SetupConnector: MediaStatus = %x \n",MediaStatus));
  //
  // If configured for coax we must start the internal transceiver.
  // If not, we stop it (in case the configuration changed across a
  // warm boot).
  //
  if (NewConnector == CONNECTOR_10BASE2) {
	  NIC_COMMAND(pAdapter, COMMAND_ENABLE_DC_CONVERTER);
	  //
	  // Check if DC converter has been enabled
	  //
	  tc90x_CheckDCConverter(pAdapter, TRUE);
  }
  else {
	  NIC_COMMAND(pAdapter, COMMAND_DISABLE_DC_CONVERTER);
	  //
	  // Check if DC converter has been disabled
	  //
	  tc90x_CheckDCConverter(pAdapter, FALSE);
  }
#endif
  return 0;
}

int nic_setup_media(struct nic *nic)
{
  unsigned short options_available;
  unsigned long internal_config;
  unsigned short mac_control;
  int rc;

  // If this is a 10mb Lightning card, assume that the 10FL bit is
  // set in the media options register
  if (nic->deviceid == 0x900A) 
  {
    options_available = MEDIA_OPTIONS_10BASEFL_AVAILABLE;
  }
  else 
  {
    // Read the MEDIA OPTIONS to see what connectors are available
    select_window(nic, 3);
    options_available = _inpw(nic->iobase + MEDIA_OPTIONS);
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

  if (internal_config != _inpd(nic->iobase + INTERNAL_CONFIG))
  {
    _outpd(nic->iobase + INTERNAL_CONFIG, internal_config);
  }

  // Get the connector to use.
  if (nic->connector == CONNECTOR_UNKNOWN) 
  {
    nic->connector = (internal_config & INTERNAL_CONFIG_TRANSCEIVER_MASK) >> INTERNAL_CONFIG_TRANSCEIVER_SHIFT;
  }

  // If auto selection of connector was specified, do it now...
  if (nic->connector == CONNECTOR_AUTONEGOTIATION) 
  {
    nic->autoselect = 1;
    execute_command(nic, CMD_STATISTICS_DISABLE, 0);
    nic_negotiate_link(nic, options_available);
  }
  else 
  {
    // MII connector needs to be initialized and the data rates
    // set up even in the non-autoselect case
    if (nic->connector == CONNECTOR_MII)
    {
      nic_program_mmi(nic, CONNECTOR_MII);
    }
    else 
    {
      if (nic->connector == CONNECTOR_100BASEFX || nic->connector == CONNECTOR_100BASETX) 
	nic->linkspeed = 100;
      else
	nic->linkspeed = 10;
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
  mac_control = _inpw(nic->iobase +  MAC_CONTROL);
  if (nic->fullduplex) 
  {
    // Set Full duplex in MacControl register
    mac_control |= MAC_CONTROL_FULL_DUPLEX_ENABLE;

    // Since we're switching to full duplex, enable flow control.
    mac_control |=  MAC_CONTROL_FLOW_CONTROL_ENABLE;
    nic->flowcontrol = 1;
  }
  else 
  {
    // Set Half duplex in MacControl register
    mac_control &= ~MAC_CONTROL_FULL_DUPLEX_ENABLE;

    // Since we're switching to half duplex, disable flow control
    mac_control &= ~ MAC_CONTROL_FLOW_CONTROL_ENABLE;
  }
  _outpw(nic->iobase + MAC_CONTROL, mac_control);

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
  if (nic->autoselect && nic->flowcontrol && nic->deviceid != 0x9200 && nic->deviceid != 0x9201 && nic->deviceid != 0x9805) 
  {
    nic_flow_control(nic);
  }

  return 0;
}

int __declspec(dllexport) install(struct unit *unit)
{
  struct nic *nic;
  int i;
  int value;
  int rc;

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

    case UNITCODE_3C905TX:
      unit->vendorname = "3Com";
      unit->productname = "3Com EtherLink 3C905-TX";
      break;

    default:
      unit->vendorname = "3Com";
      unit->productname = "3Com EtherLink 3c90xC";
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

  // Enable bus mastering
  pci_enable_busmastering(unit);

  // Install interrupt handler
  set_interrupt_handler(IRQ2INTR(nic->irq), nic_handler, nic);
  enable_irq(nic->irq);

  // Global reset
  rc = execute_command_wait(nic, CMD_RESET, 
         GLOBAL_RESET_MASK_TP_AUI_RESET | 
         GLOBAL_RESET_MASK_ENDEC_RESET | 
	 GLOBAL_RESET_MASK_AISM_RESET | 
	 GLOBAL_RESET_MASK_SMB_RESET | 
	 GLOBAL_RESET_MASK_VCO_RESET);

  if (rc < 0)
  {
    kprintf("nic: wait asic ready timeout\n");
    return -EIO;
  }

  // Read the EEPROM
  for (i = 0; i < EEPROM_SIZE; i++) 
  {
    value = nic_read_eeprom(nic, (unsigned short) i);
    if (value < 0) return value;
    nic->eeprom[i] = value;
  }

  // Initialize adapter
  rc = nic_initialize_adapter(nic);
  if (rc < 0) return rc;

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

  // Setup the transmit ring
  for (i = 0; i < TX_RING_SIZE; i++)
  {
    if (i == TX_RING_SIZE - 1)
      nic->tx_ring[i].next = &nic->tx_ring[0];
    else
      nic->tx_ring[i].next = &nic->tx_ring[i + 1];

    if (i == 0)
      nic->tx_ring[i].prev = &nic->tx_ring[TX_RING_SIZE - 1];
    else
      nic->tx_ring[i].prev = &nic->tx_ring[i - 1];

    nic->tx_ring[i].phys_addr = (unsigned long) virt2phys(&nic->tx_ring[i]);
  }

  nic->next_tx = &nic->tx_ring[0];
  nic->next_tx->header |= FSH_DPD_EMPTY;

  nic->tx_size = 0;
  nic->curr_tx = &nic->tx_ring[0];

  init_sem(&nic->tx_sem, TX_RING_SIZE);
  execute_command_wait(nic, CMD_DOWN_STALL, 0);
  _outpd(nic->iobase + DOWN_LIST_POINTER, 0);
  execute_command(nic, CMD_DOWN_UNSTALL, 0);

  // Auto-negotiate link
  rc = nic_negotiate_link(nic, 0);
  if (rc < 0) return rc;

  // Set receive filter
  execute_command(nic, CMD_SET_RX_FILTER, RECEIVE_INDIVIDUAL | RECEIVE_MULTICAST | RECEIVE_BROADCAST);

  // Enable indication for all interrupts.
  execute_command(nic, CMD_SET_INDICATION_ENABLE, ALL_INTERRUPTS);

  // Enable all interrupts to the host.
  execute_command(nic, CMD_SET_INTERRUPT_ENABLE, ALL_INTERRUPTS);
  _inpw(nic->iobase + STATUS);

  // Enable statistics
  clear_statistics(nic);
  execute_command(nic, CMD_STATISTICS_ENABLE, 0);

  // Enable the transmit and receive engines.
  execute_command(nic, CMD_RX_ENABLE, 0);
  execute_command(nic, CMD_TX_ENABLE, 0);

  nic->devno = dev_make("nic#", &nic_driver, unit, nic);
  register_proc_inode(device(nic->devno)->name, nicstat_proc, nic);

  kprintf("%s: %s, iobase 0x%x irq %d hwaddr %la\n", device(nic->devno)->name, unit->productname, nic->iobase, nic->irq, &nic->hwaddr);
  kprintf("%s: %d MBits/s, %s-duplex, %s\n", device(nic->devno)->name, nic->linkspeed, nic->fullduplex ? "full" : "half", connectorname[nic->connector]);

  return 0;
}

int __stdcall start(hmodule_t hmod, int reason, void *reserved2)
{
  return 1;
}
