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
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

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

int ls(struct httpd_connection *conn)
{
  int dir;
  int rc;
  int urllen;
  struct stat statbuf;
  struct dirent dirp;
  char path[MAXPATH];
  char buf[32];
  struct tm *tm;

  dir = opendir(conn->req->path_translated);
  if (dir < 0) return httpd_return_file_error(conn, dir);

  urllen = strlen(conn->req->decoded_url);
  if (urllen > 0 && conn->req->decoded_url[urllen - 1] == '/') urllen--;
  httpd_send(conn->rsp, "<HTML><HEAD><TITLE>Index of ", -1);
  httpd_send(conn->rsp, conn->req->decoded_url, urllen);
  httpd_send(conn->rsp, "</TITLE></HEAD>\r\n<BODY>\r\n<H1>Index of ", -1);
  
  if (urllen == 0)
    httpd_send(conn->rsp, "/", -1);
  else
    httpd_send(conn->rsp, conn->req->decoded_url, urllen);

  httpd_send(conn->rsp, "</H1><PRE>\r\n", -1);
  httpd_send(conn->rsp, "   Name                              Last Modified           Size\r\n", -1);
  httpd_send(conn->rsp, "<HR>\r\n", -1);

  if (urllen > 1) httpd_send(conn->rsp, "<IMG SRC=\"/icons/folder.gif\"> <A HREF=\"..\">..</A>\r\n", -1);

  while (readdir(dir, &dirp, 1) > 0)
  {
    strcpy(path, conn->req->path_translated);
    strcat(path, "/");
    strcat(path, dirp.name);

    rc = stat(path, &statbuf);
    if (rc < 0) return rc;

    tm = gmtime(&statbuf.mtime);
    if (!tm) return -EINVAL;

    if ((statbuf.mode & S_IFMT) == S_IFDIR) 
      httpd_send(conn->rsp, "<IMG SRC=\"/icons/folder.gif\"> ", -1);
    else
      httpd_send(conn->rsp, "<IMG SRC=\"/icons/file.gif\"> ", -1);

    httpd_send(conn->rsp, "<A HREF=\"", -1);
    httpd_send(conn->rsp, dirp.name, dirp.namelen);
    if ((statbuf.mode & S_IFMT) == S_IFDIR) httpd_send(conn->rsp, "/", -1);
    httpd_send(conn->rsp, "\">", -1);

    httpd_send(conn->rsp, dirp.name, dirp.namelen);
    if ((statbuf.mode & S_IFMT) == S_IFDIR) httpd_send(conn->rsp, "/", -1);
    httpd_send(conn->rsp, "</A>", -1);
    if ((statbuf.mode & S_IFMT) != S_IFDIR) httpd_send(conn->rsp, " ", -1);

    if (dirp.namelen < 32) httpd_send(conn->rsp, "                                        ", 32 - dirp.namelen);

    strftime(buf, 32, "%d-%b-%Y %H:%M:%S", tm);
    httpd_send(conn->rsp, buf, -1);

    if ((statbuf.mode & S_IFMT) != S_IFDIR)
    {
      if (statbuf.quad.size_high == 0 && statbuf.quad.size_low < 1*K)
	sprintf(buf, "%8d B", statbuf.quad.size_low);
      else if (statbuf.quad.size_high == 0 && statbuf.quad.size_low < 1*M)
	sprintf(buf, "%8d KB", statbuf.quad.size_low / K);
      else if (statbuf.quad.size_high == 0)
	sprintf(buf, "%8d MB", statbuf.quad.size_low / M);
      else
	sprintf(buf, "%8d GB", (statbuf.quad.size_high << 2) | (statbuf.quad.size_low >> 30));

      httpd_send(conn->rsp, buf, -1);
    }

    httpd_send(conn->rsp, "\r\n", -1);
  }

  httpd_send(conn->rsp, "</PRE><HR>\r\n", -1);
  httpd_send(conn->rsp, "<ADDRESS>", -1);
  httpd_send(conn->rsp, conn->server->swname, -1);
  httpd_send(conn->rsp, "</ADDRESS>\r\n", -1);
  httpd_send(conn->rsp, "</BODY></HTML>\r\n", -1);
  
  close(dir);
  return 0;
}

int httpd_file_handler(struct httpd_connection *conn)
{
  int rc;
  int fd;
  struct stat statbuf;
  char *filename;
  char buf[MAXPATH];

  if (strcmp(conn->req->method, "GET") != 0 && strcmp(conn->req->method, "HEAD") != 0)
  {
    return httpd_send_error(conn->rsp, 405, "Method Not Allowed", NULL);
  }

  filename = conn->req->path_translated;
  rc = stat(filename, &statbuf);
  if (rc < 0) return httpd_return_file_error(conn, rc);

  if ((statbuf.mode & S_IFMT) == S_IFDIR)
  {
    int urllen = strlen(conn->req->decoded_url);

    if (urllen < 1 || urllen >= MAXPATH - 1) return 400;

    if (conn->req->decoded_url[urllen - 1] != '/')
    {
      strcpy(buf, conn->req->decoded_url);
      strcat(buf, "/");

      rc = httpd_redirect(conn->rsp, buf);
      if (rc < 0) return rc;

      return 0;
    }

    if (conn->server->indexname && *conn->server->indexname)
    {
      if (strlen(conn->req->path_translated) + strlen(conn->server->indexname) + 2 > MAXPATH) return 400;
      strcpy(buf, conn->req->path_translated);
      strcat(buf, "/");
      strcat(buf, conn->server->indexname);

      rc = stat(buf, &statbuf);
      if (rc < 0) 
      {
	if (conn->server->allowdirbrowse) 
	  return ls(conn);
	else
	  return httpd_send_error(conn->rsp, 403, "Forbidden", "Directory browsing now allowed");
      }
      else
      {
	if ((statbuf.mode & S_IFMT) == S_IFDIR) return 500;
	filename = buf;
      }
    }
  }

  if ((statbuf.mode & S_IFMT) == S_IFREG)
  {
    conn->rsp->content_length = statbuf.quad.size_low;
    conn->rsp->last_modified = statbuf.mtime;
    conn->rsp->content_type = httpd_get_mimetype(conn->server, get_extension(filename));

    if (conn->rsp->last_modified <= conn->req->if_modified_since)
    {
      return httpd_send_header(conn->rsp, 304, "Not Modified", NULL);
    }
  }
  else
  {
    conn->rsp->content_type = "text/plain";
  }


  if (strcmp(conn->req->method, "HEAD") == 0) return 0;

  fd = open(filename, O_RDONLY);
  if (fd < 0) return httpd_return_file_error(conn, fd);

  rc = httpd_send_file(conn->rsp, fd);
  if (rc < 0) return rc;

  return 0;
}

int httpd_resource_handler(struct httpd_connection *conn)
{
  char *resdata;
  int reslen;
  int rc;

  if (strcmp(conn->req->method, "GET") != 0 && strcmp(conn->req->method, "HEAD") != 0)
  {
    return httpd_send_error(conn->rsp, 405, "Method Not Allowed", NULL);
  }

  resdata = getresdata(conn->req->context->hmod, 10, conn->req->pathinfo, 0);
  if (!resdata)
  {
    return httpd_send_error(conn->rsp, 404, "Not Found", NULL);
  }
  reslen = getreslen(conn->req->context->hmod, 10, conn->req->pathinfo, 0);

  conn->rsp->content_length = reslen;
  conn->rsp->last_modified = conn->req->context->mtime;
  conn->rsp->content_type = httpd_get_mimetype(conn->server, get_extension(conn->req->pathinfo));

  if (conn->rsp->last_modified <= conn->req->if_modified_since)
  {
    return httpd_send_header(conn->rsp, 304, "Not Modified", NULL);
  }

  if (strcmp(conn->req->method, "HEAD") == 0) return 0;

  rc = httpd_send_fixed_data(conn->rsp, resdata, reslen);
  if (rc < 0) return rc;

  return 0;
}
