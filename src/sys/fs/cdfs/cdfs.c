//
// cdfs.c
//
// ISO-9660 CD-ROM Filesystem
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
#include "iso9660.h"

#define CDFS_DEFAULT_CACHESIZE 128
#define CDFS_BLOCKSIZE         2048

struct cdfs
{
  dev_t devno;
  int blks;
  struct bufpool *cache;
  unsigned char *path_table_buffer;
  struct iso_pathtable_record **path_table;
  int path_table_records;
};

struct cdfs_file
{
  unsigned long size;
  time_t date;
};

static int cdfs_read_path_table(struct cdfs *cdfs, struct iso_primary_descriptor *vd)
{
  struct buf *buf;
  unsigned char *pt;
  int ptblk;
  int ptlen;
  int ptpos;
  int n;

  // Determine size and location of path table and allocate buffer
  ptlen = isonum_733(vd->path_table_size);
  ptblk = isonum_731(vd->type_l_path_table);
  cdfs->path_table_buffer = malloc(ptlen);
  if (!cdfs->path_table_buffer) return -ENOMEM;

  // Read L path table into buffer
  ptpos = 0;
  while (ptpos < ptlen)
  {
    buf = get_buffer(cdfs->cache, ptblk++);
    if (!buf) return -EIO;

    if (ptlen - ptpos > CDFS_BLOCKSIZE)
    {
      memcpy(cdfs->path_table_buffer + ptpos, buf->data, CDFS_BLOCKSIZE);
      ptpos += CDFS_BLOCKSIZE;
    }
    else
    {
      memcpy(cdfs->path_table_buffer + ptpos, buf->data, ptlen - ptpos);
      ptpos = ptlen;
    }

    release_buffer(cdfs->cache, buf);
  }

  // Determine number of records in pathtable
  // Path table records are indexed from 1 (first entry not used)
  pt = cdfs->path_table_buffer;
  n = 1;
  while (pt < cdfs->path_table_buffer + ptlen)
  {
    struct iso_pathtable_record *pathrec = (struct iso_pathtable_record *) pt;
    int namelen;
    int reclen;

    namelen = isonum_711(pathrec->length);
    reclen = sizeof(struct iso_pathtable_record) + namelen + (namelen & 1);

    n++;
    pt += reclen;
  }

  cdfs->path_table_records = n;

  kprintf("%d path table records\n", cdfs->path_table_records);

  // Allocate path table
  cdfs->path_table = (struct iso_pathtable_record **) kmalloc(cdfs->path_table_records * sizeof(struct iso_pathtable_record **));
  if (!cdfs->path_table) return -ENOMEM;
  cdfs->path_table[0] = NULL;

  // Setup pointers into path table buffers
  pt = cdfs->path_table_buffer;
  for (n = 1; n < cdfs->path_table_records; n++)
  {
    struct iso_pathtable_record *pathrec = (struct iso_pathtable_record *) pt;
    int namelen;
    int reclen;

    namelen = isonum_711(pathrec->length);
    reclen = sizeof(struct iso_pathtable_record) + namelen + (namelen & 1);

    cdfs->path_table[n] = pathrec;
    pt += reclen;
  }

  return 0;
}

static int cdfs_find_dir(char *name, int len)
{
  char *p;
  int l;
  int dir = 2;
  int parent = 1;

  while (1)
  {
    // Skip path separator
    if (*name == PS1 || *name == PS2)
    {
      name++;
      len--;
    }
    if (len == 0) return dir;

    // Find next part of name
    p = name;
    l = 0;
    while (l < len && *p != PS1 && *p != PS2)
    {
      l++;
      p++;
    }

    // Find directory for next name part
    
    // TODO
    
    // If we have parsed the whole name return the directory number
    if (l == len) return dir;

    // Prepare for next name part
    name = p;
    len -= l;
  }
}

int cdfs_mount(struct fs *fs, char *opts)
{
  struct cdfs *cdfs;
  dev_t devno;
  int cachebufs;
  int rc;
  struct buf *buf;
  struct iso_primary_descriptor *vd;

  // Check device
  devno = dev_open(fs->mntfrom);
  if (devno == NODEV) return -NODEV;
  if (device(devno)->driver->type != DEV_TYPE_BLOCK) return -ENOTBLK;

  // Revalidate device and check block size
  rc = dev_ioctl(devno, IOCTL_REVALIDATE, NULL, 0);
  if (rc < 0) return rc;
  if (dev_ioctl(devno, IOCTL_GETBLKSIZE, NULL, 0) != CDFS_BLOCKSIZE) return -ENXIO;

  // Allocate file system
  cdfs = (struct cdfs *) kmalloc(sizeof(struct cdfs));
  memset(cdfs, 0, sizeof(struct cdfs));
  cdfs->devno = devno;
  cdfs->blks = dev_ioctl(devno, IOCTL_GETDEVSIZE, NULL, 0);
  if (cdfs->blks < 0) return cdfs->blks;

  // Allocate cache
  cachebufs = get_num_option(opts, "cache", CDFS_DEFAULT_CACHESIZE);
  cdfs->cache = init_buffer_pool(devno, cachebufs, CDFS_BLOCKSIZE, NULL, cdfs);
  if (!cdfs->cache) return -ENOMEM;

  // Read volume descriptor
  buf  = get_buffer(cdfs->cache, 16);
  if (!buf) return -EIO;
  vd = (struct iso_primary_descriptor *) buf->data;
  if (memcmp(vd->id, ISO_STANDARD_ID, 5) != 0) 
  {
    free_buffer_pool(cdfs->cache);
    dev_close(cdfs->devno);
    kfree(cdfs);
    return -EIO;
  }

  //kprintf("volume_space_size=%d\n", isonum_733(vd->volume_space_size));
  //kprintf("path_table_size=%d\n", isonum_733(vd->path_table_size));
  //kprintf("type_l_path_table=%d\n", isonum_731(vd->type_l_path_table));
  //kprintf("type_m_path_table=%d\n", isonum_732(vd->type_m_path_table));

  release_buffer(cdfs->cache, buf);

  // Read path table
  rc = cdfs_read_path_table(cdfs, vd);
  if (rc < 0) return rc;

#if 1
  {
    int n;

    for (n = 1; n < cdfs->path_table_records; n++)
    {
      struct iso_pathtable_record *pathrec = cdfs->path_table[n];
      int namelen = isonum_711(pathrec->length);
      char name[MAXPATH];

      memcpy(name, pathrec->name, namelen);
      name[namelen] = 0;
      kprintf("path%d: [%s] parent %d extent %d\n", n, name, isonum_721(pathrec->parent), isonum_731(pathrec->extent));
    }
  }
#endif

  // Device mounted successfully
  fs->data = cdfs;
  return 0;
}

int cdfs_umount(struct fs *fs)
{
  struct cdfs *cdfs = (struct cdfs *) fs->data;

  // Free cache
  if (cdfs->cache) free_buffer_pool(cdfs->cache);

  // Close device
  dev_close(cdfs->devno);

  // Deallocate file system
  if (cdfs->path_table_buffer) kfree(cdfs->path_table_buffer);
  if (cdfs->path_table) kfree(cdfs->path_table);
  kfree(cdfs);

  return 0;
}

int cdfs_statfs(struct fs *fs, struct statfs *buf)
{
  struct cdfs *cdfs = (struct cdfs *) fs->data;

  buf->bsize = CDFS_BLOCKSIZE;
  buf->iosize = CDFS_BLOCKSIZE;
  buf->blocks = cdfs->blks;
  buf->bfree = 0;
  buf->files = -1;
  buf->ffree = 0;
  buf->cachesize = cdfs->cache->poolsize * CDFS_BLOCKSIZE;

  return 0;
}

int cdfs_open(struct file *filp, char *name)
{
  return -ENOSYS;
}

int cdfs_close(struct file *filp)
{
  return -ENOSYS;
}

int cdfs_flush(struct file *filp)
{
  return 0;
}

int cdfs_read(struct file *filp, void *data, size_t size)
{
  return -ENOSYS;
}

off64_t cdfs_tell(struct file *filp)
{
  return filp->pos;
}

off64_t cdfs_lseek(struct file *filp, off64_t offset, int origin)
{
  return -ENOSYS;
}

int cdfs_fstat(struct file *filp, struct stat64 *buffer)
{
  return -ENOSYS;
}

int cdfs_stat(struct fs *fs, char *name, struct stat64 *buffer)
{
  return -ENOSYS;
}

int cdfs_opendir(struct file *filp, char *name)
{
  return -ENOSYS;
}

int cdfs_readdir(struct file *filp, struct dirent *dirp, int count)
{
  return -ENOSYS;
}

struct fsops cdfsops =
{
  FSOP_OPEN | FSOP_CLOSE | FSOP_FLUSH | FSOP_READ | 
  FSOP_TELL | FSOP_LSEEK | FSOP_STAT | FSOP_FSTAT | 
  FSOP_OPENDIR | FSOP_READDIR,

  NULL,
  NULL,

  NULL,
  cdfs_mount,
  cdfs_umount,

  cdfs_statfs,

  cdfs_open,
  cdfs_close,
  cdfs_flush,

  cdfs_read,
  NULL,
  NULL,

  cdfs_tell,
  cdfs_lseek,
  NULL,

  NULL,
  NULL,

  cdfs_fstat,
  cdfs_stat,

  NULL,
  NULL,

  NULL,
  NULL,
  NULL,

  cdfs_opendir,
  cdfs_readdir
};

void init_cdfs()
{
  register_filesystem("cdfs", &cdfsops);
}
