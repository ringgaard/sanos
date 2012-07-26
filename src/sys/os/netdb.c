//
// netdb.c
//
// Network data base library
//
// Copyright (C) 2002 Michael Ringgaard. All rights reserved.
// Portions Copyright (C) 1996-2002  Internet Software Consortium.
// Portions Copyright (C) 1996-2001  Nominum, Inc.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 
// 1. Redistributions of source code must retain the above copyright 
//    notice, this list of conditions and the following disclaimer.  
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.  
// 3. Neither the name of the project nor the names of its contributors
//    may be used to endorse or promote products derived from this software
//    without specific prior written permission. 
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
// OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
// OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF 
// SUCH DAMAGE.
// 

#include <os.h>
#include <string.h>
#include <inifile.h>

#include "resolv.h"

int sprintf(char *buf, const char *fmt, ...);

//
// Protocol aliases
//

char *palias[] = {
  "IP", NULL,       //  0 ip
  "ICMP", NULL,     //  2 icmp
  "GGP", NULL,      //  4 ggp
  "TCP", NULL,      //  6 tcp
  "EGP", NULL,      //  8 egp
  "PUP", NULL,      // 10 pup
  "UDP", NULL,      // 12 udp
  "HMP", NULL,      // 14 hmp
  "XNS-IDP", NULL,  // 16 xns-idp
  "RDP", NULL,      // 18 rdp
  "RVD", NULL,      // 20 rvd
};

//
// Internet protocols as defined by RFC 1700 (Assigned Numbers).
//

struct protoent protocols[] = {
  {"ip",      palias + 0,  0},   // Internet protocol
  {"icmp",    palias + 2,  1},   // Internet control message protocol
  {"ggp",     palias + 4,  3},   // Gateway-gateway protocol
  {"tcp",     palias + 6,  6},   // Transmission control protocol
  {"egp",     palias + 8,  8},   // Exterior gateway protocol
  {"pup",     palias + 10, 12},  // PARC universal packet protocol
  {"udp",     palias + 12, 17},  // User datagram protocol
  {"hmp",     palias + 14, 20},  // Host monitoring protocol
  {"xns-idp", palias + 16, 22},  // Xerox NS IDP
  {"rdp",     palias + 18, 27},  // "reliable datagram" protocol
  {"rvd",     palias + 20, 66},  // MIT remote virtual disk
  {NULL,      NULL, 0}
};

char *salias[] = {
  NULL,
  "sink", "null", NULL,         //  1 echo
  "users", NULL,                //  4 sysstat
  "quote", NULL,                //  6 qutd
  "ttytst", "source", NULL,     //  8 chargen
  "mail", NULL,                 // 11 smtp
  "timeserver", NULL,           // 13 time
  "resource", NULL,             // 15 rlp
  "name", NULL,                 // 17 nameserver
  "whois", NULL,                // 19 nicname
  "dhcps", NULL,                // 21 bootps
  "dhcpc", NULL,                // 23 bootpc
  "www", "www-http", NULL,      // 25 hhtp
  "krb5", "kerberos-sec", NULL, // 28 kerberos
  "hostnames", NULL,            // 31 hostname
  "postoffice", NULL,           // 33 pop2
  "rpcbind", "portmap", NULL,   // 35 sunrpc
  "ident", "tap", NULL,         // 38 auth
  "usenet", NULL,               // 41 nntp
  "loc-srv", NULL,              // 43 epmap
  "nbname", NULL,               // 45 netbios-ns
  "nbdatagram", NULL,           // 47 netbios-dgm
  "nbsession", NULL,            // 49 netbios-ssn
  "imap4", NULL,                // 51 imap
  "snmp-trap", NULL,            // 53 snmptrap
  "MCom", NULL,                 // 55 https
  "ike", NULL,                  // 57 isakmp
  "comsat", NULL,               // 59 biff
  "whod", NULL,                 // 61 who
  "shell", NULL,                // 63 cmd
  "spooler", NULL,              // 65 printer
  "route", "routed", NULL,      // 67 router
  "timeserver", NULL,           // 70 timed
  "newdate", NULL,              // 72 tempo
  "rpc", NULL,                  // 74 courier
  "chat", NULL,                 // 76 conference
  "readnews", NULL,             // 78 netnews
  "uucpd", NULL,                // 80 uucp
  "krcmd", NULL,                // 82 kshell
  "new-who", NULL,              // 84 new-rwho
  "rfs", "rfs_server", NULL,    // 86 remotefs
  "rmonitord", NULL,            // 89 rmonitor
  "slapd", NULL,                // 91 ldaps
  "ingres", NULL,               // 93 ingreslock
  "nfs", NULL                   // 95 nfsd
};

//
// Port numbers for well-known services defined by IANA
//

struct servent services[] = {
  {"echo",             salias,         7, "tcp"},
  {"echo",             salias,         7, "udp"},
  {"discard",          salias + 1,     9, "tcp"},
  {"discard",          salias + 1,     9, "udp"},
  {"systat",           salias + 4,    11, "tcp"},  // Active users
  {"systat",           salias + 4,    11, "tcp"},  // Active users
  {"daytime",          salias,        13, "tcp"},
  {"daytime",          salias,        13, "udp"},
  {"qotd",             salias + 6,    17, "tcp"},  // Quote of the day
  {"qotd",             salias + 6,    17, "udp"},  // Quote of the day
  {"chargen",          salias + 8,    19, "tcp"},  // Character generator
  {"chargen",          salias + 8,    19, "udp"},  // Character generator
  {"ftp-data",         salias,        20, "tcp"},  // FTP, data
  {"ftp",              salias,        21, "tcp"},  // FTP, control
  {"telnet",           salias,        23, "tcp"},
  {"smtp",             salias + 11,   25, "tcp"},  // Simple Mail Transfer Protocol
  {"time",             salias + 13,   37, "tcp"},
  {"time",             salias + 13,   37, "udp"},
  {"rlp",              salias + 15,   39, "udp"},  // Resource Location Protocol
  {"nameserver",       salias + 17,   42, "tcp"},  // Host Name Server
  {"nameserver",       salias + 17,   42, "udp"},  // Host Name Server
  {"nicname",          salias + 19,   43, "tcp"},
  {"domain",           salias,        53, "tcp"},  // Domain Name Server
  {"domain",           salias,        53, "udp"},  // Domain Name Server
  {"bootps",           salias + 21,   67, "udp"},  // Bootstrap Protocol Server
  {"bootpc",           salias + 23,   68, "udp"},  // Bootstrap Protocol Client
  {"tftp",             salias,        69, "udp"},  // Trivial File Transfer
  {"gopher",           salias,        70, "tcp"},
  {"finger",           salias,        79, "tcp"},
  {"http",             salias + 25,   80, "tcp"},  // World Wide Web
  {"kerberos",         salias + 28,   88, "tcp"},  // Kerberos
  {"kerberos",         salias + 28,   88, "udp"},  // Kerberos
  {"hostname",         salias + 31,  101, "tcp"},  // NIC Host Name Server
  {"iso-tsap",         salias,       102, "tcp"},  // ISO-TSAP Class 0
  {"rtelnet",          salias,       107, "tcp"},  // Remote Telnet Service
  {"pop2",             salias + 33,  109, "tcp"},  // Post Office Protocol - Version 2
  {"pop3",             salias,       110, "tcp"},  // Post Office Protocol - Version 3
  {"sunrpc",           salias + 35,  111, "tcp"},  // SUN Remote Procedure Call
  {"sunrpc",           salias + 35,  111, "udp"},  // SUN Remote Procedure Call
  {"auth",             salias + 38,  113, "tcp"},  // Identification Protocol
  {"uucp-path",        salias,       117, "tcp"},
  {"nntp",             salias + 41,  119, "tcp"},  // Network News Transfer Protocol
  {"ntp",              salias,       123, "udp"},  // Network Time Protocol
  {"epmap",            salias + 43,  135, "tcp"},  // DCE endpoint resolution
  {"epmap",            salias + 43,  135, "udp"},  // DCE endpoint resolution
  {"netbios-ns",       salias + 45,  137, "tcp"},  // NETBIOS Name Service
  {"netbios-ns",       salias + 45,  137, "udp"},  // NETBIOS Name Service
  {"netbios-dgm",      salias + 47,  138, "udp"},  // NETBIOS Datagram Service
  {"netbios-ssn",      salias + 49,  139, "tcp"},  // NETBIOS Session Service
  {"imap",             salias + 51,  143, "tcp"},  // Internet Message Access Protocol
  {"pcmail-srv",       salias,       158, "tcp"},  // PCMail Server
  {"snmp",             salias,       161, "udp"},  // SNMP
  {"snmptrap",         salias + 53,  162, "udp"},  // SNMP trap
  {"print-srv",        salias,       170, "tcp"},  // Network PostScript
  {"bgp",              salias,       179, "tcp"},  // Border Gateway Protocol
  {"irc",              salias,       194, "tcp"},  // Internet Relay Chat Protocol        
  {"ipx",              salias,       213, "udp"},  // IPX over IP
  {"ldap",             salias,       389, "tcp"},  // Lightweight Directory Access Protocol
  {"https",            salias + 55,  443, "tcp"},
  {"https",            salias + 55,  443, "udp"},
  {"microsoft-ds",     salias,       445, "tcp"},
  {"microsoft-ds",     salias,       445, "udp"},
  {"kpasswd",          salias,       464, "tcp"},  // Kerberos (v5)
  {"kpasswd",          salias,       464, "udp"},  // Kerberos (v5)
  {"isakmp",           salias + 57,  500, "udp"},  // Internet Key Exchange
  {"exec",             salias,       512, "tcp"},  // Remote Process Execution
  {"biff",             salias + 59,  512, "udp"},
  {"login",            salias,       513, "tcp"},  // Remote Login
  {"who",              salias + 61,  513, "udp"},
  {"cmd",              salias + 63,  514, "tcp"},
  {"syslog",           salias,       514, "udp"},
  {"printer",          salias + 65,  515, "tcp"},
  {"talk",             salias,       517, "udp"},
  {"ntalk",            salias,       518, "udp"},
  {"efs",              salias,       520, "tcp"},  // Extended File Name Server
  {"router",           salias + 67,  520, "udp"},
  {"timed",            salias + 70,  525, "udp"},
  {"tempo",            salias + 72,  526, "tcp"},
  {"courier",          salias + 74,  530, "tcp"},
  {"conference",       salias + 76,  531, "tcp"},
  {"netnews",          salias + 78,  532, "tcp"},
  {"netwall",          salias,       533, "udp"},  // For emergency broadcasts
  {"uucp",             salias + 80,  540, "tcp"},
  {"klogin",           salias,       543, "tcp"},  // Kerberos login
  {"kshell",           salias + 82,  544, "tcp"},  // Kerberos remote shell
  {"new-rwho",         salias + 84,  550, "udp"},
  {"remotefs",         salias + 86,  556, "tcp"},
  {"rmonitor",         salias + 89,  560, "udp"},
  {"monitor",          salias,       561, "udp"},
  {"ldaps",            salias + 91,  636, "tcp"},  // LDAP over TLS/SSL
  {"doom",             salias,       666, "tcp"},  // Doom Id Software
  {"doom",             salias,       666, "udp"},  // Doom Id Software
  {"kerberos-adm",     salias,       749, "tcp"},  // Kerberos administration
  {"kerberos-adm",     salias,       749, "udp"},  // Kerberos administration
  {"kerberos-iv",      salias,       750, "udp"},  // Kerberos version IV
  {"kpop",             salias,      1109, "tcp"},  // Kerberos POP
  {"phone",            salias,      1167, "udp"},  // Conference calling
  {"ms-sql-s",         salias,      1433, "tcp"},  // Microsoft-SQL-Server 
  {"ms-sql-s",         salias,      1433, "udp"},  // Microsoft-SQL-Server 
  {"ms-sql-m",         salias,      1434, "tcp"},  // Microsoft-SQL-Monitor
  {"ms-sql-m",         salias,      1434, "udp"},  // Microsoft-SQL-Monitor                
  {"wins",             salias,      1512, "tcp"},  // Microsoft Windows Internet Name Service
  {"wins",             salias,      1512, "udp"},  // Microsoft Windows Internet Name Service
  {"ingreslock",       salias + 93, 1524, "tcp"},
  {"l2tp",             salias,      1701, "udp"},  // Layer Two Tunneling Protocol
  {"pptp",             salias,      1723, "tcp"},  // Point-to-point tunnelling protocol
  {"radius",           salias,      1812, "udp"},  // RADIUS authentication protocol
  {"radacct",          salias,      1813, "udp"},  // RADIUS accounting protocol
  {"nfsd",             salias + 95, 2049, "udp"},  // NFS server
  {"knetd",            salias,      2053, "tcp"},  // Kerberos de-multiplexor
  {"man",              salias,      9535, "tcp"},  // Remote Man Server
  {NULL, NULL, 0, NULL}
};

//
// getanswer
//

static struct hostent *getanswer(const char *answer, int anslen, const char *qname, int qtype) {
  struct tib *tib = gettib();
  const struct dns_hdr *hp;
  const unsigned char *cp;
  int n;
  const unsigned char *eom, *erdata;
  char *bp, **ap, **hap;
  int type, class, ttl, buflen, ancount, qdcount;
  int haveanswer, had_error;
  char tbuf[NS_MAXDNAME];
  const char *tname;

  tname = qname;
  tib->host.h_name = NULL;
  eom = answer + anslen;

  // Find first satisfactory answer
  hp = (const struct dns_hdr *) answer;
  ancount = ntohs(hp->ancount);
  qdcount = ntohs(hp->qdcount);
  bp = tib->hostbuf;
  buflen = sizeof tib->hostbuf;
  cp = answer;

  cp += NS_HFIXEDSZ;
  if (cp > eom)  {
    errno = EMSGSIZE;
    return NULL;
  }

  if (qdcount != 1) {
    errno = EIO;
    return NULL;
  }

  n = dn_expand(answer, eom, cp, bp, buflen);
  if (n < 0) return NULL;

  cp += n + NS_QFIXEDSZ;
  if (cp > eom) {
    errno = EMSGSIZE;
    return NULL;
  }

  if (qtype == DNS_TYPE_A) {
    // res_send() has already verified that the query name is the
    // same as the one we sent; this just gets the expanded name
    // (i.e., with the succeeding search-domain tacked on).

    n = strlen(bp) + 1;
    if (n >= MAXHOSTNAMELEN) {
      errno = EMSGSIZE;
      return NULL;
    }
    tib->host.h_name = bp;
    bp += n;
    buflen -= n;
  
    // The qname can be abbreviated, but h_name is now absolute
    qname = tib->host.h_name;
  }

  ap = tib->host_aliases;
  *ap = NULL;
  tib->host.h_aliases = tib->host_aliases;
  hap = tib->h_addr_ptrs;
  *hap = NULL;
  tib->host.h_addr_list = tib->h_addr_ptrs;
  
  haveanswer = 0;
  had_error = 0;
  while (ancount-- > 0 && cp < eom && !had_error) {
    n = dn_expand(answer, eom, cp, bp, buflen);
    if (n < 0) {
      had_error++;
      continue;
    }

    cp += n; // name

    if (cp + 3 * sizeof(short) + sizeof(long) > eom) {
      errno = EMSGSIZE;
      return NULL;
    }

    type = ntohs(*(unsigned short *) cp);
    cp += sizeof(unsigned short); // type

    class = ntohs(*(unsigned short *) cp);
    cp += sizeof(unsigned short); // class

    ttl = ntohl(*(unsigned long *) cp);
    cp += sizeof(unsigned long); // ttl

    n = ntohs(*(unsigned short *) cp);
    cp += sizeof(unsigned short); // len

    if (cp + n > eom) {
      errno = EMSGSIZE;
      return NULL;
    }
    
    erdata = cp + n;
    if (class != DNS_CLASS_IN) {
      cp += n;
      continue;
    }

    if (qtype == DNS_TYPE_A && type == DNS_TYPE_CNAME) {
      if (ap >= &tib->host_aliases[MAX_HOST_ALIASES - 1]) continue;
      n = dn_expand(answer, eom, cp, tbuf, sizeof tbuf);
      if (n < 0) {
        had_error++;
        continue;
      }
      cp += n;
      if (cp != erdata) {
        errno = EIO;
        return NULL;
      }
      
      // Store alias
      *ap++ = bp;
      n = strlen(bp) + 1;
      if (n >= MAXHOSTNAMELEN) {
        errno = EMSGSIZE;
        had_error++;
        continue;
      }
      
      bp += n;
      buflen -= n;
      
      // Get canonical name
      n = strlen(tbuf) + 1;
      if (n > buflen || n >= MAXHOSTNAMELEN) {
        errno = EMSGSIZE;
        had_error++;
        continue;
      }
      
      strcpy(bp, tbuf);
      tib->host.h_name = bp;
      bp += n;
      buflen -= n;
      continue;
    }

    if (qtype == DNS_TYPE_PTR && type == DNS_TYPE_CNAME) {
      n = dn_expand(answer, eom, cp, tbuf, sizeof tbuf);
      if (n < 0) {
        had_error++;
        continue;
      }
      cp += n;
      if (cp != erdata) {
        errno = EIO;
        return NULL; 
      }

      // Get canonical name
      n = strlen(tbuf) + 1;
      if (n > buflen || n >= MAXHOSTNAMELEN) {
        errno = EMSGSIZE;
        had_error++;
        continue;
      }
      strcpy(bp, tbuf);
      tname = bp;
      bp += n;
      buflen -= n;
      continue;
    }

    if (type != qtype) {
      cp += n;
      continue;
    }

    switch (type) {
      case DNS_TYPE_PTR:
        if (stricmp(tname, bp) != 0) {
          cp += n;
          continue;
        }

        n = dn_expand(answer, eom, cp, bp, buflen);
        if (n < 0) {
          had_error++;
          break;
        }
        tib->host.h_name = bp;
        return &tib->host;

      case DNS_TYPE_A:
        if (stricmp(tib->host.h_name, bp) != 0) {
          cp += n;
          continue;
        }
        if (n != tib->host.h_length) {
          cp += n;
          continue;
        }
        if (!haveanswer) {
          int nn;

          tib->host.h_name = bp;
          nn = strlen(bp) + 1;
          bp += nn;
          buflen -= nn;
        }

        // Align next buffer entry
        buflen -= sizeof(int) - ((unsigned long) bp % sizeof(int));
        bp += sizeof(int) - ((unsigned long) bp % sizeof(int));

        if (bp + n >= &tib->hostbuf[sizeof tib->hostbuf]) {
          errno = EMSGSIZE;
          had_error++;
          continue;
        }

        if (hap >= &tib->h_addr_ptrs[MAX_HOST_ADDRS - 1]) {
          cp += n;
          continue;
        }

        memmove(*hap++ = bp, cp, n);
        bp += n;
        buflen -= n;
        cp += n;
        if (cp != erdata) {
          errno = EIO;
          return NULL;
        }
        break;

      default:
        errno = EIO;
        return NULL;
    }

    if (!had_error) haveanswer++;
  }

  if (haveanswer) {
    *ap = NULL;
    *hap = NULL;
    if (!tib->host.h_name) {
      n = strlen(qname) + 1;
      if (n > buflen || n >= MAXHOSTNAMELEN) {
        errno = EMSGSIZE;
        return NULL;
      }
      strcpy(bp, qname);
      tib->host.h_name = bp;
      bp += n;
      buflen -= n;
    }

    return &tib->host;
  }

  return NULL;
}

//
// gethostbyname
//

struct hostent *gethostbyname(const char *name) {
  char buf[QUERYBUF_SIZE];
  const char *cp;
  int n;
  struct tib *tib = gettib();

  tib->host.h_addrtype = AF_INET;
  tib->host.h_length = sizeof(struct in_addr);

  // Return 127.0.0.1 for localhost
  if (strcmp(name, "localhost") == 0) {
    struct in_addr loaddr;
    loaddr.s_addr = htonl(INADDR_LOOPBACK);
    memcpy(tib->host_addr, &loaddr, sizeof(struct in_addr));
    strcpy(tib->hostbuf, "localhost");
    tib->host.h_name = tib->hostbuf;
    tib->host.h_aliases = tib->host_aliases;
    tib->host_aliases[0] = NULL;
    tib->h_addr_ptrs[0] = (char *) tib->host_addr;
    tib->h_addr_ptrs[1] = NULL;
    tib->host.h_addr_list = tib->h_addr_ptrs;

    return &tib->host;
  }

  // Disallow names consisting only of digits/dots, unless they end in a dot
  if (*name >= '0' && *name <= '9') {
    for (cp = name;; ++cp) {
      if (!*cp) {
        struct in_addr addr;

        if (*--cp == '.') break;

        // All-numeric, no dot at the end. Fake up a hostent as if we'd actually done a lookup.
        addr.s_addr = inet_addr(name);
        if (addr.s_addr == INADDR_NONE) return NULL;

        memcpy(tib->host_addr, &addr, tib->host.h_length);
        strncpy(tib->hostbuf, name, HOSTBUF_SIZE - 1);
        tib->hostbuf[HOSTBUF_SIZE - 1] = '\0';
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

struct hostent *gethostbyaddr(const char *addr, int len, int type) {
  struct tib *tib = gettib();
  const unsigned char *uaddr = (const unsigned char *) addr;
  int n;
  char buf[QUERYBUF_SIZE];
  struct hostent *hp;
  char qbuf[NS_MAXDNAME + 1];

  if (type != AF_INET) {
    errno = EPROTONOSUPPORT;
    return NULL;
  }

  if (len != sizeof(struct in_addr)) {
    errno = EMSGSIZE;
    return NULL;
  }

  sprintf(qbuf, "%u.%u.%u.%u.in-addr.arpa", uaddr[3], addr[2], uaddr[1], uaddr[0]);

  n = res_query(qbuf, DNS_CLASS_IN, DNS_TYPE_PTR, buf, sizeof buf);
  if (n < 0) return NULL;

  hp = getanswer(buf, n, qbuf, DNS_TYPE_PTR);
  if (!hp) return NULL;

  hp->h_addrtype = AF_INET;
  hp->h_length = len;
  
  memmove(tib->host_addr, addr, len);
  tib->h_addr_ptrs[0] = (char *) tib->host_addr;
  tib->h_addr_ptrs[1] = NULL;

  return hp;
}

//
// inet_ntoa
//

char *inet_ntoa(struct in_addr in) {
  char *buf = gettib()->hostbuf;
  sprintf(buf, "%d.%d.%d.%d", in.s_un_b.s_b1, in.s_un_b.s_b2, in.s_un_b.s_b3, in.s_un_b.s_b4);
  return buf;
}

//
// inet_addr
//

unsigned long inet_addr(const char *cp) {
  static const unsigned long max[4] = {0xFFFFFFFF, 0xFFFFFF, 0xFFFF, 0xFF};
  unsigned long val;
  char c;
  union iaddr {
    unsigned char bytes[4];
    unsigned long word;
  } res;
  unsigned char *pp = res.bytes;
  int digit;
  int base;

  res.word = 0;

  c = *cp;
  for (;;) {
    // Collect number up to ``.''.
    // Values are specified as for C: 0x=hex, 0=octal, isdigit=decimal.

    if (c < '0' || c > '9') {
      errno = EINVAL;
      return INADDR_NONE;
    }

    val = 0; base = 10; digit = 0;
    if (c == '0') {
      c = *++cp;
      if (c == 'x' || c == 'X') {
        base = 16, c = *++cp;
      } else {
        base = 8;
        digit = 1;
      }
    }

    for (;;) {
      if (c >= '0' && c <= '9') {
        if (base == 8 && (c == '8' || c == '9')) {
          errno = EINVAL;
          return INADDR_NONE;
        }
        val = (val * base) + (c - '0');
        c = *++cp;
        digit = 1;
      } else if (base == 16 && c >= 'a' && c <= 'f') {
        val = (val << 4) | (c + 10 - 'a');
        c = *++cp;
        digit = 1;
      } else if (base == 16 && c >= 'A' && c <= 'F') {
        val = (val << 4) | (c + 10 - 'A');
        c = *++cp;
        digit = 1;
      } else {
        break;
      }
    }

    if (c == '.') {
      // Internet format:
      //   a.b.c.d
      //   a.b.c        (with c treated as 16 bits)
      //   a.b          (with b treated as 24 bits)

      if (pp > res.bytes + 2 || val > 0xFF) {
        errno = EINVAL;
        return INADDR_NONE;
      }
      *pp++ = (unsigned char) val;
      c = *++cp;
    } else {
      break;
    }
  }

  // Check for trailing characters.
  if (c > ' ') {
    errno = EINVAL;
    return INADDR_NONE;
  }

  // Did we get a valid digit?
  if (!digit) {
    errno = EINVAL;
    return INADDR_NONE;
  }

  // Check whether the last part is in its limits depending on 
  // the number of parts in total.
  if (val > max[pp - res.bytes]) {
    errno = EINVAL;
    return INADDR_NONE;
  }

  return res.word | htonl(val);
}

//
// gethostname
//

int gethostname(char *name, int namelen) {
  char buf[MAXHOSTNAMELEN];
  char *host;
  char *domain;
  struct peb *peb = getpeb();

  if (*peb->hostname) {
    host = peb->hostname;
  } else {
    host = get_property(osconfig(), "os", "hostname", NULL);
  }

  if (*peb->default_domain) {
    domain = peb->default_domain;
  } else {
    host = get_property(osconfig(), "dns", "domain", NULL);
  }

  if (!host) {
    strncpy(name, "localhost", namelen);
  } else if (!domain) {
    strncpy(name, host, namelen);
  } else {
    sprintf(buf, "%s.%s", host, domain);
    strncpy(name, buf, namelen);
  }

  return 0;
}

//
// getprotobyname
//

struct protoent *getprotobyname(const char *name) {
  struct protoent *p;
  char **alias;

  if (!name) {
    errno = EFAULT;
    return NULL;
  }

  p = protocols;
  while (p->p_name != NULL) {
    if (strcmp(p->p_name, name) == 0) return p;
    alias = p->p_aliases;
    while (*alias) {
      if (strcmp(*alias, name) == 0) return p;
      alias++;
    }

    p++;
  }

  errno = ENOENT;
  return NULL;
}

//
// getprotobynumber
//

struct protoent *getprotobynumber(int proto) {
  struct protoent *pp;

  pp = protocols;
  while (pp->p_name != NULL) {
    if (pp->p_proto == proto) return pp;
    pp++;
  }

  errno = ENOENT;
  return NULL;
}

//
// getservbyname
//

struct servent *getservbyname(const char *name, const char *proto) {
  struct servent *sp;
  char **alias;

  if (!name) {
    errno = EFAULT;
    return NULL;
  }

  sp = services;
  while (sp->s_name != NULL) {
    if (strcmp(sp->s_name, name) == 0) {
      if (!proto || strcmp(sp->s_proto, proto) == 0) return sp;
    }

    alias = sp->s_aliases;
    while (*alias) {
      if (strcmp(*alias, name) == 0) {
        if (!proto || strcmp(sp->s_proto, proto) == 0) return sp;
      }
      alias++;
    }

    sp++;
  }

  errno = ENOENT;
  return NULL;
}

//
// getservbyport
//

struct servent *getservbyport(int port, const char *proto) {
  struct servent *sp;

  sp = services;
  while (sp->s_name != NULL) {
    if (sp->s_port == port) {
      if (!proto || strcmp(sp->s_proto, proto) == 0) return sp;
    }
    sp++;
  }

  errno = ENOENT;
  return NULL;
}
