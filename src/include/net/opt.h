//
// opt.h
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Network configuration parameters
//

#ifndef OPT_H
#define OPT_H

#define LITTLE_ENDIAN           1234
#define BIG_ENDIAN              4321

#define BYTEORDER               LITTLE_ENDIAN

#define ARP_TABLE_SIZE          32
#define ARP_XMIT_QUEUE_SIZE     4

#define MTU                     1500             // Maximum transfer unit

#define ICMP_TTL                255              // ICMP time to live
#define UDP_TTL                 255              // UDP time to live
#define TCP_TTL                 255              // TCP time to live

//#define TCP_MIN_SEGLEN          512              // Minimum segment allocation size
#define TCP_MIN_SEGLEN          (MTU - 40)       // Minimum segment allocation size

#define TCP_MSS                 (MTU - 40)       // Maximum segment size
#define TCP_WND                 (12 * TCP_MSS)    // TCP window size
#define TCP_MAXRTX              12               // Maximum number of retransmissions
#define TCP_SYNMAXRTX           6                // Maximum number of SYN retransmissions 
#define TCP_MSL                 60000            // The maximum segment lifetime in milliseconds

#define TCP_SND_BUF             (32 * K)         // TCP send buffer size

//#define TCP_SND_QUEUELEN        (2 * TCP_SND_BUF / TCP_MSS)
#define TCP_SND_QUEUELEN        (2 * TCP_SND_BUF / TCP_MIN_SEGLEN)

#define MEM_ALIGNMENT           4
#define PBUF_POOL_SIZE          128
#define PBUF_POOL_BUFSIZE       128

#endif
