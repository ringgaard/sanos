#include "types.h"
#include <vfs.h>

#include "bitops.h"
#include "buf.h"
#include "dfs.h"

static void split_levels(struct inode *inode, unsigned int iblock, unsigned int offsets[DFS_MAX_DEPTH])
{
  unsigned int shift;
  int d;

  shift = inode->desc->depth * inode->fs->log_blkptrs_per_block;
  offsets[0] = iblock >> shift;

  for (d = 1; d <= inode->desc->depth; d++)
  {
    iblock &= ((1 << shift) - 1);
    shift -= inode->fs->log_blkptrs_per_block;
    offsets[d] = iblock >> shift;
  }
}

void mark_inode_dirty(struct inode *inode)
{
  mark_buffer_updated(inode->buf);
}

blkno_t get_inode_block(struct inode *inode, unsigned int iblock)
{
  int d;
  blkno_t block;
  struct buf *buf;
  unsigned int offsets[DFS_MAX_DEPTH];

  split_levels(inode, iblock, offsets);
  block = inode->desc->blockdir[offsets[0]];

  for (d = 1; d <= inode->desc->depth; d++)
  {
    buf = get_buffer(inode->fs->cache, block);
    block = ((blkno_t *) buf->data)[offsets[d]];
    release_buffer(inode->fs->cache, buf);

    if (!block) return -1;
  }

  return block;
}

blkno_t set_inode_block(struct inode *inode, unsigned int iblock, blkno_t block)
{
  int d;
  struct buf *buf;
  blkno_t dirblock;
  blkno_t goal;
  unsigned int offsets[DFS_MAX_DEPTH];

  goal = inode->ino / inode->fs->super->inodes_per_group * inode->fs->super->blocks_per_group;

  if (inode->desc->depth == 0)
  {
    // Allocate new block in same group as inode if requested
    if (block == -1) 
    {
      block = new_block(inode->fs, goal);
      inode->desc->blocks++;
    }

    // Update top block directory in inode descriptor
    inode->desc->blockdir[iblock] = block;
    mark_inode_dirty(inode);
  }
  else
  {
    buf = NULL;

    // Get block directory block for first level
    split_levels(inode, iblock, offsets);
    dirblock = inode->desc->blockdir[offsets[0]];

    // Allocate block and update inode
    if (dirblock == 0)
    {
      dirblock = new_block(inode->fs, goal);

      inode->desc->blockdir[offsets[0]] = dirblock;
      mark_inode_dirty(inode);

      buf = alloc_buffer(inode->fs->cache, dirblock);
      memset(buf->data, 0, inode->fs->blocksize);
    }

    // Traverse and allocate internal pages in block directory
    for (d = 1; d < inode->desc->depth; d++)
    {
      // Get directory page for next level
      if (!buf) buf = get_buffer(inode->fs->cache, dirblock);
      goal = dirblock + 1;
      dirblock = ((blkno_t *) buf->data)[offsets[d]];

      // Allocate directory page block if missing        
      if (dirblock == 0)
      {
        dirblock = new_block(inode->fs, goal);
        ((blkno_t *) buf->data)[offsets[d]] = dirblock;
        mark_buffer_updated(buf);

	buf = alloc_buffer(inode->fs->cache, dirblock);
	memset(buf->data, 0, inode->fs->blocksize);
      }
      else
      {
        release_buffer(inode->fs->cache, buf);
        buf = NULL;
      }
    }
    
    // Get leaf block directory page
    if (!buf) buf = get_buffer(inode->fs->cache, dirblock);

    // Allocate new block near previous block or leaf directory page if requested
    if (block == -1) 
    {
      block = new_block(inode->fs, offsets[d] == 0 ? dirblock + 1 : ((blkno_t *) buf->data)[offsets[d] - 1] + 1);
      inode->desc->blocks++;
      mark_inode_dirty(inode);
    }

    // Update leaf with new block
    ((blkno_t *) buf->data)[offsets[d]] = block;
    mark_buffer_updated(buf);
    release_buffer(inode->fs->cache, buf);
  }

  return block;
}

struct inode *alloc_inode(struct inode *parent, int flags)
{
  ino_t ino;
  struct inode *inode;
  unsigned int group;
  unsigned int block;

  ino = new_inode(parent->fs, parent->ino, flags);
  if (ino == -1) return NULL; 

  inode = (struct inode *) kmalloc(sizeof(struct inode));
  if (!inode) return NULL;

  inode->fs = parent->fs;
  inode->ino = ino;

  group = ino / inode->fs->super->inodes_per_group;
  block = inode->fs->groups[group].desc->inode_table_block + (ino % inode->fs->super->inodes_per_group) / inode->fs->inodes_per_block;

  inode->buf = get_buffer(inode->fs->cache, block);
  inode->desc = (struct inodedesc *) (inode->buf->data) + (ino % inode->fs->inodes_per_block);

  memset(inode->desc, 0, sizeof(struct inodedesc));
  inode->desc->flags = flags;
  inode->desc->ctime = inode->desc->mtime = time(NULL);

  mark_inode_dirty(inode);

  return inode;  
}

int unlink_inode(struct inode *inode)
{
  int rc;

  inode->desc->linkcount--;
  mark_inode_dirty(inode);
  if (inode->desc->linkcount > 0) return 0;

  rc = truncate_inode(inode, 0);
  if (rc < 0) return rc;

  memset(inode->desc, 0, sizeof(struct inodedesc));

  free_inode(inode->fs, inode->ino);

  return 0;
}

struct inode *get_inode(struct filsys *fs, ino_t ino)
{
  struct inode *inode;
  unsigned int group;
  unsigned int block;

  inode = (struct inode *) kmalloc(sizeof(struct inode));
  if (!inode) return NULL;

  inode->fs = fs;
  inode->ino = ino;

  group = ino / fs->super->inodes_per_group;
  block = fs->groups[group].desc->inode_table_block + (ino % fs->super->inodes_per_group) / fs->inodes_per_block;

  inode->buf = get_buffer(fs->cache, block);
  inode->desc = (struct inodedesc *) (inode->buf->data) + (ino % fs->inodes_per_block);

  return inode;  
}

void release_inode(struct inode *inode)
{
  if (inode->buf) release_buffer(inode->fs->cache, inode->buf);
  kfree(inode);
}

blkno_t expand_inode(struct inode *inode)
{
  unsigned int maxblocks;
  unsigned int dirblock;
  unsigned int i;
  struct buf *buf;

  // Increase depth of block directory tree if tree is full
  maxblocks = DFS_TOPBLOCKDIR_SIZE * (1 << (inode->desc->depth * inode->fs->log_blkptrs_per_block));
  if (inode->desc->blocks == maxblocks)
  {
    // Allocate block for new directory page
    dirblock = new_block(inode->fs, inode->desc->blockdir[0] - 1);
    if (dirblock == -1) return -1;

    // Move top directory entries to new directory page
    buf = alloc_buffer(inode->fs->cache, dirblock);
    memset(buf->data, 0, inode->fs->blocksize);
    for (i = 0; i < DFS_TOPBLOCKDIR_SIZE; i++)
    {
      ((blkno_t *) buf->data)[i] = inode->desc->blockdir[i];
      inode->desc->blockdir[i] = 0;
    }

    // Set top block dir to point to new directory page and increase depth
    inode->desc->blockdir[0] = dirblock;
    inode->desc->depth++;

    mark_buffer_updated(buf);
    mark_inode_dirty(inode);
    release_buffer(inode->fs->cache, buf);
  }

  // Allocate new block and add to inode block directory
  return set_inode_block(inode, inode->desc->blocks, -1);
}

static void remove_blocks(struct filsys *fs, blkno_t *blocks, int count)
{
  if (count > 0)
  {
    free_blocks(fs, blocks, count);
    memset(blocks, 0, sizeof(blkno_t) * count);
  }
}

int truncate_inode(struct inode *inode, unsigned int blocks)
{
  int d;
  unsigned int iblock;
  unsigned int blocksleft;
  unsigned int count;
  blkno_t blk;
  unsigned int offsets[DFS_MAX_DEPTH];
  struct buf *buf[DFS_MAX_DEPTH];

  // Check arguments
  if (blocks > inode->desc->blocks) return -1;

  // Check for no-op case
  if (blocks == inode->desc->blocks) return 0;
  if (inode->desc->blocks == 0) return 0;

  // If depth 0 we just have to free blocks from top directory
  if (inode->desc->depth == 0)
  {
    remove_blocks(inode->fs, inode->desc->blockdir + blocks, inode->desc->blocks - blocks);
    inode->desc->blocks = blocks;
    mark_inode_dirty(inode);
    return 0;
  }

  // Clear buffers
  for (d = 0; d < DFS_MAX_DEPTH; d++) buf[d] = NULL;

  // Remove blocks from the end of the file until we reach the requested size
  iblock = inode->desc->blocks - 1;
  blocksleft = inode->desc->blocks - blocks;
  while (blocksleft > 0)
  {
    // Traverse block directory tree until we reach the last block
    split_levels(inode, iblock, offsets);
    blk = inode->desc->blockdir[offsets[0]];

    for (d = 1; d <= inode->desc->depth; d++)
    {
      if (!buf[d] || buf[d]->blkno != blk)
      {
	if (buf[d]) release_buffer(inode->fs->cache, buf[d]);
	buf[d] = get_buffer(inode->fs->cache, blk);
      }
      blk = ((blkno_t *) buf[d]->data)[offsets[d]];
    }

    d = inode->desc->depth;
    if (blocksleft > offsets[d])
    {
      // Remove all blocks in leaf directory page and free it
      count = offsets[d] + 1;
      remove_blocks(inode->fs, (blkno_t *) buf[d]->data, count);
      free_blocks(inode->fs, &(buf[d]->blkno), 1);
      mark_buffer_invalid(buf[d]);

      iblock -= count;
      blocksleft -= count;
      offsets[d] = 0;

      // Remove internal directory pages
      while (--d > 0)
      {
	((blkno_t *) buf[d]->data)[offsets[d]] = 0;
	if (offsets[d] == 0)
	{
          free_blocks(inode->fs, &(buf[d]->blkno), 1);
          mark_buffer_invalid(buf[d]);
	}
	else
	{
  	  mark_buffer_updated(buf[d]);
	  break;
	}
      }

      // Update top directory page in inode
      if (d == 0 && offsets[1] == 0)
      {
        inode->desc->blockdir[offsets[0]] = 0;
	mark_inode_dirty(inode);
      }
    }
    else
    {
      // Remove range of blocks in leaf directory page
      remove_blocks(inode->fs, (blkno_t *) buf[d]->data + offsets[d] + 1 - blocksleft, blocksleft);
      mark_buffer_updated(buf[d]);

      iblock -= blocksleft;
      blocksleft = 0;
    }
  }

  // Release buffers
  for (d = 0; d < DFS_MAX_DEPTH; d++) 
  {
    if (buf[d]) release_buffer(inode->fs->cache, buf[d]);
  }

  // Update inode
  inode->desc->blocks = blocks;
  if (blocks == 0) inode->desc->depth = 0;
  mark_inode_dirty(inode);

  return 0;
}
