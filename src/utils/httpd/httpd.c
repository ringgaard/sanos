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

#include <httpd.h>

#define HTTPD_BUFFER_INITIAL_SIZE 4096

struct critsect srvlock;

char *getstrconfig(struct section *cfg, char *name, char *defval)
{
  char *val;

  if (!cfg) return defval;
  val = find_property(cfg, name);
  if (!val) return defval;
  return val;
}

int getnumconfig(struct section *cfg, char *name, int defval)
{
  char *val;

  if (!cfg) return defval;
  val = find_property(cfg, name);
  if (!val) return defval;
  return atoi(val);
}

int allocate_buffer(struct httpd_buffer *buf, int size)
{
  if (size == 0)
  {
    buf->floor = buf->ceil = NULL;
    buf->start = buf->end = NULL;
  }
  else
  {
    buf->floor = (char *) malloc(size);
    if (!buf->floor) return -ENOMEM;
    buf->ceil = buf->floor + size;
    buf->start = buf->end = buf->floor;
  }

  return 0;
}

void free_buffer(struct httpd_buffer *buf)
{
  if (buf->floor) free(buf->floor);
  buf->floor = buf->ceil = buf->start = buf->end = NULL;
}

int expand_buffer(struct httpd_buffer *buf, int minfree)
{
  char *p;
  int size;
  int minsize;

  if (buf->ceil - buf->end >= minfree) return 0;
  
  size = buf->ceil - buf->floor;
  minsize = buf->end + minfree - buf->floor;
  while (size < minsize)
  {
    if (size == 0)
      size = HTTPD_BUFFER_INITIAL_SIZE;
    else
      size *= 2;
  }

  p = (char *) realloc(buf->floor, size);
  if (!p) return -ENOMEM;

  buf->start += p - buf->floor;
  buf->end += p - buf->floor;
  buf->floor = p;
  buf->ceil = p + size;

  return 0;
}

char *bufgets(struct httpd_buffer *buf)
{
  char *start;
  char *s;

  s = start = buf->start;
  while (s < buf->end)
  {
    switch (*s)
    {
      case '\n':
	if (s[1] != ' ' && s[1] != '\t')
	{
	  if (s > start && s[-1] == ' ')
	    s[-1] = 0;
	  else
	    s[0] = 0;

	  buf->start = s + 1;
	  return start;
	}
	else
	  *s++ = ' ';

	break;

      case '\r':
      case '\t':
	*s++ = ' ';
	break;

      default:
	s++;
    }
  }

  return NULL;
}

int hexdigit(int x)
{
  return (x <= '9') ? x - '0' : (x & 7) + 9;
}

int decode_url(char *from, char *to)
{
  char c, x1, x2;

  while ((c = *from++) != 0) 
  {
    if (c == '%') 
    {
      x1 = *from++;
      if (!isxdigit(x1)) return -EINVAL;
      x2 = *from++;
      if (!isxdigit(x2)) return -EINVAL;
      *to++ = (hexdigit(x1) << 4) + hexdigit(x2);
    } 
    else
      *to++ = c;
  }

  *to = 0;
  return 0;
}

time_t timerfc(char *s)
{
  struct tm tm;
  char month[3];
  char c;
  unsigned n;
  char flag;
  char state;
  char isctime;
  enum { D_START, D_END, D_MON, D_DAY, D_YEAR, D_HOUR, D_MIN, D_SEC };

  tm.tm_sec = 60;
  tm.tm_min = 60;
  tm.tm_hour = 24;
  tm.tm_mday = 32;
  tm.tm_year = 1969;
  isctime = 0;
  month[0] = 0;
  state = D_START;
  n = 0;
  flag = 1;
  while (*s && state != D_END)
  {
    c = *s++;
    switch (state) 
    {
      case D_START:
	if (c == ' ') 
	{
	  state = D_MON;
	  isctime = 1;
	} 
	else if (c == ',') 
	  state = D_DAY;
	break;

      case D_MON:
	if (isalpha(c)) 
	{
	  if (n < 3) month[n++] = c;
	} 
	else 
	{
	  if (n < 3) return -1;
	  n = 0;
	  state = isctime ? D_DAY : D_YEAR;
	}
	break;

      case D_DAY:
	if (c == ' ' && flag)
	{
	}
	else if (isdigit(c)) 
	{
	  flag = 0;
	  n = 10 * n + (c - '0');
	} 
	else 
	{
	  tm.tm_mday = n;
	  n = 0;
	  state = isctime ? D_HOUR : D_MON;
	}
	break;

      case D_YEAR:
	if (isdigit(c))
	  n = 10 * n + (c - '0');
	else 
	{
	  tm.tm_year = n;
   	  n = 0;
  	  state = isctime ? D_END : D_HOUR;
	}
	break;

      case D_HOUR:
	if (isdigit(c))
  	  n = 10 * n + (c - '0');
	else 
	{
	  tm.tm_hour = n;
	  n = 0;
	  state = D_MIN;
	}
	break;

      case D_MIN:
	if (isdigit(c))
  	  n = 10 * n + (c - '0');
	else 
	{
	  tm.tm_min = n;
	  n = 0;
	  state = D_SEC;
	}
	break;

      case D_SEC:
	if (isdigit(c))
  	  n = 10 * n + (c - '0');
	else
	{
	  tm.tm_sec = n;
	  n = 0;
	  state = isctime ? D_YEAR : D_END;
	}
	break;
    }
  }

  switch (month[0]) 
  {
    case 'A': tm.tm_mon = (month[1] == 'p') ? 4 : 8; break;
    case 'D': tm.tm_mon = 12; break;
    case 'F': tm.tm_mon = 2; break;
    case 'J': tm.tm_mon = (month[1] == 'a') ? 1 : ((month[2] == 'l') ? 7 : 6); break;
    case 'M': tm.tm_mon = (month[2] == 'r') ? 3 : 5; break;
    case 'N': tm.tm_mon = 11; break;
    case 'O': tm.tm_mon = 10; break;
    case 'S': tm.tm_mon = 9; break;
    default: return -1;
  }
  if (tm.tm_year <= 100) tm.tm_year += (tm.tm_year < 70) ? 2000 : 1900;

  tm.tm_year -= 1900;
  tm.tm_mon--;
  tm.tm_mday--;

  return mktime(&tm);
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

void httpd_close_connection(struct httpd_connection *conn)
{
  printf("close %a port %d\n", &conn->client_addr.sa_in.sin_addr.s_addr, ntohs(conn->client_addr.sa_in.sin_port));

  close(conn->sock);
  free_buffer(&conn->reqhdr);
  free(conn);
  // TODO: remove connection from server
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
  
  printf("httpd_parse_request\n");
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

    printf("hdr: [%s]\n", s);
   
    s = strchr(l, ':');
    if (!s) continue;
    *s++ = 0;
    while (*s == ' ') s++;
    if (!*s) continue;

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
      req->nheaders;
    }
  }

  return 0;
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

  rc = httpd_parse_request(&req);
  if (rc < 0) return rc;

  return 0;
}

int httpd_io(struct httpd_connection *conn)
{
  int rc;

  if (conn->state == HTTP_STATE_IDLE)
  {
    rc = allocate_buffer(&conn->reqhdr, HTTPD_BUFFER_INITIAL_SIZE);
    if (rc < 0) return rc;
    conn->state = HTTP_STATE_HEADER;
    conn->hdrstate = HDR_STATE_FIRSTWORD;
  }

  if (conn->state == HTTP_STATE_HEADER)
  {
    rc = recv(conn->sock, conn->reqhdr.end, conn->reqhdr.ceil - conn->reqhdr.end, 0);
    printf("httpd_io %d bytes\n", rc);
    if (rc <= 0) return rc;

    write(1, conn->reqhdr.end, rc);
    conn->reqhdr.end += rc;

    rc = httpd_check_header(conn);
    printf("httpd_check_header returned %d\n", rc);
    if (rc < 0) return rc;
    if (rc > 0)
    {
      rc = httpd_process(conn);
      if (rc < 0) return rc;
    }
  }

  rc = dispatch(conn->server->iomux, conn->sock, IOEVT_READ | IOEVT_CLOSE | IOEVT_ERROR, (int) conn);
  if (rc < 0) return rc;

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

  rc = listen(sock, 5);
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
