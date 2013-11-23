//
// os.h
//
// Operating system API
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

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef OS_H
#define OS_H

#ifndef VERSION_H
#include <os/version.h>
#endif VERSION_H

#ifndef SYS_TYPES_H
#include <sys/types.h>
#endif

#ifndef SANOS
#define SANOS
#endif

#ifndef SANOS_VER
#define SANOS_VER (OS_MAJ_VERS * 10000 + OS_MIN_VERS * 100 + OS_RELEASE)
#endif

#if defined(OSLDR)
#define krnlapi
#else
#ifdef KRNL_LIB
#define krnlapi __declspec(dllexport)
#else
#define krnlapi __declspec(dllimport)
#endif
#endif

//
// Basic types
//

#ifndef _TM_DEFINED
#define _TM_DEFINED

struct tm {
  int tm_sec;                   // Seconds after the minute [0, 59]
  int tm_min;                   // Minutes after the hour [0, 59]
  int tm_hour;                  // Hours since midnight [0, 23]
  int tm_mday;                  // Day of the month [1, 31]
  int tm_mon;                   // Months since January [0, 11]
  int tm_year;                  // Years since 1900
  int tm_wday;                  // Days since Sunday [0, 6]
  int tm_yday;                  // Days since January 1 [0, 365]
  int tm_isdst;                 // Daylight Saving Time flag
  int tm_gmtoff;                // Seconds east of UTC
  char *tm_zone;                // Timezone abbreviation
};

#endif

#ifndef _TIMEVAL_DEFINED
#define _TIMEVAL_DEFINED

struct timeval {
  long tv_sec;                  // Seconds
  long tv_usec;                 // Microseconds
};

#endif

struct section;

#define INFINITE  0xFFFFFFFF

#define INTRES(x) ((char *)((unsigned long)((unsigned short)(x))))

//#define TRACEAPI
//#define TRACEAPIX

//
// Module initialization
//

#define DLL_PROCESS_ATTACH      1
#define DLL_THREAD_ATTACH       2
#define DLL_THREAD_DETACH       3
#define DLL_PROCESS_DETACH      0

//
// Flags for dlopen()
//

#ifndef RTLD_LAZY

#define RTLD_LAZY     0x0001
#define RTLD_NOW      0x0002
#define RTLD_GLOBAL   0x0100
#define RTLD_LOCAL    0x0000
#define RTLD_NOSHARE  0x1000
#define RTLD_EXE      0x2000
#define RTLD_SCRIPT   0x4000

#define RTLD_DEFAULT ((void *) 0)

#endif

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

#define MEM_ALIGN64K            0x10000000

//
// Thread priorities
//

#define PRIORITY_SYSIDLE         0
#define PRIORITY_IDLE            1
#define PRIORITY_LOWEST          2
#define PRIORITY_BELOW_NORMAL    7
#define PRIORITY_NORMAL          8
#define PRIORITY_ABOVE_NORMAL    9
#define PRIORITY_HIGHEST         14
#define PRIORITY_TIME_CRITICAL   15
#define PRIORITY_LOW_REALTIME    16
#define PRIORITY_HIGH_REALTIME   31

//
// Thread creation flags
//

#define CREATE_SUSPENDED        0x00000004
#define CREATE_NEW_PROCESS      0x00000010
#define CREATE_DETACHED         0x00000020
#define CREATE_POSIX            0x00000040
#define CREATE_NO_ENV           0x00000080
#define CREATE_CHILD            0x00000100

//
// Spawn flags
//

#define P_WAIT      0        
#define P_NOWAIT    1
#define P_DETACH    4
#define P_SUSPEND   8
#define P_CHILD     16

//
// File open modes
//

#ifndef O_RDONLY

#define O_ACCMODE               0x0003  // Access mode
#define O_RDONLY                0x0000  // Open for reading only
#define O_WRONLY                0x0001  // Open for writing only
#define O_RDWR                  0x0002  // Open for reading and writing
#define O_SPECIAL               0x0004  // Open for special access
#define O_APPEND                0x0008  // Writes done at EOF

#define O_RANDOM                0x0010  // File access is primarily random
#define O_SEQUENTIAL            0x0020  // File access is primarily sequential
#define O_TEMPORARY             0x0040  // Temporary file bit
#define O_NOINHERIT             0x0080  // Child process doesn't inherit file

#define O_CREAT                 0x0100  // Create and open file
#define O_TRUNC                 0x0200  // Truncate file
#define O_EXCL                  0x0400  // Open only if file doesn't already exist
#define O_DIRECT                0x0800  // Do not use cache for reads and writes

#define O_SHORT_LIVED           0x1000  // Temporary storage file, try not to flush
#define O_NONBLOCK              0x2000  // Open in non-blocking mode
#define O_TEXT                  0x4000  // File mode is text (translated)
#define O_BINARY                0x8000  // File mode is binary (untranslated)

#endif

//
// File mode flags (type and permissions)
//

#ifndef S_IFMT

#define S_IFMT         0170000         // File type mask
#define S_IFPKT        0160000         // Packet device
#define S_IFSOCK       0140000         // Socket
#define S_IFLNK        0120000         // Symbolic link
#define S_IFREG        0100000         // Regular file
#define S_IFBLK        0060000         // Block device
#define S_IFDIR        0040000         // Directory
#define S_IFCHR        0020000         // Character device
#define S_IFIFO        0010000         // Pipe

#define S_IREAD        0000400         // Read permission, owner
#define S_IWRITE       0000200         // Write permission, owner
#define S_IEXEC        0000100         // Execute/search permission, owner

#define S_ISLNK(m)      (((m) & S_IFMT) == S_IFLNK)
#define S_ISREG(m)      (((m) & S_IFMT) == S_IFREG)
#define S_ISDIR(m)      (((m) & S_IFMT) == S_IFDIR)
#define S_ISCHR(m)      (((m) & S_IFMT) == S_IFCHR)
#define S_ISBLK(m)      (((m) & S_IFMT) == S_IFBLK)
#define S_ISFIFO(m)     (((m) & S_IFMT) == S_IFIFO)
#define S_ISSOCK(m)     (((m) & S_IFMT) == S_IFSOCK)
#define S_ISPKT(m)      (((m) & S_IFMT) == S_IFPKT)

#define S_IRWXU 00700
#define S_IRUSR 00400
#define S_IWUSR 00200
#define S_IXUSR 00100

#define S_IRWXG 00070
#define S_IRGRP 00040
#define S_IWGRP 00020
#define S_IXGRP 00010

#define S_IRWXO 00007
#define S_IROTH 00004
#define S_IWOTH 00002
#define S_IXOTH 00001

#define S_IRWXUGO 00777

#endif

//
// Sharing mode flags
//

#ifndef SH_COMPAT

#define SH_COMPAT               0x0000
#define SH_DENYRW               0x0010  // Denies read and write access to file
#define SH_DENYWR               0x0020  // Denies write access to file
#define SH_DENYRD               0x0030  // Denies read access to file
#define SH_DENYNO               0x0040  // Permits read and write access

#endif

//
// Seek types
//

#ifndef SEEK_SET

#define SEEK_SET                0       // Seek relative to begining of file
#define SEEK_CUR                1       // Seek relative to current file position
#define SEEK_END                2       // Seek relative to end of file

#endif

//
// Values for the second argument to access
//

#ifndef R_OK

#define R_OK    4               // Test for read permission
#define W_OK    2               // Test for write permission
#define X_OK    1               // Test for execute permission
#define F_OK    0               // Test for existence

#endif

//
// Logging
//

#ifndef LOG_EMERG

#define LOG_EMERG       0
#define LOG_ALERT       1
#define LOG_CRIT        2
#define LOG_ERR         3
#define LOG_WARNING     4
#define LOG_NOTICE      5
#define LOG_INFO        6
#define LOG_DEBUG       7
#define LOG_TRACE       8

#endif

#ifndef LOG_KERN

#define LOG_KERN        (0<<3)
#define LOG_USER        (1<<3)
#define LOG_MAIL        (2<<3)
#define LOG_DAEMON      (3<<3)
#define LOG_AUTH        (4<<3)
#define LOG_SYSLOG      (5<<3)
#define LOG_LPR         (6<<3)
#define LOG_NEWS        (7<<3)
#define LOG_UUCP        (8<<3)
#define LOG_CRON        (9<<3)
#define LOG_AUTHPRIV    (10<<3)
#define LOG_FTP         (11<<3)

#define LOG_LOCAL0      (16<<3)
#define LOG_LOCAL1      (17<<3)
#define LOG_LOCAL2      (18<<3)
#define LOG_LOCAL3      (19<<3)
#define LOG_LOCAL4      (20<<3)
#define LOG_LOCAL5      (21<<3)
#define LOG_LOCAL6      (22<<3)
#define LOG_LOCAL7      (23<<3)

#endif

#define LOG_AUX                 (12<<3)
#define LOG_MODULE              (13<<3)
#define LOG_HEAP                (14<<3)
#define LOG_APITRACE            (15<<3)

#ifdef TRACEAPI
#define TRACE(s) syslog(LOG_APITRACE, "%s called", s);
#else
#define TRACE(s)
#endif

#ifdef TRACEAPIX
#define TRACEX(s) syslog(LOG_APITRACE, "%s called", s);
#else
#define TRACEX(s)
#endif

//
// Standard C runtime libarry error codes
//

#ifndef _ERRORS_DEFINED
#define _ERRORS_DEFINED

#define EPERM           1                // Operation not permitted
#define ENOENT          2                // No such file or directory
#define ESRCH           3                // No such process
#define EINTR           4                // Interrupted system call
#define EIO             5                // Input/output error
#define ENXIO           6                // Device not configured
#define E2BIG           7                // Argument list too long
#define ENOEXEC         8                // Exec format error
#define EBADF           9                // Bad file number
#define ECHILD          10               // No spawned processes
#define EAGAIN          11               // Resource temporarily unavailable
#define ENOMEM          12               // Cannot allocate memory
#define EACCES          13               // Access denied
#define EFAULT          14               // Bad address
#define ENOTBLK         15               // Not a block device
#define EBUSY           16               // Device busy
#define EEXIST          17               // File exist
#define EXDEV           18               // Cross-device link
#define ENODEV          19               // Operation not supported by device
#define ENOTDIR         20               // Not a directory
#define EISDIR          21               // Is a directory
#define EINVAL          22               // Invalid argument
#define ENFILE          23               // Too many open files in system
#define EMFILE          24               // Too many files open
#define ENOTTY          25               // Inappropriate ioctl for device
#define ETXTBSY         26               // Unknown error
#define EFBIG           27               // File too large
#define ENOSPC          28               // No space left on device
#define ESPIPE          29               // Illegal seek
#define EROFS           30               // Read-only file system
//#define EMLINK          31               // Too many links
#define EPIPE           32               // Broken pipe
#define EDOM            33               // Numerical arg out of domain
#define ERANGE          34               // Result too large
#define EUCLEAN           35               // Structure needs cleaning
#define EDEADLK         36               // Resource deadlock avoided
#define EUNKNOWN        37               // Unknown error
#define ENAMETOOLONG    38               // File name too long
//#define ENOLCK          39               // No locks available
#define ENOSYS          40               // Function not implemented
#define ENOTEMPTY       41               // Directory not empty
//#define EILSEQ          42               // Invalid multibyte sequence

//
// Sockets errors
//

#define EWOULDBLOCK     45               // Operation would block
#define EINPROGRESS     46               // Operation now in progress
#define EALREADY        47               // Operation already in progress
#define ENOTSOCK        48               // Socket operation on nonsocket
#define EDESTADDRREQ    49               // Destination address required
#define EMSGSIZE        50               // Message too long
#define EPROTOTYPE      51               // Protocol wrong type for socket
#define ENOPROTOOPT     52               // Bad protocol option
#define EPROTONOSUPPORT 53               // Protocol not supported
#define ESOCKTNOSUPPORT 54               // Socket type not supported
#define EOPNOTSUPP      55               // Operation not supported
#define EPFNOSUPPORT    56               // Protocol family not supported
#define EAFNOSUPPORT    57               // Address family not supported 
#define EADDRINUSE      58               // Address already in use
#define EADDRNOTAVAIL   59               // Cannot assign requested address
#define ENETDOWN        60               // Network is down
#define ENETUNREACH     61               // Network is unreachable
#define ENETRESET       62               // Network dropped connection on reset
#define ECONNABORTED    63               // Connection aborted
#define ECONNRESET      64               // Connection reset by peer
#define ENOBUFS         65               // No buffer space available
#define EISCONN         66               // Socket is already connected
#define ENOTCONN        67               // Socket is not connected
#define ESHUTDOWN       68               // Cannot send after socket shutdown
#define ETOOMANYREFS    69               // Too many references
#define ETIMEDOUT       70               // Operation timed out
#define ECONNREFUSED    71               // Connection refused
#define ELOOP           72               // Cannot translate name
#define EWSNAMETOOLONG  73               // Name component or name was too long
#define EHOSTDOWN       74               // Host is down
#define EHOSTUNREACH    75               // No route to host
#define EWSNOTEMPTY     76               // Cannot remove a directory that is not empty
#define EPROCLIM        77               // Too many processes
#define EUSERS          78               // Ran out of quota
#define EDQUOT          79               // Ran out of disk quota
#define ESTALE          80               // File handle reference is no longer available
#define EREMOTE         81               // Item is not available locally

//
// Resolver errors
//

#define EHOSTNOTFOUND   82               // Host not found
#define ETRYAGAIN       83               // Nonauthoritative host not found
#define ENORECOVERY     84               // A nonrecoverable error occured
#define ENODATA         85               // Valid name, no data record of requested type

//
// Misc. error codes
//

#define EPROTO          86               // Protocol error
#define ECHKSUM         87               // Checksum error
#define EBADSLT         88               // Invalid slot
#define EREMOTEIO       89               // Remote I/O error

//
// Error code aliases
//

#define ETIMEOUT        ETIMEDOUT
#define EBUF            ENOBUFS
#define EROUTE          ENETUNREACH
#define ECONN           ENOTCONN
#define ERST            ECONNRESET
#define EABORT          ECONNABORTED

#endif

//
// Signals
//
// Some of these signals are not used in sanos, but are included
// for completeness.
//

#ifndef _NSIG

#define SIGHUP          1       // Hangup (POSIX)
#define SIGINT          2       // Interrupt (ANSI)
#define SIGQUIT         3       // Quit (POSIX)
#define SIGILL          4       // Illegal instruction (ANSI)
#define SIGTRAP         5       // Trace trap (POSIX)
#define SIGABRT         6       // Abort (ANSI)
#define SIGBUS          7       // BUS error (4.2 BSD)
#define SIGFPE          8       // Floating-point exception (ANSI)
#define SIGKILL         9       // Kill, unblockable (POSIX)
#define SIGUSR1         10      // User-defined signal 1 (POSIX)
#define SIGSEGV         11      // Segmentation violation (ANSI)
#define SIGUSR2         12      // User-defined signal 2 (POSIX)
#define SIGPIPE         13      // Broken pipe (POSIX)
#define SIGALRM         14      // Alarm clock (POSIX)
#define SIGTERM         15      // Termination (ANSI)
#define SIGSTKFLT       16      // Stack fault
#define SIGCHLD         17      // Child status has changed (POSIX)
#define SIGCONT         18      // Continue (POSIX)
#define SIGSTOP         19      // Stop, unblockable (POSIX)
#define SIGTSTP         20      // Keyboard stop (POSIX)
#define SIGTTIN         21      // Background read from tty (POSIX)
#define SIGTTOU         22      // Background write to tty (POSIX)
#define SIGURG          23      // Urgent condition on socket (4.2 BSD)
#define SIGXCPU         24      // CPU limit exceeded (4.2 BSD)
#define SIGXFSZ         25      // File size limit exceeded (4.2 BSD)
#define SIGVTALRM       26      // Virtual alarm clock (4.2 BSD)
#define SIGPROF         27      // Profiling alarm clock (4.2 BSD)
#define SIGWINCH        28      // Window size change (4.3 BSD, Sun)
#define SIGIO           29      // I/O now possible (4.2 BSD)
#define SIGPWR          30      // Power failure restart (System V)
#define SIGSYS          31      // Bad system call

#define _NSIG 32

#endif

#ifndef _CONTEXT_DEFINED
#define _CONTEXT_DEFINED

struct context {
  unsigned long es, ds;
  unsigned long edi, esi, ebp, ebx, edx, ecx, eax;
  unsigned long traptype;
  unsigned long errcode;

  unsigned long eip, ecs;
  unsigned long eflags;
  unsigned long esp, ess;
};

#endif

#ifndef _SIGHANDLER_T_DEFINED
#define _SIGHANDLER_T_DEFINED
typedef void (*sighandler_t)(int signum);
#endif

#ifndef SIG_DFL

#define SIG_DFL ((sighandler_t) 0)      // Default signal action
#define SIG_IGN ((sighandler_t) 1)      // Ignore signal
#define SIG_ERR ((sighandler_t) -1)     // Signal error value

#endif

#ifndef _SIGINFO_T_DEFINED
#define _SIGINFO_T_DEFINED

struct siginfo  {
  int si_signo;
  int si_code;

  struct context *si_ctxt;
  void *si_addr;
};

typedef struct siginfo siginfo_t;

#endif

#ifndef _SIGACTION_DEFINED
#define _SIGACTION_DEFINED

struct sigaction {
  union {
    void (*sa_handler)(int signum);
    void (*sa_sigaction)(int signum, siginfo_t *info, void *context);
  };
  sigset_t sa_mask;
  int sa_flags;
};

#endif

//
// Bits in sa_flags
//

#ifndef SA_NOCLDSTOP

#define SA_NOCLDSTOP 0x0001
#define SA_ONSTACK   0x0002
#define SA_RESETHAND 0x0004
#define SA_RESTART   0x0008
#define SA_SIGINFO   0x0010
#define SA_NOCLDWAIT 0x0020
#define SA_NODEFER   0x0030

#endif

//
// Values for sigprocmask
//

#ifndef SIG_BLOCK

#define SIG_BLOCK   1   // Block signals in 'set', other signals unaffected
#define SIG_UNBLOCK 2   // Unblock signals in 'set'
#define SIG_SETMASK 3   // New mask is 'set'

#endif

//
// Critical sections
//

struct critsect {
  long count;
  long recursion;
  tid_t owner;
  handle_t event;
};

typedef struct critsect *critsect_t;

//
// File system
//

#define MAXPATH                 256     // Maximum filename length (including trailing zero)
#define MFSNAMELEN              16      // Length of fs type name

#define PS1                     '/'     // Primary path separator
#define PS2                     '\\'    // Alternate path separator

#define SH_FLAGS(flags)                 (((flags) >> 16) & 0x0F)
#define FILE_FLAGS(flags, shflags)      (((flags) & 0xFFFF) | (((shflags) & 0x0F) << 16))

#ifndef _STAT_DEFINED
#define _STAT_DEFINED

struct stat {
  dev_t st_dev;
  ino_t st_ino;
  mode_t st_mode;
  nlink_t st_nlink;
  uid_t st_uid;
  gid_t st_gid;
  dev_t st_rdev;
  loff_t st_size;
  time_t st_atime;
  time_t st_mtime;
  time_t st_ctime;
};

struct stat64 {
  dev_t st_dev;
  ino_t st_ino;
  mode_t st_mode;
  nlink_t st_nlink;
  uid_t st_uid;
  gid_t st_gid;
  dev_t st_rdev;
  off64_t st_size;
  time_t st_atime;
  time_t st_mtime;
  time_t st_ctime;
};

#endif

struct statfs  {
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

struct direntry {
  ino_t ino;
  unsigned int reclen;
  unsigned int namelen;
  char name[MAXPATH];
};

#ifndef _UTIMBUF_DEFINED
#define _UTIMBUF_DEFINED

struct utimbuf  {
  time_t modtime;
  time_t actime;
  time_t ctime;
};

#endif

#ifndef _IOVEC_DEFINED
#define _IOVEC_DEFINED

struct iovec { 
  size_t iov_len;
  void *iov_base;
};

#endif

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

#define PARITY_NONE                0x0
#define PARITY_EVEN                0x1
#define PARITY_ODD                 0x2
#define PARITY_MARK                0x3
#define PARITY_SPACE               0x4

#define LINESTAT_OVERRUN           0x0002
#define LINESTAT_PARITY_ERROR      0x0004
#define LINESTAT_FRAMING_ERROR     0x0008
#define LINESTAT_BREAK             0x0010
#define LINESTAT_OVERFLOW          0x0100

#define MODEMSTAT_DCD              0x80
#define MODEMSTAT_RI               0x40
#define MODEMSTAT_DSR              0x20
#define MODEMSTAT_CTS              0x10

struct serial_config {
  int speed;                             // Baud rate
  int databits;                          // Number of data bits
  int parity;                            // Parity type
  int stopbits;                          // Number of stop bits
  unsigned int rx_timeout;               // Receive timeout
  unsigned int tx_timeout;               // Transmit timeout
};

struct serial_status {
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
#define IOCTL_BEEP               1027
#define IOCTL_SOUND              1028
#define IOCTL_REBOOT             1029
#define IOCTL_KBHIT              1030

#define IOCTL_KPRINT_ENABLED     1031
#define IOCTL_KLOG_WAIT          1032
#define IOCTL_SET_TTY            1033
#define IOCTL_GET_TTY            1034

//
// I/O control codes
//
//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//  +---+-------------+-------------+---------------+---------------+
//  |dir|             |   paramlen  |  category     |   function    |
//  +---+-------------+-------------+---------------+---------------+

#define IOCPARM_MASK    0x7F            // Parameters must be < 128 bytes
#define IOC_VOID        0x20000000      // No parameters
#define IOC_OUT         0x40000000      // Copy out parameters
#define IOC_IN          0x80000000      // Copy in parameters
#define IOC_INOUT       (IOC_IN | IOC_OUT)

#define _IOC(x, y)       (IOC_VOID | ((x) << 8) | (y))
#define _IOCR(x, y, t)   (IOC_OUT | (((long) sizeof(t) & IOCPARM_MASK) << 16) | ((x) << 8) | (y))
#define _IOCW(x, y, t)   (IOC_IN |(((long) sizeof(t) & IOCPARM_MASK) << 16) | ((x) << 8)| (y))
#define _IOCRW(x, y, t)  (IOC_INOUT |(((long) sizeof(t) & IOCPARM_MASK) << 16) | ((x) << 8)| (y))

//
// Sockets
//

#define FIONREAD      _IOCR('f', 127, unsigned long)    // Get # bytes to read
#define FIONBIO       _IOCW('f', 126, unsigned long)    // Set/clear non-blocking i/o

#define SIOWAITRECV   _IOCW('s', 120, unsigned long)

#define SIOIFLIST     _IOCRW('i', 20, void *)           // Get netif list
#define SIOIFCFG      _IOCRW('i', 21, void *)           // Configure netif

#ifndef _IN_ADDR_DEFINED
#define _IN_ADDR_DEFINED

struct in_addr {
  union {
    struct { unsigned char s_b1, s_b2, s_b3, s_b4; } s_un_b;
    struct { unsigned short s_w1, s_w2; } s_un_w;
    unsigned long s_addr;
  };
};

#endif

#ifndef _SOCKADDR_IN_DEFINED
#define _SOCKADDR_IN_DEFINED

struct sockaddr_in {
  unsigned short sin_family;
  unsigned short sin_port;
  struct in_addr sin_addr;
  char sin_zero[8];
};

#endif

#ifndef _SOCKADDR_DEFINED
#define _SOCKADDR_DEFINED

struct sockaddr {
  unsigned short sa_family;
  char sa_data[14];
};

#endif

#define IFCFG_UP         1
#define IFCFG_DHCP       2
#define IFCFG_DEFAULT    4
#define IFCFG_LOOPBACK   8

#define NET_NAME_MAX 16

struct ifcfg {
  char name[NET_NAME_MAX];
  int flags;
  char hwaddr[12];
  struct sockaddr addr;
  struct sockaddr gw;
  struct sockaddr netmask;
  struct sockaddr broadcast;
};

#ifndef _LINGER_DEFINED
#define _LINGER_DEFINED

struct linger {
  unsigned short l_onoff;
  unsigned short l_linger;
};

#endif

#ifndef _MSGHDR_DEFINED
#define _MSGHDR_DEFINED

struct msghdr {
  struct sockaddr *msg_name;
  int msg_namelen;
  struct iovec *msg_iov;
  int msg_iovlen;
};

#endif

#ifndef _HOSTENT_DEFINED
#define _HOSTENT_DEFINED

struct hostent {
  char *h_name;        // Official name of host
  char **h_aliases;    // Alias list
  short h_addrtype;    // Host address type
  short h_length;      // Length of address
  char **h_addr_list;  // List of addresses
};

#define h_addr h_addr_list[0]

#endif

#ifndef _PROTOENT_DEFINED
#define _PROTOENT_DEFINED

struct protoent {
  char *p_name;
  char **p_aliases;
  short p_proto;
};

#endif

#ifndef _SERVENT_DEFINED
#define _SERVENT_DEFINED

struct servent {
  char *s_name;
  char **s_aliases;
  short s_port;
  char *s_proto;
};

#endif

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

#define SO_REUSEADDR    0x0004
#define SO_KEEPALIVE    0x0008
#define SO_BROADCAST    0x0020
#define SO_SNDTIMEO     0x1005
#define SO_RCVTIMEO     0x1006
#define SO_LINGER       0x0080

#define SO_DONTLINGER   ((unsigned int) (~SO_LINGER))

#define TCP_NODELAY     0x0001

#define SHUT_RD         0x00
#define SHUT_WR         0x01
#define SHUT_RDWR       0x02

#ifndef _XTONX_DEFINED
#define _XTONX_DEFINED

__inline unsigned short htons(unsigned short n) {
  return ((n & 0xFF) << 8) | ((n & 0xFF00) >> 8);
}

__inline unsigned short ntohs(unsigned short n) {
  return ((n & 0xFF) << 8) | ((n & 0xFF00) >> 8);
}

__inline unsigned long htonl(unsigned long n) {
  return ((n & 0xFF) << 24) | ((n & 0xFF00) << 8) | ((n & 0xFF0000) >> 8) | ((n & 0xFF000000) >> 24);
}

__inline unsigned long ntohl(unsigned long n) {
  return ((n & 0xFF) << 24) | ((n & 0xFF00) << 8) | ((n & 0xFF0000) >> 8) | ((n & 0xFF000000) >> 24);
}

#endif

#ifndef FD_SETSIZE
#define FD_SETSIZE 64
#endif

#ifndef _FD_SET_DEFINED
#define _FD_SET_DEFINED

typedef struct fd_set {
  unsigned int count;
  int fd[FD_SETSIZE];
} fd_set;

#endif

#define FD_ZERO(set) _fd_zero(set)
#define FD_ISSET(fd, set) _fd_isset(fd, set)
#define FD_SET(fd, set) _fd_set(fd, set)
#define FD_CLR(fd, set) _fd_clr(fd, set)

#ifndef _FD_FUNCS_DEFINED
#define _FD_FUNCS_DEFINED

__inline void _fd_zero(fd_set *set) {
  set->count = 0;
}

__inline int _fd_isset(int fd, fd_set *set) {
  unsigned int i;

  for (i = 0; i < set->count; i++) if (set->fd[i] == fd) return 1;
  return 0;
}

__inline void _fd_set(int fd, fd_set *set) {
  if (set->count < FD_SETSIZE) set->fd[set->count++] = fd;
}

__inline void _fd_clr(int fd, fd_set *set) {
  unsigned int i;

  for (i = 0; i < set->count ; i++) {
    if (set->fd[i] == fd) {
      while (i < set->count - 1) {
        set->fd[i] = set->fd[i + 1];
        i++;
      }
      set->count--;
      break;
    }
  }
}

#endif

//
// Poll
//

#ifndef _POLL_FD_DEFINED
#define _POLL_FD_DEFINED

struct pollfd {
  int fd;                     // File descriptor
  short events;               // Requested events
  short revents;              // Returned events
};

#define POLLIN      0x0001    // Data may be read without blocking
#define POLLPRI     0x0002    // High priority data may be read without blocking
#define POLLOUT     0x0004    // Data may be written without blocking
#define POLLERR     0x0008    // An error has occurred (revents only)
#define POLLHUP     0x0010    // Device has been disconnected (revents only)
#define POLLNVAL    0x0020    // Invalid fd member (revents only)

#endif

//
// I/O multiplexer events
//

#define IOEVT_READ     0x0001
#define IOEVT_WRITE    0x0002
#define IOEVT_ERROR    0x0004
#define IOEVT_ACCEPT   0x0008
#define IOEVT_CONNECT  0x0010
#define IOEVT_CLOSE    0x0020

//
// Module version info
//

#pragma pack(push, 1)

struct verinfo {
  unsigned long signature;
  unsigned long format;

  unsigned short file_minor_version;
  unsigned short file_major_version;
  unsigned short file_build_number;
  unsigned short file_release_number;

  unsigned short product_minor_version;
  unsigned short product_major_version;
  unsigned short product_build_number;
  unsigned short product_release_number;

  unsigned long file_flags_mask;
  unsigned long file_flags;

  unsigned long file_os;
  unsigned long file_type;
  unsigned long file_subtype;
  unsigned __int64 file_date;
};

#pragma pack(pop)

//
// User database
//

#define NGROUPS_MAX 8

#ifndef _PASSWD_DEFINED
#define _PASSWD_DEFINED

struct passwd {
  char *pw_name;    // User name
  char *pw_passwd;  // User password
  uid_t pw_uid;     // User id
  gid_t pw_gid;     // Group id
  char *pw_gecos;   // Real name
  char *pw_dir;     // Home directory
  char *pw_shell;   // Shell program
};

#endif

#ifndef _GROUP_DEFINED
#define _GROUP_DEFINED

struct group {
  char *gr_name;    // Group name
  char *gr_passwd;  // Group password
  gid_t gr_gid;     // Group id
  char **gr_mem;    // Group members
};

#endif

//
// Process Environment Block
//

struct process;
struct heap;

#define PEB_ADDRESS 0x7FFDF000

struct peb {
  struct moddb *usermods;
  int fast_syscalls_supported;

  char hostname[256];
  struct in_addr ipaddr;
  struct in_addr primary_dns;
  struct in_addr secondary_dns;
  char default_domain[256];
  struct in_addr ntp_server1;
  struct in_addr ntp_server2;

  void (*globalhandler)(struct siginfo *);
  int debug;
  int rcdone;
  int umaskval;
  int fmodeval;
  char pathsep;

  struct process *firstproc;
  struct process *lastproc;

  char osname[16];
  time_t ostimestamp;
  struct verinfo osversion;
  struct heap *heap;
};

//
// Process Object
//

#define TERM_UNKNOWN   0
#define TERM_CONSOLE   1
#define TERM_VT100     2

#define CRTBASESIZE    (16 + 3 * 32 + 512 + 7 * 4 + 4)

struct term {
  int type;
  int cols;
  int lines;
  int ttyin;
  int ttyout;
};

struct zombie {
  pid_t pid;
  int status;
  struct zombie *next;
};

struct process {
  int id;                           // Process id
  int threadcnt;                    // Number of threads in process
  struct process *parent;           // Parent process
  struct process *nextproc;         // Next process in global process list
  struct process *prevproc;         // Previous process in global process list
  hmodule_t hmod;                   // Module handle for exec module
  char *cmdline;                    // Command line arguments
  char **env;                       // Environment variables

  handle_t hndl;                    // Handle for main thread in process
  void (*atexit)(int, int);         // Exit handler (used by libc)

  handle_t iob[3];                  // Standard input, output, and error handle
  struct term *term;                // Terminal type
  struct heap *heap;                // Local heap

  struct sigaction handlers[_NSIG]; // Signal handlers
  struct zombie *zombies;           // List of terminated childs

  char *ident;                      // Process identifier for syslog
  int facility;                     // Default facility for syslog

  char crtbase[CRTBASESIZE];        // Used by C runtime library 
};

//
// Thread Information Block
//

#define TIB_SELF_OFFSET 0x18

#define MAX_TLS           64
#define INVALID_TLS_INDEX 0xFFFFFFFF

#define MAX_HOST_ALIASES  35
#define MAX_HOST_ADDRS    35
#define HOSTBUF_SIZE      1024

#define CVTBUFSIZE        (309 + 43)
#define ASCBUFSIZE        (26 + 2)
#define CRYPTBUFSIZE      14

struct xcptrec {
  struct xcptrec *next;
  void (*handler)();
};

struct tib {
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
  int flags;                       // Thread creation flags

  handle_t hndl;                   // Handle for thread
  struct process *proc;            // Process object for thread

  struct hostent host;             // Per-thread hostent buffer
  unsigned char host_addr[sizeof(struct in_addr)];
  char *h_addr_ptrs[MAX_HOST_ADDRS + 1];
  char *host_aliases[MAX_HOST_ALIASES];
  char hostbuf[HOSTBUF_SIZE];

  struct tm tmbuf;                 // For gmtime() and localtime()
  char *nexttoken;                 // For strtok()
  char cvtbuf[CVTBUFSIZE];         // For ecvt() and fcvt()
  char ascbuf[ASCBUFSIZE];         // For asctime()
  char tmpnambuf[MAXPATH];         // For tmpnam()
  char cryptbuf[CRYPTBUFSIZE];     // For crypt()
  void *forkctx;                   // For vfork()

  char reserved1[1486];

  void *tls[MAX_TLS];              // Thread local storage
  char reserved2[240];
};

#define fdin  __getstdhndl(0)
#define fdout __getstdhndl(1)
#define fderr __getstdhndl(2)

//
// SVID2/XPG mallinfo structure
//

struct mallinfo {
  int arena;    // Non-mmapped space allocated from system
  int ordblks;  // Number of free chunks
  int smblks;   // Number of fastbin blocks
  int hblks;    // Number of mmapped regions
  int hblkhd;   // Space in mmapped regions
  int usmblks;  // Maximum total allocated space
  int fsmblks;  // Space available in freed fastbin blocks
  int uordblks; // Total allocated space
  int fordblks; // Total free space
  int keepcost; // Top-most, releasable (via malloc_trim) space
};

//
// sysinfo
//

#define SYSINFO_CPU  0
#define SYSINFO_MEM  1
#define SYSINFO_LOAD 2

struct cpuinfo {
  int cpu_vendor;
  int cpu_family;
  int cpu_model;
  int cpu_stepping;
  int cpu_mhz;
  unsigned long cpu_features;
  int pagesize;
  char vendorid[16];
  char modelid[64];
};

struct meminfo {
  unsigned int physmem_total;
  unsigned int physmem_avail;
  unsigned int virtmem_total;
  unsigned int virtmem_avail;
  unsigned int pagesize;
};

struct loadinfo {
  long uptime;
  int load_user;
  int load_system;
  int load_intr;
  int load_idle;
};

#define UTSNAMELEN 65

#ifndef _UTSNAME_DEFINED
#define _UTSNAME_DEFINED
struct utsname {
  char sysname[UTSNAMELEN];
  char nodename[UTSNAMELEN];
  char release[UTSNAMELEN];
  char version[UTSNAMELEN];
  char machine[UTSNAMELEN];
};
#endif

//
// OS exit mode
//

#define EXITOS_HALT      0
#define EXITOS_POWEROFF  1
#define EXITOS_REBOOT    2
#define EXITOS_DEBUG     3

//
// OS API functions
//

#ifndef KERNEL

#ifdef  __cplusplus
extern "C" {
#endif

osapi int syscall(int syscallno, void *params);

osapi int mkfs(const char *devname, const char *type, const char *opts);
osapi int mount(const char *type, const char *mntto, const char *mntfrom, const char *opts);
osapi int umount(const char *path);

osapi int getfsstat(struct statfs *buf, size_t size);
osapi int fstatfs(handle_t f, struct statfs *buf);
osapi int statfs(const char *name, struct statfs *buf);

osapi handle_t open(const char *name, int flags, ...);
osapi handle_t sopen(const char *name, int flags, int shflags, ...);
osapi handle_t creat(const char *name, int mode);
osapi int close(handle_t h);
osapi int fsync(handle_t f);
osapi handle_t dup(handle_t h);
osapi handle_t dup2(handle_t h1, handle_t h2);
osapi int read(handle_t f, void *data, size_t size);
osapi int write(handle_t f, const void *data, size_t size);
osapi int pread(handle_t f, void *data, size_t size, off64_t offset);
osapi int pwrite(handle_t f, const void *data, size_t size, off64_t offset);
osapi int ioctl(handle_t f, int cmd, const void *data, size_t size);
osapi int readv(handle_t f, const struct iovec *iov, int count);
osapi int writev(handle_t f, const struct iovec *iov, int count);
osapi loff_t tell(handle_t f);
osapi off64_t tell64(handle_t f);
osapi loff_t lseek(handle_t f, loff_t offset, int origin);
osapi off64_t lseek64(handle_t f, off64_t offset, int origin);
osapi int ftruncate(handle_t f, loff_t size);
osapi int ftruncate64(handle_t f, off64_t size);
osapi int futime(handle_t f, struct utimbuf *times);
osapi int utime(const char *name, struct utimbuf *times);
osapi int fstat(handle_t f, struct stat *buffer);
osapi int fstat64(handle_t f, struct stat64 *buffer);
osapi int stat(const char *name, struct stat *buffer);
osapi int stat64(const char *name, struct stat64 *buffer);
osapi int lstat(const char *name, struct stat *buffer);
osapi int lstat64(const char *name, struct stat64 *buffer);
osapi int access(const char *name, int mode);
osapi loff_t filelength(handle_t f);
osapi off64_t filelength64(handle_t f);
osapi int isatty(handle_t f);

osapi int eof(handle_t f);
osapi int umask(int mask);
osapi int setmode(handle_t f, int mode);

osapi int chmod(const char *name, int mode);
osapi int fchmod(handle_t f, int mode);
osapi int chown(const char *name, int owner, int group);
osapi int fchown(handle_t f, int owner, int group);

osapi int chdir(const char *name);
osapi char *getcwd(char *buf, size_t size);

osapi int mkdir(const char *name, int mode);
osapi int rmdir(const char *name);

osapi int rename(const char *oldname, const char *newname);
osapi int link(const char *oldname, const char *newname);
osapi int unlink(const char *name);

osapi handle_t _opendir(const char *name);
osapi int _readdir(handle_t f, struct direntry *dirp, int count);

osapi int pipe(handle_t fildes[2]);

osapi void *vmalloc(void *addr, unsigned long size, int type, int protect, unsigned long tag);
osapi int vmfree(void *addr, unsigned long size, int type);
osapi void *vmrealloc(void *addr, unsigned long oldsize, unsigned long newsize, int type, int protect, unsigned long tag);
osapi int vmprotect(void *addr, unsigned long size, int protect);
osapi int vmlock(void *addr, unsigned long size);
osapi int vmunlock(void *addr, unsigned long size);
osapi void *vmmap(void *addr, unsigned long size, int protect, handle_t h, off64_t offset);
osapi int vmsync(void *addr, unsigned long size);

osapi int waitone(handle_t h, int timeout);
osapi int waitall(handle_t *h, int count, int timeout);
osapi int waitany(handle_t *h, int count, int timeout);

osapi handle_t mkevent(int manual_reset, int initial_state);
osapi int epulse(handle_t h);
osapi int eset(handle_t h);
osapi int ereset(handle_t h);

osapi handle_t mksem(int initial_count);
osapi int semrel(handle_t h, int count);

osapi handle_t mkmutex(int owned);
osapi int mutexrel(handle_t h);

osapi handle_t mkiomux(int flags);
osapi int dispatch(handle_t iomux, handle_t h, int events, int context);
osapi int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, const struct timeval *timeout);
osapi int poll(struct pollfd fds[], unsigned int nfds, int timeout);

osapi int sysinfo(int cmd, void *data, size_t size);
osapi int uname(struct utsname *buf);
osapi handle_t self();
osapi void exitos(int mode);
osapi void dbgbreak();
osapi char *strerror(int errnum);

osapi char *crypt(const char *key, const char *salt);
osapi char *crypt_r(const char *key, const char *salt, char *buf);

osapi struct passwd *getpwnam(const char *name);
osapi struct passwd *getpwuid(uid_t uid);
osapi struct group *getgrnam(const char *name);
osapi struct group *getgrgid(uid_t uid);
osapi int initgroups(const char *user, gid_t basegid);

osapi int getuid();
osapi int getgid();
osapi int setuid(uid_t uid);
osapi int setgid(gid_t gid);
osapi int geteuid();
osapi int getegid();
osapi int seteuid(uid_t uid);
osapi int setegid(gid_t gid);
osapi int getgroups(int size, gid_t list[]);
osapi int setgroups(int size, const gid_t list[]);

osapi handle_t beginthread(void (__stdcall *startaddr)(void *), unsigned int stacksize, void *arg, int flags, char *name, struct tib **ptib);
osapi int suspend(handle_t thread);
osapi int resume(handle_t thread);
osapi struct tib *getthreadblock(handle_t thread);
osapi handle_t getprochandle(pid_t pid);
osapi void endthread(int status);
osapi tid_t gettid();
osapi pid_t getpid();
osapi pid_t getppid();
osapi int getchildstat(pid_t pid, int *status);
osapi int setchildstat(pid_t pid, int status);
osapi int setcontext(handle_t thread, void *context);
osapi int getcontext(handle_t thread, void *context);
osapi int getprio(handle_t thread);
osapi int setprio(handle_t thread, int priority);
osapi int msleep(int millisecs);
osapi unsigned sleep(unsigned seconds);
osapi struct tib *gettib();
osapi int spawn(int mode, const char *pgm, const char *cmdline, char **env, struct tib **tibptr);
osapi void exit(int status);
osapi void _exit(int status);

osapi sighandler_t signal(int signum, sighandler_t handler);
osapi int raise(int signum);
osapi int kill(pid_t pid, int signum);
osapi int sendsig(handle_t thread, int signum);
osapi void sigexit(struct siginfo *info, int action);
osapi char *strsignal(int signum);

osapi int sigemptyset(sigset_t *set);
osapi int sigfillset(sigset_t *set);
osapi int sigaddset(sigset_t *set, int signum);
osapi int sigdelset(sigset_t *set, int signum);
osapi int sigismember(sigset_t *set, int signum);

osapi int sigaction(int signum, const struct sigaction *act, struct sigaction *oldact);
osapi int sigprocmask(int how, const sigset_t *set, sigset_t *oldset);
osapi int sigpending(sigset_t *set);
osapi int sigsuspend(const sigset_t *mask);
osapi unsigned alarm(unsigned seconds);

osapi time_t time(time_t *timeptr);
osapi int gettimeofday(struct timeval *tv, void *tzp);
osapi int settimeofday(struct timeval *tv);
osapi clock_t clock();

osapi void openlog(char *ident, int option, int facility);
osapi void closelog();
osapi int setlogmask(int mask);
osapi void syslog(int pri, const char *fmt, ...);
osapi void vsyslog(int pri, const char *fmt, va_list args);

osapi void panic(const char *msg);
osapi int canonicalize(const char *filename, char *buffer, int size);

osapi void mkcs(critsect_t cs);
osapi void csfree(critsect_t cs);
osapi void enter(critsect_t cs);
osapi void leave(critsect_t cs);

osapi void *_lmalloc(size_t size);
osapi void *_lrealloc(void *mem, size_t size);
osapi void *_lcalloc(size_t num, size_t size);
osapi void _lfree(void *p);

#ifdef USE_LOCAL_HEAP

#define malloc(n) _lmalloc(n)
#define realloc(p, n) _lrealloc((p), (n))
#define calloc(n, s) _lcalloc((n), (s))
#define free(p) _lfree(p)

#else
osapi void *malloc(size_t size);
osapi void *realloc(void *mem, size_t size);
osapi void *calloc(size_t num, size_t size);
osapi void free(void *p);
#endif

osapi struct mallinfo mallinfo();
osapi int malloc_usable_size(void *p);

osapi hmodule_t dlopen(const char *name, int mode);
osapi int dlclose(hmodule_t hmod);
osapi void *dlsym(hmodule_t hmod, const char *procname);
osapi char *dlerror();
osapi hmodule_t getmodule(const char *name);
osapi int getmodpath(hmodule_t hmod, char *buffer, int size);
osapi int exec(hmodule_t hmod, const char *args, char **env);
osapi void *getresdata(hmodule_t hmod, int type, char *name, int lang, int *len);
osapi int getreslen(hmodule_t hmod, int type, char *name, int lang);
osapi struct verinfo *getverinfo(hmodule_t hmod);
osapi int getvervalue(hmodule_t hmod, char *name, char *buf, int size);

osapi tls_t tlsalloc();
osapi void tlsfree(tls_t index);
osapi void *tlsget(tls_t index);
osapi int tlsset(tls_t index, void *value);

osapi int accept(int s, struct sockaddr *addr, int *addrlen);
osapi int bind(int s, const struct sockaddr *name, int namelen);
osapi int connect(int s, const struct sockaddr *name, int namelen);
osapi int getpeername(int s, struct sockaddr *name, int *namelen);
osapi int getsockname(int s, struct sockaddr *name, int *namelen);
osapi int getsockopt(int s, int level, int optname, char *optval, int *optlen);
osapi int listen(int s, int backlog);
osapi int recv(int s, void *data, int size, unsigned int flags);
osapi int recvfrom(int s, void *data, int size, unsigned int flags, struct sockaddr *from, int *fromlen);
osapi int recvmsg(int s, struct msghdr *hdr, unsigned int flags);
osapi int send(int s, const void *data, int size, unsigned int flags);
osapi int sendto(int s, const void *data, int size, unsigned int flags, const struct sockaddr *to, int tolen);
osapi int sendmsg(int s, struct msghdr *hdr, unsigned int flags);
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

osapi char *getenv(const char *name);
osapi int setenv(const char *name, const char *value, int rewrite);
osapi void unsetenv(const char *name);
osapi int putenv(const char *str);

osapi struct section *osconfig();
osapi struct peb *getpeb();

osapi int __getstdhndl(int n);

#ifndef errno
osapi int *_errno();
#define errno (*_errno())
#endif

#ifndef fmode
osapi int *_fmode();
#define fmode (*_fmode())
#endif

#ifndef environ
osapi char ***_environ();
#define environ (*_environ())
#endif

#ifdef  __cplusplus
}
#endif

#endif

#endif
