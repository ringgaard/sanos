#include "types.h"
#include <vfs.h>

#include "bitops.h"
#include "buf.h"
#include "dfs.h"

#define SECTORSIZE   512

#define DEFAULT_BLOCKSIZE   4096
#define DEFAULT_INODE_RATIO 4096

#define CACHEBUFFERS     1024
#define FIRSTBLOCK       1
#define RESERVED_INODES  16

void panic(char *reason);
int dev_read(devno_t devno, void *buffer, size_t count, blkno_t blkno);
int dev_write(devno_t devno, void *buffer, size_t count, blkno_t blkno);
unsigned int dev_getsize(devno_t devno);

static void mark_group_desc_dirty(struct filsys *fs, int group)
{
  mark_buffer_updated(fs->groupdesc_buffers[group / fs->groupdescs_per_block]);
}

static int log2(int n)
{
  int l = 0;
  n >>= 1;
  while (n) 
  {
    l++;
    n >>= 1;
  }

  return l;
}

static int parse_options(char *opts, int *blocksize, int *inode_ratio)
{
  char *value;
  char *p;
  char endch;

  if (!opts) return 0;
  while (*opts)
  {
    p = opts;
    while (*p != 0 && *p != ',') p++;
    endch = *p;
    *p = 0;

    value = opts;
    while (*value != 0 && *value != '=') value++;
    if (*value) 
      *value++ = 0;
    else
      value = NULL;

    if (strcmp(value, "blocksize") == 0)
    {
      if (blocksize && value && atoi(value) != -1) *blocksize = atoi(value);
    }
    else if (strcmp(value, "inoderatio") == 0)
    {
      if (inode_ratio && value && atoi(value) != -1) *inode_ratio = atoi(value);
    }
    else
      return -1;

    if (value) *(value - 1) = '=';
    *p = endch;
    opts = p;
    if (*opts == ',') opts++;
  }

  return 0;
}

static struct filsys *create_filesystem(devno_t devno, int blocksize, int inode_ratio)
{
  struct filsys *fs;
  unsigned int blocks;
  unsigned int first_block;
  struct groupdesc *gd;
  struct buf *buf;
  unsigned int i;
  ino_t ino;
  struct inode *root;

  // Allocate file system
  fs = (struct filsys *) kmalloc(sizeof(struct filsys));
  memset(fs, 0, sizeof(struct filsys));

  // Allocate super block
  fs->super = (struct superblock *) kmalloc(SECTORSIZE);
  memset(fs->super, 0, SECTORSIZE);
  fs->super_dirty = 1;

  // Set device number and block size
  fs->devno = devno;
  fs->blocksize = blocksize;

  // Initialize buffer cache
  fs->cache = init_buffer_pool(devno, CACHEBUFFERS, fs->blocksize);

  // Set signature, version and block size in super block
  fs->super->signature = DFS_SIGNATURE;
  fs->super->version = DFS_VERSION;
  fs->super->log_block_size = log2(blocksize);

  // Each group has as many blocks as can be represented by the block bitmap block
  fs->super->blocks_per_group = fs->blocksize * BITS_PER_BYTE;

  // The number of inodes in a group is computed as a ratio of the size of group
  fs->inodes_per_block = fs->blocksize / sizeof(struct inodedesc);
  fs->super->inodes_per_group = fs->blocksize * fs->super->blocks_per_group / inode_ratio;
  if (fs->super->inodes_per_group > fs->blocksize * BITS_PER_BYTE) fs->super->inodes_per_group = fs->blocksize * BITS_PER_BYTE;
  fs->inode_blocks_per_group = (fs->super->inodes_per_group * sizeof(struct inodedesc) + fs->blocksize - 1) / fs->blocksize;

  // Get the device size in sectors from the device and convert it to blocks
  fs->super->block_count = dev_getsize(fs->devno) / (fs->blocksize / SECTORSIZE);

  // Calculate the number of block pointers per block directory page
  fs->log_blkptrs_per_block = fs->super->log_block_size - 2;

  // Calculate the number of group descriptors and the number of blocks to store them
  fs->super->group_count = (fs->super->block_count + fs->super->blocks_per_group - 1) / fs->super->blocks_per_group;
  fs->groupdescs_per_block = fs->blocksize / sizeof(struct groupdesc);
  fs->groupdesc_blocks = (fs->super->group_count * sizeof(struct groupdesc) + fs->blocksize - 1) / fs->blocksize;

  // The group descriptor table starts in block 1 (or block 2 of blocksize is 512)
  fs->super->groupdesc_table_block = 1;
  if (fs->blocksize <= SECTORSIZE) fs->super->groupdesc_table_block = 2;

  // If the last group is too small to hold the bitmaps and inode table skip it
  blocks =  fs->super->block_count % fs->super->blocks_per_group;
  if (blocks > 0 && blocks < fs->inode_blocks_per_group + 2) fs->super->group_count--;
  if (fs->super->group_count == 0) panic("filesystem too small");

  // Allocate group descriptors
  fs->groupdesc_buffers = (struct buf **) kmalloc(sizeof(struct buf *) * fs->groupdesc_blocks);
  fs->groups = (struct group *) kmalloc(sizeof(struct group) * fs->super->group_count);

  for (i = 0; i < fs->groupdesc_blocks; i++)
  {
    fs->groupdesc_buffers[i] = alloc_buffer(fs->cache, fs->super->groupdesc_table_block + i);
  }

  for (i = 0; i < fs->super->group_count; i++)
  {
    gd = (struct groupdesc *) fs->groupdesc_buffers[i / fs->groupdescs_per_block]->data;
    gd += (i % fs->groupdescs_per_block);

    fs->groups[i].desc = gd;
    fs->groups[i].first_free_block = 0;
    fs->groups[i].first_free_inode = 0;
  }

  // Reserve inode for root directory
  fs->super->reserved_inodes = 1;

  // Set inode count based on group count
  fs->super->inode_count = fs->inode_blocks_per_group * fs->super->group_count;

  // All blocks and inodes initially free
  fs->super->free_inode_count = fs->super->inode_count;
  fs->super->free_block_count = fs->super->block_count;

  // Initialize block bitmaps
  for (i = 0; i < fs->super->group_count; i++)
  {
    gd = fs->groups[i].desc;
    blocks = 0;
    first_block = fs->super->blocks_per_group * i;

    // The first group needs blocks for the super block and the group descriptors
    if (i == 0) blocks = fs->super->groupdesc_table_block + fs->groupdesc_blocks;

    // Next blocks in group are the block bitmap, inode bitmap and the inode table
    gd->block_bitmap_block = first_block + blocks++;
    gd->inode_bitmap_block = first_block + blocks++;
    gd->inode_table_block = first_block + blocks;
    blocks += fs->inode_blocks_per_group;

    // Update block bitmap
    buf = alloc_buffer(fs->cache, gd->block_bitmap_block);
    set_bits(buf->data, 0, blocks);
    mark_buffer_updated(buf);
    release_buffer(fs->cache, buf);

    // Determine the block count for the group. The last group may be truncated
    if (fs->super->blocks_per_group * (i + 1) > fs->super->block_count)
      gd->block_count = fs->super->block_count - fs->super->blocks_per_group * i;
    else
      gd->block_count = fs->super->blocks_per_group;

    // Set the count of free blocks and inodes for group
    gd->free_inode_count = fs->super->inodes_per_group;
    gd->free_block_count = gd->block_count - blocks;

    // Update super block
    fs->super->free_block_count -= blocks;

    mark_group_desc_dirty(fs, i);
  }

  // Reserve inodes
  for (i = 0; i < RESERVED_INODES; i++)
  {
    ino = new_inode(fs, 0, 0);
    if (ino != i) panic("unexpected inode");
  }

  // Create root directory
  root = get_inode(fs, DFS_INODE_ROOT);
  root->desc->flags = DFS_INODE_FLAG_DIRECTORY;
  root->desc->ctime = root->desc->mtime = time(NULL);
  root->desc->linkcount = 1;
  mark_buffer_updated(root->buf);
  release_inode(root);

  return fs;
}

static struct filsys *open_filesystem(devno_t devno)
{
  struct filsys *fs;
  struct groupdesc *gd;
  unsigned int i;

  // Allocate file system
  fs = (struct filsys *) kmalloc(sizeof(struct filsys));
  memset(fs, 0, sizeof(struct filsys));

  // Allocate and read super block
  fs->super = (struct superblock *) kmalloc(SECTORSIZE);
  memset(fs->super, 0, SECTORSIZE);
  if (dev_read(devno, fs->super, SECTORSIZE, 1) != SECTORSIZE) panic("unable to read superblock");
  fs->super_dirty = 0;

  // Check signature and version
  if (fs->super->signature != DFS_SIGNATURE) panic("invalid DFS signature");
  if (fs->super->version != DFS_VERSION) panic("invalid DFS version");

  // Set device number and block size
  fs->devno = devno;
  fs->blocksize = 1 << fs->super->log_block_size;
  fs->inodes_per_block = fs->blocksize / sizeof(struct inodedesc);

  // Initialize buffer cache
  fs->cache = init_buffer_pool(devno, CACHEBUFFERS, fs->blocksize);

  // Calculate the number of group descriptors blocks
  fs->groupdescs_per_block = fs->blocksize / sizeof(struct groupdesc);
  fs->groupdesc_blocks = (fs->super->group_count * sizeof(struct groupdesc) + fs->blocksize - 1) / fs->blocksize;

  // Calculate the number of block pointers per block directory page
  fs->log_blkptrs_per_block = fs->super->log_block_size - 2;

  // Read group descriptors
  fs->groupdesc_buffers = (struct buf **) kmalloc(sizeof(struct buf *) * fs->groupdesc_blocks);
  fs->groups = (struct group *) kmalloc(sizeof(struct group) * fs->super->group_count);
  for (i = 0; i < fs->groupdesc_blocks; i++)
  {
    fs->groupdesc_buffers[i] = get_buffer(fs->cache, fs->super->groupdesc_table_block + i);
  }

  for (i = 0; i < fs->super->group_count; i++)
  {
    gd = (struct groupdesc *) fs->groupdesc_buffers[i / fs->groupdescs_per_block]->data;
    gd += (i % fs->groupdescs_per_block);

    fs->groups[i].desc = gd;
    fs->groups[i].first_free_block = -1;
    fs->groups[i].first_free_inode = -1;
  }

  return fs;
}

static void close_filesystem(struct filsys *fs)
{
  unsigned int i;

  // Release all group descriptors
  for (i = 0; i < fs->groupdesc_blocks; i++) release_buffer(fs->cache, fs->groupdesc_buffers[i]);
  kfree(fs->groupdesc_buffers);
  kfree(fs->groups);

  // Write super block
  if (fs->super_dirty) dev_write(fs->devno, fs->super, SECTORSIZE, 1);
  kfree(fs->super);

  // Flush buffer cache
  flush_buffers(fs->cache);

  // Free cache
  free_buffer_pool(fs->cache);

  // Deallocate file system
  kfree(fs);
}

int dfs_format(devno_t devno, char *opts)
{
  int blocksize;
  int inode_ratio;
  struct filsys *fs;

  blocksize = DEFAULT_BLOCKSIZE;
  inode_ratio = DEFAULT_INODE_RATIO;
  if (parse_options(opts, &blocksize, &inode_ratio) != 0) return -1;
  fs = create_filesystem(devno, blocksize, inode_ratio);
  if (!fs) return -1;
  close_filesystem(fs);
  return 0;
}

int dfs_mount(struct fs *fs, char *opts)
{
  if (parse_options(opts, NULL, NULL) != 0) return -1;

  fs->data = open_filesystem(fs->devno);
  if (!fs->data) return -1;

  return 0;
}

int dfs_unmount(struct fs *fs)
{
  close_filesystem((struct filsys *) fs->data);
  return 0;
}

