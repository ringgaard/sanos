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

#define MTU                     1500

#define ICMP_TTL                255
#define UDP_TTL                 255
#define TCP_TTL                 255
#define TCP_MSS                 128   // A *very* conservative default
#define TCP_WND                 2048
#define TCP_MAXRTX              12
#define TCP_SYNMAXRTX           6

#define TCP_SND_BUF             128
#define TCP_SND_QUEUELEN        (2 * TCP_SND_BUF / TCP_MSS)

#define MEM_ALIGNMENT           4
#define PBUF_POOL_SIZE          128
#define PBUF_POOL_BUFSIZE       128
#define PBUF_LINK_HLEN          0

#endif
