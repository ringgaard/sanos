//
// dfs.h
//
// Disk Filesystem
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

#ifndef DFS_H
#define DFS_H

#define DFS_SIGNATURE              0x00534644
#define DFS_VERSION                1

#define DFS_TOPBLOCKDIR_SIZE       16
#define DFS_MAX_DEPTH              6

#define DFS_INODE_FLAG_DIRECTORY   1

#define DFS_INODE_ROOT             0
#define DFS_INODE_KRNL             1

#define DFS_MAXFNAME               255
#define DFS_MAXFILESIZE            0x7FFFFFFF

#define NOINODE                    (-1)
#define NOBLOCK                    (-1)

struct fsoptions
{
  int cache;
  int blocksize;
  int inode_ratio;
  int quick;
  int reserved_inodes;
  int reserved_blocks;
};

struct superblock
{
  unsigned int signature;
  unsigned int version;
  unsigned int log_block_size;
  blkno_t groupdesc_table_block;
  unsigned int reserved_inodes;
  unsigned int group_count;
  unsigned int inode_count;
  unsigned int block_count;
  unsigned int free_inode_count;
  unsigned int free_block_count;
  unsigned int blocks_per_group;
  unsigned int inodes_per_group;
  blkno_t first_reserved_block;
  unsigned int reserved_blocks;
  unsigned int cache_buffers;
  unsigned int compress_offset;
  unsigned int compress_size;
};

struct groupdesc
{
  unsigned int block_count;
  blkno_t block_bitmap_block;
  blkno_t inode_bitmap_block;
  blkno_t inode_table_block;
  unsigned int free_block_count;
  unsigned int free_inode_count;
  char reserved[8];
};

struct inodedesc
{
  unsigned int flags;
  time_t ctime;
  time_t mtime;
  loff_t size;
  int linkcount;
  unsigned int blocks;
  int depth;
  char reserved[36];
  blkno_t blockdir[DFS_TOPBLOCKDIR_SIZE];
};

struct dentry
{
  ino_t ino;
  unsigned int reclen;
  unsigned int namelen;
  char name[0];
};

struct group
{
  struct groupdesc *desc;
  unsigned int first_free_block; // relative to group
  unsigned int first_free_inode; // relative to group
};

struct inode
{
  struct filsys *fs;
  ino_t ino;
  struct inodedesc *desc;
  struct buf *buf;
};

struct filsys
{
  dev_t devno;

  unsigned int blocksize;
  unsigned int groupdesc_blocks;
  unsigned int inode_blocks_per_group;
  unsigned int inodes_per_block;
  unsigned int groupdescs_per_block;
  unsigned int log_blkptrs_per_block;
  int super_dirty;

  struct superblock *super;
  struct bufpool *cache;
  struct buf **groupdesc_buffers;
  struct group *groups;
};

typedef int (*filldir_t)(char *name, int len, ino_t ino, void *data);

// dfs.c
void init_dfs();
int dfs_mkdir(struct fs *fs, char *name, int mode);
int dfs_rmdir(struct fs *fs, char *name);
int dfs_rename(struct fs *fs, char *oldname, char *newname);
int dfs_link(struct fs *fs, char *oldname, char *newname);
int dfs_unlink(struct fs *fs, char *name);
int dfs_utime(struct fs *fs, char *name, struct utimbuf *times);
int dfs_stat(struct fs *fs, char *name, struct stat64 *buffer);

// super.c
struct filsys *create_filesystem(char *devname, struct fsoptions *fsopts);
struct filsys *open_filesystem(char *devname, struct fsoptions *fsopts);
void close_filesystem(struct filsys *fs);
int dfs_mkfs(char *devname, char *opts);
int dfs_mount(struct fs *fs, char *opts);
int dfs_umount(struct fs *fs);
int dfs_statfs(struct fs *fs, struct statfs *buf);

// group.c
blkno_t new_block(struct filsys *fs, blkno_t goal);
void free_blocks(struct filsys *fs, blkno_t *blocks, int count);

ino_t new_inode(struct filsys *fs, ino_t parent, int flags);
void free_inode(struct filsys *fs, ino_t ino);

// inode.c
void mark_inode_dirty(struct inode *inode);
blkno_t get_inode_block(struct inode *inode, unsigned int iblock);
blkno_t set_inode_block(struct inode *inode, unsigned int iblock, blkno_t block);
struct inode *alloc_inode(struct inode *parent, int flags);
int unlink_inode(struct inode *inode);
struct inode *get_inode(struct filsys *fs, ino_t ino);
void release_inode(struct inode *inode);
blkno_t expand_inode(struct inode *inode);
int truncate_inode(struct inode *inode, unsigned int blocks);

// dir.c
ino_t find_dir_entry(struct inode *dir, char *name, int len);
ino_t lookup_name(struct filsys *fs, ino_t ino, char *name, int len);
struct inode *parse_name(struct filsys *fs, char **name, int *len);
int add_dir_entry(struct inode *dir, char *name, int len, ino_t ino);
ino_t modify_dir_entry(struct inode *dir, char *name, int len, ino_t ino);
int delete_dir_entry(struct inode *dir, char *name, int len);
int iterate_dir(struct inode *dir, filldir_t filldir, void *data);
int dfs_opendir(struct file *filp, char *name);
int dfs_readdir(struct file *filp, struct dirent *dirp, int count);

// file.c
int dfs_open(struct file *filp, char *name);
int dfs_close(struct file *filp);
int dfs_flush(struct file *filp);
int dfs_read(struct file *filp, void *data, size_t size);
int dfs_write(struct file *filp, void *data, size_t size);
int dfs_ioctl(struct file *filp, int cmd, void *data, size_t size);
off64_t dfs_tell(struct file *filp);
off64_t dfs_lseek(struct file *filp, off64_t offset, int origin);
int dfs_chsize(struct file *filp, off64_t size);
int dfs_futime(struct file *filp, struct utimbuf *times);
int dfs_fstat(struct file *filp, struct stat64 *buffer);

#endif
