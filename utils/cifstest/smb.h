#ifndef SMB_H
#define SMB_H

#define SMB_HEADER_LEN 35
#define SMB_MAX_BUFFER 4356
#define SMB_BLK_SIZE   4096

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
// SMB file attributes and flags
//

#define SMB_FILE_ATTR_ARCHIVE		0x020	// The file has not been archived since it was last modified.
#define SMB_FILE_ATTR_COMPRESSED	0x800	// The file or directory is compressed. 
#define SMB_FILE_ATTR_NORMAL		0x080	// The file has no other attributes set. 
#define SMB_FILE_ATTR_HIDDEN		0x002	// The file is hidden. 
#define SMB_FILE_ATTR_READONLY		0x001	// The file is read only. 
#define SMB_FILE_ATTR_TEMPORARY		0x100	// The file is temporary
#define SMB_FILE_ATTR_DIRECTORY		0x010	// The file is a directory
#define SMB_FILE_ATTR_SYSTEM		0x004	// The file is part of or is used exclusively by the operating system.

#define SMB_FILE_FLAG_WRITE_THROUGH     0x80000000
#define SMB_FILE_FLAG_NO_BUFFERING      0x20000000
#define SMB_FILE_FLAG_RANDOM_ACCESS     0x10000000
#define SMB_FILE_FLAG_SEQUENTIAL_SCAN   0x08000000
#define SMB_FILE_FLAG_DELETE_ON_CLOSE   0x04000000
#define SMB_FILE_FLAG_BACKUP_SEMANTICS  0x02000000
#define SMB_FILE_FLAG_POSIX_SEMANTICS   0x01000000

//
// SMB access mask

#define SMB_ACCESS_DELETE               0x00010000
#define SMB_ACCESS_READ_CONTROL         0x00020000
#define SMB_ACCESS_WRITE_DAC            0x00040000
#define SMB_ACCESS_WRITE_OWNER          0x00080000
#define SMB_ACCESS_SYNCHRONIZE          0x00100000

#define SMB_ACCESS_GENERIC_READ         0x80000000
#define SMB_ACCESS_GENERIC_WRITE        0x40000000
#define SMB_ACCESS_GENERIC_EXECUTE      0x20000000
#define SMB_ACCESS_GENERIC_ALL          0x10000000

//
// SMB file sharing access
//

#define SMB_FILE_SHARE_READ		0x00000001
#define SMB_FILE_SHARE_WRITE		0x00000002
#define SMB_FILE_SHARE_DELETE	        0x00000004


//
// SMB file create disposition
//

#define SMB_CREATE_NEW			1
#define SMB_CREATE_ALWAYS		2
#define SMB_OPEN_EXISTING		3
#define SMB_OPEN_ALWAYS			4
#define SMB_TRUNCATE_EXISTING		5

//
// SMB impersonation levels
//

#define SMB_SECURITY_ANONYMOUS		0
#define SMB_SECURITY_IDENTIFICATION	1
#define SMB_SECURITY_IMPERSONATION	2
#define SMB_SECURITY_DELEGATION		3

#pragma pack(push)
#pragma pack(1)

struct smb_size
{
  unsigned long low;
  unsigned long high;
};

struct smb_time
{
  unsigned long low_time;
  long high_time;
};

//
// SMB ANDX structure
//

struct smb_andx
{
  unsigned char cmd;			// Secondary (X) command; 0xFF = none
  unsigned char reserved;		// Reserved (must be 0)
  unsigned short offset;		// Offset to next command WordCount
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
// SMB CREATE FILE request parameters
//

struct smb_create_file_request
{
  struct smb_andx andx;
  unsigned char reserved;		// Reserved (must be 0)
  unsigned short name_length;		// Length of name[] in bytes
  unsigned long flags;			// Create bit set:
                                        //   0x02 - Request an oplock
                                        //   0x04 - Request a batch oplock
                                        //   0x08 - Target of open must be directory
  unsigned long root_directory_fid;	// If non-zero, open is relative to this directory
  unsigned long desired_access;		// Access desired
  struct smb_size allocation_size;	// Initial allocation size
  unsigned long ext_file_attributes;	// File attributes
  unsigned long share_access;		// Type of share access
  unsigned long create_disposition;	// Action to take if file exists or not
  unsigned long create_options;		// Options to use if creating a file
  unsigned long impersonation_level;	// Security QOS information
  unsigned char security_flags;		// Security tracking mode flags:
                                        //   0x1 - SECURITY_CONTEXT_TRACKING
                                        //   0x2 - SECURITY_EFFECTIVE_ONLY
};

//
// SMB CREATE FILE response parameters
//

struct smb_create_file_response
{
  struct smb_andx andx;
  unsigned char oplock_level;		// The oplock level granted
                                        //   0 - No oplock granted
                                        //   1 - Exclusive oplock granted
                                        //   2 - Batch oplock granted
                                        //   3 - Level II oplock granted
  unsigned short fid;			// The file ID
  unsigned long create_action;		// The action taken
  struct smb_time creation_time;	// The time the file was created
  struct smb_time last_access_time;	// The time the file was accessed
  struct smb_time last_write_time;	// The time the file was last written
  struct smb_time change_time;		// The time the file was last changed
  unsigned long ext_file_attributes;	// The file attributes
  struct smb_size allocation_size;	// The number of byes allocated
  struct smb_size end_of_file;		// The end of file offset
  unsigned short file_type;	
  unsigned short device_state;		// State of IPC device (e.g. pipe)
  unsigned char directory;		// TRUE if this is a directory
};

//
// SMB CLOSE FILE request parameters
//

struct smb_close_file_request
{
  unsigned short fid;			  // File handle
  struct smb_time last_write_time; 	  // Time of last write
};

//
// SMB READ FILE request parameters
//

struct smb_read_file_request
{
  struct smb_andx andx;
  unsigned short fid;			// File handle
  unsigned long offset;			// Offset in file to begin read
  unsigned short max_count;		// Max number of bytes to return
  unsigned short min_count;		// Reserved
  unsigned long reserved;		// Must be 0
  unsigned short remaining;		// Reserved
  unsigned long offset_high;		// Upper 32 bits of offset (only if WordCount is 12)
};

//
// SMB READ FILE response parameters
//

struct smb_read_file_response
{
  struct smb_andx andx;
  unsigned short remaining;		// Reserved -- must be -1
  unsigned short data_compaction_mode;	
  unsigned short reserved1;		// Reserved (must be 0)
  unsigned short data_length;		// Number of data bytes (min = 0)
  unsigned short data_offset;		// Offset (from header start) to data
  unsigned short reserved2[5];		// Reserved (must be 0)
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
    struct smb_andx andx;
    union
    {
      struct smb_session_setup_request setup;
      struct smb_tree_connect_request connect;
      struct smb_create_file_request create;
      struct smb_close_file_request close;
      struct smb_read_file_request read;
    } req;
    union
    {
      struct smb_negotiate_response negotiate;
      struct smb_session_setup_response setup;
      struct smb_tree_connect_response connect;
      struct smb_create_file_response create;
      struct smb_read_file_response read;
    } rsp;
  } params;
};

#pragma pack(pop)

#endif
