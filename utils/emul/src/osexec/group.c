#include "types.h"
#include <vfs.h>

#include "bitops.h"
#include "buf.h"
#include "dfs.h"

void panic(char *reason);

static void mark_group_desc_dirty(struct filsys *fs, int group)
{
  mark_buffer_updated(fs->groupdesc_buffers[group / fs->groupdescs_per_block]);
}

blkno_t new_block(struct filsys *fs, blkno_t goal)
{
  unsigned int group;
  unsigned int i;
  struct buf *buf;
  blkno_t block;

  if (goal < fs->super->block_count)
  {
    // Check the goal
    group = goal / fs->super->blocks_per_group;
    block = goal % fs->super->blocks_per_group;
    buf = get_buffer(fs->cache, fs->groups[group].desc->block_bitmap_block);
    if (!test_bit(buf->data, block)) goto block_found;

    // Try to find first free block in group after goal
    block = find_next_zero_bit(buf->data, fs->groups[group].desc->block_count, block + 1);
    if (block != fs->groups[group].desc->block_count) goto block_found;

    release_buffer(fs->cache, buf);
  }
  else
    group = 0;

  // Try to locate a free block by going through all groups cyclicly
  for (i = 0; i < fs->super->group_count; i++)
  {
    if (fs->groups[group].desc->free_block_count > 0)
    {
      // Get block bitmap
      buf = get_buffer(fs->cache, fs->groups[group].desc->block_bitmap_block);

      // Try to find first free block
      if (fs->groups[group].first_free_block != -1)
        block = find_next_zero_bit(buf->data, fs->groups[group].desc->block_count, fs->groups[group].first_free_block);
      else
      {
        block = find_first_zero_bit(buf->data, fs->groups[group].desc->block_count);
	fs->groups[group].first_free_block = block;
      }

      if (block != fs->groups[group].desc->block_count) goto block_found;
    
      release_buffer(fs->cache, buf);
    }

    // Try next group
    group++;
    if (group >= fs->super->group_count) group = 0;
  }

  panic("disk full");
  return -1;

block_found:
  set_bit(buf->data, block);
  mark_buffer_updated(buf);

  fs->super->free_block_count--;
  fs->super_dirty = 1;

  if (fs->groups[group].first_free_block == block) fs->groups[group].first_free_block = block + 1;
  fs->groups[group].desc->free_block_count--;
  mark_group_desc_dirty(fs, group);

  release_buffer(fs->cache, buf);
  block += group * fs->super->blocks_per_group;
  //printf("alloc block %d\n", block);
  return block;
}

void free_blocks(struct filsys *fs, blkno_t *blocks, int count)
{
  unsigned int group;
  unsigned int prev_group;
  struct buf *buf;
  blkno_t block;
  int i;

  prev_group = -1;
  buf = NULL;

  for (i = 0; i < count; i++)
  {
    group = blocks[i] / fs->super->blocks_per_group;
    block = blocks[i] % fs->super->blocks_per_group;

    if (group != prev_group)
    {
      if (buf) release_buffer(fs->cache, buf);
      buf = get_buffer(fs->cache, fs->groups[group].desc->block_bitmap_block);
      prev_group = group;
    }

    clear_bit(buf->data, block);
    mark_buffer_updated(buf);

    fs->super->free_block_count++;
    fs->super_dirty = 1;

    if (fs->groups[group].first_free_block > block) fs->groups[group].first_free_block = block;
    fs->groups[group].desc->free_block_count++;
    mark_group_desc_dirty(fs, group);
  }

  if (buf) release_buffer(fs->cache, buf);
}

ino_t new_inode(struct filsys *fs, ino_t parent, int flags)
{
  unsigned int group;
  unsigned int i;
  unsigned int avefreei;
  struct buf *buf;
  ino_t ino;

  if (flags & DFS_INODE_FLAG_DIRECTORY)
  {
    // Find group with above average free inodes with most free blocks
    avefreei = fs->super->free_inode_count / fs->super->group_count;

    group = -1;
    for (i = 0; i < fs->super->group_count; i++)
    {
      if (fs->groups[i].desc->free_inode_count && fs->groups[i].desc->free_inode_count >= avefreei)
      {
	if (group == -1 || fs->groups[i].desc->free_block_count > fs->groups[group].desc->free_block_count)
	{
	  group = i;
	}
      }
    }
    if (group == -1) group = 0;
  }
  else
  {
    // Try to locate a free inode by going through all groups cyclicly starting with parents group
    group = parent / fs->super->inodes_per_group;
    for (i = 0; i < fs->super->group_count; i++)
    {
      if (fs->groups[group].desc->free_inode_count) break;
      group++;
      if (group >= fs->super->group_count) group = 0;
    }
  }

  // Get inode bitmap
  buf = get_buffer(fs->cache, fs->groups[group].desc->inode_bitmap_block);

  // Find first free inode in block
  if (fs->groups[group].first_free_inode != -1)
    ino = find_next_zero_bit(buf->data, fs->super->inodes_per_group, fs->groups[group].first_free_inode);
  else
  {
    ino = find_first_zero_bit(buf->data, fs->super->inodes_per_group);
    fs->groups[group].first_free_inode = ino;
  }

  if (ino == fs->super->inodes_per_group)
  {
    release_buffer(fs->cache, buf);
    panic("inode table full");
    return -1;
  }

  // Mark inode as used
  set_bit(buf->data, ino);
  mark_buffer_updated(buf);

  fs->super->free_inode_count--;
  fs->super_dirty = 1;

  if (fs->groups[group].first_free_inode == ino) fs->groups[group].first_free_inode = ino + 1;
  fs->groups[group].desc->free_inode_count--;
  mark_group_desc_dirty(fs, group);

  release_buffer(fs->cache, buf);
  ino += group * fs->super->inodes_per_group;

  return ino;
}

void free_inode(struct filsys *fs, ino_t ino)
{
  unsigned int group;
  struct buf *buf;

  group = ino / fs->super->inodes_per_group;
  ino = ino % fs->super->inodes_per_group;

  buf = get_buffer(fs->cache, fs->groups[group].desc->inode_bitmap_block);

  clear_bit(buf->data, ino);
  mark_buffer_updated(buf);

  fs->super->free_inode_count++;
  fs->super_dirty = 1;

  if (fs->groups[group].first_free_inode > ino) fs->groups[group].first_free_inode = ino;
  fs->groups[group].desc->free_inode_count++;
  mark_group_desc_dirty(fs, group);

  release_buffer(fs->cache, buf);
}
