//
// httpd.h
//
// HTTP Server
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

#ifndef HTTPD_H
#define HTTPD_H

#ifdef HTTPD_LIB
#define httpdapi __declspec(dllexport)
#else
#define httpdapi __declspec(dllimport)
#endif

// Methods

#define METHOD_GET   1
#define METHOD_HEAD  2
#define METHOD_POST  3

// HTTP Request states

#define HRS_IDLE           0

#define HRS_HDR_FIRSTWORD  1
#define HRS_HDR_FIRSTWS    2
#define HRS_HDR_SECONDWORD 3
#define HRS_HDR_SECONDWS   4
#define HRS_HDR_THIRDWORD  5
#define HRS_HDR_LINE       6
#define HRS_HDR_LF         7
#define HRS_HDR_CR         8
#define HRS_HDR_CRLF       9
#define HRS_HDR_CRLFCR     10

#define HRS_PROCESSING     11
#define HRS_RESPONSE       12

#define HRS_HDR_BOGUS      13

struct httpd_context;
struct httpd_request;

typedef int (*httpd_handler)(struct httpd_request *req);

// HTTP socket address

typedef union 
{
  struct sockaddr sa;
  struct sockaddr_in sa_in;
} httpd_sockaddr;

// HTTP server

struct httpd_server
{
  struct section *cfg;
  int port;
  int sock;
  int num_workers;
  int iomux;
  struct httpd_context *contexts;
  struct httpd_requests *requests;
};

// HTTP context

struct httpd_context
{
  struct httpd_server *server;
  struct section *cfg;
  struct httpd_context *next;
  char *alias;
  httpd_handler handler;
};

// HTTP request

struct httpd_request
{
  struct httpd_server *server;
  struct httpd_context *context;
  int sock;
  httpd_sockaddr client_addr;
  
  int http11;
  char *encoded_url;
  char *decoded_url;
  
  char *method;
  char *protocol;
  char *pathinfo;
  char *query;

  char *realpath;

  char *referer;
  char *useragent;
  char *accept;
  char *cookie;
  char *authorization;
  char *contenttype;
  int contentlength;
  char *host;
  time_t if_modified_since;
  int keep_alive;
};

httpdapi struct httpd_server *httpd_initialize(struct section *cfg); 
httpdapi struct httpd_context *httpd_add_context(struct httpd_server *server, struct section *cfg, httpd_handler handler); 
httpdapi int httpd_start(struct httpd_server *server);

#endif