//
// udp.h
//
// User Datagram Protocol (UDP)
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

#ifndef UDP_H
#define UDP_H

#define UDP_HLEN 8

#pragma pack(push, 1)

struct udp_hdr {
  unsigned short src;          // Source port number 
  unsigned short dest;         // Destination port number
  unsigned short len;          // Length
  unsigned short chksum;       // Checksum
};

#pragma pack(pop)

#define UDP_FLAGS_NOCHKSUM  0x01
#define UDP_FLAGS_BROADCAST 0x02
#define UDP_FLAGS_CONNECTED 0x04

struct udp_pcb {
  struct udp_pcb *next;

  struct ip_addr local_ip, remote_ip;
  unsigned short local_port, remote_port;
  
  int flags;
  
  err_t (*recv)(void *arg, struct udp_pcb *pcb, struct pbuf *p, struct ip_addr *addr, unsigned short port);
  void *recv_arg;  
};

// Socket layer interface to the UDP code

struct udp_pcb *udp_new();
void udp_remove (struct udp_pcb *pcb);
err_t udp_bind(struct udp_pcb *pcb, struct ip_addr *ipaddr, unsigned short port);
err_t udp_connect(struct udp_pcb *pcb, struct ip_addr *ipaddr, unsigned short port);
void udp_recv(struct udp_pcb *pcb, err_t (*recv)(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, unsigned short port), void *recv_arg);
err_t udp_send(struct udp_pcb *pcb, struct pbuf *p, struct ip_addr *dst_ip, unsigned short dst_port, struct netif *netif);

#define udp_flags(pcb) ((pcb)->flags)
#define udp_setflags(pcb, f) ((pcb)->flags = (f))

// Lower layer interface to UDP

err_t udp_input(struct pbuf *p, struct netif *inp);
void udp_init();

#endif
