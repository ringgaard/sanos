//
// resolv.h
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// DNS resolver
//

#ifndef RESOLV_H
#define RESOLV_H

//
// DNS opcodes
//

#define DNS_OP_QUERY    0	// Standard query
#define DNS_OP_IQUERY   1	// Inverse query (deprecated/unsupported)
#define DNS_OP_STATUS   2	// Name server status query (unsupported)
#define DNS_OP_NOTIFY   4	// Zone change notification
#define DNS_OP_UPDATE   5	// Zone update message

//
// DNS response codes
//

#define DNS_ERR_NOERROR   0	// No error occurred
#define DNS_ERR_FORMERR   1	// Format error
#define DNS_ERR_SERVFAIL  2	// Server failure
#define DNS_ERR_NXDOMAIN  3	// Name error
#define DNS_ERR_NOTIMPL   4	// Unimplemented
#define DNS_ERR_REFUSED   5	// Operation refused
#define DNS_ERR_YXDOMAIN  6	// Name exists
#define DNS_ERR_YXRRSET   7	// RRset exists
#define DNS_ERR_NXRRSET   8	// RRset does not exist
#define DNS_ERR_NOTAUTH   9	// Not authoritative for zone
#define DNS_ERR_NOTZONE   10	// Zone of record different from zone section

//
// DNS resource record types
//

#define DNS_TYPE_INVALID  0	// Cookie
#define DNS_TYPE_A        1	// Host address
#define DNS_TYPE_NS       2	// Authoritative server
#define DNS_TYPE_MD       3	// Mail destination
#define DNS_TYPE_MF       4	// Mail forwarder
#define DNS_TYPE_CNAME    5	// Canonical name
#define DNS_TYPE_SOA      6	// Start of authority zone
#define DNS_TYPE_MB       7	// Mailbox domain name
#define DNS_TYPE_MG       8	// Mail group member
#define DNS_TYPE_MR       9	// Mail rename name
#define DNS_TYPE_NULL     10	// Null resource record
#define DNS_TYPE_WKS      11	// Well known service
#define DNS_TYPE_PTR      12	// Domain name pointer
#define DNS_TYPE_HINFO    13	// Host information
#define DNS_TYPE_MINFO    14	// Mailbox information
#define DNS_TYPE_MX       15	// Mail routing information
#define DNS_TYPE_TXT      16	// Text strings
#define DNS_TYPE_RP       17	// Responsible person
#define DNS_TYPE_AFSDB    18	// AFS cell database
#define DNS_TYPE_X25      19	// X_25 calling address
#define DNS_TYPE_ISDN     20	// ISDN calling address
#define DNS_TYPE_RT       21	// Router
#define DNS_TYPE_NSAP     22	// NSAP address
#define DNS_TYPE_NSAP_PTR 23	// Reverse NSAP lookup (deprecated)
#define DNS_TYPE_SIG      24	// Security signature
#define DNS_TYPE_KEY      25	// Security key
#define DNS_TYPE_PX       26	// X.400 mail mapping
#define DNS_TYPE_GPOS     27	// Geographical position (withdrawn)
#define DNS_TYPE_AAAA     28	// Ip6 Address
#define DNS_TYPE_LOC      29	// Location Information
#define DNS_TYPE_NXT      30	// Next domain (security)
#define DNS_TYPE_EID      31	// Endpoint identifier
#define DNS_TYPE_NIMLOC   32	// Nimrod Locator
#define DNS_TYPE_SRV      33	// Server Selection
#define DNS_TYPE_ATMA     34	// ATM Address
#define DNS_TYPE_NAPTR    35	// Naming Authority PoinTeR
#define DNS_TYPE_KX       36	// Key Exchange
#define DNS_TYPE_CERT     37	// Certification record
#define DNS_TYPE_A6       38	// IPv6 address (deprecates AAAA)
#define DNS_TYPE_DNAME    39	// Non-terminal DNAME (for IPv6)
#define DNS_TYPE_SINK     40	// Kitchen sink (experimentatl)
#define DNS_TYPE_OPT      41	// EDNS0 option (meta-RR)

#define DNS_TYPE_TSIG     250	// Transaction signature
#define DNS_TYPE_IXFR     251	// Incremental zone transfer
#define DNS_TYPE_AXFR     252	// Transfer zone of authority
#define DNS_TYPE_MAILB    253	// Transfer mailbox records
#define DNS_TYPE_MAILA    254	// Transfer mail agent records
#define DNS_TYPE_ANY      255	// Wildcard match

//
// DNS classes
//

#define DNS_C_INVALID	  0	// Cookie
#define DNS_C_IN	  1	// Internet
#define DNS_C_2		  2	// Unallocated/unsupported
#define DNS_C_CHAOS	  3	// MIT Chaos-net
#define DNS_C_HS	  4	// MIT Hesiod

#define DNS_C_NONE	  254	// For prereq. sections in update request
#define DNS_C_ANY	  255	// Wildcard match

//
// DNS message header
//

typedef struct dns_hdr
{
  unsigned	id : 16;	// Query identification number

  // Fields in third byte
  unsigned	rd : 1;		// Recursion desired
  unsigned	tc : 1;		// Truncated message
  unsigned	aa : 1;		// Authoritive answer
  unsigned	opcode : 4;	// Purpose of message
  unsigned	qr : 1;		// Response flag

  // Fields in fourth byte
  unsigned	rcode : 4;	// Response code
  unsigned	cd: 1;		// Checking disabled by resolver
  unsigned	ad: 1;		// Authentic data from named
  unsigned	unused : 1;	// Unused bits (MBZ as of 4.9.3a3)
  unsigned	ra : 1;		// Recursion available

  // Remaining bytes
  unsigned	qdcount : 16;	// Number of question entries
  unsigned	ancount : 16;	// Number of answer entries
  unsigned	nscount : 16;	// Number of authority entries
  unsigned	arcount : 16;	// Number of resource entries
};

int res_init(); 

int res_query(const char *dname, int class, int type, unsigned char *answer, int anslen); 

int res_search(const char *dname, int class, int type, unsigned char *answer, int anslen); 

int res_querydomain(const char *name, const char *domain, int class, int type, 
		    unsigned char *answer, int anslen); 

int res_mkquery(int op, const char *dname, int class, int type, char *data, int datalen, 
		struct rrec *newrr, char *buf, int buflen); 

int res_send(const char *msg, int msglen, char *answer, int anslen); 

int dn_comp(const char *src, unsigned char *dst, int dstsiz, unsigned char **dnptrs, unsigned char **lastdnptr);

int dn_expand(const unsigned char *msg, const unsigned char *eom, const unsigned char *src,  char *dst, int dstsiz);

#endif
