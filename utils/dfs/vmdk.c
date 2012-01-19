//
// Block driver for the VMDK format
// 
// Copyright (c) 2004 Fabrice Bellard
// Copyright (c) 2005 Filip Navara
// Copyright (c) 2006 Michael Ringgaard
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#include <windows.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "blockdev.h"

#define VMDK3_MAGIC (('D' << 24) | ('W' << 16) | ('O' << 8) | 'C')
#define VMDK4_MAGIC (('V' << 24) | ('M' << 16) | ('D' << 8) | 'K')

#pragma pack(push)
#pragma pack(1)

struct vmdk3header
{
  uint32_t version;
  uint32_t flags;
  uint32_t disk_sectors;
  uint32_t granularity;
  uint32_t l1dir_offset;
  uint32_t l1dir_size;
  uint32_t file_sectors;
  uint32_t cylinders;
  uint32_t heads;
  uint32_t sectors_per_track;
};

struct vmdk4header
{
  uint32_t version;
  uint32_t flags;
  int64_t capacity;
  int64_t granularity;
  int64_t desc_offset;
  int64_t desc_size;
  int32_t num_gtes_per_gte;
  int64_t rgd_offset;
  int64_t gd_offset;
  int64_t grain_offset;
  char filler[1];
  char check_bytes[4];
};

#pragma pack(pop)

#define L2_CACHE_SIZE 16

struct vmdkdev
{
  HANDLE hdev;
  int64_t l1_table_offset;
  int64_t l1_backup_table_offset;
  uint32_t *l1_table;
  uint32_t *l1_backup_table;
  unsigned int l1_size;
  uint32_t l1_entry_sectors;

  unsigned int l2_size;
  uint32_t *l2_cache;
  uint32_t l2_cache_offsets[L2_CACHE_SIZE];
  uint32_t l2_cache_counts[L2_CACHE_SIZE];

  unsigned int cluster_sectors;
};

static int vmdk_probe(const uint8_t *buf, int buf_size, const char *filename)
{
  uint32_t magic;

  if (buf_size < 4) return 0;
  magic = *(uint32_t *)buf;
  if (magic == VMDK3_MAGIC || magic == VMDK4_MAGIC)
    return 100;
  else
    return 0;
}

static int vmdk_open(struct blockdevice *bs, const char *filename)
{
  struct vmdkdev *s = bs->opaque;
  HANDLE hdev;
  DWORD bytes;
  uint32_t magic;
  int l1_size;


  hdev = CreateFile(filename, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
  if (hdev == INVALID_HANDLE_VALUE) return -1;

  if (!ReadFile(hdev, &magic, sizeof(magic), &bytes, NULL) || bytes != sizeof(magic)) goto fail;

  if (magic == VMDK3_MAGIC) 
  {
    struct vmdk3header header;

    if (!ReadFile(hdev, &header, sizeof(header), &bytes, NULL) || bytes != sizeof(header)) goto fail;
    s->cluster_sectors = header.granularity;
    s->l2_size = 1 << 9;
    s->l1_size = 1 << 6;
    bs->total_sectors = header.disk_sectors;
    s->l1_table_offset = header.l1dir_offset << 9;
    s->l1_backup_table_offset = 0;
    s->l1_entry_sectors = s->l2_size * s->cluster_sectors;
  } 
  else if (magic == VMDK4_MAGIC) 
  {
    struct vmdk4header header;
    
    if (!ReadFile(hdev, &header, sizeof(header), &bytes, NULL) || bytes != sizeof(header)) goto fail;
    bs->total_sectors = (int) header.capacity;
    s->cluster_sectors = (unsigned int) header.granularity;
    s->l2_size = header.num_gtes_per_gte;
    s->l1_entry_sectors = s->l2_size * s->cluster_sectors;
    if (s->l1_entry_sectors <= 0) goto fail;
    s->l1_size = (uint32_t) ((bs->total_sectors + s->l1_entry_sectors - 1) / s->l1_entry_sectors);
    s->l1_table_offset = header.rgd_offset << 9;
    s->l1_backup_table_offset = header.gd_offset << 9;
  } 
  else 
      goto fail;

  // Read the L1 table
  l1_size = s->l1_size * sizeof(uint32_t);
  s->l1_table = malloc(l1_size);
  if (!s->l1_table) goto fail;

  if (SetFilePointerEx(hdev, *(LARGE_INTEGER*) &s->l1_table_offset, NULL, FILE_BEGIN) == -1) goto fail;
  if (!ReadFile(hdev, s->l1_table, l1_size, &bytes, NULL)) goto fail;

  if (s->l1_backup_table_offset) 
  {
    s->l1_backup_table = malloc(l1_size);
    if (!s->l1_backup_table) goto fail;

    if (SetFilePointerEx(hdev, *(LARGE_INTEGER*) &s->l1_backup_table_offset, NULL, FILE_BEGIN) == -1) goto fail;
    if (!ReadFile(hdev, s->l1_backup_table, l1_size, &bytes, NULL)) goto fail;
  }

  s->l2_cache = malloc(s->l2_size * L2_CACHE_SIZE * sizeof(uint32_t));
  if (!s->l2_cache) goto fail;
  memset(s->l2_cache, 0, s->l2_size * L2_CACHE_SIZE * sizeof(uint32_t));
  s->hdev = hdev;
  return 0;

fail:
  if (s->l1_backup_table) free(s->l1_backup_table);
  if (s->l1_table) free(s->l1_table);
  if (s->l2_cache) free(s->l2_cache);
  CloseHandle(hdev);
  return -1;
}

static uint64_t get_cluster_offset(struct blockdevice *bs, uint64_t offset, int allocate)
{
  struct vmdkdev *s = bs->opaque;
  unsigned int l1_index, l2_offset, l2_index;
  int min_index, i, j;
  uint32_t min_count, *l2_table, tmp;
  uint64_t cluster_offset, pos;
  DWORD bytes;
  
  l1_index = (unsigned int) (offset >> 9) / s->l1_entry_sectors;
  if (l1_index >= s->l1_size) return 0;
  l2_offset = s->l1_table[l1_index];
  if (!l2_offset) return 0;
  for (i = 0; i < L2_CACHE_SIZE; i++) 
  {
    if (l2_offset == s->l2_cache_offsets[i]) 
    {
      // Increment the hit count
      if (++s->l2_cache_counts[i] == 0xffffffff) 
      {
        for (j = 0; j < L2_CACHE_SIZE; j++) 
        {
          s->l2_cache_counts[j] >>= 1;
        }
      }
      l2_table = s->l2_cache + (i * s->l2_size);
      goto found;
    }
  }

  // Not found: load a new entry in the least used one
  min_index = 0;
  min_count = 0xffffffff;
  for (i = 0; i < L2_CACHE_SIZE; i++) 
  {
    if (s->l2_cache_counts[i] < min_count) 
    {
      min_count = s->l2_cache_counts[i];
      min_index = i;
    }
  }
  l2_table = s->l2_cache + (min_index * s->l2_size);
  pos = l2_offset * 512;
  SetFilePointerEx(s->hdev, *(LARGE_INTEGER*) &pos, NULL, FILE_BEGIN);
  if (!ReadFile(s->hdev, l2_table, s->l2_size * sizeof(uint32_t), &bytes, NULL) || bytes != s->l2_size * sizeof(uint32_t)) return 0;
  s->l2_cache_offsets[min_index] = l2_offset;
  s->l2_cache_counts[min_index] = 1;

found:
  l2_index = (unsigned int) ((offset >> 9) / s->cluster_sectors) % s->l2_size;
  cluster_offset = l2_table[l2_index];
  if (!cluster_offset) 
  {
    if (!allocate) return 0;
    GetFileSizeEx(s->hdev, (LARGE_INTEGER *) &cluster_offset);
    pos = cluster_offset + (s->cluster_sectors << 9);
    SetFilePointerEx(s->hdev, *(LARGE_INTEGER*) &pos, NULL, FILE_BEGIN);
    SetEndOfFile(s->hdev);
    cluster_offset >>= 9;

    // Update L2 table
    tmp = (uint32_t) cluster_offset;
    l2_table[l2_index] = tmp;
    pos = ((int64_t)l2_offset * 512) + (l2_index * sizeof(tmp));
    SetFilePointerEx(s->hdev, *(LARGE_INTEGER*) &pos, NULL, FILE_BEGIN);
    if (!WriteFile(s->hdev, &tmp, sizeof(tmp), &bytes, NULL) || bytes != sizeof(tmp)) return 0;

    // Update backup L2 table
    if (s->l1_backup_table_offset != 0) 
    {
      l2_offset = s->l1_backup_table[l1_index];
      pos = ((int64_t)l2_offset * 512) + (l2_index * sizeof(tmp));
      SetFilePointerEx(s->hdev, *(LARGE_INTEGER*) &pos, NULL, FILE_BEGIN);
      if (!WriteFile(s->hdev, &tmp, sizeof(tmp), &bytes, NULL) || bytes != sizeof(tmp)) return 0;
    }
  }

  cluster_offset <<= 9;
  return cluster_offset;
}

static int vmdk_is_allocated(struct blockdevice *bs, int64_t sector_num, int nb_sectors, int *pnum)
{
  struct vmdkdev *s = bs->opaque;
  int index_in_cluster, n;
  uint64_t cluster_offset;

  cluster_offset = get_cluster_offset(bs, sector_num << 9, 0);
  index_in_cluster = (int) sector_num % s->cluster_sectors;
  n = s->cluster_sectors - index_in_cluster;
  if (n > nb_sectors) n = nb_sectors;

  *pnum = n;
  return cluster_offset != 0;
}

static int vmdk_read(struct blockdevice *bs, int64_t sector_num, uint8_t *buf, int nb_sectors)
{
  struct vmdkdev *s = bs->opaque;
  int index_in_cluster, n;
  uint64_t cluster_offset, pos;
  DWORD bytes;
  
  while (nb_sectors > 0) 
  {
    cluster_offset = get_cluster_offset(bs, sector_num << 9, 0);
    index_in_cluster = (int) sector_num % s->cluster_sectors;
    n = s->cluster_sectors - index_in_cluster;
    if (n > nb_sectors) n = nb_sectors;
    if (!cluster_offset)
    {
      memset(buf, 0, 512 * n);
    } 
    else 
    {
      pos = cluster_offset + index_in_cluster * 512;
      SetFilePointerEx(s->hdev, *(LARGE_INTEGER*) &pos, NULL, FILE_BEGIN);
      if (!ReadFile(s->hdev, buf, n * 512, &bytes, NULL) || bytes != n * 512) return -1;
    }
    nb_sectors -= n;
    sector_num += n;
    buf += n * 512;
  }

  return 0;
}

static int vmdk_write(struct blockdevice *bs, int64_t sector_num,  const uint8_t *buf, int nb_sectors)
{
  struct vmdkdev *s = bs->opaque;
  int index_in_cluster, n;
  uint64_t cluster_offset, pos;
  DWORD bytes;

  while (nb_sectors > 0) 
  {
    index_in_cluster = (int) sector_num & (s->cluster_sectors - 1);
    n = s->cluster_sectors - index_in_cluster;
    if (n > nb_sectors) n = nb_sectors;
    cluster_offset = get_cluster_offset(bs, sector_num << 9, 1);
    if (!cluster_offset) return -1;
    pos = cluster_offset + index_in_cluster * 512;
    SetFilePointerEx(s->hdev, *(LARGE_INTEGER*) &pos, NULL, FILE_BEGIN);
    if (!WriteFile(s->hdev, buf, n * 512, &bytes, NULL) || bytes != n * 512) return -1;
    nb_sectors -= n;
    sector_num += n;
    buf += n * 512;
  }

  return 0;
}

static int vmdk_create(const char *filename, int64_t total_size, int flags)
{
  HANDLE hdev;
  DWORD bytes;
  int64_t pos;
  uint32_t i;
  struct vmdk4header header;
  uint32_t tmp, magic, grains, gd_size, gt_size, gt_count;
  char *desc_template =
      "# Disk DescriptorFile\n"
      "version=1\n"
      "CID=%x\n"
      "parentCID=ffffffff\n"
      "createType=\"monolithicSparse\"\n"
      "\n"
      "# Extent description\n"
      "RW %lu SPARSE \"%s\"\n"
      "\n"
      "# The Disk Data Base \n"
      "#DDB\n"
      "\n"
      "ddb.virtualHWVersion = \"3\"\n"
      "ddb.geometry.cylinders = \"%lu\"\n"
      "ddb.geometry.heads = \"16\"\n"
      "ddb.geometry.sectors = \"63\"\n"
      "ddb.adapterType = \"ide\"\n";
  char desc[1024];
  const char *real_filename, *temp_str;

  hdev = CreateFile(filename, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
  if (hdev == INVALID_HANDLE_VALUE) return -1;
  magic = VMDK4_MAGIC;
  memset(&header, 0, sizeof(header));
  header.version = 1;
  header.flags = 3; // ??
  header.capacity = total_size;
  header.granularity = 128;
  header.num_gtes_per_gte = 512;

  grains = (uint32_t) ((total_size + header.granularity - 1) / header.granularity);
  gt_size = ((header.num_gtes_per_gte * sizeof(uint32_t)) + 511) >> 9;
  gt_count = (grains + header.num_gtes_per_gte - 1) / header.num_gtes_per_gte;
  gd_size = (gt_count * sizeof(uint32_t) + 511) >> 9;

  header.desc_offset = 1;
  header.desc_size = 20;
  header.rgd_offset = header.desc_offset + header.desc_size;
  header.gd_offset = header.rgd_offset + gd_size + (gt_size * gt_count);
  header.grain_offset =
    ((header.gd_offset + gd_size + (gt_size * gt_count) +
    header.granularity - 1) / header.granularity) *
    header.granularity;

  header.desc_offset = header.desc_offset;
  header.desc_size = header.desc_size;
  header.rgd_offset = header.rgd_offset;
  header.gd_offset = header.gd_offset;
  header.grain_offset = header.grain_offset;

  header.check_bytes[0] = 0xa;
  header.check_bytes[1] = 0x20;
  header.check_bytes[2] = 0xd;
  header.check_bytes[3] = 0xa;
  
  // Write all the data
  WriteFile(hdev, &magic, sizeof(magic), &bytes, NULL);
  WriteFile(hdev, &header, sizeof(header), &bytes, NULL);

  pos = header.grain_offset << 9;
  SetFilePointerEx(hdev, *(LARGE_INTEGER*) &pos, NULL, FILE_BEGIN);
  SetEndOfFile(hdev);

  // Write grain directory
  pos = header.rgd_offset << 9;
  SetFilePointerEx(hdev, *(LARGE_INTEGER*) &pos, NULL, FILE_BEGIN);
  for (i = 0, tmp = (uint32_t) header.rgd_offset + gd_size; i < gt_count; i++, tmp += gt_size)
  {
    WriteFile(hdev, &tmp, sizeof(tmp), &bytes, NULL);
  }
  
  // Write backup grain directory
  pos = header.gd_offset << 9;
  SetFilePointerEx(hdev, *(LARGE_INTEGER*) &pos, NULL, FILE_BEGIN);
  for (i = 0, tmp = (uint32_t) header.gd_offset + gd_size; i < gt_count; i++, tmp += gt_size)
  {
    WriteFile(hdev, &tmp, sizeof(tmp), &bytes, NULL);
  }

  // Compose the descriptor
  real_filename = filename;
  if ((temp_str = strrchr(real_filename, '\\')) != NULL) real_filename = temp_str + 1;
  if ((temp_str = strrchr(real_filename, '/')) != NULL) real_filename = temp_str + 1;
  if ((temp_str = strrchr(real_filename, ':')) != NULL) real_filename = temp_str + 1;
  sprintf(desc, desc_template, time(NULL), (unsigned long) total_size, real_filename, total_size / (63 * 16));

  // Write the descriptor
  pos = header.desc_offset << 9;
  SetFilePointerEx(hdev, *(LARGE_INTEGER*) &pos, NULL, FILE_BEGIN);
  WriteFile(hdev, desc, strlen(desc), &bytes, NULL);

  CloseHandle(hdev);
  return 0;
}

static void vmdk_close(struct blockdevice *bs)
{
  struct vmdkdev *s = bs->opaque;
  free(s->l1_table);
  free(s->l2_cache);
  CloseHandle(s->hdev);
}

struct blockdriver bdrv_vmdk =
{
  "vmdk",
  sizeof(struct vmdkdev),
  vmdk_probe,
  vmdk_open,
  vmdk_read,
  vmdk_write,
  vmdk_close,
  vmdk_create,
  vmdk_is_allocated,
};
