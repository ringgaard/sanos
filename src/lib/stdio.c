//
// stdio.c
//
// Standard I/O routines
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
#include <stdlib.h>
#include <stdarg.h>

FILE *fopen(const char *filename, const char *mode)
{
  int oflag;
  int fd;

  switch (*mode)
  {
    case 'r':
      oflag = O_RDONLY;
      break;

    case 'w':
      oflag = O_WRONLY | O_CREAT | O_TRUNC;
      break;

    case 'a':
      oflag = O_WRONLY | O_CREAT | O_APPEND;
      break;
  
    default:
      errno = EINVAL;
      return NULL;
  }

  while (*++mode)
  {
    switch (*mode)
    {
      case '+':
	oflag |= O_RDWR;
	oflag &= ~(O_RDONLY | O_WRONLY);
	break;
    }
  }

  fd = open(filename, oflag);
  if (fd < 0) 
  {
    errno = -fd;
    return NULL;
  }

  return strmno(fd);
}

int fflush(FILE *stream)
{
  int rc;

  rc = flush(fileno(stream));
  if (rc < 0)
  {
    errno = -rc;
    return EOF;
  }

  return 0;
}

int fclose(FILE *stream)
{
  int rc;

  rc = close(fileno(stream));
  if (rc < 0)
  {
    errno = -rc;
    return EOF;
  }

  return 0;
}

int fseek(FILE *stream, long offset, int whence)
{
  int rc;

  rc = lseek(fileno(stream), offset, whence);
  if (rc < 0)
  {
    errno = -rc;
    return -1;
  }

  return 0;
}

long ftell(FILE *stream)
{
  int rc;

  rc = tell(fileno(stream));
  if (rc < 0)
  {
    errno = -rc;
    return -1;
  }

  return rc;
}

int feof(FILE *stream)
{
  return tell(fileno(stream)) == (loff_t) fstat(fileno(stream), NULL);
}

int fgetc(FILE *stream)
{
  unsigned char ch;
  int rc;

  rc = read(fileno(stream), &ch, 1);
  if (rc <= 0) return EOF;

  return ch;
}

int fputc(int c, FILE *stream)
{
  unsigned char ch;
  int rc;

  ch = (unsigned char) c;

  rc = write(fileno(stream), &ch, 1);
  if (rc <= 0) return EOF;

  return c;
}

char *fgets(char *buf, int n, FILE *stream)
{
  char *ptr = buf;
  int ch;

  if (n <= 0) return NULL;

  while (--n)
  {
    if ((ch = getc(stream)) == EOF)
    {
      if (ptr == buf)  return NULL;
      break;
    }

    if ((*ptr++ = ch) == '\n') break;
  }

  *ptr = '\0';
  return buf;
}

int fputs(const char *string, FILE *stream)
{
  int len;
  int rc;

  len = strlen(string);
  rc = write(fileno(stream), string, len);

  return rc == len ? 0 : EOF;
}

size_t fread(void *buffer, size_t size, size_t num, FILE *stream)
{
  int rc;
  int count;

  if ((count = size * num) == 0) return 0;

  rc = read(fileno(stream), buffer, size * num);
  if (rc < 0)
  {
    errno = -rc;
    return 0;
  }

  return rc / size;
}

size_t fwrite(const void *buffer, size_t size, size_t num, FILE *stream)
{
  int rc;
  int count;

  if ((count = size * num) == 0) return 0;

  rc = write(fileno(stream), buffer, size * num);
  if (rc < 0)
  {
    errno = -rc;
    return 0;
  }

  return rc / size;
}

char *gets(char *buf)
{
  char *p = buf;
  char ch;
  int rc;

  while (1)
  {
    rc = read(fdin, &ch, 1);
    if (rc == -ETIMEOUT) continue;
    if (rc <= 0) return NULL;

    if (ch == 8)
    {
      if (p > buf)
      {
	write(fdout, "\b \b", 3);
	p--;
      }
    }
    else if (ch == '\r' || ch =='\n' || (unsigned char) ch >= ' ')
    {
      write(fdout, &ch, 1);
      if (ch == '\r')
      {
	if (fdin == 0)
	{
	  write(fdout, "\n", 1);
	  break;
	}
	else
	  continue;
      }

      if (ch == '\n') break;
      *p++ = ch;
    }
  }

  *p = 0;
  return buf;
}

int puts(const char *string)
{
  int len;
  int rc;

  len = strlen(string);

  rc = write(fdout, string, len);
  if (rc <= 0) return EOF;

  rc = write(fdout, "\r\n", 2);
  if (rc <= 0) return EOF;

  return 0;
}

void perror(const char *message)
{
  if (message && *message)
  {
    write(fdout, message, strlen(message));
    write(fdout, ": ", 2);
  }

  message = strerror(errno);

  write(fdout, message, strlen(message));
  write(fdout, "\r\n", 2);
}

int vfprintf(FILE *stream, const char *fmt, va_list args)
{
  char buffer[1024];
  char *p;
  char *q;
  int n;
  int fd;

  fd = fileno(stream);

  n = vsprintf(buffer, fmt, args);

  p = buffer;
  while (*p)
  {
    q = p;
    while (*q && *q != '\n') q++;

    if (p != q) write(fd, p, q - p);
    if (*q == '\n') 
    {
      write(fd, "\r\n", 2);
      q++;
    }
    p = q;
  }

  return n;
}

int fprintf(FILE *stream, const char *fmt,...)
{
  va_list args;
  int n;

  va_start(args, fmt);
  n = vfprintf(stream, fmt, args);
  va_end(args);

  return n;
}

int vprintf(const char *fmt, va_list args)
{
  return vfprintf(stdout, fmt, args);
}

int printf(const char *fmt,...)
{
  va_list args;
  int n;

  va_start(args, fmt);
  n = vprintf(fmt, args);
  va_end(args);

  return n;
}
