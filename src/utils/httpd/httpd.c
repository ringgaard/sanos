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

#include <httpd.h>

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
  printf("httpd_accept\n");
}

void httpd_io(struct httpd_request *req)
{
  printf("httpd_io\n");
}

void __stdcall httpd_worker(void *arg)
{
  struct httpd_server *server = (struct httpd_server *) arg;
  struct httpd_request *req;
  int rc;

  while (1)
  {
    rc = wait(server->iomux, INFINITE);
    if (rc < 0) break;

    req = (struct httpd_request *) rc;
    if (req == NULL) 
      httpd_accept(server);
    else
      httpd_io(req);
  }
}

int httpd_start(struct httpd_server *server)
{
  int sock;
  int rc;
  int i;
  struct sockaddr_in sin;
  int hthread;

  sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0) return sock;

  sin.sin_family = AF_INET;
  sin.sin_port = htons(server->port);
  sin.sin_addr.s_addr = htonl(INADDR_ANY);

  rc = bind(sock, (struct sockaddr *) &sin, sizeof(struct sockaddr_in));
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

  server->sock = sock;
  server->iomux = mkiomux(0);
  dispatch(server->iomux, sock, IOEVT_ACCEPT, 0);

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

  server = httpd_initialize(NULL);
  httpd_add_context(server, NULL, NULL);
  httpd_start(server);

  return TRUE;
}
