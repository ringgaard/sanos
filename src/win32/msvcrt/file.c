//
// file.c
//
// File I/O
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

#include "msvcrt.h"

#define _NSTREAM_ 128

crtapi struct _iobuf _iob[_NSTREAM_];

struct critsect iob_lock;

#define stdin  (&_iob[0])
#define stdout (&_iob[1])
#define stderr (&_iob[2])

int vsprintf(char *buf, const char *fmt, va_list args);

static FILE *alloc_stream()
{
  FILE *stream;

  enter(&iob_lock);
  stream = _iob;
  while (stream < _iob + _NSTREAM_) 
  {
    if (stream->flag == 0)
    {
      stream->flag = -1;
      leave(&iob_lock);
      return stream;
    }

    stream++;
  }
  leave(&iob_lock);

  return NULL;
}

static void free_stream(FILE *stream)
{
  stream->flag = 0;
}

int _pipe(int *phandles, unsigned int psize, int textmode)
{
  TRACE("pipe");
  errno = -ENOSYS;
  return -1;
}

int _open(const char *filename, int oflag)
{
  int rc;

  TRACE("_open");
  //syslog(LOG_DEBUG, "_open(%s,%p)\n", filename, oflag);
  rc = open(filename, oflag);
  if (rc < 0)
  {
    errno = rc;
    return -1;
  }

  return rc;
}

int _close(int handle)
{
  int rc;

  TRACE("_close");
  rc = close(handle);
  if (rc < 0)
  {
    errno = rc;
    return -1;
  }

  return rc;
}

int _commit(int handle)
{
  int rc;

  TRACE("_commit");
  rc = flush(handle);
  if (rc < 0)
  {
    errno = rc;
    return -1;
  }

  return rc;
}

int _read(int handle, void *buffer, unsigned int count)
{
  int rc;

  TRACE("_read");
  rc = read(handle, buffer, count);
  if (rc < 0)
  {
    errno = rc;
    return -1;
  }

  return rc;
}

int _write(int handle, const void *buffer, unsigned int count)
{
  int rc;

  TRACE("_write");
  
  rc = write(handle, buffer, count);
  if (rc < 0)
  {
    errno = rc;
    return -1;
  }

  return rc;
}

int _setmode(int handle, int mode)
{
  TRACE("_setmode");
  // TODO: check that mode is O_BINARY
  return 0;
}

int _stat(const char *path, struct _stat *buffer)
{
  int rc;
  struct stat fs;

  TRACE("_stat");
  //syslog(LOG_DEBUG, "stat on %s\n", path);

  rc = stat(path, &fs);
  if (rc < 0) 
  {
    errno = rc;
    return -1;
  }

  if (buffer)
  {
    memset(buffer, 0, sizeof(struct _stat));
    buffer->st_atime = fs.atime;
    buffer->st_ctime = fs.ctime;
    buffer->st_mtime = fs.mtime;
    buffer->st_size = fs.quad.size_low;
    buffer->st_mode = S_IREAD | S_IWRITE | S_IEXEC;
    if (fs.mode & FS_DIRECTORY)
      buffer->st_mode |= S_IFDIR;
    else
      buffer->st_mode |= S_IFREG;
  }

  //syslog(LOG_DEBUG, "%s: mode=%d size=%d\n", path, buffer->st_mode, buffer->st_size);
  return 0;
}

__int64 _stati64(const char *path, struct _stati64 *buffer)
{
  int rc;
  struct stat fs;

  TRACE("_stati64");
  //syslog(LOG_DEBUG, "stat on %s\n", path);

  rc = stat(path, &fs);
  if (rc < 0) 
  {
    errno = rc;
    return -1;
  }

  if (buffer)
  {
    memset(buffer, 0, sizeof(struct _stati64));
    buffer->st_atime = fs.atime;
    buffer->st_ctime = fs.ctime;
    buffer->st_mtime = fs.mtime;
    buffer->st_size = fs.size;
    buffer->st_mode = S_IREAD | S_IWRITE | S_IEXEC;
    if (fs.mode & FS_DIRECTORY)
      buffer->st_mode |= S_IFDIR;
    else
      buffer->st_mode |= S_IFREG;
  }

  //syslog(LOG_DEBUG, "%s: mode=%d size=%d\n", path, buffer->st_mode, buffer->st_size);
  return 0;
}

int _fstat(int handle, struct _stat *buffer)
{
  int rc;
  struct stat fs;

  TRACE("_fstat");
  rc = fstat(handle, &fs);
  if (rc < 0) 
  {
    errno = rc;
    return -1;
  }

  if (buffer)
  {
    memset(buffer, 0, sizeof(struct _stat));
    buffer->st_atime = fs.atime;
    buffer->st_ctime = fs.ctime;
    buffer->st_mtime = fs.mtime;
    buffer->st_size = fs.quad.size_low;
    buffer->st_mode = S_IREAD | S_IWRITE | S_IEXEC;
    if (fs.mode & FS_DIRECTORY)
      buffer->st_mode |= S_IFDIR;
    else
      buffer->st_mode |= S_IFREG;
  }

  return 0;
}

__int64 _fstati64(int handle, struct _stati64 *buffer)
{
  int rc;
  struct stat fs;

  TRACE("_fstati64");
  rc = fstat(handle, &fs);
  if (rc < 0) 
  {
    errno = rc;
    return -1;
  }

  if (buffer)
  {
    memset(buffer, 0, sizeof(struct _stati64));
    buffer->st_atime = fs.atime;
    buffer->st_ctime = fs.ctime;
    buffer->st_mtime = fs.mtime;
    buffer->st_size = fs.size;
    buffer->st_mode = S_IREAD | S_IWRITE | S_IEXEC;
    if (fs.mode & FS_DIRECTORY)
      buffer->st_mode |= S_IFDIR;
    else
      buffer->st_mode |= S_IFREG;
  }

  return 0;
}

__int64 _lseeki64(int handle, __int64 offset, int origin)
{
  int rc;

  TRACE("_lseeki64");
  rc = lseek(handle, (int) offset, origin);
  if (rc < 0)
  {
    errno = rc;
    return -1;
  }

  return rc;
}

int _open_osfhandle(long osfhandle, int flags)
{
  int rc;

  TRACE("_open_osfhandle");
  rc = dup(osfhandle);
  if (rc < 0)
  {
    errno = rc;
    return -1;
  }

  return rc;
}

int _dup2(int handle1, int handle2)
{
  syslog(LOG_WARNING, "dup2(%d,%d) not implemented\n", handle1, handle2);
  errno = -EMFILE;
  return -1;
}

long _get_osfhandle(int filehandle)
{
  TRACE("_get_osfhandle");
  return filehandle;
}

int _getdrive()
{
  TRACE("_getdrive");
  // Drive C is current drive
  return 3;
}

char *_getdcwd(int drive, char *buffer, int maxlen)
{
  TRACE("_getdcwd");
  return getcwd(buffer, maxlen);
}

char *_fullpath(char *abspath, const char *relpath, size_t maxlen)
{
  int rc;

  TRACE("_fullpath");

  if (maxlen < 3) 
  {
    errno = -ERANGE;
    return NULL;
  }

  abspath[0] = 'C';
  abspath[1] = ':';

  rc = canonicalize(relpath, abspath + 2, maxlen - 2);
  if (rc < 0)
  {
    errno = rc;
    return NULL;
  }

  return abspath;
}

int _unlink(const char *filename)
{
  int rc;

  TRACE("_unlink");
  rc = unlink(filename);
  if (rc < 0)
  {
    errno = rc;
    return -1;
  }

  return rc;
}

int remove(const char *path)
{
  int rc;

  TRACE("_remove");
  rc = unlink(path);
  if (rc < 0)
  {
    errno = rc;
    return -1;
  }

  return rc;
}

int _rename(const char *oldname, const char *newname)
{
  int rc;

  TRACE("_rename");
  rc = rename(oldname, newname);
  if (rc < 0)
  {
    errno = rc;
    return -1;
  }

  return rc;
}

int _access(const char *path, int mode)
{
  int rc;

  TRACE("_access");

  rc = stat(path, NULL);
  if (rc < 0)
  {
    errno = rc;
    return -1;
  }

  return 0;
}

int _chmod(const char *filename, int pmode)
{
  int rc;

  TRACE("_chmod");

  // Just check that file exists
  rc = stat(filename, NULL);
  if (rc < 0)
  {
    errno = rc;
    return -1;
  }

  return 0;
}

int _mkdir(const char *dirname)
{
  int rc;

  TRACE("_mkdir");
  rc = mkdir(dirname);
  if (rc < 0)
  {
    errno = rc;
    return -1;
  }

  return rc;
}

int _chdir(const char *dirname)
{
  TRACE("_chdir");

  return chdir(dirname);
}

char *_getcwd(char *buffer, int maxlen)
{
  TRACE("_getcwd");

  return getcwd(buffer, maxlen);
}

int _fileno(FILE *stream)
{
  TRACE("_fileno");

  return stream->file;
}

FILE *_fdopen(int handle, const char *mode)
{
  FILE *stream; 

  TRACE("fdopen");

  stream = alloc_stream();
  if (stream == NULL) panic("too many files open");

  stream->file = handle;
  return stream;
}

FILE *fopen(const char *filename, const char *mode)
{
  FILE *stream; 
  int oflag;
  handle_t handle;

  TRACE("fopen");
  //syslog(LOG_DEBUG, "fopen(%s,%s)\n", filename, mode);

  switch (mode[0])
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
      return NULL;
  }

  if (mode[1] == '+')
  {
    oflag |= O_RDWR;
    oflag &= ~(O_RDONLY | O_WRONLY);
  }

  handle = open(filename, oflag);
  if (handle < 0) 
  {
    errno = handle;
    return NULL;
  }

  stream = alloc_stream();
  if (stream == NULL) panic("too many files open");

  stream->file = handle;
  return stream;
}

FILE *freopen(const char *path, const char *mode, FILE *stream)
{
  int oflag;
  handle_t handle;

  TRACE("freopen");

  switch (mode[0])
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
      return NULL;
  }

  if (mode[1] == '+')
  {
    oflag |= O_RDWR;
    oflag &= ~(O_RDONLY | O_WRONLY);
  }

  handle = open(path, oflag);
  if (handle < 0) 
  {
    errno = handle;
    return NULL;
  }

  close(stream->file);
  stream->file = handle;
  return stream;
}

int fclose(FILE *stream)
{
  int rc;

  TRACE("fclose");
  rc = close(stream->file);
  free_stream(stream);
  if (rc < 0)
  {
    errno = rc;
    return -1;
  }

  return rc;
}

int fflush(FILE *stream)
{
  int rc;

  TRACE("fflush");
  rc = flush(stream->file);
  if (rc < 0)
  {
    errno = rc;
    return -1;
  }

  return rc;
}

size_t fread(void *buffer, size_t size, size_t num, FILE *stream)
{
  int rc;
  int count;

  TRACE("fread");

  if ((count = size * num) == 0) return 0;

  rc = read(stream->file, buffer, size * num);
  if (rc < 0)
  {
    errno = rc;
    return 0;
  }

  return rc / size;
}

size_t fwrite(const void *buffer, size_t size, size_t num, FILE *stream)
{
  int rc;
  int count;

  TRACE("fwrite");

  if ((count = size * num) == 0) return 0;

  rc = write(stream->file, buffer, size * num);
  if (rc < 0)
  {
    errno = rc;
    return 0;
  }

  return rc / size;
}

int fputs(const char *string, FILE *stream)
{
  int len;
  int rc;

  TRACE("fputs");

  len = strlen(string);
  rc = write(stream->file, string, len);

  return rc == len ? 0 : EOF;
}

int fseek(FILE *stream, long offset, int whence)
{
  int rc;

  TRACE("fseek");

  rc = lseek(stream->file, offset, whence);
  if (rc < 0)
  {
    errno = rc;
    return -1;
  }

  return 0;
}

int fgetpos(FILE *stream, fpos_t *pos)
{
  int rc;

  TRACE("fgetpos");

  rc = tell(stream->file);
  if (rc < 0)
  {
    errno = rc;
    return -1;
  }

  *pos = rc;
  return 0;
}

void clearerr(FILE *stream)
{
  TRACE("clearerr");
}

int getc(FILE *stream)
{
  char ch;
  int rc;

  TRACE("getc");

  rc = read(stream->file, &ch, 1);
  if (rc <= 0) return EOF;
  return ch;
}

int fgetc(FILE *stream)
{
  unsigned char ch;
  int rc;

  TRACE("fgetc");

  rc = read(stream->file, &ch, 1);
  if (rc <= 0) return EOF;

  return ch;
}

int fputc(int c, FILE *stream)
{
  char ch;

  TRACE("fputc");
  ch = c;
  if (write(stream->file, &ch, 1) < 0) return -1;
  return c;
}

char *fgets(char *string, int n, FILE *stream)
{
  TRACE("fgets");
  panic("fgets not implemented");
  return NULL;
}

int fprintf(FILE *stream, const char *fmt, ...)
{
  va_list args;
  int n;
  char buffer[1024];

  TRACEX("fprintf");
  va_start(args, fmt);
  n = vsprintf(buffer, fmt, args);
  va_end(args);
  return write(stream->file, buffer, n);
}

int vfprintf(FILE *stream, const char *fmt, va_list args)
{
  int n;
  char buffer[1024];

  TRACEX("vfprintf");
  n = vsprintf(buffer, fmt, args);
  return write(stream->file, buffer, n);
}

int putchar(int c)
{
  char ch;

  TRACEX("putchar");
  ch = c;
  write(1, &ch, 1);
  return c;
}

void _splitpath(const char *path, char *drive, char *dir, char *fname, char *ext)
{
  char *p;
  char *last_slash = NULL, *dot = NULL;
  int len;

  if (strlen(path) >= 1 && path[1] == ':')
  {
      if (drive) 
      {
	drive[0] = path[0];
	drive[1] = '\0';
      }

      path += 2;
  }
  else if (drive) 
  {
    *drive = '\0';
  }

  for (last_slash = NULL, p = (char *) path; *p; p++) 
  {
    if (*p == '/' || *p == '\\')
     last_slash = p + 1;
    else if (*p == '.')
      dot = p;
  }

  if (last_slash) 
  {
    if (dir) 
    {
      len = last_slash - path;
      if (len > MAXPATH - 1) len = MAXPATH - 1;
      memcpy(dir, path, len);
      dir[len] = '\0';
    }

    path = last_slash;
  }
  else if (dir) 
  {
    *dir = '\0';
  }

  if (dot && dot >= path)
  {
    if (fname) 
    {
      len = dot - path;
      if (len > MAXPATH - 1) len = MAXPATH - 1;
      memcpy(fname, path, len);
      fname[len] = '\0';
    }

    if (ext) 
    {
      len = p - dot;
      if (len > MAXPATH - 1) len = MAXPATH - 1;
      memcpy(ext, dot, len);
      ext[len] = '\0';
    }
  }
  else 
  {
    if (fname) 
    {
      len = p - path;
      if (len > MAXPATH - 1) len = MAXPATH - 1;
      memcpy(fname, path, len);
      fname[len] = '\0';
    }
    if (ext) 
    {
      *ext = '\0';
    }
  }
}

void init_fileio()
{
  int i;

  mkcs(&iob_lock);

  memset(_iob, 0, sizeof(struct _iobuf) * _NSTREAM_);
  for (i = 0; i < _NSTREAM_; i++) _iob[i].file = NOHANDLE;

  stdin->file = 0;
  stdin->flag = -1;
  stdout->file = 1;
  stdout->flag = -1;
  stderr->file = 2;
  stderr->flag = -1;
}
