//
// tcp.h
//
// Transmission Control Protocol (TCP)
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

#ifndef TCP_H
#define TCP_H

struct tcp_pcb;

// Lower layer interface to TCP

void tcp_init();              // Must be called first to initialize TCP
void tcp_shutdown();          // Must be called before shutdown to clear connections
void tcp_slowtmr(void *arg);  // Must be called every 500 ms.
void tcp_fasttmr(void *arg);  // Must be called every 100 ms.

err_t tcp_input(struct pbuf *p, struct netif *inp); // Called by IP to deliver TCP segment to TCP

// Application layer interface to TCP

struct tcp_pcb *tcp_new();
void tcp_arg(struct tcp_pcb *pcb, void *arg);
void tcp_accept(struct tcp_pcb *pcb, err_t (*accept)(void *arg, struct tcp_pcb *newpcb, err_t err));
void tcp_recv(struct tcp_pcb *pcb, err_t (*recv)(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err));
void tcp_sent(struct tcp_pcb *pcb, err_t (*sent)(void *arg, struct tcp_pcb *tpcb, unsigned short len));
void tcp_poll(struct tcp_pcb *pcb, err_t (*poll)(void *arg, struct tcp_pcb *tpcb), int interval);
void tcp_err(struct tcp_pcb *pcb, void (*err)(void *arg, err_t err));

#define tcp_sndbuf(pcb)   ((pcb)->snd_buf)

void tcp_recved(struct tcp_pcb *pcb, int len);
err_t tcp_bind(struct tcp_pcb *pcb, struct ip_addr *ipaddr, unsigned short port);
err_t tcp_connect (struct tcp_pcb *pcb, struct ip_addr *ipaddr, unsigned short port, err_t (*connected)(void *arg, struct tcp_pcb *tpcb, err_t err));
struct tcp_pcb *tcp_listen(struct tcp_pcb *pcb);
void tcp_abort(struct tcp_pcb *pcb);
err_t tcp_close(struct tcp_pcb *pcb);
err_t tcp_write(struct tcp_pcb *pcb, const void *data, int len, int opt);

// Options for tcp_write

#define TCP_WRITE_FLUSH          0
#define TCP_WRITE_NAGLE          1
#define TCP_WRITE_NOFLUSH        2

// Used within the TCP code only

err_t tcp_output(struct tcp_pcb *pcb);

#define TCP_SEQ_LT(a,b)     ((int)((a)-(b)) < 0)
#define TCP_SEQ_LEQ(a,b)    ((int)((a)-(b)) <= 0)
#define TCP_SEQ_GT(a,b)     ((int)((a)-(b)) > 0)
#define TCP_SEQ_GEQ(a,b)    ((int)((a)-(b)) >= 0)

#define TCP_FIN 0x01
#define TCP_SYN 0x02
#define TCP_RST 0x04
#define TCP_PSH 0x08
#define TCP_ACK 0x10
#define TCP_URG 0x20

#define TCP_HLEN 20                 // Length of the TCP header, excluding options 
#define TCP_FAST_INTERVAL      200  // The fine grained timeout in milliseconds
#define TCP_SLOW_INTERVAL      500  // The coarse grained timeout in milliseconds
#define TCP_FIN_WAIT_TIMEOUT 20000  // milliseconds
#define TCP_SYN_RCVD_TIMEOUT 20000  // milliseconds

#define TCP_OOSEQ_TIMEOUT        6  // x RTO

#pragma pack(push, 1)

struct tcp_hdr {
  unsigned short src, dest;
  unsigned long seqno, ackno;
  unsigned short _offset_flags;
  unsigned short wnd;
  unsigned short chksum;
  unsigned short urgp;
};

#pragma pack(pop)

#define TCPH_OFFSET(hdr) (NTOHS((hdr)->_offset_flags) >> 8)
#define TCPH_FLAGS(hdr) (NTOHS((hdr)->_offset_flags) & 0xFF)

#define TCPH_OFFSET_SET(hdr, offset) (hdr)->_offset_flags = HTONS(((offset) << 8) | TCPH_FLAGS(hdr))
#define TCPH_FLAGS_SET(hdr, flags) (hdr)->_offset_flags = HTONS((TCPH_OFFSET(hdr) << 8) | (flags))

#define TCP_TCPLEN(seg) ((seg)->len + ((TCPH_FLAGS((seg)->tcphdr) & TCP_FIN || \
                                        TCPH_FLAGS((seg)->tcphdr) & TCP_SYN) ? 1 : 0))

enum tcp_state {
  CLOSED      = 0,
  LISTEN      = 1,
  SYN_SENT    = 2,
  SYN_RCVD    = 3,
  ESTABLISHED = 4,
  FIN_WAIT_1  = 5,
  FIN_WAIT_2  = 6,
  CLOSE_WAIT  = 7,
  CLOSING     = 8,
  LAST_ACK    = 9,
  TIME_WAIT   = 10
};

// TCP protocol control block

#define TF_ACK_DELAY 0x01   // Delayed ACK
#define TF_ACK_NOW   0x02   // Immediate ACK
#define TF_INFR      0x04   // In fast recovery
#define TF_RESET     0x08   // Connection was reset
#define TF_CLOSED    0x10   // Connection was sucessfully closed
#define TF_GOT_FIN   0x20   // Connection was closed by the remote end
#define TF_IN_RECV   0x40   // Connection is processing received segment

struct tcp_pcb {
  struct tcp_pcb *next;   // For the linked list

  enum tcp_state state;   // TCP state

  void *callback_arg;
  
  // Function to call when a listener has been connected
  err_t (*accept)(void *arg, struct tcp_pcb *newpcb, err_t err);

  struct ip_addr local_ip;
  unsigned short local_port;
  
  struct ip_addr remote_ip;
  unsigned short remote_port;
  
  // Receiver variables
  unsigned long rcv_nxt;   // Next seqno expected
  unsigned short rcv_wnd;  // Receiver window

  // Timers
  int tmr;

  // Retransmission timer
  int rtime;
  
  int mss;                 // Maximum segment size

  int flags;
  
  // RTT estimation variables
  unsigned long rttest;   // RTT estimate in 500ms ticks
  unsigned long rtseq;    // sequence number being timed
  int sa, sv;

  unsigned short rto;     // Retransmission time-out
  unsigned short nrtx;    // Number of retransmissions

  // Fast retransmit/recovery
  unsigned long lastack;  // Highest acknowledged seqno
  unsigned short dupacks;
  
  // Congestion avoidance/control variables
  unsigned long cwnd;  
  unsigned long ssthresh;

  // Sender variables
  unsigned long snd_nxt;  // Next seqno to be sent
  unsigned long snd_max;  // Highest seqno sent
  unsigned long snd_wnd;  // Sender window
  unsigned long snd_wl1;  // Sequence number of last window update
  unsigned long snd_wl2;  // Acknowlegement number of last window update
  unsigned long snd_lbb;  // Sequence number of next byte to be buffered

  unsigned short snd_buf; // Avaliable buffer space for sending
  unsigned short snd_queuelen;

  // Function to be called when more send buffer space is available
  err_t (*sent)(void *arg, struct tcp_pcb *pcb, unsigned short space);
  unsigned short acked;
  
  // Function to be called when (in-sequence) data has arrived
  err_t (*recv)(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err);
  struct pbuf *recv_data;

  // Function to be called when a connection has been set up
  err_t (*connected)(void *arg, struct tcp_pcb *pcb, err_t err);

  // Function which is called periodically.
  err_t (*poll)(void *arg, struct tcp_pcb *pcb);

  // Function to be called whenever a fatal error occurs.
  void (*errf)(void *arg, err_t err);
  
  int polltmr;
  int pollinterval;
  
  // These are ordered by sequence number:
  struct tcp_seg *unsent;   // Unsent (queued) segments
  struct tcp_seg *unacked;  // Sent but unacknowledged segments
  struct tcp_seg *ooseq;    // Received out of sequence segments
};

struct tcp_pcb_listen {
  struct tcp_pcb_listen *next;   // For the linked list
  
  enum tcp_state state;          // TCP state

  void *callback_arg;
  
  // Function to call when a listener has been connected.
  void (*accept)(void *arg, struct tcp_pcb *newpcb);

  struct ip_addr local_ip;
  unsigned short local_port;
};

// TCP segments

struct tcp_seg {
  struct tcp_seg *next;    // Used when putting segments on a queue
  struct pbuf *p;          // Buffer containing data + TCP header
  void *dataptr;           // Pointer to the TCP data in the pbuf
  int len;                 // TCP length of this segment
  struct tcp_hdr *tcphdr;  // TCP header
};

// Internal functions and global variables

void tcp_pcb_purge(struct tcp_pcb *pcb);
void tcp_pcb_remove(struct tcp_pcb **pcblist, struct tcp_pcb *pcb);

int tcp_segs_free(struct tcp_seg *seg);
int tcp_seg_free(struct tcp_seg *seg);
struct tcp_seg *tcp_seg_copy(struct tcp_seg *seg);

__inline void tcp_ack(struct tcp_pcb *pcb) {
  if (pcb->flags & TF_ACK_DELAY) {
    pcb->flags |= TF_ACK_NOW;
    tcp_output(pcb);
  } else {
    pcb->flags |= TF_ACK_DELAY;
  }
}

__inline void tcp_ack_now(struct tcp_pcb *pcb) {
  pcb->flags |= TF_ACK_NOW;
  tcp_output(pcb);
}

err_t tcp_send_ctrl(struct tcp_pcb *pcb, int flags);
err_t tcp_enqueue(struct tcp_pcb *pcb, void *data, int len, int flags, unsigned char *optdata, int optlen);

void tcp_rexmit(struct tcp_pcb *pcb);
void tcp_rst(unsigned long seqno, unsigned long ackno, struct ip_addr *local_ip, struct ip_addr *remote_ip, unsigned short local_port, unsigned short remote_port);

unsigned long tcp_next_iss();

extern unsigned long tcp_ticks;

void tcp_debug_print(struct tcp_hdr *tcphdr);
void tcp_debug_print_flags(int flags);
void tcp_debug_print_state(enum tcp_state s);
void tcp_debug_print_pcbs();

// TCP PCB lists

extern struct tcp_pcb_listen *tcp_listen_pcbs;  // List of all TCP PCBs in LISTEN state
extern struct tcp_pcb *tcp_active_pcbs;         // List of all TCP PCBs that are in a state in which they accept or send data
extern struct tcp_pcb *tcp_tw_pcbs;             // List of all TCP PCBs in TIME-WAIT

//
// Axoims about the above lists:
//   1) Every TCP PCB that is not CLOSED is in one of the lists.
//   2) A PCB is only in one of the lists.
//   3) All PCBs in the tcp_listen_pcbs list is in LISTEN state.
//   4) All PCBs in the tcp_tw_pcbs list is in TIME-WAIT state.
//

// Define two macros, TCP_REG and TCP_RMV that registers a TCP PCB
// with a PCB list or removes a PCB from a list, respectively.

#define TCP_REG(pcbs, npcb) do { \
                            npcb->next = *pcbs; \
                            *pcbs = npcb; \
                            } while (0)

#define TCP_RMV(pcbs, npcb) do { \
                            struct tcp_pcb *tcp_tmp_pcb; \
                            if (*pcbs == npcb) { \
                               *pcbs = (*pcbs)->next; \
                            } else for (tcp_tmp_pcb = *pcbs; tcp_tmp_pcb != NULL; tcp_tmp_pcb = tcp_tmp_pcb->next) { \
                               if (tcp_tmp_pcb->next != NULL && tcp_tmp_pcb->next == npcb) { \
                                  tcp_tmp_pcb->next = npcb->next; \
                                  break; \
                               } \
                            } \
                            npcb->next = NULL; \
                            } while (0)

#endif
