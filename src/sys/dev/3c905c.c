//
// 3c905c.c
//
// Copyright (c) 2002 S�ren Thygesen Gjesse. All rights reserved.
//
// 3Com 3C905C NIC network driver
//

#include <os/krnl.h>

#define ETHER_FRAME_LEN         1544
#define EEPROM_SIZE             0x21
#define RX_COPYBREAK            128
#define TX_TIMEOUT              5000

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
#define CMD_STATISTICS_ENABLE      0xA800
#define CMD_STATISTICS_DISABLE     0xB000
#define CMD_ACKNOWLEDGE_INTERRUPT  0x6800
#define CMD_UP_STALL		   0x3000
#define CMD_UP_UNSTALL		   0x3001
#define CMD_DOWN_STALL		   0x3002
#define CMD_DOWN_UNSTALL	   0x3003

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
#define FRAMES_RECEIVED_OK    0x07
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
// TX status flags
//

#define TX_STATUS_MAXIMUM_COLLISION	(1 << 3)
#define TX_STATUS_HWERROR		(1 << 4)
#define TX_STATUS_JABBER		(1 << 5)
#define TX_STATUS_INTERRUPT_REQUESTED	(1 << 6)
#define TX_STATUS_COMPLETE		(1 << 7)

//
// Global reset flags
//

#define GLOBAL_RESET_MASK_TP_AUI_RESET	(1 << 0)
#define GLOBAL_RESET_MASK_ENDEC_RESET   (1 << 1)
#define GLOBAL_RESET_MASK_NETWORK_RESET	(1 << 2)
#define GLOBAL_RESET_MASK_FIFO_RESET    (1 << 3)
#define GLOBAL_RESET_MASK_AISM_RESET    (1 << 4)
#define GLOBAL_RESET_MASK_HOST_RESET	(1 << 5)
#define GLOBAL_RESET_MASK_SMB_RESET     (1 << 6)
#define GLOBAL_RESET_MASK_VCO_RESET     (1 << 7)
#define GLOBAL_RESET_MASK_UP_DOWN_RESET (1 << 8)

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

#define ALL_ACK	                   0x07FF   

//
// RxFilter
//

#define RECEIVE_INDIVIDUAL        0x01
#define RECEIVE_MULTICAST         0x02
#define RECEIVE_BROADCAST         0x04
#define RECEIVE_ALL_FRAMES        0x08
#define RECEIVE_MULTICAST_HASH    0x10

//
// UpStatus
//

#define UP_PACKET_STATUS_ERROR			(1 << 14)
#define UP_PACKET_STATUS_COMPLETE		(1 << 15)
#define UP_PACKET_STATUS_OVERRUN		(1 << 16)
#define UP_PACKET_STATUS_RUNT_FRAME		(1 << 17)
#define UP_PACKET_STATUS_ALIGNMENT_ERROR	(1 << 18)
#define UP_PACKET_STATUS_CRC_ERROR             	(1 << 19)
#define UP_PACKET_STATUS_OVERSIZE_FRAME        	(1 << 20)
#define UP_PACKET_STATUS_DRIBBLE_BITS		(1 << 23)
#define UP_PACKET_STATUS_OVERFLOW		(1 << 24)
#define UP_PACKET_STATUS_IP_CHECKSUM_ERROR	(1 << 25)
#define UP_PACKET_STATUS_TCP_CHECKSUM_ERROR	(1 << 26)
#define UP_PACKET_STATUS_UDP_CHECKSUM_ERROR	(1 << 27)
#define UP_PACKET_STATUS_IMPLIED_BUFFER_ENABLE	(1 << 28)
#define UP_PACKET_STATUS_IP_CHECKSUM_CHECKED	(1 << 29)
#define UP_PACKET_STATUS_TCP_CHECKSUM_CHECKED	(1 << 30)
#define UP_PACKET_STATUS_UDP_CHECKSUM_CHECKED	(1 << 31)
#define UP_PACKET_STATUS_ERROR_MASK		0x1F0000

//
// Frame Start Header
//
#define FSH_CRC_APPEND_DISABLE		        (1 << 13)
#define FSH_TX_INDICATE			        (1 << 15)
#define FSH_DOWN_COMPLETE		        (1 << 16)
#define FSH_LAST_KEEP_ALIVE_PACKET	        (1 << 24)
#define FSH_ADD_IP_CHECKSUM		        (1 << 25)
#define FSH_ADD_TCP_CHECKSUM		        (1 << 26)
#define FSH_ADD_UDP_CHECKSUM		        (1 << 27)
#define FSH_ROUND_UP_DEFEAT		        (1 << 28)
#define FSH_DPD_EMPTY			        (1 << 29)
#define FSH_DOWN_INDICATE		        (1 << 31)

//
// Internal Config
//

#define INTERNAL_CONFIG_DISABLE_BAD_SSD		(1 << 8)
#define INTERNAL_CONFIG_ENABLE_TX_LARGE		(1 << 14)
#define INTERNAL_CONFIG_ENABLE_RX_LARGE		(1 << 15)
#define INTERNAL_CONFIG_AUTO_SELECT		(1 << 24)
#define INTERNAL_CONFIG_DISABLE_ROM		(1 << 25)

#define INTERNAL_CONFIG_TRANSCEIVER_MASK	0x00F00000
#define INTERNAL_CONFIG_TRANSCEIVER_SHIFT	20

//
// Connector types
//

#define CONNECTOR_10BASET         0
#define CONNECTOR_10AUI           1
#define CONNECTOR_10BASE2         3
#define CONNECTOR_100BASETX       4
#define CONNECTOR_100BASEFX       5
#define CONNECTOR_MII             6
#define CONNECTOR_AUTONEGOTIATION 8
#define CONNECTOR_EXTERNAL_MII    9
#define CONNECTOR_UNKNOWN         0xFF

//
// Physical Management
//

#define PHY_WRITE			        0x0004  // Write to PHY (drive MDIO)
#define PHY_DATA1			        0x0002  // MDIO data bit
#define PHY_CLOCK			        0x0001  // MII clock signal

#define MII_PHY_ADDRESS				0x0C00
#define MII_PHY_ADDRESS_READ  		        (MII_PHY_ADDRESS | 0x6000)
#define MII_PHY_ADDRESS_WRITE  		        (MII_PHY_ADDRESS | 0x5002)

//
// Media Options
//

#define MEDIA_OPTIONS_100BASET4_AVAILABLE	(1 << 0)
#define MEDIA_OPTIONS_100BASETX_AVAILABLE	(1 << 1)
#define MEDIA_OPTIONS_100BASEFX_AVAILABLE	(1 << 2)
#define MEDIA_OPTIONS_10BASET_AVAILABLE		(1 << 3)
#define MEDIA_OPTIONS_10BASE2_AVAILABLE		(1 << 4)
#define MEDIA_OPTIONS_10AUI_AVAILABLE		(1 << 5)
#define MEDIA_OPTIONS_MII_AVAILABLE		(1 << 6)
#define MEDIA_OPTIONS_10BASEFL_AVAILABLE	(1 << 8)

//
// MAC Control
//

#define MAC_CONTROL_FULL_DUPLEX_ENABLE		(1 << 5)
#define MAC_CONTROL_ALLOW_LARGE_PACKETS		(1 << 6)
#define MAC_CONTROL_FLOW_CONTROL_ENABLE 	(1 << 8)

//
// MII Registers
//

#define MII_PHY_CONTROL			0   // Control reg address
#define MII_PHY_STATUS              	1   // Status reg address
#define MII_PHY_OUI                 	2   // Most of the OUI bits
#define MII_PHY_MODEL               	3   // Model/rev bits, and rest of OUI
#define MII_PHY_ANAR                	4   // Auto negotiate advertisement reg
#define MII_PHY_ANLPAR              	5   // Auto negotiate link partner reg
#define MII_PHY_ANER                	6   // Auto negotiate expansion reg

//
// MII control register
//

#define MII_CONTROL_RESET		0x8000  // Reset bit in control reg
#define MII_CONTROL_100MB		0x2000  // 100Mbit or 10 Mbit flag
#define MII_CONTROL_ENABLE_AUTO		0x1000  // Autonegotiate enable
#define MII_CONTROL_ISOLATE		0x0400  // Islolate bit
#define MII_CONTROL_START_AUTO		0x0200  // Restart autonegotiate
#define MII_CONTROL_FULL_DUPLEX		0x0100  // Full duplex

//
// MII status register
//

#define MII_STATUS_100MB_MASK	0xE000  // Any of these indicate 100 Mbit
#define MII_STATUS_10MB_MASK	0x1800  // Either of these indicate 10 Mbit
#define MII_STATUS_AUTO_DONE	0x0020  // Auto negotiation complete
#define MII_STATUS_AUTO		0x0008  // Auto negotiation is available
#define MII_STATUS_LINK_UP	0x0004  // Link status bit
#define MII_STATUS_EXTENDED	0x0001  // Extended regs exist
#define MII_STATUS_100T4	0x8000  // Capable of 100BT4
#define MII_STATUS_100TXFD	0x4000  // Capable of 100BTX full duplex
#define MII_STATUS_100TX	0x2000  // Capable of 100BTX
#define MII_STATUS_10TFD	0x1000  // Capable of 10BT full duplex
#define MII_STATUS_10T		0x0800  // Capable of 10BT

//
// MII Auto-Negotiation Link Partner Ability
//

#define MII_ANLPAR_100T4	0x0200  // Support 100BT4
#define MII_ANLPAR_100TXFD	0x0100  // Support 100BTX full duplex
#define MII_ANLPAR_100TX	0x0080  // Support 100BTX half duplex
#define MII_ANLPAR_10TFD	0x0040  // Support 10BT full duplex
#define MII_ANLPAR_10T		0x0020  // Support 10BT half duplex

//
// MII Auto-Negotiation Advertisement
//

#define MII_ANAR_100T4		0x0200  // Support 100BT4
#define MII_ANAR_100TXFD	0x0100  // Support 100BTX full duplex
#define MII_ANAR_100TX		0x0080  // Support 100BTX half duplex
#define MII_ANAR_10TFD		0x0040  // Support 10BT full duplex
#define MII_ANAR_10T		0x0020  // Support 10BT half duplex
#define MII_ANAR_FLOWCONTROL	0x0400  // Support Flow Control

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

#define LAST_FRAG 	          0x80000000  // Last entry in descriptor
#define DN_COMPLETE	          0x00010000  // This packet has been downloaded
#define UP_COMPLETE               0x00008000  // This packet has been uploaded

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

  unsigned short iobase;		// Configured I/O base
  unsigned short irq;		        // Configured IRQ

  int connector;                        // Active connector
  int linkspeed;                        // Link speed in mbits/s
  int fullduplex;                       // Full duplex link

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

void execute_command_wait(struct nic *nic, int cmd, int param)
{
  int i;

  execute_command(nic, cmd, param);
  for (i = 0; i < 2000; i++)
  {
    if (!(_inpw(nic->iobase + STATUS) & INTSTATUS_CMD_IN_PROGRESS)) return;
  }

  for (i = 0; i < 200; i++)
  {
    if (!(_inpw(nic->iobase + STATUS) & INTSTATUS_CMD_IN_PROGRESS)) return;
    sleep(10);
  }

  kprintf("nic: command did not complete\n");
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

  pprintf(pf, "Frames transmitted... : %d\n", nic->stat.tx_frames_ok);
  pprintf(pf, "Bytes transmitted.... : %d\n", nic->stat.tx_bytes_ok);
  pprintf(pf, "Frames deferred...... : %d\n", nic->stat.tx_frames_deferred);
  pprintf(pf, "Single collisions.... : %d\n", nic->stat.tx_single_collisions);
  pprintf(pf, "Multiple collisions.. : %d\n", nic->stat.tx_multiple_collisions);
  pprintf(pf, "Late collisions...... : %d\n", nic->stat.tx_late_collisions);
  pprintf(pf, "Carrier lost......... : %d\n", nic->stat.tx_carrier_lost);
  pprintf(pf, "Maximum collisions .. : %d\n", nic->stat.tx_maximum_collisions);
  pprintf(pf, "SQE errors........... : %d\n", nic->stat.tx_sqe_errors);
  pprintf(pf, "HW errors............ : %d\n", nic->stat.tx_hw_errors);
  pprintf(pf, "Jabber errors........ : %d\n", nic->stat.tx_jabber_error);
  pprintf(pf, "Unknown errors....... : %d\n", nic->stat.tx_unknown_error);
  pprintf(pf, "Frames received...... : %d\n", nic->stat.rx_frames_ok);
  pprintf(pf, "Bytes received....... : %d\n", nic->stat.rx_bytes_ok);
  pprintf(pf, "Overruns............. : %d\n", nic->stat.rx_overruns);
  pprintf(pf, "Bad SSD.............. : %d\n", nic->stat.rx_bad_ssd);
  pprintf(pf, "Allignment errors.... : %d\n", nic->stat.rx_alignment_error);
  pprintf(pf, "CRC errors........... : %d\n", nic->stat.rx_bad_crc_error);
  pprintf(pf, "Oversized frames..... : %d\n", nic->stat.rx_oversize_error);

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

int nic_negotiate_link(struct nic *nic)
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
    kprintf("nic: not configured for auto-negotiate\n");
    return -EINVAL;
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

int __declspec(dllexport) install(struct unit *unit)
{
  struct nic *nic;
  unsigned short eeprom_checksum = 0;
  int eeprom_busy = 0;
  int i, j;
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

  // Enable bus mastering
  pci_enable_busmastering(unit);

  // Install interrupt handler
  set_interrupt_handler(IRQ2INTR(nic->irq), nic_handler, nic);
  enable_irq(nic->irq);

  // Reset NIC
  // Don't reset the PHY - that upsets autonegotiation during DHCP operations
  execute_command_wait(nic, CMD_RX_RESET, 0x04);
  execute_command_wait(nic, CMD_TX_RESET, 0);

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
    if (j == 10) eeprom_busy = 1;
    nic->eeprom[i] = _inpw(nic->iobase + EEPROM_DATA);
  }
  if (eeprom_busy) kprintf("warning: nic eeprom busy while reading\n");

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
  rc = nic_negotiate_link(nic);
  if (rc < 0) return rc;

  // Set receive filter
  execute_command(nic, CMD_SET_RX_FILTER, RECEIVE_INDIVIDUAL | RECEIVE_MULTICAST | RECEIVE_BROADCAST);

  // Acknowledge any pending interrupts.
  execute_command(nic, CMD_ACKNOWLEDGE_INTERRUPT, ALL_ACK);

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
