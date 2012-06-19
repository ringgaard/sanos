#ifndef VFS_H
#define VFS_H

#define VFS_O_RDONLY                0x0000  // Open for reading only
#define VFS_O_WRONLY                0x0001  // Open for writing only
#define VFS_O_RDWR                  0x0002  // Open for reading and writing
#define VFS_O_APPEND                0x0008  // Writes done at EOF

#define VFS_O_SPECIAL               0x0010

#define VFS_O_CREAT                 0x0100  // Create and open file
#define VFS_O_TRUNC                 0x0200  // Truncate file
#define VFS_O_EXCL                  0x0400  // Open only if file doesn't already exist

#define F_MODIFIED                  0x1000  // File has been modified since it was opened
#define F_DIR                       0x2000  // File is a directory

#define FS_DIRECTORY                1       // File is a directory

#define VFS_SEEK_SET                0       // Seek relative to begining of file
#define VFS_SEEK_CUR                1       // Seek relative to current file position
#define VFS_SEEK_END                2       // Seek relative to end of file

#define MAXPATH                     256     // Maximum filename length (including trailing zero)

#define PS1                         '/'    // Primary path separator
#define PS2                         '\\'   // Alternate path separator

struct vfs_utimbuf 
{
  vfs_time_t ctime;
  vfs_time_t mtime;
  vfs_time_t atime;
};

struct vfs_stat
{
  int mode;
  vfs_ino_t ino;
  vfs_time_t atime;
  vfs_time_t mtime;
  vfs_time_t ctime;
  union
  {
    struct
    {
      unsigned long size_low;
      unsigned long size_high;
    } quad;
    uint64_t size;
  };
};

struct vfs_dirent
{
  vfs_ino_t ino;
  unsigned int reclen;
  unsigned int namelen;
  char name[MAXPATH];
};

struct vfsops;

struct filesystem
{
  char *name;
  struct fsops *ops;
  struct filesystem *next;
};

struct fs
{
  vfs_devno_t devno;
  char path[MAXPATH + 1];
  struct fsops *ops;
  struct fs *next;
  void *data;
};

struct file
{
  struct fs *fs;
  int flags;
  vfs_loff_t pos;
  void *data;
};

struct fsops
{
  int (*format)(vfs_devno_t devno, char *opts);
  int (*mount)(struct fs *fs, char *opts);
  int (*unmount)(struct fs *fs);

  int (*open)(struct file *filp, char *name, int mode);
  int (*close)(struct file *filp);
  int (*flush)(struct file *filp);

  int (*read)(struct file *filp, void *data, size_t size);
  int (*write)(struct file *filp, void *data, size_t size);

  vfs_loff_t (*tell)(struct file *filp);
  vfs_loff_t (*lseek)(struct file *filp, vfs_loff_t offset, int origin);
  int (*chsize)(struct file *filp, vfs_loff_t size);

  int (*futime)(struct file *filp, struct vfs_utimbuf *times);

  int (*fstat)(struct file *filp, struct vfs_stat *buffer);
  int (*stat)(struct fs *fs, char *name, struct vfs_stat *buffer);

  int (*mkdir)(struct fs *fs, char *name, int mode);
  int (*rmdir)(struct fs *fs, char *name);

  int (*rename)(struct fs *fs, char *oldname, char *newname);
  int (*link)(struct fs *fs, char *oldname, char *newname);
  int (*unlink)(struct fs *fs, char *name);
  
  int (*opendir)(struct file *filp, char *name);
  int (*readdir)(struct file *filp, struct vfs_dirent *dirp, int count);
};

int fnmatch(char *fn1, int len1, char *fn2, int len2);
struct filesystem *register_filesystem(char *name, struct fsops *ops);
struct fs *fslookup(char *name, char **rest);

int vfs_format(vfs_devno_t devno, char *type, char *opts);
int vfs_mount(char *type, char *path, vfs_devno_t devno, char *opts);
int vfs_unmount_all();

struct file *vfs_open(char *name, int flags, int mode);
int vfs_close(struct file *filp);
int vfs_flush(struct file *filp);

int vfs_read(struct file *filp, void *data, size_t size);
int vfs_write(struct file *filp, void *data, size_t size);

vfs_loff_t vfs_tell(struct file *filp);
vfs_loff_t vfs_lseek(struct file *filp, vfs_loff_t offset, int origin);
int vfs_chsize(struct file *filp, vfs_loff_t size);

int vfs_futime(struct file *filp, struct vfs_utimbuf *times);

int vfs_fstat(struct file *filp, struct vfs_stat *buffer);
int vfs_stat(char *name, struct vfs_stat *buffer);

int vfs_mkdir(char *name, int mode);
int vfs_rmdir(char *name);

int vfs_rename(char *oldname, char *newname);
int vfs_link(char *oldname, char *newname);
int vfs_unlink(char *name);

struct file *vfs_opendir(char *name);
int vfs_readdir(struct file *filp, struct vfs_dirent *dirp, int count);

#endif
