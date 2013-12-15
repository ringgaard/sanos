//
// genvmdk.c
//
// Convert raw device to vmware virtual disk format (.vmdk)
//
// Copyright (C) 2013 Michael Ringgaard. All rights reserved.
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
#include <stdint.h>
#include <libgen.h>

#define SECTOR_SIZE 512
#define BUFFER_SIZE (64 * 1024)
#define L2_CACHE_SIZE 16
#define VMDK4_MAGIC (('V' << 24) | ('M' << 16) | ('D' << 8) | 'K')

#pragma pack(push, 1)
#pragma pack(1)

struct vmdkheader {
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

struct vmdkdev {
  handle_t hdev;
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
  int64_t total_sectors;
};

static int vmdk_open(struct vmdkdev *s, const char *filename) {
  handle_t hdev;
  uint32_t magic;
  int l1_size;
  struct vmdkheader header;

  // Open vmdk file
  hdev = open(filename, O_RDWR | O_BINARY);
  if (hdev == NOHANDLE) return -1;

  // Read header
  if (read(hdev, &magic, sizeof(magic)) != sizeof(magic)) goto fail;
  if (magic != VMDK4_MAGIC) goto fail;

  if (read(hdev, &header, sizeof(header)) != sizeof(header)) goto fail;
  s->total_sectors = (int) header.capacity;
  s->cluster_sectors = (unsigned int) header.granularity;
  s->l2_size = header.num_gtes_per_gte;
  s->l1_entry_sectors = s->l2_size * s->cluster_sectors;
  if (s->l1_entry_sectors <= 0) goto fail;
  s->l1_size = (uint32_t) ((s->total_sectors + s->l1_entry_sectors - 1) / s->l1_entry_sectors);
  s->l1_table_offset = header.rgd_offset << 9;
  s->l1_backup_table_offset = header.gd_offset << 9;

  // Read the L1 table
  l1_size = s->l1_size * sizeof(uint32_t);
  s->l1_table = malloc(l1_size);
  if (!s->l1_table) goto fail;

  if (lseek64(hdev, s->l1_table_offset, SEEK_SET) < 0) goto fail;
  if (read(hdev, s->l1_table, l1_size) < 0) goto fail;

  if (s->l1_backup_table_offset) {
    s->l1_backup_table = malloc(l1_size);
    if (!s->l1_backup_table) goto fail;

    if (lseek64(hdev, s->l1_backup_table_offset, SEEK_SET) < 0) goto fail;
    if (read(hdev, s->l1_backup_table, l1_size) < 0) goto fail;
  }

  // Initialize L2 cache
  s->l2_cache = malloc(s->l2_size * L2_CACHE_SIZE * sizeof(uint32_t));
  if (!s->l2_cache) goto fail;
  memset(s->l2_cache, 0, s->l2_size * L2_CACHE_SIZE * sizeof(uint32_t));

  s->hdev = hdev;
  return 0;

fail:
  free(s->l1_backup_table);
  free(s->l1_table);
  free(s->l2_cache);
  close(hdev);
  return -1;
}

static uint64_t get_cluster_offset(struct vmdkdev *s, uint64_t offset, int allocate) {
  unsigned int l1_index, l2_offset, l2_index;
  int min_index, i, j;
  uint32_t min_count, *l2_table, tmp;
  uint64_t cluster_offset, pos;

  l1_index = (unsigned int) (offset >> 9) / s->l1_entry_sectors;
  if (l1_index >= s->l1_size) return 0;
  l2_offset = s->l1_table[l1_index];
  if (!l2_offset) return 0;
  for (i = 0; i < L2_CACHE_SIZE; i++) {
    if (l2_offset == s->l2_cache_offsets[i]) {
      // Increment the hit count
      if (++s->l2_cache_counts[i] == 0xffffffff) {
	for (j = 0; j < L2_CACHE_SIZE; j++) {
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
  for (i = 0; i < L2_CACHE_SIZE; i++) {
    if (s->l2_cache_counts[i] < min_count) {
      min_count = s->l2_cache_counts[i];
      min_index = i;
    }
  }
  l2_table = s->l2_cache + (min_index * s->l2_size);
  pos = l2_offset * 512;
  lseek64(s->hdev, pos, SEEK_SET);
  if (read(s->hdev, l2_table, s->l2_size * sizeof(uint32_t)) != s->l2_size * sizeof(uint32_t)) return 0;
  s->l2_cache_offsets[min_index] = l2_offset;
  s->l2_cache_counts[min_index] = 1;

found:
  l2_index = (unsigned int) ((offset >> 9) / s->cluster_sectors) % s->l2_size;
  cluster_offset = l2_table[l2_index];
  if (!cluster_offset) {
    if (!allocate) return 0;
    cluster_offset = filelength64(s->hdev);
    pos = cluster_offset + (s->cluster_sectors << 9);
    ftruncate64(s->hdev, pos);
    cluster_offset >>= 9;

    // Update L2 table
    tmp = (uint32_t) cluster_offset;
    l2_table[l2_index] = tmp;
    pos = ((int64_t) l2_offset * 512) + (l2_index * sizeof(tmp));
    lseek64(s->hdev, pos, SEEK_SET);
    if (write(s->hdev, &tmp, sizeof(tmp)) != sizeof(tmp)) return 0;

    // Update backup L2 table
    if (s->l1_backup_table_offset != 0) {
      l2_offset = s->l1_backup_table[l1_index];
      pos = ((int64_t) l2_offset * 512) + (l2_index * sizeof(tmp));
      lseek64(s->hdev, pos, SEEK_SET);
      if (write(s->hdev, &tmp, sizeof(tmp)) != sizeof(tmp)) return 0;
    }
  }

  cluster_offset <<= 9;
  return cluster_offset;
}

static int vmdk_write(struct vmdkdev *s, int64_t sector_num,  const uint8_t *buf, int nb_sectors) {
  int index_in_cluster, n;
  uint64_t cluster_offset, pos;

  while (nb_sectors > 0) {
    index_in_cluster = (int) sector_num & (s->cluster_sectors - 1);
    n = s->cluster_sectors - index_in_cluster;
    if (n > nb_sectors) n = nb_sectors;
    cluster_offset = get_cluster_offset(s, sector_num << 9, 1);
    if (!cluster_offset) return -1;
    pos = cluster_offset + index_in_cluster * 512;
    lseek64(s->hdev, pos, SEEK_SET);
    if (write(s->hdev, buf, n * 512) != n * 512) return -1;
    nb_sectors -= n;
    sector_num += n;
    buf += n * 512;
  }

  return 0;
}

static int vmdk_create(const char *filename, int64_t total_size, int flags) {
  handle_t hdev;
  int64_t pos;
  uint32_t i;
  struct vmdkheader header;
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
  char fn[256];

  hdev = open(filename, O_CREAT | O_TRUNC | O_BINARY, 0644);
  if (hdev == NOHANDLE) return -1;
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
  if (write(hdev, &magic, sizeof(magic)) < 0) return -1;
  if (write(hdev, &header, sizeof(header)) < 0) return -1;

  pos = header.grain_offset << 9;
  if (ftruncate64(hdev, pos) < 0) return -1;

  // Write grain directory
  pos = header.rgd_offset << 9;
  if (lseek64(hdev, pos, SEEK_SET) < 0) return -1;
  for (i = 0, tmp = (uint32_t) header.rgd_offset + gd_size; i < gt_count; i++, tmp += gt_size) {
    if (write(hdev, &tmp, sizeof(tmp)) < 0) return -1;
  }
  
  // Write backup grain directory
  pos = header.gd_offset << 9;
  if (lseek64(hdev, pos, SEEK_SET) < 0) return -1;
  for (i = 0, tmp = (uint32_t) header.gd_offset + gd_size; i < gt_count; i++, tmp += gt_size) {
    if (write(hdev, &tmp, sizeof(tmp)) < 0) return -1;
  }

  // Compose the descriptor
  strcpy(fn, filename);
  sprintf(desc, desc_template, time(NULL), (unsigned long) total_size, basename(fn), total_size / (63 * 16));

  // Write the descriptor
  pos = header.desc_offset << 9;
  if (lseek64(hdev, pos, SEEK_SET) < 0) return -1;
  if (write(hdev, desc, strlen(desc)) < 0) return -1;

  close(hdev);
  return 0;
}

static void vmdk_close(struct vmdkdev *s) {
  free(s->l1_table);
  free(s->l2_cache);
  close(s->hdev);
}

static int is_zero(char *buffer, int len) {
  char *end = buffer + len;
  while (buffer < end) if (*buffer++) return 0;
  return 1;
}

int main(int argc, char *argv[]) {
  char *devname;
  char *imgname;
  handle_t hdev;
  struct vmdkdev vmdk;
  int64_t devsize;
  int bytes;
  char buffer[SECTOR_SIZE];
  int64_t total;
  int pct, prev_pct;

  // Get parameters
  if (argc != 3) {
    fprintf(stderr, "usage: genvmdk <device> <vmdk image file>\n");
    return 1;
  }

  devname = argv[1];
  imgname = argv[2];

  // Open source device file.
  hdev = open(devname, O_BINARY);
  if (hdev == NOHANDLE) {
    perror(devname);
    return 3;
  }

  devsize = filelength64(hdev);
  if (devsize < 0) {
    perror(devname);
    return 3;
  }

  if (vmdk_create(imgname, devsize / SECTOR_SIZE, 0) < 0) {
    printf("genvmdk: error creating vmdk file %s\n", imgname);
    return 3;
  }

  memset(&vmdk, 0, sizeof(vmdk));
  if (vmdk_open(&vmdk, imgname) < 0) {
    printf("genvmdk: error opening vmdk file %s\n", imgname);
    return 3;
  }

  total = 0;
  prev_pct = -1;
  while (total < devsize) {
    if ((bytes = read(hdev, buffer, SECTOR_SIZE)) < 0) {
      perror(devname);
      return 3;
    }

    if (bytes == 0) break;
    
    if (!is_zero(buffer, bytes)) {
      if (vmdk_write(&vmdk, total / SECTOR_SIZE, buffer, bytes / SECTOR_SIZE) < 0) {
        printf("genvmdk: error writing to vmdk file\n");
        return 3;
      }
    }
    
    total += bytes;

    pct = (int) (total * 100 / devsize);
    if (pct != prev_pct) {
      printf("%3d%% complete\r", pct);
      prev_pct = pct;
    }
  }

  close(hdev);
  vmdk_close(&vmdk);
  printf("image successfully written\n");
}
