//
// stats.c
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Network statistics
//

#include <net/net.h>

struct stats_all stats;

void stats_init()
{
  memset(&stats, 0, sizeof(struct stats_all));
}
