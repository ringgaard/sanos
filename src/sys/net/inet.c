//
// inet.c
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Functions common to all TCP/IP modules, such as the Internet checksum and the
// byte order functions.
//

#include <net/net.h>

//
// chksum
//
// Sums up all 16 bit words in a memory portion. Also includes any odd byte.
// This function is used by the other checksum functions.
//
// For now, this is not optimized. Must be optimized for the particular processor
// arcitecture on which it is to run. Preferebly coded in assembler.
//

static unsigned long chksum(void *dataptr, int len)
{
  unsigned long acc;
    
  for (acc = 0; len > 1; len -= 2)  acc += *((unsigned short *) dataptr)++;

  // Add up any odd byte
  if (len == 1) acc += htons((unsigned short) ((*(unsigned char *) dataptr) & 0xFF) << 8);

  return acc;
}

//
// inet_chksum_pseudo
//
// Calculates the pseudo Internet checksum used by TCP and UDP for a pbuf chain.
//

unsigned short inet_chksum_pseudo(struct pbuf *p, struct ip_addr *src, struct ip_addr *dest, unsigned char proto, int proto_len)
{
  unsigned long acc;
  struct pbuf *q;
  int swapped;

  acc = 0;
  swapped = 0;
  for (q = p; q != NULL; q = q->next) 
  {
    acc += chksum(q->payload, q->len);
    while (acc >> 16) acc = (acc & 0xFFFF) + (acc >> 16);
    if (q->len % 2 != 0) 
    {
      swapped = 1 - swapped;
      acc = ((acc & 0xFF) << 8) | ((acc & 0xff00) >> 8);
    }
  }

  if (swapped) acc = ((acc & 0xFF) << 8) | ((acc & 0xFF00) >> 8);
  acc += (src->addr & 0xFFFF);
  acc += ((src->addr >> 16) & 0xFFFF);
  acc += (dest->addr & 0xFFFF);
  acc += ((dest->addr >> 16) & 0xFFFF);
  acc += (unsigned long) htons((unsigned short) proto);
  acc += (unsigned long) htons((unsigned short) proto_len);
  
  while (acc >> 16) acc = (acc & 0xFFFF) + (acc >> 16);
  return (unsigned short) ~(acc & 0xFFFF);
}

//
// inet_chksum
//
// Calculates the Internet checksum over a portion of memory. Used primarely for IP
// and ICMP.
//

unsigned short inet_chksum(void *dataptr, int len)
{
  unsigned long acc;

  acc = chksum(dataptr, len);
  while (acc >> 16) acc = (acc & 0xFFFF) + (acc >> 16);
  return (unsigned short) ~(acc & 0xFFFF);
}

unsigned short inet_chksum_pbuf(struct pbuf *p)
{
  unsigned long acc;
  struct pbuf *q;
  int swapped;
  
  acc = 0;
  swapped = 0;
  for (q = p; q != NULL; q = q->next) 
  {
    acc += chksum(q->payload, q->len);
    while (acc >> 16) acc = (acc & 0xFFFF) + (acc >> 16);
    if (q->len % 2 != 0) 
    {
      swapped = 1 - swapped;
      acc = (acc & 0xFF << 8) | (acc & 0xFF00 >> 8);
    }
  }
 
  if (swapped) acc = ((acc & 0xFF) << 8) | ((acc & 0xFF00) >> 8);
  return (unsigned short) ~(acc & 0xFFFF);
}
