#include <windows.h>
#include <stdio.h>

#define SMB_MAX_BUFFER 4096
#define SMB_HEADER_LEN 35

#define SMB_CLIENT_OS      "sanos"
#define SMB_CLIENT_LANMAN  "smbfs"
#define SMB_SERVICE_DISK   "A:"

//
// SMB Command Codes
//

#define SMB_COM_CREATE_DIRECTORY        0x00
#define SMB_COM_DELETE_DIRECTORY        0x01
#define SMB_COM_OPEN                    0x02
#define SMB_COM_CREATE                  0x03
#define SMB_COM_CLOSE                   0x04
#define SMB_COM_FLUSH                   0x05
#define SMB_COM_DELETE                  0x06
#define SMB_COM_RENAME                  0x07
#define SMB_COM_QUERY_INFORMATION       0x08
#define SMB_COM_SET_INFORMATION         0x09
#define SMB_COM_READ                    0x0A
#define SMB_COM_WRITE                   0x0B
#define SMB_COM_LOCK_BYTE_RANGE         0x0C
#define SMB_COM_UNLOCK_BYTE_RANGE       0x0D
#define SMB_COM_CREATE_TEMPORARY        0x0E
#define SMB_COM_CREATE_NEW              0x0F
#define SMB_COM_CHECK_DIRECTORY         0x10
#define SMB_COM_PROCESS_EXIT            0x11
#define SMB_COM_SEEK                    0x12
#define SMB_COM_LOCK_AND_READ           0x13
#define SMB_COM_WRITE_AND_UNLOCK        0x14
#define SMB_COM_READ_RAW                0x1A
#define SMB_COM_READ_MPX                0x1B
#define SMB_COM_READ_MPX_SECONDARY      0x1C
#define SMB_COM_WRITE_RAW               0x1D
#define SMB_COM_WRITE_MPX               0x1E
#define SMB_COM_WRITE_COMPLETE          0x20
#define SMB_COM_SET_INFORMATION2        0x22
#define SMB_COM_QUERY_INFORMATION2      0x23
#define SMB_COM_LOCKING_ANDX            0x24
#define SMB_COM_TRANSACTION             0x25
#define SMB_COM_TRANSACTION_SECONDARY   0x26
#define SMB_COM_IOCTL                   0x27
#define SMB_COM_IOCTL_SECONDARY         0x28
#define SMB_COM_COPY                    0x29
#define SMB_COM_MOVE                    0x2A
#define SMB_COM_ECHO                    0x2B
#define SMB_COM_WRITE_AND_CLOSE         0x2C
#define SMB_COM_OPEN_ANDX               0x2D
#define SMB_COM_READ_ANDX               0x2E
#define SMB_COM_WRITE_ANDX              0x2F
#define SMB_COM_CLOSE_AND_TREE_DISC     0x31
#define SMB_COM_TRANSACTION2            0x32
#define SMB_COM_TRANSACTION2_SECONDARY  0x33
#define SMB_COM_FIND_CLOSE2             0x34
#define SMB_COM_FIND_NOTIFY_CLOSE       0x35
#define SMB_COM_TREE_CONNECT            0x70
#define SMB_COM_TREE_DISCONNECT         0x71
#define SMB_COM_NEGOTIATE               0x72
#define SMB_COM_SESSION_SETUP_ANDX      0x73
#define SMB_COM_LOGOFF_ANDX             0x74
#define SMB_COM_TREE_CONNECT_ANDX       0x75
#define SMB_COM_QUERY_INFORMATION_DISK  0x80
#define SMB_COM_SEARCH                  0x81
#define SMB_COM_FIND                    0x82
#define SMB_COM_FIND_UNIQUE             0x83
#define SMB_COM_NT_TRANSACT             0xA0
#define SMB_COM_NT_TRANSACT_SECONDARY   0xA1
#define SMB_COM_NT_CREATE_ANDX          0xA2
#define SMB_COM_NT_CANCEL               0xA4
#define SMB_COM_OPEN_PRINT_FILE         0xC0
#define SMB_COM_WRITE_PRINT_FILE        0xC1
#define SMB_COM_CLOSE_PRINT_FILE        0xC2
#define SMB_COM_GET_PRINT_QUEUE         0xC3
#define SMB_COM_READ_BULK               0xD8
#define SMB_COM_WRITE_BULK              0xD9
#define SMB_COM_WRITE_BULK_DATA         0xDA

//
// SMB protocol capability flags
//

#define SMB_CAP_RAW_MODE		0x0001
#define SMB_CAP_MPX_MODE		0x0002
#define SMB_CAP_UNICODE			0x0004
#define SMB_CAP_LARGE_FILES		0x0008
#define SMB_CAP_NT_SMBS			0x0010
#define SMB_CAP_RPC_REMOTE_APIS		0x0020
#define SMB_CAP_STATUS32		0x0040
#define SMB_CAP_LEVEL_II_OPLOCKS	0x0080
#define SMB_CAP_LOCK_AND_READ		0x0100
#define SMB_CAP_NT_FIND			0x0200
#define SMB_CAP_DFS			0x1000
#define SMB_CAP_LARGE_READX		0x4000

//
// SMB file attributes
//

#define SMB_FILE_ATTR_ARCHIVE		0x020	// The file has not been archived since it was last modified.
#define SMB_FILE_ATTR_COMPRESSED	0x800	// The file or directory is compressed. 
#define SMB_FILE_ATTR_NORMAL		0x080	// The file has no other attributes set. 
#define SMB_FILE_ATTR_HIDDEN		0x002	// The file is hidden. 
#define SMB_FILE_ATTR_READONLY		0x001	// The file is read only. 
#define SMB_FILE_ATTR_TEMPORARY		0x100	// The file is temporary
#define SMB_FILE_ATTR_DIRECTORY		0x010	// The file is a directory
#define SMB_FILE_ATTR_SYSTEM		0x004	// The file is part of or is used exclusively by the operating system.

#pragma pack(push)
#pragma pack(1)

//
// SMB ANDX structure
//

struct smb_andx
{
  UCHAR cmd;				// Secondary (X) command; 0xFF = none
  UCHAR reserved;			// Reserved (must be 0)
  USHORT offset;			// Offset to next command WordCount
};

//
// SMB NEGOTIATE response parameters
//

struct smb_negotiate_response
{
  unsigned short dialect_index;		// Index of selected dialect
  unsigned char security_mode;		// Security mode:
					//   bit 0: 0 = share, 1 = user
					//   bit 1: 1 = encrypt passwords
					//   bit 2: 1 = Security Signatures (SMB sequence numbers) enabled
					//   bit 3: 1 = Security Signatures (SMB sequence numbers) required
  unsigned short max_mpx_count;		// Max pending multiplexed requests
  unsigned short max_number_vcs;	// Max VCs between client and server
  unsigned long max_buffer_size;	// Max transmit buffer size
  unsigned long max_raw_size;	        // Maximum raw buffer size
  unsigned long session_key;	        // Unique token identifying this session
  unsigned long capabilities;	        // Server capabilities
  unsigned long systemtime_low;	        // System (UTC) time of the server (low).
  unsigned long systemtime_high;	// System (UTC) time of the server (high).
  short server_timezone;		// Time zone of server (min from UTC)
  unsigned char encryption_key_length;	// Length of encryption key.
};

//
// SMB SESSION SETUP request parameters
//

struct smb_session_setup_request
{
  struct smb_andx andx;
  unsigned short max_buffer_size;	  // Client maximum buffer size
  unsigned short max_mpx_count;		  // Actual maximum multiplexed pending requests
  unsigned short vc_number;		  // 0 = first (only), nonzero=additional VC number
  unsigned long session_key;		  // Session key (valid iff VcNumber != 0)
  unsigned short ansi_password_length;	  // Account password size (ansi, case insensitive)
  unsigned short unicode_password_length; // Account password size (unicode, case sensitive)
  unsigned long reserved;		  // Must be 0
  unsigned long capabilities;		  // Client capabilities
};

//
// SMB SESSION SETUP response parameters
//

struct smb_session_setup_response
{
  struct smb_andx andx;
  unsigned short action;		  // Request mode: bit0 = logged in as client
};

//
// SMB TREE CONNECT request parameters
//

struct smb_tree_connect_request
{
  struct smb_andx andx;
  unsigned short flags;			  // Additional information; bit 0 set = disconnect tid
  unsigned short password_length;	  // Length of password[]
};

//
// SMB TREE CONNECT response parameters
//

struct smb_tree_connect_response
{
  struct smb_andx andx;
  unsigned short optional_support;	  // Optional support bits
};

//
// Server Message Block (SMB)
//

struct smb
{
  unsigned char len[4];                // Length field
  unsigned char protocol[4];           // Always 0xFF,'SMB'
  unsigned char cmd;                   // Command
  unsigned long status;                // Error code
  unsigned char flags;                 // Flags
  unsigned short flags2;               // More flags
  unsigned short pidhigh;              // High part of PID
  unsigned char secsig[8];             // Reserved for security
  unsigned short pad;
  unsigned short tid;                  // Tree identifier
  unsigned short pid;                  // Caller's process id
  unsigned short uid;                  // Unauthenticated user id
  unsigned short mid;                  // Multiplex id
  unsigned char wordcount;             // Count of parameter words
  union
  {
    unsigned short words[0];
    union
    {
      struct smb_session_setup_request setup;
      struct smb_tree_connect_request connect;
    } req;
    union
    {
      struct smb_negotiate_response negotiate;
      struct smb_session_setup_response setup;
      struct smb_tree_connect_response connect;
    } rsp;
  } params;
};

#pragma pack(pop)

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
};

void panic(char *msg)
{
  printf("panic: %s\n", msg);
  exit(1);
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
  smb->wordcount = (unsigned char) params;
  smb->flags2 = 1 | (1 << 14); // Long filenames and 32-bit status codes

  p = (char *) smb->params.words + params * 2;
  *((unsigned short *) p) = (unsigned short) datasize;
  memcpy(p + 2, data, datasize);

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

  smb = (struct smb *) malloc(SMB_MAX_BUFFER);
  if (!smb) goto error;

  sess->s = s;
  sess->packet = smb;

  // Negotiate protocol version
  smb = sess->packet;
  memset(smb, 0, sizeof(struct smb));
  smb->tid = 0xFFFF;
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
  smb->tid = 0xFFFF;
  smb->params.req.setup.andx.cmd = 0xFF;
  smb->params.req.setup.max_buffer_size = (unsigned short) SMB_MAX_BUFFER;
  smb->params.req.setup.max_mpx_count = max_mpx_count;
  smb->params.req.setup.ansi_password_length = strlen(password) + 1;
  smb->params.req.setup.unicode_password_length = 0;
  smb->params.req.setup.capabilities = SMB_CAP_STATUS32 | SMB_CAP_NT_SMBS;

  p = buf;

  strcpy(p, password);
  p += strlen(password) + 1;

  strcpy(p, username);
  p += strlen(username) + 1;

  strcpy(p, domain);
  p += strlen(domain) + 1;

  strcpy(p, SMB_CLIENT_OS);
  p += strlen(SMB_CLIENT_OS) + 1;

  strcpy(p, SMB_CLIENT_LANMAN);
  p += strlen(SMB_CLIENT_LANMAN) + 1;

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

  strcpy(p, password);
  p += strlen(password) + 1;

  *p++ = '\\';
  *p++ = '\\';
  strcpy(p, server);
  p += strlen(server);
  *p++ = '\\';
  strcpy(p, share);
  p += strlen(share) + 1;

  strcpy(p, SMB_SERVICE_DISK);
  p += strlen(SMB_SERVICE_DISK) + 1;
  
  if (send_smb(sess, smb, SMB_COM_TREE_CONNECT, 4, buf, p - buf) <= 0) goto error;

  smb = recv_smb(sess);
  if (!smb) goto error;
  if (smb->status != 0) goto error;
  sess->tid = smb->tid;

  return sess;

error:
  if (smb) free(smb);
  if (sess) free(sess);
  closesocket(s);

  return NULL;
}

void main(int argc, char *argv[])
{
  WSADATA wsadata;
  int rc;
  char *servername;
  struct smb_session *sess;

  if (argc > 1)
    servername = argv[1];
  else
    servername = "localhost";

  rc = WSAStartup(MAKEWORD(2, 2), &wsadata);
  if (rc != 0) panic("error in WSAStartup");

  sess = smb_open_session(servername, "d", "ebu", "mri", "remote");
 
  if (sess) closesocket(sess->s);

  printf("done\n");
}
