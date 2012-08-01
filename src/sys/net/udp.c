//
// udp.c
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

#include <net/net.h>

static struct udp_pcb *udp_pcbs = NULL;

int udp_debug_print(struct udp_hdr *udphdr);

static int udpstat_proc(struct proc_file *pf, void *arg) {
  struct udp_pcb *pcb;

  pprintf(pf, "local port  remote port local ip        remote ip\n");
  pprintf(pf, "----------- ----------- --------------- ---------------\n");

  for (pcb = udp_pcbs; pcb != NULL; pcb = pcb->next) {
    pprintf(pf, "%8d    %8d    %-15a %-15a\n", pcb->local_port, pcb->remote_port, &pcb->local_ip, &pcb->remote_ip);
  }

  return 0;
}

//
// udp_init
//

void udp_init() {
  register_proc_inode("udpstat", udpstat_proc, NULL);
}

//
// udp_new_port
//
// A nastly hack featuring 'goto' statements that allocates a
// new UDP local port.
//

static unsigned short udp_new_port() {
  struct udp_pcb *pcb;
  static unsigned short port = 4096;
  
again:
  if(++port > 0x7FFF) port = 4096;
  
  for (pcb = udp_pcbs; pcb != NULL; pcb = pcb->next) {
    if (pcb->local_port == port) goto again;
  }

  return port;
}

err_t udp_input(struct pbuf *p, struct netif *inp) {
  struct udp_hdr *udphdr;  
  struct udp_pcb *pcb;
  struct ip_hdr *iphdr;
  unsigned short src, dest;

  stats.udp.recv++;
  
  iphdr = p->payload;
  if (pbuf_header(p, -(IPH_HL(iphdr) * 4)) < 0 || p->tot_len < sizeof(struct udp_hdr)) {
    kprintf(KERN_WARNING "udp_input: short packet (%u bytes) discarded\n", p->tot_len);
    stats.udp.lenerr++;
    stats.udp.drop++;
    return -EPROTO;
  }
  udphdr = p->payload;

  //udp_debug_print(udphdr);

#ifdef CHECK_UDP_CHECKSUM
  // Check checksum
  if ((inp->flags & NETIF_UDP_RX_CHECKSUM_OFFLOAD) == 0) {
    if (udphdr->chksum != 0) {
      if (inet_chksum_pseudo(p, &iphdr->src, &iphdr->dest, IP_PROTO_UDP, p->tot_len) != 0) {
        kprintf(KERN_WARNING "udp_input: UDP datagram discarded due to failing checksum\n");

        stats.udp.chkerr++;
        stats.udp.drop++;
        return -ECHKSUM;
      }
    }
  }
#endif

  src = NTOHS(udphdr->src);
  dest = NTOHS(udphdr->dest);

  // Demultiplex packet. First, go for a perfect match
  for (pcb = udp_pcbs; pcb != NULL; pcb = pcb->next) {
    if (pcb->remote_port == src && pcb->local_port == dest &&
        (ip_addr_isany(&pcb->remote_ip) || ip_addr_cmp(&pcb->remote_ip, &iphdr->src)) &&
        (ip_addr_isany(&pcb->local_ip) || ip_addr_cmp(&pcb->local_ip, &iphdr->dest))) {
      break;
    }
  }

  if (pcb == NULL) {
    // No fully matching pcb found, look for an unconnected pcb
    for (pcb = udp_pcbs; pcb != NULL; pcb = pcb->next) {
      if (!(pcb->flags & UDP_FLAGS_CONNECTED) &&
          pcb->local_port == dest &&
          (ip_addr_isany(&pcb->remote_ip) || ip_addr_cmp(&pcb->remote_ip, &iphdr->src)) &&
          (ip_addr_isany(&pcb->local_ip) || ip_addr_cmp(&pcb->local_ip, &iphdr->dest))) {
        break;
      }      
    }
  }

  if (pcb == NULL) {
    // No match was found, send ICMP destination port unreachable unless
    // destination address was broadcast/multicast.
    if (!ip_addr_isbroadcast(&iphdr->dest, &inp->netmask) && !ip_addr_ismulticast(&iphdr->dest)) {
      // Adjust pbuf pointer
      p->payload = iphdr;
      icmp_dest_unreach(p, ICMP_DUR_PORT);
    }

    ++stats.udp.proterr;
    ++stats.udp.drop;

    return -EHOSTUNREACH;
  }

  pbuf_header(p, -UDP_HLEN);
  return pcb->recv(pcb->recv_arg, pcb, p, &iphdr->src, src);
}

err_t udp_send(struct udp_pcb *pcb, struct pbuf *p, struct ip_addr *dst_ip, unsigned short dst_port, struct netif *netif) {
  struct udp_hdr *udphdr;
  struct ip_addr *src_ip;
  err_t err;

  if (!dst_ip) dst_ip = &pcb->remote_ip;
  if (ip_addr_isany(dst_ip)) return -EDESTADDRREQ;

  if (dst_port == 0) dst_port = pcb->remote_port;
  if (dst_port == 0) return -ENOTCONN;

  if (pbuf_header(p, UDP_HLEN) < 0) {
    kprintf(KERN_ERR "udp_send: not enough room for UDP header in pbuf\n");
    stats.udp.err++;
    return -EBUF;
  }

  udphdr = p->payload;
  udphdr->src = htons(pcb->local_port);
  udphdr->dest = htons(dst_port);
  udphdr->chksum = 0x0000;

  if (netif == NULL) {
    if ((netif = ip_route(dst_ip)) == NULL) {
      kprintf(KERN_ERR "udp_send: No route to %a\n", dst_ip);
      stats.udp.rterr++;
      return -EROUTE;
    }
  }

  if (ip_addr_isbroadcast(dst_ip, &netif->netmask) && (pcb->flags & UDP_FLAGS_BROADCAST) == 0) return -EACCES;

  if (ip_addr_isany(&pcb->local_ip)) {
    src_ip = &netif->ipaddr;
  } else {
    src_ip = &pcb->local_ip;
  }

  //kprintf("udp_send: sending datagram of length %d\n", p->tot_len);
  
  udphdr->len = htons((unsigned short) p->tot_len);
  
  // Calculate checksum
  if ((netif->flags & NETIF_UDP_TX_CHECKSUM_OFFLOAD) == 0) {
    if ((pcb->flags & UDP_FLAGS_NOCHKSUM) == 0) {
      udphdr->chksum = inet_chksum_pseudo(p, src_ip, dst_ip, IP_PROTO_UDP, p->tot_len);
      if (udphdr->chksum == 0x0000) udphdr->chksum = 0xFFFF;
    }
  }

  //udp_debug_print(udphdr);
  err = ip_output_if(p, src_ip, dst_ip, UDP_TTL, IP_PROTO_UDP, netif);
  
  stats.udp.xmit++;

  return err;
}

err_t udp_bind(struct udp_pcb *pcb, struct ip_addr *ipaddr, unsigned short port) {
  struct udp_pcb *ipcb;

  if (!ip_addr_isany(ipaddr) && !ip_ownaddr(ipaddr)) return -EADDRNOTAVAIL;
  ip_addr_set(&pcb->local_ip, ipaddr);

  if (port != 0) {
    pcb->local_port = port;
  } else {
    pcb->local_port = udp_new_port();
  }

  // Insert UDP PCB into the list of active UDP PCBs
  for (ipcb = udp_pcbs; ipcb != NULL; ipcb = ipcb->next) {
    // If it is already on the list, just return
    if (pcb == ipcb) return 0;
  }

  // We need to place the PCB on the list
  pcb->next = udp_pcbs;
  udp_pcbs = pcb;

  //kprintf("udp_bind: bound to port %d\n", port);

  return 0;
}

err_t udp_connect(struct udp_pcb *pcb, struct ip_addr *ipaddr, unsigned short port) {
  struct udp_pcb *ipcb;
  
  ip_addr_set(&pcb->remote_ip, ipaddr);
  pcb->remote_port = port;
  pcb->flags |= UDP_FLAGS_CONNECTED;
  if (pcb->local_port == 0) pcb->local_port = udp_new_port();

  // Insert UDP PCB into the list of active UDP PCBs
  for (ipcb = udp_pcbs; ipcb != NULL; ipcb = ipcb->next) {
    // If it is already on the list, just return
    if (pcb == ipcb) return 0;
  }

  // We need to place the PCB on the list
  pcb->next = udp_pcbs;
  udp_pcbs = pcb;
  
  return 0;
}

void udp_recv(struct udp_pcb *pcb, 
              err_t (*recv)(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, unsigned short port), 
              void *recv_arg) {
  pcb->recv = recv;
  pcb->recv_arg = recv_arg;
}

void udp_remove(struct udp_pcb *pcb) {
  struct udp_pcb *pcb2;
  
  if (udp_pcbs == pcb) {
    udp_pcbs = udp_pcbs->next;
  } else {
    for (pcb2 = udp_pcbs; pcb2 != NULL; pcb2 = pcb2->next) {
      if (pcb2->next != NULL && pcb2->next == pcb) {
        pcb2->next = pcb->next;
        break;
      }
    }
  }

  kfree(pcb);
}

struct udp_pcb *udp_new() {
  struct udp_pcb *pcb;

  pcb = (struct udp_pcb *) kmalloc(sizeof(struct udp_pcb));
  if (pcb != NULL) {
    memset(pcb, 0, sizeof(struct udp_pcb));
    return pcb;
  }

  return NULL;
}

int udp_debug_print(struct udp_hdr *udphdr) {
  kprintf("UDP header:\n");
  kprintf("+-------------------------------+\n");
  kprintf("|     %5d     |     %5d     | (src port, dest port)\n", ntohs(udphdr->src), ntohs(udphdr->dest));
  kprintf("+-------------------------------+\n");
  kprintf("|     %5d     |     0x%04x    | (len, chksum)\n", ntohs(udphdr->len), ntohs(udphdr->chksum));
  kprintf("+-------------------------------+\n");

  return 0;
}
