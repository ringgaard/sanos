//
// smbfs.c
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// SMB filesystem routines
//

#include <os/krnl.h>

int smbfs_format(char *devname, char *opts)
{
  return -ENOSYS;
}

int smbfs_mount(struct fs *fs, char *opts)
{
  return -ENOSYS;
}

int smbfs_unmount(struct fs *fs)
{
  return -ENOSYS;
}

int smbfs_statfs(struct fs *fs, struct statfs *buf)
{
  return -ENOSYS;
}

int smbfs_open(struct file *filp, char *name)
{
  return -ENOSYS;
}

int smbfs_close(struct file *filp)
{
  return -ENOSYS;
}

int smbfs_flush(struct file *filp)
{
  return -ENOSYS;
}

int smbfs_read(struct file *filp, void *data, size_t size)
{
  return -ENOSYS;
}

int smbfs_write(struct file *filp, void *data, size_t size)
{
  return -ENOSYS;
}

int smbfs_ioctl(struct file *filp, int cmd, void *data, size_t size)
{
  return -ENOSYS;
}

loff_t smbfs_tell(struct file *filp)
{
  return -ENOSYS;
}

int smbfs_chsize(struct file *filp, loff_t size)
{
  return -ENOSYS;
}

int smbfs_futime(struct file *filp, struct utimbuf *times)
{
  return -ENOSYS;
}

int smbfs_fstat(struct file *filp, struct stat *buffer)
{
  return -ENOSYS;
}

int smbfs_utime(struct fs *fs, char *name, struct utimbuf *times)
{
  return -ENOSYS;
}

int smbfs_stat(struct fs *fs, char *name, struct stat *buffer)
{
  return -ENOSYS;
}

int smbfs_mkdir(struct fs *fs, char *name)
{
  return -ENOSYS;
}

int smbfs_rmdir(struct fs *fs, char *name)
{
  return -ENOSYS;
}

int smbfs_rename(struct fs *fs, char *oldname, char *newname)
{
  return -ENOSYS;
}

int smbfs_link(struct fs *fs, char *oldname, char *newname)
{
  return -ENOSYS;
}

int smbfs_unlink(struct fs *fs, char *name)
{
  return -ENOSYS;
}

int smbfs_opendir(struct file *filp, char *name)
{
  return -ENOSYS;
}

int smbfs_readdir(struct file *filp, struct dirent *dirp, int count)
{
  return -ENOSYS;
}

struct fsops smbfsfsops =
{
  FSOP_READ | FSOP_WRITE | FSOP_IOCTL | FSOP_TELL | FSOP_LSEEK | FSOP_CHSIZE | 
  FSOP_FUTIME | FSOP_FSTAT,

  smbfs_format,
  smbfs_mount,
  smbfs_unmount,

  smbfs_statfs,

  smbfs_open,
  smbfs_close,
  smbfs_flush,

  smbfs_read,
  smbfs_write,
  smbfs_ioctl,

  smbfs_tell,
  smbfs_lseek,
  smbfs_chsize,

  smbfs_futime,
  smbfs_utime,

  smbfs_fstat,
  smbfs_stat,

  smbfs_mkdir,
  smbfs_rmdir,

  smbfs_rename,
  smbfs_link,
  smbfs_unlink,

  smbfs_opendir,
  smbfs_readdir
};

void init_smbfsfs()
{
  register_filesystem("smbfs", &smbfsops);
}
