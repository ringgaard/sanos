#ifndef VFS_H
#define VFS_H

#define O_RDONLY                0x0000  // Open for reading only
#define O_WRONLY                0x0001  // Open for writing only
#define O_RDWR                  0x0002  // Open for reading and writing
#define O_APPEND                0x0008  // Writes done at EOF

#define O_CREAT                 0x0100  // Create and open file
#define O_TRUNC                 0x0200  // Truncate file
#define O_EXCL                  0x0400  // Open only if file doesn't already exist

#define F_MODIFIED              0x1000  // File has been modified since it was opened
#define F_DIR                   0x2000  // File is a directory

#define FS_DIRECTORY            1       // File is a directory

#define SEEK_SET                0       // Seek relative to begining of file
#define SEEK_CUR                1       // Seek relative to current file position
#define SEEK_END                2       // Seek relative to end of file

#define MAXPATH                 256     // Maximum filename length (including trailing zero)

#define PS1                     '/'    // Primary path separator
#define PS2                     '\\'   // Alternate path separator

struct stat
{
  int mode;
  ino_t ino;
  time_t atime;
  time_t mtime;
  time_t ctime;
  union
  {
    struct
    {
      unsigned long size_low;
      unsigned long size_high;
    } quad;
    unsigned __int64 size;
  };
};

struct dirent
{
  ino_t ino;
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
  devno_t devno;
  char path[MAXPATH + 1];
  struct fsops *ops;
  struct fs *next;
  void *data;
};

struct file
{
  struct fs *fs;
  int flags;
  loff_t pos;
  void *data;
};

struct fsops
{
  int (*format)(devno_t devno, char *opts);
  int (*mount)(struct fs *fs, char *opts);
  int (*unmount)(struct fs *fs);

  int (*open)(struct file *filp, char *name);
  int (*close)(struct file *filp);
  int (*flush)(struct file *filp);

  int (*read)(struct file *filp, void *data, size_t size);
  int (*write)(struct file *filp, void *data, size_t size);

  loff_t (*tell)(struct file *filp);
  loff_t (*lseek)(struct file *filp, loff_t offset, int origin);
  int (*chsize)(struct file *filp, loff_t size);

  int (*fstat)(struct file *filp, struct stat *buffer);
  int (*stat)(struct fs *fs, char *name, struct stat *buffer);

  int (*mkdir)(struct fs *fs, char *name);
  int (*rmdir)(struct fs *fs, char *name);

  int (*rename)(struct fs *fs, char *oldname, char *newname);
  int (*link)(struct fs *fs, char *oldname, char *newname);
  int (*unlink)(struct fs *fs, char *name);
  
  int (*opendir)(struct file *filp, char *name);
  int (*readdir)(struct file *filp, struct dirent *dirp, int count);
};

#ifdef KERNEL

int fnmatch(char *fn1, int len1, char *fn2, int len2);
struct filesystem *register_filesystem(char *name, struct fsops *ops);
struct fs *fslookup(char *name, char **rest);

int format(devno_t devno, char *type, char *opts);
int mount(char *type, char *path, devno_t devno, char *opts);
int unmount_all();

struct file *open(char *name, int mode);
int close(struct file *filp);
int flush(struct file *filp);

int read(struct file *filp, void *data, size_t size);
int write(struct file *filp, void *data, size_t size);

loff_t tell(struct file *filp);
loff_t lseek(struct file *filp, loff_t offset, int origin);
int chsize(struct file *filp, loff_t size);

int fstat(struct file *filp, struct stat *buffer);
int stat(char *name, struct stat *buffer);

int mkdir(char *name);
int rmdir(char *name);

int rename(char *oldname, char *newname);
int link(char *oldname, char *newname);
int unlink(char *name);

struct file *opendir(char *name);
int readdir(struct file *filp, struct dirent *dirp, int count);

#endif

#endif
