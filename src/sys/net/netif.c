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

#include <net/net.h>

struct netif *netif_list = NULL;
struct netif *netif_default = NULL;

static int netif_proc(struct proc_file *pf, void *arg)
{
  struct netif *netif;
  
  for (netif = netif_list; netif != NULL; netif = netif->next) 
  {
    pprintf(pf, "%s: device %s addr %a mask %a gw %a\n", netif->name, device((devno_t) netif->state)->name, &netif->ipaddr, &netif->netmask, &netif->gw);
  }

  return 0;
}

void netif_init()
{
  register_proc_inode("netif", netif_proc, NULL);
}

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
