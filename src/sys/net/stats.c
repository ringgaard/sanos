//
// stats.c
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Network statistics
//

#include <net/net.h>

struct stats_all stats;

static void protstat(struct proc_file *pf, char *prot, struct stats_proto *stat)
{
  pprintf(pf, "%-6s%6d%6d%6d%6d%6d%6d%6d%6d%6d%6d%6d%6d\n",
          prot, stat->xmit, stat->rexmit, stat->recv, stat->fw, stat->drop,
	  stat->chkerr, stat->lenerr, stat->memerr, stat->rterr, stat->proterr,
	  stat->opterr, stat->err);
}

static int netstat_proc(struct proc_file *pf, void *arg)
{
  pprintf(pf, "                                     ----------------- errors ----------------\n");
  pprintf(pf, "        xmit rexmt  recv forwd  drop cksum   len   mem route proto   opt  misc\n");
  pprintf(pf, "------ ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----\n");

  protstat(pf, "link", &stats.link);
  protstat(pf, "ip", &stats.ip);
  protstat(pf, "icmp", &stats.icmp);
  protstat(pf, "udp", &stats.udp);
  protstat(pf, "tcp", &stats.tcp);

  return 0;
}

static int pbufs_proc(struct proc_file *pf, void *arg)
{
  pprintf(pf, "Pool Available .. : %6d\n", stats.pbuf.avail);
  pprintf(pf, "Pool Used ....... : %6d\n", stats.pbuf.used);
  pprintf(pf, "Pool Max Used ... : %6d\n", stats.pbuf.max);
  pprintf(pf, "Errors .......... : %6d\n", stats.pbuf.err);
  pprintf(pf, "Reclaimed ....... : %6d\n", stats.pbuf.reclaimed);
  pprintf(pf, "R/W Allocated ... : %6d\n", stats.pbuf.rwbufs);

  return 0;
}

void stats_init()
{
  //memset(&stats, 0, sizeof(struct stats_all));
  
  //register_proc_inode("pbufs", pbufs_proc, NULL);
  register_proc_inode("netstat", netstat_proc, NULL);
}
