//
// stats.c
//
// Network statistics
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

struct netstats stats;

static void protstat(struct proc_file *pf, char *prot, struct stats_proto *stat) {
  pprintf(pf, "%-4s%7d%6d%7d%6d%6d%6d%6d%6d%6d%6d%6d%6d\n",
          prot, stat->xmit, stat->rexmit, stat->recv, stat->fw, stat->drop,
          stat->chkerr, stat->lenerr, stat->memerr, stat->rterr, stat->proterr,
          stat->opterr, stat->err);
}

static int netstat_proc(struct proc_file *pf, void *arg) {
  pprintf(pf, "     -------------- packets -------- ----------------- errors ----------------\n");
  pprintf(pf, "       xmit rexmt   recv forwd  drop cksum   len   mem route proto   opt  misc\n");
  pprintf(pf, "---- ------ ----- ------ ----- ----- ----- ----- ----- ----- ----- ----- -----\n");

  protstat(pf, "link", &stats.link);
  protstat(pf, "ip", &stats.ip);
  protstat(pf, "icmp", &stats.icmp);
  protstat(pf, "udp", &stats.udp);
  protstat(pf, "tcp", &stats.tcp);
  protstat(pf, "raw", &stats.raw);

  return 0;
}

static int pbufs_proc(struct proc_file *pf, void *arg) {
  pprintf(pf, "Pool Available .. : %6d\n", stats.pbuf.avail);
  pprintf(pf, "Pool Used ....... : %6d\n", stats.pbuf.used);
  pprintf(pf, "Pool Max Used ... : %6d\n", stats.pbuf.max);
  pprintf(pf, "Errors .......... : %6d\n", stats.pbuf.err);
  pprintf(pf, "Reclaimed ....... : %6d\n", stats.pbuf.reclaimed);
  pprintf(pf, "R/W Allocated ... : %6d\n", stats.pbuf.rwbufs);

  return 0;
}

struct netstats *get_netstats() {
  return &stats;
}

void stats_init() {
  //memset(&stats, 0, sizeof(struct stats_all));
  
  register_proc_inode("pbufs", pbufs_proc, NULL);
  register_proc_inode("netstat", netstat_proc, NULL);
}

