//
// netif.h
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Network interface
//

#include <net/net.h>

struct netif *netif_list = NULL;
struct netif *netif_default = NULL;

struct netif *netif_add(char *name, struct ip_addr *ipaddr, struct ip_addr *netmask, struct ip_addr *gw)
{
  struct netif *netif;
  
  netif = kmalloc(sizeof(struct netif));
  if (!netif) return NULL;

  strcpy(netif->name, name);
  netif->input = ip_input;
  netif->output = NULL;
  netif->flags = 0;
  netif->mtu = MTU;

  ip_addr_set(&netif->ipaddr, ipaddr);
  ip_addr_set(&netif->netmask, netmask);
  ip_addr_set(&netif->gw, gw);

  netif->next = netif_list;
  netif_list = netif;

  return netif;
}

struct netif *netif_find(char *name)
{
  struct netif *netif;
  
  if (!name) return NULL;

  for (netif = netif_list; netif != NULL; netif = netif->next) 
  {
    if(strcmp(name, netif->name) == 0) return netif;
  }

  return NULL;
}

void netif_set_ipaddr(struct netif *netif, struct ip_addr *ipaddr)
{
  ip_addr_set(&netif->ipaddr, ipaddr);
}

void netif_set_gw(struct netif *netif, struct ip_addr *gw)
{
  ip_addr_set(&netif->gw, gw);
}

void netif_set_netmask(struct netif *netif, struct ip_addr *netmask)
{
  ip_addr_set(&netif->netmask, netmask);
}

void netif_set_default(struct netif *netif)
{
  netif_default = netif;
}
