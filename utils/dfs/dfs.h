#ifndef DFS_H
#define DFS_H

#define DFS_SIGNATURE              0x00534644
#define DFS_VERSION                2

#define DFS_TOPBLOCKDIR_SIZE       16
#define DFS_MAX_DEPTH              16

#define DFS_INODE_ROOT             0
#define DFS_INODE_KERNEL           1

#define DFS_MAXFNAME               255

struct superblock
{
  unsigned int signature;
  unsigned int version;
  unsigned int log_block_size;
  vfs_blkno_t groupdesc_table_block;
  unsigned int reserved_inodes;
  unsigned int group_count;
  unsigned int inode_count;
  unsigned int block_count;
  unsigned int free_inode_count;
  unsigned int free_block_count;
  unsigned int blocks_per_group;
  unsigned int inodes_per_group;
  vfs_blkno_t first_reserved_block;
  unsigned int reserved_blocks;
  unsigned int cache_buffers;
};

struct groupdesc
{
  unsigned int block_count;
  vfs_blkno_t block_bitmap_block;
  vfs_blkno_t inode_bitmap_block;
  vfs_blkno_t inode_table_block;
  unsigned int free_block_count;
  unsigned int free_inode_count;
  char reserved[8];
};

struct inodedesc
{
  unsigned short mode;
  unsigned short uid;
  unsigned short gid;
  unsigned short resv;
  vfs_time_t atime;
  vfs_time_t ctime;
  vfs_time_t mtime;
  unsigned int blocks;
  uint64_t size;
  int linkcount;
  int depth;
  char reserved[24];
  vfs_blkno_t blockdir[DFS_TOPBLOCKDIR_SIZE];
};

struct dentry
{
  vfs_ino_t ino;
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
  vfs_ino_t ino;
  struct inodedesc *desc;
  struct buf *buf;
};

struct filsys
{
  vfs_devno_t devno;

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

typedef int (*filldir_t)(char *name, int len, vfs_ino_t ino, void *data);

// dfs.c
void dfs_init();

// super.c
struct filsys *create_filesystem(vfs_devno_t devno, int blocksize, int inode_ratio, int quick);
struct filsys *open_filesystem(vfs_devno_t devno);
void close_filesystem(struct filsys *fs);

// group.c
vfs_blkno_t new_block(struct filsys *fs, vfs_blkno_t goal);
void free_blocks(struct filsys *fs, vfs_blkno_t *blocks, int count);

vfs_ino_t new_inode(struct filsys *fs, vfs_ino_t parent, int flags);
void free_inode(struct filsys *fs, vfs_ino_t ino);

// inode.c
void mark_inode_dirty(struct inode *inode);
vfs_blkno_t get_inode_block(struct inode *inode, unsigned int iblock);
vfs_blkno_t set_inode_block(struct inode *inode, unsigned int iblock, vfs_blkno_t block);
struct inode *alloc_inode(struct inode *parent, int flags);
int unlink_inode(struct inode *inode);
struct inode *get_inode(struct filsys *fs, vfs_ino_t ino);
void release_inode(struct inode *inode);
vfs_blkno_t expand_inode(struct inode *inode);
int truncate_inode(struct inode *inode, unsigned int blocks);

// dir.c
vfs_ino_t find_dir_entry(struct inode *dir, char *name, int len);
vfs_ino_t lookup_name(struct filsys *fs, vfs_ino_t ino, char *name, int len);
struct inode *parse_name(struct filsys *fs, char **name, int *len);
int add_dir_entry(struct inode *dir, char *name, int len, vfs_ino_t ino);
vfs_ino_t modify_dir_entry(struct inode *dir, char *name, int len, vfs_ino_t ino);
int delete_dir_entry(struct inode *dir, char *name, int len);
int read_dir(struct inode *dir, filldir_t filldir, void *data);

// file.c
struct file *open_file(struct inode *inode, int mode);
struct file *create_file(struct inode *dir, char *name, int len, int mode);
int read_file(struct file *filp, void *data, int len);
int write_file(struct file *filp, void *data, int len);
vfs_loff_t set_file_pointer(struct file *filp, vfs_loff_t offset, int origin);
vfs_loff_t get_file_size(struct file *filp);
int truncate_file(struct file *filp);
int close_file(struct file *filp);
int unlink_file(char *name, int len);

#endif
