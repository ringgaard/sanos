//
// ether.h
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Ethernet network
//

#ifndef ETHER_H
#define ETHER_H

#pragma pack(push)
#pragma pack(1)

#define ETHER_HLEN 14

#define ETHER_ADDR_LEN 6

struct eth_addr 
{
  unsigned char addr[ETHER_ADDR_LEN];
};
  
struct eth_hdr 
{
  struct eth_addr dest;
  struct eth_addr src;
  unsigned short type;
};

#pragma pack(pop)

__inline int eth_addr_isbroadcast(struct eth_addr *addr)
{
  int n;
  for (n = 0; n < ETHER_ADDR_LEN; n++) if (addr->addr[n] != 0xFF) return 0;
  return 1;
}

krnlapi char *ether2str(struct eth_addr *hwaddr, char *s);
krnlapi struct netif *ether_netif_add(char *name, char *devname, struct ip_addr *ipaddr, struct ip_addr *netmask, struct ip_addr *gw);
krnlapi err_t ether_input(struct netif *netif, struct pbuf *p);
krnlapi err_t ether_output(struct netif *netif, struct pbuf *p, struct ip_addr *ipaddr);

void ether_init();

#endif
