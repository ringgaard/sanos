//
// icmp.h
//
// Internet Control Message Protocol (ICMP)
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

#ifndef ICMP_H
#define ICMP_H

#define ICMP_ER    0       // Echo reply
#define ICMP_DUR   3       // Destination unreachable
#define ICMP_SQ    4       // Source quench
#define ICMP_RD    5       // Redirect
#define ICMP_ECHO  8       // Echo
#define ICMP_TE   11       // Time exceeded
#define ICMP_PP   12       // Parameter problem
#define ICMP_TS   13       // Timestamp
#define ICMP_TSR  14       // Timestamp reply
#define ICMP_IRQ  15       // Information request
#define ICMP_IR   16       // Information reply

#define ICMP_DUR_NET    0  // Net unreachable
#define ICMP_DUR_HOST   1  // Host unreachable
#define ICMP_DUR_PROTO  2  // Protocol unreachable
#define ICMP_DUR_PORT   3  // Port unreachable
#define ICMP_DUR_FRAG   4  // Fragmentation needed and DF set
#define ICMP_DUR_SR     5  // Source route failed

#define ICMP_TE_TTL     0  // Time to live exceeded in transit
#define ICMP_TE_FRAG    1  // Fragment reassembly time exceeded

#define ICMP_HLEN       8

err_t icmp_input(struct pbuf *p, struct netif *inp);

void icmp_dest_unreach(struct pbuf *p, int t);
void icmp_time_exceeded(struct pbuf *p, int t);

#pragma pack(push, 1)

struct icmp_echo_hdr {
  unsigned short _type_code;
  unsigned short chksum;
  unsigned short id;
  unsigned short seqno;
};

struct icmp_dur_hdr {
  unsigned short _type_code;
  unsigned short chksum;
  unsigned long unused;
};

struct icmp_te_hdr {
  unsigned short _type_code;
  unsigned short chksum;
  unsigned long unused;
};

#pragma pack(pop)

#define ICMPH_TYPE(hdr) (NTOHS((hdr)->_type_code) >> 8)
#define ICMPH_CODE(hdr) (NTOHS((hdr)->_type_code) & 0xFF)

#define ICMPH_TYPE_SET(hdr, type) ((hdr)->_type_code = HTONS(ICMPH_CODE(hdr) | ((type) << 8)))
#define ICMPH_CODE_SET(hdr, code) ((hdr)->_type_code = HTONS((code) | (ICMPH_TYPE(hdr) << 8)))

#endif
