#include <windows.h>
#include <stdio.h>
#include <time.h>

#include "smb.h"

static int day_n[] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 0, 0, 0, 0};

//
// SMB session
//

struct smb_session
{
  int s;
  unsigned short tid;
  unsigned short uid;
  int tzofs;
  unsigned long server_caps;
  unsigned long max_buffer_size;
  struct smb *packet;
  char buffer[SMB_MAX_BUFFER + 4];
};

//
// SMB file
//

struct smb_file
{
  struct smb_session *sess;
  unsigned long pos;
  unsigned short fid;
};

void panic(char *msg)
{
  printf("panic: %s\n", msg);
  exit(1);
}

//
// Convert a MS-DOS time/date pair to a UNIX date (seconds since 1 1 70).
//

static time_t date_dos2unix(struct smb_session *sess, unsigned short date, unsigned short time)
{
  int month, year;
  time_t secs;

  month = ((date >> 5) & 15) - 1;
  year = date >> 9;
  secs = (time & 31) * 2 + 60 * ((time >> 5) & 63) + (time >> 11) * 3600 + 86400 *
         ((date & 31) - 1 + day_n[month] + (year / 4) + year * 365 - ((year & 3) == 0 &&
         month < 2 ? 1 : 0) + 3653);

  return secs + sess->tzofs;
}

//
// Convert linear UNIX date to a MS-DOS time/date pair.
//

static void date_unix2dos(struct smb_session *sess, int unix_date, unsigned short *date, unsigned short *time)
{
  int day, year, nl_day, month;

  unix_date -= sess->tzofs;
  *time = (unix_date % 60) / 2 +
          (((unix_date / 60) % 60) << 5) +
          (((unix_date / 3600) % 24) << 11);

  day = unix_date / 86400 - 3652;
  year = day / 365;
  if ((year + 3) / 4 + 365 * year > day) year--;
  day -= (year + 3) / 4 + 365 * year;
  if (day == 59 && !(year & 3)) 
  {
    nl_day = day;
    month = 2;
  } 
  else 
  {
    nl_day = (year & 3) || day <= 59 ? day : day - 1;
    for (month = 0; month < 12; month++)
      if (day_n[month] > nl_day)
        break;
  }

  *date = nl_day - day_n[month - 1] + 1 + (month << 5) + (year << 9);
}

#define EPOC            116444736000000000     // 00:00:00 GMT on January 1, 1970
#define SECTIMESCALE    10000000               // 1 sec resolution

static time_t ft2time(__int64 filetime)
{
  return (time_t) ((filetime - EPOC) / SECTIMESCALE);
}

char *addstr(char *p, char *s)
{
  while (*s) *p++ = *s++;
  return p;
}

char *addstrz(char *p, char *s)
{
  while (*s) *p++ = *s++;
  *p++ = 0;
  return p;
}

int recv_fully(int s, char *buf, int size, int flags)
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

#if 0
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
#endif

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
  smb->flags2 = 1; // Long filenames

  p = (char *) smb->params.words + params * 2;
  *((unsigned short *) p) = (unsigned short) datasize;
  if (datasize) memcpy(p + 2, data, datasize);

  rc = send(sess->s, (char *) smb, len + 4, 0);
  if (rc != len + 4) return -1;

  return rc;
}

struct smb *recv_smb(struct smb_session *sess)
{
  int len;
  int rc;
  struct smb *smb = sess->packet;

  rc = recv_fully(sess->s, (char *) smb, 4, 0);
  if (rc != 4) return NULL;

  len = smb->len[3] | (smb->len[2] << 8) | (smb->len[1] << 16) | (smb->len[0] << 24);

  if (len > SMB_MAX_BUFFER) panic("response too large");

  rc = recv_fully(sess->s, (char *) &smb->protocol, len, 0);
  if (rc != len) panic("response truncated");
  if (smb->protocol[0] != 0xFF || smb->protocol[1] != 'S' || smb->protocol[2] != 'M' || smb->protocol[3] != 'B') panic("invalid smb response");

  return smb;
}

#if 0
int get_smb_data_size(struct smb *smb)
{
  return *((unsigned short *) ((char *) smb->params.words + smb->wordcount * 2));
}

void *get_smb_data(struct smb *smb)
{
  return (char *) smb->params.words + smb->wordcount * 2 + 2;
}
#endif

#define ROUNDUP(x) (((x) + 3) & ~3)

int smb_trans_send(struct smb_session *sess, unsigned short cmd, 
                   void *params, int paramlen,
                   void *data, int datalen,
                   int maxparamlen, int maxdatalen)
{
  struct smb *smb = sess->packet;
  int wordcount = 15;
  int paramofs = ROUNDUP(SMB_HEADER_LEN + 2 * wordcount + 2 + 3);
  int dataofs = ROUNDUP(paramofs + paramlen);
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
  smb->flags2 = 1 | (0 << 14); // Long filenames and 32-bit status codes

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
  if (rc != len + 4) return -1;

  return rc;
}

int smb_trans_recv(struct smb_session *sess,
                   void *params, int *paramlen,
                   void *data, int *datalen)
{
  struct smb *smb;
  int paramofs, paramdisp, paramcnt;
  int dataofs, datadisp, datacnt;

  while (1)
  {
    // Receive next block
    smb = recv_smb(sess);
    if (!smb) return -1; // -EIO
    if (smb->error != 0)
    {
      printf("smb: error %d class %d\n", smb->error, smb->error_class);
      return -2; // -EPROTO
    }

    // Copy parameters
    paramofs = smb->params.rsp.trans.parameter_offset;
    paramdisp = smb->params.rsp.trans.parameter_displacement;
    paramcnt = smb->params.rsp.trans.parameter_count;
    if (params)
    {
      if (paramdisp + paramcnt > *paramlen) return -3; // -EBUF;
      if (paramcnt) memcpy((char *) params + paramdisp, (char *) smb + paramofs + 4, paramcnt);
    }

    // Copy data
    dataofs = smb->params.rsp.trans.data_offset;
    datadisp = smb->params.rsp.trans.data_displacement;
    datacnt = smb->params.rsp.trans.data_count;
    if (data)
    {
      if (datadisp + datacnt > *datalen) return -3; // -EBUF;
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

struct smb_session *smb_open_session(char *server, char *share, char *domain, char *username, char *password)
{
  int s;
  int rc;
  SOCKADDR_IN sin;
  struct smb_session *sess = NULL;
  struct smb *smb = NULL;
  unsigned short max_mpx_count;
  char buf[256];
  char *p;

  // Connect to SMB server on port 445
  s = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
  if (s == INVALID_SOCKET) return NULL;

  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  if (*server >= '0' && *server <= '9')
    sin.sin_addr.s_addr = inet_addr(server);
  else
  {
    struct hostent *hp;

    if ((hp = gethostbyname(server)) == NULL) return NULL;
    memcpy(&sin.sin_addr, hp->h_addr, hp->h_length);
  }
  sin.sin_port = htons(445);
  
  rc = connect(s, (struct sockaddr *) &sin, sizeof(sin));
  if (rc == SOCKET_ERROR) goto error;

  // Allocate session structure
  sess = (struct smb_session *) malloc(sizeof(struct smb_session));
  if (!sess) goto error;
  memset(sess, 0, sizeof(struct smb_session));

  smb = (struct smb *) sess->buffer;

  sess->s = s;
  sess->packet = smb;
  sess->tid = 0xFFFF;

  // Negotiate protocol version
  smb = sess->packet;
  memset(smb, 0, sizeof(struct smb));
  if (send_smb(sess, smb, SMB_COM_NEGOTIATE, 0, "\002NT LM 0.12", 12) < 0) goto error;

  smb = recv_smb(sess);
  if (!smb) goto error;
  if (smb->error != 0) goto error;
  if (smb->params.rsp.negotiate.dialect_index == 0xFFFF) goto error;
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
  smb->params.req.setup.capabilities = /*SMB_CAP_STATUS32 |*/ SMB_CAP_NT_SMBS;

  p = buf;
  p = addstrz(p, password);
  p = addstrz(p, username);
  p = addstrz(p, domain);
  p = addstrz(p, SMB_CLIENT_OS);
  p = addstrz(p, SMB_CLIENT_LANMAN);

  if (send_smb(sess, smb, SMB_COM_SESSION_SETUP_ANDX, 13, buf, p - buf) <= 0) goto error;

  smb = recv_smb(sess);
  if (!smb) goto error;
  if (smb->error != 0) goto error;
  sess->uid = smb->uid;

  // Connect to share
  smb = sess->packet;
  memset(smb, 0, sizeof(struct smb));
  smb->params.req.connect.andx.cmd = 0xFF;
  smb->params.req.connect.password_length = strlen(password) + 1;

  p = buf;
  p = addstrz(p, password);
  p = addstr(p, "\\\\");
  p = addstr(p, server);
  p = addstr(p, "\\");
  p = addstrz(p, share);
  p = addstrz(p, SMB_SERVICE_DISK);
  
  if (send_smb(sess, smb, SMB_COM_TREE_CONNECT_ANDX, 4, buf, p - buf) <= 0) goto error;

  smb = recv_smb(sess);
  if (!smb) goto error;
  if (smb->error != 0) goto error;
  sess->tid = smb->tid;

  return sess;

error:
  if (sess) free(sess);
  closesocket(s);

  return NULL;
}

int smb_close_session(struct smb_session *sess)
{
  struct smb *smb;

  // Disconnect from share
  smb = sess->packet;
  memset(smb, 0, sizeof(struct smb));
  send_smb(sess, smb, SMB_COM_TREE_DISCONNECT, 0, NULL, 0);
  smb = recv_smb(sess);

  // Logoff server
  smb = sess->packet;
  memset(smb, 0, sizeof(struct smb));
  smb->params.andx.cmd = 0xFF;
  send_smb(sess, smb, SMB_COM_LOGOFF_ANDX, 2, NULL, 0);
  smb = recv_smb(sess);

  // Close socket
  closesocket(sess->s);

  // Deallocate session structure
  free(sess);

  return 0;
}

struct smb_file *smb_open_file(struct smb_session *sess, char *filename, unsigned long mode, unsigned long access)
{
  struct smb_file *file;
  struct smb *smb;

  // Open/create file
  smb = sess->packet;
  memset(smb, 0, sizeof(struct smb));
  smb->params.req.create.andx.cmd = 0xFF;
  smb->params.req.create.name_length = strlen(filename) + 1;
  smb->params.req.create.desired_access = access;
  smb->params.req.create.ext_file_attributes = SMB_FILE_ATTR_NORMAL;
  smb->params.req.create.create_disposition = mode;
  smb->params.req.create.impersonation_level = 0x02;

  if (send_smb(sess, smb, SMB_COM_NT_CREATE_ANDX, 24, filename, strlen(filename) + 1) < 0) return NULL;

  smb = recv_smb(sess);
  if (!smb) return NULL;
  if (smb->error != 0) return NULL;

  // Allocate smb file structure
  printf("fid: %d\n", smb->params.rsp.create.fid);
  file = (struct smb_file *) malloc(sizeof(struct smb_file));
  if (!file) return NULL;
  file->sess = sess;
  file->pos = 0;
  file->fid = smb->params.rsp.create.fid;

  return file;
}

int smb_close_file(struct smb_file *file)
{
  struct smb *smb;
  int rc;

  // Close file
  smb = file->sess->packet;
  memset(smb, 0, sizeof(struct smb));
  smb->params.req.close.fid = file->fid;

  if (send_smb(file->sess, smb, SMB_COM_CLOSE, 3, NULL, 0) < 0)
    rc = -1;
  else
  {
    smb = recv_smb(file->sess);
    if (!smb) 
      rc = -1;
    else if (smb->error != 0) 
      rc = -1;
    else
      rc = 0;
  }

  // Deallocate file structure
  free(file);

  return rc;
}

int smb_file_read(struct smb_file *file, void *data, int size)
{
  struct smb *smb;
  int len;

  // Read from file
  smb = file->sess->packet;
  memset(smb, 0, sizeof(struct smb));
  smb->params.req.read.andx.cmd = 0xFF;
  smb->params.req.read.fid = file->fid;
  smb->params.req.read.offset = file->pos;
  smb->params.req.read.max_count = size;

  if (send_smb(file->sess, smb, SMB_COM_READ_ANDX, 12, NULL, 0) < 0) return -1;

  smb = recv_smb(file->sess);
  if (!smb) return -1;
  if (smb->error != 0) return -1;

  len = smb->params.rsp.read.data_length;
  if (len)
  {
    file->pos += len;
    memcpy(data, (char *) smb + smb->params.rsp.read.data_offset + 4, len);
  }

  return len;
}

int smb_file_read_raw(struct smb_file *file, void *data, int size)
{
  struct smb *smb;
  unsigned char hdr[4];
  int len;
  int rc;

  // Read from file
  smb = file->sess->packet;
  memset(smb, 0, sizeof(struct smb));
  smb->params.req.readraw.fid = file->fid;
  smb->params.req.readraw.offset = file->pos;
  smb->params.req.readraw.max_count = size;

  if (send_smb(file->sess, smb, SMB_COM_READ_RAW, 8, NULL, 0) < 0) return -1;

  rc = recv_fully(file->sess->s, (char *) &hdr, 4, 0);
  if (rc != 4) return -1;

  len = hdr[3] | (hdr[2] << 8) | (hdr[1] << 16) | (hdr[0] << 24);

  rc = recv_fully(file->sess->s, data, len, 0);

  file->pos += rc;
  return rc;
}

int smb_file_write(struct smb_file *file, void *data, int size)
{
  struct smb *smb;

  // Write to  file
  smb = file->sess->packet;
  memset(smb, 0, sizeof(struct smb));
  smb->params.req.write.andx.cmd = 0xFF;
  smb->params.req.write.fid = file->fid;
  smb->params.req.write.offset = file->pos & 0xFFFF;
  smb->params.req.write.data_length = size;
  smb->params.req.write.data_offset = SMB_HEADER_LEN + 14 * 2;
  smb->params.req.write.offset_high = file->pos >> 16;

  if (send_smb(file->sess, smb, SMB_COM_WRITE_ANDX, 14, data, size) < 0) return -1;

  smb = recv_smb(file->sess);
  if (!smb) return -1;
  if (smb->error != 0) return -1;

  return smb->params.rsp.write.count;
}

int smb_fstat(struct smb_file *file)
{
  struct smb_fileinfo_request req;
  //struct smb_info_standard rsp;
  struct smb_file_all_info rsp;
  int rc;
  int rsplen;
  time_t created;
  time_t modified;
  size_t size;

  req.fid = file->fid;
  //req.infolevel = SMB_INFO_STANDARD;
  req.infolevel = SMB_QUERY_FILE_ALL_INFO;

  rsplen = sizeof(rsp);
  rc = smb_trans(file->sess, TRANS2_QUERY_FILE_INFORMATION, &req, sizeof(req), NULL, 0, NULL, NULL, &rsp, &rsplen);
  if (rc < 0) return rc;

  created = ft2time(rsp.basic.creation_time);
  modified = ft2time(rsp.basic.last_write_time);
  size = (size_t) rsp.standard.end_of_file;

  printf("create: %s", asctime(gmtime(&created)));
  printf("modified: %s", asctime(gmtime(&modified)));
  printf("size: %d\n", size);

  return 0;
}

int smb_stat(struct smb_session *sess, char *name)
{
  struct smb_pathinfo_request req;
  struct smb_file_basic_info rspb;
  struct smb_file_standard_info rsps;
  int rsplen;
  int rc;
  short dummy;
  int dummylen;

  req.infolevel = SMB_QUERY_FILE_BASIC_INFO;
  req.reserved = 0;
  strcpy(req.filename, name);

  rsplen = sizeof(rspb);
  dummylen = sizeof(dummy);
  rc = smb_trans(sess, TRANS2_QUERY_PATH_INFORMATION, &req, 6 + strlen(name) + 1, NULL, 0, &dummy, &dummylen, &rspb, &rsplen);
  if (rc < 0) 
  {
    printf("smb: stat %s basic rc=%d\n", name, rc);
    return rc;
  }

  req.infolevel = SMB_QUERY_FILE_STANDARD_INFO;
  req.reserved = 0;
  strcpy(req.filename, name);

  rsplen = sizeof(rsps);
  dummylen = sizeof(dummy);
  rc = smb_trans(sess, TRANS2_QUERY_PATH_INFORMATION, &req, 6 + strlen(name) + 1, NULL, 0, &dummy, &dummylen, &rsps, &rsplen);
  if (rc < 0) 
  {
    printf("smb: stat %s standard rc=%d\n", name, rc);
    return rc;
  }

  return 0;
}

int smb_delete(struct smb_session *sess, char *filename)
{
  struct smb *smb;
  char namebuf[256 + 1 + 1];
  char *p;

  // Delete file
  smb = sess->packet;
  memset(smb, 0, sizeof(struct smb));
  smb->params.req.rename.search_attributes = 0;

  p = namebuf;
  *p++ = 4;
  p = addstrz(p, filename);

  if (send_smb(sess, smb, SMB_COM_DELETE, 1, namebuf, p - namebuf) < 0) return -1;

  smb = recv_smb(sess);
  if (!smb) return -1;
  if (smb->error != 0) return -1;

  return 0;
}

int smb_mkdir(struct smb_session *sess, char *dirname)
{
  struct smb *smb;
  char namebuf[256 + 1 + 1];
  char *p;

  // Make directory
  smb = sess->packet;
  memset(smb, 0, sizeof(struct smb));

  p = namebuf;
  *p++ = 4;
  p = addstrz(p, dirname);

  if (send_smb(sess, smb, SMB_COM_CREATE_DIRECTORY, 0, namebuf, p - namebuf) < 0) return -1;

  smb = recv_smb(sess);
  if (!smb) return -1;
  if (smb->error != 0) return -1;

  return 0;
}

int smb_rmdir(struct smb_session *sess, char *dirname)
{
  struct smb *smb;
  char namebuf[256 + 1 + 1];
  char *p;

  // Make directory
  smb = sess->packet;
  memset(smb, 0, sizeof(struct smb));

  p = namebuf;
  *p++ = 4;
  p = addstrz(p, dirname);

  if (send_smb(sess, smb, SMB_COM_DELETE_DIRECTORY, 0, namebuf, p - namebuf) < 0) return -1;

  smb = recv_smb(sess);
  if (!smb) return -1;
  if (smb->error != 0) return -1;

  return 0;
}

int smb_rename(struct smb_session *sess, char *oldname, char *newname)
{
  struct smb *smb;
  char namebuf[(256 + 1 + 1) * 2];
  char *p;

  // Rename file
  smb = sess->packet;
  memset(smb, 0, sizeof(struct smb));
  smb->params.req.rename.search_attributes = 0; //SMB_FILE_ATTR_NORMAL | SMB_FILE_ATTR_DIRECTORY;

  p = namebuf;
  *p++ = 4;
  p = addstrz(p, oldname);
  *p++ = 4;
  p = addstrz(p, newname);

  if (send_smb(sess, smb, SMB_COM_RENAME, 1, namebuf, p - namebuf) < 0) return -1;

  smb = recv_smb(sess);
  if (!smb) return -1;
  if (smb->error != 0) return -1;

  return 0;
}

int smb_copy(struct smb_session *sess, char *sourcename, char *targetname)
{
  struct smb *smb;
  char namebuf[(256 + 1 + 1) * 2];
  char *p;

  // Copy file
  smb = sess->packet;
  memset(smb, 0, sizeof(struct smb));
  smb->params.req.copy.tid2 = sess->tid;
  smb->params.req.copy.open_function = 1;
  smb->params.req.copy.flags = 1;

  p = namebuf;
  *p++ = 4;
  p = addstrz(p, sourcename);
  *p++ = 4;
  p = addstrz(p, targetname);

  if (send_smb(sess, smb, SMB_COM_COPY, 3, namebuf, p - namebuf) < 0) return -1;

  smb = recv_smb(sess);
  if (!smb) return -1;
  if (smb->error != 0) return -1;

  return 0;
}

int smb_statfs(struct smb_session *sess)
{
  struct smb *smb = sess->packet;
  struct smb_fsinfo_request req;
  struct smb_info_allocation rsp;
  int rc;
  int rsplen;

  req.infolevel = SMB_INFO_ALLOCATION;

  rsplen = sizeof(rsp);
  rc = smb_trans(sess, TRANS2_QUERY_FS_INFORMATION, &req, sizeof(req), NULL, 0, NULL, NULL, &rsp, &rsplen);
  if (rc < 0) return rc;
  if (smb->error != 0) return -1;

  printf("Total: %d MB Free: %d MB\n", rsp.units_total * (rsp.sector_per_unit / 2) / 1024, rsp.units_avail * (rsp.sector_per_unit / 2) / 1024);
  return 0;
}

int smb_listdir(struct smb_session *sess, char *dirname)
{
  struct smb_findfirst_request reqf;
  struct smb_findnext_request reqn;
  struct smb_findfirst_response rspf;
  struct smb_findnext_response rspn;
  int rsplen;
  char buf[4096];
  int buflen;
  int rc;
  struct smb_file_directory_info *fi;
  int i;
  int first;
  int skip;
  unsigned short sid;
  int count;
  time_t created;
  time_t modified;
  size_t size;

  memset(&reqf, 0, sizeof(reqf));
  reqf.search_attributes = SMB_FILE_ATTR_SYSTEM | SMB_FILE_ATTR_HIDDEN | SMB_FILE_ATTR_DIRECTORY;
  reqf.flags = SMB_CLOSE_IF_END;
  reqf.infolevel = 0x101;
  reqf.search_count = 512;
  strcpy(reqf.filename, dirname);
  if (*dirname) strcat(reqf.filename, "\\*");

  rsplen = sizeof(rspf);
  buflen = sizeof(buf);
  rc = smb_trans(sess, TRANS2_FIND_FIRST2, &reqf, 12 + strlen(reqf.filename) + 1, NULL, 0, &rspf, &rsplen, buf, &buflen);
  if (rc < 0) return rc;
  if (sess->packet->error != 0) return -1;
  
  sid = rspf.sid;
  fi = (struct smb_file_directory_info *) buf;
  count = rspf.search_count;
  first = 1;

  while (1)
  {
    for (i = 0; i < count; i++)
    {
      skip = fi->filename[0] == '.' && (fi->filename[1] == 0 || (fi->filename[1] == '.' && fi->filename[2] == 0));
      if (!skip)
      {
        created = ft2time(fi->creation_time);
        modified = ft2time(fi->last_write_time);
        size = (size_t) fi->end_of_file;

        printf("%04X %-32s%12d %s %s", fi->ext_file_attributes, fi->filename, size, fi->ext_file_attributes & SMB_FILE_ATTR_DIRECTORY ? "DIR" : "   ", asctime(gmtime(&modified)));
      }

      fi = (struct smb_file_directory_info *) ((char *) fi + fi->next_entry_offset);
    }

    if (first)
    {
      if (rspf.end_of_search) break;
      first = 0;
    }
    else
    {
      if (rspn.end_of_search) break;
    }

    memset(&reqn, 0, sizeof(reqn));
    reqn.sid = sid;
    reqn.flags = SMB_CONTINUE_BIT | SMB_CLOSE_IF_END;
    reqn.infolevel = 0x101;
    reqn.search_count = 512;

    rsplen = sizeof(rspn);
    buflen = sizeof(buf);
    rc = smb_trans(sess, TRANS2_FIND_NEXT2, &reqn, sizeof(reqn), NULL, 0, &rspn, &rsplen, buf, &buflen);
    if (rc < 0) 
    {
      printf("error %d in smb_trans\n", rc);
      return rc;
    }
    if (sess->packet->error != 0)
    {
      printf("error %d in find next\n", sess->packet->error);
      return -1;
    }

    fi = (struct smb_file_directory_info *) buf;
    count = rspn.search_count;
  }

  return 0;
}

void main(int argc, char *argv[])
{
  WSADATA wsadata;
  int rc;
  char *servername;
  struct smb_session *sess;
  struct smb_file *file;
  char buf[32 * 1024];
  int len;
  //FILE *f;

  tzset();

  if (argc > 1)
    servername = argv[1];
  else
    servername = "localhost";

  rc = WSAStartup(MAKEWORD(2, 2), &wsadata);
  if (rc != 0) panic("error in WSAStartup");

  sess = smb_open_session("192.168.123.190", "usr", "", "mri", "remote");
  //sess = smb_open_session("192.168.123.118", "c", "", "mri", "remote");
  //sess = smb_open_session("127.0.0.1", "c", "", "mri", "remote");
  //sess = smb_open_session("pcmringgaa", "d", "ebu", "mri", "remote");
  if (!sess) panic("unable to connect to server");

  //smb_rename(sess, "yyy.txt", "def\\yyy.txt");
  //smb_statfs(sess);
  //smb_listdir(sess, "\\apps\\VPN");
  //smb_delete(sess, "hello.txt");
  //smb_mkdir(sess, "testing");
  //smb_rmdir(sess, "testing");
  //smb_copy(sess, "hello.txt", "hello2.txt");
  //smb_stat(sess, "\\share\\usr\\java\\lib\\rt.jar");
  smb_stat(sess, "\\tomcat\\webapps");

#if 0
  file = smb_open_file(sess, "hello.txt", SMB_OPEN_EXISTING, SMB_ACCESS_GENERIC_READ);
  if (!file) panic("unable to create file");

  //smb_file_write(file, "HELLO", 5);
  smb_fstat(file);

  smb_close_file(file);
#endif

#if 0
  file = smb_open_file(sess, "staabi\\3b2_output.txt", SMB_OPEN_EXISTING, SMB_ACCESS_GENERIC_READ);
  if (!file) panic("unable to open file");

  //f = fopen("ar40eng.exe", "wb");

  while ((len = smb_file_read_raw(file, buf, 32 * 1024)) > 0)
  {
    //fwrite(buf, len, 1, f);
    printf(".");
  }
  printf("\n");
  if (len < 0) printf("error reading file\n");

  //fclose(f);
  smb_close_file(file);
#endif

  smb_close_session(sess);

  printf("done\n");
}
