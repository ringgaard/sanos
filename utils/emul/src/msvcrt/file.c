#include "msvcrt.h"

#define _NSTREAM_ 128

crtapi struct _iobuf _iob[_NSTREAM_];

struct critsect iob_lock;

#define stdin  (&_iob[0])
#define stdout (&_iob[1])
#define stderr (&_iob[2])

static FILE *alloc_stream()
{
  FILE *stream;

  enter_critsect(&iob_lock);
  stream = _iob;
  while (stream < _iob + _NSTREAM_) 
  {
    if (stream->flag == 0)
    {
      stream->flag = -1;
      leave_critsect(&iob_lock);
      return stream;
    }

    stream++;
  }
  leave_critsect(&iob_lock);

  return NULL;
}

static void free_stream(FILE *stream)
{
  stream->flag = 0;
}

int _open(const char *filename, int oflag)
{
  syslog(LOG_DEBUG, "_open(%s,%p)\n", filename, oflag);
  return (int) open_file((char *) filename, oflag);
}

int _close(int handle)
{
  return close_file((handle_t) handle);
}

int _read(int handle, void *buffer, unsigned int count)
{
  if (handle == 0) handle = (int) get_std_handle(0);

  return read_file((handle_t) handle, buffer, count);
}

int _write(int handle, const void *buffer, unsigned int count)
{
  unsigned int rc;

  //syslog(LOG_DEBUG, "_write %d bytes to %p\n", count, handle);
  
  if (handle == 1)
    handle = (int) get_std_handle(1);
  else if (handle == 2)
    handle = (int) get_std_handle(2);

  rc = write_file((handle_t) handle, (void *) buffer, count);
  if (rc != count) panic("error writing to file");
  return rc;
}

int _setmode(int handle, int mode)
{
  // TODO: check that mode is O_BINARY
  return 0;
}

int _stat(const char *path, struct _stat *buffer)
{
  int rc;
  struct stat fs;

  syslog(LOG_DEBUG, "stat on %s\n", path);

  rc = get_file_stat((char *) path, &fs);
  if (rc < 0) return -1;

  if (buffer)
  {
    memset(buffer, 0, sizeof(struct _stat));
    buffer->st_atime = fs.atime;
    buffer->st_ctime = fs.ctime;
    buffer->st_mtime = fs.mtime;
    buffer->st_size = fs.quad.size_low;
    buffer->st_mode = S_IREAD | S_IWRITE | S_IEXEC;
    if (fs.mode & FILE_STAT_DIRECTORY)
      buffer->st_mode |= S_IFDIR;
    else
      buffer->st_mode |= S_IFREG;
  }

  syslog(LOG_DEBUG, "%s: mode=%d size=%d\n", path, buffer->st_mode, buffer->st_size);
  return 0;
}

__int64 _stati64(const char *path, struct _stati64 *buffer)
{
  int rc;
  struct stat fs;

  syslog(LOG_DEBUG, "stat on %s\n", path);

  rc = get_file_stat((char *) path, &fs);
  if (rc < 0) return -1;

  if (buffer)
  {
    memset(buffer, 0, sizeof(struct _stati64));
    buffer->st_atime = fs.atime;
    buffer->st_ctime = fs.ctime;
    buffer->st_mtime = fs.mtime;
    buffer->st_size = fs.size;
    buffer->st_mode = S_IREAD | S_IWRITE | S_IEXEC;
    if (fs.mode & FILE_STAT_DIRECTORY)
      buffer->st_mode |= S_IFDIR;
    else
      buffer->st_mode |= S_IFREG;
  }

  syslog(LOG_DEBUG, "%s: mode=%d size=%d\n", path, buffer->st_mode, buffer->st_size);
  return 0;
}

__int64 _fstati64(int handle, struct _stati64 *buffer)
{
  int rc;
  struct stat fs;

  rc = get_handle_stat((handle_t) handle, &fs);
  if (rc < 0) return -1;

  if (buffer)
  {
    memset(buffer, 0, sizeof(struct _stati64));
    buffer->st_atime = fs.atime;
    buffer->st_ctime = fs.ctime;
    buffer->st_mtime = fs.mtime;
    buffer->st_size = fs.size;
    buffer->st_mode = S_IREAD | S_IWRITE | S_IEXEC;
    if (fs.mode & FILE_STAT_DIRECTORY)
      buffer->st_mode |= S_IFDIR;
    else
      buffer->st_mode |= S_IFREG;
  }

  return 0;
}

__int64 _lseeki64(int handle, __int64 offset, int origin)
{
  return set_file_pointer((handle_t) handle, (int) offset, origin);
}

int _open_osfhandle(long osfhandle, int flags)
{
  return (int) dup_handle((handle_t) osfhandle);
}

long _get_osfhandle(int filehandle)
{
  return filehandle;
}

int _getdrive()
{
  // Drive C is current drive
  return 3;
}

char *_getdcwd(int drive, char *buffer, int maxlen)
{
  strcpy(buffer, "c:\\");
  return buffer;
}

char *_fullpath(char *abspath, const char *relpath, size_t maxlen)
{
  get_file_path((char *) relpath, abspath, maxlen);
  return abspath;
}

int _rename(const char *oldname, const char *newname)
{
  panic("rename not implemented");
  return 0;
}

int _access(const char *path, int mode)
{
  struct stat fs;
  int rc;

  syslog(LOG_DEBUG, "access check on %s\n", path);

  rc = get_file_stat((char *) path, &fs);
  if (rc < 0) return -1;
  return 0;
}

int _mkdir(const char *dirname)
{
  return mkdir((char *) dirname);
}

FILE *fopen(const char *filename, const char *mode)
{
  FILE *stream; 
  int oflag;
  handle_t handle;

  syslog(LOG_DEBUG, "fopen(%s,%s)\n", filename, mode);

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

  handle = open_file((char *) filename, oflag);
  if (handle == NOHANDLE) return NULL;

  stream = alloc_stream();
  if (stream == NULL) panic("too many files open");

  stream->file = handle;
  return stream;
}

int fclose(FILE *stream)
{
  close_file(stream->file);
  free_stream(stream);
  return 0;
}

int fflush(FILE *stream)
{
  // TODO: flush file buffers, for now we just ignore
  return 0;
}

int getc(FILE *stream)
{
  syslog(LOG_DEBUG, "getc on %p\n", stream - _iob);
  panic("getc not implemented");
  return 0;
}

int fputc(int c, FILE *stream)
{
  panic("fputc not implemented");
  return 0;
}

char *fgets(char *string, int n, FILE *stream)
{
  panic("fgets not implemented");
  return NULL;
}

int fprintf(FILE *stream, const char *fmt, ...)
{
  va_list args;
  int n;
  char buffer[1024];

  va_start(args, fmt);
  n = vformat(buffer, fmt, args);
  va_end(args);
  write_file(stream->file, buffer, strlen(buffer));
  return n;
}

int vfprintf(FILE *stream, const char *fmt, va_list args)
{
  char buffer[1024];

  vformat(buffer, fmt, args);
  
  //syslog(LOG_DEBUG, "vfprintf called (fileno = %d) (%s)\n", stream - _iob, buffer);

  return write_file(stream->file, buffer, strlen(buffer));
}

int putchar(int c)
{
  char buf[2];

  buf[0] = c;
  buf[1] = 0;
  print_string(buf);
  return c;
}

void init_fileio()
{
  int i;

  init_critsect(&iob_lock);

  memset(_iob, 0, sizeof(struct _iobuf) * _NSTREAM_);
  for (i = 0; i < _NSTREAM_; i++) _iob[i].file = NOHANDLE;

  stdin->file = get_std_handle(0);
  stdin->flag = -1;
  stdout->file = get_std_handle(1);
  stdout->flag = -1;
  stderr->file = get_std_handle(2);
  stderr->flag = -1;
}
