//
// httpd.c
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

#include <os.h>
#include <stdio.h>
#include <string.h>
#include <inifile.h>
#include <ctype.h>
#include <time.h>
#include <os/version.h>

#include <httpd.h>

#define HTTPD_SOFTWARE (OSNAME "/" OSVERSION)

struct critsect srvlock;

char *statustitle(int status)
{
  switch (status)
  {
    case 200: return "OK";
    case 204: return "No Content";
    case 302: return "Moved";
    case 304: return "Not Modified";
    case 400: return "Bad Request";
    case 401: return "Not Authorized";
    case 403: return "Forbidden";
    case 404: return "Not Found";
    case 405: return "Method Not Allowed";
    case 500: return "Internal Server Error";
    case 501: return "Not Implemented";
    case 503: return "Service Unavailable";
    case 505: return "HTTP Version Not Supported";
  }

  return "Internal Error";
}

struct httpd_server *httpd_initialize(struct section *cfg)
{
  struct httpd_server *server;

  server = (struct httpd_server *) malloc(sizeof(struct httpd_server));
  if (!server) return NULL;
  memset(server, 0, sizeof(struct httpd_server));

  server->cfg = cfg;
  server->port = getnumconfig(cfg, "port", 80);

  server->num_workers = getnumconfig(cfg, "workerthreads", 1);
  server->min_hdrbufsiz = getnumconfig(cfg, "minhdrsize", 1*K);
  server->max_hdrbufsiz = getnumconfig(cfg, "maxhdrsize", 16*K);
  server->reqbufsiz = getnumconfig(cfg, "requestbuffer", 4*K);
  server->rspbufsiz = getnumconfig(cfg, "responsebuffer", 4*K);
  server->backlog = getnumconfig(cfg, "backlog", 5);
  server->indexname = getstrconfig(cfg, "indexname", "index.htm");

  return server;
}

int httpd_terminate(struct httpd_server *server)
{
  return -ENOSYS;
}

struct httpd_context *httpd_add_context(struct httpd_server *server, struct section *cfg, httpd_handler handler)
{
  struct httpd_context *context;

  context = (struct httpd_context *) malloc(sizeof(struct httpd_context));
  if (!context) return NULL;
  memset(context, 0, sizeof(struct httpd_context));

  context->server = server;
  context->cfg = cfg;
  context->alias = getstrconfig(cfg, "alias", "/");
  context->handler = handler;
  context->next = server->contexts;
  server->contexts = context;

  return context;
}

void httpd_accept(struct httpd_server *server)
{
  int sock;
  httpd_sockaddr addr;
  struct httpd_connection *conn;
  int addrlen;

  printf("httpd_accept\n");

  addrlen = sizeof(addr);
  sock = accept(server->sock, &addr.sa, &addrlen);
  if (sock < 0) return;
    
  printf("%a port %d connected\n", &addr.sa_in.sin_addr.s_addr, ntohs(addr.sa_in.sin_port));

  conn = (struct httpd_connection *) malloc(sizeof(struct httpd_connection));
  if (!conn) return;
  memset(conn, 0, sizeof(struct httpd_connection));

  conn->server = server;
  memcpy(&conn->client_addr, &addr, sizeof(httpd_sockaddr));
  conn->sock = sock;
  
  enter(&srvlock);
  conn->next = server->connections;
  server->connections = conn;
  leave(&srvlock);

  dispatch(server->iomux, conn->sock, IOEVT_READ | IOEVT_CLOSE | IOEVT_ERROR, (int) conn);
}

void httpd_clear_connection(struct httpd_connection *conn)
{
  if (conn->req)
  {
    if (conn->req->decoded_url) free(conn->req->decoded_url);
    conn->req = NULL;
  }

  if (conn->rsp)
  {
    conn->rsp = NULL;
  }
}

void httpd_close_connection(struct httpd_connection *conn)
{
  printf("close %a port %d\n", &conn->client_addr.sa_in.sin_addr.s_addr, ntohs(conn->client_addr.sa_in.sin_port));

  close(conn->sock);
  free_buffer(&conn->reqhdr);
  free_buffer(&conn->reqbody);
  free_buffer(&conn->rsphdr);
  free_buffer(&conn->rspbody);
  free(conn);
  // TODO: remove connection from server
}

int httpd_recv(struct httpd_response *req, char *data, int len)
{
  int rc;
  int n;

  // First copy any remaining data in the request header buffer
  n = buffer_left(&req->conn->reqhdr);
  if (n > 0)
  {
    if (n > len) n = len;
    memcpy(data, req->conn->reqhdr.end, n);
    req->conn->reqhdr.end += n;
    return n;
  }

  // Allocate request body buffer if it has not already been done
  if (!req->conn->reqbody.floor)
  {
    rc = allocate_buffer(&req->conn->reqbody, req->conn->server->reqbufsiz);
    if (rc < 0) return rc;
  }

  // Receive new data if request body buffer empty
  if (buffer_avail(
}

int httpd_send_header(struct httpd_response *rsp, int state, char *title, char *headers)
{
  int rc;
  char buf[2048];
  char datebuf[32];

  sprintf(buf, "HTTP/%s %d %s\r\nServer: %s\r\nDate: %s\r\n", 
    rsp->conn->req->http11 ? "1.1" : "1.0",
    state, title, HTTPD_SOFTWARE,
    rfctime(time(0), datebuf));

  rc = bufcat(&rsp->conn->rsphdr, buf);
  if (rc < 0) return rc;

  if (headers)
  {
    rc = bufcat(&rsp->conn->rsphdr, headers);
    if (rc < 0) return rc;
  }

  rc = bufcat(&rsp->conn->rsphdr, "\r\n");
  if (rc < 0) return rc;

  return 0;
}

int httpd_send_error(struct httpd_response *rsp, int state, char *title)
{
  int rc;

  rc = httpd_send_header(rsp, state, title ? title : statustitle(state), NULL);
  if (rc < 0) return rc;

  return 0;
}

int httpd_send(struct httpd_response *rsp, char *data, int len)
{
  return -ENOSYS;
}

int httpd_flush(struct httpd_response *rsp)
{
  return -ENOSYS;
}

int httpd_send_file(struct httpd_response *rsp, int fd)
{
  return -ENOSYS;
}

int httpd_check_header(struct httpd_connection *conn)
{
  char c;

  while (conn->reqhdr.start < conn->reqhdr.end)
  {
    c = *conn->reqhdr.start++;
    switch (conn->hdrstate)
    {
      case HDR_STATE_FIRSTWORD:
        switch (c)
	{
	  case ' ': 
	  case '\t':
	    conn->hdrstate = HDR_STATE_FIRSTWS;
	    break;

	  case '\n': 
	  case '\r':
	    conn->hdrstate = HDR_STATE_BOGUS;
	    return -EINVAL;
	}
        break;

      case HDR_STATE_FIRSTWS:
        switch (c)
	{
	  case ' ': 
	  case '\t':
	    break;

	  case '\n': 
	  case '\r':
 	    conn->hdrstate = HDR_STATE_BOGUS;
	    return -EINVAL;
	
	  default:
	    conn->hdrstate = HDR_STATE_SECONDWORD;
	}
        break;

      case HDR_STATE_SECONDWORD:
        switch (c)
	{
	  case ' ': 
	  case '\t':
	    conn->hdrstate = HDR_STATE_SECONDWS;
	    break;

	  case '\n': 
	  case '\r':
	    // The first line has only two words - an HTTP/0.9 request
	   return 1;
	}
        break;

      case HDR_STATE_SECONDWS:
        switch (c)
	{
  	  case ' ':
	  case '\t':
	    break;

	  case '\n': 
	  case '\r':
	    conn->hdrstate = HDR_STATE_BOGUS;
	    return -EINVAL;

	  default:
	    conn->hdrstate = HDR_STATE_THIRDWORD;
	}
        break;

      case HDR_STATE_THIRDWORD:
        switch (c)
	{
	  case ' ': 
	  case '\t':
	    conn->hdrstate = HDR_STATE_BOGUS;
	    return -EINVAL;

	  case '\n':
	    conn->hdrstate = HDR_STATE_LF;
	    break;

	  case '\r':
	    conn->hdrstate = HDR_STATE_CR;
	    break;
	}
        break;

      case HDR_STATE_LINE:
        switch (c)
	{
	  case '\n':
	    conn->hdrstate = HDR_STATE_LF;
	    break;

	  case '\r':
	    conn->hdrstate = HDR_STATE_CR;
	    break;
	}
        break;

      case HDR_STATE_LF:
        switch (c)
	{
	  case '\n':
	    // Two newlines in a row - a blank line - end of request
	   return 1;

	  case '\r':
	    conn->hdrstate = HDR_STATE_CR;
	    break;

	  default:
	    conn->hdrstate = HDR_STATE_LINE;
	}
        break;

      case HDR_STATE_CR:
        switch (c)
	{
	  case '\n':
	    conn->hdrstate = HDR_STATE_CRLF;
	    break;

	  case '\r':
	    // Two returns in a row - end of request
	    return 1;

	  default:
	    conn->hdrstate = HDR_STATE_LINE;
	}
        break;

      case HDR_STATE_CRLF:
        switch (c)
	{
	  case '\n':
	    // Two newlines in a row - end of request
	    return 1;
	
	  case '\r':
	    conn->hdrstate = HDR_STATE_CRLFCR;
	    break;

	  default:
	    conn->hdrstate = HDR_STATE_LINE;
	}
        break;
    
      case HDR_STATE_CRLFCR:
        switch (c)
	{
	  case '\n': 
	  case '\r':
	    // Two CRLFs or two CRs in a row - end of request
	    return 1;

	  default:
	    conn->hdrstate = HDR_STATE_LINE;
	}
        break;

      case HDR_STATE_BOGUS:
        return -EINVAL;
    }
  }
  
  return 0;
}

int httpd_parse_request(struct httpd_request *req)
{
  struct httpd_buffer *buf;
  char *s;
  char *l;
  int rc;
  
  buf = &req->conn->reqhdr;
  buf->start = buf->floor;

  s = bufgets(buf);
  if (!s) return -EINVAL;

  // Parse method
  req->method = s;
  s = strchr(s, ' ');
  if (!s) return -EINVAL;
  *s++ = 0;

  // Parse URL
  while (*s == ' ') s++;
  req->encoded_url = s;
  s = strchr(s, ' ');

  // Parse protocol version
  if (*s)
  {
    *s++ = 0;
    while (*s == ' ') s++;
    if (*s)
      req->protocol = s;
    else
      req->protocol = "HTTP/0.9";

    if (strcmp(req->protocol, "HTTP/1.1") == 0) req->http11 = 1;
  }

  // Decode and split URL
  req->decoded_url = (char *) malloc(strlen(req->encoded_url) + 1);
  if (!req->decoded_url) return -ENOMEM;

  rc = decode_url(req->encoded_url, req->decoded_url);
  if (rc < 0) return rc;

  req->pathinfo = req->decoded_url;
  s = strchr(req->pathinfo, '?');
  if (s)
  {
    *s++ = 0;
    req->query = s;
  }

  printf("method: [%s]\n", req->method);
  printf("pathinfo: [%s]\n", req->pathinfo);
  printf("query: [%s]\n", req->query);
  printf("protocol: [%s]\n", req->protocol);

  // Parse headers
  req->nheaders = 0;
  while ((l = bufgets(buf)))
  {
    if (!*l) continue;
   
    s = strchr(l, ':');
    if (!s) continue;
    *s++ = 0;
    while (*s == ' ') s++;
    if (!*s) continue;

    printf("hdr: [%s]=[%s]\n", l, s);

    if (stricmp(l, "Referer") == 0)
      req->referer = s;
    else if (stricmp(l, "User-agent") == 0)
      req->useragent = s;
    else if (stricmp(l, "Accept") == 0)
      req->accept = s;
    else if (stricmp(l, "Cookie") == 0)
      req->cookie = s;
    else if (stricmp(l, "Authorization") == 0)
      req->authorization = s;
    else if (stricmp(l, "Content-type") == 0)
      req->contenttype = s;
    else if (stricmp(l, "Content-length") == 0)
      req->contentlength = atoi(s);
    else if (stricmp(l, "Host") == 0)
      req->host = s;
    else if (stricmp(l, "If-modified-since") == 0)
      req->if_modified_since = timerfc(s);
    else if (stricmp(l, "Connection") == 0)
      req->keep_alive = stricmp(s, "keep-alive") == 0;

    if (req->nheaders < MAX_HTTP_HEADERS)
    {
      req->headers[req->nheaders].name = l;
      req->headers[req->nheaders].value = s;
      req->nheaders++;
    }
  }

  return 0;
}

int httpd_find_context(struct httpd_request *req)
{
  int n;
  char *s;
  struct httpd_context *context = req->conn->server->contexts;
  char *pathinfo = req->pathinfo;


  while (context)
  {
    n = strlen(context->alias);
    s = pathinfo + n;
    if (strncmp(context->alias, pathinfo, n) == 0 && (*s == '/' || *s == 0))
    {
      if (*s)
      {
	*s++ = 0;
	req->pathinfo = s;
	req->contextpath = pathinfo;
      }
      else
      {
	req->contextpath = pathinfo;
	req->pathinfo = "";
      }

      req->context = context;
      return 0;
    }

    context = context->next;
  }

  return -ENOENT;
}

int httpd_process(struct httpd_connection *conn)
{
  struct httpd_request req;
  struct httpd_response rsp;
  int rc;

  memset(&req, 0, sizeof(struct httpd_request));
  memset(&rsp, 0, sizeof(struct httpd_response));

  req.conn = conn;
  rsp.conn = conn;
  conn->req = &req;
  conn->rsp = &rsp;

  rc = httpd_parse_request(&req);
  if (rc < 0) goto errorexit;

  rc = httpd_find_context(&req);
  if (rc < 0)
  {
    httpd_send_error(&rsp, 404, "Not Found");
  }
  else
  {
  }

  clear_connection(conn);
  return 0;

errorexit:
  clear_connection(conn);
  return rc;
}

int httpd_io(struct httpd_connection *conn)
{
  int rc;

  switch (conn->state)
  {
    case HTTP_STATE_IDLE:
      rc = allocate_buffer(&conn->reqhdr, conn->server->min_hdrbufsiz);
      if (rc < 0) return rc;
      conn->state = HTTP_STATE_HEADER;
      conn->hdrstate = HDR_STATE_FIRSTWORD;
      // Fall through

    case HTTP_STATE_HEADER:
      if (buffer_allocated(&conn->reqhdr) >= conn->server->max_hdrbufsiz) return -EBUF;
      rc = expand_buffer(&conn->reqhdr, 1);
      if (rc < 0) return rc;

      rc = recv(conn->sock, conn->reqhdr.end, buffer_left(&conn->reqhdr), 0);
      printf("httpd_io %d bytes\n", rc);
      if (rc <= 0) return rc;

      conn->reqhdr.end += rc;

      rc = httpd_check_header(conn);
      printf("httpd_check_header returned %d\n", rc);
      if (rc < 0) return rc;
      if (rc > 0)
      {
        rc = httpd_process(conn);
        if (rc < 0) return rc;
      }
      else
      {
        rc = dispatch(conn->server->iomux, conn->sock, IOEVT_READ | IOEVT_CLOSE | IOEVT_ERROR, (int) conn);
        if (rc < 0) return rc;
      }

      return 1;

    default:
      return -EINVAL;
  }

  return 1;
}

void __stdcall httpd_worker(void *arg)
{
  struct httpd_server *server = (struct httpd_server *) arg;
  struct httpd_connection *conn;
  int rc;

  while (1)
  {
    rc = wait(server->iomux, INFINITE);
    //printf("iomux: wait returned %d\n", rc);
    if (rc < 0) break;

    conn = (struct httpd_connection *) rc;
    if (conn == NULL)
    {
      httpd_accept(server);
      dispatch(server->iomux, server->sock, IOEVT_ACCEPT, 0);
    }
    else
    {
      rc = httpd_io(conn);
      if (rc <= 0)
      {
	printf("httpd: error %d in i/o\n", rc);
        httpd_close_connection(conn);
      }
    }
  }
}

int httpd_start(struct httpd_server *server)
{
  int sock;
  int rc;
  int i;
  httpd_sockaddr addr;
  int hthread;

  sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0) return sock;

  addr.sa_in.sin_family = AF_INET;
  addr.sa_in.sin_port = htons(server->port);
  addr.sa_in.sin_addr.s_addr = htonl(INADDR_ANY);

  rc = bind(sock, &addr.sa, sizeof(addr));
  if (rc < 0)
  {
    close(sock);
    return rc;
  }

  rc = listen(sock, server->backlog);
  if (rc < 0)
  {
    close(sock);
    return rc;
  }

  ioctl(sock, FIONBIO, NULL, 0);

  server->sock = sock;
  server->iomux = mkiomux(0);
  dispatch(server->iomux, server->sock, IOEVT_ACCEPT, 0);

  for (i = 0; i < server->num_workers; i++)
  {
    hthread = beginthread(httpd_worker, 0, server, 0, NULL);
    close(hthread);
  }

  return 0;
}

int __stdcall DllMain(handle_t hmod, int reason, void *reserved)
{
  struct httpd_server *server;

  if (reason == DLL_PROCESS_ATTACH)
  {
    mkcs(&srvlock);
    server = httpd_initialize(NULL);
    httpd_add_context(server, NULL, NULL);
    httpd_start(server);
  }

  return TRUE;
}
