//
// dhcp.c
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Dynamic Host Configuration Protocol (DHCP)
//

#include <net/net.h>

static char *dhcp_lookup_option_buffer(unsigned char *buffer, int size, int option, int *len, unsigned char *overload)
{
  unsigned char *end = buffer + size;
  int code;
  int length;

  if (overload) *overload = 0;

  while (buffer < end)
  {
    code = *buffer++;
    
    // Stop if End option reached
    if (code == DHO_END) break;

    // Pad options don't have a length - just skip them
    if (code == DHO_PAD) continue;

    // Don't look for length if the buffer isn't that big
    if (buffer == end) break;
    
    length = *buffer++;

    // If the length is outrageous, the options are bad
    if (buffer + length > end) break;

    // Check for Overload option
    if (overload && code == DHO_DHCP_OPTION_OVERLOAD) *overload = *buffer;

    // If this the option we are lookiing for then return
    if (code == option)
    {
      if (len) *len = length;
      return buffer;
    }

    // Move to next option
    buffer += length;
  }

  return NULL;
}

char *dhcp_lookup_option(struct dhcp_packet *packet, int option, int *len)
{
  char *opt;
  unsigned char overload;

  // Check for dhcp packet
  if (!packet) return NULL;
  if (!packet->hdr) return NULL;

  // If we don't see the magic cookie, there's nothing to parse
  if (memcmp(packet->hdr->options, DHCP_OPTIONS_COOKIE, 4) != 0) return NULL;

  // Go through the options field, up to the end of the packet or the End field
  opt = dhcp_lookup_option_buffer(packet->hdr->options + 4, packet->opt_len - 4, option, len, &overload);
  if (opt) return opt;

  // If we parsed a DHCP Option Overload option, parse more options out of the buffer(s) containing them
  if (overload & 1)
  {
    opt = dhcp_lookup_option_buffer((unsigned char *) (packet->hdr->file), DHCP_FILE_LEN, option, len, NULL);
    if (opt) return opt;
  }

  if (overload & 2)
  {
    opt = dhcp_lookup_option_buffer((unsigned char *) (packet->hdr->sname), DHCP_SNAME_LEN, option, len, NULL);
    if (opt) return opt;
  }

  return NULL;
}

void dhcp_add_option(struct dhcp_packet *packet, int option, void *data, int len)
{
  unsigned char *buffer;

  // Find start of options
  buffer = packet->hdr->options + packet->opt_len;

  // Add magic cookie if first option
  if (packet->opt_len == 0)
  {
    memcpy(buffer, DHCP_OPTIONS_COOKIE, 4);
    buffer += 4;
    packet->opt_len = 4;
  }

  // Add option
  *buffer++ = (unsigned char) option;
  *buffer++ = (unsigned char) len;
  memcpy(buffer, data, len);
  buffer += len;
  packet->opt_len += 2 + len;

  // Add End option
  *buffer = DHO_END;
}

int dhcp_init_packet(struct dhcp_packet *packet, struct netif *netif, struct pbuf *p)
{
  packet->netif = netif;
  if (p)
  {
    packet->p = p;
    packet->hdr = p->payload;
    packet->opt_len = p->len - sizeof(struct dhcp_hdr);
  }
  else
  {
    packet->p = pbuf_alloc(PBUF_TRANSPORT, netif->mtu, PBUF_RW);
    if (!packet->p) return -ENOMEM;
    packet->hdr = p->payload;
    packet->opt_len = 0;
  }

  return 0;
}
