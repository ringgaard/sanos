//
// netif.h
//
// Network interface
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

#ifndef NETIF_H
#define NETIF_H

#define NETIF_UP                      0x00000001   // Interface is up
#define NETIF_DHCP                    0x00000002   // Get IP address for interface via DHCP
#define NETIF_LOOPBACK                0x00000004   // Interface is loopback

#define NETIF_MULTICAST               0x00000010   // Supports multicast
#define NETIF_ALLMULTI                0x00000020   // Receive all multicast packets
#define NETIF_PROMISC                 0x00000040   // Receive all packets

#define NETIF_IP_TX_CHECKSUM_OFFLOAD  0x00010000
#define NETIF_IP_RX_CHECKSUM_OFFLOAD  0x00020000
#define NETIF_UDP_RX_CHECKSUM_OFFLOAD 0x00040000
#define NETIF_UDP_TX_CHECKSUM_OFFLOAD 0x00080000
#define NETIF_TCP_RX_CHECKSUM_OFFLOAD 0x00100000
#define NETIF_TCP_TX_CHECKSUM_OFFLOAD 0x00200000

struct mclist {
  struct mclist *next;
  struct ip_addr ipaddr;
  struct eth_addr hwaddr;
};

struct netif {
  struct netif *next;
  struct ip_addr ipaddr;
  struct ip_addr netmask;
  struct ip_addr gw;
  struct ip_addr broadcast;
  struct eth_addr hwaddr;
  int flags;
  char name[NET_NAME_MAX];

  struct mclist *mclist;
  int mccount;

  err_t (*input)(struct pbuf *p, struct netif *inp);
  err_t (*output)(struct netif *netif, struct pbuf *p, struct ip_addr *ipaddr);
  
  void *state;
};

// The list of network interfaces.

extern struct netif *netif_list;
extern struct netif *netif_default;

void netif_init();

struct netif *netif_add(char *name, struct ip_addr *ipaddr, struct ip_addr *netmask, struct ip_addr *gw);
struct netif *netif_find(char *name);

void netif_set_default(struct netif *netif);

void netif_set_ipaddr(struct netif *netif, struct ip_addr *ipaddr);
void netif_set_netmask(struct netif *netif, struct ip_addr *netmast);
void netif_set_gw(struct netif *netif, struct ip_addr *gw);

int netif_ioctl_list(void *data, size_t size);
int netif_ioctl_cfg(void *data, size_t size);

#endif
