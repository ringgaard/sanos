//
// 3c905c.h
//
// 3Com 3C905C NIC network driver
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

#ifndef _3C905C_H
#define _3C905C_H

#define ETHER_FRAME_LEN         1544
#define EEPROM_SIZE             0x21
#define RX_COPYBREAK            128
#define TX_TIMEOUT              5000

#define TX_RING_SIZE              16
#define RX_RING_SIZE              32
#define TX_MAX_FRAGS              16

#define LAST_FRAG                 0x80000000  // Last entry in descriptor
#define DN_COMPLETE               0x00010000  // This packet has been downloaded
#define UP_COMPLETE               0x00008000  // This packet has been uploaded

//
// PCI IDs
//

#define UNITCODE_3C905B1           PCI_UNITCODE(0x10B7, 0x9055)
#define UNITCODE_3C905C            PCI_UNITCODE(0x10B7, 0x9200)
#define UNITCODE_3C9051            PCI_UNITCODE(0x10B7, 0x9050)

//
// Commands
//

#define CMD_RESET                  (0x0 << 0xB)
#define CMD_SELECT_WINDOW          (0x1 << 0xB)
#define CMD_ENABLE_DC_CONVERTER    (0x2 << 0xB)
#define CMD_RX_DISABLE             (0x3 << 0xB)
#define CMD_RX_ENABLE              (0x4 << 0xB)
#define CMD_RX_RESET               (0x5 << 0xB)
#define CMD_UP_STALL               ((0x6 << 0xB) | 0x0)
#define CMD_UP_UNSTALL             ((0x6 << 0xB) | 0x1)
#define CMD_DOWN_STALL             ((0x6 << 0xB) | 0x2)
#define CMD_DOWN_UNSTALL           ((0x6 << 0xB) | 0x3)
#define CMD_TX_DONE                (0x7 << 0xB)
#define CMD_RX_DISCARD             (0x8 << 0xB)
#define CMD_TX_ENABLE              (0x9 << 0xB)
#define CMD_TX_DISABLE             (0xA << 0xB)
#define CMD_TX_RESET               (0xB << 0xB)
#define CMD_REQUEST_INTERRUPT      (0xC << 0xB)
#define CMD_ACKNOWLEDGE_INTERRUPT  (0xD << 0xB)
#define CMD_SET_INTERRUPT_ENABLE   (0xE << 0xB)
#define CMD_SET_INDICATION_ENABLE  (0xF << 0xB)
#define CMD_SET_RX_FILTER          (0x10 << 0xB)
#define CMD_TX_AGAIN               (0x13 << 0xB)
#define CMD_STATISTICS_ENABLE      (0x15 << 0xB)
#define CMD_STATISTICS_DISABLE     (0x16 << 0xB)
#define CMD_DISABLE_DC_CONVERTER   (0x17 << 0xB)
#define CMD_SET_HASH_FILTER_BIT    (0x19 << 0xB)
#define CMD_TX_FIFO_BISECT         (0x1B << 0xB)

//
// Non-windowed registers
//

#define CMD                   0x0E 
#define STATUS                0x0E

#define TIMER                 0x1A
#define TX_STATUS             0x1B
#define INT_STATUS_AUTO       0x1E
#define DMA_CONTROL           0x20
#define DOWN_LIST_POINTER     0x24
#define DOWN_POLL             0x2D
#define UP_PACKET_STATUS      0x30
#define FREE_TIMER            0x34
#define COUNTDOWN             0x36
#define UP_LIST_POINTER       0x38
#define UP_POLL               0x3D
#define REAL_TIME_COUNTER     0x40
#define CONFIG_ADDRESS        0x44
#define CONFIG_DATA           0x48
#define DEBUG_DATA            0x70
#define DEBUG_CONTROL         0x74

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
#define MAC_CONTROL           0x06
#define MEDIA_OPTIONS         0x08
#define RX_FREE               0x0A
#define TX_FREE               0x0C

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

#define RX_FILTER             0x08
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

#define TX_STATUS_MAXIMUM_COLLISION     (1 << 3)
#define TX_STATUS_HWERROR               (1 << 4)
#define TX_STATUS_JABBER                (1 << 5)
#define TX_STATUS_INTERRUPT_REQUESTED   (1 << 6)
#define TX_STATUS_COMPLETE              (1 << 7)

//
// Global reset flags
//

#define GLOBAL_RESET_MASK_TP_AUI_RESET  (1 << 0)
#define GLOBAL_RESET_MASK_ENDEC_RESET   (1 << 1)
#define GLOBAL_RESET_MASK_NETWORK_RESET (1 << 2)
#define GLOBAL_RESET_MASK_FIFO_RESET    (1 << 3)
#define GLOBAL_RESET_MASK_AISM_RESET    (1 << 4)
#define GLOBAL_RESET_MASK_HOST_RESET    (1 << 5)
#define GLOBAL_RESET_MASK_SMB_RESET     (1 << 6)
#define GLOBAL_RESET_MASK_VCO_RESET     (1 << 7)
#define GLOBAL_RESET_MASK_UP_DOWN_RESET (1 << 8)

//
// TX reset flags
//

#define TX_RESET_MASK_TP_AUI_RESET      (1 << 0)
#define TX_RESET_MASK_ENDEC_RESET       (1 << 1)
#define TX_RESET_MASK_NETWORK_RESET     (1 << 2)
#define TX_RESET_MASK_FIFO_RESET        (1 << 3)
#define TX_RESET_MASK_DOWN_RESET        (1 << 8)

//
// RX reset flags
//

#define RX_RESET_MASK_TP_AUI_RESET      (1 << 0)
#define RX_RESET_MASK_ENDEC_RESET       (1 << 1)
#define RX_RESET_MASK_NETWORK_RESET     (1 << 2)
#define RX_RESET_MASK_FIFO_RESET        (1 << 3)
#define RX_RESET_MASK_UP_RESET          (1 << 8)

//
// IntStatus flags
//

#define INTSTATUS_INT_LATCH             (1 << 0)
#define INTSTATUS_HOST_ERROR            (1 << 1)
#define INTSTATUS_TX_COMPLETE           (1 << 2)
#define INTSTATUS_RX_COMPLETE           (1 << 4)
#define INTSTATUS_RX_EARLY              (1 << 5)
#define INTSTATUS_INT_REQUESTED         (1 << 6)
#define INTSTATUS_UPDATE_STATS          (1 << 7)
#define INTSTATUS_LINK_EVENT            (1 << 8)
#define INTSTATUS_DN_COMPLETE           (1 << 9)
#define INTSTATUS_UP_COMPLETE           (1 << 10)
#define INTSTATUS_CMD_IN_PROGRESS       (1 << 12)

#define ALL_INTERRUPTS                  0x06EE

//
// AcknowledgeInterrupt flags
//

#define INTERRUPT_LATCH_ACK        0x0001
#define LINK_EVENT_ACK             0x0002
#define RX_EARLY_ACK               0x0020
#define INT_REQUESTED_ACK          0x0040
#define DN_COMPLETE_ACK            0x0200
#define UP_COMPLETE_ACK            0x0400

#define ALL_ACK                    0x07FF   

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

#define UP_PACKET_STATUS_ERROR                  (1 << 14)
#define UP_PACKET_STATUS_COMPLETE               (1 << 15)
#define UP_PACKET_STATUS_OVERRUN                (1 << 16)
#define UP_PACKET_STATUS_RUNT_FRAME             (1 << 17)
#define UP_PACKET_STATUS_ALIGNMENT_ERROR        (1 << 18)
#define UP_PACKET_STATUS_CRC_ERROR              (1 << 19)
#define UP_PACKET_STATUS_OVERSIZE_FRAME         (1 << 20)
#define UP_PACKET_STATUS_DRIBBLE_BITS           (1 << 23)
#define UP_PACKET_STATUS_OVERFLOW               (1 << 24)
#define UP_PACKET_STATUS_IP_CHECKSUM_ERROR      (1 << 25)
#define UP_PACKET_STATUS_TCP_CHECKSUM_ERROR     (1 << 26)
#define UP_PACKET_STATUS_UDP_CHECKSUM_ERROR     (1 << 27)
#define UP_PACKET_STATUS_IMPLIED_BUFFER_ENABLE  (1 << 28)
#define UP_PACKET_STATUS_IP_CHECKSUM_CHECKED    (1 << 29)
#define UP_PACKET_STATUS_TCP_CHECKSUM_CHECKED   (1 << 30)
#define UP_PACKET_STATUS_UDP_CHECKSUM_CHECKED   (1 << 31)
#define UP_PACKET_STATUS_ERROR_MASK             0x1F0000

//
// Frame Start Header
//
#define FSH_CRC_APPEND_DISABLE                  (1 << 13)
#define FSH_TX_INDICATE                         (1 << 15)
#define FSH_DOWN_COMPLETE                       (1 << 16)
#define FSH_LAST_KEEP_ALIVE_PACKET              (1 << 24)
#define FSH_ADD_IP_CHECKSUM                     (1 << 25)
#define FSH_ADD_TCP_CHECKSUM                    (1 << 26)
#define FSH_ADD_UDP_CHECKSUM                    (1 << 27)
#define FSH_ROUND_UP_DEFEAT                     (1 << 28)
#define FSH_DPD_EMPTY                           (1 << 29)
#define FSH_DOWN_INDICATE                       (1 << 31)

//
// Internal Config
//

#define INTERNAL_CONFIG_DISABLE_BAD_SSD         (1 << 8)
#define INTERNAL_CONFIG_ENABLE_TX_LARGE         (1 << 14)
#define INTERNAL_CONFIG_ENABLE_RX_LARGE         (1 << 15)
#define INTERNAL_CONFIG_AUTO_SELECT             (1 << 24)
#define INTERNAL_CONFIG_DISABLE_ROM             (1 << 25)

#define INTERNAL_CONFIG_TRANSCEIVER_MASK        0x00F00000
#define INTERNAL_CONFIG_TRANSCEIVER_SHIFT       20

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

#define PHY_WRITE                               0x0004  // Write to PHY (drive MDIO)
#define PHY_DATA1                               0x0002  // MDIO data bit
#define PHY_CLOCK                               0x0001  // MII clock signal

#define MII_PHY_ADDRESS                         0x0C00
#define MII_PHY_ADDRESS_READ                    (MII_PHY_ADDRESS | 0x6000)
#define MII_PHY_ADDRESS_WRITE                   (MII_PHY_ADDRESS | 0x5002)

//
// DMA control
//

#define DMA_CONTROL_DOWN_STALLED        (1 << 2)
#define DMA_CONTROL_UP_COMPLETE         (1 << 3)
#define DMA_CONTROL_DOWN_COMPLETE       (1 << 4)
#define DMA_CONTROL_ARM_COUNTDOWN       (1 << 6)
#define DMA_CONTROL_DOWN_IN_PROGRESS    (1 << 7)
#define DMA_CONTROL_COUNTER_SPEED       (1 << 8)
#define DMA_CONTROL_COUNTDOWN_MODE      (1 << 9)
#define DMA_CONTROL_DOWN_SEQ_DISABLE    (1 << 17)
#define DMA_CONTROL_DEFEAT_MWI          (1 << 20)
#define DMA_CONTROL_DEFEAT_MRL          (1 << 21)
#define DMA_CONTROL_UPOVERDISC_DISABLE  (1 << 22)
#define DMA_CONTROL_TARGET_ABORT        (1 << 30)
#define DMA_CONTROL_MASTER_ABORT        (1 << 31)

//
// Media status
//

#define MEDIA_STATUS_SQE_STATISTICS_ENABLE      (1 << 3)
#define MEDIA_STATUS_CARRIER_SENSE              (1 << 5)
#define MEDIA_STATUS_JABBER_GUARD_ENABLE        (1 << 6)
#define MEDIA_STATUS_LINK_BEAT_ENABLE           (1 << 7)
#define MEDIA_STATUS_LINK_DETECT                (1 << 11)
#define MEDIA_STATUS_TX_IN_PROGRESS             (1 << 12)
#define MEDIA_STATUS_DC_CONVERTER_ENABLED       (1 << 14)

//
// Media Options
//

#define MEDIA_OPTIONS_100BASET4_AVAILABLE       (1 << 0)
#define MEDIA_OPTIONS_100BASETX_AVAILABLE       (1 << 1)
#define MEDIA_OPTIONS_100BASEFX_AVAILABLE       (1 << 2)
#define MEDIA_OPTIONS_10BASET_AVAILABLE         (1 << 3)
#define MEDIA_OPTIONS_10BASE2_AVAILABLE         (1 << 4)
#define MEDIA_OPTIONS_10AUI_AVAILABLE           (1 << 5)
#define MEDIA_OPTIONS_MII_AVAILABLE             (1 << 6)
#define MEDIA_OPTIONS_10BASEFL_AVAILABLE        (1 << 8)

//
// MAC Control
//

#define MAC_CONTROL_FULL_DUPLEX_ENABLE          (1 << 5)
#define MAC_CONTROL_ALLOW_LARGE_PACKETS         (1 << 6)
#define MAC_CONTROL_FLOW_CONTROL_ENABLE         (1 << 8)

//
// Network diagnostics
//

#define NETWORK_DIAGNOSTICS_ASIC_REVISION       0x003E
#define NETWORK_DIAGNOSTICS_ASIC_REVISION_LOW   0x000E 
#define NETWORK_DIAGNOSTICS_UPPER_BYTES_ENABLE  (1 << 6)

//
// MII Registers
//

#define MII_PHY_CONTROL                 0   // Control reg address
#define MII_PHY_STATUS                  1   // Status reg address
#define MII_PHY_OUI                     2   // Most of the OUI bits
#define MII_PHY_MODEL                   3   // Model/rev bits, and rest of OUI
#define MII_PHY_ANAR                    4   // Auto negotiate advertisement reg
#define MII_PHY_ANLPAR                  5   // Auto negotiate link partner reg
#define MII_PHY_ANER                    6   // Auto negotiate expansion reg

//
// MII control register
//

#define MII_CONTROL_RESET               0x8000  // Reset bit in control reg
#define MII_CONTROL_100MB               0x2000  // 100Mbit or 10 Mbit flag
#define MII_CONTROL_ENABLE_AUTO         0x1000  // Autonegotiate enable
#define MII_CONTROL_ISOLATE             0x0400  // Islolate bit
#define MII_CONTROL_START_AUTO          0x0200  // Restart autonegotiate
#define MII_CONTROL_FULL_DUPLEX         0x0100  // Full duplex

//
// MII status register
//

#define MII_STATUS_100MB_MASK   0xE000  // Any of these indicate 100 Mbit
#define MII_STATUS_10MB_MASK    0x1800  // Either of these indicate 10 Mbit
#define MII_STATUS_AUTO_DONE    0x0020  // Auto negotiation complete
#define MII_STATUS_AUTO         0x0008  // Auto negotiation is available
#define MII_STATUS_LINK_UP      0x0004  // Link status bit
#define MII_STATUS_EXTENDED     0x0001  // Extended regs exist
#define MII_STATUS_100T4        0x8000  // Capable of 100BT4
#define MII_STATUS_100TXFD      0x4000  // Capable of 100BTX full duplex
#define MII_STATUS_100TX        0x2000  // Capable of 100BTX
#define MII_STATUS_10TFD        0x1000  // Capable of 10BT full duplex
#define MII_STATUS_10T          0x0800  // Capable of 10BT

//
// MII Auto-Negotiation Link Partner Ability
//

#define MII_ANLPAR_100T4        0x0200  // Support 100BT4
#define MII_ANLPAR_100TXFD      0x0100  // Support 100BTX full duplex
#define MII_ANLPAR_100TX        0x0080  // Support 100BTX half duplex
#define MII_ANLPAR_10TFD        0x0040  // Support 10BT full duplex
#define MII_ANLPAR_10T          0x0020  // Support 10BT half duplex

//
// MII Auto-Negotiation Advertisement
//

#define MII_ANER_LPANABLE       0x0001  // Link partner autonegotiatable ?
#define MII_ANAR_100T4          0x0200  // Support 100BT4
#define MII_ANAR_100TXFD        0x0100  // Support 100BTX full duplex
#define MII_ANAR_100TX          0x0080  // Support 100BTX half duplex
#define MII_ANAR_10TFD          0x0040  // Support 10BT full duplex
#define MII_ANAR_10T            0x0020  // Support 10BT half duplex
#define MII_ANAR_FLOWCONTROL    0x0400  // Support Flow Control

#define MII_ANAR_MEDIA_MASK     0x07E0  // Mask the media selection bits
#define MII_ANAR_MEDIA_100_MASK (MII_ANAR_100TXFD | MII_ANAR_100TX)
#define MII_ANAR_MEDIA_10_MASK  (MII_ANAR_10TFD | MII_ANAR_10T)

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
#define EEPROM_SOFTWARE_INFO1     0x0D
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

//
// EEPROM software information 1
//

#define LINK_BEAT_DISABLE         (1 << 14)

//
// EEPROM software information 2
//

#define ENABLE_MWI_WORK            0x0020

//
// MII Transceiver Type
//
#define MIISELECT_GENERIC       0x0000
#define MIISELECT_100BT4        0x0001
#define MIISELECT_10BT          0x0002
#define MIISELECT_100BTX        0x0003
#define MIISELECT_10BT_ANE      0x0004
#define MIISELECT_100BTX_ANE    0x0005
#define MIITXTYPE_MASK          0x000F

#endif
