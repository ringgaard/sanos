//
// hfile.c
//
// HTTP file handler
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
#include <stdlib.h>

#include <httpd.h>

char *get_extension(char *path)
{
  char *ext;

  ext = NULL;
  while (*path)
  {
    if (*path == PS1 || *path == PS2)
      ext = NULL;
    else if (*path == '.')
      ext = path + 1;

    path++;
  }

  return ext;
}

int httpd_return_file_error(struct httpd_connection *conn, int err)
{
  switch (err)
  {
    case -EACCES: 
      return httpd_send_error(conn->rsp, 403, "Forbidden", NULL);

    case -ENOENT:
      return httpd_send_error(conn->rsp, 404, "Not Found", NULL);

    default:
      return httpd_send_error(conn->rsp, 500, "Internal Server Error", strerror(err));
  }
}

int httpd_file_handler(struct httpd_connection *conn)
{
  int rc;
  int fd;
  struct stat statbuf;

  if (strcmp(conn->req->method, "GET") != 0 && strcmp(conn->req->method, "HEAD") != 0)
  {
    return httpd_send_error(conn->rsp, 405, "Method Not Allowed", NULL);
  }

  rc = stat(conn->req->path_translated, &statbuf);
  if (rc < 0) return httpd_return_file_error(conn, rc);

  if (statbuf.mode & FS_DIRECTORY)
  {
    return httpd_send_error(conn->rsp, 501, "Not Implemented", "Directory browsing not supported");
  }

  conn->rsp->content_length = statbuf.quad.size_low;
  conn->rsp->last_modified = statbuf.mtime;
  conn->rsp->content_type = httpd_get_mimetype(conn->server, get_extension(conn->req->path_translated));

  if (conn->rsp->last_modified <= conn->req->if_modified_since)
  {
    return httpd_send_header(conn->rsp, 304, "Not Modified", NULL);
  }

  if (strcmp(conn->req->method, "HEAD") == 0) return 0;

  fd = open(conn->req->path_translated, O_RDONLY);
  if (fd < 0) return httpd_return_file_error(conn, fd);

  rc = httpd_send_file(conn->rsp, fd);
  if (rc < 0) return rc;

  return 0;
}
