//
// arp.c
//
// Address Resolution Protocol (ARP)
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

#define HWTYPE_ETHERNET    1

#define ARP_REQUEST        1
#define ARP_REPLY          2

#define ARP_MAXAGE         120         // 120 * 10 seconds = 20 minutes
#define ARP_TIMER_INTERVAL 10000       // The ARP cache is checked every 10 seconds

#define MAX_XMIT_DELAY     1000        // Maximum delay for packets in millisecs     

#pragma pack(push, 1)

struct arp_hdr {
  struct eth_hdr ethhdr;           // Ethernet header
  unsigned short hwtype;           // Hardware type
  unsigned short proto;            // Protocol type
  unsigned short _hwlen_protolen;  // Protocol address length
  unsigned short opcode;           // Opcode
  struct eth_addr shwaddr;         // Source hardware address
  struct ip_addr sipaddr;          // Source protocol address
  struct eth_addr dhwaddr;         // Target hardware address
  struct ip_addr dipaddr;          // Target protocol address
};

struct ethip_hdr {
  struct eth_hdr eth;
  struct ip_hdr ip;
};

#pragma pack(pop)

#define ARPH_HWLEN(hdr) (NTOHS((hdr)->_hwlen_protolen) >> 8)
#define ARPH_PROTOLEN(hdr) (NTOHS((hdr)->_hwlen_protolen) & 0xFF)

#define ARPH_HWLEN_SET(hdr, len) (hdr)->_hwlen_protolen = HTONS(ARPH_PROTOLEN(hdr) | ((len) << 8))
#define ARPH_PROTOLEN_SET(hdr, len) (hdr)->_hwlen_protolen = HTONS((len) | (ARPH_HWLEN(hdr) << 8))

struct arp_entry {
  struct ip_addr ipaddr;
  struct eth_addr ethaddr;
  int ctime;
};

struct xmit_queue_entry {
  struct netif *netif;
  struct pbuf *p;
  struct ip_addr ipaddr;
  unsigned int expires;
};

static struct arp_entry arp_table[ARP_TABLE_SIZE];
static struct xmit_queue_entry xmit_queue_table[ARP_XMIT_QUEUE_SIZE];

struct timer arp_timer;
int arp_ctime;

static int arp_proc(struct proc_file *pf, void *arg) {
  int i;

  for (i = 0; i < ARP_TABLE_SIZE; ++i) {
    if (!ip_addr_isany(&arp_table[i].ipaddr)) {
      pprintf(pf, "%la %a\n", &arp_table[i].ethaddr, &arp_table[i].ipaddr);
    }
  }

  return 0;
}

static void arp_tmr(void *arg) {
  int i;
  
  arp_ctime++;
  for (i = 0; i < ARP_TABLE_SIZE; ++i) {
    if (!ip_addr_isany(&arp_table[i].ipaddr) && arp_ctime - arp_table[i].ctime >= ARP_MAXAGE) {
      //kprintf("arp: expired entry %d\n", i);
      ip_addr_set(&arp_table[i].ipaddr, IP_ADDR_ANY);
    }
  }
  
  for (i = 0; i < ARP_XMIT_QUEUE_SIZE; i++) {
    struct xmit_queue_entry *entry = xmit_queue_table + i;
    if (entry->p && time_before(entry->expires, ticks)) {
      //kprintf("arp: xmit queue entry %d expired\n", i);
      pbuf_free(entry->p);
      entry->p = NULL;
      stats.link.drop++;
    }
  }

  mod_timer(&arp_timer, ticks + ARP_TIMER_INTERVAL / MSECS_PER_TICK);
}

void arp_init() {
  int i;
  
  for (i = 0; i < ARP_TABLE_SIZE; ++i) ip_addr_set(&arp_table[i].ipaddr, IP_ADDR_ANY);
  memset(xmit_queue_table, 0, sizeof(xmit_queue_table));
  init_timer(&arp_timer, arp_tmr, NULL);
  mod_timer(&arp_timer, ticks + ARP_TIMER_INTERVAL / MSECS_PER_TICK);
  register_proc_inode("arp", arp_proc, NULL);
}

static void add_arp_entry(struct ip_addr *ipaddr, struct eth_addr *ethaddr) {
  int i, j, k;
  int maxtime;
  int err;
  
  //kprintf("arp: add %la -> %a\n", ethaddr, ipaddr);

  // Walk through the ARP mapping table and try to find an entry to
  // update. If none is found, the IP -> MAC address mapping is
  // inserted in the ARP table.
  for (i = 0; i < ARP_TABLE_SIZE; i++) {
    // Only check those entries that are actually in use.
    if (!ip_addr_isany(&arp_table[i].ipaddr)) {
      // Check if the source IP address of the incoming packet matches
      // the IP address in this ARP table entry.
      if (ip_addr_cmp(ipaddr, &arp_table[i].ipaddr)) {
        // An old entry found, update this and return.
        for (k = 0; k < 6; ++k) arp_table[i].ethaddr.addr[k] = ethaddr->addr[k];
        arp_table[i].ctime = arp_ctime;
        return;
      }
    }
  }

  // If we get here, no existing ARP table entry was found, so we create one.
  // First, we try to find an unused entry in the ARP table.
  for (i = 0; i < ARP_TABLE_SIZE; i++) {
    if (ip_addr_isany(&arp_table[i].ipaddr)) break;
  }

  // If no unused entry is found, we try to find the oldest entry and throw it away
  if (i == ARP_TABLE_SIZE) {
    maxtime = 0;
    j = 0;
    for (i = 0; i < ARP_TABLE_SIZE; ++i) {
      if (arp_ctime - arp_table[i].ctime > maxtime) {
        maxtime = arp_ctime - arp_table[i].ctime;
        j = i;
      }
    }
    i = j;
  }

  // Now, i is the ARP table entry which we will fill with the new information.
  ip_addr_set(&arp_table[i].ipaddr, ipaddr);
  for (k = 0; k < 6; k++) arp_table[i].ethaddr.addr[k] = ethaddr->addr[k];
  arp_table[i].ctime = arp_ctime;

  // Check for delayed transmissions
  for (i = 0; i < ARP_XMIT_QUEUE_SIZE; i++) {
    struct xmit_queue_entry *entry = xmit_queue_table + i;

    if (entry->p && ip_addr_cmp(&entry->ipaddr, ipaddr)) {
      struct pbuf *p = entry->p;
      struct eth_hdr *ethhdr = p->payload;

      entry->p = NULL;

      for (i = 0; i < 6; i++) {
        ethhdr->dest.addr[i] = ethaddr->addr[i];
        ethhdr->src.addr[i] = entry->netif->hwaddr.addr[i];
      }
      ethhdr->type = htons(ETHTYPE_IP);

      err = dev_transmit((dev_t) entry->netif->state, p);
      if (err < 0) {
        kprintf(KERN_ERR "arp: error %d in delayed transmit\n", err);
        pbuf_free(p);
      }
    }
  }
}

void arp_ip_input(struct netif *netif, struct pbuf *p) {
  struct ethip_hdr *hdr;
  
  hdr = p->payload;
  
  // Only insert/update an entry if the source IP address of the
  // incoming IP packet comes from a host on the local network.
  if (!ip_addr_maskcmp(&hdr->ip.src, &netif->ipaddr, &netif->netmask)) return;

  add_arp_entry(&hdr->ip.src, &hdr->eth.src);
}

struct pbuf *arp_arp_input(struct netif *netif, struct eth_addr *ethaddr, struct pbuf *p) {
  struct arp_hdr *hdr;
  int i;
  
  if (p->tot_len < sizeof(struct arp_hdr)) {
    pbuf_free(p);
    return NULL;
  }

  hdr = p->payload;
  
  switch(htons(hdr->opcode)) {
    case ARP_REQUEST:
      // ARP request. If it asked for our address, we send out a reply
      if (ip_addr_cmp(&hdr->dipaddr, &netif->ipaddr)) {
        hdr->opcode = htons(ARP_REPLY);

        ip_addr_set(&hdr->dipaddr, &hdr->sipaddr);
        ip_addr_set(&hdr->sipaddr, &netif->ipaddr);

        for (i = 0; i < 6; i++) {
          hdr->dhwaddr.addr[i] = hdr->shwaddr.addr[i];
          hdr->shwaddr.addr[i] = ethaddr->addr[i];
          hdr->ethhdr.dest.addr[i] = hdr->dhwaddr.addr[i];
          hdr->ethhdr.src.addr[i] = ethaddr->addr[i];
        }

        hdr->hwtype = htons(HWTYPE_ETHERNET);
        ARPH_HWLEN_SET(hdr, 6);
      
        hdr->proto = htons(ETHTYPE_IP);
        ARPH_PROTOLEN_SET(hdr, sizeof(struct ip_addr));      
      
        hdr->ethhdr.type = htons(ETHTYPE_ARP);      
        return p;
      }
      break;

    case ARP_REPLY:
      // ARP reply. We insert or update the ARP table.
      if (ip_addr_cmp(&hdr->dipaddr, &netif->ipaddr)) {
        add_arp_entry(&hdr->sipaddr, &hdr->shwaddr);
        dhcp_arp_reply(&hdr->sipaddr);
      }
      break;

    default:
      break;
  }

  pbuf_free(p);
  return NULL;
}

struct eth_addr *arp_lookup(struct ip_addr *ipaddr) {
  int i;
  
  for (i = 0; i < ARP_TABLE_SIZE; ++i) {
    if (ip_addr_cmp(ipaddr, &arp_table[i].ipaddr)) return &arp_table[i].ethaddr;
  }
  return NULL;  
}

struct pbuf *arp_query(struct netif *netif, struct eth_addr *ethaddr, struct ip_addr *ipaddr) {
  struct arp_hdr *hdr;
  struct pbuf *p;
  int i;

  p = pbuf_alloc(PBUF_LINK, sizeof(struct arp_hdr), PBUF_RW);
  if (p == NULL) return NULL;

  hdr = p->payload;
  hdr->opcode = htons(ARP_REQUEST);

  for (i = 0; i < 6; ++i) {
    hdr->dhwaddr.addr[i] = 0x00;
    hdr->shwaddr.addr[i] = ethaddr->addr[i];
  }
  
  ip_addr_set(&hdr->dipaddr, ipaddr);
  ip_addr_set(&hdr->sipaddr, &netif->ipaddr);

  hdr->hwtype = htons(HWTYPE_ETHERNET);
  ARPH_HWLEN_SET(hdr, 6);

  hdr->proto = htons(ETHTYPE_IP);
  ARPH_PROTOLEN_SET(hdr, sizeof(struct ip_addr));

  for (i = 0; i < 6; ++i) {
    hdr->ethhdr.dest.addr[i] = 0xFF;
    hdr->ethhdr.src.addr[i] = ethaddr->addr[i];
  }
  
  hdr->ethhdr.type = htons(ETHTYPE_ARP);      
  return p;
}

int arp_queue(struct netif *netif, struct pbuf *p, struct ip_addr *ipaddr) {
  int i;
  struct xmit_queue_entry *entry = NULL;

  // Find empty entry
  for (i = 0; i < ARP_XMIT_QUEUE_SIZE; i++) {
    if (xmit_queue_table[i].p == NULL) {
      entry = &xmit_queue_table[i];
      break;
    }
  }

  // If no entry entry found, try to find an expired entry
  if (entry == NULL) {
    for (i = 0; i < ARP_XMIT_QUEUE_SIZE; i++) {
      if (time_before(xmit_queue_table[i].expires, ticks)) {
        entry = &xmit_queue_table[i];
        break;
      }
    }
  }

  // If there are no room in the xmit queue, we have to drop the packet
  if (!entry) {
    stats.link.drop++;
    return -ENOMEM;
  }

  // Expire entry if it is not empty
  if (entry->p) {
    pbuf_free(entry->p);
    stats.link.drop++;
  }

  // Fill xmit queue entry
  entry->netif = netif;
  entry->p = p;
  ip_addr_set(&entry->ipaddr, ipaddr);
  entry->expires = ticks + MAX_XMIT_DELAY / MSECS_PER_TICK;
  
  return 0;
}
