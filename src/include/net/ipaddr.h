//
// ipaddr.h
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// IP address functions
//

#ifndef IPADDR_H
#define IPADDR_H

#define IP_ADDR_ANY 0

#define IP_ADDR_BROADCAST (&ip_addr_broadcast)

struct ip_addr 
{
  unsigned long addr;
};

extern struct ip_addr ip_addr_broadcast;

#define IP4_ADDR(ipaddr, a,b,c,d) (ipaddr)->addr = htonl(((unsigned long)(a & 0xFF) << 24) | ((unsigned long)(b & 0xFF) << 16) | ((unsigned long)(c & 0xFF) << 8) | (unsigned long)(d & 0xFF))

#define ip_addr_set(dest, src) (dest)->addr = ((src) == IP_ADDR_ANY ? IP_ADDR_ANY: ((struct ip_addr *) src)->addr)

#define ip_addr_maskcmp(addr1, addr2, mask) (((addr1)->addr & \
                                              (mask)->addr) == \
                                             ((addr2)->addr & \
                                              (mask)->addr))

#define ip_addr_cmp(addr1, addr2) ((addr1)->addr == (addr2)->addr)

#define ip_addr_isany(addr1) ((addr1) == NULL || (addr1)->addr == 0)

#define ip_addr_isbroadcast(addr1, mask) (((((addr1)->addr) & ~((mask)->addr)) == \
					 (0xFFFFFFFF & ~((mask)->addr))) || \
                                         ((addr1)->addr == 0xFFFFFFFF) || \
                                         ((addr1)->addr == 0x00000000))

#define ip_addr_ismulticast(addr1) (((addr1)->addr & NTOHL(0xF0000000)) == NTOHL(0xE0000000))
				   
#define ip_addr_debug_print(ipaddr)  kprintf("%d.%d.%d.%d", \
		    (ntohl((ipaddr)->addr) >> 24) & 0xFF, \
		    (ntohl((ipaddr)->addr) >> 16) & 0xFF, \
		    (ntohl((ipaddr)->addr) >> 8) & 0xFF, \
		    ntohl((ipaddr)->addr) & 0xFF)

#define ip4_addr1(ipaddr) ((unsigned char)(ntohl((ipaddr)->addr) >> 24) & 0xFF)
#define ip4_addr2(ipaddr) ((unsigned char)(ntohl((ipaddr)->addr) >> 16) & 0xFF)
#define ip4_addr3(ipaddr) ((unsigned char)(ntohl((ipaddr)->addr) >> 8) & 0xFF)
#define ip4_addr4(ipaddr) ((unsigned char)(ntohl((ipaddr)->addr)) & 0xFF)

#endif
