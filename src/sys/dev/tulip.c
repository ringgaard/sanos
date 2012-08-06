//
// tulip.c
//
// Digital "Tulip" Ethernet network driver for DEC 21*4*-based chips/ethercards
//
// Written/copyright 1994-2002 by Donald Becker
// Ported to sanos 2002 by Michael Ringgaard.
//
// This software may be used and distributed according to the terms of
// the GNU General Public License (GPL), incorporated herein by reference.
// Drivers based on or derived from this code fall under the GPL and must
// retain the authorship, copyright and license notice.  This file is not
// a complete program and may only be used when the entire operating
// system is licensed under the GPL.
//
// This driver is for the Digital "Tulip" Ethernet adapter interface.
// It should work with most DEC 21*4*-based chips/ethercards, as well as
// with work-alike chips from Lite-On (PNIC) and Macronix (MXIC) and ASIX.
//
// The author may be reached as becker@scyld.com, or C/O
// Scyld Computing Corporation
// 410 Severn Ave., Suite 210
// Annapolis MD 21403
//

#include <os/krnl.h>

// Maximum events (Rx packets, etc.) to handle at each interrupt

static int max_interrupt_work = 25;

// The possible media types that can be set in options[] are:

#define MEDIA_MASK 31

static char *medianame[32] = {
  "10baseT", "10base2", "AUI", "100baseTx",
  "10baseT-FDX", "100baseTx-FDX", "100baseT4", "100baseFx",
  "100baseFx-FDX", "MII 10baseT", "MII 10baseT-FDX", "MII",
  "10baseT(forced)", "MII 100baseTx", "MII 100baseTx-FDX", "MII 100baseT4",
  "MII 100baseFx-HDX", "MII 100baseFx-FDX", "Home-PNA 1Mbps", "Invalid-19",
  "","","","", "","","","",  "","","","Transceiver reset",
};

// Set if the PCI BIOS detects the chips on a multiport board backwards.

#ifdef REVERSE_PROBE_ORDER
static int reverse_probe = 1;
#else
static int reverse_probe = 0;
#endif

// Set the copy breakpoint for the copy-only-tiny-buffer Rx structure.
static int rx_copybreak = 100;

//
// Set the bus performance register.
//   Typical: Set 16 longword cache alignment, no burst limit.
//   Cache alignment bits 15:14       Burst length 13:8
//   0000  No alignment  0x00000000 unlimited    0800 8 longwords
//   4000  8  longwords    0100 1 longword   1000 16 longwords
//   8000  16 longwords    0200 2 longwords  2000 32 longwords
//   C000  32  longwords   0400 4 longwords
//   Warning: many older 486 systems are broken and require setting 0x00A04800
//   8 longword cache alignment, 8 longword burst.
//

static int csr0 = 0x01A00000 | 0x8000;

//
// Maximum number of multicast addresses to filter (vs. rx-all-multicast).
// Typical is a 64 element hash table based on the Ethernet CRC.
// This value does not apply to the 512 bit table chips.
//

static int multicast_filter_limit = 32;

//
// Keep the descriptor ring sizes a power of two for efficiency.
// The Tx queue length limits transmit packets to a portion of the available
// ring entries.  It should be at least one element less to allow multicast
// filter setup frames to be queued.  It must be at least four for hysteresis.
// Making the Tx queue too long decreases the effectiveness of channel
// bonding and packet priority.
// Large receive rings merely consume memory.
//

#define TX_RING_SIZE  16
#define TX_QUEUE_LEN  10
#define RX_RING_SIZE  32

#define TX_TIMEOUT  (4*HZ)      // Time in ticks before concluding the transmitter is hung
#define PKT_BUF_SZ    1536      // Size of each temporary Rx buffer

//
// This is a mysterious value that can be written to CSR11 in the 21040 (only)
// to support a pre-NWay full-duplex signaling mechanism using short frames.
// No one knows what it should be, but if left at its default value some
// 10base2(!) packets trigger a full-duplex-request interrupt.
//

#define FULL_DUPLEX_MAGIC 0x6969

//
// This driver was originally written to use I/O space access, but now
// uses memory space by default. Override this this with -DUSE_IO_OPS.
//

#ifdef USE_IO_OPS

#define readb(port) inp(port)
#define readw(port) inpw(port)
#define readl(port) inpd(port)

#define writeb(data, port) outp((port), (data))
#define writew(data, port) outpw((port), (data))
#define writel(data, port) outpd((port), (data))

#else

#define readb(addr) (*(volatile unsigned char *) (addr))
#define readw(addr) (*(volatile unsigned short *) (addr))
#define readl(addr) (*(volatile unsigned int *) (addr))

#define writeb(data ,addr) (*(volatile unsigned char *) (addr) = (data))
#define writew(data ,addr) (*(volatile unsigned short *) (addr) = (data))
#define writel(data ,addr) (*(volatile unsigned int *) (addr) = (data))

#endif

//
//         Theory of Operation
// 
// I. Board Compatibility
// 
// This device driver is designed for the DECchip "Tulip", Digital's
// single-chip ethernet controllers for PCI.  Supported members of the family
// are the 21040, 21041, 21140, 21140A, 21142, and 21143.  Similar work-alike
// chips from Lite-On, Macronics, ASIX, Compex and other listed below are also
// supported.
// 
// These chips are used on at least 140 unique PCI board designs.  The great
// number of chips and board designs supported is the reason for the
// driver size and complexity.  Almost of the increasing complexity is in the
// board configuration and media selection code.  There is very little
// increasing in the operational critical path length.
// 
// II. Board-specific settings
// 
// PCI bus devices are configured by the system at boot time, so no jumpers
// need to be set on the board.  The system BIOS preferably should assign the
// PCI INTA signal to an otherwise unused system IRQ line.
// 
// Some boards have EEPROMs tables with default media entry.  The factory default
// is usually "autoselect".  This should only be overridden when using
// transceiver connections without link beat e.g. 10base2 or AUI, or (rarely!)
// for forcing full-duplex when used with old link partners that do not do
// autonegotiation.
// 
// III. Driver operation
// 
// IIIa. Ring buffers
// 
// The Tulip can use either ring buffers or lists of Tx and Rx descriptors.
// This driver uses statically allocated rings of Rx and Tx descriptors, set at
// compile time by RX/TX_RING_SIZE.  This version of the driver allocates pbufs
// for the Rx ring buffers at open() time and passes the p->payload field to the
// Tulip as receive data buffers.  When an incoming frame is less than
// RX_COPYBREAK bytes long, a fresh pbuf is allocated and the frame is
// copied to the new pbuf.  When the incoming frame is larger, the pbuf is
// passed directly up the protocol stack and replaced by a newly allocated
// pbuf.
// 
// The RX_COPYBREAK value is chosen to trade-off the memory wasted by
// using a full-sized pbuf for small frames vs. the copying costs of larger
// frames.  For small frames the copying cost is negligible (esp. considering
// that we are pre-loading the cache with immediately useful header
// information).  For large frames the copying cost is non-trivial, and the
// larger copy might flush the cache of useful data.  A subtle aspect of this
// choice is that the Tulip only receives into longword aligned buffers, thus
// the IP header at offset 14 is not longword aligned for further processing.
// 
// IIIC. Synchronization
// The driver runs as two independent, single-threaded flows of control.  One
// is the send-packet routine, which enforces single-threaded use by the
// semaphore.  The other thread is the interrupt handler, which is single
// threaded by the hardware and other software.
// 
// The interrupt handler has exclusive control over the Rx ring and records stats
// from the Tx ring.  (The Tx-done interrupt can not be selectively turned off, so
// we cannot avoid the interrupt overhead by having the Tx routine reap the Tx
// stats.)  After reaping the stats, it marks the queue entry as empty by setting
// the 'base' to zero.  
// 
// IV. Notes
// 
// Thanks to Duke Kamstra of SMC for long ago providing an EtherPower board.
// Greg LaPolla at Linksys provided PNIC and other Linksys boards.
// Znyx provided a four-port card for testing.
// 
// IVb. References
// 
// http://scyld.com/expert/NWay.html
// http://www.digital.com  (search for current 21*4* datasheets and "21X4 SROM")
// http://www.national.com/pf/DP/DP83840A.html
// http://www.asix.com.tw/pmac.htm
// http://www.admtek.com.tw/
// 
// IVc. Errata
// 
// The old DEC databooks were light on details.
// The 21040 databook claims that CSR13, CSR14, and CSR15 should each be the last
// register of the set CSR12-15 written.  Hmmm, now how is that possible?
// 
// The DEC SROM format is very badly designed not precisely defined, leading to
// part of the media selection junkheap below.  Some boards do not have EEPROM
// media tables and need to be patched up.  Worse, other boards use the DEC
// design kit media table when it is not correct for their design.
// 
// We cannot use MII interrupts because there is no defined GPIO pin to attach
// them.  The MII transceiver status is polled using an kernel timer.
// 

static int tulip_pwr_event(void *dev_instance, int event);

#ifdef USE_IO_OPS
#define TULIP_IOTYPE  PCI_USES_MASTER | PCI_USES_IO | PCI_ADDR0
#define TULIP_SIZE  0x80
#define TULIP_SIZE1 0x100
#else
#define TULIP_IOTYPE  PCI_USES_MASTER | PCI_USES_MEM | PCI_ADDR1
#define TULIP_SIZE   0x400     // New PCI v2.1 recommends 4K min mem size
#define TULIP_SIZE1  0x400     // New PCI v2.1 recommends 4K min mem size
#endif

// This much match tulip_tbl[]!  Note 21142 == 21143.

enum tulip_chips {
  DC21040=0, 
  DC21041=1, 
  DC21140=2, 
  DC21142=3, 
  DC21143=3,
  LC82C168, 
  MX98713, 
  MX98715, 
  MX98725, 
  AX88141, 
  AX88140, 
  PNIC2, 
  COMET,
  COMPEX9881, 
  I21145, 
  XIRCOM, 
  CONEXANT,
  // These flags may be added to the chip type
  HAS_VLAN = 0x100,
};

static struct board board_tbl[] = {
  {"Digital", "Digital DC21040 Tulip",                     BUSTYPE_PCI, PCI_UNITCODE(0x1011, 0x0002), 0xffffffff, 0,                            0,          0,    0,    DC21040},
  {"Digital", "Digital DC21041 Tulip",                     BUSTYPE_PCI, PCI_UNITCODE(0x1011, 0x0014), 0xffffffff, 0,                            0,          0,    0,    DC21041},
  {"Digital", "Digital DS21140A Tulip",                    BUSTYPE_PCI, PCI_UNITCODE(0x1011, 0x0009), 0xffffffff, 0,                            0,          0x20, 0xf0, DC21140},
  {"Digital", "Digital DS21140 Tulip",                     BUSTYPE_PCI, PCI_UNITCODE(0x1011, 0x0009), 0xffffffff, 0,                            0,          0,    0,    DC21140},
  {"Digital", "Digital DS21143-xD Tulip",                  BUSTYPE_PCI, PCI_UNITCODE(0x1011, 0x0019), 0xffffffff, 0,                            0,          0x40, 0xf0, DC21142 | HAS_VLAN},
  {"Digital", "Digital DS21143-xC Tulip",                  BUSTYPE_PCI, PCI_UNITCODE(0x1011, 0x0019), 0xffffffff, 0,                            0,          0x30, 0xf0, DC21142},
  {"Digital", "Digital DS21142 Tulip",                     BUSTYPE_PCI, PCI_UNITCODE(0x1011, 0x0019), 0xffffffff, 0,                            0,          0,    0,    DC21142},
  {"Kingston", "Kingston KNE110tx (PNIC)",                 BUSTYPE_PCI, PCI_UNITCODE(0x11AD, 0x0002), 0xffffffff, PCI_UNITCODE(0x2646, 0xf002), 0xffffffff, 0,    0,    LC82C168}, 
  {"Linksys", "Linksys LNE100TX (82c168 PNIC)",            BUSTYPE_PCI, PCI_UNITCODE(0x11AD, 0x0002), 0xffffffff, PCI_UNITCODE(0x11ad, 0xffff), 0xffffffff, 17,   0xff, LC82C168},
  {"Linksys", "Linksys LNE100TX (82c169 PNIC)",            BUSTYPE_PCI, PCI_UNITCODE(0x11AD, 0x0002), 0xffffffff, PCI_UNITCODE(0x11ad, 0xf003), 0xffffffff, 32,   0xff, LC82C168},
  {"Lite-On", "Lite-On 82c168 PNIC",                       BUSTYPE_PCI, PCI_UNITCODE(0x11AD, 0x0002), 0xffffffff, 0,                            0,          0,    0,    LC82C168},
  {"Macronix", "Macronix 98713 PMAC",                      BUSTYPE_PCI, PCI_UNITCODE(0x10d9, 0x0512), 0xffffffff, 0,                            0,          0,    0,    MX98713},
  {"Macronix", "Macronix 98715 PMAC",                      BUSTYPE_PCI, PCI_UNITCODE(0x10d9, 0x0531), 0xffffffff, 0,                            0,          0,    0,    MX98715},
  {"Macronix", "Macronix 98725 PMAC",                      BUSTYPE_PCI, PCI_UNITCODE(0x10d9, 0x0531), 0xffffffff, 0,                            0,          0,    0,    MX98725},
  {"ASIX", "ASIX AX88141",                                 BUSTYPE_PCI, PCI_UNITCODE(0x125B, 0x1400), 0xffffffff, 0,                            0,          0x10, 0xf0, AX88141},
  {"ASIX", "ASIX AX88140",                                 BUSTYPE_PCI, PCI_UNITCODE(0x125B, 0x1400), 0xffffffff, 0,                            0,          0,    0,    AX88140},
  {"Lite-On", "Lite-On LC82C115 PNIC-II",                  BUSTYPE_PCI, PCI_UNITCODE(0x11AD, 0xc115), 0xffffffff, 0,                            0,          0,    0,    PNIC2},
  {"ADMtek", "ADMtek AN981 Comet",                         BUSTYPE_PCI, PCI_UNITCODE(0x1317, 0x0981), 0xffffffff, 0,                            0,          0,    0,    COMET},
  {"ADMtek", "ADMtek Centaur-P",                           BUSTYPE_PCI, PCI_UNITCODE(0x1317, 0x0985), 0xffffffff, 0,                            0,          0,    0,    COMET},
  {"ADMtek", "ADMtek Centaur-C",                           BUSTYPE_PCI, PCI_UNITCODE(0x1317, 0x1985), 0xffffffff, 0,                            0,          0,    0,    COMET},
  {"D-Link", "D-Link DFE-680TXD v1.0 (ADMtek Centaur-C)",  BUSTYPE_PCI, PCI_UNITCODE(0x1186, 0x1541), 0xffffffff, 0,                            0,          0,    0,    COMET},
  {"ADMtek", "ADMtek Centaur-C (Linksys v2)",              BUSTYPE_PCI, PCI_UNITCODE(0x13d1, 0xab02), 0xffffffff, 0,                            0,          0,    0,    COMET},
  {"ADMtek", "ADMtek Centaur-C (Linksys)",                 BUSTYPE_PCI, PCI_UNITCODE(0x13d1, 0xab03), 0xffffffff, 0,                            0,          0,    0,    COMET},
  {"ADMtek", "ADMtek Centaur-C (Linksys)",                 BUSTYPE_PCI, PCI_UNITCODE(0x13d1, 0xab08), 0xffffffff, 0,                            0,          0,    0,    COMET},
  {"ADMtek", "ADMtek Centaur-C (Linksys PCM200 v3)",       BUSTYPE_PCI, PCI_UNITCODE(0x1737, 0xab09), 0xffffffff, 0,                            0,          0,    0,    COMET},
  {"STMicro", "STMicro STE10/100 Comet",                   BUSTYPE_PCI, PCI_UNITCODE(0x104a, 0x0981), 0xffffffff, 0,                            0,          0,    0,    COMET},
  {"STMicro", "STMicro STE10/100A Comet",                  BUSTYPE_PCI, PCI_UNITCODE(0x104a, 0x2774), 0xffffffff, 0,                            0,          0,    0,    COMET},
  {"ADMtek", "ADMtek Comet-II",                            BUSTYPE_PCI, PCI_UNITCODE(0x1317, 0x9511), 0xffffffff, 0,                            0,          0,    0,    COMET},
  {"ADMtek", "ADMtek Comet-II (9513)",                     BUSTYPE_PCI, PCI_UNITCODE(0x1317, 0x9513), 0xffffffff, 0,                            0,          0,    0,    COMET},
  {"SMC", "SMC1255TX (ADMtek Comet)",                      BUSTYPE_PCI, PCI_UNITCODE(0x1113, 0x1216), 0xffffffff, PCI_UNITCODE(0x10b8, 0x1255), 0xffffffff, 0,    0,    COMET},
  {"Accton", "Accton EN1217/EN2242 (ADMtek Comet)",        BUSTYPE_PCI, PCI_UNITCODE(0x1113, 0x1216), 0xffffffff, 0,                            0,          0,    0,    COMET},
  {"SMC", "SMC1255TX (ADMtek Comet-II)",                   BUSTYPE_PCI, PCI_UNITCODE(0x10b8, 0x1255), 0xffffffff, 0,                            0,          0,    0,    COMET},
  {"ADMtek", "ADMtek Comet-II (model 1020)",               BUSTYPE_PCI, PCI_UNITCODE(0x111a, 0x1020), 0xffffffff, 0,                            0,          0,    0,    COMET},
  {"Allied Telesyn", "Allied Telesyn A120 (ADMtek Comet)", BUSTYPE_PCI, PCI_UNITCODE(0x1259, 0xa120), 0xffffffff, 0,                            0,          0,    0,    COMET},
  {"Compex", "Compex RL100-TX",                            BUSTYPE_PCI, PCI_UNITCODE(0x11F6, 0x9881), 0xffffffff, 0,                            0,          0,    0,    COMPEX9881},
  {"Intel", "Intel 21145 Tulip",                           BUSTYPE_PCI, PCI_UNITCODE(0x8086, 0x0039), 0xffffffff, 0,                            0,          0,    0,    I21145},
  {"Xircom", "Xircom Tulip clone",                         BUSTYPE_PCI, PCI_UNITCODE(0x115d, 0x0003), 0xffffffff, 0,                            0,          0,    0,    XIRCOM},
  {"Davicom", "Davicom DM9102",                            BUSTYPE_PCI, PCI_UNITCODE(0x1282, 0x9102), 0xffffffff, 0,                            0,          0,    0,    DC21140},
  {"Davicom", "Davicom DM9100",                            BUSTYPE_PCI, PCI_UNITCODE(0x1282, 0x9100), 0xffffffff, 0,                            0,          0,    0,    DC21140},
  {"Macronix", "Macronix mxic-98715 (EN1217)",             BUSTYPE_PCI, PCI_UNITCODE(0x1113, 0x1217), 0xffffffff, 0,                            0,          0,    0,    MX98715},
  {"Conexant", "Conexant LANfinity",                       BUSTYPE_PCI, PCI_UNITCODE(0x14f1, 0x1803), 0xffffffff, 0,                            0,          0,    0,    CONEXANT},
  {"3Com", "3Com 3cSOHO100B-TX (ADMtek Centaur)",          BUSTYPE_PCI, PCI_UNITCODE(0x10b7, 0x9300), 0xffffffff, 0,                            0,          0,    0,    COMET},
  {NULL,},
};

// This table is used during operation for capabilities and media timer

static void tulip_timer(void *data);
static void nway_timer(void *data);
static void mxic_timer(void *data);
static void pnic_timer(void *data);
static void comet_timer(void *data);

enum tbl_flag {
  HAS_MII             = 0x001,
  HAS_MEDIA_TABLE     = 0x002,
  CSR12_IN_SROM       = 0x004,
  ALWAYS_CHECK_MII    = 0x008,
  HAS_PWRDWN          = 0x010, 
  MC_HASH_ONLY        = 0x020, // Hash-only multicast filter
  HAS_PNICNWAY        = 0x080, 
  HAS_NWAY            = 0x040, // Uses internal NWay xcvr
  HAS_INTR_MITIGATION = 0x100, 
  IS_ASIX             = 0x200, 
  HAS_8023X           = 0x400,
  COMET_MAC_ADDR      = 0x800,
};

//
// Note: this table must match  enum tulip_chips  above
//

struct tulip_chip_table {
  char *chip_name;
  int io_size;          // Unused
  int valid_intrs;      // CSR7 interrupt enable settings
  int flags;
  void (*media_timer)(void *data);
};

static struct tulip_chip_table tulip_tbl[] = {
  {"Digital DC21040 Tulip", 128, 0x0001ebef, 0, tulip_timer},
  {"Digital DC21041 Tulip", 128, 0x0001ebff, HAS_MEDIA_TABLE | HAS_NWAY, tulip_timer},
  {"Digital DS21140 Tulip", 128, 0x0001ebef, HAS_MII | HAS_MEDIA_TABLE | CSR12_IN_SROM, tulip_timer},
  {"Digital DS21143 Tulip", 128, 0x0801fbff, HAS_MII | HAS_MEDIA_TABLE | ALWAYS_CHECK_MII | HAS_PWRDWN | HAS_NWAY | HAS_INTR_MITIGATION, nway_timer},
  {"Lite-On 82c168 PNIC", 256, 0x0001ebef, HAS_MII | HAS_PNICNWAY, pnic_timer},
  {"Macronix 98713 PMAC", 128, 0x0001ebef, HAS_MII | HAS_MEDIA_TABLE | CSR12_IN_SROM, mxic_timer},
  {"Macronix 98715 PMAC", 256, 0x0001ebef, HAS_MEDIA_TABLE, mxic_timer},
  {"Macronix 98725 PMAC", 256, 0x0001ebef, HAS_MEDIA_TABLE, mxic_timer},
  {"ASIX AX88140", 128, 0x0001fbff, HAS_MII | HAS_MEDIA_TABLE | CSR12_IN_SROM | MC_HASH_ONLY | IS_ASIX, tulip_timer},
  {"ASIX AX88141", 128, 0x0001fbff, HAS_MII | HAS_MEDIA_TABLE | CSR12_IN_SROM | MC_HASH_ONLY | IS_ASIX, tulip_timer},
  {"Lite-On PNIC-II", 256, 0x0801fbff, HAS_MII | HAS_NWAY | HAS_8023X, nway_timer},
  {"ADMtek Comet", 256, 0x0001abef, HAS_MII | MC_HASH_ONLY | COMET_MAC_ADDR, comet_timer},
  {"Compex 9881 PMAC", 128, 0x0001ebef, HAS_MII | HAS_MEDIA_TABLE | CSR12_IN_SROM, mxic_timer},
  {"Intel DS21145 Tulip", 128, 0x0801fbff, HAS_MII | HAS_MEDIA_TABLE | ALWAYS_CHECK_MII | HAS_PWRDWN | HAS_NWAY, nway_timer},
  {"Xircom tulip work-alike", 128, 0x0801fbff, HAS_MII | HAS_MEDIA_TABLE | ALWAYS_CHECK_MII | HAS_PWRDWN | HAS_NWAY, nway_timer},
  {"Conexant LANfinity", 256, 0x0001ebef, HAS_MII | HAS_PWRDWN, tulip_timer},
  {0},
};

//
// A full-duplex map for media types
//

enum MediaIs {
  MediaIsFD     = 1, 
  MediaAlwaysFD = 2, 
  MediaIsMII    = 4, 
  MediaIsFx     = 8,
  MediaIs100    = 16
};

static const char media_cap[32] = {0,0,0,16,  3,19,16,24,  27,4,7,5, 0,20,23,20,  28,31,0,0, };
static unsigned char t21040_csr13[] = {2,0x0C,8,4,  4,0,0,0, 0,0,0,0, 4,0,0,0};

// 21041 transceiver register settings: 10-T, 10-2, AUI, 10-T, 10T-FD
static unsigned short t21041_csr13[] = { 0xEF01, 0xEF09, 0xEF09, 0xEF01, 0xEF09, };
static unsigned short t21041_csr14[] = { 0xFFFF, 0xF7FD, 0xF7FD, 0x6F3F, 0x6F3D, };
static unsigned short t21041_csr15[] = { 0x0008, 0x0006, 0x000E, 0x0008, 0x0008, };

static unsigned short t21142_csr13[] = { 0x0001, 0x0009, 0x0009, 0x0000, 0x0001, };
static unsigned short t21142_csr14[] = { 0xFFFF, 0x0705, 0x0705, 0x0000, 0x7F3D, };
static unsigned short t21142_csr15[] = { 0x0008, 0x0006, 0x000E, 0x0008, 0x0008, };

//
// Offsets to the Command and Status Registers, "CSRs".  All accesses
// must be longword instructions and quadword aligned.
//

enum tulip_offsets {
  CSR0  = 0,    
  CSR1  = 0x08, 
  CSR2  = 0x10, 
  CSR3  = 0x18, 
  CSR4  = 0x20, 
  CSR5  = 0x28,
  CSR6  = 0x30, 
  CSR7  = 0x38, 
  CSR8  = 0x40, 
  CSR9  = 0x48, 
  CSR10 = 0x50, 
  CSR11 = 0x58,
  CSR12 = 0x60, 
  CSR13 = 0x68, 
  CSR14 = 0x70, 
  CSR15 = 0x78
};

// The bits in the CSR5 status registers, mostly interrupt sources

enum status_bits {
  TimerInt        = 0x800, 
  TPLnkFail       = 0x1000, 
  TPLnkPass       = 0x10,
  NormalIntr      = 0x10000, 
  AbnormalIntr    = 0x8000, 
  PCIBusError     = 0x2000,
  RxJabber        = 0x200, 
  RxStopped       = 0x100, 
  RxNoBuf         = 0x80, 
  RxIntr          = 0x40,
  TxFIFOUnderflow = 0x20, 
  TxJabber        = 0x08, 
  TxNoBuf         = 0x04, 
  TxDied          = 0x02, 
  TxIntr          = 0x01,
};

// The configuration bits in CSR6

enum csr6_mode_bits {
  TxOn               = 0x2000, 
  RxOn               = 0x0002, 
  FullDuplex         = 0x0200,
  AcceptBroadcast    = 0x0100, 
  AcceptAllMulticast = 0x0080,
  AcceptAllPhys      = 0x0040, 
  AcceptRunt         = 0x0008,
};

// The Tulip Rx and Tx buffer descriptors

struct tulip_rx_desc {
  long status;
  long length;
  unsigned long buffer1, buffer2;
};

struct tulip_tx_desc {
  long status;
  long length;
  unsigned long buffer1, buffer2;       // We use only buffer 1
};

enum desc_status_bits {
  DescOwned      = 0x80000000, 
  RxDescFatalErr = 0x8000, 
  RxWholePkt     = 0x0300,
};

//
// Ring-wrap flag in length field, use for last ring entry.
// 0x01000000 means chain on buffer2 address,
// 0x02000000 means use the ring start address in CSR2/3.
// Note: Some work-alike chips do not function correctly in chained mode.
// The ASIX chip works only in chained mode.
// Thus we indicates ring mode, but always write the 'next' field for
// chained mode as well.
//

#define DESC_RING_WRAP 0x02000000

#define EEPROM_SIZE 512   // support 256*16 EEPROMs

struct medialeaf {
  unsigned char type;
  unsigned char media;
  unsigned char *leafdata;
};

struct mediatable {
  unsigned short defaultmedia;
  unsigned char leafcount, csr12dir;       // General purpose pin directions
  unsigned has_mii:1, has_nonmii:1, has_reset:6;
  unsigned long csr15dir, csr15val;       // 21143 NWay setting
  struct medialeaf mleaf[0];
};

struct mediainfo {
  struct mediainfo *next;
  int info_type;
  int index;
  unsigned char *info;
};

struct tulip_private {
  struct tulip_rx_desc rx_ring[RX_RING_SIZE];
  struct tulip_tx_desc tx_ring[TX_RING_SIZE];
  // The saved addresses of Rx/Tx-in-place packet buffers
  struct pbuf *tx_pbuf[TX_RING_SIZE];
  struct pbuf *rx_pbuf[RX_RING_SIZE];
  struct dev *next_module;
  unsigned short setup_frame[96];      // Pseudo-Tx frame to init address table
  unsigned long mc_filter[2];          // Multicast hash filter
  struct eth_addr hwaddr;              // MAC address for NIC
  dev_t devno;
  struct unit *pci_dev;
  int chip_id, revision;
  int flags;
  int iobase;                         // Configured I/O base
  int irq;                            // Configured IRQ
  struct timer timer;                 // Media selection timer
  struct interrupt intr;              // Interrupt object for driver
  struct dpc dpc;                     // DPC for driver
  unsigned int csr0, csr6;            // Current CSR0, CSR6 settings
  // Note: cache line pairing and isolation of Rx vs. Tx indicies
  struct sem tx_sem;                  // Semaphore for Tx ring not full
  unsigned int cur_rx, dirty_rx;      // Producer/consumer ring indices
  struct stats_nic stats;
  unsigned int cur_tx, dirty_tx;
  unsigned int tx_full:1;             // The Tx queue is full
  unsigned int rx_dead:1;             // We have no Rx buffers
  unsigned int full_duplex:1;         // Full-duplex operation requested
  unsigned int full_duplex_lock:1;
  unsigned int fake_addr:1;           // Multiport board faked address
  unsigned int media2:4;              // Secondary monitored media port
  unsigned int medialock:1;           // Do not sense media type
  unsigned int mediasense:1;          // Media sensing in progress
  unsigned int nway:1, nwayset:1;     // 21143 internal NWay
  unsigned int default_port:8;        // Last if_port value
  unsigned char eeprom[EEPROM_SIZE];  // Serial EEPROM contents
  void (*link_change)(struct dev *dev, int csr5);
  unsigned short lpar;                // 21143 Link partner ability
  unsigned short sym_advertise, mii_advertise; // NWay to-advertise
  unsigned short advertising[4];      // MII advertise, from SROM table
  signed char phys[4], mii_cnt;       // MII device addresses
  //spinlock_t mii_lock;
  struct mediatable *mtable;
  int cur_index;                      // Current media index
  int if_port;
  int saved_if_port;
  int trans_start;
  int last_rx;
};

static void start_link(struct dev *dev);
static void parse_eeprom(struct dev *dev);
static int read_eeprom(long ioaddr, int location, int addr_len);
static int mdio_read(struct dev *dev, int phy_id, int location);
static void mdio_write(struct dev *dev, int phy_id, int location, int value);
static int tulip_open(struct dev *dev);

// Chip-specific media selection (timer functions prototyped above)
static int  check_duplex(struct dev *dev);
static void select_media(struct dev *dev, int startup);
static void init_media(struct dev *dev);
static void nway_lnk_change(struct dev *dev, int csr5);
static void nway_start(struct dev *dev);
static void pnic_lnk_change(struct dev *dev, int csr5);
static void pnic_do_nway(struct dev *dev);

static void tulip_tx_timeout(struct dev *dev);
static void tulip_init_ring(struct dev *dev);
static int tulip_rx(struct dev *dev);
static void tulip_interrupt(int irq, void *dev_instance, struct pt_regs *regs);
static int tulip_close(struct dev *dev);
static struct stats_nic *tulip_get_stats(struct dev *dev);
static int tulip_set_rx_mode(struct dev *dev);

// A list of all installed Tulip devices

static struct dev *root_tulip_dev = NULL;

//
// Start the link, typically called at probe1() time but sometimes later with
// multiport cards
//

static void start_link(struct dev *dev) {
  struct tulip_private *tp = (struct tulip_private *) dev->privdata;
  long ioaddr = tp->iobase;
  int i;

  if ((tp->flags & ALWAYS_CHECK_MII) || (tp->mtable && tp->mtable->has_mii) || (!tp->mtable && (tp->flags & HAS_MII))) {
    int phyn, phy_idx = 0;
    
    if (tp->mtable && tp->mtable->has_mii) {
      for (i = 0; i < tp->mtable->leafcount; i++) {
        if (tp->mtable->mleaf[i].media == 11) {
          tp->cur_index = i;
          tp->saved_if_port = tp->if_port;
          select_media(dev, 2);
          tp->if_port = tp->saved_if_port;
          break;
        }
      }
    }

    // Find the connected MII xcvrs
    // Doing this in open() would allow detecting external xcvrs later,
    // but takes much time

    for (phyn = 1; phyn <= 32 && phy_idx < sizeof(tp->phys); phyn++) {
      int phy = phyn & 0x1f;
      int mii_status = mdio_read(dev, phy, 1);
      if ((mii_status & 0x8301) == 0x8001 || ((mii_status & 0x8000) == 0  && (mii_status & 0x7800) != 0)) {
        int mii_reg0 = mdio_read(dev, phy, 0);
        int mii_advert = mdio_read(dev, phy, 4);
        int to_advert;

        if (tp->mii_advertise) {
          to_advert = tp->mii_advertise;
        } else if (tp->advertising[phy_idx]) {
          to_advert = tp->advertising[phy_idx];
        } else {
          // Leave unchanged
          tp->mii_advertise = to_advert = mii_advert;
        }

        tp->phys[phy_idx++] = phy;
        kprintf(KERN_INFO "%s:  MII transceiver #%d config %4.4x status %4.4x advertising %4.4x.\n", dev->name, phy, mii_reg0, mii_status, mii_advert);

        // Fixup for DLink with miswired PHY
        if (mii_advert != to_advert) {
          kprintf(KERN_DEBUG "%s:  Advertising %4.4x on PHY %d, previously advertising %4.4x.\n", dev->name, to_advert, phy, mii_advert);
          mdio_write(dev, phy, 4, to_advert);
        }

        // Enable autonegotiation: some boards default to off
        mdio_write(dev, phy, 0, (mii_reg0 & ~0x3000) | (tp->full_duplex ? 0x0100 : 0x0000) | ((media_cap[tp->default_port] & MediaIs100) ? 0x2000 : 0x1000));
      }
    }

    tp->mii_cnt = phy_idx;
    if (tp->mtable && tp->mtable->has_mii && phy_idx == 0) {
      kprintf(KERN_INFO "%s: ***WARNING***: No MII transceiver found!\n", dev->name);
      tp->phys[0] = 1;
    }
  }

  // Reset the xcvr interface and turn on heartbeat
  switch (tp->chip_id) {
    case DC21040:
      writel(0x00000000, ioaddr + CSR13);
      writel(0x00000004, ioaddr + CSR13);
      break;

    case DC21041:
      // This is nway_start()
      if (tp->sym_advertise == 0) tp->sym_advertise = 0x0061;
      writel(0x00000000, ioaddr + CSR13);
      writel(0xFFFFFFFF, ioaddr + CSR14);
      writel(0x00000008, ioaddr + CSR15); // Listen on AUI also
      writel(readl(ioaddr + CSR6) | FullDuplex, ioaddr + CSR6);
      writel(0x0000EF01, ioaddr + CSR13);
      break;

    case DC21140: 
    default:
      if (tp->mtable) writel(tp->mtable->csr12dir | 0x100, ioaddr + CSR12);
      break;

    case DC21142:
    case PNIC2:
      if (tp->mii_cnt || media_cap[tp->if_port] & MediaIsMII) {
        writel(0x82020000, ioaddr + CSR6);
        writel(0x0000, ioaddr + CSR13);
        writel(0x0000, ioaddr + CSR14);
        writel(0x820E0000, ioaddr + CSR6);
      } else {
        nway_start(dev);
      }
      break;

    case LC82C168:
      if (!tp->mii_cnt) {
        tp->nway = 1;
        tp->nwayset = 0;
        writel(0x00420000, ioaddr + CSR6);
        writel(0x30, ioaddr + CSR12);
        writel(0x0001F078, ioaddr + 0xB8);
        writel(0x0201F078, ioaddr + 0xB8); // Turn on autonegotiation
      }
      break;

    case MX98713: 
    case COMPEX9881:
      writel(0x00000000, ioaddr + CSR6);
      writel(0x000711C0, ioaddr + CSR14); // Turn on NWay
      writel(0x00000001, ioaddr + CSR13);
      break;

    case MX98715: 
    case MX98725:
      writel(0x01a80000, ioaddr + CSR6);
      writel(0xFFFFFFFF, ioaddr + CSR14);
      writel(0x00001000, ioaddr + CSR12);
      break;

    case COMET:
      break;
  }

  if (tp->flags & HAS_PWRDWN) pci_write_config_dword(tp->pci_dev, 0x40, 0x40000000);
}

//
// Serial EEPROM section
//
// The main routine to parse the very complicated SROM structure.
// Search www.digital.com for "21X4 SROM" to get details.
// This code is very complex, and will require changes to support
// additional cards, so I will be verbose about what is going on.
//
// Known cards that have old-style EEPROMs.
// Writing this table is described at http://www.scyld.com/network/tulip-media.html
//

struct fixups {
  char *name;
  unsigned char addr0, addr1, addr2;
  unsigned short newtable[32];       /* Max length below. */
};

static struct fixups eeprom_fixups[] = {
  {"Asante", 0, 0, 0x94, {0x1e00, 0x0000, 0x0800, 0x0100, 0x018c,
              0x0000, 0x0000, 0xe078, 0x0001, 0x0050, 0x0018}},
  {"SMC9332DST", 0, 0, 0xC0, { 0x1e00, 0x0000, 0x0800, 0x041f,
                 0x0000, 0x009E, // 10baseT
                 0x0004, 0x009E, // 10baseT-FD
                 0x0903, 0x006D, // 100baseTx
                 0x0905, 0x006D, // 100baseTx-FD 
  }},
  {"Cogent EM100", 0, 0, 0x92, { 0x1e00, 0x0000, 0x0800, 0x063f,
                 0x0107, 0x8021, // 100baseFx
                 0x0108, 0x8021, // 100baseFx-FD
                 0x0100, 0x009E, // 10baseT
                 0x0104, 0x009E, // 10baseT-FD
                 0x0103, 0x006D, // 100baseTx
                 0x0105, 0x006D, // 100baseTx-FD
  }},
  {"Maxtech NX-110", 0, 0, 0xE8, { 0x1e00, 0x0000, 0x0800, 0x0513,
                 0x1001, 0x009E, // 10base2, CSR12 0x10
                 0x0000, 0x009E, // 10baseT
                 0x0004, 0x009E, // 10baseT-FD
                 0x0303, 0x006D, // 100baseTx, CSR12 0x03
                 0x0305, 0x006D, // 100baseTx-FD CSR12 0x03
  }},
  {"Accton EN1207", 0, 0, 0xE8, { 0x1e00, 0x0000, 0x0800, 0x051F,
                  0x1B01, 0x0000, // 10base2,   CSR12 0x1B
                  0x0B00, 0x009E, // 10baseT,   CSR12 0x0B
                  0x0B04, 0x009E, // 10baseT-FD,CSR12 0x0B
                  0x1B03, 0x006D, // 100baseTx, CSR12 0x1B
                  0x1B05, 0x006D, // 100baseTx-FD CSR12 0x1B
  }},
  {NULL,}
};

static char *block_name[] = {
  "21140 non-MII", "21140 MII PHY", "21142 Serial PHY", "21142 MII PHY", "21143 SYM PHY", "21143 reset method"
};

#define get_u16(ptr) (*(unsigned short *)(ptr))

static void parse_eeprom(struct dev *dev) {
  // The last media info list parsed, for multiport boards
  static struct mediatable *last_mediatable = NULL;
  static unsigned char *last_ee_data = NULL;
  static int controller_index = 0;
  struct tulip_private *tp = (struct tulip_private *) dev->privdata;
  unsigned char *p, *ee_data = tp->eeprom;
  int new_advertise = 0;
  int i;

  tp->mtable = 0;
  
  // Detect an old-style (SA only) EEPROM layout: memcmp(eedata, eedata + 16, 8)
  for (i = 0; i < 8; i ++) {
    if (ee_data[i] != ee_data[16 + i]) break;
  }

  if (i >= 8) {
    if (ee_data[0] == 0xff) {
      if (last_mediatable) {
        controller_index++;
        kprintf(KERN_INFO "%s:  Controller %d of multiport board.\n", dev->name, controller_index);
        tp->mtable = last_mediatable;
        ee_data = last_ee_data;
        goto subsequent_board;
      } else {
        kprintf(KERN_INFO "%s:  Missing EEPROM, this interface may not work correctly!\n", dev->name);
      }
      return;
    }

    // Do a fix-up based on the vendor half of the station address
    for (i = 0; eeprom_fixups[i].name; i++) {
      if (tp->hwaddr.addr[0] == eeprom_fixups[i].addr0 &&
          tp->hwaddr.addr[1] == eeprom_fixups[i].addr1 &&
          tp->hwaddr.addr[2] == eeprom_fixups[i].addr2) {
        if (tp->hwaddr.addr[2] == 0xE8 && ee_data[0x1a] == 0x55) i++;  // An Accton EN1207, not an outlaw Maxtech
        memcpy(ee_data + 26, eeprom_fixups[i].newtable, sizeof(eeprom_fixups[i].newtable));
        kprintf(KERN_INFO "%s: Old format EEPROM on '%s' board. Using substitute media control info.\n", dev->name, eeprom_fixups[i].name);
        break;
      }
    }

    if (eeprom_fixups[i].name == NULL) {
      // No fixup found
      kprintf(KERN_INFO "%s: Old style EEPROM with no media selection information.\n", dev->name);
      return;
    }
  }

  controller_index = 0;
  if (ee_data[19] > 1) {
    struct dev *prev_dev;
    struct tulip_private *otp;

    // This is a multiport board. The probe order may be "backwards", so we patch up already found devices.
    last_ee_data = ee_data;
    for (prev_dev = tp->next_module; prev_dev; prev_dev = otp->next_module) {
      otp = (struct tulip_private *) prev_dev->privdata;
      if (otp->eeprom[0] == 0xff && otp->mtable == 0) {
        parse_eeprom(prev_dev);
        start_link(prev_dev);
      } else {
        break;
      }
    }
    controller_index = 0;
  }

subsequent_board:
  p = (void *) (ee_data + ee_data[27 + controller_index * 3]);
  if (ee_data[27] == 0) {
    // No valid media table
  } else if (tp->chip_id == DC21041) {
    int media = get_u16(p);
    int count = p[2];
    p += 3;

    kprintf(KERN_INFO "%s: 21041 Media table, default media %4.4x (%s).\n", dev->name, media, media & 0x0800 ? "Autosense" : medianame[media & MEDIA_MASK]);
    for (i = 0; i < count; i++) {
      unsigned char media_block = *p++;
      int media_code = media_block & MEDIA_MASK;
      
      if (media_block & 0x40) p += 6;
      switch(media_code) {
        case 0: new_advertise |= 0x0020; break;
        case 4: new_advertise |= 0x0040; break;
      }
      kprintf(KERN_INFO "%s:  21041 media #%d, %s.\n", dev->name, media_code, medianame[media_code]);
    }
  } else {
    unsigned char csr12dir = 0;
    int count;
    struct mediatable *mtable;
    unsigned short media = get_u16(p);

    p += 2;
    if (tp->flags & CSR12_IN_SROM) csr12dir = *p++;
    count = *p++;
    mtable = (struct mediatable *) kmalloc(sizeof(struct mediatable) + count * sizeof(struct medialeaf));
    if (mtable == NULL) return;
    last_mediatable = tp->mtable = mtable;
    mtable->defaultmedia = media;
    mtable->leafcount = count;
    mtable->csr12dir = csr12dir;
    mtable->has_nonmii = mtable->has_mii = mtable->has_reset = 0;
    mtable->csr15dir = mtable->csr15val = 0;

    kprintf(KERN_INFO "%s: EEPROM default media type %s.\n", dev->name, media & 0x0800 ? "Autosense" : medianame[media & MEDIA_MASK]);
    for (i = 0; i < count; i++) {
      struct medialeaf *leaf = &mtable->mleaf[i];

      if ((p[0] & 0x80) == 0) {
        // 21140 Compact block
        leaf->type = 0;
        leaf->media = p[0] & 0x3f;
        leaf->leafdata = p;
        if ((p[2] & 0x61) == 0x01)  mtable->has_mii = 1; // Bogus, but Znyx boards do it
        p += 4;
      } else {
        switch(leaf->type = p[1]) {
          case 5:
            mtable->has_reset = i + 1; // Assure non-zero
            // Fall through

          case 6:
            leaf->media = 31;
            break;

          case 1: 
          case 3:
            mtable->has_mii = 1;
            leaf->media = 11;
            break;

          case 2:
            if ((p[2] & 0x3f) == 0) {
              unsigned long base15 = (p[2] & 0x40) ? get_u16(p + 7) : 0x0008;
              unsigned short *p1 = (unsigned short *)(p + (p[2] & 0x40 ? 9 : 3));
              mtable->csr15dir = (p1[0] << 16) + base15;
              mtable->csr15val = (p1[1] << 16) + base15;
            }
            // Fall through

          case 0: 
          case 4:
            mtable->has_nonmii = 1;
            leaf->media = p[2] & MEDIA_MASK;
            switch (leaf->media) {
              case 0: new_advertise |= 0x0020; break;
              case 4: new_advertise |= 0x0040; break;
              case 3: new_advertise |= 0x0080; break;
              case 5: new_advertise |= 0x0100; break;
              case 6: new_advertise |= 0x0200; break;
            }
            break;

          default:
            leaf->media = 19;
        }
        leaf->leafdata = p + 2;
        p += (p[0] & 0x3f) + 1;
      }
    }
    if (new_advertise) tp->sym_advertise = new_advertise;
  }
}

// Reading a serial EEPROM is a "bit" grungy, but we work our way through:->

// EEPROM_Ctrl bits

#define EE_SHIFT_CLK  0x02  // EEPROM shift clock
#define EE_CS         0x01  // EEPROM chip select
#define EE_DATA_WRITE 0x04  // Data from the Tulip to EEPROM
#define EE_WRITE_0    0x01
#define EE_WRITE_1    0x05
#define EE_DATA_READ  0x08  // Data from the EEPROM chip
#define EE_ENB        (0x4800 | EE_CS)

// Delay between EEPROM clock transitions.
// Even at 33Mhz current PCI implementations do not overrun the EEPROM clock.
// We add a bus turn-around to insure that this remains true

#define eeprom_delay()  readl(ee_addr)

// The EEPROM commands include the alway-set leading bit

#define EE_READ_CMD   (6)

// Note: this routine returns extra data bits for size detection

static int read_eeprom(long ioaddr, int location, int addr_len) {
  int i;
  unsigned retval = 0;
  long ee_addr = ioaddr + CSR9;
  int read_cmd = location | (EE_READ_CMD << addr_len);

  writel(EE_ENB & ~EE_CS, ee_addr);
  writel(EE_ENB, ee_addr);

  // Shift the read command bits out
  for (i = 4 + addr_len; i >= 0; i--) {
    short dataval = (read_cmd & (1 << i)) ? EE_DATA_WRITE : 0;
    writel(EE_ENB | dataval, ee_addr);
    eeprom_delay();
    writel(EE_ENB | dataval | EE_SHIFT_CLK, ee_addr);
    eeprom_delay();
    retval = (retval << 1) | ((readl(ee_addr) & EE_DATA_READ) ? 1 : 0);
  }
  writel(EE_ENB, ee_addr);
  eeprom_delay();

  for (i = 16; i > 0; i--) {
    writel(EE_ENB | EE_SHIFT_CLK, ee_addr);
    eeprom_delay();
    retval = (retval << 1) | ((readl(ee_addr) & EE_DATA_READ) ? 1 : 0);
    writel(EE_ENB, ee_addr);
    eeprom_delay();
  }

  // Terminate the EEPROM access
  writel(EE_ENB & ~EE_CS, ee_addr);
  return retval;
}

//
// MII transceiver control section.
// Read and write the MII registers using software-generated serial
// MDIO protocol.  See the MII specifications or DP83840A data sheet
// for details
//

// The maximum data clock rate is 2.5 Mhz.  The minimum timing is usually
// met by back-to-back PCI I/O cycles, but we insert a delay to avoid
// "overclocking" issues or future 66Mhz PCI

#define mdio_delay() readl(mdio_addr)

// Read and write the MII registers using software-generated serial
// MDIO protocol.  It is just different enough from the EEPROM protocol
// to not share code.  The maxium data clock rate is 2.5 Mhz

#define MDIO_SHIFT_CLK   0x10000
#define MDIO_DATA_WRITE0 0x00000
#define MDIO_DATA_WRITE1 0x20000
#define MDIO_ENB         0x00000   // Ignore the 0x02000 databook setting
#define MDIO_ENB_IN      0x40000
#define MDIO_DATA_READ   0x80000

static const unsigned char comet_miireg2offset[32] = {
  0xB4, 0xB8, 0xBC, 0xC0,  0xC4, 0xC8, 0xCC, 0,  0,0,0,0,  0,0,0,0,
  0,0xD0,0,0,  0,0,0,0,  0,0,0,0, 0, 0xD4, 0xD8, 0xDC, 
};

static int mdio_read(struct dev *dev, int phy_id, int location) {
  struct tulip_private *tp = (struct tulip_private *) dev->privdata;
  int i;
  int read_cmd = (0xf6 << 10) | ((phy_id & 0x1f) << 5) | location;
  int retval = 0;
  long ioaddr = tp->iobase;
  long mdio_addr = ioaddr + CSR9;
  //unsigned long flags;

  if (location & ~0x1f) return 0xffff;

  if (tp->chip_id == COMET && phy_id == 30) {
    if (comet_miireg2offset[location]) {
      return readl(ioaddr + comet_miireg2offset[location]);
    }
    return 0xffff;
  }

  //spin_lock_irqsave(&tp->mii_lock, flags);
  if (tp->chip_id == LC82C168) {
    int i = 1000;
    writel(0x60020000 + (phy_id<<23) + (location<<18), ioaddr + 0xA0);
    readl(ioaddr + 0xA0);
    readl(ioaddr + 0xA0);
    readl(ioaddr + 0xA0);
    readl(ioaddr + 0xA0);
    
    while (--i > 0) {
      if (!((retval = readl(ioaddr + 0xA0)) & 0x80000000)) break;
    }

    //spin_unlock_irqrestore(&tp->mii_lock, flags);
    return retval & 0xffff;
  }

  // Establish sync by sending at least 32 logic ones
  for (i = 32; i >= 0; i--) {
    writel(MDIO_ENB | MDIO_DATA_WRITE1, mdio_addr);
    mdio_delay();
    writel(MDIO_ENB | MDIO_DATA_WRITE1 | MDIO_SHIFT_CLK, mdio_addr);
    mdio_delay();
  }

  // Shift the read command bits out
  for (i = 15; i >= 0; i--) {
    int dataval = (read_cmd & (1 << i)) ? MDIO_DATA_WRITE1 : 0;

    writel(MDIO_ENB | dataval, mdio_addr);
    mdio_delay();
    writel(MDIO_ENB | dataval | MDIO_SHIFT_CLK, mdio_addr);
    mdio_delay();
  }

  // Read the two transition, 16 data, and wire-idle bits
  for (i = 19; i > 0; i--) {
    writel(MDIO_ENB_IN, mdio_addr);
    mdio_delay();
    retval = (retval << 1) | ((readl(mdio_addr) & MDIO_DATA_READ) ? 1 : 0);
    writel(MDIO_ENB_IN | MDIO_SHIFT_CLK, mdio_addr);
    mdio_delay();
  }

  //spin_unlock_irqrestore(&tp->mii_lock, flags);
  
  return (retval >> 1) & 0xffff;
}

static void mdio_write(struct dev *dev, int phy_id, int location, int val) {
  struct tulip_private *tp = (struct tulip_private *) dev->privdata;
  int i;
  int cmd = (0x5002 << 16) | (phy_id << 23) | (location << 18) | (val & 0xffff);
  long ioaddr = tp->iobase;
  long mdio_addr = ioaddr + CSR9;

  if (location & ~0x1f) return;

  if (tp->chip_id == COMET && phy_id == 30) {
    if (comet_miireg2offset[location]) writel(val, ioaddr + comet_miireg2offset[location]);
    return;
  }

  if (tp->chip_id == LC82C168) {
    int i = 1000;
    writel(cmd, ioaddr + 0xA0);
    do {
      if (!(readl(ioaddr + 0xA0) & 0x80000000)) break;
    } while (--i > 0);
    return;
  }

  // Establish sync by sending 32 logic ones
  for (i = 32; i >= 0; i--)  {
    writel(MDIO_ENB | MDIO_DATA_WRITE1, mdio_addr);
    mdio_delay();
    writel(MDIO_ENB | MDIO_DATA_WRITE1 | MDIO_SHIFT_CLK, mdio_addr);
    mdio_delay();
  }
  
  // Shift the command bits out
  for (i = 31; i >= 0; i--) {
    int dataval = (cmd & (1 << i)) ? MDIO_DATA_WRITE1 : 0;
    writel(MDIO_ENB | dataval, mdio_addr);
    mdio_delay();
    writel(MDIO_ENB | dataval | MDIO_SHIFT_CLK, mdio_addr);
    mdio_delay();
  }

  // Clear out extra bits
  for (i = 2; i > 0; i--) {
    writel(MDIO_ENB_IN, mdio_addr);
    mdio_delay();
    writel(MDIO_ENB_IN | MDIO_SHIFT_CLK, mdio_addr);
    mdio_delay();
  }
}

//
// Set up the transceiver control registers for the selected media type.
// STARTUP indicates to reset the transceiver.  It is set to '2' for
// the initial card detection, and '1' during resume or open().
//

static void select_media(struct dev *dev, int startup) {
  struct tulip_private *tp = (struct tulip_private *) dev->privdata;
  long ioaddr = tp->iobase;
  struct mediatable *mtable = tp->mtable;
  unsigned long new_csr6;
  int i;

  if (mtable) {
    struct medialeaf *mleaf = &mtable->mleaf[tp->cur_index];
    unsigned char *p = mleaf->leafdata;
    
    //kprintf(KERN_DEBUG "%s:  Media table type %d.\n", dev->name, mleaf->type);

    switch (mleaf->type) {
      case 0: // 21140 non-MII xcvr
        //kprintf(KERN_DEBUG "%s: Using a 21140 non-MII transceiver with control setting %2.2x.\n", dev->name, p[1]);
        tp->if_port = p[0];
        if (startup) writel(mtable->csr12dir | 0x100, ioaddr + CSR12);
        writel(p[1], ioaddr + CSR12);
        new_csr6 = 0x02000000 | ((p[2] & 0x71) << 18);
        break;

      case 2: 
      case 4: {
        unsigned short setup[5];
        unsigned long csr13val, csr14val, csr15dir, csr15val;
        for (i = 0; i < 5; i++) setup[i] = get_u16(&p[i * 2 + 1]);

        tp->if_port = p[0] & MEDIA_MASK;
        if (media_cap[tp->if_port] & MediaAlwaysFD) tp->full_duplex = 1;

        if (startup && mtable->has_reset) {
          struct medialeaf *rleaf = &mtable->mleaf[mtable->has_reset-1];
          unsigned char *rst = rleaf->leafdata;
          kprintf(KERN_DEBUG "%s: Resetting the transceiver.\n", dev->name);
          for (i = 0; i < rst[0]; i++) writel(get_u16(rst + 1 + (i<<1)) << 16, ioaddr + CSR15);
        }

        kprintf(KERN_DEBUG "%s: 21143 non-MII %s transceiver control %4.4x/%4.4x.\n", dev->name, medianame[tp->if_port], setup[0], setup[1]);
        if (p[0] & 0x40) {
          // SIA (CSR13-15) setup values are provided
          csr13val = setup[0];
          csr14val = setup[1];
          csr15dir = (setup[3]<<16) | setup[2];
          csr15val = (setup[4]<<16) | setup[2];
          writel(0, ioaddr + CSR13);
          writel(csr14val, ioaddr + CSR14);
          writel(csr15dir, ioaddr + CSR15); // Direction
          writel(csr15val, ioaddr + CSR15); // Data
          writel(csr13val, ioaddr + CSR13);
        } else {
          csr13val = 1;
          csr14val = 0x0003FFFF;
          csr15dir = (setup[0]<<16) | 0x0008;
          csr15val = (setup[1]<<16) | 0x0008;
          if (tp->if_port <= 4) csr14val = t21142_csr14[tp->if_port];
          if (startup) {
            writel(0, ioaddr + CSR13);
            writel(csr14val, ioaddr + CSR14);
          }
          writel(csr15dir, ioaddr + CSR15); // Direction
          writel(csr15val, ioaddr + CSR15); // Data
          if (startup) writel(csr13val, ioaddr + CSR13);
        }
        kprintf(KERN_DEBUG "%s:  Setting CSR15 to %8.8x/%8.8x.\n", dev->name, csr15dir, csr15val);
        if (mleaf->type == 4) {
          new_csr6 = 0x820A0000 | ((setup[2] & 0x71) << 18);
        } else {
          new_csr6 = 0x82420000;
        }
        break;
      }

      case 1: 
      case 3: {
        int phy_num = p[0];
        int init_length = p[1];
        unsigned short *misc_info;

        tp->if_port = 11;
        new_csr6 = 0x020E0000;
        if (mleaf->type == 3) { 
          // 21142
          unsigned short *init_sequence = (unsigned short *)(p + 2);
          unsigned short *reset_sequence = &((unsigned short *)(p + 3))[init_length];
          int reset_length = p[2 + init_length * 2];
          misc_info = reset_sequence + reset_length;
          if (startup) {
            for (i = 0; i < reset_length; i++) {
              writel(get_u16(&reset_sequence[i]) << 16, ioaddr + CSR15);
             }
          }
          for (i = 0; i < init_length; i++) {
            writel(get_u16(&init_sequence[i]) << 16, ioaddr + CSR15);
          }
        } else {
          unsigned char *init_sequence = p + 2;
          unsigned char *reset_sequence = p + 3 + init_length;
          int reset_length = p[2 + init_length];
          misc_info = (unsigned short *)(reset_sequence + reset_length);
          if (startup) {
            writel(mtable->csr12dir | 0x100, ioaddr + CSR12);
            for (i = 0; i < reset_length; i++) writel(reset_sequence[i], ioaddr + CSR12);
          }
          for (i = 0; i < init_length; i++) writel(init_sequence[i], ioaddr + CSR12);
        }
        tp->advertising[phy_num] = get_u16(&misc_info[1]) | 1;
        if (startup < 2) {
          if (tp->mii_advertise == 0) tp->mii_advertise = tp->advertising[phy_num];
          kprintf(KERN_DEBUG "%s:  Advertising %4.4x on MII %d.\n", dev->name, tp->mii_advertise, tp->phys[phy_num]);
          mdio_write(dev, tp->phys[phy_num], 4, tp->mii_advertise);
        }
        break;
      }

      default:
        kprintf(KERN_DEBUG "%s:  Invalid media table selection %d.\n", dev->name, mleaf->type);
        new_csr6 = 0x020E0000;
    }

    //kprintf(KERN_DEBUG "%s: Using media type %s, CSR12 is %2.2x.\n", dev->name, medianame[tp->if_port], readl(ioaddr + CSR12) & 0xff);
  } else if (tp->chip_id == DC21041) {
    int port = tp->if_port <= 4 ? tp->if_port : 0;
    kprintf(KERN_DEBUG "%s: 21041 using media %s, CSR12 is %4.4x.\n", dev->name, medianame[port == 3 ? 12: port], readl(ioaddr + CSR12));
    writel(0x00000000, ioaddr + CSR13); /* Reset the serial interface */
    writel(t21041_csr14[port], ioaddr + CSR14);
    writel(t21041_csr15[port], ioaddr + CSR15);
    writel(t21041_csr13[port], ioaddr + CSR13);
    new_csr6 = 0x80020000;
  } else if (tp->chip_id == LC82C168) {
    if (startup && ! tp->medialock) tp->if_port = tp->mii_cnt ? 11 : 0;
    kprintf(KERN_DEBUG "%s: PNIC PHY status is %3.3x, media %s.\n", dev->name, readl(ioaddr + 0xB8), medianame[tp->if_port]);
    if (tp->mii_cnt) {
      new_csr6 = 0x810C0000;
      writel(0x0001, ioaddr + CSR15);
      writel(0x0201B07A, ioaddr + 0xB8);
    } else if (startup) {
      // Start with 10mbps to do autonegotiation
      writel(0x32, ioaddr + CSR12);
      new_csr6 = 0x00420000;
      writel(0x0001B078, ioaddr + 0xB8);
      writel(0x0201B078, ioaddr + 0xB8);
    } else if (tp->if_port == 3 || tp->if_port == 5) {
      writel(0x33, ioaddr + CSR12);
      new_csr6 = 0x01860000;
      // Trigger autonegotiation
      writel(startup ? 0x0201F868 : 0x0001F868, ioaddr + 0xB8);
    } else {
      writel(0x32, ioaddr + CSR12);
      new_csr6 = 0x00420000;
      writel(0x1F078, ioaddr + 0xB8);
    }
  } else if (tp->chip_id == DC21040) {
    // 21040
    
    // Turn on the xcvr interface
    int csr12 = readl(ioaddr + CSR12);
    kprintf(KERN_DEBUG "%s: 21040 media type is %s, CSR12 is %2.2x.\n", dev->name, medianame[tp->if_port], csr12);
    if (media_cap[tp->if_port] & MediaAlwaysFD) tp->full_duplex = 1;
    new_csr6 = 0x20000;
    
    // Set the full duplux match frame
    writel(FULL_DUPLEX_MAGIC, ioaddr + CSR11);
    writel(0x00000000, ioaddr + CSR13); // Reset the serial interface
    if (t21040_csr13[tp->if_port] & 8) {
      writel(0x0705, ioaddr + CSR14);
      writel(0x0006, ioaddr + CSR15);
    } else {
      writel(0xffff, ioaddr + CSR14);
      writel(0x0000, ioaddr + CSR15);
    }
    writel(0x8f01 | t21040_csr13[tp->if_port], ioaddr + CSR13);
  } else {
    // Unknown chip type with no media table
    if (tp->default_port == 0) tp->if_port = tp->mii_cnt ? 11 : 3;

    if (media_cap[tp->if_port] & MediaIsMII) {
      new_csr6 = 0x020E0000;
    } else if (media_cap[tp->if_port] & MediaIsFx) {
      new_csr6 = 0x02860000;
    } else {
      new_csr6 = 0x038E0000;
    }
    kprintf(KERN_DEBUG "%s: No media description table, assuming %s transceiver, CSR12 %2.2x.\n", dev->name, medianame[tp->if_port], readl(ioaddr + CSR12));
  }

  tp->csr6 = new_csr6 | (tp->csr6 & 0xfdff) | (tp->full_duplex ? FullDuplex : 0);
}

static void init_media(struct dev *dev) {
  struct tulip_private *tp = (struct tulip_private *) dev->privdata;
  long ioaddr = tp->iobase;
  int i;

  tp->saved_if_port = tp->if_port;
  if (tp->if_port == 0) tp->if_port = tp->default_port;

  // Allow selecting a default media
  i = 0;
  if (tp->mtable == NULL) goto media_picked;

  if (tp->if_port) {
    int looking_for = media_cap[tp->if_port] & MediaIsMII ? 11 : (tp->if_port == 12 ? 0 : tp->if_port);
    for (i = 0; i < tp->mtable->leafcount; i++) {
      if (tp->mtable->mleaf[i].media == looking_for) {
        kprintf(KERN_INFO "%s: Using user-specified media %s.\n", dev->name, medianame[tp->if_port]);
        goto media_picked;
      }
    }
  }
  if ((tp->mtable->defaultmedia & 0x0800) == 0) {
    int looking_for = tp->mtable->defaultmedia & MEDIA_MASK;
    for (i = 0; i < tp->mtable->leafcount; i++) {
      if (tp->mtable->mleaf[i].media == looking_for) {
        kprintf(KERN_INFO "%s: Using EEPROM-set media %s.\n", dev->name, medianame[looking_for]);
        goto media_picked;
      }
    }
  }

  // Start sensing first non-full-duplex media
  for (i = tp->mtable->leafcount - 1; (media_cap[tp->mtable->mleaf[i].media] & MediaAlwaysFD) && i > 0; i--);

media_picked:
  tp->csr6 = 0;
  tp->cur_index = i;
  tp->nwayset = 0;

  if (tp->if_port) {
    if (tp->chip_id == DC21143  && (media_cap[tp->if_port] & MediaIsMII)) {
      // We must reset the media CSRs when we force-select MII mode
      writel(0x0000, ioaddr + CSR13);
      writel(0x0000, ioaddr + CSR14);
      writel(0x0008, ioaddr + CSR15);
    }
    select_media(dev, 1);
    return;
  }

  switch(tp->chip_id) {
    case DC21041:
      // tp->nway = 1;
      nway_start(dev);
      break;

    case DC21142:
      if (tp->mii_cnt) {
        select_media(dev, 1);
        kprintf(KERN_INFO "%s: Using MII transceiver %d, status %4.4x.\n", dev->name, tp->phys[0], mdio_read(dev, tp->phys[0], 1));
        writel(0x82020000, ioaddr + CSR6);
        tp->csr6 = 0x820E0000;
        tp->if_port = 11;
        writel(0x0000, ioaddr + CSR13);
        writel(0x0000, ioaddr + CSR14);
      } else {
        nway_start(dev);
      }
      break;

    case PNIC2:
      nway_start(dev);
      break;

    case LC82C168:
      if (tp->mii_cnt) {
        tp->if_port = 11;
        tp->csr6 = 0x814C0000 | (tp->full_duplex ? FullDuplex : 0);
        writel(0x0001, ioaddr + CSR15);
      } else if (readl(ioaddr + CSR5) & TPLnkPass) {
        pnic_do_nway(dev);
      } else {
        // Start with 10mbps to do autonegotiation
        writel(0x32, ioaddr + CSR12);
        tp->csr6 = 0x00420000;
        writel(0x0001B078, ioaddr + 0xB8);
        writel(0x0201B078, ioaddr + 0xB8);
      }
      break;

    case MX98713: 
    case COMPEX9881:
      tp->if_port = 0;
      tp->csr6 = 0x01880000 | (tp->full_duplex ? FullDuplex : 0);
      writel(0x0f370000 | readw(ioaddr + 0x80), ioaddr + 0x80);
      break;

    case MX98715: 
    case MX98725:
      // Provided by BOLO, Macronix - 12/10/1998
      tp->if_port = 0;
      tp->csr6 = 0x01a80000 | FullDuplex;
      writel(0x0f370000 | readw(ioaddr + 0x80), ioaddr + 0x80);
      writel(0x11000 | readw(ioaddr + 0xa0), ioaddr + 0xa0);
      break;

    case COMET: 
    case CONEXANT:
      // Enable automatic Tx underrun recovery
      writel(readl(ioaddr + 0x88) | 1, ioaddr + 0x88);
      tp->if_port = tp->mii_cnt ? 11 : 0;
      tp->csr6 = 0x00040000;
      break;

    case AX88140: 
    case AX88141:
      tp->csr6 = tp->mii_cnt ? 0x00040100 : 0x00000100;
      break;

    default:
      select_media(dev, 1);
  }
}

//
// Check the MII negotiated duplex, and change the CSR6 setting if
// required.
// Return 0 if everything is OK.
// Return < 0 if the transceiver is missing or has no link beat.
//

static int check_duplex(struct dev *dev) {
  struct tulip_private *tp = (struct tulip_private *) dev->privdata;
  long ioaddr = tp->iobase;
  int mii_reg1, mii_reg5, negotiated, duplex;

  if (tp->full_duplex_lock) return 0;
  mii_reg5 = mdio_read(dev, tp->phys[0], 5);
  negotiated = mii_reg5 & tp->mii_advertise;

  kprintf(KERN_INFO "%s: MII link partner %4.4x, negotiated %4.4x.\n", dev->name, mii_reg5, negotiated);

  if (mii_reg5 == 0xffff) return -2;

  if ((mii_reg5 & 0x4000) == 0 && ((mii_reg1 = mdio_read(dev, tp->phys[0], 1)) & 0x0004) == 0) {
    int new_reg1 = mdio_read(dev, tp->phys[0], 1);
    if ((new_reg1 & 0x0004) == 0) {
      kprintf(KERN_INFO "%s: No link beat on the MII interface, status %4.4x.\n", dev->name, new_reg1);
      return -1;
    }
  }

  duplex = ((negotiated & 0x0300) == 0x0100 || (negotiated & 0x00C0) == 0x0040);

  // 100baseTx-FD or 10T-FD, but not 100-HD
  if (tp->full_duplex != duplex) {
    tp->full_duplex = duplex;
    if (negotiated & 0x0380) tp->csr6 &= ~0x00400000;
    
    if (tp->full_duplex) {
      tp->csr6 |= FullDuplex;
     } else {
      tp->csr6 &= ~FullDuplex;
     }

    writel(tp->csr6 | RxOn, ioaddr + CSR6);
    writel(tp->csr6 | TxOn | RxOn, ioaddr + CSR6);
    
    kprintf(KERN_INFO "%s: Setting %s-duplex based on MII #%d link partner capability of %4.4x.\n",
           dev->name, tp->full_duplex ? "full" : "half",
           tp->phys[0], mii_reg5);

    return 1;
  }

  return 0;
}

static void tulip_timer(void *data) {
  struct dev *dev = (struct dev *) data;
  struct tulip_private *tp = (struct tulip_private *) dev->privdata;
  long ioaddr = tp->iobase;
  unsigned long csr12 = readl(ioaddr + CSR12);
  int next_tick = 2 * HZ;

  //kprintf(KERN_DEBUG "%s: Media selection tick, %s, status %8.8x mode %8.8x SIA %8.8x %8.8x %8.8x %8.8x.\n",
  //     dev->name, medianame[tp->if_port], readl(ioaddr + CSR5),
  //     readl(ioaddr + CSR6), csr12, readl(ioaddr + CSR13),
  //     readl(ioaddr + CSR14), readl(ioaddr + CSR15));

  switch (tp->chip_id) {
    case DC21040:
      if (!tp->medialock && (csr12 & 0x0002)) { 
        // Network error
        kprintf(KERN_INFO "%s: No link beat found.\n", dev->name);
        tp->if_port = (tp->if_port == 2 ? 0 : 2);
        select_media(dev, 0);
        tp->trans_start = get_ticks();
      }
      break;

    case DC21041:
      kprintf(KERN_DEBUG "%s: 21041 media tick  CSR12 %8.8x.\n", dev->name, csr12);
      if (tp->medialock) break;
      switch (tp->if_port) {
        case 0: 
        case 3: 
        case 4:
          if (csr12 & 0x0004) { 
            // LnkFail. 10baseT is dead.  Check for activity on alternate port.
            tp->mediasense = 1;
            if (csr12 & 0x0200) {
              tp->if_port = 2;
            } else {
              tp->if_port = 1;
            }
            kprintf(KERN_INFO "%s: No 21041 10baseT link beat, Media switched to %s.\n", dev->name, medianame[tp->if_port]);
            writel(0, ioaddr + CSR13); // Reset
            writel(t21041_csr14[tp->if_port], ioaddr + CSR14);
            writel(t21041_csr15[tp->if_port], ioaddr + CSR15);
            writel(t21041_csr13[tp->if_port], ioaddr + CSR13);
            next_tick = 10 * HZ;      // 10 sec
          } else {
            next_tick = 30 * HZ;
          }
          break;

        case 1: // 10base2
        case 2: // AUI
          if (csr12 & 0x0100) {
            next_tick = (30 * HZ); // 30 sec
            tp->mediasense = 0;
          } else if ((csr12 & 0x0004) == 0) {
            kprintf(KERN_INFO "%s: 21041 media switched to 10baseT.\n", dev->name);
            tp->if_port = 0;
            select_media(dev, 0);
            next_tick = (24 * HZ) / 10;       // 2.4 sec
          } else if (tp->mediasense || (csr12 & 0x0002)) {
            tp->if_port = 3 - tp->if_port; // Swap ports
            select_media(dev, 0);
            next_tick = 20 * HZ;
          } else {
            next_tick = 20 * HZ;
          }
          break;
      }
      break;

    case DC21140:  
    case DC21142: 
    case MX98713: 
    case COMPEX9881: 
    default: {
      struct medialeaf *mleaf;
      unsigned char *p;
      if (tp->mtable == NULL) { 
        // No EEPROM info, use generic code
        // Not much that can be done. Assume this a generic MII or SYM transceiver
        next_tick = 60 * HZ;
        kprintf(KERN_DEBUG "%s: network media monitor CSR6 %8.8x CSR12 0x%2.2x.\n", dev->name, readl(ioaddr + CSR6), csr12 & 0xff);
        break;
      }
      mleaf = &tp->mtable->mleaf[tp->cur_index];
      p = mleaf->leafdata;
      switch (mleaf->type) {
        case 0: case 4: {
          // Type 0 serial or 4 SYM transceiver.  Check the link beat bit.
          int offset = mleaf->type == 4 ? 5 : 2;
          char bitnum = p[offset];
          if (p[offset + 1] & 0x80) {
            kprintf(KERN_DEBUG "%s: Transceiver monitor tick CSR12=%#2.2x, no media sense.\n", dev->name, csr12);
            if (mleaf->type == 4) {
              if (mleaf->media == 3 && (csr12 & 0x02)) goto select_next_media;
            }
            break;
          }

          //kprintf(KERN_DEBUG "%s: Transceiver monitor tick: CSR12=%#2.2x bit %d is %d, expecting %d.\n",
          //     dev->name, csr12, (bitnum >> 1) & 7,
          //     (csr12 & (1 << ((bitnum >> 1) & 7))) != 0,
          //     (bitnum >= 0));

          // Check that the specified bit has the proper value.
          if ((bitnum < 0) != ((csr12 & (1 << ((bitnum >> 1) & 7))) != 0)) {
            //kprintf(KERN_DEBUG "%s: Link beat detected for %s.\n", dev->name, medianame[mleaf->media & MEDIA_MASK]);
            if ((p[2] & 0x61) == 0x01) goto actually_mii; // Bogus Znyx board
            break;
          }

          if (tp->medialock) break;

        select_next_media:
          if (--tp->cur_index < 0) {
            // We start again, but should instead look for default
            tp->cur_index = tp->mtable->leafcount - 1;
          }

          tp->if_port = tp->mtable->mleaf[tp->cur_index].media;
          if (media_cap[tp->if_port] & MediaIsFD) goto select_next_media; // Skip FD entries
          kprintf(KERN_DEBUG "%s: No link beat on media %s, trying transceiver type %s.\n",
                 dev->name, medianame[mleaf->media & MEDIA_MASK],
                 medianame[tp->mtable->mleaf[tp->cur_index].media]);

          select_media(dev, 0);

          // Restart the transmit process
          writel(tp->csr6 | RxOn, ioaddr + CSR6);
          writel(tp->csr6 | TxOn | RxOn, ioaddr + CSR6);
          next_tick = (24 * HZ) / 10;
          break;
        }

        case 1:  
        case 3: // 21140, 21142 MII
        actually_mii:
          check_duplex(dev);
          next_tick = 60 * HZ;
          break;

        case 2: // 21142 serial block has no link beat. */
        default:
          break;
      }
    }
    break;
  }

  mod_timer(&tp->timer, get_ticks() + next_tick);
}

//
// Handle internal NWay transceivers uniquely.
// These exist on the 21041, 21143 (in SYM mode) and the PNIC2.
//

static void nway_timer(void *data) {
  struct dev *dev = (struct dev *) data;
  struct tulip_private *tp = (struct tulip_private *) dev->privdata;
  long ioaddr = tp->iobase;
  int csr12 = readl(ioaddr + CSR12);
  int next_tick = 60 * HZ;
  int new_csr6 = 0;

  kprintf(KERN_INFO "%s: N-Way autonegotiation status %8.8x, %s.\n", dev->name, csr12, medianame[tp->if_port]);
  
  if (media_cap[tp->if_port] & MediaIsMII) {
    check_duplex(dev);
  } else if (tp->nwayset) {
    // Do not screw up a negotiated session!
    kprintf(KERN_INFO "%s: Using NWay-set %s media, csr12 %8.8x.\n", dev->name, medianame[tp->if_port], csr12);
  } else if (tp->medialock) {
  } else if (tp->if_port == 3) {
    if (csr12 & 2) {  
      // No 100mbps link beat, revert to 10mbps.
      kprintf(KERN_INFO "%s: No 21143 100baseTx link beat, %8.8x, trying NWay.\n", dev->name, csr12);
      nway_start(dev);
      next_tick = 3 * HZ;
    }
  } else if ((csr12 & 0x7000) != 0x5000) {
    // Negotiation failed.  Search media types.
    kprintf(KERN_INFO "%s: 21143 negotiation failed, status %8.8x.\n", dev->name, csr12);

    if (!(csr12 & 4)) {
      // 10mbps link beat good
      new_csr6 = 0x82420000;
      tp->if_port = 0;
      writel(0, ioaddr + CSR13);
      writel(0x0003FFFF, ioaddr + CSR14);
      writew(t21142_csr15[tp->if_port], ioaddr + CSR15);
      writel(t21142_csr13[tp->if_port], ioaddr + CSR13);
    } else {
      // Select 100mbps port to check for link beat
      new_csr6 = 0x83860000;
      tp->if_port = 3;
      writel(0, ioaddr + CSR13);
      writel(0x0003FF7F, ioaddr + CSR14);
      writew(8, ioaddr + CSR15);
      writel(1, ioaddr + CSR13);
    }

    kprintf(KERN_INFO "%s: Testing new 21143 media %s.\n", dev->name, medianame[tp->if_port]);

    if (new_csr6 != (tp->csr6 & ~0x20D7)) {
      tp->csr6 &= 0x20D7;
      tp->csr6 |= new_csr6;
      writel(0x0301, ioaddr + CSR12);
      writel(tp->csr6 | RxOn, ioaddr + CSR6);
      writel(tp->csr6 | TxOn | RxOn, ioaddr + CSR6);
    }
    
    next_tick = 3 * HZ;
  }

  if (tp->cur_tx - tp->dirty_tx > 0  && get_ticks() - tp->trans_start > TX_TIMEOUT) {
    kprintf(KERN_WARNING "%s: Tx hung, %d vs. %d.\n", dev->name, tp->cur_tx, tp->dirty_tx);
    tulip_tx_timeout(dev);
  }

  mod_timer(&tp->timer, get_ticks() + next_tick);
}

static void nway_start(struct dev *dev) {
  struct tulip_private *tp = (struct tulip_private *) dev->privdata;
  long ioaddr = tp->iobase;
  int csr14 = ((tp->sym_advertise & 0x0780) << 9) | ((tp->sym_advertise & 0x0020) << 1) | 0xffbf;

  tp->if_port = 0;
  tp->nway = tp->mediasense = 1;
  tp->nwayset = tp->lpar = 0;
  
  if (tp->chip_id == PNIC2) {
    tp->csr6 = 0x01000000 | (tp->sym_advertise & 0x0040 ? FullDuplex : 0);
    return;
  }

  kprintf(KERN_DEBUG "%s: Restarting internal NWay autonegotiation, %8.8x.\n", dev->name, csr14);
  writel(0x0001, ioaddr + CSR13);
  writel(csr14, ioaddr + CSR14);
  tp->csr6 = 0x82420000 | (tp->sym_advertise & 0x0040 ? FullDuplex : 0) | (tp->csr6 & 0x20ff);
  writel(tp->csr6, ioaddr + CSR6);
  if (tp->mtable && tp->mtable->csr15dir) {
    writel(tp->mtable->csr15dir, ioaddr + CSR15);
    writel(tp->mtable->csr15val, ioaddr + CSR15);
  } else if (tp->chip_id != PNIC2) {
    writew(0x0008, ioaddr + CSR15);
  }

  if (tp->chip_id == DC21041) {
    writel(0xEF01, ioaddr + CSR12);
  } else {
    writel(0x1301, ioaddr + CSR12);
  }
}

static void nway_lnk_change(struct dev *dev, int csr5) {
  struct tulip_private *tp = (struct tulip_private *) dev->privdata;
  long ioaddr = tp->iobase;
  int csr12 = readl(ioaddr + CSR12);

  if (tp->chip_id == PNIC2) {
    kprintf(KERN_INFO "%s: PNIC-2 link status changed, CSR5/12/14 %8.8x %8.8x, %8.8x.\n", dev->name, csr12, csr5, readl(ioaddr + CSR14));
    tp->if_port = 5;
    tp->lpar = csr12 >> 16;
    tp->nwayset = 1;
    tp->csr6 = 0x01000000 | (tp->csr6 & 0xffff);
    writel(tp->csr6, ioaddr + CSR6);
    return;
  }

  kprintf(KERN_INFO "%s: 21143 link status interrupt %8.8x, CSR5 %x, %8.8x.\n", dev->name, csr12, csr5, readl(ioaddr + CSR14));

  // If NWay finished and we have a negotiated partner capability
  if (tp->nway && !tp->nwayset && (csr12 & 0x7000) == 0x5000) {
    int setup_done = 0;
    int negotiated = tp->sym_advertise & (csr12 >> 16);
    tp->lpar = csr12 >> 16;
    tp->nwayset = 1;

    if (negotiated & 0x0100) {
      tp->if_port = 5;
    } else if (negotiated & 0x0080) {
      tp->if_port = 3;
    } else if (negotiated & 0x0040) {
      tp->if_port = 4;
    } else if (negotiated & 0x0020) {
      tp->if_port = 0;
    } else {
      tp->nwayset = 0;
      if ((csr12 & 2) == 0 && (tp->sym_advertise & 0x0180)) tp->if_port = 3;
    }

    tp->full_duplex = (media_cap[tp->if_port] & MediaAlwaysFD) ? 1 : 0;

    if (tp->nwayset) {
      kprintf(KERN_INFO "%s: Switching to %s based on link negotiation %4.4x & %4.4x = %4.4x.\n", dev->name, medianame[tp->if_port], tp->sym_advertise, tp->lpar, negotiated);
    } else {
      kprintf(KERN_INFO "%s: Autonegotiation failed, using %s, link beat status %4.4x.\n", dev->name, medianame[tp->if_port], csr12);
    }

    if (tp->mtable) {
      int i;
      for (i = 0; i < tp->mtable->leafcount; i++) {
        if (tp->mtable->mleaf[i].media == tp->if_port) {
          tp->cur_index = i;
          select_media(dev, 0);
          setup_done = 1;
          break;
        }
      }
    }

    if (! setup_done) {
      tp->csr6 = (tp->if_port & 1 ? 0x838E0000 : 0x82420000) | (tp->csr6 & 0x20ff);
      if (tp->full_duplex) tp->csr6 |= FullDuplex;
      writel(1, ioaddr + CSR13);
    }

    writel(tp->csr6 | TxOn | RxOn, ioaddr + CSR6);
    kprintf(KERN_DEBUG "%s:  Setting CSR6 %8.8x/%x CSR12 %8.8x.\n", dev->name, tp->csr6, readl(ioaddr + CSR6), readl(ioaddr + CSR12));
  } else if ((tp->nwayset && (csr5 & 0x08000000) &&
             (tp->if_port == 3 || tp->if_port == 5) &&
             (csr12 & 2) == 2) || (tp->nway && (csr5 & (TPLnkFail)))) {
    // Link blew? Maybe restart NWay
    del_timer(&tp->timer);
    nway_start(dev);
    mod_timer(&tp->timer, get_ticks() + 3 * HZ);
  } else if (tp->if_port == 3 || tp->if_port == 5) {
    kprintf(KERN_INFO "%s: 21143 %s link beat %s.\n", dev->name, medianame[tp->if_port], (csr12 & 2) ? "failed" : "good");
    if ((csr12 & 2) && ! tp->medialock) {
      del_timer(&tp->timer);
      nway_start(dev);
      mod_timer(&tp->timer, get_ticks() + 3 * HZ);
    } else if (tp->if_port == 5) {
      writel(readl(ioaddr + CSR14) & ~0x080, ioaddr + CSR14);
    }
  } else if (tp->if_port == 0 || tp->if_port == 4) {
    if ((csr12 & 4) == 0) {
      kprintf(KERN_INFO "%s: 21143 10baseT link beat good.\n", dev->name);
    }
  } else if (!(csr12 & 4)) {
    // 10mbps link beat good
    kprintf(KERN_INFO "%s: 21143 10mbps sensed media.\n", dev->name);
    tp->if_port = 0;
  } else if (tp->nwayset) {
    kprintf(KERN_INFO "%s: 21143 using NWay-set %s, csr6 %8.8x.\n", dev->name, medianame[tp->if_port], tp->csr6);
  } else {    
    // 100mbps link beat good
    kprintf(KERN_INFO "%s: 21143 100baseTx sensed media.\n", dev->name);
    tp->if_port = 3;
    tp->csr6 = 0x838E0000 | (tp->csr6 & 0x20ff);
    writel(0x0003FF7F, ioaddr + CSR14);
    writel(0x0301, ioaddr + CSR12);
    writel(tp->csr6 | RxOn, ioaddr + CSR6);
    writel(tp->csr6 | RxOn | TxOn, ioaddr + CSR6);
  }
}

static void mxic_timer(void *data) {
  struct dev *dev = (struct dev *)data;
  struct tulip_private *tp = (struct tulip_private *) dev->privdata;
  long ioaddr = tp->iobase;
  int next_tick = 60 * HZ;

  kprintf(KERN_INFO "%s: MXIC negotiation status %8.8x.\n", dev->name, readl(ioaddr + CSR12));
  mod_timer(&tp->timer, get_ticks() + next_tick);
}

static void pnic_do_nway(struct dev *dev) {
  struct tulip_private *tp = (struct tulip_private *) dev->privdata;
  long ioaddr = tp->iobase;
  unsigned long phy_reg = readl(ioaddr + 0xB8);
  unsigned long new_csr6 = tp->csr6 & ~0x40C40200;

  if (phy_reg & 0x78000000) { 
    // Ignore baseT4
    if (phy_reg & 0x20000000) {
      tp->if_port = 5;
    } else if (phy_reg & 0x40000000) {
      tp->if_port = 3;
    } else if (phy_reg & 0x10000000) {
      tp->if_port = 4;
    } else if (phy_reg & 0x08000000) {
      tp->if_port = 0;
    }

    tp->nwayset = 1;
    new_csr6 = (tp->if_port & 1) ? 0x01860000 : 0x00420000;
    writel(0x32 | (tp->if_port & 1), ioaddr + CSR12);
    if (tp->if_port & 1) writel(0x1F868, ioaddr + 0xB8);
    if (phy_reg & 0x30000000) {
      tp->full_duplex = 1;
      new_csr6 |= FullDuplex;
    }

    kprintf(KERN_DEBUG "%s: PNIC autonegotiated status %8.8x, %s.\n", dev->name, phy_reg, medianame[tp->if_port]);

    if (tp->csr6 != new_csr6) {
      tp->csr6 = new_csr6;
      writel(tp->csr6 | RxOn, ioaddr + CSR6); // Restart Tx
      writel(tp->csr6 | TxOn | RxOn, ioaddr + CSR6);
      tp->trans_start = get_ticks();
    }
  }
}

static void pnic_lnk_change(struct dev *dev, int csr5) {
  struct tulip_private *tp = (struct tulip_private *) dev->privdata;
  long ioaddr = tp->iobase;
  int phy_reg = readl(ioaddr + 0xB8);

  kprintf(KERN_DEBUG "%s: PNIC link changed state %8.8x, CSR5 %8.8x.\n", dev->name, phy_reg, csr5);

  if (readl(ioaddr + CSR5) & TPLnkFail) {
    writel((readl(ioaddr + CSR7) & ~TPLnkFail) | TPLnkPass, ioaddr + CSR7);
    if (!tp->nwayset || get_ticks() - tp->trans_start > 1 * HZ) {
      tp->csr6 = 0x00420000 | (tp->csr6 & 0x0000fdff);
      writel(tp->csr6, ioaddr + CSR6);
      writel(0x30, ioaddr + CSR12);
      writel(0x0201F078, ioaddr + 0xB8); // Turn on autonegotiation
      tp->trans_start = get_ticks();
    }
  } else if (readl(ioaddr + CSR5) & TPLnkPass) {
    pnic_do_nway(dev);
    writel((readl(ioaddr + CSR7) & ~TPLnkPass) | TPLnkFail, ioaddr + CSR7);
  }
}

static void pnic_timer(void *data) {
  struct dev *dev = (struct dev *) data;
  struct tulip_private *tp = (struct tulip_private *) dev->privdata;
  long ioaddr = tp->iobase;
  int next_tick = 60 * HZ;

  if (media_cap[tp->if_port] & MediaIsMII) {
    if (check_duplex(dev) > 0) next_tick = 3 * HZ;
  } else {
    int csr12 = readl(ioaddr + CSR12);
    int new_csr6 = tp->csr6 & ~0x40C40200;
    int phy_reg = readl(ioaddr + 0xB8);
    int csr5 = readl(ioaddr + CSR5);

    kprintf(KERN_DEBUG "%s: PNIC timer PHY status %8.8x, %s CSR5 %8.8x.\n", dev->name, phy_reg, medianame[tp->if_port], csr5);

    if (phy_reg & 0x04000000) { 
      // Remote link fault
      writel(0x0201F078, ioaddr + 0xB8);
      next_tick = 1 * HZ;
      tp->nwayset = 0;
    } else if (phy_reg & 0x78000000) { 
      // Ignore baseT4
      pnic_do_nway(dev);
      next_tick = 60 * HZ;
    } else if (csr5 & TPLnkFail) { 
      // 100baseTx link beat
      kprintf(KERN_DEBUG "%s: %s link beat failed, CSR12 %4.4x, CSR5 %8.8x, PHY %3.3x.\n",
             dev->name, medianame[tp->if_port], csr12,
             readl(ioaddr + CSR5), readl(ioaddr + 0xB8));

      next_tick = 3 * HZ;

      if (tp->medialock) {
      } else if (tp->nwayset && (tp->if_port & 1)) {
        next_tick = 1 * HZ;
      } else if (tp->if_port == 0) {
        tp->if_port = 3;
        writel(0x33, ioaddr + CSR12);
        new_csr6 = 0x01860000;
        writel(0x1F868, ioaddr + 0xB8);
      } else {
        tp->if_port = 0;
        writel(0x32, ioaddr + CSR12);
        new_csr6 = 0x00420000;
        writel(0x1F078, ioaddr + 0xB8);
      }
      
      if (tp->csr6 != new_csr6) {
        tp->csr6 = new_csr6;
        writel(tp->csr6 | RxOn, ioaddr + CSR6); // Restart Tx
        writel(tp->csr6 | RxOn | TxOn, ioaddr + CSR6);
        tp->trans_start = get_ticks();
        kprintf(KERN_INFO "%s: Changing PNIC configuration to %s %s-duplex, CSR6 %8.8x.\n",
               dev->name, medianame[tp->if_port],
               tp->full_duplex ? "full" : "half", new_csr6);
      }
    }
  }

  mod_timer(&tp->timer, get_ticks() + next_tick);
}

static void comet_timer(void *data) {
  struct dev *dev = (struct dev *) data;
  struct tulip_private *tp = (struct tulip_private *) dev->privdata;
  int next_tick = 60 * HZ;

  kprintf(KERN_DEBUG "%s: Comet link status %4.4x partner capability %4.4x.\n",
         dev->name, mdio_read(dev, tp->phys[0], 1),
         mdio_read(dev, tp->phys[0], 5));

  check_duplex(dev);

  mod_timer(&tp->timer, get_ticks() + next_tick);
}

static void tulip_tx_timeout(struct dev *dev) {
  struct tulip_private *tp = (struct tulip_private *) dev->privdata;
  long ioaddr = tp->iobase;

  if (media_cap[tp->if_port] & MediaIsMII) {
    // Do nothing -- the media monitor should handle this
    int mii_bmsr = mdio_read(dev, tp->phys[0], 1);
    kprintf(KERN_WARNING "%s: Transmit timeout using MII device, status %4.4x.\n", dev->name, mii_bmsr);
    if (!(mii_bmsr & 0x0004)) {
      // No link beat present
      tp->trans_start = get_ticks();
      //netif_link_down(dev);
      return;
    }
  } else {
    switch (tp->chip_id) {
      case DC21040:
        if (!tp->medialock && readl(ioaddr + CSR12) & 0x0002) {
          tp->if_port = (tp->if_port == 2 ? 0 : 2);
          kprintf(KERN_INFO "%s: transmit timed out, switching to %s.\n", dev->name, medianame[tp->if_port]);
          select_media(dev, 0);
        }
        tp->trans_start = get_ticks();
        return; // Note: not break!

      case DC21041: {
        int csr12 = readl(ioaddr + CSR12);

        kprintf(KERN_WARNING "%s: 21041 transmit timed out, status %8.8x, CSR12 %8.8x, CSR13 %8.8x, CSR14 %8.8x, resetting...\n",
               dev->name, readl(ioaddr + CSR5), csr12,
               readl(ioaddr + CSR13), readl(ioaddr + CSR14));

        tp->mediasense = 1;
        if (! tp->medialock) {
          if (tp->if_port == 1 || tp->if_port == 2) {
            tp->if_port = (csr12 & 0x0004) ? 2 - tp->if_port : 0;
          } else {
            tp->if_port = 1;
          }
          select_media(dev, 0);
        }
        break;
      }

      case DC21142:
        if (tp->nwayset) {
          kprintf(KERN_WARNING "%s: Transmit timed out, status %8.8x, SIA %8.8x %8.8x %8.8x %8.8x, restarting NWay .\n",
              dev->name, (int)readl(ioaddr + CSR5),
              readl(ioaddr + CSR12), readl(ioaddr + CSR13),
              readl(ioaddr + CSR14), readl(ioaddr + CSR15));
          nway_start(dev);
          break;
        }
        // Fall through

      case DC21140: 
      case MX98713: 
      case COMPEX9881:
        kprintf(KERN_WARNING "%s: %s transmit timed out, status %8.8x, SIA %8.8x %8.8x %8.8x %8.8x, resetting...\n",
               dev->name, tulip_tbl[tp->chip_id].chip_name,
               readl(ioaddr + CSR5), readl(ioaddr + CSR12),
               readl(ioaddr + CSR13), readl(ioaddr + CSR14),
               readl(ioaddr + CSR15));

        if (!tp->medialock && tp->mtable) {
          do {
            --tp->cur_index;
          } while (tp->cur_index >= 0 && (media_cap[tp->mtable->mleaf[tp->cur_index].media] & MediaIsFD));

          if (tp->cur_index < 0) {
            // We start again, but should instead look for default
            tp->cur_index = tp->mtable->leafcount - 1;
          }

          select_media(dev, 0);
          kprintf(KERN_WARNING "%s: transmit timed out, switching to %s media.\n", dev->name, medianame[tp->if_port]);
        }
        break;

      case PNIC2:
        kprintf(KERN_WARNING "%s: PNIC2 transmit timed out, status %8.8x, CSR6/7 %8.8x / %8.8x CSR12 %8.8x, resetting...\n",
               dev->name, readl(ioaddr + CSR5), readl(ioaddr + CSR6),
               readl(ioaddr + CSR7), readl(ioaddr + CSR12));
        break;

      default:
        kprintf(KERN_WARNING "%s: Transmit timed out, status %8.8x, CSR12 %8.8x, resetting...\n",
               dev->name, readl(ioaddr + CSR5), readl(ioaddr + CSR12));
    }
  }

  // Stop and restart the Tx process.
  writel(tp->csr6 | RxOn, ioaddr + CSR6);
  writel(tp->csr6 | RxOn | TxOn, ioaddr + CSR6);

  // Trigger an immediate transmit demand
  writel(0, ioaddr + CSR1);
  writel(tulip_tbl[tp->chip_id].valid_intrs, ioaddr + CSR7);

  tp->trans_start = get_ticks();
  tp->stats.tx_errors++;
}

// Initialize the Rx and Tx rings, along with various 'dev' bits

static void tulip_init_ring(struct dev *dev) {
  struct tulip_private *tp = (struct tulip_private *) dev->privdata;
  int i;

  tp->rx_dead = tp->tx_full = 0;
  tp->cur_rx = tp->cur_tx = 0;
  tp->dirty_rx = tp->dirty_tx = 0;

  for (i = 0; i < RX_RING_SIZE; i++) {
    tp->rx_ring[i].status = 0x00000000;
    tp->rx_ring[i].length = PKT_BUF_SZ;
    tp->rx_ring[i].buffer2 = virt2phys(&tp->rx_ring[i + 1]);
    tp->rx_pbuf[i] = NULL;
  }

  // Mark the last entry as wrapping the ring
  tp->rx_ring[i - 1].length = PKT_BUF_SZ | DESC_RING_WRAP;
  tp->rx_ring[i - 1].buffer2 = virt2phys(&tp->rx_ring[0]);

  for (i = 0; i < RX_RING_SIZE; i++) {
    // Note the receive buffer must be longword aligned.
    // alloc_pbuf() provides the alignment.
    struct pbuf *pbuf = pbuf_alloc(PBUF_RAW, PKT_BUF_SZ, PBUF_RW);
    tp->rx_pbuf[i] = pbuf;
    if (pbuf == NULL) break;
    tp->rx_ring[i].status = DescOwned;
    tp->rx_ring[i].buffer1 = virt2phys(pbuf->payload);
  }

  tp->dirty_rx = (unsigned int)(i - RX_RING_SIZE);

  // The Tx buffer descriptor is filled in as needed, but we
  // do need to clear the ownership bit

  for (i = 0; i < TX_RING_SIZE; i++) {
    tp->tx_pbuf[i] = 0;
    tp->tx_ring[i].status = 0x00000000;
    tp->tx_ring[i].buffer2 = virt2phys(&tp->tx_ring[i + 1]);
  }
  tp->tx_ring[i - 1].buffer2 = virt2phys(&tp->tx_ring[0]);
}

static int tulip_transmit(struct dev *dev, struct pbuf *p) {
  struct tulip_private *tp = (struct tulip_private *) dev->privdata;
  int entry, q_used_cnt;
  unsigned long flag;

  // Wait for free entry in transmit ring
  if (wait_for_object(&tp->tx_sem, TX_TIMEOUT) < 0) {
    kprintf(KERN_WARNING "%s: transmit timeout, drop packet\n", dev->name);
    tp->stats.tx_dropped++;
    return -ETIMEOUT;
  }

  // Make sure the packet buffer is not fragmented
  p = pbuf_linearize(PBUF_RAW, p);

  //kprintf(KERN_DEBUG "%s: Transmit packet, %d bytes\n", dev->name, p->len);

  // Caution: the write order is important here, set the field with the ownership bits last

  // Calculate the next Tx descriptor entry
  entry = tp->cur_tx % TX_RING_SIZE;
  q_used_cnt = tp->cur_tx - tp->dirty_tx;

  tp->tx_pbuf[entry] = p;
  tp->tx_ring[entry].buffer1 = virt2phys(p->payload);

  if (q_used_cnt < TX_QUEUE_LEN / 2) {
    // Typical path
    flag = 0x60000000; // No interrupt
  } else if (q_used_cnt == TX_QUEUE_LEN / 2) {
    flag = 0xe0000000; // Tx-done intr
  } else if (q_used_cnt < TX_QUEUE_LEN) {
    flag = 0x60000000; // No Tx-done intr
  } else {
    // Leave room for set_rx_mode() to fill entries
    tp->tx_full = 1;
    flag = 0xe0000000; // Tx-done intr
  }

  if (entry == TX_RING_SIZE - 1) flag = 0xe0000000 | DESC_RING_WRAP;

  tp->tx_ring[entry].length = p->len | flag;
  tp->tx_ring[entry].status = DescOwned;
  tp->cur_tx++;
  
  tp->trans_start = get_ticks();

  // Trigger an immediate transmit demand
  writel(0, tp->iobase + CSR1);

  return 0;
}

static int tulip_rx(struct dev *dev) {
  struct tulip_private *tp = (struct tulip_private *) dev->privdata;
  int entry = tp->cur_rx % RX_RING_SIZE;
  int rx_work_limit = tp->dirty_rx + RX_RING_SIZE - tp->cur_rx;
  int work_done = 0;

  //kprintf(KERN_DEBUG " In tulip_rx(), entry %d %8.8x.\n", entry, tp->rx_ring[entry].status);

  // If we own the next entry, it is a new packet. Send it up
  while (!(tp->rx_ring[entry].status & DescOwned)) {
    long status = tp->rx_ring[entry].status;

    //kprintf(KERN_DEBUG "%s: In tulip_rx(), entry %d %8.8x.\n", dev->name, entry, status);

    if (--rx_work_limit < 0) break;

    if ((status & 0x38008300) != 0x0300) {
      if ((status & 0x38000300) != 0x0300) {
        // Ingore earlier buffers
        if ((status & 0xffff) != 0x7fff) {
          kprintf(KERN_WARNING "%s: Oversized Ethernet frame spanned multiple buffers, status %8.8x!\n", dev->name, status);
          tp->stats.rx_length_errors++;
        }
      } else if (status & RxDescFatalErr) {
        // There was a fatal error
        kprintf(KERN_DEBUG "%s: Receive error, Rx status %8.8x.\n", dev->name, status);
        tp->stats.rx_errors++; /* end of a packet.*/
        if (status & 0x0890) tp->stats.rx_length_errors++;
        if (status & 0x0004) tp->stats.rx_frame_errors++;
        if (status & 0x0002) tp->stats.rx_crc_errors++;
        if (status & 0x0001) tp->stats.rx_fifo_errors++;
      }
    } else {
      // Omit the four octet CRC from the length
      int pkt_len = ((status >> 16) & 0x7ff) - 4;
      struct pbuf *p;

      // Check if the packet is long enough to accept without copying
      // to a minimally-sized packet buffer
      if (pkt_len < rx_copybreak && (p = pbuf_alloc(PBUF_RAW, pkt_len, PBUF_RW)) != NULL) {
        memcpy(p->payload, tp->rx_pbuf[entry]->payload, pkt_len);
        work_done++;
      } else {  
        // Pass up the packet buffer already on the Rx ring
        p = tp->rx_pbuf[entry];
        tp->rx_pbuf[entry] = NULL;
      }

      //kprintf(KERN_DEBUG "%s: Received packet, %d bytes\n", dev->name, pkt_len);

      // Resize packet buffer
      pbuf_realloc(p, pkt_len);

      // Send packet to upper layer
      if (dev_receive(tp->devno, p) < 0) pbuf_free(p);

      tp->last_rx = get_ticks();
      tp->stats.rx_packets++;
      tp->stats.rx_bytes += pkt_len;
    }
    entry = (++tp->cur_rx) % RX_RING_SIZE;
  }

  // Refill the Rx ring buffers
  for (; tp->cur_rx - tp->dirty_rx > 0; tp->dirty_rx++) {
    entry = tp->dirty_rx % RX_RING_SIZE;
    if (tp->rx_pbuf[entry] == NULL) {
      struct pbuf *p;
      p = tp->rx_pbuf[entry] = pbuf_alloc(PBUF_RAW, PKT_BUF_SZ, PBUF_RW);
      if (p == NULL) {
        if (tp->cur_rx - tp->dirty_rx == RX_RING_SIZE) kprintf(KERN_ERR "%s: No kernel memory to allocate receive buffers.\n", dev->name);
        break;
      }
      tp->rx_ring[entry].buffer1 = virt2phys(p->payload);
      work_done++;
    }
    tp->rx_ring[entry].status = DescOwned;
  }

  return work_done;
}

static void empty_rings(struct dev *dev) {
  struct tulip_private *tp = (struct tulip_private *) dev->privdata;
  int i;

  // Free all the buffers in the Rx queue
  for (i = 0; i < RX_RING_SIZE; i++) {
    struct pbuf *p = tp->rx_pbuf[i];
    tp->rx_pbuf[i] = 0;
    tp->rx_ring[i].status = 0;    // Not owned by Tulip chip
    tp->rx_ring[i].length = 0;
    tp->rx_ring[i].buffer1 = 0xBADF00D0; // An invalid address
    if (p) pbuf_free(p);
  }

  for (i = 0; i < TX_RING_SIZE; i++) {
    if (tp->tx_pbuf[i]) pbuf_free(tp->tx_pbuf[i]);
    tp->tx_pbuf[i] = 0;
  }
}

static int tulip_close(struct dev *dev) {
  struct tulip_private *tp = (struct tulip_private *) dev->privdata;
  long ioaddr = tp->iobase;

  //netif_stop_tx_queue(dev);

  kprintf(KERN_DEBUG "%s: Shutting down ethercard, status was %2.2x.\n", dev->name, readl(ioaddr + CSR5));

  // Disable interrupts by clearing the interrupt mask
  writel(0x00000000, ioaddr + CSR7);

  // Stop the Tx and Rx processes
  writel(readl(ioaddr + CSR6) & ~TxOn & ~RxOn, ioaddr + CSR6);

  // 21040 -- Leave the card in 10baseT state
  if (tp->chip_id == DC21040) writel(0x00000004, ioaddr + CSR13);

  if (readl(ioaddr + CSR6) != 0xffffffff) tp->stats.rx_missed_errors += readl(ioaddr + CSR8) & 0xffff;

  del_timer(&tp->timer);
  disable_irq(tp->irq);
  tp->if_port = tp->saved_if_port;

  empty_rings(dev);

  // Leave the driver in snooze, not sleep, mode
  if (tp->flags & HAS_PWRDWN) pci_write_config_dword(tp->pci_dev, 0x40, 0x40000000);

  return 0;
}

static struct stats_nic *tulip_get_stats(struct dev *dev) {
  struct tulip_private *tp = (struct tulip_private *) dev->privdata;
  long ioaddr = tp->iobase;
  int csr8 = readl(ioaddr + CSR8);

  if (csr8 != 0xffffffff) tp->stats.rx_missed_errors += (unsigned short) csr8;

  return &tp->stats;
}

//
// Set or clear the multicast filter for this adaptor.
// Note that we only use exclusion around actually queueing the
// new frame, not around filling tp->setup_frame.  This is non-deterministic
// when re-entered but still correct
//

// The little-endian AUTODIN32 ethernet CRC calculation.
// N.B. Do not use for bulk data, use a table-based routine instead.

static unsigned const ethernet_polynomial_le = 0xedb88320U;

static unsigned long ether_crc_le(int length, unsigned char *data) {
  unsigned long crc = 0xffffffff;
  while(--length >= 0) {
    unsigned char current_octet = *data++;
    int bit;
    for (bit = 8; --bit >= 0; current_octet >>= 1) {
      if ((crc ^ current_octet) & 1) {
        crc >>= 1;
        crc ^= ethernet_polynomial_le;
      } else {
        crc >>= 1;
      }
    }
  }

  return crc;
}

static int tulip_set_rx_mode(struct dev *dev) {
  struct tulip_private *tp = (struct tulip_private *) dev->privdata;
  long ioaddr = tp->iobase;
  int csr6 = readl(ioaddr + CSR6) & ~0x00D5;

  tp->csr6 &= ~0x00D5;
  if (!dev->netif) {
    // Network interface not attached yet -- accept all multicasts
    tp->csr6 |= AcceptAllMulticast;
    csr6 |= AcceptAllMulticast;
  } else if (dev->netif->flags & NETIF_PROMISC) {     
    // Set promiscuous
    tp->csr6 |= AcceptAllMulticast | AcceptAllPhys;
    csr6 |= AcceptAllMulticast | AcceptAllPhys;
    // Unconditionally log net taps
    kprintf(KERN_INFO "%s: Promiscuous mode enabled.\n", dev->name);
  } else if (dev->netif->mccount > 1000 || (dev->netif->flags & NETIF_ALLMULTI)) {
    // Too many to filter well -- accept all multicasts
    tp->csr6 |= AcceptAllMulticast;
    csr6 |= AcceptAllMulticast;
  } else  if (tp->flags & MC_HASH_ONLY) {
    // Some work-alikes have only a 64-entry hash filter table
    struct mclist *mclist;
    int i;
    if (dev->netif->mccount > multicast_filter_limit) {
      tp->csr6 |= AcceptAllMulticast;
      csr6 |= AcceptAllMulticast;
    } else {
      unsigned long mc_filter[2] = {0, 0};     // Multicast hash filter
      int filterbit;
      for (i = 0, mclist = dev->netif->mclist; mclist && i < dev->netif->mccount; i++, mclist = mclist->next) {
        if (tp->flags & COMET_MAC_ADDR) {
          filterbit = ether_crc_le(ETHER_ADDR_LEN, mclist->hwaddr.addr);
        } else {
          filterbit = ether_crc(ETHER_ADDR_LEN, mclist->hwaddr.addr) >> 26;
        }
        filterbit &= 0x3f;
        set_bit(mc_filter, filterbit);
      }

      if (mc_filter[0] == tp->mc_filter[0] && mc_filter[1] == tp->mc_filter[1]) {
        // No change
      } else if (tp->flags & IS_ASIX) {
        writel(2, ioaddr + CSR13);
        writel(mc_filter[0], ioaddr + CSR14);
        writel(3, ioaddr + CSR13);
        writel(mc_filter[1], ioaddr + CSR14);
      } else if (tp->flags & COMET_MAC_ADDR) {
        writel(mc_filter[0], ioaddr + 0xAC);
        writel(mc_filter[1], ioaddr + 0xB0);
      }
      tp->mc_filter[0] = mc_filter[0];
      tp->mc_filter[1] = mc_filter[1];
    }
  } else {
    unsigned short *eaddrs, *setup_frm = tp->setup_frame;
    struct mclist *mclist;
    unsigned long tx_flags = 0x08000000 | 192;
    int i;

    // Note that only the low-address shortword of setup_frame is valid!
    // The values are doubled for big-endian architectures
    if (dev->netif->mccount > 14) {
      // Must use a multicast hash table
      unsigned short hash_table[32];
      tx_flags = 0x08400000 | 192;    // Use hash filter
      memset(hash_table, 0, sizeof(hash_table));
      set_bit(hash_table, 255);      // Broadcast entry

      for (i = 0, mclist = dev->netif->mclist; mclist && i < dev->netif->mccount; i++, mclist = mclist->next) {
        set_bit(hash_table, ether_crc_le(ETHER_ADDR_LEN, mclist->hwaddr.addr) & 0x1ff);
      }

      for (i = 0; i < 32; i++) {
        *setup_frm++ = hash_table[i];
        *setup_frm++ = hash_table[i];
      }

      setup_frm = &tp->setup_frame[13 * 6];
    } else {
      // We have <= 14 addresses so we can use the wonderful
      // 16 address perfect filtering of the Tulip

      for (i = 0, mclist = dev->netif->mclist; i < dev->netif->mccount; i++, mclist = mclist->next) {
        eaddrs = (unsigned short *) mclist->hwaddr.addr;
        *setup_frm++ = *eaddrs; *setup_frm++ = *eaddrs++;
        *setup_frm++ = *eaddrs; *setup_frm++ = *eaddrs++;
        *setup_frm++ = *eaddrs; *setup_frm++ = *eaddrs++;
      }

      // Fill the unused entries with the broadcast address
      memset(setup_frm, 0xff, (15 - i) * 12);
      setup_frm = &tp->setup_frame[15 * 6];
    }

    // Fill the final entry with our physical address
    eaddrs = (unsigned short *) tp->hwaddr.addr;
    *setup_frm++ = eaddrs[0]; *setup_frm++ = eaddrs[0];
    *setup_frm++ = eaddrs[1]; *setup_frm++ = eaddrs[1];
    *setup_frm++ = eaddrs[2]; *setup_frm++ = eaddrs[2];
    
    // Now add this frame to the Tx list
    if (tp->cur_tx - tp->dirty_tx > TX_RING_SIZE - 2) {
      // Same setup recently queued, we need not add it
    } else {
      unsigned int entry;

      entry = tp->cur_tx++ % TX_RING_SIZE;

      if (entry != 0) {
        // Avoid a chip errata by prefixing a dummy entry
        tp->tx_pbuf[entry] = 0;
        tp->tx_ring[entry].length = (entry == TX_RING_SIZE-1) ? DESC_RING_WRAP : 0;
        tp->tx_ring[entry].buffer1 = 0;
        tp->tx_ring[entry].status = DescOwned;
        entry = tp->cur_tx++ % TX_RING_SIZE;
      }

      tp->tx_pbuf[entry] = 0;

      // Put the setup frame on the Tx list
      if (entry == TX_RING_SIZE - 1) tx_flags |= DESC_RING_WRAP;   // Wrap ring
      tp->tx_ring[entry].length = tx_flags;
      tp->tx_ring[entry].buffer1 = virt2phys(tp->setup_frame);
      tp->tx_ring[entry].status = DescOwned;
      if (tp->cur_tx - tp->dirty_tx >= TX_RING_SIZE - 2)  tp->tx_full = 1;
      
      // Trigger an immediate transmit demand
      writel(0, ioaddr + CSR1);
    }
  }

  writel(csr6, ioaddr + CSR6);
  return 0;
}

// The interrupt handler does all of the Rx thread work and cleans up
// after the Tx thread

static void tulip_dpc(void *arg) {
  struct dev *dev = (struct dev *) arg;
  struct tulip_private *tp = (struct tulip_private *) dev->privdata;
  long ioaddr = tp->iobase;
  int csr5, work_budget = max_interrupt_work;

  while (1) {
    csr5 = readl(ioaddr + CSR5);
    if ((csr5 & (NormalIntr | AbnormalIntr)) == 0) break;

    //kprintf(KERN_DEBUG "%s: interrupt  csr5=%#8.8x new csr5=%#8.8x.\n", dev->name, csr5, readl(tp->iobase + CSR5));

    // Acknowledge all of the current interrupt sources ASAP
    writel(csr5 & 0x0001ffff, ioaddr + CSR5);

    if (csr5 & (RxIntr | RxNoBuf)) work_budget -= tulip_rx(dev);

    if (csr5 & (TxNoBuf | TxDied | TxIntr)) {
      unsigned int dirty_tx;

      for (dirty_tx = tp->dirty_tx; tp->cur_tx - dirty_tx > 0; dirty_tx++) {
        int entry = dirty_tx % TX_RING_SIZE;
        int status = tp->tx_ring[entry].status;

        if (status < 0) break;      // It still has not been Txed
        
        // Check for Rx filter setup frames
        if (tp->tx_pbuf[entry] == NULL) continue;

        if (status & 0x8000) {
          // There was an major error, log it
          kprintf(KERN_DEBUG "%s: Transmit error, Tx status %8.8x.\n", dev->name, status);
          tp->stats.tx_errors++;
          if (status & 0x4104) tp->stats.tx_aborted_errors++;
          if (status & 0x0C00) tp->stats.tx_carrier_errors++;
          if (status & 0x0200) tp->stats.tx_window_errors++;
          if (status & 0x0002) tp->stats.tx_fifo_errors++;
          if ((status & 0x0080) && tp->full_duplex == 0) tp->stats.tx_heartbeat_errors++;
          if (status & 0x0100) tp->stats.collisions++;
        } else {
          //kprintf(KERN_DEBUG "%s: Transmit complete, status %8.8x.\n", dev->name, status);
          //if (status & 0x0001) tp->stats.tx_deferred++;
          tp->stats.tx_bytes += tp->tx_pbuf[entry]->len;
          tp->stats.collisions += (status >> 3) & 15;
          tp->stats.tx_packets++;
        }

        // Free the original packet buffer
        pbuf_free(tp->tx_pbuf[entry]);
        tp->tx_pbuf[entry] = 0;

        release_sem(&tp->tx_sem, 1);
      }

      if (tp->tx_full && tp->cur_tx - dirty_tx  < TX_QUEUE_LEN - 4) {
        // The ring is no longer full, clear tbusy
        tp->tx_full = 0;
      }

      tp->dirty_tx = dirty_tx;
    }

    if (tp->rx_dead) {
      tulip_rx(dev);
      if (tp->cur_rx - tp->dirty_rx < RX_RING_SIZE - 3) {
        kprintf(KERN_ERR "%s: Restarted Rx at %d / %d.\n", dev->name, tp->cur_rx, tp->dirty_rx);
        writel(0, ioaddr + CSR2);   // Rx poll demand
        tp->rx_dead = 0;
      }
    }

    // Log errors
    if (csr5 & AbnormalIntr) {  
      // Abnormal error summary bit
      if (csr5 == 0xffffffff) break;
      if (csr5 & TxJabber) tp->stats.tx_errors++;
      if (csr5 & PCIBusError) kprintf(KERN_ERR "%s: PCI Fatal Bus Error, %8.8x.\n", dev->name, csr5);
      if (csr5 & TxFIFOUnderflow) {
        if ((tp->csr6 & 0xC000) != 0xC000) {
          tp->csr6 += 0x4000; // Bump up the Tx threshold
        } else {
          tp->csr6 |= 0x00200000;  // Store-n-forward
        }
        kprintf(KERN_WARNING "%s: Tx threshold increased, new CSR6 %x.\n", dev->name, tp->csr6);
      }

      if (csr5 & TxDied) {
        // This is normal when changing Tx modes
        kprintf(KERN_WARNING "%s: The transmitter stopped. CSR5 is %x, CSR6 %x, new CSR6 %x.\n",
               dev->name, csr5, readl(ioaddr + CSR6), tp->csr6);
      }

      if (csr5 & (TxDied | TxFIFOUnderflow | PCIBusError)) {
        // Restart the transmit process
        writel(tp->csr6 | RxOn, ioaddr + CSR6);
        writel(tp->csr6 | RxOn | TxOn, ioaddr + CSR6);
      }

      if (csr5 & (RxStopped | RxNoBuf)) {
        // Missed a Rx frame or mode change
        tp->stats.rx_missed_errors += readl(ioaddr + CSR8) & 0xffff;
        if (tp->flags & COMET_MAC_ADDR) {
          writel(tp->mc_filter[0], ioaddr + 0xAC);
          writel(tp->mc_filter[1], ioaddr + 0xB0);
        }
        tulip_rx(dev);
        if (csr5 & RxNoBuf) tp->rx_dead = 1;
        writel(tp->csr6 | RxOn | TxOn, ioaddr + CSR6);
      }

      if (csr5 & TimerInt) {
        kprintf(KERN_ERR "%s: Re-enabling interrupts, %8.8x.\n", dev->name, csr5);
        writel(tulip_tbl[tp->chip_id].valid_intrs, ioaddr + CSR7);
      }

      if (csr5 & (TPLnkPass | TPLnkFail | 0x08000000)) {
        if (tp->link_change) (tp->link_change)(dev, csr5);
      }

      // Clear all error sources, included undocumented ones!
      writel(0x0800f7ba, ioaddr + CSR5);
    }

    if (--work_budget < 0) {
      kprintf(KERN_WARNING "%s: Too much work during an interrupt, csr5=0x%8.8x.\n", dev->name, csr5);
      
      // Acknowledge all interrupt sources
      writel(0x8001ffff, ioaddr + CSR5);
      if (tp->flags & HAS_INTR_MITIGATION) {
        // Josip Loncaric at ICASE did extensive experimentation
        // to develop a good interrupt mitigation setting
        writel(0x8b240000, ioaddr + CSR11);
      } else {
        // Mask all interrupting sources, set timer to re-enable
        writel(((~csr5) & 0x0001ebef) | AbnormalIntr | TimerInt, ioaddr + CSR7);
        writel(0x0012, ioaddr + CSR11);
      }
      break;
    }
  }

  //kprintf(KERN_DEBUG "%s: exiting interrupt, csr5=%#4.4x.\n", dev->name, readl(ioaddr + CSR5));
  eoi(tp->irq);
}

static int tulip_handler(struct context *ctxt, void *arg) {
  struct dev *dev = (struct dev *) arg;
  struct tulip_private *tp = (struct tulip_private *) dev->privdata;

  // Queue DPC to service interrupt
  //kprintf("%s: interrupt\n", dev->name);
  queue_irq_dpc(&tp->dpc, tulip_dpc, dev);

  return 0;
}

static int tulip_ioctl(struct dev *dev, int cmd, void *args, size_t size) {
  return -ENOSYS;
}

static int tulip_attach(struct dev *dev, struct eth_addr *hwaddr) {
  struct tulip_private *tp = dev->privdata;
  *hwaddr = tp->hwaddr;

  return 0;
}

static int tulip_detach(struct dev *dev) {
  return 0;
}

struct driver tulip_driver = {
  "tulip",
  DEV_TYPE_PACKET,
  tulip_ioctl,
  NULL,
  NULL,
  tulip_attach,
  tulip_detach,
  tulip_transmit,
  tulip_set_rx_mode
};

static struct dev *tulip_probe(struct unit *pdev, struct board *board, int ioaddr, char *opts) {
  struct dev *dev;
  struct tulip_private *tp;

  // See note below on the multiport cards.
  static unsigned char last_phys_addr[6] = {0x00, 'S', 'a', 'n', 'o', 's'};
  static int last_irq = 0;
  static int multiport_cnt = 0;   // For four-port boards w/one EEPROM
  unsigned char chip_rev;
  int i;
  unsigned short sum;
  unsigned char ee_data[EEPROM_SIZE];

  int chip_idx = board->flags & 0xff;
  int irq = get_unit_irq(pdev);

  // Bring the 21041/21143 out of sleep mode. Caution: Snooze mode does not work with some boards!
  if (tulip_tbl[chip_idx].flags & HAS_PWRDWN) pci_write_config_dword(pdev, 0x40, 0x00000000);

  if (readl(ioaddr + CSR5) == 0xffffffff) {
    kprintf(KERN_ERR "The Tulip chip at %#lx is not functioning.\n", ioaddr);
    return 0;
  }

  // Allocate private data structures (quadword aligned)
  tp = kmalloc(sizeof(*tp));

  // Check for the very unlikely case of no memory
  if (tp == NULL) return NULL;
  memset(tp, 0, sizeof(*tp));

  // Create new device
  tp->devno = dev_make("eth#", &tulip_driver, pdev, tp);
  if (tp->devno == NODEV) return NULL;
  dev = device(tp->devno);

  tp->next_module = root_tulip_dev;
  root_tulip_dev = dev;

  chip_rev = pci_read_config_byte(pdev, 0x08 /*PCI_REVISION_ID*/);

  kprintf(KERN_INFO "%s: %s rev %d at %#3lx,", dev->name, board->productname, chip_rev, ioaddr);

  // Stop the Tx and Rx processes
  writel(readl(ioaddr + CSR6) & ~TxOn & ~RxOn, ioaddr + CSR6);

  // Clear the missed-packet counter
  readl(ioaddr + CSR8);

  if (chip_idx == DC21041 && readl(ioaddr + CSR9) & 0x8000) {
    kprintf(" 21040 compatible mode,");
    chip_idx = DC21040;
  }

  // The SROM/EEPROM interface varies dramatically
  sum = 0;
  if (chip_idx == DC21040) {
    writel(0, ioaddr + CSR9);   // Reset the pointer with a dummy write
    for (i = 0; i < 6; i++) {
      int value, boguscnt = 100000;
      do {
        value = readl(ioaddr + CSR9);
      } while (value < 0  && --boguscnt > 0);
      tp->hwaddr.addr[i] = value;
      sum += value & 0xff;
    }
  } else if (chip_idx == LC82C168) {
    for (i = 0; i < 3; i++) {
      int value, boguscnt = 100000;
      writel(0x600 | i, ioaddr + 0x98);
      do {
        value = readl(ioaddr + CSR9);
      } while (value < 0  && --boguscnt > 0);

      ((unsigned short *) tp->hwaddr.addr)[i] = value;
      sum += value & 0xffff;
    }
  } else if (chip_idx == COMET) {
    // No need to read the EEPROM
    *(unsigned long *) (tp->hwaddr.addr) = readl(ioaddr + 0xA4);
    *(unsigned short *) (tp->hwaddr.addr + 4) = readl(ioaddr + 0xA8);
    for (i = 0; i < 6; i++) sum += tp->hwaddr.addr[i];
  } else {
    // A serial EEPROM interface, we read now and sort it out later
    int sa_offset = 0;
    int ee_addr_size = read_eeprom(ioaddr, 0xff, 8) & 0x40000 ? 8 : 6;
    int eeprom_word_cnt = 1 << ee_addr_size;

    for (i = 0; i < eeprom_word_cnt; i++) {
      ((unsigned short *) ee_data)[i] = read_eeprom(ioaddr, i, ee_addr_size);
    }

    // DEC now has a specification (see Notes) but early board makers
    // just put the address in the first EEPROM locations
    // This does  memcmp(eedata, eedata + 16, 8)
    for (i = 0; i < 8; i ++) {
      if (ee_data[i] != ee_data[16 + i]) sa_offset = 20;
    }

    if (chip_idx == CONEXANT) {
      // Check that the tuple type and length is correct
      if (ee_data[0x198] == 0x04 && ee_data[0x199] == 6) sa_offset = 0x19A;
    } else if (ee_data[0] == 0xff && ee_data[1] == 0xff && ee_data[2] == 0) {
      sa_offset = 2;    // Grrr, damn Matrox boards
      multiport_cnt = 4;
    }

    for (i = 0; i < 6; i ++) {
      tp->hwaddr.addr[i] = ee_data[i + sa_offset];
      sum += ee_data[i + sa_offset];
    }
  }

  // Lite-On boards have the address byte-swapped
  if ((tp->hwaddr.addr[0] == 0xA0 || tp->hwaddr.addr[0] == 0xC0) &&  tp->hwaddr.addr[1] == 0x00) {
    for (i = 0; i < 6; i += 2) {
      unsigned char tmp = tp->hwaddr.addr[i];
      tp->hwaddr.addr[i] = tp->hwaddr.addr[i + 1];
      tp->hwaddr.addr[i + 1] = tmp;
    }
  }

  // On the Zynx 315 Etherarray and other multiport boards only the
  // first Tulip has an EEPROM.
  // The addresses of the subsequent ports are derived from the first.
  // Many PCI BIOSes also incorrectly report the IRQ line, so we correct
  // that here as well

  if (sum == 0 || sum == 6 * 0xff) {
    kprintf(" EEPROM not present,");
    for (i = 0; i < 5; i++) tp->hwaddr.addr[i] = last_phys_addr[i];
    tp->hwaddr.addr[i] = last_phys_addr[i] + 1;
    // Patch up x86 BIOS bug
    if (last_irq) irq = last_irq;
  }

  for (i = 0; i < 6; i++) kprintf("%c%2.2X", i ? ':' : ' ', last_phys_addr[i] = tp->hwaddr.addr[i]);
  kprintf(", IRQ %d.\n", irq);
  last_irq = irq;

  // We do a request_region() to register /proc/ioports info
  //request_region(ioaddr, pci_id_tbl[chip_idx].io_size, dev->name);

  tp->iobase = ioaddr;
  tp->irq = irq;

  tp->pci_dev = pdev;
  tp->chip_id = chip_idx;
  tp->revision = chip_rev;
  tp->flags = tulip_tbl[chip_idx].flags | (board->flags & 0xffffff00);
  tp->csr0 = csr0;

  // BugFixes: The 21143-TD hangs with PCI Write-and-Invalidate cycles.
  // And the ASIX must have a burst limit or horrible things happen
  if (chip_idx == DC21143 && chip_rev == 65) {
    tp->csr0 &= ~0x01000000;
  } else if (tp->flags & IS_ASIX) {
    tp->csr0 |= 0x2000;
  }

  // We support a zillion ways to set the media type
#ifdef TULIP_FULL_DUPLEX
  tp->full_duplex = 1;
  tp->full_duplex_lock = 1;
#endif
#ifdef TULIP_DEFAULT_MEDIA
  tp->default_port = TULIP_DEFAULT_MEDIA;
#endif
#ifdef TULIP_NO_MEDIA_SWITCH
  tp->medialock = 1;
#endif

  // Set options
  if (opts) {
    tp->default_port = get_num_option(opts, "defaultport", tp->default_port);
    tp->full_duplex = get_num_option(opts, "fullduplex", tp->full_duplex);
    //tp->mtu = get_num_option(opts, "mtu", tp->mtu);
  }

  // ???
  //if (dev->mem_start) tp->default_port = dev->mem_start & 0x1f;

  if (tp->default_port) {
    kprintf(KERN_INFO "%s: Transceiver selection forced to %s.\n", dev->name, medianame[tp->default_port & MEDIA_MASK]);
    tp->medialock = 1;
    if (media_cap[tp->default_port] & MediaAlwaysFD) tp->full_duplex = 1;
  }

  if (tp->full_duplex) tp->full_duplex_lock = 1;

  if (media_cap[tp->default_port] & MediaIsMII) {
    unsigned short media2advert[] = {0x20, 0x40, 0x03e0, 0x60, 0x80, 0x100, 0x200};
    tp->mii_advertise = media2advert[tp->default_port - 9];
    tp->mii_advertise |= (tp->flags & HAS_8023X); // Matching bits!
  }

  if (tp->flags & HAS_MEDIA_TABLE) {
    memcpy(tp->eeprom, ee_data, sizeof(tp->eeprom));
    parse_eeprom(dev);
  }

  if (tp->flags & HAS_NWAY) {
    tp->link_change = nway_lnk_change;
  } else if (tp->flags & HAS_PNICNWAY) {
    tp->link_change = pnic_lnk_change;
  }
  start_link(dev);

  if (chip_idx == COMET) {
    // Set the Comet LED configuration
    writel(0xf0000000, ioaddr + CSR9);
  }

  return dev;
}

static int tulip_open(struct dev *dev) {
  struct tulip_private *tp = (struct tulip_private *) dev->privdata;
  long ioaddr = tp->iobase;
  int next_tick = 3*HZ;

  // Wake the chip from sleep/snooze mode
  if (tp->flags & HAS_PWRDWN) pci_write_config_dword(tp->pci_dev, 0x40, 0);

  // On some chip revs we must set the MII/SYM port before the reset!?
  if (tp->mii_cnt || (tp->mtable && tp->mtable->has_mii)) writel(0x00040000, ioaddr + CSR6);

  // Reset the chip, holding bit 0 set at least 50 PCI cycles
  writel(0x00000001, ioaddr + CSR0);

  // This would be done after interrupts are initialized, but we do not want
  // to frob the transceiver only to fail later
  init_dpc(&tp->dpc);
  register_interrupt(&tp->intr, IRQ2INTR(tp->irq), tulip_handler, dev);
  enable_irq(tp->irq);

  // Deassert reset.
  // Wait the specified 50 PCI cycles after a reset by initializing
  // Tx and Rx queues and the address filter list
  writel(tp->csr0, ioaddr + CSR0);

  //kprintf(KERN_DEBUG "%s: tulip_open() irq %d.\n", dev->name, tp->irq);

  tulip_init_ring(dev);
  init_sem(&tp->tx_sem, TX_RING_SIZE);

  if (tp->chip_id == PNIC2) {
    unsigned long addr_high = (tp->hwaddr.addr[1] << 8) + (tp->hwaddr.addr[0] << 0);
    
    // This address setting does not appear to impact chip operation??
    writel((tp->hwaddr.addr[5] << 8) + tp->hwaddr.addr[4] +
       (tp->hwaddr.addr[3] << 24) + (tp->hwaddr.addr[2] << 16),
       ioaddr + 0xB0);
    writel(addr_high + (addr_high << 16), ioaddr + 0xB8);
  }

  if (tp->flags & MC_HASH_ONLY) {
    unsigned long addr_low = *(unsigned long *) tp->hwaddr.addr;
    unsigned long addr_high = *(unsigned short *) (tp->hwaddr.addr + 4);

    if (tp->flags & IS_ASIX) {
      writel(0, ioaddr + CSR13);
      writel(addr_low,  ioaddr + CSR14);
      writel(1, ioaddr + CSR13);
      writel(addr_high, ioaddr + CSR14);
    } else if (tp->flags & COMET_MAC_ADDR) {
      writel(addr_low,  ioaddr + 0xA4);
      writel(addr_high, ioaddr + 0xA8);
      writel(0, ioaddr + 0xAC);
      writel(0, ioaddr + 0xB0);
    }
  }

  writel(virt2phys(tp->rx_ring), ioaddr + CSR3);
  writel(virt2phys(tp->tx_ring), ioaddr + CSR4);

  if (!tp->full_duplex_lock) tp->full_duplex = 0;
  init_media(dev);
  if (media_cap[tp->if_port] & MediaIsMII) check_duplex(dev);
  tulip_set_rx_mode(dev);

  // Start the Tx to process setup frame
  writel(tp->csr6, ioaddr + CSR6);
  writel(tp->csr6 | TxOn, ioaddr + CSR6);

  // Enable interrupts by setting the interrupt mask
  writel(tulip_tbl[tp->chip_id].valid_intrs, ioaddr + CSR5);
  writel(tulip_tbl[tp->chip_id].valid_intrs, ioaddr + CSR7);
  writel(tp->csr6 | TxOn | RxOn, ioaddr + CSR6);
  writel(0, ioaddr + CSR2);   // Rx poll demand

  // Set the timer to switch to check for link beat and perhaps switch
  // to an alternate media type
  init_timer(&tp->timer, tulip_tbl[tp->chip_id].media_timer, dev);
  mod_timer(&tp->timer, get_ticks() + next_tick);

  return 0;
}

int __declspec(dllexport) install(struct unit *unit, char *opts) {
  struct board *board;
  struct dev *dev;
  int ioaddr;
#ifndef USE_IO_OPS
  struct resource *memres;
#endif

  // Check license
  if (license() != LICENSE_GPL) kprintf(KERN_WARNING "notice: tulip driver is under GPL license\n");

  // Check for PCI device
  if (unit->bus->bustype != BUSTYPE_PCI) return -EINVAL;

  // Determine NIC type
  board = lookup_board(board_tbl, unit);
  if (!board) return -EIO;

  unit->vendorname = board->vendorname;
  unit->productname = board->productname;

  // Determine I/O address
#ifdef USE_IO_OPS
  ioaddr = get_unit_iobase(unit);
#else
  memres = get_unit_resource(unit, RESOURCE_MEM, 0);
  ioaddr = (int) iomap(memres->start, memres->len);
#endif

  // Enable bus mastering
  pci_enable_busmastering(unit);

  // Probe device
  dev = tulip_probe(unit, board, ioaddr, opts);
  if (!dev) return -ENODEV;

  return tulip_open(dev);
}

int __stdcall start(hmodule_t hmod, int reason, void *reserved2) {
  return 1;
}
