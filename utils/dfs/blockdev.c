#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef WIN32
#include <io.h>
#include <windows.h>
#endif

#ifdef __linux__
#include <unistd.h>
#endif

#include "blockdev.h"

extern struct blockdriver bdrv_vmdk;

//
// RAW block device driver
//

struct rawdev
{
  int fd;
};

static int raw_probe(const uint8_t *buf, int buf_size, const char *filename)
{
  return 1; // maybe
}

static int raw_open(struct blockdevice *bs, const char *filename)
{
  struct rawdev *s = bs->opaque;
  int fd;
  int64_t size;
#ifdef WIN32
  struct __stat64 st;
#else
  struct stat st;
#endif

  fd = open(filename, O_RDWR | O_BINARY);
  if (fd == -1) return -1;

#ifdef WIN32
  if (_fstat64(fd, &st) == -1) return -1;
  size = st.st_size;
#else
  if (fstat(fd, &st) == -1) return -1;
  size = st.st_size;
#endif

  bs->total_sectors = size / 512;
  s->fd = fd;

  return 0;
}

static int raw_read(struct blockdevice *bs, int64_t sector_num,  uint8_t *buf, int nb_sectors)
{
  struct rawdev *s = bs->opaque;
  int64_t pos = sector_num * 512;
#ifdef WIN32
  if (_lseeki64(s->fd, pos, SEEK_SET) == -1) return -1;
#else
  if (lseek(s->fd, pos, SEEK_SET) == -1) return -1;
#endif
  if (read(s->fd, buf, nb_sectors * 512) != nb_sectors * 512) return -1;
  return 0;
}

static int raw_write(struct blockdevice *bs, int64_t sector_num,  const uint8_t *buf, int nb_sectors)
{
  struct rawdev *s = bs->opaque;
  int64_t pos = sector_num * 512;
  int rc;
#ifdef WIN32
  if (_lseeki64(s->fd, pos, SEEK_SET) == -1) return -1;
#else
  if (lseek(s->fd, pos, SEEK_SET) == -1) return -1;
#endif
  rc = write(s->fd, buf, nb_sectors * 512);
  if (rc != nb_sectors * 512) return -1;
  return 0;
}

static void raw_close(struct blockdevice *bs)
{
  struct rawdev *s = bs->opaque;
  close(s->fd);
}

#ifdef WIN32
static int raw_create(const char *filename, int64_t total_size, int flags)
{
  HANDLE hdev;
  int64_t size;

  if (flags) return -1;

  hdev = CreateFile(filename, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
  if (hdev == INVALID_HANDLE_VALUE) return -1;

  size = total_size * 512;
  SetFilePointerEx(hdev, *(LARGE_INTEGER*) &size, NULL, FILE_BEGIN);
  SetEndOfFile(hdev);
  CloseHandle(hdev);

  return 0;
}
#else
static int raw_create(const char *filename, int64_t total_size, int flags)
{
  int fd;
  off_t size;
  int rc;

  if (flags) return -1;

  fd = creat(filename, 0644);
  if (fd == -1) {
    perror(filename);
    return -1;
  }

  size = total_size * 512;
  if (ftruncate(fd, size) < 0) perror(filename);
  close(fd);

  return 0;
}
#endif

struct blockdriver bdrv_raw = 
{
  "raw",
  sizeof(struct rawdev),
  raw_probe,
  raw_open,
  raw_read,
  raw_write,
  raw_close,
  raw_create,
};

//
// Block device functions
//

struct blockdriver *drivers[] = {&bdrv_raw, &bdrv_vmdk, NULL};

static struct blockdriver *find_image_format(const char *filename)
{
  int buflen, score, score_max;
  struct blockdriver *drv1, *drv;
  uint8_t *buf;
  size_t bufsize = 1024;
  int fd;
  int i;

  fd = open(filename, O_BINARY);
  if (fd == -1)
  {
    buf = NULL;
    buflen = 0;
  } 
  else 
  {
    buf = malloc(bufsize);
    if (!buf) return NULL;
    buflen = read(fd, buf, bufsize);
    if (buflen <= 0)
    {
      close(fd);
      free(buf);
      return NULL;
    }
    close(fd);
  }
  
  drv = NULL;
  score_max = 0;
  for (i = 0; drivers[i]; i++)
  {
    drv1 = drivers[i];
    score = drv1->bdrv_probe(buf, buflen, filename);
    if (score > score_max) 
    {
      score_max = score;
      drv = drv1;
    }
    drv1++;
  }

  free(buf);
  return drv;
}

static struct blockdriver *get_driver(char *type)
{
  int i;

  for (i = 0; drivers[i]; i++)
  {
    if (strcmp(drivers[i]->format_name, type) == 0) return drivers[i];
  }
  return NULL;
}

int bdrv_create(char *type, const char *filename, int64_t size_in_sectors, int flags)
{
  struct blockdriver *drv;

  drv = get_driver(type);
  if (!drv) return -1;

  if (!drv->bdrv_create) return -1;
  return drv->bdrv_create(filename, size_in_sectors, flags);
}

int bdrv_open(struct blockdevice *bs, const char *filename, struct blockdriver *drv)
{
  int ret;
  
  strncpy(bs->filename, filename, sizeof(bs->filename));
  if (!drv) 
  {
    drv = find_image_format(filename);
    if (!drv) return -1;
  }

  bs->drv = drv;
  bs->opaque = malloc(drv->instance_size);
  if (drv->instance_size > 0)
  {
    if (bs->opaque == NULL) return -1;
    memset(bs->opaque, 0, drv->instance_size);
  }
    
  ret = drv->bdrv_open(bs, filename);
  if (ret < 0) 
  {
    if (bs->opaque) free(bs->opaque);
    return -1;
  }

  return 0;
}

void bdrv_close(struct blockdevice *bs)
{
  bs->drv->bdrv_close(bs);
  free(bs->opaque);
  bs->opaque = NULL;
  bs->drv = NULL;
}

int bdrv_read(struct blockdevice *bs, int64_t sector_num, uint8_t *buf, int nb_sectors)
{
  return bs->drv->bdrv_read(bs, sector_num, buf, nb_sectors);
}

int bdrv_write(struct blockdevice *bs, int64_t sector_num, uint8_t *buf, int nb_sectors)
{
  return bs->drv->bdrv_write(bs, sector_num, buf, nb_sectors);
}
