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

#define MAX_HTTP_HEADERS 32

// Methods

#define METHOD_GET   1
#define METHOD_HEAD  2
#define METHOD_POST  3

// HTTP connection states

#define HTTP_STATE_IDLE       0
#define HTTP_STATE_HEADER     1
#define HTTP_STATE_PROCESSING 2

#define HDR_STATE_FIRSTWORD   0
#define HDR_STATE_FIRSTWS     1
#define HDR_STATE_SECONDWORD  2
#define HDR_STATE_SECONDWS    3
#define HDR_STATE_THIRDWORD   4
#define HDR_STATE_LINE        5
#define HDR_STATE_LF          6
#define HDR_STATE_CR          7
#define HDR_STATE_CRLF        8
#define HDR_STATE_CRLFCR      9
#define HDR_STATE_BOGUS       10

struct httpd_context;
struct httpd_request;
struct httpd_response;
struct httpd_connection;

typedef int (*httpd_handler)(struct httpd_connection *conn);

// HTTP buffer

struct httpd_buffer
{
  char *floor;
  char *ceil;
  char *start;
  char *end;
};

// HTTP header

struct httpd_header
{
  char *name;
  char *value;
};

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
  int iomux;
  struct httpd_context *contexts;
  struct httpd_connection *connections;

  int num_workers;
  int min_hdrbufsiz;
  int max_hdrbufsiz;
  int reqbufsiz;
  int rspbufsiz;
  int backlog;
  char *indexname;
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
  struct httpd_connection *conn;
  struct httpd_context *context;

  int http11;
  char *encoded_url;
  char *decoded_url;

  char *method;
  char *contextpath;
  char *pathinfo;
  char *query;
  char *protocol;

  char *path_translated;

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

  int nheaders;
  struct httpd_header headers[MAX_HTTP_HEADERS];
};

// HTTP response

struct httpd_response
{
  struct httpd_connection *conn;
};

// HTTP connection

struct httpd_connection
{
  struct httpd_server *server;
  struct httpd_connection *next;
  int sock;
  httpd_sockaddr client_addr;

  struct httpd_request *req;
  struct httpd_response *rsp;

  struct httpd_buffer reqhdr;
  struct httpd_buffer reqbody;
  struct httpd_buffer rsphdr;
  struct httpd_buffer rspbody;
  int state;
  int hdrstate;
};

httpdapi struct httpd_server *httpd_initialize(struct section *cfg); 
httpdapi int httpd_terminate(struct httpd_server *server);
httpdapi struct httpd_context *httpd_add_context(struct httpd_server *server, struct section *cfg, httpd_handler handler); 
httpdapi int httpd_start(struct httpd_server *server);

httpdapi int httpd_recv(struct httpd_response *req, char *data, int len);

httpdapi int httpd_send_header(struct httpd_response *rsp, int state, char *title, char *headers);
httpdapi int httpd_send_error(struct httpd_response *rsp, int state, char *title);
httpdapi int httpd_send(struct httpd_response *rsp, char *data, int len);
httpdapi int httpd_send_file(struct httpd_response *rsp, int fd);
httpdapi int httpd_flush(struct httpd_response *rsp);

#ifdef HTTPD_LIB

// hutils.c

char *getstrconfig(struct section *cfg, char *name, char *defval);
int getnumconfig(struct section *cfg, char *name, int defval);
int decode_url(char *from, char *to);
time_t timerfc(char *s);
char *rfctime(time_t t, char *buf);

// hbuf.c

int buffer_size(struct httpd_buffer *buf);
int buffer_capacity(struct httpd_buffer *buf);
int buffer_left(struct httpd_buffer *buf);
int allocate_buffer(struct httpd_buffer *buf, int size);
void free_buffer(struct httpd_buffer *buf);
void clear_buffer(struct httpd_buffer *buf);
int expand_buffer(struct httpd_buffer *buf, int minfree);
char *bufgets(struct httpd_buffer *buf);
int bufncat(struct httpd_buffer *buf, char *data, int len);
int bufcat(struct httpd_buffer *buf, char *data);

#endif

#endif
