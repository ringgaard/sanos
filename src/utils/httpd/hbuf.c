//
// hbuf.c
//
// HTTP buffer management
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
#include <string.h>

#include <httpd.h>

int buffer_size(struct httpd_buffer *buf) {
  return buf->end - buf->start;
}

int buffer_capacity(struct httpd_buffer *buf) {
  return buf->ceil - buf->floor;
}

int buffer_left(struct httpd_buffer *buf) {
  return buf->ceil - buf->end;
}

int buffer_empty(struct httpd_buffer *buf) {
  return buf->start == buf->end;
}

int buffer_full(struct httpd_buffer *buf) {
  return buf->end == buf->ceil;
}

int allocate_buffer(struct httpd_buffer *buf, int size) {
  if (size == 0) {
    buf->floor = buf->ceil = NULL;
    buf->start = buf->end = NULL;
  } else {
    buf->floor = (char *) malloc(size);
    if (!buf->floor) return -1;
    buf->ceil = buf->floor + size;
    buf->start = buf->end = buf->floor;
  }

  return 0;
}

void free_buffer(struct httpd_buffer *buf) {
  if (buf->floor) free(buf->floor);
  buf->floor = buf->ceil = buf->start = buf->end = NULL;
}

void clear_buffer(struct httpd_buffer *buf) {
  buf->start = buf->end = buf->floor;
}

int expand_buffer(struct httpd_buffer *buf, int minfree) {
  char *p;
  int size;
  int minsize;

  if (buf->ceil - buf->end >= minfree) return 0;
  
  size = buf->ceil - buf->floor;
  minsize = buf->end + minfree - buf->floor;
  while (size < minsize) {
    if (size == 0) {
      size = 1024;
    } else {
      size *= 2;
    }
  }

  p = (char *) realloc(buf->floor, size);
  if (!p) return -1;

  buf->start += p - buf->floor;
  buf->end += p - buf->floor;
  buf->floor = p;
  buf->ceil = p + size;

  return 0;
}

char *bufgets(struct httpd_buffer *buf) {
  char *start;
  char *s;

  s = start = buf->start;
  while (s < buf->end) {
    switch (*s) {
      case '\n':
        if (s[1] != ' ' && s[1] != '\t') {
          if (s > start && s[-1] == ' ') {
            s[-1] = 0;
          } else {
            s[0] = 0;
          }

          buf->start = s + 1;
          return start;
        } else {
          *s++ = ' ';
        }
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

int bufncat(struct httpd_buffer *buf, char *data, int len) {
  int rc;

  rc = expand_buffer(buf, len);
  if (rc < 0) return rc;
  memcpy(buf->end, data, len);
  buf->end += len;

  return 0;
}

int bufcat(struct httpd_buffer *buf, char *data) {
  if (!data) return 0;
  return bufncat(buf, data, strlen(data));
}
