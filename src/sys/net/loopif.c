//
// loopif.c
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Loopback network interface
//

#include <net/net.h>

static err_t loopif_output(struct netif *netif, struct pbuf *p, struct ip_addr *ipaddr)
{
  struct pbuf *q, *r;
  char *ptr;

  r = pbuf_alloc(PBUF_RAW, p->tot_len, PBUF_RW);
  if (r != NULL) 
  {
    ptr = r->payload;
    
    for (q = p; q != NULL; q = q->next) 
    {
      memcpy(ptr, q->payload, q->len);
      ptr += q->len;
    }

    netif->input(r, netif);
    return 0;
  }
  return -ENOMEM;
}

void loopif_init(struct netif *netif)
{
  strcpy(netif->name, "lo");
  netif->output = loopif_output;
}
