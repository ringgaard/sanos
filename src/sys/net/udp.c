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

void udp_input(struct pbuf *p, struct netif *inp)
{
  struct udp_hdr *udphdr;  
  struct udp_pcb *pcb;
  struct ip_hdr *iphdr;

  stats.udp.recv++;
  
  iphdr = (struct ip_hdr *) ((char *) p->payload - IP_HLEN);
  udphdr = p->payload;
  
  if ((inp->flags & NETIF_UDP_RX_CHECKSUM_OFFLOAD) == 0)
  {
    if (IPH_PROTO(iphdr) == IP_PROTO_UDPLITE) 
    {
      // Do the UDP Lite checksum
      if (inet_chksum_pseudo(p, (struct ip_addr *) &(iphdr->src), (struct ip_addr *) &(iphdr->dest), IP_PROTO_UDPLITE, ntohs(udphdr->len)) != 0) 
      {
	kprintf("udp_input: UDP Lite datagram discarded due to failing checksum\n");
	stats.udp.chkerr++;
	stats.udp.drop++;
	pbuf_free(p);
	return;
      }
    } 
    else 
    {
      if (udphdr->chksum != 0) 
      {
	if (inet_chksum_pseudo(p, (struct ip_addr *) &(iphdr->src), (struct ip_addr *) &(iphdr->dest), IP_PROTO_UDP, p->tot_len) != 0) 
	{
	  kprintf("udp_input: UDP datagram discarded due to failing checksum\n");

	  stats.udp.chkerr++;
	  stats.udp.drop++;
	  pbuf_free(p);
	  return;
	}
      }
    }
  }

  pbuf_header(p, -UDP_HLEN);

  kprintf("udp_input: received datagram of length %d\n", p->tot_len);
	
  udphdr->src = ntohs(udphdr->src);
  udphdr->dest = ntohs(udphdr->dest);
  //udphdr->len = ntohs(udphdr->len);

  udp_debug_print(udphdr);
  
  // Demultiplex packet. First, go for a perfect match.
  for (pcb = udp_pcbs; pcb != NULL; pcb = pcb->next) 
  {
    kprintf("udp_input: pcb local port %d (dgram %d)\n", pcb->local_port, udphdr->dest);

    if (pcb->dest_port == udphdr->src &&
        pcb->local_port == udphdr->dest &&
        (ip_addr_isany(&pcb->dest_ip) ||
 	 ip_addr_cmp(&(pcb->dest_ip), &(iphdr->src))) &&
        (ip_addr_isany(&pcb->local_ip) ||
	ip_addr_cmp(&(pcb->local_ip), &(iphdr->dest)))) 
    {
      pcb->recv(pcb->recv_arg, pcb, p, &(iphdr->src), udphdr->src);
      return;
    }
  }
  
  for (pcb = udp_pcbs; pcb != NULL; pcb = pcb->next) 
  {
    kprintf("udp_input: pcb local port %d (dgram %d)\n", pcb->local_port, udphdr->dest);

    if (pcb->local_port == udphdr->dest &&
        (ip_addr_isany(&pcb->dest_ip) ||
 	 ip_addr_cmp(&(pcb->dest_ip), &(iphdr->src))) &&
        (ip_addr_isany(&pcb->local_ip) ||
	 ip_addr_cmp(&(pcb->local_ip), &(iphdr->dest)))) 
    {
      pcb->recv(pcb->recv_arg, pcb, p, &(iphdr->src), udphdr->src);
      return;
    }
  }
   
  // No match was found, send ICMP destination port unreachable unless
  // destination address was broadcast/multicast
  if (!ip_addr_isbroadcast(&iphdr->dest, &inp->netmask) && !ip_addr_ismulticast(&iphdr->dest)) 
  {  
    // Deconvert from host to network byte order
    udphdr->src = htons(udphdr->src);
    udphdr->dest = htons(udphdr->dest); 
  
    /* Adjust pbuf pointer */
    p->payload = iphdr;
    icmp_dest_unreach(p, ICMP_DUR_PORT);
  }

  stats.udp.proterr++;
  stats.udp.drop++;
  pbuf_free(p);
}

err_t udp_send(struct udp_pcb *pcb, struct pbuf *p)
{
  struct udp_hdr *udphdr;
  struct netif *netif;
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
  udphdr->dest = htons(pcb->dest_port);
  udphdr->chksum = 0x0000;

  if ((netif = ip_route(&(pcb->dest_ip))) == NULL)
  {
    kprintf("udp_send: No route to 0x%lx\n", pcb->dest_ip.addr);
    stats.udp.rterr++;
    return -EROUTE;
  }

  if (ip_addr_isany(&pcb->local_ip)) 
    src_ip = &(netif->ip_addr);
  else 
    src_ip = &(pcb->local_ip);
  
  kprintf("udp_send: sending datagram of length %d\n", p->tot_len);
  
  if (pcb->flags & UDP_FLAGS_UDPLITE) 
  {
    udphdr->len = htons(pcb->chksum_len);
    
    // Calculate checksum
    if ((netif->flags & NETIF_UDP_TX_CHECKSUM_OFFLOAD) == 0)
    {
      udphdr->chksum = inet_chksum_pseudo(p, src_ip, &(pcb->dest_ip), IP_PROTO_UDP, pcb->chksum_len);
      if (udphdr->chksum == 0x0000) udphdr->chksum = 0xFFFF;
    }
    err = ip_output_if(p, src_ip, &pcb->dest_ip, UDP_TTL, IP_PROTO_UDPLITE, netif);
  } 
  else 
  {
    udphdr->len = htons(p->tot_len);
    
    // Calculate checksum
    if ((netif->flags & NETIF_UDP_TX_CHECKSUM_OFFLOAD) == 0)
    {
      if ((pcb->flags & UDP_FLAGS_NOCHKSUM) == 0) 
      {
	udphdr->chksum = inet_chksum_pseudo(p, src_ip, &pcb->dest_ip, IP_PROTO_UDP, p->tot_len);
	if (udphdr->chksum == 0x0000) udphdr->chksum = 0xFFFF;
      }
    }

    err = ip_output_if(p, src_ip, &pcb->dest_ip, UDP_TTL, IP_PROTO_UDP, netif);
  }
  
  stats.udp.xmit++;

  return err;
}

err_t udp_bind(struct udp_pcb *pcb, struct ip_addr *ipaddr, unsigned short port)
{
  struct udp_pcb *ipcb;
  
  if (ipaddr != NULL) pcb->local_ip = *ipaddr;
  pcb->local_port = port;

  // Insert UDP PCB into the list of active UDP PCBs
  for (ipcb = udp_pcbs; ipcb != NULL; ipcb = ipcb->next) 
  {
    // If it is already on the list, just return
    if (pcb == ipcb) return 0;
  }

  // We need to place the PCB on the list
  pcb->next = udp_pcbs;
  udp_pcbs = pcb;

  kprintf("udp_bind: bound to port %d\n", port);

  return 0;
}

err_t udp_connect(struct udp_pcb *pcb, struct ip_addr *ipaddr, unsigned short port)
{
  struct udp_pcb *ipcb;
  
  if (ipaddr != NULL) pcb->dest_ip = *ipaddr;
  pcb->dest_port = port;

  // Insert UDP PCB into the list of active UDP PCBs
  for (ipcb = udp_pcbs; ipcb != NULL; ipcb = ipcb->next) 
  {
    // If it is already on the list, just return
    if(pcb == ipcb) return 0;
  }

  // We need to place the PCB on the list
  pcb->next = udp_pcbs;
  udp_pcbs = pcb;
  
  return 0;
}

void udp_recv(struct udp_pcb *pcb, void (* recv)(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, unsigned short port), void *recv_arg)
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
