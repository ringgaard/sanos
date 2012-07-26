//
// group.c
//
// Disk filesystem group routines
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

static void mark_group_desc_dirty(struct filsys *fs, int group) {
  mark_buffer_updated(fs->cache, fs->groupdesc_buffers[group / fs->groupdescs_per_block]);
}

blkno_t new_block(struct filsys *fs, blkno_t goal) {
  unsigned int group;
  unsigned int i;
  struct buf *buf;
  blkno_t block;

  if (goal < fs->super->block_count) {
    // Check the goal
    group = goal / fs->super->blocks_per_group;
    block = goal % fs->super->blocks_per_group;
    buf = get_buffer(fs->cache, fs->groups[group].desc->block_bitmap_block);
    if (!buf) return NOBLOCK;
    if (!test_bit(buf->data, block)) goto block_found;

    // Try to find first free block in group after goal
    block = find_next_zero_bit(buf->data, fs->groups[group].desc->block_count, block + 1);
    if (block != fs->groups[group].desc->block_count) goto block_found;

    release_buffer(fs->cache, buf);
  } else {
    group = 0;
  }

  // Try to locate a free block by going through all groups cyclicly
  for (i = 0; i < fs->super->group_count; i++) {
    if (fs->groups[group].desc->free_block_count > 0) {
      // Get block bitmap
      buf = get_buffer(fs->cache, fs->groups[group].desc->block_bitmap_block);
      if (!buf) return NOBLOCK;

      // Try to find first free block
      if (fs->groups[group].first_free_block != -1) {
        block = find_next_zero_bit(buf->data, fs->groups[group].desc->block_count, fs->groups[group].first_free_block);
      } else {
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

  //panic("disk full");
  return NOBLOCK;

block_found:
  set_bit(buf->data, block);
  mark_buffer_updated(fs->cache, buf);

  fs->super->free_block_count--;
  fs->super_dirty = 1;

  if (fs->groups[group].first_free_block == block) fs->groups[group].first_free_block = block + 1;
  fs->groups[group].desc->free_block_count--;
  mark_group_desc_dirty(fs, group);

  release_buffer(fs->cache, buf);
  block += group * fs->super->blocks_per_group;
  return block;
}

void free_blocks(struct filsys *fs, blkno_t *blocks, int count) {
  unsigned int group;
  unsigned int prev_group;
  struct buf *buf;
  blkno_t block;
  int i;

  prev_group = -1;
  buf = NULL;

  for (i = 0; i < count; i++) {
    group = blocks[i] / fs->super->blocks_per_group;
    block = blocks[i] % fs->super->blocks_per_group;

    if (group != prev_group) {
      if (buf) release_buffer(fs->cache, buf);
      buf = get_buffer(fs->cache, fs->groups[group].desc->block_bitmap_block);
      if (!buf) return;
      prev_group = group;
    }

    clear_bit(buf->data, block);
    mark_buffer_updated(fs->cache, buf);

    fs->super->free_block_count++;
    fs->super_dirty = 1;

    if (fs->groups[group].first_free_block > block) fs->groups[group].first_free_block = block;
    fs->groups[group].desc->free_block_count++;
    mark_group_desc_dirty(fs, group);
  }

  if (buf) release_buffer(fs->cache, buf);
}

ino_t new_inode(struct filsys *fs, ino_t parent, int dir) {
  unsigned int group;
  unsigned int i;
  unsigned int avefreei;
  struct buf *buf;
  ino_t ino;

  if (dir) {
    // Find group with above average free inodes with most free blocks
    avefreei = fs->super->free_inode_count / fs->super->group_count;

    group = -1;
    for (i = 0; i < fs->super->group_count; i++) {
      if (fs->groups[i].desc->free_inode_count && fs->groups[i].desc->free_inode_count >= avefreei) {
        if (group == -1 || fs->groups[i].desc->free_block_count > fs->groups[group].desc->free_block_count) {
          group = i;
        }
      }
    }
    if (group == -1) group = 0;
  } else {
    // Try to locate a free inode by going through all groups cyclicly starting with parents group
    group = parent / fs->super->inodes_per_group;
    for (i = 0; i < fs->super->group_count; i++) {
      if (fs->groups[group].desc->free_inode_count) break;
      group++;
      if (group >= fs->super->group_count) group = 0;
    }
  }

  // Get inode bitmap
  buf = get_buffer(fs->cache, fs->groups[group].desc->inode_bitmap_block);
  if (!buf) return NOINODE;

  // Find first free inode in block
  if (fs->groups[group].first_free_inode != -1) {
    ino = find_next_zero_bit(buf->data, fs->super->inodes_per_group, fs->groups[group].first_free_inode);
  } else {
    ino = find_first_zero_bit(buf->data, fs->super->inodes_per_group);
    fs->groups[group].first_free_inode = ino;
  }

  if (ino == fs->super->inodes_per_group) {
    release_buffer(fs->cache, buf);
    //panic("inode table full");
    return NOINODE;
  }

  // Mark inode as used
  set_bit(buf->data, ino);
  mark_buffer_updated(fs->cache, buf);

  fs->super->free_inode_count--;
  fs->super_dirty = 1;

  if (fs->groups[group].first_free_inode == ino) fs->groups[group].first_free_inode = ino + 1;
  fs->groups[group].desc->free_inode_count--;
  mark_group_desc_dirty(fs, group);

  release_buffer(fs->cache, buf);
  ino += group * fs->super->inodes_per_group;

  return ino;
}

void free_inode(struct filsys *fs, ino_t ino) {
  unsigned int group;
  struct buf *buf;

  group = ino / fs->super->inodes_per_group;
  ino = ino % fs->super->inodes_per_group;

  buf = get_buffer(fs->cache, fs->groups[group].desc->inode_bitmap_block);
  if (!buf) return;

  clear_bit(buf->data, ino);
  mark_buffer_updated(fs->cache, buf);

  fs->super->free_inode_count++;
  fs->super_dirty = 1;

  if (fs->groups[group].first_free_inode > ino) fs->groups[group].first_free_inode = ino;
  fs->groups[group].desc->free_inode_count++;
  mark_group_desc_dirty(fs, group);

  release_buffer(fs->cache, buf);
}
