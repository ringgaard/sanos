//
// in.h
//
// Internet address family
//
// Copyright (C) 2002 Michael Ringgaard. All rights reserved.
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

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef NETINET_IN_H
#define NETINET_IN_H

#ifndef _IN_PORT_T_DEFINED
#define _IN_PORT_T_DEFINED
typedef unsigned short in_port_t;
#endif

#ifndef _IN_ADDR_T_DEFINED
#define _IN_ADDR_T_DEFINED
typedef unsigned long in_addr_t;
#endif

#ifndef _IN_ADDR_DEFINED
#define _IN_ADDR_DEFINED

struct in_addr {
  union {
    struct { unsigned char s_b1, s_b2, s_b3, s_b4; } s_un_b;
    struct { unsigned short s_w1, s_w2; } s_un_w;
    unsigned long s_addr;
  };
};

#endif

#ifndef _SOCKADDR_IN_DEFINED
#define _SOCKADDR_IN_DEFINED

struct sockaddr_in {
  unsigned short sin_family;
  unsigned short sin_port;
  struct in_addr sin_addr;
  char sin_zero[8];
};

#endif

#define IPPROTO_IP       0
#define IPPROTO_ICMP     1
#define IPPROTO_TCP      6
#define IPPROTO_UDP      17

#define INADDR_ANY       0
#define INADDR_BROADCAST 0xffffffff

#ifndef _XTONX_DEFINED
#define _XTONX_DEFINED

__inline unsigned short htons(unsigned short n) {
  return ((n & 0xFF) << 8) | ((n & 0xFF00) >> 8);
}

__inline unsigned short ntohs(unsigned short n) {
  return ((n & 0xFF) << 8) | ((n & 0xFF00) >> 8);
}

__inline unsigned long htonl(unsigned long n) {
  return ((n & 0xFF) << 24) | ((n & 0xFF00) << 8) | ((n & 0xFF0000) >> 8) | ((n & 0xFF000000) >> 24);
}

__inline unsigned long ntohl(unsigned long n) {
  return ((n & 0xFF) << 24) | ((n & 0xFF00) << 8) | ((n & 0xFF0000) >> 8) | ((n & 0xFF000000) >> 24);
}

#endif

#endif
