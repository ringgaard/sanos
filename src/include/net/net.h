//
// net.h
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Network
//

#ifndef NET_H
#define NET_H

#ifndef KRNL_H
#include <os/krnl.h>
#endif

#include <net/stats.h>
#include <net/opt.h>
#include <net/ether.h>
#include <net/ipaddr.h>
#include <net/inet.h>
#include <net/netif.h>
#include <net/pbuf.h>
#include <net/arp.h>
#include <net/icmp.h>
#include <net/ip.h>
#include <net/udp.h>
#include <net/tcp.h>
#include <net/socket.h>

// loopif.c

void loopif_init(struct netif *netif);

#endif
