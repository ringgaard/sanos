//
// stats.h
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Network statistics
//

#ifndef STATS_H
#define STATS_H

struct stats_proto 
{
  unsigned long xmit;    // Transmitted packets
  unsigned long rexmit;  // Retransmitted packets
  unsigned long recv;    // Received packets
  unsigned long fw;      // Forwarded packets
  unsigned long drop;    // Dropped packets
  unsigned long chkerr;  // Checksum error
  unsigned long lenerr;  // Invalid length error
  unsigned long memerr;  // Out of memory error
  unsigned long rterr;   // Routing error
  unsigned long proterr; // Protocol error
  unsigned long opterr;  // Error in options
  unsigned long err;     // Misc error
};

struct stats_pbuf 
{
  unsigned long avail;
  unsigned long used;
  unsigned long max;  
  unsigned long err;
  unsigned long reclaimed;

  unsigned long alloc_locked;
  unsigned long refresh_locked;
};

struct stats_all
{
  struct stats_proto link;
  struct stats_proto ip;
  struct stats_proto icmp;
  struct stats_proto udp;
  struct stats_proto tcp;
  struct stats_pbuf pbuf;
};

krnlapi extern struct stats_all stats;

void stats_init();

#endif
