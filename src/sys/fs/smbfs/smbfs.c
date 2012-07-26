//
// smbfs.c
//
// SMB filesystem
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
#include "smb.h"

int smb_lockfs(struct fs *fs) {
  struct smb_share *share = (struct smb_share *) fs->data;
  return wait_for_object(&share->server->lock, VFS_LOCK_TIMEOUT);
}

void smb_unlockfs(struct fs *fs) {
  struct smb_share *share = (struct smb_share *) fs->data;
  release_mutex(&share->server->lock);
}

int smb_mkfs(char *devname, char *opts) {
  return -ENOSYS;
}

int smb_mount(struct fs *fs, char *opts) {
  struct ip_addr ipaddr;
  char username[SMB_NAMELEN];
  char password[SMB_NAMELEN];
  char domain[SMB_NAMELEN];
  struct smb_share *share;
  int rc;
  unsigned short port;

  // Get options
  ipaddr.addr = get_num_option(opts, "addr", IP_ADDR_ANY);
  if (ipaddr.addr == IP_ADDR_ANY) return -EINVAL;
  get_option(opts, "user", username, sizeof(username), "sanos");
  get_option(opts, "domain", domain, sizeof(domain), "");
  get_option(opts, "password", password, sizeof(password), "");
  port = get_num_option(opts, "port", 445);

  // Check arguments
  if (!fs->mntfrom) return -EINVAL;
  if (strlen(fs->mntfrom) + 1 > SMB_NAMELEN) return -EINVAL;

  if (strlen(password) + 1 + 
      strlen(username) + 1 + 
      strlen(domain) + 1 +
      strlen(SMB_CLIENT_OS) + 1 +
      strlen(SMB_CLIENT_LANMAN) + 1 > SMB_NAMELEN) {
    return -EBUF;
  }

  if (strlen(password) + 1 + 
      strlen(fs->mntfrom) + 1 + 
      strlen(SMB_SERVICE_DISK) + 1 > SMB_NAMELEN) {
    return -EBUF;
  }

  // Allocate share block
  share = (struct smb_share *) kmalloc(sizeof(struct smb_share));
  if (!share) return -ENOMEM;
  memset(share, 0, sizeof(struct smb_share));
  strcpy(share->sharename, fs->mntfrom);

  // Get connection to server
  rc = smb_get_connection(share, &ipaddr, port, domain, username, password);
  if (rc < 0) {
    kfree(share);
    return rc;
  }

  // Connect to share
  rc = smb_connect_tree(share);
  if (rc == -ECONN || rc == -ERST) rc = smb_reconnect(share);
  if (rc < 0) {
    smb_release_connection(share);
    kfree(share);
    return rc;
  }

  fs->data = share;

  return 0;
}

int smb_umount(struct fs *fs) {
  struct smb_share *share = (struct smb_share *) fs->data;

  // Disconnect from share
  smb_disconnect_tree(share);
  
  // Release server connection
  smb_release_connection(share);

  // Deallocate share block
  kfree(share);

  return 0;
}

int smb_statfs(struct fs *fs, struct statfs *buf) {
  struct smb_share *share = (struct smb_share *) fs->data;
  struct smb_fsinfo_request req;
  struct smb_info_allocation rsp;
  int rc;
  int rsplen;

  req.infolevel = SMB_INFO_ALLOCATION;
  rsplen = sizeof(rsp);
  rc = smb_trans(share, TRANS2_QUERY_FS_INFORMATION, &req, sizeof(req), NULL, 0, NULL, NULL, &rsp, &rsplen);
  if (rc < 0) return rc;

  buf->bsize = rsp.sector_per_unit * rsp.sectorsize;
  buf->iosize = rsp.sector_per_unit * rsp.sectorsize;
  buf->blocks = rsp.units_total;
  buf->bfree = rsp.units_avail;
  buf->files = -1;
  buf->ffree = -1;
  buf->cachesize = 0;

  return 0;
}

int smb_open(struct file *filp, char *name) {
  struct smb_share *share = (struct smb_share *) filp->fs->data;
  struct smb *smb;
  struct smb_file *file;
  unsigned long mode;
  unsigned long access;
  unsigned long sharing;
  unsigned long attrs;
  int rc;

  // Convert filename
  rc = smb_convert_filename(name);
  if (rc < 0) return rc;

  // Determine open mode
  switch (filp->flags & (O_CREAT | O_EXCL | O_TRUNC)) {
    case 0:
    case O_EXCL:
      // Open existing file
      mode = SMB_OPEN_EXISTING;
      break;

    case O_CREAT:
      // Open file, create new file if it does not exists
      mode = SMB_OPEN_ALWAYS;
      break;

    case O_CREAT | O_EXCL:
    case O_CREAT | O_TRUNC | O_EXCL:
      // Create new file, fail if it exists
      mode = SMB_CREATE_NEW;
      filp->flags |= F_MODIFIED;
      break;

    case O_TRUNC:
    case O_TRUNC | O_EXCL:
      // Open and truncate existing file
      mode = SMB_TRUNCATE_EXISTING;
      break;

    case O_CREAT | O_TRUNC:
      // Create new file, trunc existing file if it exists
      mode = SMB_CREATE_ALWAYS;
      break;

    default:
      return -EINVAL;
  }

  // Determine file access
  if (filp->flags & O_RDWR) {
    access = SMB_ACCESS_GENERIC_READ | SMB_ACCESS_GENERIC_WRITE;
  } else if (filp->flags & O_WRONLY) {
    access = SMB_ACCESS_GENERIC_WRITE;
  } else if (filp->flags & (O_CREAT | O_TRUNC)) {
    access = SMB_ACCESS_GENERIC_READ | SMB_ACCESS_GENERIC_WRITE;
  } else {
    access = SMB_ACCESS_GENERIC_READ;
  }

  // Determine sharing access
  switch (SH_FLAGS(filp->flags)) {
    case SH_DENYRW:
      sharing = 0;
      break;

    case SH_DENYWR:
      sharing = SMB_FILE_SHARE_READ;
      break;

    case SH_DENYRD:
      sharing = SMB_FILE_SHARE_WRITE;
      break;

    case SH_DENYNO:
    case SH_COMPAT:
      sharing = SMB_FILE_SHARE_READ | SMB_FILE_SHARE_WRITE;
      break;

    default:
      return -EINVAL;
  }

  // Determine file attributes
  if ((filp->flags & O_CREAT) != 0 && (filp->mode & S_IWRITE) == 0) {
    attrs = SMB_FILE_ATTR_READONLY;
  } else {
    attrs = SMB_FILE_ATTR_NORMAL;
  }

  if (filp->flags & O_TEMPORARY) {
    attrs |= SMB_FILE_FLAG_DELETE_ON_CLOSE;
    access |= SMB_ACCESS_DELETE;
    sharing |= SMB_FILE_SHARE_DELETE;
  }

  if (filp->flags & O_SHORT_LIVED) attrs |= SMB_FILE_ATTR_TEMPORARY;

  if (filp->flags & O_SEQUENTIAL) {
    attrs |= SMB_FILE_FLAG_SEQUENTIAL_SCAN;
  } else if (filp->flags & O_RANDOM) {
    attrs |= SMB_FILE_FLAG_RANDOM_ACCESS;
  }

  if (filp->flags & O_DIRECT) attrs |= SMB_FILE_FLAG_NO_BUFFERING | SMB_FILE_FLAG_WRITE_THROUGH;

  // Allocate file structure
  file = (struct smb_file *) kmalloc(sizeof(struct smb_file));
  if (!file) return -EMFILE;
  memset(file, 0, sizeof(struct smb_file));

  // Open/create file
  smb = smb_init(share, 0);
  smb->params.req.create.andx.cmd = 0xFF;
  smb->params.req.create.name_length = strlen(name) + 1;
  smb->params.req.create.desired_access = access;
  smb->params.req.create.ext_file_attributes = attrs;
  smb->params.req.create.share_access = sharing;
  smb->params.req.create.create_disposition = mode;
  smb->params.req.create.impersonation_level = 0x02;

  rc = smb_request(share, smb, SMB_COM_NT_CREATE_ANDX, 24, name, strlen(name) + 1, 1);
  if (rc < 0) {
    kfree(file);
    return rc;
  }

  file->fid = smb->params.rsp.create.fid;
  file->attrs = (unsigned short) smb->params.rsp.create.ext_file_attributes;

  if (file->attrs & SMB_FILE_ATTR_DIRECTORY) {
    file->statbuf.st_mode = S_IFDIR;
  } else {
    file->statbuf.st_mode = S_IFREG;
  }

  if (file->attrs & SMB_FILE_ATTR_READONLY) {
    file->statbuf.st_mode |= S_IREAD | S_IEXEC;
  } else {
    file->statbuf.st_mode |= S_IREAD | S_IWRITE | S_IEXEC;
  }

  file->statbuf.st_dev = NODEV;
  file->statbuf.st_ino = file->fid;
  file->statbuf.st_nlink = 1;
  file->statbuf.st_ctime = ft2time(smb->params.rsp.create.creation_time);
  file->statbuf.st_mtime = ft2time(smb->params.rsp.create.last_write_time);
  file->statbuf.st_atime = ft2time(smb->params.rsp.create.last_access_time);
  file->statbuf.st_size = smb->params.rsp.create.end_of_file;

  if (filp->flags & O_APPEND) {
    filp->pos = file->statbuf.st_size;
  } else {
    filp->pos = 0;
  }

  filp->data = file;

  return 0;
}

int smb_close(struct file *filp) {
  struct smb_share *share = (struct smb_share *) filp->fs->data;
  struct smb *smb;
  int rc;

  if (filp->flags & F_DIR) {
    struct smb_directory *dir = (struct smb_directory *) filp->data;

    if (!dir->eos) {
      smb = smb_init(share, 0);
      smb->params.req.findclose.sid = dir->sid;

      rc = smb_request(share, smb, SMB_COM_FIND_CLOSE2, 1, NULL, 0, 0);
      if (rc < 0) return rc;
    }

    kfree(dir);
    filp->data = NULL;
  } else {
    struct smb_file *file = (struct smb_file *) filp->data;

    smb = smb_init(share, 0);
    smb->params.req.close.fid = file->fid;

    rc = smb_request(share, smb, SMB_COM_CLOSE, 3, NULL, 0, 0);
    if (rc < 0) return rc;

    kfree(file);
    filp->data = NULL;
  }

  smb_clear_cache(share);

  return 0;
}

int smb_destroy(struct file *filp) {
  return 0;
}

int smb_fsync(struct file *filp) {
  struct smb_share *share = (struct smb_share *) filp->fs->data;
  struct smb_file *file = (struct smb_file *) filp->data;
  struct smb *smb;
  int rc;

  if (filp->flags & F_DIR) return -EBADF;

  smb = smb_init(share, 0);
  smb->params.req.flush.fid = file->fid;

  rc = smb_request(share, smb, SMB_COM_FLUSH, 1, NULL, 0, 0);
  if (rc < 0) return rc;

  return 0;
}

static int smb_read_raw(struct smb_share *share, struct smb_file *file, void *data, size_t size, unsigned long pos) {
  struct smb *smb;
  unsigned char hdr[4];
  int len;
  int rc;

  smb = smb_init(share, 0);
  smb->params.req.readraw.fid = file->fid;
  smb->params.req.readraw.offset = pos;
  smb->params.req.readraw.max_count = size;

  rc = smb_send(share, smb, SMB_COM_READ_RAW, 8, NULL, 0);
  if (rc < 0) return rc;
  
  rc = recv_fully(share->server->sock, (char *) &hdr, 4, 0);
  if (rc < 0) return rc;

  len = hdr[3] | (hdr[2] << 8) | (hdr[1] << 16) | (hdr[0] << 24);
  if (len == 0) return 0;

  rc = recv_fully(share->server->sock, data, len, 0);
  if (rc < 0) return rc;

  return rc;
}

static int smb_read_normal(struct smb_share *share, struct smb_file *file, void *data, size_t size, off64_t pos) {
  struct smb *smb;
  int len;
  int rc;

  // Read from file
  smb = smb_init(share, 0);
  smb->params.req.read.andx.cmd = 0xFF;
  smb->params.req.read.fid = file->fid;
  smb->params.req.read.offset = ((struct smb_pos *) &pos)->low_part;
  smb->params.req.read.max_count = size;
  smb->params.req.read.offset_high = ((struct smb_pos *) &pos)->high_part;

  rc = smb_request(share, smb, SMB_COM_READ_ANDX, 12, NULL, 0, 0);
  if (rc < 0) return rc;

  len = smb->params.rsp.read.data_length;
  if (len) memcpy(data, (char *) smb + smb->params.rsp.read.data_offset + 4, len);

  return len;
}

int smb_read(struct file *filp, void *data, size_t size, off64_t pos) {
  struct smb_share *share = (struct smb_share *) filp->fs->data;
  struct smb_file *file = (struct smb_file *) filp->data;
  char *p;
  size_t left;
  size_t count;
  int rc;

  if (filp->flags & F_DIR) return -EBADF;
  if (size == 0) return 0;

  left = size;
  p = (char *) data;

  if (filp->pos < file->statbuf.st_size && pos < 0x80000000) {
    // Read data using raw mode
    while (1) {
      count = left;
      if (count > SMB_RAW_CHUNKSIZE) count = SMB_RAW_CHUNKSIZE;

      rc = smb_read_raw(share, file, p, count, (unsigned long) pos);
      if (rc < 0) return rc;
      if (rc == 0) break;

      pos += rc;
      left -= rc;
      p += rc;

      if (left == 0) return size;
    }
  }

  // Read rest using normal mode
  while (pos < file->statbuf.st_size && left > 0) {
    count = left;
    if (count > SMB_NORMAL_CHUNKSIZE) count = SMB_NORMAL_CHUNKSIZE;

    rc = smb_read_normal(share, file, p, count, pos);
    if (rc < 0) return rc;
    if (rc == 0) return size - left;

    pos += rc;
    left -= rc;
    p += rc;
  }

  return size - left;
}

static int smb_write_normal(struct smb_share *share, struct smb_file *file, void *data, size_t size, off64_t pos) {
  struct smb *smb;
  int rc;

  // Write to file
  smb = smb_init(share, 0);
  smb->params.req.write.andx.cmd = 0xFF;
  smb->params.req.write.fid = file->fid;
  smb->params.req.write.offset = ((struct smb_pos *) &pos)->low_part;
  smb->params.req.write.data_length = size;
  smb->params.req.write.data_offset = SMB_HEADER_LEN + 14 * 2;
  smb->params.req.write.offset_high = ((struct smb_pos *) &pos)->high_part;

  rc = smb_request(share, smb, SMB_COM_WRITE_ANDX, 14, data, size, 0);
  if (rc < 0) return rc;

  return smb->params.rsp.write.count;
}

int smb_write(struct file *filp, void *data, size_t size, off64_t pos) {
  struct smb_share *share = (struct smb_share *) filp->fs->data;
  struct smb_file *file = (struct smb_file *) filp->data;
  char *p;
  size_t left;
  size_t count;
  int rc;

  if (filp->flags & F_DIR) return -EBADF;
  if (size == 0) return 0;

  if (filp->flags & O_APPEND) pos = file->statbuf.st_size;

  left = size;
  p = (char *) data;

  while (left > 0) {
    count = left;
    if (count > SMB_NORMAL_CHUNKSIZE) count = SMB_NORMAL_CHUNKSIZE;

    rc = smb_write_normal(share, file, p, count, pos);
    if (rc < 0) return rc;

    pos += rc;
    filp->flags |= F_MODIFIED;
    left -= rc;
    p += rc;

    if (pos > file->statbuf.st_size) file->statbuf.st_size = pos;
  }

  return size;
}

int smb_ioctl(struct file *filp, int cmd, void *data, size_t size) {
  return -ENOSYS;
}

off64_t smb_tell(struct file *filp) {
  if (filp->flags & F_DIR) return -EBADF;

  return filp->pos;
}

off64_t smb_lseek(struct file *filp, off64_t offset, int origin) {
  struct smb_file *file = (struct smb_file *) filp->data;

  if (filp->flags & F_DIR) return -EBADF;

  switch (origin) {
    case SEEK_END:
      offset += file->statbuf.st_size;
      break;

    case SEEK_CUR:
      offset += filp->pos;
  }

  if (offset < 0) return -EINVAL;

  filp->pos = offset;
  return offset;
}

int smb_ftruncate(struct file *filp, off64_t size) {
  struct smb_share *share = (struct smb_share *) filp->fs->data;
  struct smb_file *file = (struct smb_file *) filp->data;
  struct smb_set_fileinfo_request req;
  struct smb_file_end_of_file_info info;
  struct smb_set_fileinfo_response rsp;
  int rc;
  int rsplen;

  memset(&req, 0, sizeof(req));
  req.fid = file->fid;
  req.infolevel = 0x104;

  info.end_of_file = size;

  rsplen = sizeof(rsp);
  rc = smb_trans(share, TRANS2_SET_FILE_INFORMATION, &req, sizeof(req), &info, sizeof(info), &rsp, &rsplen, NULL, NULL);
  if (rc < 0) return rc;

  if (filp->pos > size) filp->pos = size;

  return 0;
}

int smb_futime(struct file *filp, struct utimbuf *times) {
  struct smb_share *share = (struct smb_share *) filp->fs->data;
  struct smb_file *file = (struct smb_file *) filp->data;
  struct smb_set_fileinfo_request req;
  struct smb_file_basic_info info;
  struct smb_set_fileinfo_response rsp;
  int rc;
  int rsplen;

  memset(&req, 0, sizeof(req));
  req.fid = file->fid;
  req.infolevel = 0x101;

  info.creation_time = time2ft(times->ctime == -1 ? file->statbuf.st_ctime : times->ctime);
  info.last_access_time = time2ft(times->actime == -1 ? file->statbuf.st_atime : times->actime);
  info.last_write_time = time2ft(times->modtime == -1 ? file->statbuf.st_mtime : times->modtime);
  info.change_time = time2ft(times->modtime == -1 ? file->statbuf.st_mtime : times->modtime);
  info.attributes = file->attrs;

  rsplen = sizeof(rsp);
  rc = smb_trans(share, TRANS2_SET_FILE_INFORMATION, &req, sizeof(req), &info, sizeof(info), &rsp, &rsplen, NULL, NULL);
  if (rc < 0) return rc;

  if (times->ctime != -1) file->statbuf.st_ctime = times->ctime;
  if (times->modtime != -1) file->statbuf.st_mtime = times->modtime;
  if (times->actime != -1) file->statbuf.st_atime = times->actime;

  return 0;
}

int smb_utime(struct fs *fs, char *name, struct utimbuf *times) {
  struct file filp;
  int rc;

  memset(&filp, 0, sizeof(struct file));
  filp.fs = fs;

  rc = smb_open(&filp, name);
  if (rc < 0) return rc;

  rc = smb_futime(&filp, times);
  
  smb_close(&filp);
  return rc;
}

int smb_fstat(struct file *filp, struct stat64 *buffer) {
  struct smb_file *file = (struct smb_file *) filp->data;

  if (filp->flags & F_DIR) return -EBADF;

  if (buffer) memcpy(buffer, &file->statbuf, sizeof(struct stat64));

  return (int) file->statbuf.st_size;
}

int smb_stat(struct fs *fs, char *name, struct stat64 *buffer) {
  struct smb_share *share = (struct smb_share *) fs->data;
  struct smb_pathinfo_request req;
  struct smb_file_basic_info rspb;
  struct smb_file_standard_info rsps;
  struct smb_dentry *dentry;
  int rsplen;
  short dummy;
  int dummylen;
  int rc;

  rc = smb_convert_filename(name);
  if (rc < 0) return rc;

  // Handle root mount point
  if (!*name) {
    if (buffer) {
      memset(buffer, 0, sizeof(struct stat64));
      buffer->st_atime = time(0);
      buffer->st_ctime = share->mounttime;
      buffer->st_mtime = share->mounttime;
      buffer->st_mode = S_IFDIR | S_IREAD | S_IWRITE;
      buffer->st_nlink = 1;
      return 0;
    }
  }

  // Look in cache
  dentry = smb_find_in_cache(share, name);
  if (dentry != NULL) {
    if (buffer) memcpy(buffer, &dentry->statbuf, sizeof(struct stat64));
    return (int) dentry->statbuf.st_size;
  }

  // Query server for file information
  if (buffer) {
    req.infolevel = SMB_QUERY_FILE_BASIC_INFO;
    req.reserved = 0;
    strcpy(req.filename, name);

    rsplen = sizeof(rspb);
    dummylen = sizeof(dummy);
    rc = smb_trans(share, TRANS2_QUERY_PATH_INFORMATION, &req, sizeof(req) - MAXPATH + strlen(name) + 1, NULL, 0, &dummy, &dummylen, &rspb, &rsplen);
    if (rc < 0) return rc;
  }

  req.infolevel = SMB_QUERY_FILE_STANDARD_INFO;
  req.reserved = 0;
  strcpy(req.filename, name);

  rsplen = sizeof(rsps);
  dummylen = sizeof(dummy);
  rc = smb_trans(share, TRANS2_QUERY_PATH_INFORMATION, &req, sizeof(req) - MAXPATH + strlen(name) + 1, NULL, 0, &dummy, &dummylen, &rsps, &rsplen);
  if (rc < 0) return rc;

  if (buffer) {
    memset(buffer, 0, sizeof(struct stat64));

    if (rspb.attributes & SMB_FILE_ATTR_DIRECTORY) {
      buffer->st_mode = S_IFDIR | S_IREAD | S_IWRITE;
    } else if (rspb.attributes & SMB_FILE_ATTR_READONLY) {
      buffer->st_mode = S_IFREG | S_IREAD | S_IEXEC;
    } else {
      buffer->st_mode = S_IFREG | S_IREAD | S_IWRITE | S_IEXEC;
    }

    buffer->st_ino = 0;
    buffer->st_nlink = (short) rsps.number_of_links;
    buffer->st_dev = NODEV;
    buffer->st_atime = ft2time(rspb.last_access_time);
    buffer->st_mtime = ft2time(rspb.last_write_time);
    buffer->st_ctime = ft2time(rspb.creation_time);
    buffer->st_size = rsps.end_of_file;
  }

  return (int) rsps.end_of_file;
}

int smb_mkdir(struct fs *fs, char *name, int mode) {
  struct smb_share *share = (struct smb_share *) fs->data;
  struct smb *smb;
  char namebuf[MAXPATH + 1 + 1];
  char *p;
  int rc;

  rc = smb_convert_filename(name);
  if (rc < 0) return rc;

  // Make directory
  smb = smb_init(share, 0);

  p = namebuf;
  *p++ = 4;
  p = addstrz(p, name);

  rc = smb_request(share, smb, SMB_COM_CREATE_DIRECTORY, 0, namebuf, p - namebuf, 1);
  if (rc < 0) return rc;

  return 0;
}

int smb_rmdir(struct fs *fs, char *name) {
  struct smb_share *share = (struct smb_share *) fs->data;
  struct smb *smb;
  char namebuf[MAXPATH + 1 + 1];
  char *p;
  int rc;

  rc = smb_convert_filename(name);
  if (rc < 0) return rc;

  // Delete directory
  smb = smb_init(share, 0);

  p = namebuf;
  *p++ = 4;
  p = addstrz(p, name);

  rc = smb_request(share, smb, SMB_COM_DELETE_DIRECTORY, 0, namebuf, p - namebuf, 1);
  if (rc < 0) return rc;

  return 0;
}

int smb_rename(struct fs *fs, char *oldname, char *newname) {
  struct smb_share *share = (struct smb_share *) fs->data;
  struct smb *smb;
  char namebuf[(MAXPATH + 1 + 1) * 2];
  char *p;
  int rc;

  rc = smb_convert_filename(oldname);
  if (rc < 0) return rc;

  rc = smb_convert_filename(newname);
  if (rc < 0) return rc;

  // Rename file
  smb = smb_init(share, 0);

  p = namebuf;
  *p++ = 4;
  p = addstrz(p, oldname);
  *p++ = 4;
  p = addstrz(p, newname);

  rc = smb_request(share, smb, SMB_COM_RENAME, 1, namebuf, p - namebuf, 1);
  if (rc < 0) return rc;

  return 0;
}

int smb_link(struct fs *fs, char *oldname, char *newname) {
  return -ENOSYS;
}

int smb_unlink(struct fs *fs, char *name) {
  struct smb_share *share = (struct smb_share *) fs->data;
  struct smb *smb;
  char namebuf[MAXPATH + 1 + 1];
  char *p;
  int rc;

  rc = smb_convert_filename(name);
  if (rc < 0) return rc;

  // Delete file
  smb = smb_init(share, 0);

  p = namebuf;
  *p++ = 4;
  p = addstrz(p, name);

  rc = smb_request(share, smb, SMB_COM_DELETE, 1, namebuf, p - namebuf, 1);
  if (rc < 0) return rc;

  return 0;
}

int smb_opendir(struct file *filp, char *name) {
  struct smb_share *share = (struct smb_share *) filp->fs->data;
  struct smb_findfirst_request req;
  struct smb_findfirst_response rsp;
  struct smb_directory *dir;
  int rsplen;
  int buflen;
  int rc;

  rc = smb_convert_filename(name);
  if (rc < 0) return rc;

  dir = (struct smb_directory *) kmalloc(sizeof(struct smb_directory));
  if (!dir) return -ENOMEM;

  memset(&req, 0, sizeof(req));
  req.search_attributes = SMB_FILE_ATTR_SYSTEM | SMB_FILE_ATTR_HIDDEN | SMB_FILE_ATTR_DIRECTORY;
  req.flags = SMB_CLOSE_IF_END;
  req.infolevel = 0x101;
  req.search_count = 512;
  strcpy(req.filename, name);
  if (*name) strcat(req.filename, "\\*");

  rsplen = sizeof(rsp);
  buflen = SMB_DIRBUF_SIZE;
  rc = smb_trans(share, TRANS2_FIND_FIRST2, &req, 12 + strlen(req.filename) + 1, NULL, 0, &rsp, &rsplen, dir->buffer, &buflen);
  if (rc < 0) {
    kfree(dir);
    return rc;
  }

  dir->sid = rsp.sid;
  dir->eos = rsp.end_of_search;
  dir->entries_left = rsp.search_count;
  dir->fi = (struct smb_file_directory_info *) dir->buffer;
  strcpy(dir->path, name);

  filp->data = dir;
  return 0;
}

int smb_readdir(struct file *filp, struct direntry *dirp, int count) {
  struct smb_share *share = (struct smb_share *) filp->fs->data;
  struct smb_directory *dir = (struct smb_directory *) filp->data;
  struct stat64 statbuf;

  if (count != 1) return -EINVAL;

again:
  if (dir->entries_left == 0) {
    struct smb_findnext_request req;
    struct smb_findnext_response rsp;
    int rsplen;
    int buflen;
    int rc;

    if (dir->eos) return 0;

    memset(&req, 0, sizeof(req));
    req.sid = dir->sid;
    req.flags = SMB_CONTINUE_BIT | SMB_CLOSE_IF_END;
    req.infolevel = 0x101;
    req.search_count = 512;

    rsplen = sizeof(rsp);
    buflen = SMB_DIRBUF_SIZE;
    rc = smb_trans(share, TRANS2_FIND_NEXT2, &req, sizeof(req), NULL, 0, &rsp, &rsplen, dir->buffer, &buflen);
    if (rc < 0) return rc;

    dir->eos = rsp.end_of_search;
    dir->entries_left = rsp.search_count;
    dir->fi = (struct smb_file_directory_info *) dir->buffer;

    if (dir->entries_left == 0) return 0;
  }

  if (dir->fi->filename[0] == '.' && (dir->fi->filename[1] == 0 || (dir->fi->filename[1] == '.' && dir->fi->filename[2] == 0))) {
    dir->entries_left--;
    dir->fi = (struct smb_file_directory_info *) ((char *) dir->fi + dir->fi->next_entry_offset);
    goto again;
  }

  memset(&statbuf, 0, sizeof(struct stat64));

  if (dir->fi->ext_file_attributes & SMB_FILE_ATTR_DIRECTORY) {
    statbuf.st_mode = S_IFDIR | S_IREAD;
  } else if (dir->fi->ext_file_attributes & SMB_FILE_ATTR_READONLY) {
    statbuf.st_mode = S_IFREG | S_IREAD | S_IEXEC;
  } else {
    statbuf.st_mode = S_IFREG | S_IREAD | S_IWRITE | S_IEXEC;
  }

  statbuf.st_nlink = 1;
  statbuf.st_ctime = ft2time(dir->fi->creation_time);
  statbuf.st_mtime = ft2time(dir->fi->last_write_time);
  statbuf.st_atime = ft2time(dir->fi->last_access_time);
  statbuf.st_size = dir->fi->end_of_file;

  smb_add_to_cache(share, dir->path, dir->fi->filename, &statbuf);

  dirp->ino = 0;
  dirp->namelen = strlen(dir->fi->filename);
  dirp->reclen = sizeof(struct direntry) - MAXPATH + dirp->namelen + 1;
  strcpy(dirp->name, dir->fi->filename);

  dir->entries_left--;
  dir->fi = (struct smb_file_directory_info *) ((char *) dir->fi + dir->fi->next_entry_offset);

  return 1;
}

struct fsops smbfsops = {
  0,

  smb_lockfs,
  smb_unlockfs,

  smb_mkfs,
  smb_mount,
  smb_umount,

  smb_statfs,

  smb_open,
  smb_close,
  smb_destroy,
  smb_fsync,

  smb_read,
  smb_write,
  smb_ioctl,

  smb_tell,
  smb_lseek,
  smb_ftruncate,

  smb_futime,
  smb_utime,

  smb_fstat,
  smb_stat,

  NULL,

  NULL,
  NULL,
  NULL,
  NULL,

  smb_mkdir,
  smb_rmdir,

  smb_rename,
  smb_link,
  smb_unlink,

  smb_opendir,
  smb_readdir
};

void init_smbfs() {
  register_filesystem("smbfs", &smbfsops);
}
