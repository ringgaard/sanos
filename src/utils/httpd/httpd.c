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
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inifile.h>
#include <ctype.h>
#include <time.h>

#include <httpd.h>

struct mimetype {
  char *ext;
  char *mime;
};

struct mimetype mimetypes[] = {
  {"txt",  "text/plain"},
  {"html", "text/html"},
  {"htm",  "text/html"},
  {"rtx",  "text/richtext"},
  {"css",  "text/css"},
  {"xml",  "text/xml"},
  {"dtd",  "text/xml"},

  {"gif",  "image/gif"},
  {"jpg",  "image/jpeg"},
  {"jpeg", "image/jpeg"},
  {"tif",  "image/tiff"},
  {"tiff", "image/tiff"},
  {"pbm",  "image/x-portable-bitmap"},
  {"pgm",  "image/x-portable-graymap"},
  {"ppm",  "image/x-portable-pixmap"},
  {"pnm",  "image/x-portable-anymap"},
  {"xbm",  "image/x-xbitmap"},
  {"xpm",  "image/x-xpixmap"},
  {"png",  "image/png"},

  {"au",   "audio/basic"},
  {"snd",  "audio/basic"},
  {"aif",  "audio/x-aiff"},
  {"aiff", "audio/x-aiff"},
  {"aifc", "audio/x-aiff"},
  {"ra",   "audio/x-pn-realaudio"},
  {"ram",  "audio/x-pn-realaudio"},
  {"rm",   "audio/x-pn-realaudio"},
  {"rpm",  "audio/x-pn-realaudio-plugin"},
  {"wav",  "audio/wav"},
  {"mid",  "audio/midi"},
  {"midi", "audio/midi"},
  {"mpga", "audio/mpeg"},
  {"mp2",  "audio/mpeg"},
  {"mp3",  "audio/mpeg"},

  {"mpeg", "video/mpeg"},
  {"mpg",  "video/mpeg"},
  {"mpe",  "video/mpeg"},
  {"qt",   "video/quicktime"},
  {"mov",  "video/quicktime"},
  {"avi",  "video/x-msvideo"},

  {"dll",  "application/octet-stream"},
  {"exe",  "application/octet-stream"},
  {"sys",  "application/octet-stream"},
  {"class","application/java"},
  {"js",   "application/x-javascript"},
  {"eps",  "application/postscript"},
  {"ps",   "application/postscript"},
  {"spl",  "application/futuresplash"},
  {"swf",  "application/x-shockwave-flash"},
  {"dvi",  "application/x-dvi"},
  {"gtar", "application/x-gtar"},
  {"hqx",  "application/mac-binhex40"},
  {"latex","application/x-latex"},
  {"oda",  "application/oda"},
  {"pdf",  "application/pdf"},
  {"rtf",  "application/rtf"},
  {"sit",  "application/x-stuffit"},
  {"tar",  "application/x-tar"},
  {"tex",  "application/x-tex"},
  {"zip",  "application/x-zip-compressed"},
  {"doc",  "application/msword"},
  {"ppt",  "application/powerpoint"},

  {"wrl",  "model/vrml"},
  {"vrml", "model/vrml"},
  {"mime", "message/rfc822"},
  {"ini",  "text/plain"},

  {"wml",  "text/vnd.wap.wml"},
  {"wmlc", "application/vnd.wap.wmlc"},
  {"wmls", "text/vnd.wap.wmlscript"},
  {"wmlsc","application/vnd.wap.wmlscriptc"},
  {"wbmp", "image/vnd.wap.wbmp"},

  {NULL, NULL}
};

char *statustitle(int status) {
  switch (status) {
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

struct httpd_server *httpd_initialize(struct section *cfg) {
  struct httpd_server *server;
  char *name;

  server = (struct httpd_server *) malloc(sizeof(struct httpd_server));
  if (!server) return NULL;
  memset(server, 0, sizeof(struct httpd_server));

  mkcs(&server->srvlock);
  server->cfg = cfg;
  server->port = getnumconfig(cfg, "port", 80);
  server->num_workers = getnumconfig(cfg, "workerthreads", 1);
  server->min_hdrbufsiz = getnumconfig(cfg, "minhdrsize", 1024);
  server->max_hdrbufsiz = getnumconfig(cfg, "maxhdrsize", 16 * 1024);
  server->reqbufsiz = getnumconfig(cfg, "requestbuffer", 4096);
  server->rspbufsiz = getnumconfig(cfg, "responsebuffer", 4096);
  server->backlog = getnumconfig(cfg, "backlog", 5);
  server->indexname = getstrconfig(cfg, "indexname", "index.htm");
  server->swname = getstrconfig(cfg, "swname", gettib()->peb->osname);
  server->allowdirbrowse = getnumconfig(cfg, "allowdirbrowse", 1);

  parse_log_columns(server, getstrconfig(cfg, "logcolumns", "date time c-ip cs-username s-ip s-port cs-method cs-uri-stem cs-uri-query sc-status cs(user-agent)"));
  server->logdir = getstrconfig(cfg, "logdir", NULL);
  server->logfd = -1;

  if (cfg) {
    name = getstrconfig(cfg, "mimemap", "mimetypes");
    server->mimemap = find_section(cfg, name);
  }

  return server;
}

int httpd_terminate(struct httpd_server *server) {
  return -ENOSYS;
}

char *httpd_get_mimetype(struct httpd_server *server, char *ext) {
  struct property *prop;
  struct mimetype *m;

  if (!ext) return NULL;

  // Find MIME type in servers mime map
  if (server && server->mimemap) {
    prop = server->mimemap->properties;
    while (prop) {
      if (stricmp(ext, prop->name) == 0) return prop->value;
      prop = prop->next;
    }
  }

  // Find MIME type in default MIME type table
  m = mimetypes;
  while (m->ext) {
    if (stricmp(ext, m->ext) == 0) return m->mime;
    m++;
  }

  return NULL;
}

struct httpd_context *httpd_add_context(struct httpd_server *server, char *alias, httpd_handler handler, void *userdata, struct section *cfg) {
  struct httpd_context *context;

  context = (struct httpd_context *) malloc(sizeof(struct httpd_context));
  if (!context) return NULL;
  memset(context, 0, sizeof(struct httpd_context));

  context->server = server;
  context->cfg = cfg;
  context->location = "/";
  context->alias = getstrconfig(cfg, "alias", alias);
  if (strcmp(context->alias, "/") == 0) context->alias = "";
  context->handler = handler;
  context->userdata = userdata;
  context->next = server->contexts;
  server->contexts = context;

  return context;
}

struct httpd_context *httpd_add_file_context(struct httpd_server *server, char *alias, char *location, struct section *cfg) {
  struct httpd_context *context;

  context = httpd_add_context(server, alias, httpd_file_handler, NULL, cfg);
  if (!context) return NULL;

  context->location = getstrconfig(cfg, "location", location);
  if (strcmp(context->location, "/") == 0) context->location = "";

  return context;
}

struct httpd_context *httpd_add_resource_context(struct httpd_server *server, char *alias, hmodule_t hmod, struct section *cfg) {
  struct httpd_context *context;
  char modfn[MAXPATH];
  struct stat statbuf;

  if (getmodpath(hmod, modfn, MAXPATH) < 0) return NULL;
  if (stat(modfn, &statbuf) < 0) return NULL;

  context = httpd_add_context(server, alias, httpd_resource_handler, NULL, cfg);
  if (!context) return NULL;

  context->hmod = hmod;
  context->mtime = statbuf.st_mtime;

  return context;
}

void httpd_accept(struct httpd_server *server) {
  int sock;
  httpd_sockaddr addr;
  struct httpd_connection *conn;
  int addrlen;

  addrlen = sizeof(addr);
  sock = accept(server->sock, &addr.sa, &addrlen);
  if (sock < 0) return;

  //printf("connect %s port %d\n", inet_ntoa(addr.sa_in.sin_addr), ntohs(addr.sa_in.sin_port));

  conn = (struct httpd_connection *) malloc(sizeof(struct httpd_connection));
  if (!conn) return;
  memset(conn, 0, sizeof(struct httpd_connection));

  conn->server = server;
  memcpy(&conn->client_addr, &addr, sizeof(httpd_sockaddr));
  conn->sock = sock;

  addrlen = sizeof(conn->server_addr);
  getsockname(conn->sock, &conn->server_addr.sa, &addrlen);

  enter(&server->srvlock);
  if (server->connections) server->connections->prev = conn;
  conn->next = server->connections;
  conn->prev = NULL;
  server->connections = conn;
  leave(&server->srvlock);

  dispatch(server->iomux, conn->sock, IOEVT_READ | IOEVT_CLOSE | IOEVT_ERROR, (int) conn);
}

void httpd_finish_processing(struct httpd_connection *conn) {
  if (conn->req) {
    if (conn->req->decoded_url) free(conn->req->decoded_url);
    if (conn->req->path_translated) free(conn->req->path_translated);
    if (conn->req->username) free(conn->req->username);

    conn->req = NULL;
  }

  if (conn->rsp) {
    conn->rsp = NULL;
  }
}

int httpd_terminate_request(struct httpd_connection *conn)
{
  int off = 0;

  if (!conn->keep) return 0;
  
  if (conn->fd >= 0) {
    close(conn->fd);
    conn->fd = -1;
  }

  conn->fixed_rsp_data = NULL;
  conn->fixed_rsp_len = 0;

  ioctl(conn->sock, FIONBIO, &off, sizeof(off));

  //printf("terminate request, %d bytes in reqhdr\n", buffer_size(&conn->reqhdr));

  free_buffer(&conn->reqhdr);
  free_buffer(&conn->reqbody);
  free_buffer(&conn->rsphdr);
  free_buffer(&conn->rspbody);

  conn->keep = 0;
  conn->hdrsent = 0;

  return 1;
}

void httpd_close_connection(struct httpd_connection *conn) {
  struct httpd_server *server = conn->server;

  //printf("close %s port %d\n", inet_ntoa(conn->client_addr.sa_in.sin_addr), ntohs(conn->client_addr.sa_in.sin_port));

  close(conn->sock);
  if (conn->fd >= 0) {
    close(conn->fd);
    conn->fd = -1;
  }
  free_buffer(&conn->reqhdr);
  free_buffer(&conn->reqbody);
  free_buffer(&conn->rsphdr);
  free_buffer(&conn->rspbody);

  enter(&server->srvlock);
  if (conn->next) conn->next->prev = conn->prev;
  if (conn->prev) conn->prev->next = conn->next;
  if (conn == server->connections) server->connections = conn->next;
  leave(&server->srvlock);

  free(conn);
}

int httpd_recv(struct httpd_request *req, char *data, int len) {
  struct httpd_buffer *buf;
  int rc;
  int n;

  // First copy any remaining data in the request header buffer
  buf = &req->conn->reqhdr;
  n = buf->end - buf->start;
  if (n > 0) {
    if (n > len) n = len;
    memcpy(data, buf->start, n);
    buf->start += n;
    return n;
  }

  // Allocate request body buffer if it has not already been done
  buf = &req->conn->reqbody;
  if (!buf->floor) {
    rc = allocate_buffer(buf, req->conn->server->reqbufsiz);
    if (rc < 0) return rc;
  }

  // Receive new data if request body buffer empty
  if (buf->end == buf->start) {
    n = recv(req->conn->sock, buf->floor, buf->ceil - buf->floor, 0);
    if (n < 0) return n;
    if (n == 0) {
      errno = ECONNRESET;
      return -1;
    }

    buf->start = buf->floor;
    buf->end = buf->floor + rc;
  }

  // Copy data from request body buffer
  n = buf->end - buf->start;
  if (n > len) n = len;
  memcpy(data, buf->start, n);
  buf->start += n;

  return n;
}

int httpd_send_header(struct httpd_response *rsp, int state, char *title, char *headers) {
  int rc;
  char buf[2048];
  char datebuf[32];

  rsp->status = state;

  sprintf(buf, "HTTP/%s %d %s\r\nServer: %s\r\nDate: %s\r\n", 
    rsp->conn->req->http11 ? "1.1" : "1.0",
    state, title, rsp->conn->server->swname,
    rfctime(time(0), datebuf));

  rc = bufcat(&rsp->conn->rsphdr, buf);
  if (rc < 0) return rc;

  if (rsp->last_modified) {
    sprintf(buf, "Last-Modified: %s\r\n", rfctime(rsp->last_modified, datebuf));
    rc = bufcat(&rsp->conn->rsphdr, buf);
    if (rc < 0) return rc;
  }

  if (rsp->content_type) {
    sprintf(buf, "Content-Type: %s\r\n", rsp->content_type);
    rc = bufcat(&rsp->conn->rsphdr, buf);
    if (rc < 0) return rc;
  }

  if (rsp->content_length >= 0) {
    sprintf(buf, "Content-Length: %d\r\n", rsp->content_length);
    rc = bufcat(&rsp->conn->rsphdr, buf);
    if (rc < 0) return rc;
  }

  if (headers) {
    rc = bufcat(&rsp->conn->rsphdr, headers);
    if (rc < 0) return rc;
  }

  if (rsp->content_length < 0) rsp->keep_alive = 0;
  if (state >= 500) rsp->keep_alive = 0;

  if (rsp->keep_alive) {
    rc = bufcat(&rsp->conn->rsphdr, "Connection: Keep-Alive\r\n");
    if (rc < 0) return rc;
  }

  rc = bufcat(&rsp->conn->rsphdr, "\r\n");
  if (rc < 0) return rc;

  return 0;
}

int httpd_send_error(struct httpd_response *rsp, int state, char *title, char *msg) {
  int rc;
  char buf[2048];

  if (!msg) msg = "";
  sprintf(buf, "<HTML><HEAD><TITLE>%d %s</TITLE></HEAD>\n<BODY><H1>%d %s</H1>\n%s\n</BODY></HTML>\n",
          state, title, state, title, msg);

  rsp->content_type = "text/html";
  rsp->content_length = strlen(buf);

  rc = httpd_send_header(rsp, state, title ? title : statustitle(state), NULL);
  if (rc < 0) return rc;

  rc = httpd_send(rsp, buf, strlen(buf));
  if (rc < 0) return rc;

  return 0;
}

int httpd_redirect(struct httpd_response *rsp, char *newloc) {
  char *hdrs;
  int rc;

  hdrs = (char *) malloc(3 * strlen(newloc) + 16);
  if (!hdrs) return -1;

  strcpy(hdrs, "Location: ");
  encode_url(newloc, hdrs + 10);
  strcat(hdrs, "\r\n");

  sprintf(hdrs, "Location: %s\r\n", newloc);
  rsp->content_length = 0;

  rc = httpd_send_header(rsp, 302, "Moved", hdrs);
  free(hdrs);

  return rc;
}

int httpd_send(struct httpd_response *rsp, char *data, int len) {
  struct httpd_buffer *buf = &rsp->conn->rspbody;
  int rc;
  int left;
  int n;

  if (len == -1) len = strlen(data);

  // Send directly if data larger than buffer
  if (len > rsp->conn->server->rspbufsiz) {
    rc = httpd_flush(rsp);
    if (rc < 0) return rc;

    rc = send(rsp->conn->sock, data, len, 0);
    return rc;
  }

  // Allocate response body buffer if not already done
  if (!buf->floor) {
    rc = allocate_buffer(buf, rsp->conn->server->rspbufsiz);
    if (rc < 0) return rc;
  }

  left = len;
  while (left > 0) {
    // Send buffer if full
    if (buf->ceil == buf->end) {
      rc = httpd_flush(rsp);
      if (rc < 0) return rc;
    }

    // Copy data to buffer
    n = left;
    if (n > buf->ceil - buf->end) n = buf->ceil - buf->end;
    memcpy(buf->end, data, n);
    buf->end += n;
    left -= n;
    data += n;
  }

  return len;
}

int httpd_flush(struct httpd_response *rsp) {
  struct httpd_buffer *buf;
  int rc;

  // Send response header if not already done
  if (!rsp->conn->hdrsent) {
    buf = &rsp->conn->rsphdr;

    // Generate standard header
    if (buf->start == buf->end) {
      rc = httpd_send_header(rsp, 200, "OK", NULL);
      if (rc < 0) return rc;
    }

    // Send header
    if (buf->end != buf->start) {
      rc = send(rsp->conn->sock, buf->start, buf->end - buf->start, 0);
      if (rc < 0) return rc;

      buf->start = buf->end;
    }

    rsp->conn->hdrsent = 1;
  }

  // Send response body
  buf = &rsp->conn->rspbody;
  if (buf->end != buf->start) {
    rc = send(rsp->conn->sock, buf->start, buf->end - buf->start, 0);
    if (rc < 0) return rc;

    buf->start = buf->end = buf->floor;
  }

  return 0;
}

int httpd_send_file(struct httpd_response *rsp, int fd) {
  rsp->conn->fd = fd;
  return 0;
}

int httpd_send_fixed_data(struct httpd_response *rsp, char *data, int len) {
  rsp->conn->fixed_rsp_data = data;
  rsp->conn->fixed_rsp_len = len;
  return 0;
}

int httpd_check_header(struct httpd_connection *conn) {
  char c;

  while (conn->reqhdr.start < conn->reqhdr.end) {
    c = *conn->reqhdr.start++;
    switch (conn->hdrstate) {
      case HDR_STATE_FIRSTWORD:
        switch (c) {
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
        switch (c) {
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
        switch (c) {
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
        switch (c) {
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
        switch (c) {
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
        switch (c) {
          case '\n':
            conn->hdrstate = HDR_STATE_LF;
            break;

          case '\r':
            conn->hdrstate = HDR_STATE_CR;
            break;
        }
        break;

      case HDR_STATE_LF:
        switch (c) {
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
        switch (c) {
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
        switch (c) {
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
        switch (c) {
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

int httpd_parse_request(struct httpd_request *req) {
  struct httpd_buffer *buf;
  char *s;
  char *l;
  int rc;
  
  buf = &req->conn->reqhdr;
  buf->start = buf->floor;

  s = bufgets(buf);
  if (!s) {
    errno = EINVAL;
    return -1;
  }

  // Parse method
  req->method = s;
  s = strchr(s, ' ');
  if (!s) {
    errno = EINVAL;
    return -1;
  }
  *s++ = 0;

  // Parse URL
  while (*s == ' ') s++;
  req->encoded_url = s;
  s = strchr(s, ' ');

  // Parse protocol version
  if (*s) {
    *s++ = 0;
    while (*s == ' ') s++;
    if (*s) {
      req->protocol = s;
    } else {
      req->protocol = "HTTP/0.9";
    }

    if (strcmp(req->protocol, "HTTP/1.1") == 0) req->http11 = 1;
  }

  // Decode and split URL
  req->decoded_url = (char *) malloc(strlen(req->encoded_url) + 1);
  if (!req->decoded_url) return -1;

  rc = decode_url(req->encoded_url, req->decoded_url);
  if (rc < 0) return rc;

  req->pathinfo = req->decoded_url;
  s = strchr(req->pathinfo, '?');
  if (s) {
    *s++ = 0;
    req->query = s;
  }

  //printf("method: [%s]\n", req->method);
  //printf("pathinfo: [%s]\n", req->pathinfo);
  //printf("query: [%s]\n", req->query);
  //printf("protocol: [%s]\n", req->protocol);

  // Parse headers
  req->nheaders = 0;
  while ((l = bufgets(buf))) {
    if (!*l) continue;
   
    s = strchr(l, ':');
    if (!s) continue;
    *s++ = 0;
    while (*s == ' ') s++;
    if (!*s) continue;

    if (stricmp(l, "Referer") == 0) {
      req->referer = s;
    } else if (stricmp(l, "User-agent") == 0) {
      req->user_agent = s;
    } else if (stricmp(l, "Accept") == 0) {
      req->accept = s;
    } else if (stricmp(l, "Cookie") == 0) {
      req->cookie = s;
    } else if (stricmp(l, "Authorization") == 0) {
      req->authorization = s;
    } else if (stricmp(l, "Content-type") == 0) {
      req->content_type = s;
    } else if (stricmp(l, "Content-length") == 0) {
      req->content_length = atoi(s);
    } else if (stricmp(l, "Host") == 0) {
      req->host = s;
    } else if (stricmp(l, "If-modified-since") == 0) {
      req->if_modified_since = timerfc(s);
    } else if (stricmp(l, "Connection") == 0) {
      req->keep_alive = stricmp(s, "keep-alive") == 0;
    }

    //printf("hdr: %s: %s\n", l, s);

    if (req->nheaders < MAX_HTTP_HEADERS) {
      req->headers[req->nheaders].name = l;
      req->headers[req->nheaders].value = s;
      req->nheaders++;
    }
  }

  return 0;
}

int httpd_find_context(struct httpd_request *req) {
  int n;
  char *s;
  struct httpd_context *context = req->conn->server->contexts;
  char *pathinfo = req->pathinfo;

  while (context) {
    n = strlen(context->alias);
    s = pathinfo + n;
    if (strncmp(context->alias, pathinfo, n) == 0 && (*s == '/' || *s == 0)) {
      if (*s) {
        req->pathinfo = s + 1;
      } else {
        req->pathinfo = "";
      }

      req->context = context;
      return 0;
    }

    context = context->next;
  }

  return -1;
}

int httpd_translate_path(struct httpd_request *req) {
  int rc;
  char buf[512];
  char path[256];
  int loclen;
  int pathlen;
  char *p;

  if (req->context->location == NULL) {
    rc = httpd_send_error(req->conn->rsp, 500, "Internal Server Error", "No location for context");
    if (rc < 0) return rc;
    return 0;
  }

  loclen = strlen(req->context->location);
  pathlen = strlen(req->pathinfo);
  if (loclen + 1 +  pathlen >= sizeof(buf)) {
    rc = httpd_send_error(req->conn->rsp, 400, "Bad Request", "URL to long");
    if (rc < 0) return rc;
    return 0;
  }

  memcpy(buf, req->context->location, loclen);
  buf[loclen] = '/';
  memcpy(buf + loclen + 1, req->pathinfo, pathlen);
  buf[loclen + 1 + pathlen] = 0;

  rc = canonicalize(buf, path, sizeof(path));
  if (rc < 0) {
    rc = httpd_send_error(req->conn->rsp, 400, "Bad Request", "Bad URL");
    if (rc < 0) return rc;
    return 0;
  }

  p = path;
  while (*p) {
    if (*p == '\\') *p = '/';
    p++;
  }

  if (loclen > 0 && strnicmp(req->context->location, path, loclen) != 0) {
    rc = httpd_send_error(req->conn->rsp, 400, "Bad Request", "Illegal URL");
    if (rc < 0) return rc;
    return 0;
  }

  req->path_translated = strdup(path);
  if (!req) return -1;

  return 1;
}

int httpd_write(struct httpd_connection *conn) {
  int left;
  int bytes;
  int rc;

  // Sent any remaining data in response header
  left = conn->rsphdr.end - conn->rsphdr.start;
  if (left > 0) {
    bytes = send(conn->sock, conn->rsphdr.start, left, 0);
    if (bytes < 0) return bytes;
    conn->rsphdr.start += bytes;
    if (bytes < left) return 1;
  }

  // Sent any remaining fixed response data
  left = conn->fixed_rsp_len;
  if (left > 0) {
    bytes = send(conn->sock, conn->fixed_rsp_data, left, 0);
    if (bytes < 0) return bytes;
    conn->fixed_rsp_data += bytes;
    conn->fixed_rsp_len -= bytes;
    if (bytes < left) return 1;
  }

  // Send response body
  while (1) {
    // Send any remaining data in response body buffer
    left = conn->rspbody.end - conn->rspbody.start;
    if (left > 0) {
      bytes = send(conn->sock, conn->rspbody.start, left, 0);
      if (bytes < 0) return bytes;
      conn->rspbody.start += bytes;
      if (bytes < left) return 1;
    }

    // Fill response buffer
    if (conn->fd >= 0) {
      // Allocate response body buffer if not already done
      if (conn->rspbody.floor == NULL) {
        rc = allocate_buffer(&conn->rspbody, conn->server->rspbufsiz);
        if (rc < 0) return rc;
      }

      // Read from file
      bytes = read(conn->fd, conn->rspbody.floor, buffer_capacity(&conn->rspbody));
      if (bytes < 0) return bytes;
      if (bytes == 0) return 0;

      conn->rspbody.start = conn->rspbody.floor;
      conn->rspbody.end = conn->rspbody.floor + bytes;
    } else {
      return 0;
    }
  }
}

int httpd_process(struct httpd_connection *conn) {
  struct httpd_request req;
  struct httpd_response rsp;
  int rc;
  int size;
  int on = 1;

  // Initialize new request and response objects
  memset(&req, 0, sizeof(struct httpd_request));
  memset(&rsp, 0, sizeof(struct httpd_response));
  rsp.content_length = -1;

  req.conn = conn;
  rsp.conn = conn;
  conn->req = &req;
  conn->rsp = &rsp;
  conn->fd = -1;
  conn->keep = 0;
  conn->hdrsent = 0;

  // Parse HTTP request
  rc = httpd_parse_request(&req);
  if (rc < 0) goto errorexit;
  rsp.keep_alive = req.keep_alive;

  // Find context handler for request
  rc = httpd_find_context(&req);
  if (rc < 0) {
    // No context found - return error
    rc = httpd_send_error(&rsp, 404, "Not Found", "Context does not exist");
    if (rc < 0) goto errorexit;
  } else {
    // Translate path
    rc = httpd_translate_path(&req);
    if (rc < 0) goto errorexit;
    if (rc > 0) {
      // Call context handler to handle request
      rc = req.context->handler(conn);
      if (rc < 0) goto errorexit;
      if (rc > 0) {
        // Return HTTP error to client
        rc = httpd_send_error(&rsp, rc, NULL, NULL);
        if (rc < 0) goto errorexit;
      }

      // Build HTTP header if not already done
      if (!conn->hdrsent && buffer_empty(&conn->rsphdr)) {
        if (rsp.content_length < 0) {
          int len = 0;

          len += buffer_size(&conn->rspbody);
          len += conn->fixed_rsp_len;
          if (conn->fd >= 0) {
            size = fstat(conn->fd, NULL);
            if (size >= 0) len += size;
          }

          rsp.content_length = len;
        }

        rc = httpd_send_header(&rsp, 200, "OK", NULL);
        if (rc < 0) goto errorexit;
      }
    }
  }

  // Log request
  rc = log_request(&req);
  if (rc < 0) goto errorexit;

  // Prepare for sending back response
  rc = ioctl(conn->sock, FIONBIO, &on, sizeof(on));
  if (rc < 0) goto errorexit;
  conn->keep = rsp.keep_alive;
  httpd_finish_processing(conn);

  return 0;

errorexit:
  httpd_finish_processing(conn);
  return -1;
}

int httpd_io(struct httpd_connection *conn) {
  int rc;

  switch (conn->state) {
    case HTTP_STATE_IDLE:
      rc = allocate_buffer(&conn->reqhdr, conn->server->min_hdrbufsiz);
      if (rc < 0) return rc;
      conn->state = HTTP_STATE_READ_REQUEST;
      conn->hdrstate = HDR_STATE_FIRSTWORD;
      // Fall through

    case HTTP_STATE_READ_REQUEST:
      if (buffer_capacity(&conn->reqhdr) >= conn->server->max_hdrbufsiz) return -EBUF;
      rc = expand_buffer(&conn->reqhdr, 1);
      if (rc < 0) return rc;

      rc = recv(conn->sock, conn->reqhdr.end, buffer_left(&conn->reqhdr), 0);
      if (rc <= 0) {
        if (errno == ECONNRESET && buffer_size(&conn->reqhdr) == 0) {
          // Keep-Alive connection closed
          conn->state = HTTP_STATE_TERMINATED;
          httpd_close_connection(conn);
          return 1;
        }

        return rc;
      }

      conn->reqhdr.end += rc;

      rc = httpd_check_header(conn);
      if (rc < 0) return rc;
      if (rc == 0) {
        rc = dispatch(conn->server->iomux, conn->sock, IOEVT_READ | IOEVT_CLOSE | IOEVT_ERROR, (int) conn);
        if (rc < 0) return rc;
        return 1;
      }
      conn->state = HTTP_STATE_PROCESSING;
      // Fall through

    case HTTP_STATE_PROCESSING:
      rc = httpd_process(conn);
      if (rc < 0) return rc;
      conn->state = HTTP_STATE_WRITE_RESPONSE;
      // Fall through

    case HTTP_STATE_WRITE_RESPONSE:
      rc = httpd_write(conn);
      if (rc < 0) return rc;

      if (rc > 0) {
        rc = dispatch(conn->server->iomux, conn->sock, IOEVT_WRITE | IOEVT_CLOSE | IOEVT_ERROR, (int) conn);
        if (rc < 0) return rc;
      } else {
        rc = httpd_terminate_request(conn);
        if (rc > 0) {
          conn->state = HTTP_STATE_IDLE;
          rc = dispatch(conn->server->iomux, conn->sock, IOEVT_READ | IOEVT_CLOSE | IOEVT_ERROR, (int) conn);
          if (rc < 0) return rc;
        } else {
          conn->state = HTTP_STATE_TERMINATED;
          httpd_close_connection(conn);
        }
      }
      return 1;

    case HTTP_STATE_TERMINATED:
      return 1;

    default:
      errno = EINVAL;
      return -1;
  }
}

void __stdcall httpd_worker(void *arg) {
  struct httpd_server *server = (struct httpd_server *) arg;
  struct httpd_connection *conn;
  int rc;

  while (1) {
    rc = waitone(server->iomux, INFINITE);
    if (rc < 0) break;

    conn = (struct httpd_connection *) rc;
    if (conn == NULL) {
      httpd_accept(server);
      dispatch(server->iomux, server->sock, IOEVT_ACCEPT, 0);
    } else {
      rc = httpd_io(conn);
      if (rc <= 0) {
        httpd_close_connection(conn);
      }
    }
  }
}

int httpd_start(struct httpd_server *server) {
  int sock;
  int rc;
  int i;
  httpd_sockaddr addr;
  int hthread;

  sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0) return -1;

  addr.sa_in.sin_family = AF_INET;
  addr.sa_in.sin_port = htons(server->port);
  addr.sa_in.sin_addr.s_addr = htonl(INADDR_ANY);

  rc = bind(sock, &addr.sa, sizeof(addr));
  if (rc < 0) {
    close(sock);
    return -1;
  }

  rc = listen(sock, server->backlog);
  if (rc < 0) {
    close(sock);
    return -1;
  }

  ioctl(sock, FIONBIO, NULL, 0);

  server->sock = sock;
  server->iomux = mkiomux(0);
  dispatch(server->iomux, server->sock, IOEVT_ACCEPT, 0);

  for (i = 0; i < server->num_workers; i++) {
    hthread = beginthread(httpd_worker, 0, server, 0, "http", NULL);
    close(hthread);
  }

  return 0;
}

#ifdef _MSC_VER
int __stdcall DllMain(hmodule_t hmod, int reason, void *reserved) {
  return TRUE;
}
#endif

