//
// opt.h
//
// Network configuration parameters
//
// Copyright (C) 2002 Michael Ringgaard. All rights reserved.
// Portions Copyright (C) 2001, Swedish Institute of Computer Science.
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
#define RAW_TTL                 255              // RAW time to live

//#define TCP_MIN_SEGLEN          512              // Minimum segment allocation size
#define TCP_MIN_SEGLEN          (MTU - 40)       // Minimum segment allocation size

#define TCP_MSS                 (MTU - 40)       // Maximum segment size
#define TCP_WND                 (12 * TCP_MSS)    // TCP window size
#define TCP_MAXRTX              12               // Maximum number of retransmissions
#define TCP_SYNMAXRTX           6                // Maximum number of SYN retransmissions 
#define TCP_MSL                 60000            // The maximum segment lifetime in milliseconds

#define TCP_SND_BUF             (32 * 1024)      // TCP send buffer size

//#define TCP_SND_QUEUELEN        (2 * TCP_SND_BUF / TCP_MSS)
#define TCP_SND_QUEUELEN        (2 * TCP_SND_BUF / TCP_MIN_SEGLEN)

#define MEM_ALIGNMENT           4
#define PBUF_POOL_SIZE          128
#define PBUF_POOL_BUFSIZE       128

#define CHECK_IP_CHECKSUM
#define CHECK_TCP_CHECKSUM
#define CHECK_UDP_CHECKSUM

#endif
