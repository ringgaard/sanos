//
// ip.h
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Internet Protocol (IP)
//

#ifndef IP_H
#define IP_H

void ip_init();
struct netif *ip_route(struct ip_addr *dest);
err_t ip_input(struct pbuf *p, struct netif *inp);
err_t ip_output(struct pbuf *p, struct ip_addr *src, struct ip_addr *dest, int ttl, int proto);
err_t ip_output_if(struct pbuf *p, struct ip_addr *src, struct ip_addr *dest, int ttl, int proto, struct netif *netif);

#define IP_HLEN 20

#define IP_PROTO_ICMP    1
#define IP_PROTO_UDP     17
#define IP_PROTO_UDPLITE 170
#define IP_PROTO_TCP     6

// This is passed as the destination address to ip_output_if (not
// to ip_output), meaning that an IP header already is constructed
// in the pbuf. This is used when TCP retransmits.

#define IP_HDRINCL  NULL

#define IP_RF      0x8000        // Reserved fragment flag
#define IP_DF      0x4000        // Dont fragment flag
#define IP_MF      0x2000        // More fragments flag
#define IP_OFFMASK 0x1fff        // Mask for fragmenting bits

#pragma pack(push)
#pragma pack(1)

struct ip_hdr 
{
  unsigned short _v_hl_tos;      // Version / header length / type of service
  unsigned short _len;           // Total length
  unsigned short _id;            // Identification
  unsigned short _offset;        // Fragment offset field
  unsigned short _ttl_proto;     // Time to live / protocol
  unsigned short _chksum;        // Checksum
  struct ip_addr src, dest;      // Source and destination IP addresses
};

#pragma pack(pop)

#define IPH_V(hdr)  (NTOHS((hdr)->_v_hl_tos) >> 12)
#define IPH_HL(hdr) ((NTOHS((hdr)->_v_hl_tos) >> 8) & 0x0F)
#define IPH_TOS(hdr) HTONS((NTOHS((hdr)->_v_hl_tos) & 0xFF))
#define IPH_LEN(hdr) ((hdr)->_len)
#define IPH_ID(hdr) ((hdr)->_id)
#define IPH_OFFSET(hdr) ((hdr)->_offset)
#define IPH_TTL(hdr) (NTOHS((hdr)->_ttl_proto) >> 8)
#define IPH_PROTO(hdr) (NTOHS((hdr)->_ttl_proto) & 0xFF)
#define IPH_CHKSUM(hdr) ((hdr)->_chksum)

#define IPH_VHLTOS_SET(hdr, v, hl, tos) (hdr)->_v_hl_tos = HTONS(((v) << 12) | ((hl) << 8) | (tos))
#define IPH_LEN_SET(hdr, len) (hdr)->_len = (len)
#define IPH_ID_SET(hdr, id) (hdr)->_id = (id)
#define IPH_OFFSET_SET(hdr, off) (hdr)->_offset = (off)
#define IPH_TTL_SET(hdr, ttl) (hdr)->_ttl_proto = HTONS(IPH_PROTO(hdr) | ((ttl) << 8))
#define IPH_PROTO_SET(hdr, proto) (hdr)->_ttl_proto = HTONS((proto) | (IPH_TTL(hdr) << 8))
#define IPH_CHKSUM_SET(hdr, chksum) (hdr)->_chksum = (chksum)

#endif


