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

static err_t loopif_output(struct netif *netif, struct pbuf *p, struct ip_addr *ipaddr)
{
  struct pbuf *q, *r;
  char *ptr;

  r = pbuf_alloc(PBUF_RAW, p->tot_len, PBUF_RW);
  if (!r) return -ENOMEM;

  ptr = r->payload;
    
  for (q = p; q != NULL; q = q->next) 
  {
    memcpy(ptr, q->payload, q->len);
    ptr += q->len;
  }

  return netif->input(r, netif);
}

void loopif_init()
{
  struct ip_addr loip;
  struct ip_addr logw;
  struct ip_addr lomask;
  struct netif *netif;

  loip.addr = htonl(INADDR_LOOPBACK);
  lomask.addr = htonl(INADDR_BROADCAST);
  logw.addr = htonl(INADDR_ANY);

  netif = netif_add("lo", &loip, &lomask, &logw);
  if (!netif) return;

  netif->output = loopif_output;
  netif->flags |= NETIF_LOOPBACK;
}
