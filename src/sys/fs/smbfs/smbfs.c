//
// smbfs.c
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// SMB filesystem
//

#include <os/krnl.h>

int smb_format(char *devname, char *opts);
int smb_mount(struct fs *fs, char *opts);
int smb_unmount(struct fs *fs);
int smb_statfs(struct fs *fs, struct statfs *buf);
int smb_open(struct file *filp, char *name);
int smb_close(struct file *filp);
int smb_flush(struct file *filp);
int smb_read(struct file *filp, void *data, size_t size);
int smb_write(struct file *filp, void *data, size_t size);
int smb_ioctl(struct file *filp, int cmd, void *data, size_t size);
loff_t smb_tell(struct file *filp);
loff_t smb_lseek(struct file *filp, loff_t offset, int origin);
int smb_chsize(struct file *filp, loff_t size);
int smb_futime(struct file *filp, struct utimbuf *times);
int smb_utime(struct fs *fs, char *name, struct utimbuf *times);
int smb_fstat(struct file *filp, struct stat *buffer);
int smb_stat(struct fs *fs, char *name, struct stat *buffer);
int smb_mkdir(struct fs *fs, char *name);
int smb_rmdir(struct fs *fs, char *name);
int smb_rename(struct fs *fs, char *oldname, char *newname);
int smb_link(struct fs *fs, char *oldname, char *newname);
int smb_unlink(struct fs *fs, char *name);
int smb_opendir(struct file *filp, char *name);
int smb_readdir(struct file *filp, struct dirent *dirp, int count);

struct fsops smbfsops =
{
  FSOP_FORMAT | FSOP_MOUNT | FSOP_UNMOUNT | FSOP_STATFS | FSOP_OPEN | FSOP_CLOSE |
  FSOP_FLUSH | FSOP_READ | FSOP_WRITE | FSOP_IOCTL | FSOP_TELL | FSOP_LSEEK | 
  FSOP_CHSIZE | FSOP_FUTIME | FSOP_UTIME | FSOP_FSTAT | FSOP_STAT | FSOP_MKDIR |
  FSOP_RMDIR | FSOP_RENAME | FSOP_LINK | FSOP_UNLINK | FSOP_OPENDIR | FSOP_READDIR,

  smb_format,
  smb_mount,
  smb_unmount,

  smb_statfs,

  smb_open,
  smb_close,
  smb_flush,

  smb_read,
  smb_write,
  smb_ioctl,

  smb_tell,
  smb_lseek,
  smb_chsize,

  smb_futime,
  smb_utime,

  smb_fstat,
  smb_stat,

  smb_mkdir,
  smb_rmdir,

  smb_rename,
  smb_link,
  smb_unlink,

  smb_opendir,
  smb_readdir
};

void init_smbfs()
{
  register_filesystem("smbfs", &smbfsops);
}

int smb_format(char *devname, char *opts)
{
  return -ENOSYS;
}

int smb_mount(struct fs *fs, char *opts)
{
  return -ENOSYS;
}

int smb_unmount(struct fs *fs)
{
  return -ENOSYS;
}

int smb_statfs(struct fs *fs, struct statfs *buf)
{
  return -ENOSYS;
}

int smb_open(struct file *filp, char *name)
{
  return -ENOSYS;
}

int smb_close(struct file *filp)
{
  return -ENOSYS;
}

int smb_flush(struct file *filp)
{
  return -ENOSYS;
}

int smb_read(struct file *filp, void *data, size_t size)
{
  return -ENOSYS;
}

int smb_write(struct file *filp, void *data, size_t size)
{
  return -ENOSYS;
}

int smb_ioctl(struct file *filp, int cmd, void *data, size_t size)
{
  return -ENOSYS;
}

loff_t smb_tell(struct file *filp)
{
  return -ENOSYS;
}

loff_t smb_lseek(struct file *filp, loff_t offset, int origin)
{
  return -ENOSYS;
}

int smb_chsize(struct file *filp, loff_t size)
{
  return -ENOSYS;
}

int smb_futime(struct file *filp, struct utimbuf *times)
{
  return -ENOSYS;
}

int smb_utime(struct fs *fs, char *name, struct utimbuf *times)
{
  return -ENOSYS;
}

int smb_fstat(struct file *filp, struct stat *buffer)
{
  return -ENOSYS;
}

int smb_stat(struct fs *fs, char *name, struct stat *buffer)
{
  return -ENOSYS;
}

int smb_mkdir(struct fs *fs, char *name)
{
  return -ENOSYS;
}

int smb_rmdir(struct fs *fs, char *name)
{
  return -ENOSYS;
}

int smb_rename(struct fs *fs, char *oldname, char *newname)
{
  return -ENOSYS;
}

int smb_link(struct fs *fs, char *oldname, char *newname)
{
  return -ENOSYS;
}

int smb_unlink(struct fs *fs, char *name)
{
  return -ENOSYS;
}

int smb_opendir(struct file *filp, char *name)
{
  return -ENOSYS;
}

int smb_readdir(struct file *filp, struct dirent *dirp, int count)
{
  return -ENOSYS;
}
