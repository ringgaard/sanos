//
// super.c
//
// Disk filesystem superblock routines
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

#define DEFAULT_BLOCKSIZE       4096
#define DEFAULT_INODE_RATIO     4096
#define DEFAULT_CACHE_BUFFERS   1024
#define DEFAULT_RESERVED_BLOCKS 16
#define DEFAULT_RESERVED_INODES 16

#define FORMAT_BLOCKSIZE        (64 * 1024)

static void mark_group_desc_dirty(struct filsys *fs, int group) {
  mark_buffer_updated(fs->cache, fs->groupdesc_buffers[group / fs->groupdescs_per_block]);
}

static int log2(int n) {
  int l = 0;
  n >>= 1;
  while (n) {
    l++;
    n >>= 1;
  }

  return l;
}

static void dfs_sync(void *arg) {
  struct filsys *fs = (struct filsys *) arg;

  // Write super block
  if (fs->super_dirty) {
    dev_write(fs->devno, fs->super, SECTORSIZE, 1, 0);
    fs->super_dirty = 0;
  }
}

static int parse_options(char *opts, struct fsoptions *fsopts) {
  fsopts->cache = get_num_option(opts, "cache", 0);
  fsopts->blocksize = get_num_option(opts, "blocksize", DEFAULT_BLOCKSIZE);
  fsopts->inode_ratio = get_num_option(opts, "inoderatio", DEFAULT_INODE_RATIO);
  fsopts->reserved_blocks = get_num_option(opts, "resvblks", DEFAULT_RESERVED_BLOCKS);
  fsopts->reserved_inodes = get_num_option(opts, "resvinodes", DEFAULT_RESERVED_INODES);

  fsopts->flags = 0;
  if (get_option(opts, "quick", NULL, 0, NULL)) fsopts->flags |= FSOPT_QUICK;
  if (get_option(opts, "progress", NULL, 0, NULL)) fsopts->flags |= FSOPT_PROGRESS;
  if (get_option(opts, "format", NULL, 0, NULL)) fsopts->flags |= FSOPT_FORMAT;

  return 0;
}

static struct filsys *create_filesystem(char *devname, struct fsoptions *fsopts) {
  struct filsys *fs;
  dev_t devno;
  unsigned int sectcount;
  unsigned int blocks;
  unsigned int first_block;
  struct groupdesc *gd;
  struct buf *buf;
  unsigned int i, j;
  ino_t ino;
  struct inode *root;
  char *buffer;

  // Check device
  devno = dev_open(devname);
  if (devno == NODEV) return NULL;
  if (device(devno)->driver->type != DEV_TYPE_BLOCK) return NULL;
  sectcount = dev_ioctl(devno, IOCTL_GETDEVSIZE, NULL, 0);
  if (sectcount < 0) return NULL;

  // Allocate file system
  fs = (struct filsys *) kmalloc(sizeof(struct filsys));
  memset(fs, 0, sizeof(struct filsys));

  // Allocate super block
  fs->super = (struct superblock *) kmalloc(SECTORSIZE);
  memset(fs->super, 0, SECTORSIZE);
  fs->super_dirty = 1;

  // Set device number and block size
  fs->devno = devno;
  fs->blocksize = fsopts->blocksize;

  // Set signature, version and block size in super block
  fs->super->signature = DFS_SIGNATURE;
  fs->super->version = DFS_VERSION;
  fs->super->log_block_size = log2(fsopts->blocksize);

  // Each group has as many blocks as can be represented by the block bitmap block
  fs->super->blocks_per_group = fs->blocksize * 8;

  // Get the device size in sectors from the device and convert it to blocks
  fs->super->block_count =  sectcount / (fs->blocksize / SECTORSIZE);

  // Set cache size
  if (fsopts->cache == 0) {
    fs->super->cache_buffers = DEFAULT_CACHE_BUFFERS;
  } else {
    fs->super->cache_buffers = fsopts->cache;
  }
  if (fs->super->cache_buffers > fs->super->block_count) fs->super->cache_buffers = fs->super->block_count;

  // The number of inodes in a group is computed as a ratio of the size of the group.
  // If the device has only one group the inode count is based on size of device.
  // The number of inodes per block is then rounded up to fit a whole number of blocks.
  fs->inodes_per_block = fs->blocksize / sizeof(struct inodedesc);
  if (fs->super->blocks_per_group < fs->super->block_count) {
    fs->super->inodes_per_group = fs->blocksize * fs->super->blocks_per_group / fsopts->inode_ratio;
  } else {
    fs->super->inodes_per_group = fs->blocksize * fs->super->block_count / fsopts->inode_ratio;
  }
  if (fs->super->inodes_per_group > fs->blocksize * 8) fs->super->inodes_per_group = fs->blocksize * 8;
  fs->super->inodes_per_group = (fs->super->inodes_per_group + fs->inodes_per_block - 1) / fs->inodes_per_block * fs->inodes_per_block;
  fs->inode_blocks_per_group = (fs->super->inodes_per_group * sizeof(struct inodedesc) + fs->blocksize - 1) / fs->blocksize;

  // Calculate the number of block pointers per block directory page
  fs->log_blkptrs_per_block = fs->super->log_block_size - 2;

  // Calculate the number of group descriptors and the number of blocks to store them
  fs->super->group_count = (fs->super->block_count + fs->super->blocks_per_group - 1) / fs->super->blocks_per_group;
  fs->groupdescs_per_block = fs->blocksize / sizeof(struct groupdesc);
  fs->groupdesc_blocks = (fs->super->group_count * sizeof(struct groupdesc) + fs->blocksize - 1) / fs->blocksize;

  // The reserved blocks are allocated right after the super block
  fs->super->first_reserved_block = 1;
  if (fs->blocksize <= SECTORSIZE) fs->super->first_reserved_block++;
  fs->super->reserved_blocks = fsopts->reserved_blocks;
  
  // The group descriptor table starts after the superblock and reserved blocks
  fs->super->groupdesc_table_block = fs->super->first_reserved_block + fs->super->reserved_blocks;

  // If the last group is too small to hold the bitmaps and inode table skip it
  blocks = fs->super->block_count % fs->super->blocks_per_group;
  if (blocks > 0 && blocks < fs->inode_blocks_per_group + 2) fs->super->group_count--;
  if (fs->super->group_count == 0) {
    kprintf(KERN_ERR "dfs: filesystem too small\n");
    return NULL;
  }

  // Initialize buffer cache
  fs->cache = init_buffer_pool(devno, fs->super->cache_buffers, fs->blocksize, dfs_sync, fs);
  if (!fs->cache) return NULL;
  fs->cache->nosync = 1;

  // Zero all blocks on disk
  if ((fsopts->flags & FSOPT_QUICK) == 0) {
    int percent;
    int prev_percent;
    int blocks_per_io;

    blocks_per_io = FORMAT_BLOCKSIZE / fs->blocksize;
    buffer = (char *) kmalloc(FORMAT_BLOCKSIZE);
    memset(buffer, 0, FORMAT_BLOCKSIZE);

    prev_percent = -1;
    for (i = fs->super->groupdesc_table_block + fs->groupdesc_blocks; i < fs->super->block_count; i += blocks_per_io) {
      int rc;

      if (fsopts->flags & FSOPT_PROGRESS) {
        percent = (i / 100) * 100 / (fs->super->block_count / 100);
        if (percent != prev_percent) kprintf("%d%% complete\r", percent);
        prev_percent = percent;
      }

      if (i + blocks_per_io > fs->super->block_count) {
        rc = dev_write(fs->devno, buffer, (fs->super->block_count - i) * fs->blocksize, i, 0);
      } else {
        rc = dev_write(fs->devno, buffer, FORMAT_BLOCKSIZE, i, 0);
      }

      if (rc < 0) {
        kprintf("dfs: error %d in format\n", rc);
        return NULL;
      }
    }
    if (fsopts->flags & FSOPT_PROGRESS) kprintf("100%% complete\r");

    kfree(buffer);
  }

  // Allocate group descriptors
  fs->groupdesc_buffers = (struct buf **) kmalloc(sizeof(struct buf *) * fs->groupdesc_blocks);
  fs->groups = (struct blkgroup *) kmalloc(sizeof(struct group) * fs->super->group_count);

  for (i = 0; i < fs->groupdesc_blocks; i++) {
    fs->groupdesc_buffers[i] = alloc_buffer(fs->cache, fs->super->groupdesc_table_block + i);
    if (!fs->groupdesc_buffers[i]) return NULL;
  }

  for (i = 0; i < fs->super->group_count; i++) {
    gd = (struct groupdesc *) fs->groupdesc_buffers[i / fs->groupdescs_per_block]->data;
    gd += (i % fs->groupdescs_per_block);

    fs->groups[i].desc = gd;
    fs->groups[i].first_free_block = 0;
    fs->groups[i].first_free_inode = 0;
  }

  // Reserve inode for root directory
  fs->super->reserved_inodes = fsopts->reserved_inodes;

  // Set inode count based on group count
  fs->super->inode_count = fs->super->inodes_per_group * fs->super->group_count;

  // All blocks and inodes initially free
  fs->super->free_inode_count = fs->super->inode_count;
  fs->super->free_block_count = fs->super->block_count;

  // Initialize block bitmaps
  for (i = 0; i < fs->super->group_count; i++) {
    gd = fs->groups[i].desc;
    blocks = 0;
    first_block = fs->super->blocks_per_group * i;

    // The first group needs blocks for the super block, reserved blocks and the group descriptors
    if (i == 0) blocks = fs->super->groupdesc_table_block + fs->groupdesc_blocks;

    // Next blocks in group are the block bitmap, inode bitmap and the inode table
    gd->block_bitmap_block = first_block + blocks++;
    gd->inode_bitmap_block = first_block + blocks++;
    gd->inode_table_block = first_block + blocks;
    blocks += fs->inode_blocks_per_group;

    // Update block bitmap
    buf = alloc_buffer(fs->cache, gd->block_bitmap_block);
    if (!buf) return NULL;
    set_bits(buf->data, 0, blocks);
    mark_buffer_updated(fs->cache, buf);
    release_buffer(fs->cache, buf);

    // Determine the block count for the group. The last group may be truncated
    if (fs->super->blocks_per_group * (i + 1) > fs->super->block_count) {
      gd->block_count = fs->super->block_count - fs->super->blocks_per_group * i;
    } else {
      gd->block_count = fs->super->blocks_per_group;
    }

    // Set the count of free blocks and inodes for group
    gd->free_inode_count = fs->super->inodes_per_group;
    gd->free_block_count = gd->block_count - blocks;

    // Update super block
    fs->super->free_block_count -= blocks;

    mark_group_desc_dirty(fs, i);
  }

  // Zero out block and inode bitmaps and inode tables
  if (fsopts->flags & FSOPT_QUICK) {
    buffer = (char *) kmalloc(fs->blocksize);
    memset(buffer, 0, fs->blocksize);

    for (i = 0; i < fs->super->group_count; i++) {
      gd = fs->groups[i].desc;

      dev_write(fs->devno, buffer, fs->blocksize, gd->block_bitmap_block, 0);
      dev_write(fs->devno, buffer, fs->blocksize, gd->inode_bitmap_block, 0);
      for (j = 0; j < fs->inode_blocks_per_group; j++) {
        dev_write(fs->devno, buffer, fs->blocksize, gd->inode_table_block + j, 0);
      }
    }

    kfree(buffer);
  }

  // Reserve inodes
  for (i = 0; i < fs->super->reserved_inodes; i++) {
    ino = new_inode(fs, 0, 0);
    if (ino != i)  {
      kprintf(KERN_ERR "dfs: format expected inode %d, got %d\n", i, ino);
      return NULL;
    }
  }

  // Create root directory
  if (get_inode(fs, DFS_INODE_ROOT, &root) < 0) return NULL;
  root->desc->mode = S_IFDIR | S_IRWXU | S_IRWXG | S_IRWXO;
  root->desc->ctime = root->desc->mtime = time(NULL);
  root->desc->linkcount = 1;
  mark_buffer_updated(fs->cache, root->buf);
  release_inode(root);

  // Reenable buffer cache sync
  fs->cache->nosync = 0;

  return fs;
}

static struct filsys *open_filesystem(char *devname, struct fsoptions *fsopts) {
  struct filsys *fs;
  dev_t devno;
  struct groupdesc *gd;
  unsigned int i;
  unsigned int cache_buffers;

  // Check device
  devno = dev_open(devname);
  if (devno == NODEV) return NULL;
  if (device(devno)->driver->type != DEV_TYPE_BLOCK) return NULL;

  // Allocate file system
  fs = (struct filsys *) kmalloc(sizeof(struct filsys));
  memset(fs, 0, sizeof(struct filsys));

  // Allocate and read super block
  fs->super = (struct superblock *) kmalloc(SECTORSIZE);
  memset(fs->super, 0, SECTORSIZE);
  if (dev_read(devno, fs->super, SECTORSIZE, 1, 0) != SECTORSIZE) {
    kprintf(KERN_ERR "dfs: unable to read superblock on device %s\n", device(devno)->name);
    free(fs->super);
    free(fs);
    return NULL;
  }
  fs->super_dirty = 0;

  // Check signature and version
  if (fs->super->signature != DFS_SIGNATURE) {
    kprintf(KERN_ERR "dfs: invalid DFS signature on device %s\n", device(devno)->name);
    free(fs->super);
    free(fs);
    return NULL;
  }

  if (fs->super->version != DFS_VERSION) {
    kprintf(KERN_ERR "dfs: invalid DFS version on device %s\n", device(devno)->name);
    free(fs->super);
    free(fs);
    return NULL;
  }

  // Set device number and block size
  fs->devno = devno;
  fs->blocksize = 1 << fs->super->log_block_size;
  fs->inodes_per_block = fs->blocksize / sizeof(struct inodedesc);

  // Initialize buffer cache
  cache_buffers = (unsigned int) fsopts->cache;
  if (cache_buffers == 0) cache_buffers = fs->super->cache_buffers;
  if (cache_buffers == 0) cache_buffers = DEFAULT_CACHE_BUFFERS;
  if (cache_buffers > fs->super->block_count) cache_buffers = fs->super->block_count;
  fs->cache = init_buffer_pool(devno, cache_buffers, fs->blocksize, dfs_sync, fs);
  if (!fs->cache) return NULL;

  // Calculate the number of group descriptors blocks
  fs->groupdescs_per_block = fs->blocksize / sizeof(struct groupdesc);
  fs->groupdesc_blocks = (fs->super->group_count * sizeof(struct groupdesc) + fs->blocksize - 1) / fs->blocksize;

  // Calculate the number of block pointers per block directory page
  fs->log_blkptrs_per_block = fs->super->log_block_size - 2;

  // Read group descriptors
  fs->groupdesc_buffers = (struct buf **) kmalloc(sizeof(struct buf *) * fs->groupdesc_blocks);
  fs->groups = (struct blkgroup *) kmalloc(sizeof(struct group) * fs->super->group_count);
  for (i = 0; i < fs->groupdesc_blocks; i++) {
    fs->groupdesc_buffers[i] = get_buffer(fs->cache, fs->super->groupdesc_table_block + i);
    if (!fs->groupdesc_buffers[i]) return NULL;
  }

  for (i = 0; i < fs->super->group_count; i++) {
    gd = (struct groupdesc *) fs->groupdesc_buffers[i / fs->groupdescs_per_block]->data;
    gd += (i % fs->groupdescs_per_block);

    fs->groups[i].desc = gd;
    fs->groups[i].first_free_block = -1;
    fs->groups[i].first_free_inode = -1;
  }

  return fs;
}

static void close_filesystem(struct filsys *fs) {
  unsigned int i;

  // Release all group descriptors
  for (i = 0; i < fs->groupdesc_blocks; i++) release_buffer(fs->cache, fs->groupdesc_buffers[i]);
  kfree(fs->groupdesc_buffers);
  kfree(fs->groups);

  // Flush and sync buffer cache
  flush_buffers(fs->cache, 0);
  sync_buffers(fs->cache, 0);

  // Free cache
  free_buffer_pool(fs->cache);

  // Write super block
  if (fs->super_dirty) dev_write(fs->devno, fs->super, SECTORSIZE, 1, 0);
  kfree(fs->super);

  // Close device
  dev_close(fs->devno);

  // Deallocate file system
  kfree(fs);
}

static void get_filesystem_status(struct filsys *fs, struct statfs *buf) {
  buf->bsize = fs->blocksize;
  buf->iosize = fs->blocksize;
  buf->blocks = fs->super->block_count;
  buf->bfree = fs->super->free_block_count;
  buf->files = fs->super->inode_count;
  buf->ffree = fs->super->free_inode_count;
  buf->cachesize = fs->cache->poolsize * fs->cache->bufsize;
}

int dfs_mkfs(char *devname, char *opts) {
  struct fsoptions fsopts;
  struct filsys *fs;

  if (parse_options(opts, &fsopts) != 0) return -EINVAL;
  fs = create_filesystem(devname, &fsopts);
  if (!fs) return -EIO;
  close_filesystem(fs);
  return 0;
}

int dfs_mount(struct fs *fs, char *opts) {
  struct fsoptions fsopts;

  if (parse_options(opts, &fsopts) != 0) return -EINVAL;
  if (fsopts.flags & FSOPT_FORMAT) {
    fs->data = create_filesystem(fs->mntfrom, &fsopts);
  } else {
    fs->data = open_filesystem(fs->mntfrom, &fsopts);
  }
  if (!fs->data) return -EIO;

  return 0;
}

int dfs_umount(struct fs *fs) {
  close_filesystem((struct filsys *) fs->data);
  return 0;
}

int dfs_statfs(struct fs *fs, struct statfs *buf) {
  get_filesystem_status((struct filsys *) fs->data, buf);
  return 0;
}
