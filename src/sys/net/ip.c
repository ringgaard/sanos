//
// ip.c
//
// Internet Protocol (IP)
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

void ip_debug_print(struct pbuf *p);

//
// ip_init
//
// Initializes the IP layer.
//

void ip_init() {
}

//
// ip_ownaddr
//
// Returns 1 if the IP address is the IP address of one of
// the configured interfaces.
//

int ip_ownaddr(struct ip_addr *addr) {
  struct netif *netif;
  
  for (netif = netif_list; netif != NULL; netif = netif->next) {
    if (!ip_addr_isany(addr) && ip_addr_cmp(addr, &netif->ipaddr)) return 1;
  }

  return 0;
}

//
// ip_route
//
// Finds the appropriate network interface for a given IP address. It
// searches the list of network interfaces linearly. A match is found
// if the masked IP address of the network interface equals the masked
// IP address given to the function. The routine also detects if the
// address is the IP address of one of the interfaces.
//

struct netif *ip_route(struct ip_addr *dest) {
  struct netif *netif;
  
  for (netif = netif_list; netif != NULL; netif = netif->next) {
    if (ip_addr_cmp(dest, &netif->ipaddr)) return netif;
  }

  for (netif = netif_list; netif != NULL; netif = netif->next) {
    if (ip_addr_maskcmp(dest, &netif->ipaddr, &netif->netmask)) {
      //kprintf("ip: route packet to %a to interface %s\n", dest, netif->name);
      return netif;
    }
  }

  if (netif_default) {
    //kprintf("ip: route packet to %a to default interface %s\n", dest, netif_default->name);
    return netif_default;
  }

  return NULL;
}

//
// ip_forward
//
// Forwards an IP packet. It finds an appropriate route for the packet, decrements
// the TTL value of the packet, adjusts the checksum and outputs the packet on the
// appropriate interface.
//

static err_t ip_forward(struct pbuf *p, struct ip_hdr *iphdr, struct netif *inp) {
  struct netif *netif;

  // Don't route broadcasts
  if (ip_addr_isbroadcast(&iphdr->dest, &inp->netmask)) {
    pbuf_free(p);
    return 0;
  }

  // Find route for packet
  if ((netif = ip_route(&iphdr->dest)) == NULL) {
    kprintf("ip_forward: no forwarding route for %a found\n", &iphdr->dest);
    return -EROUTE;
  }

  // Don't forward packets onto the same network interface on which they arrived
  if (netif == inp) {
    //kprintf("ip_forward: not forward packets back on incoming interface. (%a->%a)\n", &iphdr->src, &iphdr->dest);
    return -EROUTE;
  }
  
  // Decrement TTL and send ICMP if ttl == 0
  IPH_TTL_SET(iphdr, IPH_TTL(iphdr) - 1);
  if (IPH_TTL(iphdr) == 0) {
    // Don't send ICMP messages in response to ICMP messages
    if (IPH_PROTO(iphdr) != IP_PROTO_ICMP) icmp_time_exceeded(p, ICMP_TE_TTL);
    pbuf_free(p);
    return 0;
  }
  
  // Incremental update of the IP checksum
  if (IPH_CHKSUM(iphdr) >= htons(0xffff - 0x100)) {
    IPH_CHKSUM_SET(iphdr, IPH_CHKSUM(iphdr) + htons(0x100) + 1);
  } else {
    IPH_CHKSUM_SET(iphdr, (unsigned short) (IPH_CHKSUM(iphdr) + htons(0x100)));
  }
  kprintf("ip_forward: forwarding packet to %a\n", &iphdr->dest);

  stats.ip.fw++;
  stats.ip.xmit++;

  return netif->output(netif, p, &iphdr->dest);
}

//
// ip_input
//
// This function is called by the network interface device driver when an IP packet is
// received. The function does the basic checks of the IP header such as packet size
// being at least larger than the header size etc. If the packet was not destined for
// us, the packet is forwarded (using ip_forward). The IP checksum is always checked.
//
// Finally, the packet is sent to the upper layer protocol input function.
//

err_t ip_input(struct pbuf *p, struct netif *inp) {
  struct ip_hdr *iphdr;
  struct netif *netif;
  int hl;
  int iphdrlen;
  int rc;

  stats.ip.recv++;
  
  //kprintf("receiving IP datagram on %s:\n", inp->name);
  //ip_debug_print(p);

  // Identify the IP header
  iphdr = p->payload;
  if (IPH_V(iphdr) != 4) {
    kprintf("IP packet dropped due to bad version number %d\n", IPH_V(iphdr));
    stats.ip.err++;
    stats.ip.drop++;
    return -EPROTO;
  }
  
  // Check header length
  hl = IPH_HL(iphdr);
  iphdrlen = hl * 4;
  if (iphdrlen > p->len) {
    kprintf("IP packet dropped due to too short packet %d\n", p->len);

    stats.ip.lenerr++;
    stats.ip.drop++;
    return -EPROTO;
  }

#ifdef CHECK_IP_CHECKSUM
  // Verify checksum
  if ((inp->flags & NETIF_IP_RX_CHECKSUM_OFFLOAD) == 0) {
    if (inet_chksum(iphdr, iphdrlen) != 0) {
      kprintf("IP packet dropped due to failing checksum 0x%x\n", inet_chksum(iphdr, iphdrlen));
      stats.ip.chkerr++;
      stats.ip.drop++;
      return -ECHKSUM;
    }
  }
#endif

  // Trim pbuf. This should have been done at the netif layer, but we'll do it anyway just to be sure that its done
  pbuf_realloc(p, ntohs(IPH_LEN(iphdr)));

  // Is this packet for us?
  for (netif = netif_list; netif != NULL; netif = netif->next) {
    if (ip_addr_isany(&netif->ipaddr) ||
        ip_addr_cmp(&iphdr->dest, &netif->ipaddr) ||
       (ip_addr_isbroadcast(&iphdr->dest, &netif->netmask) &&
        ip_addr_maskcmp(&iphdr->dest, &netif->ipaddr, &netif->netmask)) ||
        ip_addr_cmp(&iphdr->dest, IP_ADDR_BROADCAST)) {
          break;
    }
  }

  // If a DHCP packet has arrived on the interface, we pass it up the
  // stack regardless of destination IP address. The reason is that
  // DHCP replies are sent to the IP adress that will be given to this
  // node (as recommended by RFC 1542 section 3.1.1, referred by RFC 2131).
  if (!netif && IPH_PROTO(iphdr) == IP_PROTO_UDP &&
      ((struct udp_hdr *)((char *) iphdr + IPH_HL(iphdr) * 4))->src == DHCP_SERVER_PORT) {
    netif = inp;
  }  
  
  if (netif == NULL) {
    // Packet not for us, route or discard
    if (!ip_addr_isbroadcast(&iphdr->dest, &inp->netmask)) {
      if (ip_forward(p, iphdr, inp) < 0) pbuf_free(p);
    } else {
      pbuf_free(p);
    }
    return 0;
  }

  if ((IPH_OFFSET(iphdr) & htons(IP_OFFMASK | IP_MF)) != 0) {
    kprintf("IP packet dropped since it was fragmented (0x%x).\n", ntohs(IPH_OFFSET(iphdr)));
    stats.ip.opterr++;
    stats.ip.drop++;
    return -EPROTO;
  }
  
  if (iphdrlen > IP_HLEN) {
    kprintf("IP packet dropped since there were IP options.\n");
    stats.ip.opterr++;
    stats.ip.drop++;
    return -EPROTO;
  }

  // Try to see if any raw sockets wants the packet
  rc = raw_input(p, inp);
  if (rc < 0) return rc;
  if (rc > 0) return 0;

  // Send to upper layers
  switch (IPH_PROTO(iphdr)) {
    case IP_PROTO_UDP:
      return udp_input(p, inp);

    case IP_PROTO_TCP:
      return tcp_input(p, inp);

    case IP_PROTO_ICMP:
      return icmp_input(p, inp);
  
    default:
      // Send ICMP destination protocol unreachable unless is was a broadcast
      if (!ip_addr_isbroadcast(&iphdr->dest, &inp->netmask) && !ip_addr_ismulticast(&iphdr->dest)) {
        p->payload = iphdr;
        icmp_dest_unreach(p, ICMP_DUR_PROTO);
      }

      kprintf("Unsupported transport protocol %d\n", IPH_PROTO(iphdr));
      stats.ip.proterr++;
      stats.ip.drop++;
      return -EPROTO;
  }
}

//
// ip_input_dur
//

err_t ip_input_dur(int code, struct pbuf *p) {
  struct ip_hdr *orig_iphdr = (struct ip_hdr *) p->payload;

  if (p->tot_len < sizeof(struct ip_hdr)) {
    kprintf("ip_input_dur: ICMP message too short\n");
    stats.icmp.lenerr++;
    return -EPROTO;
  }

  kprintf("icmp: destination unreachable src=%a dest=%a proto=%d (code %d)\n", &orig_iphdr->src, &orig_iphdr->dest, IPH_PROTO(orig_iphdr), code);
  return -EPROTO;
}

//
// ip_output_if
//
// Sends an IP packet on a network interface. This function constructs the IP header
// and calculates the IP header checksum. If the source IP address is NULL,
// the IP address of the outgoing network interface is filled in as source address.
//

err_t ip_output_if(struct pbuf *p, struct ip_addr *src, struct ip_addr *dest, int ttl, int proto, struct netif *netif) {
  struct ip_hdr *iphdr;
  static unsigned short ip_id = 0;
  
  if (dest != IP_HDRINCL) {
    if (pbuf_header(p, IP_HLEN)) {
      kprintf("ip_output: not enough room for IP header in pbuf\n");
      stats.ip.err++;
      dbg_break();
      return -EBUF;
    }

    iphdr = p->payload;

    IPH_TTL_SET(iphdr, ttl);
    IPH_PROTO_SET(iphdr, proto);
    
    ip_addr_set(&iphdr->dest, dest);

    IPH_VHLTOS_SET(iphdr, 4, IP_HLEN / 4, 0);
    IPH_LEN_SET(iphdr, htons((unsigned short) p->tot_len));
    IPH_OFFSET_SET(iphdr, htons(IP_DF));
    IPH_ID_SET(iphdr, htons(ip_id));
    ip_id++;

    if (ip_addr_isany(src)) {
      ip_addr_set(&iphdr->src, &netif->ipaddr);
    } else {
      ip_addr_set(&iphdr->src, src);
    }

    IPH_CHKSUM_SET(iphdr, 0);

    if ((netif->flags & NETIF_IP_TX_CHECKSUM_OFFLOAD) == 0) {
      IPH_CHKSUM_SET(iphdr, inet_chksum(iphdr, IP_HLEN));
    }
  } else {
    iphdr = p->payload;
    dest = &iphdr->dest;
  }

  stats.ip.xmit++;

  //kprintf("sending IP datagram on %s:\n", netif->name);
  //ip_debug_print(p);

  return netif->output(netif, p, dest);
}

//
// ip_output
//
// Simple interface to ip_output_if. It finds the outgoing network interface and
// calls upon ip_output_if to do the actual work.
//

err_t ip_output(struct pbuf *p, struct ip_addr *src, struct ip_addr *dest, int ttl, int proto) {
  struct netif *netif;
  
  if ((netif = ip_route(dest)) == NULL) {
    kprintf("ip_output: No route to %a\n", &dest);

    stats.ip.rterr++;
    return -EROUTE;
  }

  return ip_output_if(p, src, dest, ttl, proto, netif);
}

void ip_debug_print(struct pbuf *p) {
  struct ip_hdr *iphdr = p->payload;
  unsigned char *payload;

  payload = (unsigned char *) iphdr + IP_HLEN;
  
  kprintf("+-------------------------------+\n");
  kprintf("|%2d |%2d |   %2d  |      %4d     | (v, hl, tos, len)\n", 
          IPH_V(iphdr), 
          IPH_HL(iphdr), 
          IPH_TOS(iphdr), 
          ntohs(IPH_LEN(iphdr)));
  kprintf("+-------------------------------+\n");
  kprintf("|    %5d      |%d%d%d|    %4d   | (id, flags, offset)\n",
          ntohs(IPH_ID(iphdr)),
          ntohs(IPH_OFFSET(iphdr)) >> 15 & 1,
          ntohs(IPH_OFFSET(iphdr)) >> 14 & 1,
          ntohs(IPH_OFFSET(iphdr)) >> 13 & 1,
          ntohs(IPH_OFFSET(iphdr)) & IP_OFFMASK);
  kprintf("+-------------------------------+\n");
  kprintf("|  %3d  |   %2d  |    0x%04x     | (ttl, proto, chksum)\n",
          IPH_TTL(iphdr),
          IPH_PROTO(iphdr),
          ntohs(IPH_CHKSUM(iphdr)));
  kprintf("+-------------------------------+\n");
  kprintf("|  %3ld  |  %3ld  |  %3ld  |  %3ld  | (src)\n",
          ntohl(iphdr->src.addr) >> 24 & 0xFF,
          ntohl(iphdr->src.addr) >> 16 & 0xFF,
          ntohl(iphdr->src.addr) >> 8 & 0xFF,
          ntohl(iphdr->src.addr) & 0xFF);
  kprintf("+-------------------------------+\n");
  kprintf("|  %3ld  |  %3ld  |  %3ld  |  %3ld  | (dest)\n",
          ntohl(iphdr->dest.addr) >> 24 & 0xFF,
          ntohl(iphdr->dest.addr) >> 16 & 0xFF,
          ntohl(iphdr->dest.addr) >> 8 & 0xFF,
          ntohl(iphdr->dest.addr) & 0xFF);
  kprintf("+-------------------------------+\n");
}
