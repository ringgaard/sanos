//
// icmp.c
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Internet Control Message Protocol (ICMP)
//
// Some ICMP messages should be passed to the transport protocols. This
// is not implemented.
//

#include <net/net.h>

void icmp_input(struct pbuf *p, struct netif *inp)
{
  unsigned char type;
  struct icmp_echo_hdr *iecho;
  struct ip_hdr *iphdr;
  struct ip_addr tmpaddr;
  int hlen;
  
  stats.icmp.recv++;

  iphdr = p->payload;
  hlen = IPH_HL(iphdr) * 4;
  pbuf_header(p, -hlen);

  type = *((unsigned char *) p->payload);

  switch (type) 
  {
    case ICMP_ECHO:
      if (ip_addr_isbroadcast(&iphdr->dest, &inp->netmask) || ip_addr_ismulticast(&iphdr->dest)) 
      {
	stats.icmp.err++;
	pbuf_free(p);
	return;
      }
    
      kprintf("icmp_input: ping\n");

      if (p->tot_len < sizeof(struct icmp_echo_hdr)) 
      {
	kprintf("icmp_input: bad ICMP echo received\n");
	pbuf_free(p);
	stats.icmp.lenerr++;
	return;      
      }

      iecho = p->payload;    
      if (inet_chksum_pbuf(p) != 0) 
      {
	kprintf("icmp_input: checksum failed for received ICMP echo\n");
	pbuf_free(p);
	stats.icmp.chkerr++;
	return;
      }

      tmpaddr.addr = iphdr->src.addr;
      iphdr->src.addr = iphdr->dest.addr;
      iphdr->dest.addr = tmpaddr.addr;
      ICMPH_TYPE_SET(iecho, ICMP_ER);
      
      // Adjust the checksum
      if (iecho->chksum >= htons(0xFFFF - (ICMP_ECHO << 8))) 
	iecho->chksum += htons(ICMP_ECHO << 8) + 1;
      else
	iecho->chksum += htons(ICMP_ECHO << 8);

      stats.icmp.xmit++;
      
      pbuf_header(p, hlen);
      ip_output_if(p, &(iphdr->src), IP_HDRINCL, IPH_TTL(iphdr), IP_PROTO_ICMP, inp);
      break; 

    default:
      kprintf("icmp_input: ICMP type not supported.\n");
      stats.icmp.proterr++;
      stats.icmp.drop++;
  }

  pbuf_free(p);
}

void icmp_dest_unreach(struct pbuf *p, int t)
{
  struct pbuf *q;
  struct ip_hdr *iphdr;
  struct icmp_dur_hdr *idur;
  
  // ICMP header + IP header + 8 bytes of data
  q = pbuf_alloc(PBUF_TRANSPORT, 8 + IP_HLEN + 8, PBUF_RW);

  iphdr = p->payload;
  
  idur = q->payload;
  ICMPH_TYPE_SET(idur, ICMP_DUR);
  ICMPH_CODE_SET(idur, t);

  memcpy((char *) q->payload + 8, p->payload, IP_HLEN + 8);
  
  // Calculate checksum
  idur->chksum = 0;
  idur->chksum = inet_chksum(idur, q->len);
  stats.icmp.xmit++;

  ip_output(q, NULL, &(iphdr->src), ICMP_TTL, IP_PROTO_ICMP);
  pbuf_free(q);
}

void icmp_time_exceeded(struct pbuf *p, int t)
{
  struct pbuf *q;
  struct ip_hdr *iphdr;
  struct icmp_te_hdr *tehdr;

  q = pbuf_alloc(PBUF_TRANSPORT, 8 + IP_HLEN + 8, PBUF_RW);

  iphdr = p->payload;
  tehdr = q->payload;
  ICMPH_TYPE_SET(tehdr, ICMP_TE);
  ICMPH_CODE_SET(tehdr, t);

  // Copy fields from original packet
  memcpy((char *) q->payload + 8, (char *) p->payload, IP_HLEN + 8);
  
  // Calculate checksum
  tehdr->chksum = 0;
  tehdr->chksum = inet_chksum(tehdr, q->len);
  stats.icmp.xmit++;

  ip_output(q, NULL, &(iphdr->src), ICMP_TTL, IP_PROTO_ICMP);
  pbuf_free(q);
}
