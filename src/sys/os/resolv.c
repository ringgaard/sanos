//
// resolv.c
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// DNS resolver
//

#include <os.h>
#include "resolv.h"

#define NS_MAXCDNAME	255	// Maximum compressed domain name
#define NS_CMPRSFLGS	0xc0	// Flag bits indicating name compression

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
	srcp = msg + (((n & 0x3f) << 8) | (*srcp & 0xff));
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
