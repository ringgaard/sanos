//
// tcp_output.c
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Transmission Control Protocol (TCP)
//

#include <net/net.h>

#define UMAX(a, b) ((a) > (b) ? (a) : (b))

static err_t tcp_process(struct tcp_seg *seg, struct tcp_pcb *pcb);
static void tcp_receive(struct tcp_seg *seg, struct tcp_pcb *pcb);

//
// tcp_input
//
// The initial input processing of TCP. It verifies the TCP header, demultiplexes
// the segment between the PCBs and passes it on to tcp_process(), which implements
// the TCP finite state machine. This function is called by the IP layer in ip_input.
//

void tcp_input(struct pbuf *p, struct netif *inp)
{
  struct tcp_hdr *tcphdr;
  struct tcp_pcb *pcb, *prev;
  struct ip_hdr *iphdr;
  struct tcp_seg *seg;
  int offset;
  err_t err;

  stats.tcp.recv++;
  
  tcphdr = p->payload;
  iphdr = (struct ip_hdr *) ((char *) p->payload - IP_HLEN);

  // Don't even process incoming broadcasts/multicasts
  if (ip_addr_isbroadcast(&(iphdr->dest), &(inp->netmask)) || ip_addr_ismulticast(&(iphdr->dest))) 
  {
    pbuf_free(p);
    return;
  }

  // Verify TCP checksum
  if (inet_chksum_pseudo(p, (struct ip_addr *) &(iphdr->src), (struct ip_addr *) &(iphdr->dest), IP_PROTO_TCP, p->tot_len) != 0) 
  {
    kprintf("tcp_input: packet discarded due to failing checksum 0x%04x\n", inet_chksum_pseudo(p, (struct ip_addr *)&(iphdr->src), (struct ip_addr *)&(iphdr->dest), IP_PROTO_TCP, p->tot_len));
    tcp_debug_print(tcphdr);

    stats.tcp.chkerr++;
    stats.tcp.drop++;

    pbuf_free(p);
    return;
  }

  // Move the payload pointer in the pbuf so that it points to the TCP data instead of the IP header
  offset = TCPH_OFFSET(tcphdr) >> 4;
  pbuf_header(p, -(offset * 4));

  // Convert fields in TCP header to host byte order
  tcphdr->src = ntohs(tcphdr->src);
  tcphdr->dest = ntohs(tcphdr->dest);
  tcphdr->seqno = ntohl(tcphdr->seqno);
  tcphdr->ackno = ntohl(tcphdr->ackno);
  tcphdr->wnd = ntohs(tcphdr->wnd);
  
  // Demultiplex an incoming segment. First, we check if it is destined for an active connection
  prev = NULL;  
  for (pcb = tcp_active_pcbs; pcb != NULL; pcb = pcb->next)
  {
    if (pcb->dest_port == tcphdr->src &&
        pcb->local_port == tcphdr->dest &&
        ip_addr_cmp(&(pcb->dest_ip), &(iphdr->src)) &&
        ip_addr_cmp(&(pcb->local_ip), &(iphdr->dest))) 
    {
      // Move this PCB to the front of the list so that subsequent
      // lookups will be faster (we exploit locality in TCP segment arrivals
      if (prev != NULL) 
      {
        prev->next = pcb->next;
        pcb->next = tcp_active_pcbs;
        tcp_active_pcbs = pcb; 
      }
      break;
    }

    prev = pcb;
  }

  // If it did not go to an active connection, we check the connections in the TIME-WAIT state
  if (pcb == NULL) 
  {
    for (pcb = tcp_tw_pcbs; pcb != NULL; pcb = pcb->next) 
    {
      if (pcb->dest_port == tcphdr->src &&
	  pcb->local_port == tcphdr->dest &&
	  ip_addr_cmp(&(pcb->dest_ip), &(iphdr->src)) &&
          ip_addr_cmp(&(pcb->local_ip), &(iphdr->dest))) 
      {
	// We don't really care enough to move this PCB to the front
	// of the list since we are not very likely to receive that
	// many segments for connections in TIME-WAIT. */
	break;
      }
    }  
  
    // Finally, if we still did not get a match, we check all PCBs that are LISTENing for incomming connections
    prev = NULL;  
    if (pcb == NULL) 
    {
      for (pcb = (struct tcp_pcb *)tcp_listen_pcbs; pcb != NULL; pcb = pcb->next) 
      {
	if ((ip_addr_isany(&(pcb->local_ip)) || ip_addr_cmp(&(pcb->local_ip), &(iphdr->dest))) &&
	     pcb->local_port == tcphdr->dest) 
	{
	  // Move this PCB to the front of the list so that subsequent
	  // lookups will be faster (we exploit locality in TCP segment
	  // arrivals)
	  
	  if (prev != NULL) 
	  {
	    prev->next = pcb->next;
	    pcb->next = (struct tcp_pcb *) tcp_listen_pcbs;
	    tcp_listen_pcbs = (struct tcp_pcb_listen *) pcb; 
	  }
	  break;
	}
	prev = pcb;
      }
    }
  }
  
  seg = (struct tcp_seg *) kmalloc(sizeof(struct tcp_seg));
  if (seg != NULL && pcb != NULL) 
  {
    // Set up a tcp_seg structure
    seg->next = NULL;
    seg->len = p->tot_len;
    seg->dataptr = p->payload;
    seg->p = p;
    seg->tcphdr = tcphdr;
    
    // The len field in the tcp_seg structure is the segment length
    // in TCP terms. In TCP, the SYN and FIN segments are treated as
    // one byte, hence increment the len field
    
    if (TCPH_FLAGS(tcphdr) & TCP_FIN || TCPH_FLAGS(tcphdr) & TCP_SYN) seg->len++;

    if (pcb->state != LISTEN && pcb->state != TIME_WAIT) pcb->recv_data = NULL;

    tcp_process(seg, pcb);

    if (pcb->state != LISTEN) 
    {
      if (pcb->flags & TF_RESET) 
      {
	if (pcb->state < FIN_WAIT_1)
	{
	  if (pcb->errf != NULL)  pcb->errf(pcb->callback_arg, -ERST);
	}

	if (pcb->state == TIME_WAIT) 
	  tcp_pcb_remove(&tcp_tw_pcbs, pcb);
	else
	  tcp_pcb_remove(&tcp_active_pcbs, pcb);

	kfree(pcb);
      } 
      else if (pcb->flags & TF_CLOSED) 
      {
	tcp_pcb_remove(&tcp_active_pcbs, pcb);
	kfree(pcb);
      } 
      else 
      {
	if (pcb->state < TIME_WAIT) 
	{
	  err = 0;
	  
	  // If the application has registered a "sent" function to be
	  // called when new send buffer space is avaliable, we call it now

	  if (pcb->acked > 0 && pcb->sent != NULL) err = pcb->sent(pcb->callback_arg, pcb, pcb->acked);
	  
	  if (pcb->recv != NULL)
	  {
	    if (pcb->recv_data != NULL)
	    {
	      err = pcb->recv(pcb->callback_arg, pcb, pcb->recv_data, 0);
	    }

	    if (pcb->flags & TF_GOT_FIN) 
	    {
	      err = pcb->recv(pcb->callback_arg, pcb, NULL, 0);
	    }
	  } 
	  else 
	  {
	    err = 0;
	    pbuf_free(pcb->recv_data);
	  }

	  if (err == 0)
	  {
	    tcp_output(pcb);
	  }
	}
	else if (pcb->state == TIME_WAIT) 
	{
	  pbuf_free(pcb->recv_data);
	  tcp_output(pcb);
	}
      }
    }
    
    tcp_seg_free(seg);
  }
  else 
  {
    if (seg == NULL) 
    {
      stats.tcp.memerr++;
      stats.tcp.drop++;
      pbuf_free(p);
    } 
    else 
    {
      // If no matching PCB was found, send a TCP RST (reset) to the sender
      kprintf("tcp_input: no PCB match found, resetting.\n");

      if (!(TCPH_FLAGS(tcphdr) & TCP_RST)) 
      {
	stats.tcp.proterr++;
	stats.tcp.drop++;

	tcp_rst(tcphdr->ackno, tcphdr->seqno + p->tot_len +
		((TCPH_FLAGS(tcphdr) & TCP_FIN || TCPH_FLAGS(tcphdr) & TCP_SYN) ? 1: 0),
		&(iphdr->dest), &(iphdr->src),
		tcphdr->dest, tcphdr->src);
      }

      kfree(seg);
      pbuf_free(p);
    }
  }
}

//
// tcp_process
//
// Implements the TCP state machine. Called by tcp_input. In some
// states tcp_receive() is called to receive data. The tcp_seg
// argument will be freed by the caller (tcp_input()) unless the
// recv_data pointer in the pcb is set.
//

static err_t tcp_process(struct tcp_seg *seg, struct tcp_pcb *pcb)
{
  struct tcp_pcb *npcb;
  struct ip_hdr *iphdr;
  struct tcp_hdr *tcphdr;
  unsigned long seqno, ackno;
  int flags;
  unsigned long optdata;
  struct tcp_seg *rseg;
  int acceptable = 0;
  
  iphdr = (struct ip_hdr *) ((char *) seg->tcphdr - IP_HLEN);
  tcphdr = seg->tcphdr;
  flags = TCPH_FLAGS(tcphdr);
  seqno = tcphdr->seqno;
  ackno = tcphdr->ackno;
  
  // Process incoming RST segments
  if (flags & TCP_RST) 
  {
    // First, determine if the reset is acceptable
    if (pcb->state != LISTEN) 
    {
      if (pcb->state == SYN_SENT) 
      {
	if (ackno == pcb->snd_nxt) 
	{
	  acceptable = 1;
	}
      } 
      else 
      {
	if (TCP_SEQ_GEQ(seqno, pcb->rcv_nxt) && TCP_SEQ_LEQ(seqno, pcb->rcv_nxt + pcb->rcv_wnd)) 
	{
	  acceptable = 1;
	}
      }
    }

    if (acceptable) 
    {
      kprintf("tcp_process: Connection RESET\n");
      pcb->flags |= TF_RESET;
      pcb->flags &= ~TF_ACK_NEXT;
    } 
    else 
    {
      kprintf("tcp_process: unacceptable reset seqno %lu rcv_nxt %lu\n", seqno, pcb->rcv_nxt);
    }

    return -ERST;
  }

  // Update the PCB timer unless we are in the LISTEN state, in
  // which case we don't even have memory allocated for the timer,
  // much less use it
  if (pcb->state != LISTEN) pcb->tmr = tcp_ticks;
  
  // Do different things depending on the TCP state
  switch (pcb->state) 
  {
    case CLOSED:
      // Do nothing in the CLOSED state. In fact, this case should never occur
      // since PCBs in the CLOSED state are never found in the list of active PCBs
      break;

    case LISTEN:
      // In the LISTEN state, we check for incoming SYN segments,
      // creates a new PCB, and responds with a SYN|ACK
      if (flags & TCP_ACK) 
      {
	// For incoming segments with the ACK flag set, respond with a RST
	kprintf("tcp_process: ACK in LISTEN, sending reset\n");
	tcp_rst(ackno + 1, seqno + seg->len, &(iphdr->dest), &(iphdr->src), tcphdr->dest, tcphdr->src);
      } 
      else if (flags & TCP_SYN) 
      {
	kprintf("TCP connection request %d -> %d.\n", seg->tcphdr->src, seg->tcphdr->dest);
	npcb = tcp_new();

	// If a new PCB could not be created (probably due to lack of memory),
	// we don't do anything, but rely on the sender will retransmit the
	// SYN at a time when we have more memory avaliable
	if (npcb == NULL) 
	{
	  stats.tcp.memerr++;
	  break;
	}
	
	// Set up the new PCB
	ip_addr_set(&(npcb->local_ip), &(iphdr->dest));
	npcb->local_port = pcb->local_port;
	ip_addr_set(&(npcb->dest_ip), &(iphdr->src));
	npcb->dest_port = tcphdr->src;
	npcb->state = SYN_RCVD;
	npcb->rcv_nxt = seqno + 1;
	npcb->snd_wnd = tcphdr->wnd;
	npcb->ssthresh = npcb->snd_wnd;
	npcb->snd_wl1 = tcphdr->seqno;
	npcb->accept = pcb->accept;
	npcb->callback_arg = pcb->callback_arg;

	// Register the new PCB so that we can begin receiving segments for it
	TCP_REG(&tcp_active_pcbs, npcb);
      
	// Build an MSS option
	optdata = HTONL((2 << 24) | (4 << 16) | ((npcb->mss / 256) << 8) | (npcb->mss & 255));

	// Send a SYN|ACK together with the MSS option
	tcp_enqueue(npcb, NULL, 0, TCP_SYN | TCP_ACK, 0, (unsigned char *) &optdata, 4);
	return tcp_output(npcb);
      }  
      break;

    case SYN_SENT:
      kprintf("SYN-SENT: ackno %lu pcb->snd_nxt %lu unacked %lu\n", ackno, pcb->snd_nxt, ntohl(pcb->unacked->tcphdr->seqno));
      if (flags & TCP_ACK &&
	  flags & TCP_SYN &&
	  ackno == ntohl(pcb->unacked->tcphdr->seqno) + 1) 
      {
	pcb->rcv_nxt = seqno + 1;
	pcb->rcv_wnd = tcphdr->wnd;
	pcb->state = ESTABLISHED;
	pcb->cwnd = pcb->mss;
	rseg = pcb->unacked;
	pcb->unacked = rseg->next;
	tcp_seg_free(rseg);

	// Call the user specified function to call when sucessfully connected
	if (pcb->connected != NULL) 
	{
	  pcb->connected(pcb->callback_arg, pcb, 0);
	}
	tcp_ack(pcb);
      }    
      break;

    case SYN_RCVD:
      if (flags & TCP_ACK && !(flags & TCP_RST)) 
      {
	if (TCP_SEQ_LT(pcb->lastack, ackno) && TCP_SEQ_LEQ(ackno, pcb->snd_nxt)) 
	{
	  pcb->state = ESTABLISHED;
	  kprintf("TCP connection established %d -> %d.\n", seg->tcphdr->src, seg->tcphdr->dest);

	  // Call the accept function
	  if (pcb->accept != NULL) 
	  {
	    if (pcb->accept(pcb->callback_arg, pcb, 0) != 0) 
	    {
	      // If the accept function returns with an error, we abort the connection
	      tcp_abort(pcb);
	      break;
	    }
	  } 
	  else 
	  {
	    // If a PCB does not have an accept function (i.e., no
	    // application is connected to it), the connection would
	    // linger in memory until the connection reset by the remote
	    // peer (which might never happen). Therefore, we abort the
	    // connection before it is too late
	    tcp_abort(pcb);
	    break;
	  }

	  // If there was any data contained within this ACK,
	  // we'd better pass it on to the application as well
	  tcp_receive(seg, pcb);
	  pcb->cwnd = pcb->mss;
	}	
      }  
      break;

    case CLOSE_WAIT:
    case ESTABLISHED:
      tcp_receive(seg, pcb);
      if (flags & TCP_FIN)
      {
	tcp_ack_now(pcb);
	pcb->state = CLOSE_WAIT;
      }
      break;

    case FIN_WAIT_1:
      tcp_receive(seg, pcb);
      if (flags & TCP_FIN) 
      {
	if (flags & TCP_ACK && ackno == pcb->snd_nxt) 
	{
	  kprintf("TCP connection closed %d -> %d.\n", seg->tcphdr->src, seg->tcphdr->dest);
	  tcp_ack_now(pcb);
	  tcp_pcb_purge(pcb);
	  TCP_RMV(&tcp_active_pcbs, pcb);
	  pcb->state = TIME_WAIT;
	  //pcb = memp_realloc(MEMP_TCP_PCB, MEMP_TCP_PCB_TW, pcb);
	  TCP_REG(&tcp_tw_pcbs, pcb);
	} 
	else 
	{
	  tcp_ack_now(pcb);
	  pcb->state = CLOSING;
	}
      } 
      else if (flags & TCP_ACK && ackno == pcb->snd_nxt) 
      {
	pcb->state = FIN_WAIT_2;
      }
      break;

    case FIN_WAIT_2:
      tcp_receive(seg, pcb);
      if (flags & TCP_FIN) 
      {
	kprintf("TCP connection closed %d -> %d.\n", seg->tcphdr->src, seg->tcphdr->dest);
	tcp_ack_now(pcb);
	tcp_pcb_purge(pcb);
	TCP_RMV(&tcp_active_pcbs, pcb);
	//pcb = memp_realloc(MEMP_TCP_PCB, MEMP_TCP_PCB_TW, pcb);
	pcb->state = TIME_WAIT;
	TCP_REG(&tcp_tw_pcbs, pcb);
      }
      break;

    case CLOSING:
      tcp_receive(seg, pcb);
      if (flags & TCP_ACK && ackno == pcb->snd_nxt) 
      {
	kprintf("TCP connection closed %d -> %d.\n", seg->tcphdr->src, seg->tcphdr->dest);
	tcp_ack_now(pcb);
	tcp_pcb_purge(pcb);
	TCP_RMV(&tcp_active_pcbs, pcb);
	//pcb = memp_realloc(MEMP_TCP_PCB, MEMP_TCP_PCB_TW, pcb);
	pcb->state = TIME_WAIT;
	TCP_REG(&tcp_tw_pcbs, pcb);
      }
      break;

    case LAST_ACK:
      tcp_receive(seg, pcb);
      if (flags & TCP_ACK && ackno == pcb->snd_nxt) 
      {
	kprintf("TCP connection closed %d -> %d.\n", seg->tcphdr->src, seg->tcphdr->dest);
	pcb->state = CLOSED;
	pcb->flags |= TF_CLOSED;
      }
      break;

    case TIME_WAIT:
      if (TCP_SEQ_GT(seqno + seg->len, pcb->rcv_nxt)) pcb->rcv_nxt = seqno + seg->len;
      tcp_ack_now(pcb);
      break;
  }
  
  return 0;
}

//
// tcp_receive
//
// Called by tcp_process. Checks if the given segment is an ACK for outstanding
// data, and if so frees the memory of the buffered data. Next, is places the
// segment on any of the receive queues (pcb->recved or pcb->ooseq). If the segment
// is buffered, the pbuf is referenced by pbuf_ref so that it will not be freed until
// it has been removed from the buffer.
//
// If the incoming segment constitutes an ACK for a segment that was used for RTT
// estimation, the RTT is estimated here as well.
//

static void tcp_receive(struct tcp_seg *seg, struct tcp_pcb *pcb)
{
  struct tcp_seg *next, *prev, *cseg;
  struct pbuf *p;
  unsigned long ackno, seqno;
  int off;
  int m;

  ackno = seg->tcphdr->ackno;
  seqno = seg->tcphdr->seqno;

  if (TCPH_FLAGS(seg->tcphdr) & TCP_ACK) 
  {
    // Update window
    if (TCP_SEQ_LT(pcb->snd_wl1, seqno) ||
        (pcb->snd_wl1 == seqno && TCP_SEQ_LT(pcb->snd_wl2, ackno)) ||
        (pcb->snd_wl2 == ackno && seg->tcphdr->wnd > pcb->snd_wnd)) 
    {
      pcb->snd_wnd = seg->tcphdr->wnd;
      pcb->snd_wl1 = seqno;
      pcb->snd_wl2 = ackno;

      kprintf("tcp_receive: window update %lu\n", pcb->snd_wnd);
    }

    if (pcb->lastack == ackno) 
    {
      pcb->dupacks++;
      if (pcb->dupacks >= 3 && pcb->unacked != NULL) 
      {
        if (!(pcb->flags & TF_INFR)) 
	{
          // This is fast retransmit. Retransmit the first unacked segment
          kprintf("tcp_receive: dupacks %d (%lu), fast retransmit %lu\n", pcb->dupacks, pcb->lastack, ntohl(pcb->unacked->tcphdr->seqno));
          tcp_rexmit_seg(pcb, pcb->unacked);

          // Set ssthresh to MAX(FlightSize / 2, 2 * SMSS)
          pcb->ssthresh = UMAX((unsigned long) (pcb->snd_max - pcb->lastack) / 2, (unsigned long) (2 * pcb->mss));
          pcb->cwnd = pcb->ssthresh + 3 * pcb->mss;
          pcb->flags |= TF_INFR;
        } 
	else 
	{         
          pcb->cwnd += pcb->mss;
        }
      }
    } 
    else if (TCP_SEQ_LT(pcb->lastack, ackno) && TCP_SEQ_LEQ(ackno, pcb->snd_max)) 
    {
      // We come here when the ACK acknowledges new data

      // Reset the "IN Fast Retransmit" flag, since we are no longer
      // in fast retransmit. Also reset the congestion window to the
      // slow start threshold
      if (pcb->flags & TF_INFR) 
      {
	pcb->flags &= ~TF_INFR;
	pcb->cwnd = pcb->ssthresh;
      }

      // Reset the number of retransmissions
      pcb->nrtx = 0;

      // Reset the retransmission time-out
      pcb->rto = (pcb->sa >> 3) + pcb->sv;
      
      // Update the send buffer space
      pcb->acked = (unsigned short) (ackno - pcb->lastack);
      pcb->snd_buf += pcb->acked;

      // Reset the fast retransmit variables
      pcb->dupacks = 0;
      pcb->lastack = ackno;
      
      // Update the congestion control variables (cwnd and ssthresh)
      if (pcb->state >= ESTABLISHED) 
      {
        if (pcb->cwnd < pcb->ssthresh) 
	{
          pcb->cwnd += pcb->mss;
          kprintf("tcp_receive: slow start cwnd %u\n", pcb->cwnd);
        }
	else 
	{
          pcb->cwnd += pcb->mss * pcb->mss / pcb->cwnd;
          kprintf("tcp_receive: congestion avoidance cwnd %u\n", pcb->cwnd);
        }
      }

      kprintf("tcp_receive: ACK for %lu, unacked->seqno %lu:%lu\n",
	       ackno,
	       pcb->unacked != NULL?
	       ntohl(pcb->unacked->tcphdr->seqno): 0,
	       pcb->unacked != NULL?
	       ntohl(pcb->unacked->tcphdr->seqno) + pcb->unacked->len: 0);

      // We go through the unsent list to see if any of the segments
      // on the list are acknowledged by the ACK. This may seem
      // strange since an "unsent" segment shouldn't be acked. The
      // rationale is that we put all outstanding segments on the
      // unsent list after a retransmission, so these segments may
      // in fact be sent once
      while (pcb->unsent != NULL && TCP_SEQ_LEQ(ntohl(pcb->unsent->tcphdr->seqno) + pcb->unsent->len, ackno)) 
      {
	kprintf("tcp_receive: removing %lu:%lu from pcb->unsent\n",
		   ntohl(pcb->unsent->tcphdr->seqno),
		   ntohl(pcb->unsent->tcphdr->seqno) +
		   pcb->unsent->len);

	next = pcb->unsent->next;
	pcb->snd_queuelen -= tcp_seg_free(pcb->unsent);
	pcb->unsent = next;
	
        if (pcb->unsent != NULL) 
	{
          pcb->snd_nxt = htonl(pcb->unsent->tcphdr->seqno);
        }
      }
      
      // Remove segment from the unacknowledged list if the incoming ACK acknowlegdes them
      while (pcb->unacked != NULL && TCP_SEQ_LEQ(ntohl(pcb->unacked->tcphdr->seqno) + pcb->unacked->len, ackno)) 
      {
	kprintf("tcp_receive: removing %lu:%lu from pcb->unacked\n",
		   ntohl(pcb->unacked->tcphdr->seqno),
		   ntohl(pcb->unacked->tcphdr->seqno) +
		   pcb->unacked->len);

	next = pcb->unacked->next;
	pcb->snd_queuelen -= tcp_seg_free(pcb->unacked);
	pcb->unacked = next;
      }

      pcb->polltmr = 0;
    }
    // End of ACK for new data processing
    
    kprintf("tcp_receive: pcb->rttest %d rtseq %lu ackno %lu\n", pcb->rttest, pcb->rtseq, ackno);
    
    // RTT estimation calculations. This is done by checking if the
    // incoming segment acknowledges the segment we use to take a
    // round-trip time measurement
    if (pcb->rttest && TCP_SEQ_LT(pcb->rtseq, ackno)) 
    {
      m = tcp_ticks - pcb->rttest;

      kprintf("tcp_receive: experienced rtt %d ticks (%d msec).\n", m, m * TCP_SLOW_INTERVAL);

      // This is taken directly from VJs original code in his paper
      m = m - (pcb->sa >> 3);
      pcb->sa += m;
      if (m < 0) m = -m;
      m = m - (pcb->sv >> 2);
      pcb->sv += m;
      pcb->rto = (pcb->sa >> 3) + pcb->sv;
      
      kprintf("tcp_receive: RTO %d (%d miliseconds)\n", pcb->rto, pcb->rto * TCP_SLOW_INTERVAL);

      pcb->rttest = 0;
    } 
  }
  
  // If the incoming segment contains data, we must process it further
  if (seg->len > 0) 
  {
    // This code basically does three things:
    //
    // +) If the incoming segment contains data that is the next
    //    in-sequence data, this data is passed to the application. This
    //    might involve trimming the first edge of the data. The rcv_nxt
    //    variable and the advertised window are adjusted.       
    //
    // +) If the incoming segment has data that is above the next
    //    sequence number expected (->rcv_nxt), the segment is placed on
    //    the ->ooseq queue. This is done by finding the appropriate
    //    place in the ->ooseq queue (which is ordered by sequence
    //    number) and trim the segment in both ends if needed. An
    //    immediate ACK is sent to indicate that we received an
    //    out-of-sequence segment.
    //
    // +) Finally, we check if the first segment on the ->ooseq queue
    //    now is in sequence (i.e., if rcv_nxt >= ooseq->seqno). If
    //    rcv_nxt > ooseq->seqno, we must trim the first edge of the
    //    segment on ->ooseq before we adjust rcv_nxt. The data in the
    //    segments that are now on sequence are chained onto the
    //    incoming segment so that we only need to call the application
    //    once.
    //

    // First, we check if we must trim the first edge. We have to do
    // this if the sequence number of the incoming segment is less
    // than rcv_nxt, and the sequence number plus the length of the
    // segment is larger than rcv_nxt

    if (TCP_SEQ_LT(seqno, pcb->rcv_nxt) && TCP_SEQ_LT(pcb->rcv_nxt, seqno + seg->len)) 
    {
      // Trimming the first edge is done by pushing the payload
      // pointer in the pbuf downwards. This is somewhat tricky since
      // we do not want to discard the full contents of the pbuf up to
      // the new starting point of the data since we have to keep the
      // TCP header which is present in the first pbuf in the chain.
      //
      // What is done is really quite a nasty hack: the first pbuf in
      // the pbuf chain is pointed to by seg->p. Since we need to be
      // able to deallocate the whole pbuf, we cannot change this
      // seg->p pointer to point to any of the later pbufs in the
      // chain. Instead, we point the ->payload pointer in the first
      // pbuf to data in one of the later pbufs. We also set the
      // seg->data pointer to point to the right place. This way, the
      // ->p pointer will still point to the first pbuf, but the
      // ->p->payload pointer will point to data in another pbuf.
      //
      // After we are done with adjusting the pbuf pointers we must
      // adjust the ->data pointer in the seg and the segment
      // length.

      off = pcb->rcv_nxt - seqno;
      if (seg->p->len < off) 
      {
	p = seg->p;
	while (p->len < off) 
	{
	  off -= p->len;
	  seg->p->tot_len -= p->len;
	  p->len = 0;
	  p = p->next;
	}
	pbuf_header(p, -off);
	seg->p->tot_len -= off;
	//seg->p->payload = p->payload;
      } 
      else 
      {
	pbuf_header(seg->p, -off);
      }
      seg->dataptr = seg->p->payload;
      seg->len -= pcb->rcv_nxt - seqno;
      seg->tcphdr->seqno = seqno = pcb->rcv_nxt;
    }

    // The sequence number must be within the window (above rcv_nxt
    // and below rcv_nxt + rcv_wnd) in order to be further
    // processed.
    
    if (TCP_SEQ_GEQ(seqno, pcb->rcv_nxt) && TCP_SEQ_LT(seqno, pcb->rcv_nxt + pcb->rcv_wnd)) 
    {
      if (pcb->rcv_nxt == seqno) 
      {
	// The incoming segment is the next in sequence. We check if
        // we have to trim the end of the segment and update rcv_nxt
        // and pass the data to the application.
	if (pcb->ooseq != NULL && TCP_SEQ_LEQ(pcb->ooseq->tcphdr->seqno, seqno + seg->len)) 
	{
	  // We have to trim the second edge of the incoming segment
	  seg->len = pcb->ooseq->tcphdr->seqno - seqno;
	  pbuf_realloc(seg->p, seg->len);
	}
	
	pcb->rcv_nxt += seg->len;	
	
	// Update the receiver's (our) window
	if (pcb->rcv_wnd < seg->len)
	  pcb->rcv_wnd = 0;
	else
	  pcb->rcv_wnd -= seg->len;
	
	// If there is data in the segment, we make preparations to
	// pass this up to the application. The ->recv_data variable
	// is used for holding the pbuf that goes to the
	// application. The code for reassembling out-of-sequence data
	// chains its data on this pbuf as well.
        //
	// If the segment was a FIN, we set the TF_GOT_FIN flag that will
	// be used to indicate to the application that the remote side has
	// closed its end of the connection.

	if (seg->p->tot_len > 0) 
	{
	  pcb->recv_data = seg->p;
	  
	  // Since this pbuf now is the responsibility of the
	  // application, we delete our reference to it so that we won't
	  // (mistakingly) deallocate it. */
	  seg->p = NULL;
	}

	if (TCPH_FLAGS(seg->tcphdr) & TCP_FIN) 
	{
	  kprintf("tcp_receive: received FIN.");
	  pcb->flags |= TF_GOT_FIN;
	}
	
	// We now check if we have segments on the ->ooseq queue that
        // is now in sequence.
	while (pcb->ooseq != NULL && pcb->ooseq->tcphdr->seqno == pcb->rcv_nxt) 
	{
	  seg = pcb->ooseq;
	  seqno = pcb->ooseq->tcphdr->seqno;
	  
	  pcb->rcv_nxt += seg->len;
	  if (pcb->rcv_wnd < seg->len)
	    pcb->rcv_wnd = 0;
	  else
	    pcb->rcv_wnd -= seg->len;

	  if (seg->p->tot_len > 0) 
	  {
	    // Chain this pbuf onto the pbuf that we will pass to the application
	    pbuf_chain(pcb->recv_data, seg->p);
	    seg->p = NULL;
	  }
	  
	  if (TCPH_FLAGS(seg->tcphdr) & TCP_FIN) 
	  {
	    kprintf("tcp_receive: dequeued FIN.");
	    pcb->flags |= TF_GOT_FIN;
	  }	    

	  pcb->ooseq = seg->next;
	  tcp_seg_free(seg);
	}

	// Acknowledge the segment(s)
	tcp_ack(pcb);
      } 
      else 
      {
	// We get here if the incoming segment is out-of-sequence.
	tcp_ack_now(pcb);

	// We queue the segment on the ->ooseq queue
	if (pcb->ooseq == NULL) 
	{
	  pcb->ooseq = tcp_seg_copy(seg);
	} 
	else 
	{
	  // If the queue is not empty, we walk through the queue and
	  // try to find a place where the sequence number of the
	  // incoming segment is between the sequence numbers of the
	  // previous and the next segment on the ->ooseq queue. That is
	  // the place where we put the incoming segment. If needed, we
	  // trim the second edges of the previous and the incoming
	  // segment so that it will fit into the sequence.
	  //
	  // If the incoming segment has the same sequence number as a
	  // segment on the ->ooseq queue, we discard the segment that
	  // contains less data.

	  prev = NULL;
	  for (next = pcb->ooseq; next != NULL; next = next->next) 
	  {
	    if (seqno == next->tcphdr->seqno) 
	    {
	      // The sequence number of the incoming segment is the
              // same as the sequence number of the segment on
              // ->ooseq. We check the lengths to see which one to
              // discard.

	      if (seg->len > next->len) 
	      {
		// The incoming segment is larger than the old
                // segment. We replace the old segment with the new
                // one.
		cseg = tcp_seg_copy(seg);
		if (cseg != NULL) 
		{
		  cseg->next = next->next;
		  if (prev != NULL)
		    prev->next = cseg;
		  else
		    pcb->ooseq = cseg;
		}
		break;
	      } 
	      else 
	      {
		// Either the lenghts are the same or the incoming
                // segment was smaller than the old one; in either
                // case, we ditch the incoming segment. */
		break;
	      }
	    } 
	    else 
	    {
	      if (prev == NULL) 
	      {
		if(TCP_SEQ_LT(seqno, next->tcphdr->seqno)) 
		{
		  // The sequence number of the incoming segment is lower
		  // than the sequence number of the first segment on the
		  // queue. We put the incoming segment first on the
		  // queue.
		  
		  if (TCP_SEQ_GT(seqno + seg->len, next->tcphdr->seqno))
		  {
		    // We need to trim the incoming segment.
		    seg->len = next->tcphdr->seqno - seqno;
		    pbuf_realloc(seg->p, seg->len);
		  }
		  cseg = tcp_seg_copy(seg);
		  if (cseg != NULL)
		  {
		    cseg->next = next;
		    pcb->ooseq = cseg;
		  }
		  break;
		}
	      } 
	      else if (TCP_SEQ_LT(prev->tcphdr->seqno, seqno) && TCP_SEQ_LT(seqno, next->tcphdr->seqno)) 
	      {
		// The sequence number of the incoming segment is in
                // between the sequence numbers of the previous and
                // the next segment on ->ooseq. We trim and insert the
                // incoming segment and trim the previous segment, if
                // needed.

		if (TCP_SEQ_GT(seqno + seg->len, next->tcphdr->seqno))
		{
		  // We need to trim the incoming segment
		  seg->len = next->tcphdr->seqno - seqno;
		  pbuf_realloc(seg->p, seg->len);
		}

		cseg = tcp_seg_copy(seg);
		if (cseg != NULL) 
		{
		  cseg->next = next;
		  prev->next = cseg;
		  if (TCP_SEQ_GT(prev->tcphdr->seqno + prev->len, seqno))
		  {
		    // We need to trim the prev segment
		    prev->len = seqno - prev->tcphdr->seqno;
		    pbuf_realloc(prev->p, prev->len);
		  }
		}
		break;
	      }

	      // If the "next" segment is the last segment on the
              // ooseq queue, we add the incoming segment to the end
              // of the list.
	      if (next->next == NULL && TCP_SEQ_GT(seqno, next->tcphdr->seqno)) 
	      {
		next->next = tcp_seg_copy(seg);
		if (next->next != NULL) 
		{
		  if (TCP_SEQ_GT(next->tcphdr->seqno + next->len, seqno))
		  {
		    // We need to trim the last segment
		    next->len = seqno - next->tcphdr->seqno;
		    pbuf_realloc(next->p, next->len);
		  }
		}
		break;
	      }
	    }
	    prev = next;
	  }    
	} 
      }
    }
  }
}
