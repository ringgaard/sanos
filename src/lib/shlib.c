//
// shlib.c
//
// Shell command library
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

#include <shlib.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/utsname.h>
#include <os/version.h>

char *join_path(char *dir, char *name) {
  int dirlen;
  char *path;

  if (!dir) return strdup(name);
  dirlen = strlen(dir);
  while (dirlen > 0 && dir[dirlen - 1] == '/') dirlen--;
  path = malloc(dirlen + strlen(name) + 2);
  if (!path) return NULL;
  memcpy(path, dir, dirlen);
  path[dirlen] = '/';
  strcpy(path + dirlen + 1, name);
  return path;
}

char *get_symbolic_mode(int mode, char *buf) {
  strcpy(buf, " ---------");
  switch (mode & S_IFMT) {
    case S_IFREG: buf[0] = '-'; break;
    case S_IFLNK: buf[0] = 'l'; break;
    case S_IFDIR: buf[0] = 'd'; break;
    case S_IFBLK: buf[0] = 'b'; break;
    case S_IFCHR: buf[0] = 'c'; break;
    case S_IFPKT: buf[0] = 'p'; break;
  }

  if (mode & 0400) buf[1] = 'r';
  if (mode & 0200) buf[2] = 'w';
  if (mode & 0100) buf[3] = 'x';
  if (mode & 0040) buf[4] = 'r';
  if (mode & 0020) buf[5] = 'w';
  if (mode & 0010) buf[6] = 'x';
  if (mode & 0004) buf[7] = 'r';
  if (mode & 0002) buf[8] = 'w';
  if (mode & 0001) buf[9] = 'x';
  
  return buf;
}

int parse_symbolic_mode(char *symbolic, int orig) {
  char *s;
  int mode, who, op, perm, mask, done, i;

  mode = orig;
  s = symbolic;
  while (*s) {
    if (*s >= '0' && *s <= '7') {
      // Parse octal mode 
      mask = 0;
      while (*s >= '0' && *s <= '7') {
        mask = (mask << 3) + (*s++ - '0');
      }
      mode |= mask;
    } else {
      // Parse who: user, group, other, all
      who = 0;
      done = 0;
      while (!done) {
        switch (*s++) {
          case 'u': who = 4; break;
          case 'g': who = 2; break;
          case 'o': who = 1; break;
          case 'a': who = 7; break;
          default: s--; done = 1;
        }
      }
      if (!who) who = 4;

      // Parse operation: =, +, -
      if (*s != '=' && *s != '+' && *s != '-') return -1;
      op = *s++;
    
      // Parse permissions (rwx) or permission copy (ugo)
      perm = 0;
      done = 0;
      while (!done) {
        switch (*s++) {
          case 'r': perm |= 4; break;
          case 'w': perm |= 2; break;
          case 'x': perm |= 1; break;
          case 'u': perm |= (orig >> 6) & 7; break;
          case 'g': perm |= (orig >> 3) & 7; break;
          case 'o': perm |= orig & 7; break;
          default: s--; done = 1;
        }
      }

      // Combine who, operation, and permissions
      for (i = 0; i < 3; i++) {
        int shift = i * 3;
        if (who & (1 << i)) {
          switch (op) {
            case '+':
              // Add permission bits
              mode |= perm << shift; 
              break;

            case '-': 
              // Remove permission bits
              mode &= ~(perm << shift); 
              break;

            case '=':
              // Replace permission bits
              mode &= ~(7 << shift);
              mode |= perm << shift;
          }
        }
      }
    }

    // Mode changes are separated by commas
    if (*s) {
      if (*s++ != ',') return -1;
    }
  }

  return mode;
}

int parse_url(char *url, char **host, int *port, char **path) {
  *host = NULL;
  *port = 80;
  *path = NULL;

  // Parse protocol
  if (strncmp(url, "file:", 5) == 0) {
    *port = -1;
    url += 5;
  } else if (strncmp(url, "http:", 5) == 0) {
    url += 5;
  } else if (strncmp(url, "https:", 6) == 0 || strncmp(url, "ftp:", 4)) {
    return -1;
  }

  // Parse server and port
  if (strncmp(url, "//", 2) == 0) {
    char *start;
    int len;

    url += 2;
    start = url;
    while (*url && *url != '/' && *url != ':') url++;
    len = url - start;
    *host = malloc(len + 1);
    memcpy(*host, start, len);
    (*host)[len] = 0;

    if (*url == ':') {
      *port = 0;
      url++;
      while (*url && *url != '/') {
        if (*url < '0' || *url > '9') return -1;
        *port = *port * 10 + (*url++ - '0');
      }
    }
  }

  // The remaining URL is the path
  *path = strdup(*url ? url : "/");

  return 0;
}

FILE *open_url(char *url, char *agent) {
  char *host;
  int port;
  char *path;
  struct hostent *hp;
  struct sockaddr_in sin;
  int s;
  int rc;
  FILE *f;
  int status;
  struct utsname sys;
  char header[1024];

  // Parse URL  
  if (parse_url(url, &host, &port, &path) < 0) {
    fprintf(stderr, "%s: invalid url\n", url);
    free(host);
    free(path);
    return NULL;
  }

  // Check for file-based URL
  if (port == -1) {
    f = fopen(path, "r");
    if (!f) perror(path);
    free(host);
    free(path);
    return f;
  }

  // Lookup host
  hp = gethostbyname(host);
  if (!hp) {
    perror(host);
    free(host);
    free(path);
    return NULL;
  }
  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  memcpy(&sin.sin_addr, hp->h_addr_list[0], hp->h_length);
  sin.sin_port = htons(port);

  // Connect to host
  s = socket(AF_INET, SOCK_STREAM, 0);
  if (s < 0) {
    perror(host);
    free(host);
    free(path);
    return NULL;
  }

  rc = connect(s, (struct sockaddr *) &sin, sizeof(sin));
  if (rc  < 0) {
    perror(host);
    free(path);
    close(s);
    return NULL;
  }

  // Open file for socket
  f = fdopen(s, "a+");
  if (!f) {
    perror(host);
    free(host);
    free(path);
    close(s);
    return NULL;
  }

  // Send request to host
  memset(&sys, 0, sizeof(struct utsname));
  uname(&sys);
  fprintf(f, "GET %s HTTP/1.0\r\nHost: %s\r\nUser-Agent: %s/%d.%d.%d.%d %s/%s (Build %s; CPU %s)\r\n\r\n", 
          path, host, 
          agent, OS_MAJ_VERS, OS_MIN_VERS, OS_RELEASE, OS_BUILD,
          sys.sysname, sys.release, sys.version, sys.machine);
  fseek(f, 0, SEEK_CUR);

  // Read headers
  status = -1;
  while (fgets(header, sizeof(header), f)) {
    // Remove trailing cr/nl
    char *p = header;
    while (*p && *p != '\r' && *p != '\n') p++;
    *p = 0;

    // Check status code
    if (status == -1) {
      p = header;
      if (strncmp(p, "HTTP/1.0 ", 9) == 0 || strncmp(p, "HTTP/1.1 ", 9) == 0) p += 9;
      status = atoi(p);
      if (status != 200) fprintf(stderr, "%s: %s\n", url, p);
    }
    
    // Check for end of header
    if (!*header) break;
  }

  // Check status code
  if (status != 200) {
    if (status == -1) {
      fprintf(stderr, "%s: invalid response from server\n", url);
    }
    free(host);
    free(path);
    fclose(f);
    return NULL;
  }

  // Return file
  free(host);
  free(path);
  return f;
}

