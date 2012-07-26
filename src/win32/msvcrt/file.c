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

#define _STDINBUFSIZ_ 256
#define _NSTREAM_     128

struct _iobuf _iob[_NSTREAM_];
char stdinbuf[_STDINBUFSIZ_];
int stdinbufpos = 0;
int stdinbuflen = 0;
struct critsect iob_lock;

#define stdin  (&_iob[0])
#define stdout (&_iob[1])
#define stderr (&_iob[2])

int vsprintf(char *buf, const char *fmt, va_list args);
int readline(char *buf, int size);

static FILE *alloc_stream() {
  FILE *stream;

  enter(&iob_lock);
  stream = _iob;
  while (stream < _iob + _NSTREAM_)  {
    if (stream->flag & _IOFREE) {
      stream->flag = 0;
      leave(&iob_lock);
      return stream;
    }

    stream++;
  }
  leave(&iob_lock);

  return NULL;
}

static void free_stream(FILE *stream) {
  stream->flag = _IOFREE;
}

int _pipe(int *phandles, unsigned int psize, int textmode) {
  TRACE("_pipe");
  return pipe(phandles);
}

int _open(const char *filename, int oflag) {
  TRACE("_open");
  //syslog(LOG_DEBUG, "_open(%s,%p)", filename, oflag);
  return open(filename, oflag, S_IREAD | S_IWRITE);
}

int _close(int handle) {
  TRACE("_close");

  if (handle == -1) {
    errno = EBADF;
    return -1;
  }

  return close(handle);
}

int _commit(int handle) {
  TRACE("_commit");
  return fsync(handle);
}

int _read(int handle, void *buffer, unsigned int count) {
  TRACE("_read");

#if 0
  if (handle == 0) {
    char *buf;
    int pos;

    // Handle console input using readline
    if (stdinbufpos == stdinbuflen) {
      int len;

      len = readline(stdinbuf, sizeof(stdinbuf) - 3);
      if (len < 0) return -1;

      stdinbuf[len++] = '\r';
      stdinbuf[len++] = '\n';
      stdinbuf[len] = 0;

      stdinbuflen = len;
      stdinbufpos = 0;
    }

    buf = buffer;
    pos = 0;
    while (pos < (int) count && stdinbufpos < stdinbuflen) {
      buf[pos++] = stdinbuf[stdinbufpos++];
    }
    return pos;
  } else {
    return read(handle, buffer, count);
  }
#else
  return read(handle, buffer, count);
#endif
}

int _write(int handle, const void *buffer, unsigned int count) {
  TRACE("_write");
  
  return write(handle, buffer, count);
}

int _setmode(int handle, int mode) {
  TRACE("_setmode");
  return setmode(handle, mode);
}

int _stat(const char *path, struct _stat *buffer) {
  struct stat64 fs;

  TRACE("_stat");
  //syslog(LOG_DEBUG, "stat on %s", path);

  if (stat64(path, &fs) < 0) return -1;

  if (buffer) {
    memset(buffer, 0, sizeof(struct _stat));
    buffer->st_atime = fs.st_atime;
    buffer->st_ctime = fs.st_ctime;
    buffer->st_mtime = fs.st_mtime;
    buffer->st_size = (int) fs.st_size;
    buffer->st_mode = fs.st_mode;
  }

  //syslog(LOG_DEBUG, "%s: mode=%d size=%d", path, buffer->st_mode, buffer->st_size);
  return 0;
}

__int64 _stati64(const char *path, struct _stati64 *buffer) {
  struct stat64 fs;

  TRACE("_stati64");
  //syslog(LOG_DEBUG, "stat on %s", path);

  if (stat64(path, &fs) < 0) return -1;

  if (buffer) {
    memset(buffer, 0, sizeof(struct _stati64));
    buffer->st_atime = fs.st_atime;
    buffer->st_ctime = fs.st_ctime;
    buffer->st_mtime = fs.st_mtime;
    buffer->st_size = fs.st_size;
    buffer->st_mode = fs.st_mode;
  }

  //syslog(LOG_DEBUG, "%s: mode=%d size=%d", path, buffer->st_mode, buffer->st_size);
  return 0;
}

int _fstat(int handle, struct _stat *buffer) {
  struct stat64 fs;

  TRACE("_fstat");
  if (fstat64(handle, &fs) < 0) return -1;

  if (buffer) {
    memset(buffer, 0, sizeof(struct _stat));
    buffer->st_atime = fs.st_atime;
    buffer->st_ctime = fs.st_ctime;
    buffer->st_mtime = fs.st_mtime;
    buffer->st_size = (int) fs.st_size;
    buffer->st_mode = fs.st_mode;
  }

  return 0;
}

__int64 _fstati64(int handle, struct _stati64 *buffer) {
  struct stat64 fs;

  TRACE("_fstati64");
  if (fstat64(handle, &fs) < 0) return -1;

  if (buffer) {
    memset(buffer, 0, sizeof(struct _stati64));
    buffer->st_atime = fs.st_atime;
    buffer->st_ctime = fs.st_ctime;
    buffer->st_mtime = fs.st_mtime;
    buffer->st_size = fs.st_size;
    buffer->st_mode = fs.st_mode;
  }

  return 0;
}

loff_t _lseek(int f, loff_t offset, int origin) {
  TRACE("_lseek");
  return lseek(f, offset, origin);
}

__int64 _lseeki64(int handle, __int64 offset, int origin) {
  TRACE("_lseeki64");
  return (int) lseek64(handle, offset, origin);
}

int _open_osfhandle(long osfhandle, int flags) {
  int rc;

  TRACE("_open_osfhandle");

  // Handles are the same as operating system handles
  // Just return the original handle after changing the mode
  rc = setmode(osfhandle, flags);
  if (rc < 0) return -1;

  return osfhandle;
}

int _dup2(int handle1, int handle2) {
  TRACE("_dup2");
  return dup2(handle1, handle2);
}

long _get_osfhandle(int filehandle) {
  TRACE("_get_osfhandle");
  return filehandle;
}

int _getdrive() {
  TRACE("_getdrive");
  // Drive C is current drive
  return 3;
}

char *_getdcwd(int drive, char *buffer, int maxlen) {
  TRACE("_getdcwd");
  return getcwd(buffer, maxlen);
}

char *_fullpath(char *abspath, const char *relpath, size_t maxlen) {
  int rc;

  TRACE("_fullpath");

  if (maxlen < 3) {
    errno = ERANGE;
    return NULL;
  }

  abspath[0] = 'C';
  abspath[1] = ':';

  rc = canonicalize(relpath, abspath + 2, maxlen - 2);
  if (rc < 0) return NULL;

  return abspath;
}

int _unlink(const char *filename) {
  TRACE("_unlink");
  return unlink(filename);
}

int remove(const char *path) {
  TRACE("_remove");
  return unlink(path);
}

int _rename(const char *oldname, const char *newname) {
  TRACE("_rename");
  return rename(oldname, newname);
}

int _access(const char *path, int mode) {
  TRACE("_access");
  return access(path, mode);
}

int _chmod(const char *filename, int pmode) {
  TRACE("_chmod");
  return chmod(filename, pmode);
}

int _mkdir(const char *dirname) {
  TRACE("_mkdir");
  return mkdir(dirname, 0666);
}

int _chdir(const char *dirname) {
  TRACE("_chdir");
  return chdir(dirname);
}

char *_getcwd(char *buffer, int maxlen) {
  TRACE("_getcwd");
  return getcwd(buffer, maxlen);
}

int _fileno(FILE *stream) {
  TRACE("_fileno");
  return stream->file;
}

FILE *_fdopen(int handle, const char *mode) {
  FILE *stream; 

  TRACE("fdopen");

  stream = alloc_stream();
  if (stream == NULL) panic("too many files open");

  stream->file = handle;
  return stream;
}

FILE *fopen(const char *filename, const char *mode) {
  FILE *stream; 
  int oflag;
  handle_t handle;

  TRACE("fopen");
  //syslog(LOG_DEBUG, "fopen(%s,%s)", filename, mode);

  switch (*mode) {
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

  while (*++mode) {
    switch (*mode) {
      case '+':
        oflag &= ~(O_RDONLY | O_WRONLY);
        oflag |= O_RDWR;
        break;

      case 't':
        oflag &= ~(O_TEXT | O_BINARY);
        oflag |= O_TEXT;
        break;

      case 'b':
        oflag &= ~(O_TEXT | O_BINARY);
        oflag |= O_BINARY;
        break;

      case 'c':
      case 'n':
        break;

      case 'S':
        oflag |= O_SEQUENTIAL;
        break;

      case 'R':
        oflag |= O_RANDOM;
        break;

      case 'T':
        oflag |= O_SHORT_LIVED;
        break;

      case 'D':
        oflag |= O_TEMPORARY;
        break;

      default:
        errno = EINVAL;
        return NULL;
    }
  }

  handle = open(filename, oflag, S_IREAD | S_IWRITE);
  if (handle < 0) return NULL;

  stream = alloc_stream();
  if (stream == NULL) panic("too many files open");

  stream->file = handle;
  return stream;
}

FILE *freopen(const char *path, const char *mode, FILE *stream) {
  int oflag;
  handle_t handle;

  TRACE("freopen");

  switch (*mode) {
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

  while (*++mode) {
    switch (*mode) {
      case '+':
        oflag &= ~(O_RDONLY | O_WRONLY);
        oflag |= O_RDWR;
        break;

      case 't':
        oflag &= ~(O_TEXT | O_BINARY);
        oflag |= O_TEXT;
        break;

      case 'b':
        oflag &= ~(O_TEXT | O_BINARY);
        oflag |= O_BINARY;
        break;

      case 'c':
      case 'n':
        break;

      case 'S':
        oflag |= O_SEQUENTIAL;
        break;

      case 'R':
        oflag |= O_RANDOM;
        break;

      case 'T':
        oflag |= O_SHORT_LIVED;
        break;

      case 'D':
        oflag |= O_TEMPORARY;
        break;

      default:
        errno = EINVAL;
        return NULL;
    }
  }

  handle = open(path, oflag, S_IREAD | S_IWRITE);
  if (handle < 0) return NULL;

  close(stream->file);
  stream->file = handle;
  return stream;
}

int fclose(FILE *stream) {
  int rc;

  TRACE("fclose");
  rc = close(stream->file);
  free_stream(stream);
  return rc;
}

int fflush(FILE *stream) {
  int rc;

  TRACE("fflush");
  rc = fsync(stream->file);
  if (rc < 0) {
    stream->flag |= _IOERR;
    return -1;
  }

  return rc;
}

size_t fread(void *buffer, size_t size, size_t num, FILE *stream) {
  int rc;
  int count;

  TRACE("fread");

  if ((count = size * num) == 0) return 0;

  rc = read(stream->file, buffer, size * num);
  if (rc < 0) {
    stream->flag |= _IOERR;
    return 0;
  }

  if (rc == 0) stream->flag |= _IOEOF;

  return rc / size;
}

size_t fwrite(const void *buffer, size_t size, size_t num, FILE *stream) {
  int rc;
  int count;

  TRACE("fwrite");

  if ((count = size * num) == 0) return 0;

  rc = write(stream->file, buffer, size * num);
  if (rc < 0) {
    stream->flag |= _IOERR;
    return 0;
  }

  return rc / size;
}

int fputs(const char *string, FILE *stream) {
  int len;
  int rc;

  TRACE("fputs");

  len = strlen(string);
  rc = write(stream->file, string, len);

  return rc == len ? 0 : EOF;
}

int fseek(FILE *stream, long offset, int whence) {
  TRACE("fseek");
  return lseek(stream->file, offset, whence);
}

long ftell(FILE *stream) {
  return tell(stream->file);
}

int fgetpos(FILE *stream, fpos_t *pos) {
  int rc;

  TRACE("fgetpos");

  rc = tell(stream->file);
  if (rc < 0) return -1;

  *pos = rc;
  return 0;
}

void clearerr(FILE *stream) {
  TRACE("clearerr");
  stream->flag &= ~(_IOERR | _IOEOF);
}

int getc(FILE *stream) {
  unsigned char ch;
  int rc;

  TRACE("getc");

  rc = read(stream->file, &ch, 1);
  if (rc <= 0) {
    if (rc == 0) { 
      stream->flag |= _IOEOF;
    } else {
      stream->flag |= _IOERR;
    }
    return EOF;
  }

  return ch;
}

int fgetc(FILE *stream) {
  unsigned char ch;
  int rc;

  TRACE("fgetc");

  rc = read(stream->file, &ch, 1);
  if (rc <= 0) {
    if (rc == 0) {
      stream->flag |= _IOEOF;
    } else {
      stream->flag |= _IOERR;
    }
    return EOF;
  }

  return ch;
}

int fputc(int c, FILE *stream) {
  char ch;

  TRACE("fputc");
  ch = c;
  if (write(stream->file, &ch, 1) < 0) return -1;
  return c;
}

char *fgets(char *string, int n, FILE *stream) {
  TRACE("fgets");
  panic("fgets not implemented");
  return NULL;
}

int fprintf(FILE *stream, const char *fmt, ...) {
  va_list args;
  int n;
  char buffer[1024];

  TRACEX("fprintf");
  va_start(args, fmt);
  n = vsprintf(buffer, fmt, args);
  va_end(args);
  return write(stream->file, buffer, n);
}

int vfprintf(FILE *stream, const char *fmt, va_list args) {
  int n;
  char buffer[1024];

  TRACEX("vfprintf");
  n = vsprintf(buffer, fmt, args);
  return write(stream->file, buffer, n);
}

int putchar(int c) {
  char ch;

  TRACEX("putchar");
  ch = c;
  write(stdout->file, &ch, 1);
  return c;
}

int puts(const char *string) {
  int len;
  int rc;

  TRACE("puts");

  len = strlen(string);
  rc = write(stdout->file, string, len);
  if (rc < 0) return EOF;

  rc = write(stdout->file, "\n", 1);
  if (rc < 0) return EOF;

  return 0;
}

void _splitpath(const char *path, char *drive, char *dir, char *fname, char *ext) {
  char *p;
  char *last_slash = NULL, *dot = NULL;
  int len;

  TRACE("_splitpath");
  if (strlen(path) >= 1 && path[1] == ':') {
    if (drive)  {
      drive[0] = path[0];
      drive[1] = '\0';
    }

    path += 2;
  } else if (drive) {
    *drive = '\0';
  }

  for (last_slash = NULL, p = (char *) path; *p; p++)  {
    if (*p == '/' || *p == '\\') {
     last_slash = p + 1;
    } else if (*p == '.') {
      dot = p;
    }
  }

  if (last_slash) {
    if (dir) {
      len = last_slash - path;
      if (len > MAXPATH - 1) len = MAXPATH - 1;
      memcpy(dir, path, len);
      dir[len] = '\0';
    }

    path = last_slash;
  } else if (dir) {
    *dir = '\0';
  }

  if (dot && dot >= path) {
    if (fname) {
      len = dot - path;
      if (len > MAXPATH - 1) len = MAXPATH - 1;
      memcpy(fname, path, len);
      fname[len] = '\0';
    }

    if (ext) {
      len = p - dot;
      if (len > MAXPATH - 1) len = MAXPATH - 1;
      memcpy(ext, dot, len);
      ext[len] = '\0';
    }
  } else {
    if (fname) {
      len = p - path;
      if (len > MAXPATH - 1) len = MAXPATH - 1;
      memcpy(fname, path, len);
      fname[len] = '\0';
    }
    if (ext) {
      *ext = '\0';
    }
  }
}

int _wopen(const wchar_t *filename, int oflag) {
  char buf[MAXPATH];
  int rc;

  TRACE("_wopen");
  rc = convert_filename_from_unicode(filename, buf, MAXPATH);
  if (rc < 0) return -1;

  return _open(buf, oflag);
}

int _waccess(const wchar_t *path, int mode) {
  char buf[MAXPATH];
  int rc;

  TRACE("_waccess");
  rc = convert_filename_from_unicode(path, buf, MAXPATH);
  if (rc < 0) return -1;

  return _access(buf, mode);
}

__int64 _wstati64(const wchar_t *path, struct _stati64 *buffer) {
  char buf[MAXPATH];
  int rc;

  TRACE("_wstati64");
  rc = convert_filename_from_unicode(path, buf, MAXPATH);
  if (rc < 0) return -1;

  return _stati64(buf, buffer);
}

int _wmkdir(const wchar_t *dirname) {
  char buf[MAXPATH];
  int rc;

  TRACE("_wmkdir");
  rc = convert_filename_from_unicode(dirname, buf, MAXPATH);
  if (rc < 0) return -1;

  return _mkdir(buf);
}

int _wrename(const wchar_t *oldname, const wchar_t *newname) {
  char buf1[MAXPATH];
  char buf2[MAXPATH];
  int rc;

  TRACE("_wrename");
  rc = convert_filename_from_unicode(oldname, buf1, MAXPATH);
  if (rc < 0) return -1;

  rc = convert_filename_from_unicode(newname, buf2, MAXPATH);
  if (rc < 0) return -1;

  return _rename(buf1, buf2);
}

wchar_t *_wgetdcwd(int drive, wchar_t *buffer, int maxlen) {
  char curdir[MAXPATH];
  int len;

  TRACE("_wgetdcwd");
  if (getcwd(curdir, MAXPATH) == 0) return NULL;
  len = strlen(curdir);

  if (buffer) {
    if (len >= maxlen) {
      errno = ERANGE;
      return NULL;
    }
  } else {
    if (maxlen == 0) {
      maxlen = len + 1;
    } else if (len >= maxlen) {
      errno = ERANGE;
      return NULL;
    }

    buffer = malloc(maxlen * sizeof(wchar_t));

    if (!buffer) {
      errno = ENOMEM;
      return NULL;
    }
  }

  convert_filename_to_unicode(curdir, buffer, maxlen);
  return buffer;
}

wchar_t *_wfullpath(wchar_t *abspath, const wchar_t *relpath, size_t maxlen) {
  char buf1[MAXPATH];
  char buf2[MAXPATH];
  int rc;

  TRACE("_wfullpath");
  
  if (maxlen < 2) {
    errno = EINVAL;
    return NULL;
  }

  rc = convert_filename_from_unicode(relpath, buf1, MAXPATH);
  if (rc < 0) return NULL;

  rc = canonicalize(buf1, buf2, MAXPATH);
  if (rc < 0) return NULL;

  if (!abspath) {
    maxlen = MAXPATH;
    abspath = (wchar_t *) malloc(maxlen * sizeof(wchar_t));
    if (!abspath) {
      errno = ENOMEM;
      return NULL;
    }
  }

  abspath[0] = 'c';
  abspath[1] = ':';
  rc = convert_filename_to_unicode(buf2, abspath + 2, maxlen - 2);
  if (rc < 0) return NULL;

  return abspath;
}

void init_fileio() {
  int i;

  mkcs(&iob_lock);

  memset(_iob, 0, sizeof(struct _iobuf) * _NSTREAM_);
  for (i = 0; i < _NSTREAM_; i++) {
    _iob[i].file = NOHANDLE;
    _iob[i].flag = _IOFREE;
  }

  stdin->file = fdin;
  stdin->flag = 0;
  stdout->file = fdout;
  stdout->flag = 0;
  stderr->file = fderr;
  stderr->flag = 0;
}
