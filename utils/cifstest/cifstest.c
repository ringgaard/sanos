#include <windows.h>
#include <stdio.h>

#include "smb.h"

//
// SMB session
//

struct smb_session
{
  int s;
  unsigned short tid;
  unsigned short uid;
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
  smb->flags2 = 1 | (1 << 14); // Long filenames and 32-bit status codes

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

  len = smb->len[3] | (smb->len[2] << 8) |  (smb->len[1] << 16) | (smb->len[0] << 24);

  if (len > SMB_MAX_BUFFER - 4) panic("response too large");

  rc = recv_fully(sess->s, (char *) &smb->protocol, len, 0);
  if (rc != len) panic("response truncated");
  if (smb->protocol[0] != 0xFF || smb->protocol[1] != 'S' || smb->protocol[2] != 'M' || smb->protocol[3] != 'B') panic("invalid smb response");

  return smb;
}

int get_smb_data_size(struct smb *smb)
{
  return *((unsigned short *) ((char *) smb->params.words + smb->wordcount * 2));
}

void *get_smb_data(struct smb *smb)
{
  return (char *) smb->params.words + smb->wordcount * 2 + 2;
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
  if (smb->status != 0) goto error;
  if (smb->params.rsp.negotiate.dialect_index == 0xFFFF) goto error;
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
  smb->params.req.setup.capabilities = SMB_CAP_STATUS32 | SMB_CAP_NT_SMBS;

  p = buf;
  p = addstrz(p, password);
  p = addstrz(p, username);
  p = addstrz(p, domain);
  p = addstrz(p, SMB_CLIENT_OS);
  p = addstrz(p, SMB_CLIENT_LANMAN);

  if (send_smb(sess, smb, SMB_COM_SESSION_SETUP_ANDX, 13, buf, p - buf) <= 0) goto error;

  smb = recv_smb(sess);
  if (!smb) goto error;
  if (smb->status != 0) goto error;
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
  if (smb->status != 0) goto error;
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

  if (send_smb(sess, smb, SMB_COM_NT_CREATE_ANDX, 24, filename, strlen(filename) + 1) < 0) return NULL;

  smb = recv_smb(sess);
  if (!smb) return NULL;
  if (smb->status != 0) return NULL;

  // Allocate smb file structure
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
    else if (smb->status != 0) 
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
  if (smb->status != 0) return -1;

  len = smb->params.rsp.read.data_length;
  if (len)
  {
    file->pos += len;
    memcpy(data, (char *) smb + smb->params.rsp.read.data_offset + 4, len);
  }

  return len;
}

void main(int argc, char *argv[])
{
  WSADATA wsadata;
  int rc;
  char *servername;
  struct smb_session *sess;
  struct smb_file *file;
  char buf[4096];
  int len;
  FILE *f;

  if (argc > 1)
    servername = argv[1];
  else
    servername = "localhost";

  rc = WSAStartup(MAKEWORD(2, 2), &wsadata);
  if (rc != 0) panic("error in WSAStartup");

  sess = smb_open_session("192.168.123.117", "c", "", "mri", "remote");
  if (!sess) panic("unable to connect to server");

  file = smb_open_file(sess, "os\\download\\dbg_x86_4.0.18.0.exe", SMB_OPEN_EXISTING, SMB_ACCESS_GENERIC_READ);
  if (!file) panic("unable to open file");

  f = fopen("dbg_x86_4.0.18.0.exe", "wb");

  while ((len = smb_file_read(file, buf, 4096)) > 0)
  {
    fwrite(buf, len, 1, f);
    printf(".");
  }
  printf("\n");
  if (len < 0) printf("error reading file\n");

  fclose(f);

  smb_close_file(file);
  smb_close_session(sess);

  printf("done\n");
}
