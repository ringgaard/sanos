//
// smbfs.c
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// SMB filesystem
//

#include <os/krnl.h>
#include "smb.h"

#define ROUNDUP(x) (((x) + 3) & ~3)

#define SMB_DENTRY_CACHESIZE 16
#define SMB_DIRBUF_SIZE      4096

#define EPOC            116444736000000000     // 00:00:00 GMT on January 1, 1970
#define SECTIMESCALE    10000000               // 1 sec resolution

//
// SMB directory entry
//

struct smb_dentry
{
  char path[MAXPATH];
  struct stat statbuf;
};

//
// SMB file
//

struct smb_file
{
  unsigned short fid;
  size_t size;
  time_t atime;
  time_t mtime;
  time_t ctime;
};

//
// SMF directory
//

struct smb_directory
{
  unsigned short sid;
  int eos;
  int entries_left;
  struct smb_file_directory_info *fi;
  char path[MAXPATH];
  char buffer[SMB_DIRBUF_SIZE];
};

//
// SMB session
//

struct smb_session
{
  struct socket *s;
  unsigned short tid;
  unsigned short uid;
  int tzofs;
  time_t mounttime;
  unsigned long server_caps;
  unsigned long max_buffer_size;
  struct smb *packet;
  int next_cacheidx;
  struct smb_dentry dircache[SMB_DENTRY_CACHESIZE];
  char buffer[SMB_MAX_BUFFER + 4];
};

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

static char *addstr(char *p, char *s)
{
  while (*s) *p++ = *s++;
  return p;
}

static char *addstrz(char *p, char *s)
{
  while (*s) *p++ = *s++;
  *p++ = 0;
  return p;
}

static time_t ft2time(smb_time filetime)
{
  return (time_t) ((filetime - EPOC) / SECTIMESCALE);
}

static void add_to_cache(struct smb_session *sess, char *path, char *filename, struct stat *statbuf)
{
  int idx = sess->next_cacheidx;

  if (*path)
  {
    strcpy(sess->dircache[idx].path, path);
    strcat(sess->dircache[idx].path, "\\");
    strcat(sess->dircache[idx].path, filename);
  }
  else
    strcpy(sess->dircache[idx].path, filename);

  memcpy(&sess->dircache[idx].statbuf, statbuf, sizeof(struct stat));

  if (++sess->next_cacheidx == SMB_DENTRY_CACHESIZE) sess->next_cacheidx = 0;
}

static struct smb_dentry *find_in_cache(struct smb_session *sess, char *path)
{
  int idx;

  for (idx = 0; idx < SMB_DENTRY_CACHESIZE; idx++)
  {
    if (sess->dircache[idx].path[0] && strcmp(sess->dircache[idx].path, path) == 0)
    {
      return &sess->dircache[idx];
    }
  }
  
  return NULL;
}

static void clear_cache(struct smb_session *sess)
{
  int idx;

  for (idx = 0; idx < SMB_DENTRY_CACHESIZE; idx++) sess->dircache[idx].path[0] = 0;
}

int smb_errno(struct smb *smb)
{
  int errcls = smb->error_class;
  int error  = smb->error;

  if (errcls == SMB_ERRDOS)
  {
    switch (error)
    {
      case ERRbadfunc: return -EINVAL;
      case ERRbadfile: return -ENOENT;
      case ERRbadpath: return -ENOENT;
      case ERRnofids: return -EMFILE;
      case ERRnoaccess: return -EACCES;
      case ERRbadfid: return -EBADF;
      case ERRbadmcb: return -EREMOTEIO;
      case ERRnomem: return -ENOMEM;
      case ERRbadmem: return -EFAULT;
      case ERRbadenv: return -EREMOTEIO;
      case ERRbadformat: return -EREMOTEIO;
      case ERRbadaccess: return -EACCES;
      case ERRbaddata: return -E2BIG;
      case ERRbaddrive: return -ENXIO;
      case ERRremcd: return -EREMOTEIO;
      case ERRdiffdevice: return -EXDEV;
      case ERRnofiles: return 0;
      case ERRbadshare: return -ETXTBSY;
      case ERRlock: return -EDEADLK;
      case ERRfilexists: return -EEXIST;
      case 87: return 0;
      case 123: return -ENOENT;
      case 145: return -ENOTEMPTY;
      case 183: return -EEXIST;
      default: return -EIO;
    }
  } 
  else if (errcls == SMB_ERRSRV)
  {
    switch (error)
    {
      case ERRerror: return -ENFILE;
      case ERRbadpw: return -EINVAL;
      case ERRbadtype: return -EIO;
      case ERRaccess: return -EACCES;
      case ERRinvnid: return -EBADSLT;
      default: return -EIO;
    }
  } 
  else if (errcls == SMB_ERRHRD)
  {
    switch (error)
    {
      case ERRnowrite: return -EROFS;
      case ERRbadunit: return -ENODEV;
      case ERRnotready: return -EUCLEAN;
      case ERRbadcmd: return -EIO;
      case ERRdata: return -EIO;
      case ERRbadreq: return -ERANGE;
      case ERRbadshare: return -ETXTBSY;
      case ERRlock: return -EDEADLK;
      default: return -EIO;
    }
  } 
  else if (errcls == SMB_ERRCMD)
    return -EIO;

  return -EIO;
}

int send_smb(struct smb_session *sess, struct smb *smb, unsigned char cmd, int params, char *data, int datasize)
{
  int len;
  int rc;
  char *p;

  len = SMB_HEADER_LEN + params * 2 + 2 + datasize;

  smb->len[0] = (len > 0xFF000000) >> 24;
  smb->len[1] = (len > 0xFF0000) >> 16;
  smb->len[2] = (len & 0xFF00) >> 8;
  smb->len[3] = (len & 0xFF);
    
  smb->protocol[0] = 0xFF;
  smb->protocol[1] = 'S';
  smb->protocol[2] = 'M';
  smb->protocol[3] = 'B';

  smb->cmd = cmd;
  smb->tid = sess->tid;
  smb->uid = sess->uid;
  smb->wordcount = (unsigned char) params;
  smb->flags2 = 1;

  p = (char *) smb->params.words + params * 2;
  *((unsigned short *) p) = (unsigned short) datasize;
  if (datasize) memcpy(p + 2, data, datasize);

  rc = send(sess->s, (char *) smb, len + 4, 0);
  if (rc < 0) return rc;
  if (rc != len + 4) return -EIO;

  return 0;
}

int recv_fully(struct socket *s, char *buf, int size, int flags)
{
  int bytes;
  int left;

  left = size;
  while (left > 0)
  {
    bytes = recv(s, buf, left, flags);
    if (bytes <= 0) return bytes;

    buf += bytes;
    left -= bytes;
  }

  return size;
}

int recv_smb(struct smb_session *sess)
{
  int len;
  int rc;
  struct smb *smb = sess->packet;

  rc = recv_fully(sess->s, (char *) smb, 4, 0);
  if (rc < 0) return rc;
  if (rc != 4) return -EIO;

  len = smb->len[3] | (smb->len[2] << 8) | (smb->len[1] << 16) | (smb->len[0] << 24);
  if (len > SMB_MAX_BUFFER) return -EMSGSIZE;

  rc = recv_fully(sess->s, (char *) &smb->protocol, len, 0);
  if (rc < 0) return rc;
  if (rc != len) return -EIO;
  if (smb->protocol[0] != 0xFF || smb->protocol[1] != 'S' || smb->protocol[2] != 'M' || smb->protocol[3] != 'B') return -EPROTO;
  if (smb->error_class != SMB_SUCCESS) return smb_errno(smb);

  return 0;
}

int smb_trans_send(struct smb_session *sess, unsigned short cmd, 
		   void *params, int paramlen,
		   void *data, int datalen,
		   int maxparamlen, int maxdatalen)
{
  struct smb *smb = sess->packet;
  int wordcount = 15;
  int paramofs = ROUNDUP(SMB_HEADER_LEN + 2 * wordcount + 2 + 3);
  int dataofs =	ROUNDUP(paramofs + paramlen);
  int bcc = dataofs + datalen - (SMB_HEADER_LEN + 2 * wordcount + 2);
  int len = SMB_HEADER_LEN + 2 * wordcount + 2 + bcc;
  char *p;
  int rc;

  memset(smb, 0, sizeof(struct smb));

  smb->len[0] = (len > 0xFF000000) >> 24;
  smb->len[1] = (len > 0xFF0000) >> 16;
  smb->len[2] = (len & 0xFF00) >> 8;
  smb->len[3] = (len & 0xFF);
    
  smb->protocol[0] = 0xFF;
  smb->protocol[1] = 'S';
  smb->protocol[2] = 'M';
  smb->protocol[3] = 'B';

  smb->cmd = SMB_COM_TRANSACTION2;
  smb->tid = sess->tid;
  smb->uid = sess->uid;
  smb->wordcount = wordcount;
  smb->flags2 = 1;

  smb->params.req.trans.total_parameter_count = paramlen;
  smb->params.req.trans.total_data_count = datalen;
  smb->params.req.trans.max_parameter_count = maxparamlen;
  smb->params.req.trans.max_data_count = maxdatalen;
  smb->params.req.trans.max_setup_count = 0;
  smb->params.req.trans.parameter_count = paramlen;
  smb->params.req.trans.parameter_offset = paramofs; 
  smb->params.req.trans.data_count = datalen;
  smb->params.req.trans.data_offset = dataofs;
  smb->params.req.trans.setup_count = 1;
  smb->params.req.trans.setup[0] = cmd;

  p = (char *) smb->params.words + wordcount * 2;
  *((unsigned short *) p) = (unsigned short) bcc;

  p = (char *) smb + 4;

  if (params) memcpy(p + paramofs, params, paramlen);
  if (data) memcpy(p + dataofs, data, datalen);

  rc = send(sess->s, (char *) smb, len + 4, 0);
  if (rc < 0) return rc;
  if (rc != len + 4) return -EIO;

  return 0;
}

int smb_trans_recv(struct smb_session *sess,
		   void *params, int *paramlen,
		   void *data, int *datalen)
{
  struct smb *smb;
  int paramofs, paramdisp, paramcnt;
  int dataofs, datadisp, datacnt;
  int rc;

  while (1)
  {
    // Receive next block
    rc = recv_smb(sess);
    if (rc < 0) return rc;
    smb = sess->packet;

    // Copy parameters
    paramofs = smb->params.rsp.trans.parameter_offset;
    paramdisp = smb->params.rsp.trans.parameter_displacement;
    paramcnt = smb->params.rsp.trans.parameter_count;
    if (params)
    {
      if (paramdisp + paramcnt > *paramlen) return -EBUF;
      if (paramcnt) memcpy((char *) params + paramdisp, (char *) smb + paramofs + 4, paramcnt);
    }

    // Copy data
    dataofs = smb->params.rsp.trans.data_offset;
    datadisp = smb->params.rsp.trans.data_displacement;
    datacnt = smb->params.rsp.trans.data_count;
    if (data)
    {
      if (datadisp + datacnt > *datalen) return -EBUF;
      if (datacnt) memcpy((char *) data + datadisp, (char *) smb + dataofs + 4, datacnt);
    }

    // Check for last block
    if (paramdisp + paramcnt == smb->params.rsp.trans.total_parameter_count &&
        datadisp + datacnt == smb->params.rsp.trans.total_data_count)
    {
      *paramlen = smb->params.rsp.trans.total_parameter_count;
      *datalen = smb->params.rsp.trans.total_data_count;
      return 0;
    }
  }
}

int smb_trans(struct smb_session *sess,
	      unsigned short cmd, 
	      void *reqparams, int reqparamlen,
	      void *reqdata, int reqdatalen,
	      void *rspparams, int *rspparamlen,
	      void *rspdata, int *rspdatalen)
{
  int rc;
  int dummyparamlen;
  int dummydatalen;

  if (!rspparamlen) 
  {
    dummyparamlen = 0;
    rspparamlen = &dummyparamlen;
  }

  if (!rspdatalen) 
  {
    dummydatalen = 0;
    rspdatalen = &dummydatalen;
  }

  rc = smb_trans_send(sess, cmd, reqparams, reqparamlen, reqdata, reqdatalen, *rspparamlen, *rspdatalen);
  if (rc < 0) return rc;

  rc = smb_trans_recv(sess, rspparams, rspparamlen, rspdata, rspdatalen);
  if (rc < 0) return rc;

  return 0;
}

int smb_format(char *devname, char *opts)
{
  return -ENOSYS;
}

int smb_mount(struct fs *fs, char *opts)
{
  struct ip_addr ipaddr;
  char username[256];
  char password[256];
  char domain[256];
  char buf[256];
  int rc;
  struct socket *s;
  struct sockaddr_in sin;
  struct smb_session *sess = NULL;
  struct smb *smb;
  unsigned short max_mpx_count;
  char *p;

  // Get options
  if (!fs->mntfrom) return -EINVAL;
  ipaddr.addr = get_num_option(opts, "addr", IP_ADDR_ANY);
  if (ipaddr.addr == IP_ADDR_ANY) return -EINVAL;
  get_option(opts, "user", username, sizeof(username), "");
  get_option(opts, "domain", domain, sizeof(domain), "");
  get_option(opts, "password", password, sizeof(password), "");

  // Connect to SMB server on port 445
  rc = socket(AF_INET, SOCK_STREAM, IPPROTO_IP, &s);
  if (rc < 0) return rc;

  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = ipaddr.addr;
  sin.sin_port = htons(445);
  
  rc = connect(s, (struct sockaddr *) &sin, sizeof(sin));
  if (rc < 0) goto error;

  // Allocate session structure
  sess = (struct smb_session *) kmalloc(sizeof(struct smb_session));
  if (!sess) 
  {
    rc = -ENOMEM;
    goto error;
  }
  memset(sess, 0, sizeof(struct smb_session));

  smb = (struct smb *) sess->buffer;
  sess->s = s;
  sess->packet = smb;
  sess->tid = 0xFFFF;

  // Negotiate protocol version
  smb = sess->packet;
  memset(smb, 0, sizeof(struct smb));
  rc = send_smb(sess, smb, SMB_COM_NEGOTIATE, 0, "\002NT LM 0.12", 12); 
  if (rc < 0) goto error;

  rc = recv_smb(sess);
  if (rc < 0) goto error;

  if (smb->params.rsp.negotiate.dialect_index == 0xFFFF) 
  {
    rc = -EREMOTEIO;
    goto error;
  }

  sess->tzofs = 0; //smb->params.rsp.negotiate.server_timezone * 60;
  sess->server_caps = smb->params.rsp.negotiate.capabilities;
  sess->max_buffer_size = smb->params.rsp.negotiate.max_buffer_size;
  max_mpx_count = smb->params.rsp.negotiate.max_mpx_count;

  // Setup session
  smb = sess->packet;
  memset(smb, 0, sizeof(struct smb));
  smb->params.req.setup.andx.cmd = 0xFF;
  smb->params.req.setup.max_buffer_size = (unsigned short) SMB_MAX_BUFFER;
  smb->params.req.setup.max_mpx_count = max_mpx_count;
  smb->params.req.setup.ansi_password_length = strlen(password) + 1;
  smb->params.req.setup.unicode_password_length = 0;
  smb->params.req.setup.capabilities = SMB_CAP_NT_SMBS;

  if (strlen(password) + 1 + 
      strlen(username) + 1 + 
      strlen(domain) + 1 +
      strlen(SMB_CLIENT_OS) + 1 +
      strlen(SMB_CLIENT_LANMAN) + 1 > sizeof(buf)) return -EBUF;

  p = buf;
  p = addstrz(p, password);
  p = addstrz(p, username);
  p = addstrz(p, domain);
  p = addstrz(p, SMB_CLIENT_OS);
  p = addstrz(p, SMB_CLIENT_LANMAN);

  rc = send_smb(sess, smb, SMB_COM_SESSION_SETUP_ANDX, 13, buf, p - buf); 
  if (rc < 0) goto error;

  rc = recv_smb(sess);
  if (rc < 0) goto error;
  sess->uid = smb->uid;

  // Connect to share
  smb = sess->packet;
  memset(smb, 0, sizeof(struct smb));
  smb->params.req.connect.andx.cmd = 0xFF;
  smb->params.req.connect.password_length = strlen(password) + 1;

  if (strlen(password) + 1 + 
      strlen(fs->mntfrom) + 1 + 
      strlen(domain) + 1 +
      strlen(SMB_SERVICE_DISK) + 1 > sizeof(buf)) return -EBUF;

  p = buf;
  p = addstrz(p, password);
  p = addstrz(p, fs->mntfrom);
  p = addstrz(p, SMB_SERVICE_DISK);

  rc = send_smb(sess, smb, SMB_COM_TREE_CONNECT_ANDX, 4, buf, p - buf); 
  if (rc < 0) goto error;

  rc = recv_smb(sess);
  if (rc < 0) goto error;
  sess->tid = smb->tid;
  sess->mounttime = time(0);

  fs->data = sess;
  return 0;

error:
  if (sess) free(sess);
  closesocket(s);
  return rc;
}

int smb_unmount(struct fs *fs)
{
  struct smb_session *sess = (struct smb_session *) fs->data;
  struct smb *smb = sess->packet;
  int rc;

  // Disconnect from share
  memset(smb, 0, sizeof(struct smb));
  rc = send_smb(sess, smb, SMB_COM_TREE_DISCONNECT, 0, NULL, 0);
  if (rc < 0) return rc;

  rc = recv_smb(sess);
  if (rc < 0) return rc;

  // Logoff server
  memset(smb, 0, sizeof(struct smb));
  smb->params.andx.cmd = 0xFF;
  rc = send_smb(sess, smb, SMB_COM_LOGOFF_ANDX, 2, NULL, 0);
  if (rc < 0) return rc;

  rc = recv_smb(sess);
  if (rc < 0) return rc;

  // Close socket
  closesocket(sess->s);

  // Deallocate session structure
  kfree(sess);

  return 0;
}

int smb_statfs(struct fs *fs, struct statfs *buf)
{
  struct smb_session *sess = (struct smb_session *) fs->data;
  struct smb *smb = sess->packet;
  struct smb_fsinfo_request req;
  struct smb_info_allocation rsp;
  int rc;
  int rsplen;

  req.infolevel = SMB_INFO_ALLOCATION;
  rsplen = sizeof(rsp);
  rc = smb_trans(sess, TRANS2_QUERY_FS_INFORMATION, &req, sizeof(req), NULL, 0, NULL, NULL, &rsp, &rsplen);
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

int smb_open(struct file *filp, char *name)
{
  return -ENOSYS;
}

int smb_close(struct file *filp)
{
  //if (filp->flags & F_DIR) return smb_closedir(filp);
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
  struct smb_session *sess = (struct smb_session *) filp->fs->data;
  struct smb_findfirst_request req;
  struct smb_findfirst_response rsp;
  struct smb_directory *dir;
  int rsplen;
  int buflen;
  int rc;

  rc = convert_filename(name);
  if (rc < 0) return rc;

  dir = (struct smb_directory *) kmalloc(sizeof(struct smb_sirectory));
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
  rc = smb_trans(sess, TRANS2_FIND_FIRST2, &req, 12 + strlen(req.filename) + 1, NULL, 0, &rsp, &rsplen, dir->buffer, &buflen);
  if (rc < 0) 
  {
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

int smb_readdir(struct file *filp, struct dirent *dirp, int count)
{
  if (count != 1) return -1;

  return -ENOSYS;
}
