//
// ramdisk.c
//
// RAM disk driver
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

#include <os/krnl.h>

#define SECTORSIZE              512

struct ramdisk
{
  unsigned int blks;
  char *data;
};

static int ramdisk_ioctl(struct dev *dev, int cmd, void *args, size_t size)
{
  struct ramdisk *rd = (struct ramdisk *) dev->privdata;

  switch (cmd)
  {
    case IOCTL_GETDEVSIZE:
      return rd->blks;

    case IOCTL_GETBLKSIZE:
      return SECTORSIZE;
  }

  return -ENOSYS;
}

static int ramdisk_read(struct dev *dev, void *buffer, size_t count, blkno_t blkno)
{
  struct ramdisk *rd = (struct ramdisk *) dev->privdata;

  if (count == 0) return 0;
  if (blkno + count / SECTORSIZE > rd->blks) return -EFAULT;
  memcpy(buffer, rd->data + blkno * SECTORSIZE, count);
  return count;
}

static int ramdisk_write(struct dev *dev, void *buffer, size_t count, blkno_t blkno)
{
  struct ramdisk *rd = (struct ramdisk *) dev->privdata;

  if (count == 0) return 0;
  if (blkno + count / SECTORSIZE > rd->blks) return -EFAULT;
  memcpy(rd->data + blkno * SECTORSIZE, buffer, count);
  return count;
}

struct driver ramdisk_driver =
{
  "ramdisk",
  DEV_TYPE_BLOCK,
  ramdisk_ioctl,
  ramdisk_read,
  ramdisk_write
};

int __declspec(dllexport) install_ramdisk(char *opts)
{
  struct ramdisk *rd;
  char devname[DEVNAMELEN];
  int size;
  devno_t devno;

  get_option(opts, "devname", devname, DEVNAMELEN, "ramdisk#");
  size = get_num_option(opts, "size", 1440) * K;

  rd = kmalloc(sizeof(struct ramdisk));
  rd->blks = size / SECTORSIZE;
  rd->data = kmalloc(size);
  devno = dev_make(devname, &ramdisk_driver, NULL, rd);
  
  kprintf("%s: ramdisk (%d KB)\n", device(devno)->name, size / K);
  return 0;
}
