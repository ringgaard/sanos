//
// raw.h
//
// Raw network protocol interface
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

#ifndef RAW_H
#define RAW_H

struct raw_pcb {
  struct ip_addr local_ip;
  struct ip_addr remote_ip;
  int ttl;
  struct raw_pcb *next;
  unsigned short protocol;
  void *recv_arg;

  err_t (*recv)(void *arg, struct raw_pcb *pcb, struct pbuf *p, struct ip_addr *addr);
};

// Application layer interface to the RAW code

struct raw_pcb *raw_new(unsigned short proto);
void raw_remove(struct raw_pcb *pcb);
err_t raw_bind(struct raw_pcb *pcb, struct ip_addr *ipaddr);
err_t raw_connect(struct raw_pcb *pcb, struct ip_addr *ipaddr);
err_t raw_recv(struct raw_pcb *pcb, err_t (*recv)(void *arg, struct raw_pcb *pcb, struct pbuf *p, struct ip_addr *addr), void *recv_arg);
err_t raw_sendto(struct raw_pcb *pcb, struct pbuf *p, struct ip_addr *ipaddr);
err_t raw_send(struct raw_pcb *pcb, struct pbuf *p);

// Lower layer interface to RAW

err_t raw_input(struct pbuf *p, struct netif *inp);
void raw_init();

#endif
