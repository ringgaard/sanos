//
// tcp_output.c
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Transmission Control Protocol (TCP)
//

#include <net/net.h>

#define MIN(x,y) ((x) < (y) ? (x): (y))

static void tcp_output_segment(struct tcp_seg *seg, struct tcp_pcb *pcb);

err_t tcp_send_ctrl(struct tcp_pcb *pcb, int flags)
{
  //kprintf("tcp_send_ctrl: sending flags (");
  //tcp_debug_print_flags(flags);
  //kprintf(")\n");

  return tcp_enqueue(pcb, NULL, 0, flags, 1, NULL, 0);
}

err_t tcp_write(struct tcp_pcb *pcb, const void *arg, unsigned short len, int copy)
{
  int rc;

  if (pcb->state == SYN_SENT || pcb->state == SYN_RCVD || pcb->state == ESTABLISHED || pcb->state == CLOSE_WAIT) 
  {
    if (len > 0) 
    {
      rc = tcp_enqueue(pcb, (void *) arg, len, 0, copy, NULL, 0);
      if (rc < 0) return rc;

      // This is the Nagle algorithm (RFC 896): inhibit the sending of new TCP
      // segments when new outgoing data arrives from the user if any
      // previously transmitted data on the connection remains
      // unacknowledged.

      if (pcb->unacked == NULL) tcp_output(pcb);
    }

    return 0;
  } 
  else
    return -ECONN;
}

err_t tcp_enqueue(struct tcp_pcb *pcb, void *arg, unsigned short len, int flags, int copy, unsigned char *optdata, int optlen)
{
  struct pbuf *p;
  struct tcp_seg *seg, *useg, *queue;
  unsigned long left, seqno;
  unsigned short seglen;
  void *ptr;
  int queuelen;

  left = len;
  ptr = arg;
  
  if (len > pcb->snd_buf) 
  {
    kprintf("tcp_enqueue: too much data %d\n", len);
    return -ENOMEM;
  }

  seqno = pcb->snd_lbb;
  
  queue = NULL;
  queuelen = pcb->snd_queuelen;
  if (queuelen >= TCP_SND_QUEUELEN)
  {
    kprintf("tcp_enqueue: too long queue %d (max %d)\n", queuelen, TCP_SND_QUEUELEN);
    goto memerr;
  }
  
  seg = NULL;
  seglen = 0;
  
  while (queue == NULL || left > 0) 
  {
    seglen = (unsigned short) (left > pcb->mss ? pcb->mss : left);

    // Allocate memory for tcp_seg, and fill in fields
    seg = (struct tcp_seg *) kmalloc(sizeof(struct tcp_seg));
    if (seg == NULL) 
    {
      kprintf("tcp_enqueue: could not allocate memory for tcp_seg\n");
      goto memerr;
    }
    seg->next = NULL;
    seg->p = NULL;

    if (queue == NULL) 
    {
      queue = seg;
    } 
    else 
    {
      for (useg = queue; useg->next != NULL; useg = useg->next);
      useg->next = seg;
    }
    
    // If copy is set, memory should be allocated
    // and data copied into pbuf, otherwise data comes from
    // ROM or other static memory, and need not be copied. If
    // optdata is != NULL, we have options instead of data

    if (optdata != NULL) 
    {
      if ((seg->p = pbuf_alloc(PBUF_TRANSPORT, optlen, PBUF_RW)) == NULL) goto memerr;
      queuelen++;
      seg->dataptr = (char *) seg->p->payload + optlen;
    } 
    else if (copy) 
    {
      if ((seg->p = pbuf_alloc(PBUF_TRANSPORT, seglen, PBUF_RW)) == NULL) 
      {
	kprintf("tcp_enqueue: could not allocate memory for pbuf copy\n");
	goto memerr;
      }
      queuelen++;

      if (arg != NULL) memcpy(seg->p->payload, ptr, seglen);
      seg->dataptr = seg->p->payload;
    } 
    else 
    {
      // Do not copy the data
      if ((p = pbuf_alloc(PBUF_TRANSPORT, seglen, PBUF_RO)) == NULL)
      {
	kprintf("tcp_enqueue: could not allocate memory for pbuf non-copy\n");
	goto memerr;
      }
      
      queuelen++;
      p->payload = ptr;
      seg->dataptr = ptr;

      if ((seg->p = pbuf_alloc(PBUF_TRANSPORT, 0, PBUF_RW)) == NULL) 
      {
        pbuf_free(p);
	kprintf("tcp_enqueue: could not allocate memory for header pbuf\n");
	goto memerr;
      }

      queuelen++;
      pbuf_chain(seg->p, p);
    }

    if (queuelen > TCP_SND_QUEUELEN) 
    {
      kprintf("tcp_enqueue: queue too long %d (%d)\n", queuelen, TCP_SND_QUEUELEN);
      goto memerr;
    }
    
    seg->len = seglen;
    
    // Build TCP header
    if (pbuf_header(seg->p, TCP_HLEN) < 0) 
    {
      kprintf("tcp_enqueue: no room for TCP header in pbuf.\n");
      
      stats.tcp.err++;
      goto memerr;
    }

    seg->tcphdr = seg->p->payload;
    seg->tcphdr->src = htons(pcb->local_port);
    seg->tcphdr->dest = htons(pcb->remote_port);
    seg->tcphdr->seqno = htonl(seqno);
    seg->tcphdr->urgp = 0;
    TCPH_FLAGS_SET(seg->tcphdr, flags);
    // Don't fill in tcphdr->ackno and tcphdr->wnd until later
    
    if (optdata == NULL) 
    {
      TCPH_OFFSET_SET(seg->tcphdr, 5 << 4);
    } 
    else 
    {
      TCPH_OFFSET_SET(seg->tcphdr, (5 + optlen / 4) << 4);
      
      // Copy options into segment after fixed TCP header
      memcpy(seg->tcphdr + 1, optdata, optlen);
    }

    //kprintf("tcp_enqueue: queueing %lu:%lu (0x%x)\n", ntohl(seg->tcphdr->seqno), ntohl(seg->tcphdr->seqno) + TCP_TCPLEN(seg), flags);

    left -= seglen;
    seqno += seglen;
    ptr = (void *) ((char *) ptr + seglen);
  }
  
  // Go to the last segment on the unsent queue
  if (pcb->unsent == NULL)
  {
    useg = NULL;
  } 
  else 
  {
    for (useg = pcb->unsent; useg->next != NULL; useg = useg->next);
  }
  
  // If there is room in the last pbuf on the unsent queue
  // chain the first pbuf on the queue together with that
  if (useg != NULL && 
      TCP_TCPLEN(useg) != 0 && 
      !(TCPH_FLAGS(useg->tcphdr) & (TCP_SYN | TCP_FIN)) && 
      !(flags & (TCP_SYN | TCP_FIN)) && 
      useg->len + queue->len <= pcb->mss) 
  {
    // Remove TCP header from first segment
    pbuf_header(queue->p, -TCP_HLEN);
    pbuf_chain(useg->p, queue->p);
    useg->len += queue->len;
    useg->next = queue->next;
    
    kprintf("tcp_output: chaining, new len %u\n", useg->len);

    if (seg == queue) seg = NULL;
    kfree(queue);
  } 
  else 
  {      
    if (useg == NULL) 
      pcb->unsent = queue;
    else
      useg->next = queue;
  }

  if ((flags & TCP_SYN) || (flags & TCP_FIN)) len++;
  pcb->snd_lbb += len;
  pcb->snd_buf -= len;
  pcb->snd_queuelen = queuelen;

  // Set the PSH flag in the last segment that we enqueued, but only
  // if the segment has data (indicated by seglen > 0)
  if (seg != NULL && seglen > 0 && seg->tcphdr != NULL) 
  {
    TCPH_FLAGS_SET(seg->tcphdr, TCPH_FLAGS(seg->tcphdr) | TCP_PSH);
  }

  return 0;

memerr:
  stats.tcp.memerr++;

  if (queue != NULL) tcp_segs_free(queue);
  return -ENOMEM;
}

err_t tcp_output(struct tcp_pcb *pcb)
{
  struct pbuf *p;
  struct tcp_hdr *tcphdr;
  struct tcp_seg *seg, *useg;
  unsigned long wnd;
    
  wnd = MIN(pcb->snd_wnd, pcb->cwnd);
  seg = pcb->unsent;
  
  if (pcb->flags & TF_ACK_NOW) 
  {
    // If no segments are enqueued but we should send an ACK, we
    // construct the ACK and send it
    
    //kprintf("tcp_enqueue: sending ACK for %lu\n", pcb->rcv_nxt);
    pcb->flags &= ~(TF_ACK_DELAY | TF_ACK_NOW);
    p = pbuf_alloc(PBUF_TRANSPORT, 0, PBUF_RW);
    if (!p) 
    {
      stats.tcp.memerr++;
      return -ENOMEM; 
    }
    if (pbuf_header(p, TCP_HLEN) < 0) 
    {      
      stats.tcp.err++;
      pbuf_free(p);
      return -ENOMEM;
    }
    
    tcphdr = p->payload;
    tcphdr->src = htons(pcb->local_port);
    tcphdr->dest = htons(pcb->remote_port);
    tcphdr->seqno = htonl(pcb->snd_nxt);
    tcphdr->ackno = htonl(pcb->rcv_nxt);
    TCPH_FLAGS_SET(tcphdr, TCP_ACK);
    tcphdr->wnd = htons(pcb->rcv_wnd);
    tcphdr->urgp = 0;
    TCPH_OFFSET_SET(tcphdr, 5 << 4);
    
    tcphdr->chksum = 0;
    tcphdr->chksum = inet_chksum_pseudo(p, &pcb->local_ip, &pcb->remote_ip, IP_PROTO_TCP, p->tot_len);

    //kprintf("sending TCP ack segment:\n");
    //tcp_debug_print(tcphdr);

    if (ip_output(p, &pcb->local_ip, &pcb->remote_ip, TCP_TTL, IP_PROTO_TCP) < 0) pbuf_free(p);

    return 0;
  } 
 
  while (seg != NULL &&	ntohl(seg->tcphdr->seqno) - pcb->lastack + seg->len <= wnd) 
  {
    pcb->rtime = 0;
    pcb->unsent = seg->next;
    
    if (pcb->state != SYN_SENT) 
    {
      TCPH_FLAGS_SET(seg->tcphdr, TCPH_FLAGS(seg->tcphdr) | TCP_ACK);
      pcb->flags &= ~(TF_ACK_DELAY | TF_ACK_NOW);
    }
    
    tcp_output_segment(seg, pcb);
    pcb->snd_nxt = ntohl(seg->tcphdr->seqno) + TCP_TCPLEN(seg);
    if (TCP_SEQ_LT(pcb->snd_max, pcb->snd_nxt)) pcb->snd_max = pcb->snd_nxt;

    // Put segment on unacknowledged list if length > 0
    if (TCP_TCPLEN(seg) > 0) 
    {
      seg->next = NULL;
      if (pcb->unacked == NULL) 
      {
        pcb->unacked = seg;
      } 
      else 
      {
        for (useg = pcb->unacked; useg->next != NULL; useg = useg->next);
        useg->next = seg;
      }
    } 
    else 
      tcp_seg_free(seg);
    
    seg = pcb->unsent;
  } 
  
  return 0;
}

static void tcp_output_segment(struct tcp_seg *seg, struct tcp_pcb *pcb)
{
  struct netif *netif;

  // The TCP header has already been constructed, but the ackno and wnd fields remain
  seg->tcphdr->ackno = htonl(pcb->rcv_nxt);

  // Silly window avoidance
  if (pcb->rcv_wnd < pcb->mss) 
    seg->tcphdr->wnd = 0;
  else 
    seg->tcphdr->wnd = htons(pcb->rcv_wnd);

  // Find route for segment
  netif = ip_route(&pcb->remote_ip);
  if (netif == NULL) 
  {
    kprintf("tcp_output_segment: No route to %a\n", &pcb->remote_ip);
    stats.ip.rterr++;
    return;
  }

  // If we don't have a local IP address, we get it from netif
  if (ip_addr_isany(&pcb->local_ip)) 
  {
    ip_addr_set(&pcb->local_ip, &netif->ip_addr);
  }

  pcb->rtime = 0;
  
  if (pcb->rttest == 0)
  {
    pcb->rttest = (unsigned short) tcp_ticks;
    pcb->rtseq = ntohl(seg->tcphdr->seqno);
  }

  kprintf("tcp_output_segment: %lu:%lu\n", htonl(seg->tcphdr->seqno), htonl(seg->tcphdr->seqno) + seg->len);

  seg->tcphdr->chksum = 0;
  if ((netif->flags & NETIF_TCP_TX_CHECKSUM_OFFLOAD) == 0)
  {
    seg->tcphdr->chksum = inet_chksum_pseudo(seg->p, &pcb->local_ip, &pcb->remote_ip, IP_PROTO_TCP, seg->p->tot_len);
  }
  stats.tcp.xmit++;

  //kprintf("sending TCP segment:\n");
  //tcp_debug_print(seg->tcphdr);
 
  pbuf_ref(seg->p);
  if (ip_output_if(seg->p, &pcb->local_ip, &pcb->remote_ip, TCP_TTL, IP_PROTO_TCP, netif) < 0) pbuf_free(seg->p);
}


void tcp_rexmit_seg(struct tcp_pcb *pcb, struct tcp_seg *seg)
{
  unsigned long wnd;
  struct netif *netif;
  int offset;

  kprintf("tcp_rexmit_seg: sending %ld:%ld\n", ntohl(seg->tcphdr->seqno), ntohl(seg->tcphdr->seqno) + TCP_TCPLEN(seg));
  
  wnd = MIN(pcb->snd_wnd, pcb->cwnd);
  if (ntohl(seg->tcphdr->seqno) - pcb->lastack + seg->len > wnd)
  {
    kprintf("tcp_rexmit_seg: no room in window %lu to send %lu (ack %lu)\n", wnd, ntohl(seg->tcphdr->seqno), pcb->lastack);
    return;
  }

  // If the buffer is still waiting to be sent, we do not retransmit it.
  // The packet buffer reference counter is used to determine if the
  // packet is still on the transmission queue.
  if (seg->p->ref > 1) 
  {
    kprintf("tcp_rexmit_segment: packet not retransmitted, still in tx queue\n");
    return;
  }

  // Count the number of retransmissions
  pcb->nrtx++;

  // Find route for segment
  if ((netif = ip_route(&pcb->remote_ip)) == NULL) 
  {
    kprintf("tcp_rexmit_segment: No route to %a\n", &pcb->remote_ip);
    stats.tcp.rterr++;
    return;
  }

  // The payload pointer of the packet buffer might have been changed by
  // the lower protocol layers, and we need to adjust the payload pointer
  // and length before resubmitting the packet. It is assummed that the
  // first buffer contains all the headers.
  offset = (char *) seg->p->payload - (char *) seg->tcphdr;
  pbuf_header(seg->p, offset);

  // Update ack and receive window
  seg->tcphdr->ackno = htonl(pcb->rcv_nxt);
  seg->tcphdr->wnd = htons(pcb->rcv_wnd);

  // Recalculate checksum
  seg->tcphdr->chksum = 0;
  if ((netif->flags & NETIF_TCP_TX_CHECKSUM_OFFLOAD) == 0)
  {
    seg->tcphdr->chksum = inet_chksum_pseudo(seg->p, &pcb->local_ip, &pcb->remote_ip, IP_PROTO_TCP, seg->p->tot_len);
  }

  //kprintf("resending TCP segment:\n");
  //tcp_debug_print(seg->tcphdr);

  pbuf_ref(seg->p);
  if (ip_output_if(seg->p, NULL, IP_HDRINCL, TCP_TTL, IP_PROTO_TCP, netif) < 0) pbuf_free(seg->p);

  stats.tcp.xmit++;
  stats.tcp.rexmit++;

  pcb->rtime = 0;
  
  // Don't take any rtt measurements after retransmitting
  pcb->rttest = 0;
}

void tcp_rst(unsigned long seqno, unsigned long ackno, struct ip_addr *local_ip, struct ip_addr *remote_ip, unsigned short local_port, unsigned short remote_port)
{
  struct pbuf *p;
  struct tcp_hdr *tcphdr;
  struct netif *netif;

  if ((netif = ip_route(remote_ip)) == NULL) 
  {
    kprintf("tcp_rst: No route to %a\n", &remote_ip);
    stats.tcp.rterr++;
    return;
  }

  p = pbuf_alloc(PBUF_TRANSPORT, 0, PBUF_RW);
  if (p == NULL) 
  {
    // Reclaim memory here
    kprintf("tcp_rst: could not allocate memory for pbuf\n");
    stats.tcp.memerr++;
    return;
  }

  if (pbuf_header(p, TCP_HLEN) < 0) 
  {
    kprintf("tcp_send_data: no room for TCP header in pbuf.\n");
    stats.tcp.err++;
    pbuf_free(p);
    return;
  }

  tcphdr = p->payload;
  tcphdr->src = htons(local_port);
  tcphdr->dest = htons(remote_port);
  tcphdr->seqno = htonl(seqno);
  tcphdr->ackno = htonl(ackno);
  TCPH_FLAGS_SET(tcphdr, TCP_RST | TCP_ACK);
  tcphdr->wnd = 0;
  tcphdr->urgp = 0;
  TCPH_OFFSET_SET(tcphdr, 5 << 4);
  
  tcphdr->chksum = 0;
  if ((netif->flags & NETIF_TCP_TX_CHECKSUM_OFFLOAD) == 0)
  {
    tcphdr->chksum = inet_chksum_pseudo(p, local_ip, remote_ip, IP_PROTO_TCP, p->tot_len);
  }

  stats.tcp.xmit++;

  //kprintf("sending TCP rst segment:\n");
  //tcp_debug_print(tcphdr);

  if (ip_output_if(p, local_ip, remote_ip, TCP_TTL, IP_PROTO_TCP, netif) < 0) pbuf_free(p);

  kprintf("tcp_rst: seqno %lu ackno %lu.\n", seqno, ackno);
}
