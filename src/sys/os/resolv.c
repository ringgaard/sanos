//
// resolv.c
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// DNS resolver
//

#include <os.h>
#include "resolv.h"

int res_init()
{
  return -ENOSYS;
}

int res_query(const char *dname, int class, int type, unsigned char *answer, int anslen)
{
  return -ENOSYS;
}

int res_search(const char *dname, int class, int type, unsigned char *answer, int anslen)
{
  return -ENOSYS;
}

int res_querydomain(const char *name, const char *domain, int class, int type, 
		    unsigned char *answer, int anslen)
{
  return -ENOSYS;
}

int res_mkquery(int op, const char *dname, int class, int type, char *data, int datalen, 
		struct rrec *newrr, char *buf, int buflen)
{
  return -ENOSYS;
}

int res_send(const char *msg, int msglen, char *answer, int anslen)
{
  return -ENOSYS;
}

int dn_comp(unsigned char *exp_dn, unsigned char *comp_dn, int length, unsigned char **dnptrs, 
	    unsigned char **lastdnptr)
{
  return -ENOSYS;
}

int dn_expand(unsigned char *msg, unsigned char *eomorig, unsigned char *comp_dn, 
	      unsigned char *exp_dn, int length)
{
  return -ENOSYS;
}
