//
// os.h
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Operating system API
//

#ifndef OS_H
#define OS_H

#include <types.h>
#include <stdarg.h>
#include <time.h>

#ifdef OS_LIB
#define osapi __declspec(dllexport)
#else
#ifdef KERNEL
#define osapi
#else
#define osapi __declspec(dllimport)
#endif
#endif

#ifdef KRNL_LIB
#define krnlapi __declspec(dllexport)
#else
#define krnlapi __declspec(dllimport)
#endif

struct section;

//#define TRACEAPI

//
// Module initialization
//

#define DLL_PROCESS_ATTACH      1
#define DLL_THREAD_ATTACH       2
#define DLL_THREAD_DETACH       3
#define DLL_PROCESS_DETACH      0

//
// Page protection
//

#define PAGESIZE  4096

#define PAGE_NOACCESS           0x01     
#define PAGE_READONLY           0x02     
#define PAGE_READWRITE          0x04     
#define PAGE_EXECUTE            0x10     
#define PAGE_EXECUTE_READ       0x20     
#define PAGE_EXECUTE_READWRITE  0x40     
#define PAGE_GUARD              0x100     

//
// Allocation types
//

#define MEM_COMMIT              0x1000     
#define MEM_RESERVE             0x2000     
#define MEM_DECOMMIT            0x4000     
#define MEM_RELEASE             0x8000     

//
// Thread priorities
//

#define PRIORITY_IDLE            0
#define PRIORITY_LOWEST          1
#define PRIORITY_BELOW_NORMAL    2
#define PRIORITY_NORMAL          3
#define PRIORITY_ABOVE_NORMAL    4
#define PRIORITY_HIGHEST         5
#define PRIORITY_TIME_CRITICAL   6
#define PRIORITY_SYSTEM          7

//
// File open modes
//

#define O_RDONLY                0x0000  // Open for reading only
#define O_WRONLY                0x0001  // Open for writing only
#define O_RDWR                  0x0002  // Open for reading and writing
#define O_APPEND                0x0008  // Writes done at EOF

#define O_SPECIAL               0x0010

#define O_CREAT                 0x0100  // Create and open file
#define O_TRUNC                 0x0200  // Truncate file
#define O_EXCL                  0x0400  // Open only if file doesn't already exist
#define O_DIRECT                0x0800  // Do not use cache for reads and writes

//
// Seek types
//

#define SEEK_SET                0       // Seek relative to begining of file
#define SEEK_CUR                1       // Seek relative to current file position
#define SEEK_END                2       // Seek relative to end of file

//
// Logging
//

#define LOG_LEVEL_MASK          0x0000000F
#define LOG_SUBSYS_MASK         0xFFFFFFF0

#define LOG_ERROR               1
#define LOG_WARNING             2
#define LOG_INFO                3
#define LOG_DEBUG               4
#define LOG_TRACE               5

#define LOG_MODULE              0x80000000
#define LOG_HEAP                0x40000000
#define LOG_APITRACE            0x20000000
#define LOG_AUX                 0x10000000

#ifdef TRACEAPI
#define TRACE(s) syslog(LOG_APITRACE, "%s called\n", s);
#else
#define TRACE(s)
#endif

#ifdef TRACEAPIX
#define TRACEX(s) syslog(LOG_APITRACE, "%s called\n", s);
#else
#define TRACEX(s)
#endif

#define OK    0
#define INFINITE  0xFFFFFFFF

//
// Error codes (from MSVCRT)
//

#define EPERM           1                // Operation not permitted
#define ENOENT          2                // No such file or directory
#define ESRCH           3                // No such process
#define EINTR           4                // Interrupted system call
#define EIO             5                // Input/output error
//#define ENXIO           6                // Device not configured
//#define E2BIG           7                // Argument list too long
#define ENOEXEC         8                // Exec format error
#define EBADF           9                // Bad file number
//#define ECHILD          10               // No spawned processes
#define EAGAIN          11               // Resource temporarily unavailable
#define ENOMEM          12               // Cannot allocate memory
#define EACCES          13               // Access denied
#define EFAULT          14               // Bad address
//#define ENOTBLK           15               // Unknown error
#define EBUSY           16               // Device busy
#define EEXIST          17               // File exist
#define EXDEV           18               // Cross-device link
#define ENODEV          19               // Operation not supported by device
#define ENOTDIR         20               // Not a directory
#define EISDIR          21               // Is a directory
#define EINVAL          22               // Invalid argument
#define ENFILE          23               // Too many open files in system
//#define EMFILE          24               // Too many files open
//#define ENOTTY          25               // Inappropriate ioctl for device
//#define ETXTBSY         26               // Unknown error
//#define EFBIG           27               // File too large
#define ENOSPC          28               // No space left on device
//#define ESPIPE          29               // Illegal seek
//#define EROFS           30               // Read-only file system
//#define EMLINK          31               // Too many links
//#define EPIPE           32               // Broken pipe
//#define EDOM            33               // Numerical arg out of domain
#define ERANGE          34               // Result too large
//#define EUCLEAN           35               // Unknown error
//#define EDEADLK         36               // Resource deadlock avoided
//#define EUNKNOWN         37               // Unknown error
#define ENAMETOOLONG    38               // File name too long
//#define ENOLCK          39               // No locks available
#define ENOSYS          40               // Function not implemented
#define ENOTEMPTY       41               // Directory not empty
//#define EILSEQ          42               // Invalid multibyte sequence

#define	ETIMEOUT	43	         // Timeout
#define EBUF            44               // Buffer error
#define EROUTE          45               // No route
#define ECONN           46               // Not connected
#define ERST            47               // Connection reset
#define EABORT          48               // Connction aborted
#define EUSED           49               // Address in use
#define EPROTONOSUPPORT 50               // Protocol not supported
#define EMSGSIZE        51               // Message too long
#define ECONNREFUSED    52               // Connection refused
#define EHOSTUNREACH    53               // Host unreachable
#define ENETUNREACH     54               // Network unrechable
#define EHOST           55               // Host not found
#define EPROTO          56               // Protocol error
#define ECHKSUM         57               // Checksum error
#define EDESTUNREACH    58               // Destination unreachable

//
// File system
//

#define FS_DIRECTORY            1       // File is a directory
#define FS_BLKDEV               2       // File is a block device
#define FS_STREAMDEV            4       // File is a stream device

#define MAXPATH                 256     // Maximum filename length (including trailing zero)
#define MFSNAMELEN              16      // Length of fs type name

#define PS1                     '\\'    // Primary path separator
#define PS2                     '/'     // Alternate path separator

struct stat
{
  int mode;
  ino_t ino;
  int nlink;
  devno_t devno;
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

struct statfs 
{
  unsigned int bsize;        // Fundamental file system block size
  unsigned int iosize;       // Optimal transfer block size
  unsigned int blocks;       // Total data blocks in file system
  unsigned int bfree;        // Free blocks in fs
  unsigned int files;        // Total file nodes in file system
  unsigned int ffree;        // Free file nodes in fs
  unsigned int cachesize;    // Cache buffers
  char fstype[MFSNAMELEN];   // File system type name
  char mntto[MAXPATH];       // Directory on which mounted
  char mntfrom[MAXPATH];     // Mounted file system
};

struct dirent
{
  ino_t ino;
  unsigned int reclen;
  unsigned int namelen;
  char name[MAXPATH];
};

struct utimbuf 
{
  time_t ctime;
  time_t mtime;
  time_t atime;
};

//
// Serial I/O
//

#define IOCTL_SERIAL_SETCONFIG       1024
#define IOCTL_SERIAL_GETCONFIG       1025
#define IOCTL_SERIAL_WAITEVENT       1026
#define IOCTL_SERIAL_STAT            1027
#define IOCTL_SERIAL_DTR             1028
#define IOCTL_SERIAL_RTS             1029
#define IOCTL_SERIAL_FLUSH_TX_BUFFER 1030
#define IOCTL_SERIAL_FLUSH_RX_BUFFER 1031

#define PARITY_NONE	           0x0
#define PARITY_EVEN	           0x1
#define PARITY_ODD	           0x2
#define PARITY_MARK	           0x3
#define PARITY_SPACE	           0x4

#define LINESTAT_OVERRUN           0x0002
#define LINESTAT_PARITY_ERROR      0x0004
#define LINESTAT_FRAMING_ERROR     0x0008
#define LINESTAT_BREAK             0x0010
#define LINESTAT_OVERFLOW          0x0100

#define	MODEMSTAT_DCD		   0x80
#define	MODEMSTAT_RI		   0x40
#define	MODEMSTAT_DSR		   0x20
#define	MODEMSTAT_CTS		   0x10

struct serial_config
{
  int speed;                             // Baud rate
  int databits;                          // Number of data bits
  int parity;                            // Parity type
  int stopbits;                          // Number of stop bits
  unsigned int rx_timeout;               // Receive timeout
  unsigned int tx_timeout;               // Transmit timeout
};

struct serial_status
{
  int linestatus;                        // Line status
  int modemstatus;                       // Modem status
  int rx_queue_size;                     // Number of bytes in receive queue
  int tx_queue_size;                     // Number of bytes in transmit queue
};

//
// Console I/O
//

#define IOCTL_SET_KEYTIMEOUT     1024
#define IOCTL_CTRLALTDEL_ENABLED 1025
#define IOCTL_SET_KEYMAP         1026
#define IOCTL_KPRINT_ENABLED     1027

//
// Critical sections
//

struct critsect
{
  long count;
  long recursion;
  tid_t owner;
  handle_t event;
};

typedef struct critsect *critsect_t;

//
// Sockets
//

#define IOCTL_SOCKWAIT_RECV      1024

struct in_addr 
{
  union 
  {
    struct { unsigned char s_b1, s_b2, s_b3, s_b4; } s_un_b;
    struct { unsigned short s_w1, s_w2; } s_un_w;
    unsigned long s_addr;
  };
};

struct sockaddr_in
{
  unsigned char sin_len;
  unsigned char sin_family;
  unsigned short sin_port;
  struct in_addr sin_addr;
  char sin_zero[8];
};

struct sockaddr 
{
  unsigned char sa_len;
  unsigned char sa_family;
  char sa_data[14];
};

struct hostent 
{
  char *h_name;        // Official name of host
  char **h_aliases;    // Alias list
  short h_addrtype;    // Host address type
  short h_length;      // Length of address
  char **h_addr_list;  // List of addresses
};

struct protoent
{
  char *p_name;
  char **p_aliases;
  short p_proto;
};

struct servent 
{
  char *s_name;
  char **s_aliases;
  short s_port;
  char *s_proto;
};

#define SOCK_STREAM      1
#define SOCK_DGRAM       2
#define SOCK_RAW         3

#define AF_UNSPEC        0
#define AF_INET          2

#define PF_UNSPEC        AF_UNSPEC
#define PF_INET          AF_INET

#define IPPROTO_IP       0
#define IPPROTO_ICMP     1
#define IPPROTO_TCP      6
#define IPPROTO_UDP      17

#define INADDR_ANY       0
#define INADDR_BROADCAST 0xffffffff
#define INADDR_LOOPBACK  0x7f000001
#define INADDR_NONE      0xffffffff

#define SOL_SOCKET      0xffff

#define SO_SNDTIMEO     0x1005
#define SO_RCVTIMEO     0x1006

__inline unsigned short htons(unsigned short n)
{
  return ((n & 0xFF) << 8) | ((n & 0xFF00) >> 8);
}

__inline unsigned short ntohs(unsigned short n)
{
  return ((n & 0xFF) << 8) | ((n & 0xFF00) >> 8);
}

__inline unsigned long htonl(unsigned long n)
{
  return ((n & 0xFF) << 24) | ((n & 0xFF00) << 8) | ((n & 0xFF0000) >> 8) | ((n & 0xFF000000) >> 24);
}

__inline unsigned long ntohl(unsigned long n)
{
  return ((n & 0xFF) << 24) | ((n & 0xFF00) << 8) | ((n & 0xFF0000) >> 8) | ((n & 0xFF000000) >> 24);
}

//
// Process Environment Block
//

#define PEB_ADDRESS 0x7FFDF000

struct peb
{
  struct moddb *usermods;
  int fast_syscalls_supported;

  char hostname[256];
  struct in_addr ipaddr;
  struct in_addr primary_dns;
  struct in_addr secondary_dns;
  char default_domain[256];
  struct in_addr ntp_server1;
  struct in_addr ntp_server2;
};

//
// Thread Information Block
//

#define TIB_SELF_OFFSET 0x18

#define MAX_TLS           64
#define INVALID_TLS_INDEX 0xFFFFFFFF

#define	MAX_HOST_ALIASES  35
#define	MAX_HOST_ADDRS	  35
#define HOSTBUF_SIZE      1024

struct xcptrec
{
  struct xcptrec *next;
  void (*handler)();
};

struct tib
{
  struct xcptrec *except;          // Exception handler list
  void *stacktop;                  // Topmost address of the threads stack
  void *stacklimit;                // Lowest committed page of the threads stack

  void *subsystemtib;
  unsigned long fiberdata;

  void *arbitrary;
  struct tib *self;                // Virtual address of TIB

  unsigned long unknown1;
  pid_t pid;                       // Process id
  tid_t tid;                       // Thread id
  unsigned long unknown2;

  void *tlsbase;                   // Pointer to TLS array
  struct peb *peb;                 // Process environment block

  void *stackbase;                 // Lowest reserved address of the threads stack
  int errnum;                      // Per thread last error
  void *startaddr;                 // Start address for thread
  void *startarg;                  // Argument to thread start routine

  handle_t in;                     // Thread specific stdin
  handle_t out;                    // Thread specific stdout
  handle_t err;                    // Thread specific stderr

  struct hostent host;             // Per-thread hostent buffer
  unsigned char host_addr[sizeof(struct in_addr)];
  char *h_addr_ptrs[MAX_HOST_ADDRS + 1];
  char *host_aliases[MAX_HOST_ALIASES];
  char hostbuf[HOSTBUF_SIZE];

  char reserved1[2192];

  void *tls[MAX_TLS];              // Thread local storage
  char reserved2[240];
};

#define errno (gettib()->errnum)

// OS API functions

#ifndef KERNEL

osapi int syscall(int syscallno, void *params);

osapi int format(const char *devname, const char *type, const char *opts);
osapi int mount(const char *type, const char *mntto, const char *mntfrom, const char *opts);
osapi int unmount(const char *path);

osapi int getfsstat(struct statfs *buf, size_t size);
osapi int fstatfs(handle_t f, struct statfs *buf);
osapi int statfs(const char *name, struct statfs *buf);

osapi handle_t open(const char *name, int mode);
osapi int close(handle_t h);
osapi int flush(handle_t f);
osapi handle_t dup(handle_t h);

osapi int read(handle_t f, void *data, size_t size);
osapi int write(handle_t f, const void *data, size_t size);
osapi int ioctl(handle_t f, int cmd, const void *data, size_t size);

osapi loff_t tell(handle_t f);
osapi loff_t lseek(handle_t f, loff_t offset, int origin);
osapi int chsize(handle_t f, loff_t size);

osapi int futime(handle_t f, struct utimbuf *times);
osapi int utime(const char *name, struct utimbuf *times);

osapi int fstat(handle_t f, struct stat *buffer);
osapi int stat(const char *name, struct stat *buffer);

osapi int mkdir(const char *name);
osapi int rmdir(const char *name);

osapi int rename(const char *oldname, const char *newname);
osapi int link(const char *oldname, const char *newname);
osapi int unlink(const char *name);

osapi handle_t opendir(const char *name);
osapi int readdir(handle_t f, struct dirent *dirp, int count);

osapi void *mmap(void *addr, unsigned long size, int type, int protect);
osapi int munmap(void *addr, unsigned long size, int type);
osapi void *mremap(void *addr, unsigned long oldsize, unsigned long newsize, int type, int protect);
osapi int mprotect(void *addr, unsigned long size, int protect);
osapi int mlock(void *addr, unsigned long size);
osapi int munlock(void *addr, unsigned long size);

osapi int wait(handle_t h, int timeout);
osapi int waitall(handle_t *h, int count, int timeout);
osapi int waitany(handle_t *h, int count, int timeout);

osapi handle_t mkevent(int manual_reset, int initial_state);
osapi int epulse(handle_t h);
osapi int eset(handle_t h);
osapi int ereset(handle_t h);

osapi handle_t mksem(int initial_count);
osapi int semrel(handle_t h, int count);

osapi handle_t self();
osapi void exit(int status);
osapi void dbgbreak();

osapi handle_t beginthread(void (__stdcall *startaddr)(void *), unsigned stacksize, void *arg, int suspended, tid_t *tid);
osapi int suspend(handle_t thread);
osapi int resume(handle_t thread);
osapi void endthread(int retval);
osapi tid_t gettid();
osapi int setcontext(handle_t thread, void *context);
osapi int getcontext(handle_t thread, void *context);
osapi int getprio(handle_t thread);
osapi int setprio(handle_t thread, int priority);
osapi void sleep(int millisecs);
osapi struct tib *gettib();

osapi time_t time();
osapi int gettimeofday(struct timeval *tv);
osapi int settimeofday(struct timeval *tv);
osapi clock_t clock();

osapi void panic(const char *msg);
osapi void syslog(int priority, const char *fmt,...);
osapi char *canonicalize(const char *filename, char *buffer, int size);

osapi void mkcs(critsect_t cs);
osapi void csfree(critsect_t cs);
osapi void enter(critsect_t cs);
osapi void leave(critsect_t cs);

osapi void *malloc(size_t size);
osapi void *realloc(void *mem, size_t size);
osapi void *calloc(size_t num, size_t size);
osapi void free(void *p);

osapi void *resolve(hmodule_t hmod, const char *procname);
osapi hmodule_t getmodule(char *name);
osapi int getmodpath(hmodule_t hmod, char *buffer, int size);
osapi hmodule_t load(const char *name);
osapi int unload(hmodule_t hmod);
osapi int exec(hmodule_t hmod, char *args);

osapi tls_t tlsalloc();
osapi void tlsfree(tls_t index);
osapi void *tlsget(tls_t index);
osapi void tlsset(tls_t index, void *value);

osapi int accept(int s, struct sockaddr *addr, int *addrlen);
osapi int bind(int s, const struct sockaddr *name, int namelen);
osapi int connect(int s, const struct sockaddr *name, int namelen);
osapi int getpeername(int s, struct sockaddr *name, int *namelen);
osapi int getsockname(int s, struct sockaddr *name, int *namelen);
osapi int getsockopt(int s, int level, int optname, char *optval, int *optlen);
osapi int listen(int s, int backlog);
osapi int recv(int s, void *data, int size, unsigned int flags);
osapi int recvfrom(int s, void *data, int size, unsigned int flags, struct sockaddr *from, int *fromlen);
osapi int send(int s, const void *data, int size, unsigned int flags);
osapi int sendto(int s, const void *data, int size, unsigned int flags, const struct sockaddr *to, int tolen);
osapi int setsockopt(int s, int level, int optname, const char *optval, int optlen);
osapi int shutdown(int s, int how);
osapi int socket(int domain, int type, int protocol);

osapi int res_send(const char *buf, int buflen, char *answer, int anslen);
osapi int res_query(const char *dname, int cls, int type, unsigned char *answer, int anslen);
osapi int res_search(const char *name, int cls, int type, unsigned char *answer, int anslen);
osapi int res_querydomain(const char *name, const char *domain, int cls, int type, unsigned char *answer, int anslen); 
osapi int res_mkquery(int op, const char *dname, int cls, int type, char *data, int datalen, unsigned char *newrr, char *buf, int buflen);
osapi int dn_comp(const char *src, unsigned char *dst, int dstsiz, unsigned char **dnptrs, unsigned char **lastdnptr);
osapi int dn_expand(const unsigned char *msg, const unsigned char *eom, const unsigned char *src,  char *dst, int dstsiz);

osapi struct hostent *gethostbyname(const char *name);
osapi struct hostent *gethostbyaddr(const char *addr, int len, int type);
osapi char *inet_ntoa(struct in_addr in);
osapi unsigned long inet_addr(const char *cp);

osapi int gethostname(char *name, int namelen);
osapi struct protoent *getprotobyname(const char *name);
osapi struct protoent *getprotobynumber(int proto);
osapi struct servent *getservbyname(const char *name, const char *proto);
osapi struct servent *getservbyport(int port, const char *proto);

osapi extern struct section *config;
osapi extern struct peb *peb;
osapi extern unsigned long loglevel;

#endif

#endif
