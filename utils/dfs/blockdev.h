#ifndef BLOCKDEV_H
#define BLOCKDEV_H

#include "types.h"

struct blockdevice;

struct blockdriver 
{
  const char *format_name;
  int instance_size;
  int (*bdrv_probe)(const uint8_t *buf, int buf_size, const char *filename);
  int (*bdrv_open)(struct blockdevice *bs, const char *filename);
  int (*bdrv_read)(struct blockdevice *bs, int64_t sector_num, uint8_t *buf, int nb_sectors);
  int (*bdrv_write)(struct blockdevice *bs, int64_t sector_num, const uint8_t *buf, int nb_sectors);
  void (*bdrv_close)(struct blockdevice *bs);
  int (*bdrv_create)(const char *filename, int64_t total_sectors, int flags);
  int (*bdrv_is_allocated)(struct blockdevice *bs, int64_t sector_num, int nb_sectors, int *pnum);
  int (*bdrv_set_key)(struct blockdevice *bs, const char *key);
};

struct blockdevice 
{
  int64_t total_sectors;
  struct blockdriver *drv;
  void *opaque;
  char filename[1024];
  
  // NOTE: the following infos are only hints for real hardware drivers. 
  // They are not used by the block driver

  int cyls, heads, secs, translation;
  int type;
  char device_name[32];
};

int bdrv_create(char *type, const char *filename, int64_t size_in_sectors, int flags);
int bdrv_open(struct blockdevice *bs, const char *filename, struct blockdriver *drv);
void bdrv_close(struct blockdevice *bs);
int bdrv_read(struct blockdevice *bs, int64_t sector_num, uint8_t *buf, int nb_sectors);
int bdrv_write(struct blockdevice *bs, int64_t sector_num, uint8_t *buf, int nb_sectors);

#endif
