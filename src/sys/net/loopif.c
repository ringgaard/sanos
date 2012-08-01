//
// loopif.c
//
// Loopback network interface
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

struct netif *loopback_netif;
struct queue *loopback_queue;

static err_t loopif_output(struct netif *netif, struct pbuf *p, struct ip_addr *ipaddr) {
  struct pbuf *q;

  if ((netif->flags & NETIF_UP) == 0) return -ENETDOWN;

  q = pbuf_dup(PBUF_RAW, p);
  if (!q) return -ENOMEM;

  if (enqueue(loopback_queue, q, 0) < 0) {
    kprintf("loopif: drop (queue full)\n");
    stats.link.memerr++;
    stats.link.drop++;
    return -EBUF;
  }

  pbuf_free(p);
  //yield();

  return 0;
}

void loopif_dispatcher(void *arg) {
  struct netif *netif = arg;
  struct pbuf *p;
  int rc;

  while (1) {
    p = dequeue(loopback_queue, INFINITE);
    if (!p) panic("error retrieving message from loopback packet queue\n");

    rc = netif->input(p, netif);
    if (rc < 0) pbuf_free(p);
  }
}

void loopif_init() {
  struct ip_addr loip;
  struct ip_addr logw;
  struct ip_addr lomask;

  loip.addr = htonl(INADDR_LOOPBACK);
  lomask.addr = htonl(0xFF000000);
  logw.addr = htonl(INADDR_ANY);

  loopback_netif = netif_add("lo", &loip, &lomask, &logw);
  if (!loopback_netif) return;

  loopback_netif->output = loopif_output;
  loopback_netif->flags |= NETIF_LOOPBACK | NETIF_UP |
                           NETIF_IP_TX_CHECKSUM_OFFLOAD | NETIF_IP_RX_CHECKSUM_OFFLOAD |
                           NETIF_UDP_RX_CHECKSUM_OFFLOAD | NETIF_UDP_TX_CHECKSUM_OFFLOAD |
                           NETIF_TCP_RX_CHECKSUM_OFFLOAD | NETIF_TCP_TX_CHECKSUM_OFFLOAD;

  loopback_queue = alloc_queue(256);
  create_kernel_thread(loopif_dispatcher, loopback_netif, PRIORITY_NORMAL, "loopback");
}
