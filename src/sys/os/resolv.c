//
// resolv.c
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// DNS resolver
//

#include <os.h>
#include <string.h>

#include "resolv.h"

#define NS_MAXCDNAME	255	        // Maximum compressed domain name
#define NS_MAXDNAME	255 /*1025*/	// Maximum domain name
#define NS_CMPRSFLGS	0xc0	        // Flag bits indicating name compression
#define NS_HFIXEDSZ	12		// #/bytes of fixed data in header
#define NS_QFIXEDSZ	4		// #/bytes of fixed data in query

#define MAXHOSTNAMELEN 256

#define QUERYBUF_SIZE   1024            // Size of query buffer

//
// mklower
//

static __inline int mklower(int ch)
{
  if (ch >= 0x41 && ch <= 0x5A) return ch + 0x20;
  return ch;
}

//
// special
//

static int special(int ch) 
{
  switch (ch) 
  {
    case 0x22: // '"'
    case 0x2E: // '.'
    case 0x3B: // ';'
    case 0x5C: // '\\'
    // Special modifiers in zone files
    case 0x40: // '@'
    case 0x24: // '$'
      return 1;
    default:
      return 0;
  }
}

//
// printable
//

static int printable(int ch) 
{
  return (ch > 0x20 && ch < 0x7f);
}

//
// inet_pton
//
// Convert from presentation format (which usually means ASCII printable)
// to network format (which is usually some kind of binary format).
// Return:
//   1 if the address was valid
//   0 if the address wasn't valid (`dst' is untouched in this case)
//

static int inet_pton(const char *src, unsigned char *dst)
{
  int saw_digit, octets, ch;
  unsigned char buf[sizeof(struct in_addr)], *p;

  saw_digit = 0;
  octets = 0;
  *(p = buf) = 0;
  while ((ch = *src++) != '\0') 
  {
    if (ch >= '0' && ch <= '9') 
    {
      unsigned int newval = *p * 10 + (ch - '0');
      if (newval > 255) return 0;
      *p = newval;
      
      if (!saw_digit) 
      {
        if (++octets > sizeof(struct in_addr)) return 0;
        saw_digit = 1;
      }
    } 
    else if (ch == '.' && saw_digit) 
    {
      if (octets == sizeof(struct in_addr)) return 0;
      *++p = 0;
      saw_digit = 0;
    } 
    else
      return 0;
  }

  if (octets < sizeof(struct in_addr)) return 0;
  memcpy(dst, p, sizeof(struct in_addr));
  return 1;
}

//
// dn_find
//
// Search for the counted-label name in an array of compressed names.
// Returns offset from msg if found, or error.
//
// dnptrs is the pointer to the first name on the list,
// not the pointer to the start of the message.
//

static int dn_find(unsigned char *domain, unsigned char *msg, 
		   unsigned char **dnptrs, unsigned char **lastdnptr)
{
  unsigned char *dn, *cp, *sp;
  unsigned char **cpp;
  unsigned int n;

  for (cpp = dnptrs; cpp < lastdnptr; cpp++) 
  {
    sp = *cpp;
    
    //
    // Terminate search on:
    //   - root label
    //   - compression pointer
    //   - unusable offset
    //

    while (*sp != 0 && (*sp & NS_CMPRSFLGS) == 0 && (sp - msg) < 0x4000) 
    {
      dn = domain;
      cp = sp;
      while ((n = *cp++) != 0)
      {
	 // Check for indirection
	switch (n & NS_CMPRSFLGS) 
	{
	  case 0: // normal case, n == len
	    if (n != *dn++) goto next;
	    for (; n > 0; n--) if (mklower(*dn++) != mklower(*cp++)) goto next;
	    
	    // Is next root for both ?
	    if (*dn == '\0' && *cp == '\0') return (sp - msg);
	    if (*dn) continue;
	    goto next;

	  case NS_CMPRSFLGS: // indirection
	    cp = msg + (((n & 0x3f) << 8) | *cp);
	    break;

	  default: // illegal type
	    return -EMSGSIZE;
	}
      }
next:
      sp += *sp + 1;
    }
  }

  return -ENOENT;
}

//
// ns_name_ntop
//
// Convert an encoded domain name to printable ascii as per RFC1035.
// Returns number of bytes written to buffer, or error.
// The root is returned as "."
// All other domains are returned in non absolute form
//

int ns_name_ntop(const unsigned char *src, char *dst, int dstsiz)
{
  const unsigned char *cp;
  unsigned char *dn, *eom;
  unsigned char c;
  unsigned int n;

  cp = src;
  dn = dst;
  eom = dst + dstsiz;

  while ((n = *cp++) != 0) 
  {
    if ((n & NS_CMPRSFLGS) != 0) return -EMSGSIZE;

    if (dn != dst) 
    {
      if (dn >= eom) return -EMSGSIZE;
      *dn++ = '.';
    }

    if (dn + n >= eom) return -EMSGSIZE;

    for (; n > 0; n--) 
    {
      c = *cp++;
      if (special(c))
      {
	if (dn + 1 >= eom) return -EMSGSIZE;
	*dn++ = '\\';
	*dn++ = (char) c;
      } 
      else if (!printable(c)) 
      {
	if (dn + 3 >= eom) return -EMSGSIZE;
	*dn++ = '\\';
	*dn++ = c / 100 + '0';
	*dn++ = (c % 100) / 10 + '0';
	*dn++ = c % 10 + '0';
      } 
      else 
      {
	if (dn >= eom) return -EMSGSIZE;
	*dn++ = (char) c;
      }
    }
  }

  if (dn == dst) 
  {
    if (dn >= eom) return -EMSGSIZE;
    *dn++ = '.';
  }

  if (dn >= eom) return -EMSGSIZE;
  *dn++ = '\0';

  return dn - dst;
}

//
// ns_name_pton
//	
// Convert a ascii string into an encoded domain name as per RFC1035.
//
// Return:
//     <0 if it fails
//	1 if string was fully qualified
//	0 if string was not fully qualified
//
// Enforces label and domain length limits.
//

int ns_name_pton(const char *src, unsigned char *dst, int dstsiz)
{
  unsigned char *label, *bp, *eom;
  int c, n, escaped;

  escaped = 0;
  bp = dst;
  eom = dst + dstsiz;
  label = bp++;

  while ((c = *src++) != 0)
  {
    if (escaped)
    {
      if (c >= '0' && c <= '9') 
      {
	n = (c - '0') * 100;
	if ((c = *src++) == 0 || c < '0' || c > '9') return -EINVAL;
	n += (c - '0') * 10;
	if ((c = *src++) == 0 || c < '0' || c > '9') return -EINVAL;
	n += (c - '0');
	if (n > 255) return -EINVAL;
	c = n;
      }
      escaped = 0;
    } 
    else if (c == '\\') 
    {
      escaped = 1;
      continue;
    } 
    else if (c == '.') 
    {
      c = (bp - label - 1);
      if ((c & NS_CMPRSFLGS) != 0) return -EMSGSIZE;
      if (label >= eom) return -EMSGSIZE;
      *label = c;
      
      // Fully qualified ?
      if (*src == '\0') 
      {
	if (c != 0) 
	{
	  if (bp >= eom) return -EMSGSIZE;
	  *bp++ = '\0';
	}
	if ((bp - dst) > NS_MAXCDNAME) return -EMSGSIZE;
	return 1;
      }

      if (c == 0 || *src == '.') return -EMSGSIZE;
      label = bp++;
      continue;
    }

    if (bp >= eom)  return -EMSGSIZE;
    *bp++ = (unsigned char) c;
  }

  c = (bp - label - 1);
  if ((c & NS_CMPRSFLGS) != 0) return -EMSGSIZE;
  if (label >= eom)  return -EMSGSIZE;
  *label = c;
  if (c != 0) 
  {
    if (bp >= eom)  return -EMSGSIZE;
    *bp++ = 0;
  }
  if ((bp - dst) > NS_MAXCDNAME)  return -EMSGSIZE;

  return 0;
}

//
// ns_name_pack
//
// Pack domain name 'src' into 'dst'.
// Returns size of the compressed name, or error.
//
// dnptrs' is an array of pointers to previous compressed names.
// dnptrs[0] is a pointer to the beginning of the message. The array
// ends with NULL.
// 'lastdnptr' is a pointer to the end of the array pointed to
// by 'dnptrs'.
//
// The list of pointers in dnptrs is updated for labels inserted into
// the message as we compress the name.  If 'dnptr' is NULL, we don't
// try to compress names. If 'lastdnptr' is NULL, we don't update the
// list.
//

static int ns_name_pack(const unsigned char *src, unsigned char *dst, int dstsiz,
	                unsigned char **dnptrs, unsigned char **lastdnptr)
{
  unsigned char *dstp;
  unsigned char **cpp, **lpp, *eob, *msg;
  unsigned char *srcp;
  int n, l, first = 1;

  srcp = (unsigned char *) src;
  dstp = dst;
  eob = dstp + dstsiz;
  lpp = cpp = NULL;
  if (dnptrs != NULL) 
  {
    if ((msg = *dnptrs++) != NULL) 
    {
      for (cpp = dnptrs; *cpp != NULL; cpp++);
      lpp = cpp;
    }
  } 
  else
    msg = NULL;

  // Make sure the domain we are about to add is legal
  l = 0;
  do 
  {
    n = *srcp;
    if ((n & NS_CMPRSFLGS) != 0) return -EMSGSIZE;
    l += n + 1;
    if (l > NS_MAXCDNAME) return -EMSGSIZE;
    srcp += n + 1;
  } while (n != 0);

  // From here on we need to reset compression pointer array on error
  srcp = (unsigned char *) src;
  do 
  {
    // Look to see if we can use pointers
    n = *srcp;
    if (n != 0 && msg != NULL) 
    {
      l = dn_find(srcp, msg, dnptrs, lpp);
      if (l >= 0) 
      {
	if (dstp + 1 >= eob) goto cleanup;
	*dstp++ = (l >> 8) | NS_CMPRSFLGS;
	*dstp++ = l % 256;
	return dstp - dst;
      }

      // Not found, save it
      if (lastdnptr != NULL && cpp < lastdnptr - 1 && (dstp - msg) < 0x4000 && first) 
      {
	*cpp++ = dstp;
	*cpp = NULL;
	first = 0;
      }
    }

    // Copy label to buffer
    if (n & NS_CMPRSFLGS) goto cleanup;
    if (dstp + 1 + n >= eob) goto cleanup;
    memcpy(dstp, srcp, n + 1);
    srcp += n + 1;
    dstp += n + 1;
  } while (n != 0);

  if (dstp > eob) 
  {
cleanup:
    if (msg != NULL) *lpp = NULL;
    return -EMSGSIZE;
  } 

  return dstp - dst;
}

//
// ns_name_unpack
//
// Unpack a domain name from a message, source may be compressed.
// Return error if it fails, or consumed octets if it succeeds.
//

static int ns_name_unpack(const unsigned char *msg, const unsigned char *eom, 
			  const unsigned char *src, unsigned char *dst, int dstsiz)
{
  const unsigned char *srcp, *dstlim;
  unsigned char *dstp;
  int n, len, checked;

  len = -1;
  checked = 0;
  dstp = dst;
  srcp = src;
  dstlim = dst + dstsiz;
  if (srcp < msg || srcp >= eom) return -EMSGSIZE;

  // Fetch next label in domain name
  while ((n = *srcp++) != 0) 
  {
    // Check for indirection
    switch (n & NS_CMPRSFLGS) 
    {
      case 0:
	// Limit checks
	if (dstp + n + 1 >= dstlim || srcp + n >= eom) return -EMSGSIZE;
	checked += n + 1;
	*dstp++ = n;
	memcpy(dstp, srcp, n);
	dstp += n;
	srcp += n;
	break;

      case NS_CMPRSFLGS:
	if (srcp >= eom) return -EMSGSIZE;
	if (len < 0) len = srcp - src + 1;
	srcp = msg + (((n & 0x3F) << 8) | (*srcp & 0xFF));
	if (srcp < msg || srcp >= eom) return -EMSGSIZE;
	checked += 2;

	// Check for loops in the compressed name; 
	// if we've looked at the whole message, there must be a loop.
	if (checked >= eom - msg) return -EMSGSIZE;
	break;

      default:
	return -EMSGSIZE;
    }
  }

  *dstp = '\0';
  if (len < 0) len = srcp - src;
  return len;
}

//
// res_init
//

int res_init()
{
  return -ENOSYS;
}

//
// res_query
//

int res_query(const char *dname, int class, int type, unsigned char *answer, int anslen)
{
  return -ENOSYS;
}

//
// res_search
//

int res_search(const char *dname, int class, int type, unsigned char *answer, int anslen)
{
  return -ENOSYS;
}

//
// res_querydomain
//

int res_querydomain(const char *name, const char *domain, int class, int type, 
		    unsigned char *answer, int anslen)
{
  return -ENOSYS;
}

//
// res_mkquery
//

int res_mkquery(int op, const char *dname, int class, int type, char *data, int datalen, 
		struct rrec *newrr, char *buf, int buflen)
{
  return -ENOSYS;
}

//
// res_send
//

int res_send(const char *msg, int msglen, char *answer, int anslen)
{
  return -ENOSYS;
}

//
// dn_comp
//

int dn_comp(const char *src, unsigned char *dst, int dstsiz, unsigned char **dnptrs, unsigned char **lastdnptr)
{
  unsigned char tmp[NS_MAXCDNAME];
  int rc;

  rc = ns_name_pton(src, tmp, sizeof tmp);
  if (rc < 0) return rc;

  return ns_name_pack(tmp, dst, dstsiz, dnptrs, lastdnptr);
}

//
// dn_expand
//

int dn_expand(const unsigned char *msg, const unsigned char *eom, const unsigned char *src,  char *dst, int dstsiz)
{
  unsigned char tmp[NS_MAXCDNAME];
  int rc;

  rc = ns_name_unpack(msg, eom, src, tmp, sizeof tmp);
  if (rc < 0) return rc;

  rc = ns_name_ntop(tmp, dst, dstsiz);
  if (rc < 0) return rc;

  if (rc > 0 && dst[0] == '.') dst[0] = '\0';
  return rc;
}

//
// getanswer
//

static struct hostent *getanswer(const char *answer, int anslen, const char *qname, int qtype)
{
#if 0
  struct tib *tib = gettib();
  const struct dns_hdr *hp;
  const unsigned char *cp;
  int n;
  const unsigned char *eom, *erdata;
  char *bp, **ap, **hap;
  int type, class, buflen, ancount, qdcount;
  int haveanswer, had_error;
  int toobig = 0;
  char tbuf[NS_MAXDNAME];
  const char *tname;
  int (*name_ok)(const char *));

  tname = qname;
  tib->host.h_name = NULL;
  eom = answer->buf + anslen;
  switch (qtype) 
  {
    case DNS_TYPE_A:
      name_ok = res_hnok;
      break;

    case DNS_TYPE_A:
      name_ok = res_dnok;
      break;

    default:
      return NULL;
  }

  // Find first satisfactory answer
  hp = (const struct dns_hdr *) answer;
  ancount = ntohs(hp->ancount);
  qdcount = ntohs(hp->qdcount);
  bp = tib->hhostbuf;
  buflen = sizeof tib->hostbuf;
  cp = answer;

  cp += NS_HFIXEDSZ;
  if (cp > eom) return NULL;

  if (qdcount != 1) return NULL;

  n = dn_expand(answer, eom, cp, bp, buflen);
  if (n < 0 || !name_ok(bp)) return NULL;

  cp += n + NS_QFIXEDSZ;
  if (cp > eom) return NULL;

  if (qtype == DNS_TYPE_A)
  {
    // res_send() has already verified that the query name is the
    // same as the one we sent; this just gets the expanded name
    // (i.e., with the succeeding search-domain tacked on).

    n = strlen(bp) + 1;
    if (n >= MAXHOSTNAMELEN) return NULL;
    tib->host.h_name = bp;
    bp += n;
    buflen -= n;
  
    // The qname can be abbreviated, but h_name is now absolute
    qname = tib->host.h_name;
  }

  ap = tib->host_aliases;
  *ap = NULL;
  tib->host.h_aliases = tib->host_aliases;
  hap = h_addr_ptrs;
  *hap = NULL;
  tib->host.h_addr_list = tib->h_addr_ptrs;
  
  haveanswer = 0;
  had_error = 0;
  while (ancount-- > 0 && cp < eom && !had_error) 
  {
    n = dn_expand(answer->buf, eom, cp, bp, buflen);
    if ((n < 0) || !name_ok(bp))
    {
      had_error++;
      continue;
    }

    cp += n;			/* name */

    if (cp + 3 * sizeof(short) * sizeof(long) > eom) return NULL;

    type = ns_get16(cp);
    cp += INT16SZ;			/* type */
    class = ns_get16(cp);
    cp += INT16SZ + INT32SZ;	/* class, TTL */
    n = ns_get16(cp);
    cp += INT16SZ;			/* len */
    BOUNDS_CHECK(cp, n);
    erdata = cp + n;
    if (class != C_IN) 
    {
      /* XXX - debug? syslog? */
      cp += n;
      continue;		/* XXX - had_error++ ? */
    }
    if ((qtype == T_A || qtype == T_AAAA) && type == T_CNAME) 
    {
      if (ap >= &host_aliases[MAXALIASES-1])
	      continue;
      n = dn_expand(answer->buf, eom, cp, tbuf, sizeof tbuf);
      if ((n < 0) || !(*name_ok)(tbuf)) 
      {
	had_error++;
	continue;
      }
      cp += n;
      if (cp != erdata) 
      {
	__set_h_errno (NO_RECOVERY);
	return (NULL);
      }
      
      // Store alias
      *ap++ = bp;
      n = strlen(bp) + 1;	// for the \0
      if (n >= MAXHOSTNAMELEN) 
      {
	had_error++;
	continue;
      }
      bp += n;
      buflen -= n;
      /* Get canonical name. */
      n = strlen(tbuf) + 1;	/* for the \0 */
      if (n > buflen || n >= MAXHOSTNAMELEN) 
      {
	had_error++;
	continue;
      }
      strcpy(bp, tbuf);
      host.h_name = bp;
      bp += n;
      buflen -= n;
      continue;
    }

    if (qtype == T_PTR && type == T_CNAME) 
    {
      n = dn_expand(answer->buf, eom, cp, tbuf, sizeof tbuf);
      if (n < 0 || !res_dnok(tbuf)) 
      {
	had_error++;
	continue;
      }
      cp += n;
      if (cp != erdata) 
      {
	__set_h_errno (NO_RECOVERY);
	return (NULL);
      }
      /* Get canonical name. */
      n = strlen(tbuf) + 1;	/* for the \0 */
      if (n > buflen || n >= MAXHOSTNAMELEN) 
      {
	had_error++;
	continue;
      }
      strcpy(bp, tbuf);
      tname = bp;
      bp += n;
      buflen -= n;
      continue;
    }

    if ((type == T_SIG) || (type == T_KEY) || (type == T_NXT)) 
    {
      /* We don't support DNSSEC yet.  For now, ignore
       * the record and send a low priority message
       * to syslog.
       */
      syslog(LOG_DEBUG|LOG_AUTH, "gethostby*.getanswer: asked for \"%s %s %s\", got type \"%s\"", qname, p_class(C_IN), p_type(qtype), p_type(type));
      cp += n;
      continue;
    }

    if (type != qtype)
    {
      syslog(LOG_NOTICE|LOG_AUTH, "gethostby*.getanswer: asked for \"%s %s %s\", got type \"%s\"", qname, p_class(C_IN), p_type(qtype), p_type(type));
      cp += n;
      continue;		/* XXX - had_error++ ? */
    }

    switch (type) 
    {
      case T_PTR:
	if (strcasecmp(tname, bp) != 0) 
	{
	  syslog(LOG_NOTICE|LOG_AUTH, AskedForGot, qname, bp);
	  cp += n;
	  continue;	/* XXX - had_error++ ? */
	}
	n = dn_expand(answer->buf, eom, cp, bp, buflen);
	if ((n < 0) || !res_hnok(bp)) 
	{
	  had_error++;
	  break;
	}
	host.h_name = bp;
	__set_h_errno (NETDB_SUCCESS);
	return (&host);

      case T_A:
      case T_AAAA:
	if (strcasecmp(host.h_name, bp) != 0) 
	{
	  syslog(LOG_NOTICE|LOG_AUTH, AskedForGot, host.h_name, bp);
	  cp += n;
	  continue;	/* XXX - had_error++ ? */
	}
	if (n != host.h_length) 
	{
	  cp += n;
	  continue;
	}
	if (!haveanswer) 
	{
	  int nn;

	  host.h_name = bp;
	  nn = strlen(bp) + 1;	/* for the \0 */
	  bp += nn;
	  buflen -= nn;
	}

	/* XXX: when incrementing bp, we have to decrement
	 * buflen by the same amount --okir */
	buflen -= sizeof(align) - ((u_long)bp % sizeof(align));

	bp += sizeof(align) - ((u_long)bp % sizeof(align));

	if (bp + n >= &hostbuf[sizeof hostbuf]) 
	{
	  dprintf("size (%d) too big\n", n);
	  had_error++;
	  continue;
	}
	if (hap >= &h_addr_ptrs[MAXADDRS-1]) 
	{
	  if (!toobig++) 
	  {
	    dprintf("Too many addresses (%d)\n", MAXADDRS);
	  }
	  cp += n;
	  continue;
	}
	memmove(*hap++ = bp, cp, n);
	bp += n;
	buflen -= n;
	cp += n;
	if (cp != erdata) 
	{
	  __set_h_errno (NO_RECOVERY);
	  return (NULL);
	}
	break;

      default:
        return NULL;
    }
    if (!had_error) haveanswer++;
  }

  if (haveanswer) 
  {
    *ap = NULL;
    *hap = NULL;
    if (!host.h_name) 
    {
      n = strlen(qname) + 1;	/* for the \0 */
      if (n > buflen || n >= MAXHOSTNAMELEN) goto no_recovery;
      strcpy(bp, qname);
      host.h_name = bp;
      bp += n;
      buflen -= n;
    }

    __set_h_errno (NETDB_SUCCESS);
    return &host;
  }

no_recovery:
  //__set_h_errno (NO_RECOVERY);
#endif
  return NULL;
}

//
// gethostbyname
//

struct hostent *gethostbyname(const char *name)
{
  char buf[QUERYBUF_SIZE];
  const char *cp;
  char *bp;
  int n, len;
  struct tib *tib = gettib();

  tib->host.h_addrtype = AF_INET;
  tib->host.h_length = sizeof(struct in_addr);

  // Disallow names consisting only of digits/dots, unless they end in a dot
  if (*name >= '0' && *name <= '9')
  {
    for (cp = name;; ++cp) 
    {
      if (!*cp) 
      {
	if (*--cp == '.') break;

	// All-numeric, no dot at the end. Fake up a hostent as if we'd actually done a lookup.
	if (inet_pton(name, tib->host_addr) <= 0) return NULL;

	strncpy(tib->hostbuf, name, NS_MAXDNAME);
	tib->hostbuf[NS_MAXDNAME] = '\0';
	bp = tib->hostbuf + NS_MAXDNAME;
	len = sizeof tib->hostbuf - NS_MAXDNAME;
	tib->host.h_name = tib->hostbuf;
	tib->host.h_aliases = tib->host_aliases;
	tib->host_aliases[0] = NULL;
	tib->h_addr_ptrs[0] = (char *) tib->host_addr;
	tib->h_addr_ptrs[1] = NULL;
	tib->host.h_addr_list = tib->h_addr_ptrs;

	return &tib->host;
      }

      if (*cp < '0' && *cp > '9' && *cp != '.') break;
    }
  }

  n = res_search(name, DNS_CLASS_IN, DNS_TYPE_A, buf, sizeof(buf));
  if (n < 0) return NULL;
  return getanswer(buf, n, name, DNS_TYPE_A);
}

//
// gethostbyaddr
//

struct hostent *gethostbyaddr(const char *addr, int len, int type)
{
  const unsigned char *uaddr = (const unsigned char *) addr;
  int n;
  char buf[QUERYBUF_SIZE];
  struct hostent *hp;
  char qbuf[NS_MAXDNAME + 1];

  if (type != AF_INET) return NULL;
  if (len != sizeof(struct in_addr)) return NULL;

  sprintf(qbuf, "%u.%u.%u.%u.in-addr.arpa", uaddr[3], addr[2], uaddr[1], uaddr[0]);

  n = res_query(qbuf, DNS_CLASS_IN, DNS_TYPE_PTR, buf, sizeof buf);
  if (n < 0)
  {
    //if (errno == ECONNREFUSED) return (_gethtbyaddr(addr, len, af));
    return NULL;
  }

  hp = getanswer(buf, n, qbuf, DNS_TYPE_PTR);
  if (!hp) return NULL;

  hp->h_addrtype = AF_INET;
  hp->h_length = len;
  
  //memmove(host_addr, addr, len);
  //h_addr_ptrs[0] = (char *) host_addr;
  //h_addr_ptrs[1] = NULL;

  return hp;
}
