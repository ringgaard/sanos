//
// resolv.h
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// DNS resolver
//

#ifndef RESOLV_H
#define RESOLV_H

typedef struct dns_hdr
{
  unsigned	id :16;		// Query identification number

  // Fields in third byte
  unsigned	rd :1;		// Recursion desired
  unsigned	tc :1;		// Truncated message
  unsigned	aa :1;		// Authoritive answer
  unsigned	opcode :4;	// Purpose of message
  unsigned	qr :1;		// Response flag

  // Fields in fourth byte
  unsigned	rcode :4;	// Response code
  unsigned	cd: 1;		// Checking disabled by resolver
  unsigned	ad: 1;		// Authentic data from named
  unsigned	unused :1;	// Unused bits (MBZ as of 4.9.3a3)
  unsigned	ra :1;		// Recursion available

  // Remaining bytes
  unsigned	qdcount :16;	// Number of question entries
  unsigned	ancount :16;	// Number of answer entries
  unsigned	nscount :16;	// Number of authority entries
  unsigned	arcount :16;	// Number of resource entries
};

int res_init(); 

int res_query(const char *dname, int class, int type, unsigned char *answer, int anslen); 

int res_search(const char *dname, int class, int type, unsigned char *answer, int anslen); 

int res_querydomain(const char *name, const char *domain, int class, int type, 
		    unsigned char *answer, int anslen); 

int res_mkquery(int op, const char *dname, int class, int type, char *data, int datalen, 
		struct rrec *newrr, char *buf, int buflen); 

int res_send(const char *msg, int msglen, char *answer, int anslen); 

int dn_comp(unsigned char *exp_dn, unsigned char *comp_dn, int length, unsigned char **dnptrs, 
	    unsigned char **lastdnptr); 

int dn_expand(unsigned char *msg, unsigned char *eomorig, unsigned char *comp_dn, 
	      unsigned char *exp_dn, int length); 

#endif
