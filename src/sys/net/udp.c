//
// udp.h
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// User Datagram Protocol (UDP)
//

#include <net/net.h>

static struct udp_pcb *udp_pcbs = NULL;

int udp_debug_print(struct udp_hdr *udphdr);
	  
void udp_init()
{
}

//
// udp_new_port
//
// A nastly hack featuring 'goto' statements that allocates a
// new UDP local port.
//

static unsigned short udp_new_port()
{
  struct udp_pcb *pcb;
  static unsigned short port = 4096;
  
again:
  if(++port > 0x7FFF) port = 4096;
  
  for (pcb = udp_pcbs; pcb != NULL; pcb = pcb->next) 
  {
    if (pcb->local_port == port) goto again;
  }

  return port;
}

err_t udp_input(struct pbuf *p, struct netif *inp)
{
  struct udp_hdr *udphdr;  
  struct udp_pcb *pcb;
  struct ip_hdr *iphdr;
  unsigned short src, dest;

  stats.udp.recv++;
  
  iphdr = p->payload;

  pbuf_header(p, -(UDP_HLEN + IPH_HL(iphdr) * 4));

  udphdr = (struct udp_hdr *)((char *) p->payload - UDP_HLEN);
  
  //udp_debug_print(udphdr);

  src = NTOHS(udphdr->src);
  dest = NTOHS(udphdr->dest);

  // Demultiplex packet. First, go for a perfect match. */
  for (pcb = udp_pcbs; pcb != NULL; pcb = pcb->next) 
  {
    if (pcb->remote_port == src &&
        pcb->local_port == dest &&
        (ip_addr_isany(&pcb->remote_ip) || 
	 ip_addr_cmp(&(pcb->remote_ip), &(iphdr->src))) &&
        (ip_addr_isany(&pcb->local_ip) ||
 	 ip_addr_cmp(&(pcb->local_ip), &(iphdr->dest)))) 
    {
      break;
    }
  }

  if (pcb == NULL) 
  {
    for (pcb = udp_pcbs; pcb != NULL; pcb = pcb->next) 
    {
      if (pcb->local_port == dest &&
	  (ip_addr_isany(&pcb->remote_ip) ||
	   ip_addr_cmp(&(pcb->remote_ip), &(iphdr->src))) &&
	  (ip_addr_isany(&pcb->local_ip) ||
	   ip_addr_cmp(&(pcb->local_ip), &(iphdr->dest)))) 
      {
	break;
      }      
    }
  }

  // Check checksum if this is a match or if it was directed at us
  if (pcb != NULL) 
  {
    if ((inp->flags & NETIF_UDP_RX_CHECKSUM_OFFLOAD) == 0)
    {
      pbuf_header(p, UDP_HLEN);
      
      if (IPH_PROTO(iphdr) == IP_PROTO_UDPLITE) 
      {
	// Do the UDP Lite checksum
	if (inet_chksum_pseudo(p, (struct ip_addr *) &(iphdr->src), (struct ip_addr *) &(iphdr->dest), IP_PROTO_UDPLITE, ntohs(udphdr->len)) != 0) 
	{
	  kprintf("udp_input: UDP Lite datagram discarded due to failing checksum\n");
	  stats.udp.chkerr++;
	  stats.udp.drop++;
	  return -ECHKSUM;
	}
      } 
      else 
      {
	if (udphdr->chksum != 0) 
	{
	  if (inet_chksum_pseudo(p, &iphdr->src, &iphdr->dest, IP_PROTO_UDP, p->tot_len) != 0) 
	  {
	    kprintf("udp_input: UDP datagram discarded due to failing checksum\n");

	    stats.udp.chkerr++;
	    stats.udp.drop++;
	    return -ECHKSUM;
	  }
	}
      }

      pbuf_header(p, -UDP_HLEN);
    }

    return pcb->recv(pcb->recv_arg, pcb, p, &iphdr->src, src);
  }
  else 
  {
    // No match was found, send ICMP destination port unreachable unless
    // destination address was broadcast/multicast.
    
    if (!ip_addr_isbroadcast(&iphdr->dest, &inp->netmask) && !ip_addr_ismulticast(&iphdr->dest))
    {	
      // Deconvert from host to network byte order
      udphdr->src = htons(udphdr->src);
      udphdr->dest = htons(udphdr->dest); 
      
      // Adjust pbuf pointer */
      p->payload = iphdr;
      icmp_dest_unreach(p, ICMP_DUR_PORT);
    }

    ++stats.udp.proterr;
    ++stats.udp.drop;

    return -EDESTUNREACH;
  }
}

err_t udp_send(struct udp_pcb *pcb, struct pbuf *p, struct netif *netif)
{
  struct udp_hdr *udphdr;
  struct ip_addr *src_ip;
  err_t err;
  
  if (pbuf_header(p, UDP_HLEN)) 
  {
    kprintf("udp_send: not enough room for UDP header in pbuf\n");
    stats.udp.err++;
    return -EBUF;
  }

  udphdr = p->payload;
  udphdr->src = htons(pcb->local_port);
  udphdr->dest = htons(pcb->remote_port);
  udphdr->chksum = 0x0000;

  if (netif == NULL)
  {
    if ((netif = ip_route(&pcb->remote_ip)) == NULL)
    {
      kprintf("udp_send: No route to %a\n", &pcb->remote_ip);
      stats.udp.rterr++;
      return -EROUTE;
    }
  }

  if (ip_addr_isany(&pcb->local_ip)) 
    src_ip = &netif->ip_addr;
  else 
    src_ip = &pcb->local_ip;
  
  //kprintf("udp_send: sending datagram of length %d\n", p->tot_len);
  
  if (pcb->flags & UDP_FLAGS_UDPLITE) 
  {
    udphdr->len = htons((unsigned short) pcb->chksum_len);
    
    // Calculate checksum
    if ((netif->flags & NETIF_UDP_TX_CHECKSUM_OFFLOAD) == 0)
    {
      udphdr->chksum = inet_chksum_pseudo(p, src_ip, &pcb->remote_ip, IP_PROTO_UDP, pcb->chksum_len);
      if (udphdr->chksum == 0x0000) udphdr->chksum = 0xFFFF;
    }
    err = ip_output_if(p, src_ip, &pcb->remote_ip, UDP_TTL, IP_PROTO_UDPLITE, netif);
  } 
  else 
  {
    udphdr->len = htons((unsigned short) p->tot_len);
    
    // Calculate checksum
    if ((netif->flags & NETIF_UDP_TX_CHECKSUM_OFFLOAD) == 0)
    {
      if ((pcb->flags & UDP_FLAGS_NOCHKSUM) == 0) 
      {
	udphdr->chksum = inet_chksum_pseudo(p, src_ip, &pcb->remote_ip, IP_PROTO_UDP, p->tot_len);
	if (udphdr->chksum == 0x0000) udphdr->chksum = 0xFFFF;
      }
    }

    //udp_debug_print(udphdr);
    err = ip_output_if(p, src_ip, &pcb->remote_ip, UDP_TTL, IP_PROTO_UDP, netif);
  }
  
  stats.udp.xmit++;

  return err;
}

err_t udp_bind(struct udp_pcb *pcb, struct ip_addr *ipaddr, unsigned short port)
{
  struct udp_pcb *ipcb;
  
  ip_addr_set(&pcb->local_ip, ipaddr);
  if (port != 0)
    pcb->local_port = port;
  else
    pcb->local_port = udp_new_port();

  // Insert UDP PCB into the list of active UDP PCBs
  for (ipcb = udp_pcbs; ipcb != NULL; ipcb = ipcb->next) 
  {
    // If it is already on the list, just return
    if (pcb == ipcb) return 0;
  }

  // We need to place the PCB on the list
  pcb->next = udp_pcbs;
  udp_pcbs = pcb;

  //kprintf("udp_bind: bound to port %d\n", port);

  return 0;
}

err_t udp_connect(struct udp_pcb *pcb, struct ip_addr *ipaddr, unsigned short port)
{
  struct udp_pcb *ipcb;
  
  ip_addr_set(&pcb->remote_ip, ipaddr);
  pcb->remote_port = port;
  if (pcb->local_port == 0) pcb->local_port = udp_new_port();

  // Insert UDP PCB into the list of active UDP PCBs
  for (ipcb = udp_pcbs; ipcb != NULL; ipcb = ipcb->next) 
  {
    // If it is already on the list, just return
    if (pcb == ipcb) return 0;
  }

  // We need to place the PCB on the list
  pcb->next = udp_pcbs;
  udp_pcbs = pcb;
  
  return 0;
}

void udp_recv(struct udp_pcb *pcb, err_t (*recv)(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, unsigned short port), void *recv_arg)
{
  pcb->recv = recv;
  pcb->recv_arg = recv_arg;
}

void udp_remove(struct udp_pcb *pcb)
{
  struct udp_pcb *pcb2;
  
  if (udp_pcbs == pcb)
  {
    udp_pcbs = udp_pcbs->next;
  } 
  else 
  {
    for (pcb2 = udp_pcbs; pcb2 != NULL; pcb2 = pcb2->next) 
    {
      if (pcb2->next != NULL && pcb2->next == pcb) 
      {
	pcb2->next = pcb->next;
	break;
      }
    }
  }

  kfree(pcb);
}

struct udp_pcb *udp_new() 
{
  struct udp_pcb *pcb;

  pcb = (struct udp_pcb *) kmalloc(sizeof(struct udp_pcb));
  if (pcb != NULL) 
  {
    memset(pcb, 0, sizeof(struct udp_pcb));
    return pcb;
  }

  return NULL;
}

int udp_debug_print(struct udp_hdr *udphdr)
{
  kprintf("UDP header:\n");
  kprintf("+-------------------------------+\n");
  kprintf("|     %5d     |     %5d     | (src port, dest port)\n", ntohs(udphdr->src), ntohs(udphdr->dest));
  kprintf("+-------------------------------+\n");
  kprintf("|     %5d     |     0x%04x    | (len, chksum)\n", ntohs(udphdr->len), ntohs(udphdr->chksum));
  kprintf("+-------------------------------+\n");

  return 0;
}
