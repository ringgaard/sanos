//
// sow.c
//
// Copyright (c) 2002 Michael Ringgaard. All rights reserved.
//
// Sanos on Win32
//

#define handle_t oshandle_t
#define STRING_H
#define _INC_STRING
#define strerror win32_strerror

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#undef PARITY_NONE
#undef PARITY_ODD
#undef PARITY_EVEN
#undef PARITY_MARK
#undef PARITY_SPACE

#undef handle_t
#undef STRING_H
#undef _INC_STRING
#undef strerror

#define OS_LIB
#include "../../src/include/sys/types.h"
#include "../../src/include/os/version.h"
#include "../../src/include/os.h"
#include "../../src/include/inifile.h"
#include "../../src/include/atomic.h"

#define MAXHANDLES 1024
#define MAXENVVARS 1024

#define SECTIMESCALE    10000000               // 1 sec resolution
#define SECEPOC         11644473600            // 00:00:00 GMT on January 1, 1970 (in secs) 

#define HANDLE_FREE    0x1000
#define HANDLE_DEV     0x1001
#define HANDLE_THREAD  0x1002
#define HANDLE_SEM     0x1003
#define HANDLE_EVENT   0x1004
#define HANDLE_FILE    0x1005
#define HANDLE_DIR     0x1006
#define HANDLE_SOCKET  0x1007
#define HANDLE_ANY     0x1FFF

struct handle
{
  int type;
  void *data;
};

struct finddata
{
  HANDLE fhandle;
  BOOL first;
  WIN32_FIND_DATA fdata;
};

typedef struct
{
  unsigned long signature;
  IMAGE_FILE_HEADER header;
  IMAGE_OPTIONAL_HEADER optional;
} IMAGE_HEADER;

#define WSADESCRIPTION_LEN      256
#define WSASYS_STATUS_LEN       128

#define WSAERRBASE              10000

typedef void *SOCKET;
typedef struct iovec WSABUF;
typedef struct iovec *LPWSABUF;

typedef struct WSAData 
{
  WORD wVersion;
  WORD wHighVersion;
  char szDescription[WSADESCRIPTION_LEN + 1];
  char szSystemStatus[WSASYS_STATUS_LEN + 1];
  unsigned short iMaxSockets;
  unsigned short iMaxUdpDg;
  char *lpVendorInfo;
} WSADATA, *LPWSADATA; 

#define WSAAPI __stdcall

static SOCKET (WSAAPI *_socket)(int af, int type, int protocol);
static unsigned long (WSAAPI *_inet_addr)(const char *cp);
static int (WSAAPI *_connect)(SOCKET s, const struct sockaddr *name, int namelen);
static int (WSAAPI *_setsockopt)(SOCKET s, int level, int optname, const char *optval, int optlen);
static int (WSAAPI *_closesocket)(SOCKET s);
static int (WSAAPI *_send)(SOCKET s, const char *buf, int len, int flags);
static int (WSAAPI *_recv)(SOCKET s, char *buf, int len, int flags);
static int (WSAAPI *_sendto)(SOCKET s, const char *buf, int len, int flags, const struct sockaddr *to, int tolen);
static int (WSAAPI *_recvfrom)(SOCKET s, char *buf, int len, int flags, struct sockaddr *from, int *fromlen);
static struct hostent *(WSAAPI *_gethostbyaddr)(const char *addr, int len, int type);
static struct hostent *(WSAAPI *_gethostbyname)(const char *name);
static int (WSAAPI *_getpeername)(SOCKET s, struct sockaddr *name, int *namelen);
static struct servent *(WSAAPI *_getservbyname)(const char *name, const char *proto);
static int (WSAAPI *_WSAGetLastError)(void);
static int (WSAAPI *_WSAStartup)(WORD wVersionRequired, LPWSADATA lpWSAData);
static int (WSAAPI *_WSASend)(SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount, DWORD *lpNumberOfBytesSent, DWORD dwFlags, void *lpOverlapped, void *lpCompletionRoutine);
static char *(WSAAPI *_inet_ntoa)(struct in_addr in);
static int (WSAAPI *_accept)(SOCKET s, struct sockaddr *addr, int *addrlen);
static int (WSAAPI *_bind)(SOCKET s, const struct sockaddr *name, int namelen);
static int (WSAAPI *_getsockname)(SOCKET s, struct sockaddr *name, int *namelen);
static int (WSAAPI *_getsockopt)(SOCKET s, int level, int optname, char *optval, int *optlen);
static int (WSAAPI *_listen)(SOCKET s, int backlog);
static int (WSAAPI *_shutdown)(SOCKET s, int how);
static int (WSAAPI *_select)(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, const struct timeval *timeout);
static struct protoent *(WSAAPI *_getprotobyname)(const char *name);
static struct protoent *(WSAAPI *_getprotobynumber)(int proto);
static struct servent *(WSAAPI *_getservbyport)(int port, const char *proto);
static int (WSAAPI *_gethostname)(char *name, int namelen);

handle_t halloc(int type, void *data);
int vsprintf(char *buf, const char *fmt, va_list args);
int sprintf(char *buf, const char *fmt, ...);

//
// Globals
//

struct handle htab[MAXHANDLES];
struct process *mainproc;
DWORD tibtls;
char *envtab[MAXENVVARS];

struct term console;
struct peb *peb;
unsigned long loglevel;

static struct passwd defpasswd = {"root", "", 0, 0, "root", "/", "/bin/sh.exe"};
static char *defgrpmem[] = {"root", NULL};
static struct group defgroup = {"root", "", 0, defgrpmem};

//
// Intrinsic functions
//

//#ifdef _DEBUG

#pragma function(memset)
#pragma function(memcmp)
#pragma function(memcpy)

#pragma function(strcpy)
#pragma function(strlen)
#pragma function(strcat)
#pragma function(strcmp)
#pragma function(strset)

void *memset(void *p, int c, size_t n)
{
  char *pb = (char *) p;
  char *pbend = pb + n;
  while (pb != pbend) *pb++ = c;
  return p;
}

int memcmp(const void *dst, const void *src, size_t n)
{
  if (!n) return 0;

  while (--n && *(char *) dst == *(char *) src)
  {
    dst = (char *) dst + 1;
    src = (char *) src + 1;
  }

  return *((unsigned char *) dst) - *((unsigned char *) src);
}

void *memcpy(void *dst, const void *src, size_t n)
{
  void *ret = dst;

  while (n--)
  {
    *(char *)dst = *(char *)src;
    dst = (char *) dst + 1;
    src = (char *) src + 1;
  }

  return ret;
}

size_t strlen(const char *s)
{
  const char *eos = s;

  while (*eos++);

  return (int) (eos - s - 1);
}

char *strcpy(char *dst, const char *src)
{
  char *cp = dst;
  while (*cp++ = *src++);
  return dst;
}

int strcmp(const char * src, const char * dst)
{
  int ret = 0;

  while (!(ret = *(unsigned char *) src - *(unsigned char *) dst) && *dst) ++src, ++dst;

  if (ret < 0)
    ret = -1;
  else if (ret > 0)
    ret = 1;

  return ret;
}

int strncmp(const char *s1, const char *s2, size_t count)
{
  if (!count) return 0;

  while (--count && *s1 && *s1 == *s2)
  {
    s1++;
    s2++;
  }

  return *(unsigned char *) s1 - *(unsigned char *) s2;
}

//#endif

static __declspec(naked) unsigned __int64 div64x32(unsigned __int64 dividend, unsigned int divisor)
{
  __asm
  {
      push    ebx
      mov     ecx,[esp+16]    ; load divisor
      mov     eax,[esp+12]    ; load high word of dividend
      xor     edx,edx
      div     ecx             ; eax <- high order bits of quotient
      mov     ebx,eax         ; save high bits of quotient
      mov     eax,[esp+8]     ; edx:eax <- remainder:lo word of dividend
      div     ecx             ; eax <- low order bits of quotient
      mov     edx,ebx         ; edx:eax <- quotient
      pop     ebx
      ret
  }
}

//
// Initialization/termination
//

void initsock()
{
  WSADATA wsa_data;
  HMODULE dll;
  
  dll = LoadLibrary("WS2_32");
  if (dll == NULL) return;

  _socket = (SOCKET (WSAAPI *)(int, int, int)) GetProcAddress(dll, "socket");
  _inet_addr = (unsigned long (WSAAPI *)(const char *)) GetProcAddress(dll, "inet_addr");
  _connect = (int (WSAAPI *)(SOCKET, const struct sockaddr *, int)) GetProcAddress(dll, "connect");
  _setsockopt = (int (WSAAPI *)(SOCKET, int, int, const char *, int)) GetProcAddress(dll, "setsockopt");
  _closesocket = (int (WSAAPI *)(SOCKET)) GetProcAddress(dll, "closesocket");
  _send = (int (WSAAPI *)(SOCKET, const char *, int, int)) GetProcAddress(dll, "send");
  _recv = (int (WSAAPI *)(SOCKET, char *, int, int)) GetProcAddress(dll, "recv");
  _sendto = (int (WSAAPI *)(SOCKET, const char *, int, int, const struct sockaddr *, int)) GetProcAddress(dll, "sendto");
  _recvfrom = (int (WSAAPI *)(SOCKET, char *, int, int, struct sockaddr *, int *)) GetProcAddress(dll, "recvfrom");
  _gethostbyaddr = (struct hostent * (WSAAPI *)(const char *, int, int)) GetProcAddress(dll, "gethostbyaddr");
  _gethostbyname = (struct hostent * (WSAAPI *)(const char *)) GetProcAddress(dll, "gethostbyname");
  _getpeername = (int (WSAAPI *)(SOCKET, struct sockaddr *, int *)) GetProcAddress(dll, "getpeername");
  _getservbyname = (struct servent * (WSAAPI *)(const char *, const char *)) GetProcAddress(dll, "getservbyname");
  _WSAGetLastError = (int (WSAAPI *)(void)) GetProcAddress(dll, "WSAGetLastError");
  _WSAStartup = (int (WSAAPI *)(WORD, LPWSADATA)) GetProcAddress(dll, "WSAStartup");
  _WSASend = (int (WSAAPI *)(SOCKET, LPWSABUF, DWORD, DWORD *, DWORD, void *, void *)) GetProcAddress(dll, "WSASend");
  _inet_ntoa = (char *(WSAAPI *)(struct in_addr)) GetProcAddress(dll, "inet_ntoa");
  _accept = (int (WSAAPI *)(SOCKET s, struct sockaddr *addr, int *addrlen)) GetProcAddress(dll, "accept");
  _bind = (int (WSAAPI *)(SOCKET s, const struct sockaddr *name, int namelen)) GetProcAddress(dll, "bind");
  _getsockname = (int (WSAAPI *)(SOCKET s, struct sockaddr *name, int *namelen)) GetProcAddress(dll, "getsockname");
  _getsockopt = (int (WSAAPI *)(SOCKET s, int level, int optname, char *optval, int *optlen)) GetProcAddress(dll, "getsockopt");
  _listen = (int (WSAAPI *)(SOCKET s, int backlog)) GetProcAddress(dll, "listen");
  _shutdown = (int (WSAAPI *)(SOCKET s, int how)) GetProcAddress(dll, "shutdown");
  _select = (int (WSAAPI *)(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, const struct timeval *timeout)) GetProcAddress(dll, "select");
  _getprotobyname = (struct protoent *(WSAAPI *)(const char *name)) GetProcAddress(dll, "getprotobyname");
  _getprotobynumber = (struct protoent *(WSAAPI *)(int proto)) GetProcAddress(dll, "getprotobynumber");
  _getservbyport = (struct servent *(WSAAPI *)(int port, const char *proto)) GetProcAddress(dll, "getservbyport");
  _gethostname = (int (WSAAPI *)(char *name, int namelen)) GetProcAddress(dll, "gethostname");

  // initialize winsocket
  if (_WSAStartup(0x0202, &wsa_data) == -1) return;
}

void setup_env()
{
  char *env;
  int i;
        
  env = GetEnvironmentStrings();
  i = 0;
  while (i < MAXENVVARS - 1 && *env)
  {
    envtab[i++] = env;
    env += strlen(env) + 1;
  }
  envtab[i] = NULL;
}

void init()
{
  struct tib *tib;
  struct process *proc;
  int i;

  // Initialize sockets
  initsock();

  // Clear handle table
  for (i = 0; i < MAXHANDLES; i++)
  {
    htab[i].type = HANDLE_FREE;
    htab[i].data = NULL;
  }

  // Create handles for stdin, stdout and stderr
  htab[0].type = HANDLE_DEV;
  htab[0].data = GetStdHandle(STD_INPUT_HANDLE);

  htab[1].type = HANDLE_DEV;
  htab[1].data = GetStdHandle(STD_OUTPUT_HANDLE);

  htab[2].type = HANDLE_DEV;
  htab[2].data = GetStdHandle(STD_ERROR_HANDLE);

  // Allocate TLS for tib
  tibtls = TlsAlloc();

  // Allocate process environment block (PEB)
  peb = malloc(4096);
  memset(peb, 0, 4096);

  // Get environment variables
  setup_env();

  // Allocate initial job
  console.cols = 80;
  console.lines = 25;
  console.type = TERM_CONSOLE;

  proc = malloc(sizeof(struct process));
  memset(proc, 0, sizeof(struct process));

  proc->threadcnt = 1;
  proc->id = 1;
  proc->iob[0] = 0;
  proc->iob[1] = 1;
  proc->iob[2] = 2;
  proc->env = envtab;
  proc->term = &console;
  proc->hmod = GetModuleHandle(NULL);
  proc->cmdline = GetCommandLine();
  proc->facility = LOG_DAEMON;
  proc->ident = "sanos";

  mainproc = peb->firstproc = peb->lastproc = proc;

  // Allocate tib for initial thread
  tib = malloc(4096);
  memset(tib, 0, 4096);
  tib->self = tib;
  tib->tlsbase = &tib->tls;
  tib->pid = 1;
  tib->tid = GetCurrentThreadId();
  tib->hndl = halloc(HANDLE_THREAD, GetCurrentThread());
  tib->peb = peb;
  tib->except = (struct xcptrec *) 0xFFFFFFFF;
  tib->proc = proc;

  TlsSetValue(tibtls, tib);
}

void term()
{
  if (mainproc && mainproc->atexit) mainproc->atexit(0, 0);
}

//
// Utility routines
//

static int notimpl(char *func)
{
  syslog(LOG_DEBUG, "Function %s not implemented", func);

  errno = ENOSYS;
  return -1;
}

static int winerr()
{
  int err;
  int rc = GetLastError();

  switch (rc)
  {
    case ERROR_INVALID_FUNCTION: err = ENOSYS; break;
    case ERROR_FILE_NOT_FOUND: err = ENOENT; break;
    case ERROR_PATH_NOT_FOUND: err = ENOENT; break;
    case ERROR_TOO_MANY_OPEN_FILES: err = ENFILE; break;
    case ERROR_ACCESS_DENIED: err = EACCES; break;
    case ERROR_ARENA_TRASHED: err = ENOMEM; break;
    case ERROR_NOT_ENOUGH_MEMORY: err = ENOMEM; break;
    case ERROR_INVALID_BLOCK: err = ENOMEM; break;
    case ERROR_BAD_ENVIRONMENT: err = E2BIG; break;
    case ERROR_BAD_FORMAT: err = ENOEXEC; break;
    case ERROR_INVALID_ACCESS: err = EINVAL; break;
    case ERROR_INVALID_DATA: err = EINVAL; break;
    case ERROR_INVALID_DRIVE: err = ENOENT; break;
    case ERROR_INVALID_NAME: err= EINVAL; break;
    case ERROR_CURRENT_DIRECTORY: err = EACCES; break;
    case ERROR_NOT_SAME_DEVICE: err = EXDEV; break;
    case ERROR_NO_MORE_FILES: err = ENOENT; break;
    case ERROR_LOCK_VIOLATION: err = EACCES; break;
    case ERROR_BAD_NETPATH: err = ENOENT; break;
    case ERROR_NETWORK_ACCESS_DENIED: err = EACCES; break;
    case ERROR_BAD_NET_NAME: err = ENOENT; break;
    case ERROR_FILE_EXISTS: err = EEXIST; break;
    case ERROR_CANNOT_MAKE: err = EACCES; break;
    case ERROR_FAIL_I24: err = EACCES; break;
    case ERROR_INVALID_PARAMETER: err = EINVAL; break;
    case ERROR_NO_PROC_SLOTS: err = EAGAIN; break;
    case ERROR_DRIVE_LOCKED: err = EACCES; break;
    case ERROR_BROKEN_PIPE: err = EPIPE; break;
    case ERROR_DISK_FULL: err = ENOSPC; break;
    case ERROR_INVALID_TARGET_HANDLE: err = EBADF; break;
    case ERROR_INVALID_HANDLE: err = EINVAL; break;
    //case ERROR_WAIT_NO_CHILDREN: err = ECHILD; break;
    //case ERROR_CHILD_NOT_COMPLETE: err = ECHILD; break;
    case ERROR_DIRECT_ACCESS_HANDLE: err = EBADF; break;
    case ERROR_NEGATIVE_SEEK: err = EINVAL; break;
    case ERROR_SEEK_ON_DEVICE: err = EACCES; break;
    case ERROR_DIR_NOT_EMPTY: err = ENOTEMPTY; break;
    case ERROR_NOT_LOCKED: err = EACCES; break;
    case ERROR_BAD_PATHNAME: err = ENOENT; break;
    case ERROR_MAX_THRDS_REACHED: err = EAGAIN; break;
    case ERROR_LOCK_FAILED: err = EACCES; break;
    case ERROR_ALREADY_EXISTS: err = EEXIST; break;
    case ERROR_FILENAME_EXCED_RANGE: err = ENOENT; break;
    case ERROR_NESTING_NOT_ALLOWED: err = EAGAIN; break;
    case ERROR_NOT_ENOUGH_QUOTA: err = ENOMEM; break;
    case ERROR_SHARING_VIOLATION: err = EACCES; break;

    default:
      if (rc >= WSAERRBASE && rc < WSAERRBASE + 35)
        err = rc - WSAERRBASE;
      else if (rc >= WSAERRBASE + 35 && rc < WSAERRBASE + 72)
        err = rc - WSAERRBASE + 10;
      else if (rc >= WSAERRBASE + 1000 && rc < WSAERRBASE + 1005)
        err = rc - WSAERRBASE + 81;
      else
      {
        syslog(LOG_DEBUG, "Unknown win32 error %d", rc);
        err = (rc & 0xFFFF) << 16;
      }
  }

  errno = err;
  return -1;
}

int sockcall(int rc)
{
  if (rc != -1) 
    return rc;
  else
    return winerr();
}

handle_t halloc(int type, void *data)
{
  int h;

  if (data == INVALID_HANDLE_VALUE) return winerr();

  for (h = 0; h < MAXHANDLES; h++)
  {
    if (htab[h].type == HANDLE_FREE) 
    {
      htab[h].type = type;
      htab[h].data = data;
      return h;
    }
  }

  errno = ENFILE;
  return -1;
}

void hfree(handle_t h)
{
  htab[h].type = HANDLE_FREE;
}

void *hget(handle_t h, int type)
{
  if (h < 0 || h >= MAXHANDLES || htab[h].type == HANDLE_FREE) 
  {
    errno = EBADF;
    return INVALID_HANDLE_VALUE;
  }

  if (type != HANDLE_ANY && type != htab[h].type)
  {
    errno = EBADF;
    return INVALID_HANDLE_VALUE;
  }

  return htab[h].data;
}

void make_external_filename(const char *name, char *fn)
{
  char *s = fn;

  if (name[0] && name[1] == ':') name += 2;
  while (*name)
  {
    if (*name == PS1 || *name == PS2)
      *s++ = '\\';
    else
      *s++ = *name;

    name++;
  }
  *s = 0;
}

static time_t ft2time(FILETIME *ft)
{
  return (time_t) (div64x32(*(unsigned __int64 *) ft, SECTIMESCALE) - SECEPOC);
}

//
// OS API
//

int syscall(int syscallno, void *params)
{
  return notimpl("syscall");
}

int mkfs(const char *devname, const char *type, const char *opts)
{
  return notimpl("mkfs");
}

int mount(const char *type, const char *path, const char *devname, const char *opts)
{
  return notimpl("mount");
}

int umount(const char *path)
{
  return notimpl("umount");
}

int getfsstat(struct statfs *buf, size_t size)
{
  return notimpl("getfsstat");
}

int fstatfs(handle_t f, struct statfs *buf)
{
  return notimpl("fstatfs");
}

int statfs(const char *name, struct statfs *buf)
{
  return notimpl("statfs");
}

handle_t open(const char *name, int flags, ...)
{
  HANDLE hfile;
  DWORD access;
  DWORD disp;
  DWORD attrs;
  DWORD sharing;
  char fn[MAXPATH];
  va_list args;
  int mode = 0;

  make_external_filename(name, fn);

  if (flags & O_CREAT)
  {
    va_start(args, flags);
    mode = va_arg(args, int);
    va_end(args);
  }

  // Determine open mode
  switch (flags & (O_CREAT | O_EXCL | O_TRUNC))
  {
    case 0:
    case O_EXCL:
      disp = OPEN_EXISTING;
      break;

    case O_CREAT:
      disp = OPEN_ALWAYS;
      break;

    case O_CREAT | O_EXCL:
    case O_CREAT | O_TRUNC | O_EXCL:
      disp = CREATE_NEW;
      break;

    case O_TRUNC:
    case O_TRUNC | O_EXCL:
      disp = TRUNCATE_EXISTING;
      break;

    case O_CREAT | O_TRUNC:
      disp = CREATE_ALWAYS;
      break;

    default:
      errno = EINVAL;
      return -1;
  }

  // Determine file access
  if (flags & O_RDONLY)
    access = GENERIC_READ;
  else if (flags & O_WRONLY)
    access = GENERIC_WRITE;
  else
    access = GENERIC_READ | GENERIC_WRITE;

  // Determine sharing access
  switch (SH_FLAGS(flags))
  {
    case SH_DENYRW:
      sharing = 0;
      break;

    case SH_DENYWR:
      sharing = FILE_SHARE_READ;
      break;

    case SH_DENYRD:
      sharing = FILE_SHARE_WRITE;
      break;

    case SH_DENYNO:
    case SH_COMPAT:
      sharing = FILE_SHARE_READ | FILE_SHARE_WRITE;
      break;

    default:
      errno = EINVAL;
      return -1;
  }

  // Determine file attributes
  if ((flags & O_CREAT) != 0 && (mode & S_IWRITE) == 0) 
    attrs = FILE_ATTRIBUTE_READONLY;
  else
    attrs = FILE_ATTRIBUTE_NORMAL;

  if (flags & O_TEMPORARY)
  {
    attrs |= FILE_FLAG_DELETE_ON_CLOSE;
    access |= DELETE;
    sharing |= FILE_SHARE_DELETE;
  }

  if (flags & O_SHORT_LIVED) attrs |= FILE_ATTRIBUTE_TEMPORARY;

  if (flags & O_SEQUENTIAL)
    attrs |= FILE_FLAG_SEQUENTIAL_SCAN;
  else if (flags & O_RANDOM)
    attrs |= FILE_FLAG_RANDOM_ACCESS;

  if (flags & O_DIRECT) attrs |= FILE_FLAG_NO_BUFFERING | FILE_FLAG_WRITE_THROUGH;

  hfile = CreateFile(fn, access, sharing, NULL, disp, attrs, NULL);
  if (hfile == INVALID_HANDLE_VALUE) return winerr();

  return halloc(HANDLE_FILE, hfile);
}

handle_t sopen(const char *name, int flags, int shflags, ...)
{
  va_list args;
  int mode = 0;

  if (flags & O_CREAT)
  {
    va_start(args, shflags);
    mode = va_arg(args, int);
    va_end(args);
  }

  return open(name, FILE_FLAGS(flags, shflags), mode);
}

handle_t creat(const char *name, int mode)
{
  return open(name, O_CREAT | O_TRUNC | O_WRONLY, mode);
}

int close(handle_t h)
{
  void *data = hget(h, HANDLE_ANY);
  if (data == INVALID_HANDLE_VALUE) return -1;

  switch (htab[h].type)
  {
    case HANDLE_DEV:
    case HANDLE_THREAD:
    case HANDLE_SEM:
    case HANDLE_EVENT:
      if (!CloseHandle((HANDLE) htab[h].data)) return winerr();
      hfree(h);
      return 0;

    case HANDLE_FILE:
      if (!CloseHandle((HANDLE) htab[h].data)) return winerr();
      hfree(h);
      return 0;

    case HANDLE_DIR:
      if (!FindClose(((struct finddata *) htab[h].data)->fhandle)) return winerr();
      free(htab[h].data);
      hfree(h);
      return 0;

    case HANDLE_SOCKET:
      if (_closesocket((HANDLE) htab[h].data) == -1) return winerr();
      hfree(h);
      return 0;
  }

  errno = EBADF;
  return -1;
}

int fsync(handle_t f)
{
  void *data = hget(f, HANDLE_ANY);
  if (data == INVALID_HANDLE_VALUE) return -1; 

  if (htab[f].type == HANDLE_FILE || htab[f].type == HANDLE_DEV)
  {
    errno = EBADF;
    return -1;
  }

  if (!FlushFileBuffers((HANDLE) htab[f].data)) return winerr();
  return 0;
}

handle_t dup(handle_t h)
{
  HANDLE newh;

  switch (htab[h].type)
  {
    case HANDLE_DEV:
    case HANDLE_THREAD:
    case HANDLE_SEM:
    case HANDLE_EVENT:
    case HANDLE_FILE:
      if (!DuplicateHandle(GetCurrentProcess(), htab[h].data, GetCurrentProcess(), &newh, 0, FALSE, DUPLICATE_SAME_ACCESS)) return winerr();
      break;

    case HANDLE_DIR:
    case HANDLE_FREE:
    default:
      errno = EBADF;
      return -1;
  }

  return halloc(htab[h].type, newh);
}

handle_t dup2(handle_t h1, handle_t h2)
{
  return notimpl("dup2");
}

int read(handle_t f, void *data, size_t size)
{
  DWORD bytes;

  if (htab[f].type == HANDLE_SOCKET)
  {
    return sockcall(_recv((SOCKET) htab[f].data, data, size, 0));
  }
  else
  {
    if (!ReadFile((HANDLE) htab[f].data, data, size, &bytes, NULL)) return winerr();
    return bytes;
  }
}

int write(handle_t f, const void *data, size_t size)
{
  DWORD bytes;

  if (htab[f].type == HANDLE_SOCKET)
  {
    return sockcall(_send((SOCKET) htab[f].data, data, size, 0));
  }
  else
  {
    if (!WriteFile((HANDLE) htab[f].data, data, size, &bytes, NULL)) return winerr();
    return bytes;
  }
}

int pread(handle_t f, void *data, size_t size, off64_t offset)
{
  return notimpl("pread");
}

int pwrite(handle_t f, const void *data, size_t size, off64_t offset)
{
  return notimpl("pwrite");
}

int ioctl(handle_t f, int cmd, const void *data, size_t size)
{
  return notimpl("ioctl");
}

int readv(handle_t f, const struct iovec *iov, int count)
{
  return notimpl("readv");
}

int writev(handle_t f, const struct iovec *iov, int count)
{
  return notimpl("writev");
}

loff_t tell(handle_t f)
{
  return SetFilePointer((HANDLE) htab[f].data, 0, NULL, FILE_CURRENT);
}

off64_t tell64(handle_t f)
{
  LARGE_INTEGER zero;
  LARGE_INTEGER pos;

  zero.HighPart = 0;
  zero.LowPart = 0;
  if (!SetFilePointerEx((HANDLE) htab[f].data, zero, &pos, FILE_CURRENT)) return winerr();
  return pos.QuadPart;
}

loff_t lseek(handle_t f, loff_t offset, int origin)
{
  int method;

  switch (origin)
  {
    case SEEK_SET:
      method = FILE_BEGIN;
      break;

    case SEEK_CUR:
      method = FILE_CURRENT;
      break;

    case SEEK_END:
      method = FILE_END;
      break;
    
    default:
      errno = EINVAL;
      return -1;
  }

  return SetFilePointer((HANDLE) htab[f].data, offset, NULL, method);
}

off64_t lseek64(handle_t f, off64_t offset, int origin)
{
  LARGE_INTEGER off;
  LARGE_INTEGER pos;
  int method;

  switch (origin)
  {
    case SEEK_SET:
      method = FILE_BEGIN;
      break;

    case SEEK_CUR:
      method = FILE_CURRENT;
      break;

    case SEEK_END:
      method = FILE_END;
      break;
    
    default:
      errno = EINVAL;
      return -1;
  }

  off.QuadPart = offset;
  if (!SetFilePointerEx((HANDLE) htab[f].data, off, &pos, FILE_CURRENT)) return winerr();
  return pos.QuadPart;
}

int ftruncate(handle_t f, loff_t size)
{
  int rc = 0;
  loff_t curpos;

  curpos = SetFilePointer((HANDLE) htab[f].data, size, NULL, FILE_BEGIN);
  if (!SetEndOfFile((HANDLE) htab[f].data)) rc = winerr();
  SetFilePointer((HANDLE) htab[f].data, curpos, NULL, FILE_BEGIN);

  return rc;
}

int ftruncate64(handle_t f, off64_t size)
{
  LARGE_INTEGER pos;
  LARGE_INTEGER curpos;
  int rc = 0;

  pos.QuadPart = size;
  SetFilePointerEx((HANDLE) htab[f].data, pos, &curpos, FILE_BEGIN);
  if (!SetEndOfFile((HANDLE) htab[f].data)) rc = winerr();
  SetFilePointerEx((HANDLE) htab[f].data, curpos, NULL, FILE_BEGIN);

  return rc;
}

int futime(handle_t f, struct utimbuf *times)
{
  return notimpl("futime");
}

int utime(const char *name, struct utimbuf *times)
{
  return notimpl("utime");
}

int fstat(handle_t f, struct stat *buffer)
{
  BY_HANDLE_FILE_INFORMATION fi;

  if (f == NOHANDLE) 
  {
    errno = EINVAL;
    return -1;
  }

  if (htab[f].type != HANDLE_FILE) 
  {
    errno = EBADF;
    return -1;
  }

  if (!GetFileInformationByHandle(htab[f].data, &fi)) return -EBADF;

  if (buffer)
  {
    buffer->st_ino = 0;
    buffer->st_nlink = 1;
    buffer->st_dev = -1;
    buffer->st_atime = ft2time(&fi.ftLastAccessTime);
    buffer->st_ctime = ft2time(&fi.ftCreationTime);
    buffer->st_mtime = ft2time(&fi.ftLastWriteTime);
  
    buffer->st_size = fi.nFileSizeLow;
    buffer->st_mode = 0644;
  }

  return buffer ? 0 : fi.nFileSizeLow;
}

int fstat64(handle_t f, struct stat64 *buffer)
{
  BY_HANDLE_FILE_INFORMATION fi;

  if (f == NOHANDLE) return -EINVAL;
  if (htab[f].type != HANDLE_FILE) return -EBADF;

  if (!GetFileInformationByHandle(htab[f].data, &fi)) return -EBADF;

  if (buffer)
  {
    buffer->st_ino = 0;
    buffer->st_nlink = 1;
    buffer->st_dev = -1;
    buffer->st_atime = ft2time(&fi.ftLastAccessTime);
    buffer->st_ctime = ft2time(&fi.ftCreationTime);
    buffer->st_mtime = ft2time(&fi.ftLastWriteTime);
  
    buffer->st_size = ((__int64) fi.nFileSizeHigh << 32) | fi.nFileSizeLow;
    buffer->st_mode = 0644;
  }

  return buffer ? 0 : fi.nFileSizeLow;
}

int stat(const char *name, struct stat *buffer)
{
  WIN32_FILE_ATTRIBUTE_DATA fdata;
  char fn[MAXPATH];

  make_external_filename(name, fn);

  if (!GetFileAttributesEx(fn, GetFileExInfoStandard, &fdata)) return -ENOENT;

  if (buffer)
  {
    buffer->st_ino = 0;
    buffer->st_nlink = 1;
    buffer->st_dev = -1;
    buffer->st_atime = ft2time(&fdata.ftLastAccessTime);
    buffer->st_ctime = ft2time(&fdata.ftCreationTime);
    buffer->st_mtime = ft2time(&fdata.ftCreationTime);
    buffer->st_size = fdata.nFileSizeLow;
    buffer->st_mode = 0644;

    if (fdata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) 
      buffer->st_mode |= 0040000;
    else
      buffer->st_mode |= 0100000;
  }

  return buffer ? 0 : fdata.nFileSizeLow;
}

int stat64(const char *name, struct stat64 *buffer)
{
  WIN32_FILE_ATTRIBUTE_DATA fdata;
  char fn[MAXPATH];

  make_external_filename(name, fn);

  if (!GetFileAttributesEx(fn, GetFileExInfoStandard, &fdata)) return -ENOENT;

  if (buffer)
  {
    buffer->st_ino = 0;
    buffer->st_nlink = 1;
    buffer->st_dev = -1;
    buffer->st_atime = ft2time(&fdata.ftLastAccessTime);
    buffer->st_ctime = ft2time(&fdata.ftCreationTime);
    buffer->st_mtime = ft2time(&fdata.ftCreationTime);
    buffer->st_size = ((__int64) fdata.nFileSizeHigh << 32) | fdata.nFileSizeLow;
    buffer->st_mode = 0644;

    if (fdata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) buffer->st_mode |= 0040000;
  }

  return buffer ? 0 : fdata.nFileSizeLow;
}

int lstat(const char *name, struct stat *buffer)
{
  return stat(name, buffer);
}

loff_t filelength(handle_t f)
{
  return fstat(f, NULL);
}

off64_t filelength64(handle_t f)
{
  return fstat64(f, NULL);
}

int access(const char *name, int mode)
{
  int rc;
  struct stat64 buf;

  rc = stat64(name, &buf);
  if (rc < 0) return rc;

  switch (mode)
  {
    case 0:
      // Existence
      rc = 0; 
      break;

    case 2:
      // Write permission
      if (buf.st_mode & S_IWRITE)
        rc = 0;
      else
      {
        errno = EACCES;
        rc = -1;
      }
      break; 

    case 4: 
      // Read permission
      if (buf.st_mode & S_IREAD)
        rc = 0;
      else
      {
        errno = EACCES;
        rc = -1;
      }
      break; 

    case 6:
      // Read and write permission
      if ((buf.st_mode & (S_IREAD | S_IWRITE)) ==  (S_IREAD | S_IWRITE))
        rc = 0;
      else
      {
        errno = EACCES;
        rc = -1;
      }
      break;

    default:
      errno = EINVAL;
      rc = -1;
  }

  return rc;
}

int eof(handle_t f)
{
  return tell64(f) == fstat64(f, NULL);
}

int umask(int mask)
{
  int oldmask;

  mask &= 0777;
  oldmask = peb->umaskval;
  peb->umaskval = mask;
  return oldmask;
}

int isatty(handle_t f)
{
  errno = ENOTTY;
  return -1;
}

int setmode(handle_t f, int mode)
{
  return notimpl("setmode");
}

int chmod(const char *name, int mode)
{
  return notimpl("chmod");
}

int fchmod(handle_t f, int mode)
{
  return notimpl("fchmod");
}

int chown(const char *name, int owner, int group)
{
  return notimpl("chown");
}

int chdir(const char *name)
{
  char fn[MAXPATH];

  make_external_filename(name, fn);
  if (!SetCurrentDirectory(fn)) return winerr();
  return 0;
}

char *getcwd(char *buf, size_t size)
{
  char dirbuf[MAXPATH];
  char *curdir;
  char *p;
  size_t len;

  GetCurrentDirectory(MAXPATH, dirbuf);
  curdir = dirbuf;
  if (curdir[0] && curdir[1] == ':') curdir += 2;
  p = curdir;
  while (*p)
  {
    if (*p == PS2) *p = PS1;
    p++;
  }
  len = p - curdir;

  if (buf)
  {
    if (len >= size)
    {
      errno = ERANGE;
      return NULL;
    }
  }
  else
  {
    if (size == 0)
      size = len + 1;
    else if (len >= size)
    {
      errno = ERANGE;
      return NULL;
    }

    buf = malloc(size);
    if (!buf) 
    {
      errno = ENOMEM;
      return NULL;
    }
  }

  memcpy(buf, curdir, len + 1);
  return buf;
}

int mkdir(const char *name, int mode)
{
  char fn[MAXPATH];

  make_external_filename(name, fn);
  if (!CreateDirectory(fn, NULL)) return -ENOENT;
  return 0;
}

int rmdir(const char *name)
{
  char fn[MAXPATH];

  make_external_filename(name, fn);
  if (!RemoveDirectory(fn)) return -ENOENT;
  return 0;
}

int rename(const char *oldname, const char *newname)
{
  char oldfn[MAXPATH];
  char newfn[MAXPATH];

  make_external_filename(oldname, oldfn);
  make_external_filename(newname, newfn);

  if (!MoveFile(oldfn, newfn)) return -ENOENT;
  return 0;
}

int link(const char *oldname, const char *newname)
{
  char oldfn[MAXPATH];
  char newfn[MAXPATH];

  make_external_filename(oldname, oldfn);
  make_external_filename(newname, newfn);

  // NB: link is implemented as copy
  if (!CopyFile(oldfn, newfn, TRUE)) return -ENOENT;
  return 0;
}

int unlink(const char *name)
{
  char fn[MAXPATH];

  make_external_filename(name, fn);

  // NB: unlink is implemented as delete
  if (!DeleteFile(fn)) return -ENOENT;
  return 0;
}

handle_t _opendir(const char *name)
{
  char fn[MAXPATH];
  struct finddata *finddata;
  char *mask;

  make_external_filename((char *) name, fn);
  mask = fn + strlen(fn);
  if (mask > fn && mask[-1] != '\\') *mask++ = '\\';
  *mask++ = '*';
  *mask = 0;

  finddata = (struct finddata *) malloc(sizeof(struct finddata));
  finddata->fhandle = FindFirstFile(fn, &finddata->fdata);
  if (finddata->fhandle == INVALID_HANDLE_VALUE) return winerr();
  finddata->first = TRUE;

  return halloc(HANDLE_DIR, finddata);
}

int _readdir(handle_t f, struct direntry *dirp, int count)
{
  struct finddata *finddata;

  void *data = hget(f, HANDLE_DIR);
  if (data == INVALID_HANDLE_VALUE) return -1;
  finddata = (struct finddata *) data;

  if (!finddata->first)
  {
    if (!FindNextFile(finddata->fhandle, &finddata->fdata)) return 0;
  }

  finddata->first = FALSE;
  dirp->ino = -1;
  dirp->namelen = strlen(finddata->fdata.cFileName);
  dirp->reclen = sizeof(struct direntry) + dirp->namelen;
  strcpy(dirp->name, finddata->fdata.cFileName);
  return 1;
}

int pipe(handle_t fildes[2])
{
  return notimpl("pipe");
}

int __getstdhndl(int n)
{
  return n;
}

struct passwd *getpwnam(const char *name)
{
  return &defpasswd;
}

struct passwd *getpwuid(uid_t uid)
{
  return &defpasswd;
}

struct group *getgrnam(const char *name)
{
  return &defgroup;
}

struct group *getgrgid(uid_t uid)
{
  return &defgroup;
}

int getgroups(int size, gid_t list[])
{
  if (size < 1) return -EINVAL;
  list[0] = 0;
  return 1;
}

int getgid()
{
  return 0;
}

int getuid()
{
  return 0;
}

char *crypt(const char *key, const char *salt)
{
  notimpl("crypt");
  return NULL;
}

void *vmalloc(void *addr, unsigned long size, int type, int protect, unsigned long tag)
{
  return VirtualAlloc(addr, size, type, protect);
}

int vmfree(void *addr, unsigned long size, int type)
{
  if (type == MEM_DECOMMIT)
    return VirtualFree(addr, size, MEM_DECOMMIT) ? 0 : -EFAULT;
  else if (type == MEM_RELEASE)
    return VirtualFree(addr, 0, MEM_RELEASE) ? 0 : -EFAULT;
  else
    return -EINVAL;
}

void *vmrealloc(void *addr, unsigned long oldsize, unsigned long newsize, int type, int protect, unsigned long tag)
{
  notimpl("mremap");
  return NULL;
}

int vmprotect(void *addr, unsigned long size, int protect)
{
  DWORD oldprotect;

  return VirtualProtect(addr, size, protect, &oldprotect) ? 0 : -EFAULT;
}

int vmlock(void *addr, unsigned long size)
{
  return notimpl("mlock");
}

int vmunlock(void *addr, unsigned long size)
{
  return notimpl("mnlock");
}

int waitone(handle_t h, int timeout)
{
  DWORD rc = WaitForSingleObject(hget(h, HANDLE_ANY), timeout);
  if (rc == WAIT_TIMEOUT)
  {
    errno = ETIMEDOUT;
    return -1;
  }

  return 0;
}

int waitall(handle_t *h, int count, int timeout)
{
  return notimpl("waitall");
}

int waitany(handle_t *h, int count, int timeout)
{
  return notimpl("waitany");
}

handle_t mkevent(int manual_reset, int initial_state)
{
  return halloc(HANDLE_EVENT, CreateEvent(NULL, manual_reset, initial_state, NULL));
}

int epulse(handle_t h)
{
  PulseEvent((HANDLE) htab[h].data);
  return 0;
}

int eset(handle_t h)
{
  SetEvent((HANDLE) htab[h].data);
  return 0;
}

int ereset(handle_t h)
{
  ResetEvent((HANDLE) htab[h].data);
  return 0;
}

handle_t mksem(int initial_count)
{
  return halloc(HANDLE_SEM, CreateSemaphore(NULL, initial_count, 0x7FFFFFFF, NULL));
}

int semrel(handle_t h, int count)
{
  unsigned int prevcount;

  ReleaseSemaphore(htab[h].data, count, &prevcount);
  return prevcount;
}

handle_t mkmutex(int owned)
{
  return notimpl("mkmutex");
}

int mutexrel(handle_t h)
{
  return notimpl("mutexrel");
}

handle_t mkiomux(int flags)
{
  return notimpl("mkiomux");
}

int dispatch(handle_t iomux, handle_t h, int events, int context)
{
  return notimpl("dispatch");
}

int sysinfo(int cmd, void *data, size_t size)
{
  return notimpl("sysinfo");
}

int uname(struct utsname *buf)
{
  SYSTEM_INFO sysinfo;
  OSVERSIONINFO verinfo;

  GetSystemInfo(&sysinfo);
  verinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
  GetVersionEx(&verinfo);

  memset(buf, 0, sizeof(struct utsname));
  strcpy(buf->sysname, "windows");
  gethostname(buf->nodename, UTSNAMELEN);
  sprintf(buf->release, "%d.%d.%d.%d", verinfo.dwMajorVersion, verinfo.dwMinorVersion, verinfo.dwBuildNumber, 0);
  strcpy(buf->version, *verinfo.szCSDVersion ? verinfo.szCSDVersion : "release");
  sprintf(buf->machine, "i%d", sysinfo.dwProcessorType);
  return 0;  
}

handle_t self()
{
  return gettib()->hndl;
}

handle_t getjobhandle(pid_t pid)
{
  return notimpl("getjobhandle");
}

int getchildstat(pid_t pid, int *status)
{
  return notimpl("getchildstat");
}

int setchildstat(pid_t pid, int status)
{
  return notimpl("setchildstat");
}

void exitos(int mode)
{
  ExitProcess(0);
}

void dbgbreak()
{
  notimpl("dbgbreak");
}

handle_t beginthread(void (__stdcall *startaddr)(void *), unsigned stacksize, void *arg, int flags, char *name, struct tib **ptib)
{
  DWORD tid;

  return halloc(HANDLE_THREAD, CreateThread(NULL, stacksize, (unsigned long (__stdcall *)(void *)) startaddr, arg, (flags & CREATE_SUSPENDED) ? CREATE_SUSPENDED : 0, (DWORD *) &tid));
}

int suspend(handle_t thread)
{
  return SuspendThread(htab[thread].data);
}

int resume(handle_t thread)
{
  return ResumeThread(htab[thread].data);
}

struct tib *getthreadblock(handle_t thread)
{
  notimpl("getthreadblock");
  return NULL;
}

void endthread(int retval)
{
  ExitThread(retval);
}

tid_t gettid()
{
  return GetCurrentThreadId();
}

int setcontext(handle_t thread, void *context)
{
  return notimpl("setcontext");
}

int getcontext(handle_t thread, void *context)
{
  if (!GetThreadContext(htab[thread].data, context)) return -EINVAL;
  return 0;
}

int getprio(handle_t thread)
{
  return GetThreadPriority(htab[thread].data);
}

int setprio(handle_t thread, int priority)
{
  if (!SetThreadPriority(htab[thread].data, priority)) return -EINVAL;
  return 0;
}

handle_t getprochandle(pid_t pid) 
{
  return notimpl("getprochandle");
}

int msleep(int millisecs)
{
  Sleep(millisecs);
  return 0;
}

unsigned sleep(unsigned seconds) {
  Sleep(seconds * 1000);
  return 0;
}

struct tib *gettib()
{
  return (struct tib *) TlsGetValue(tibtls);
}

int spawn(int mode, const char *pgm, const char *cmdline, char **env, struct tib **tibptr)
{
  return notimpl("spawn");
}

void exit(int status)
{
  ExitProcess(status);
}

sighandler_t signal(int signum, sighandler_t handler)
{
  return (sighandler_t) notimpl("signal");
}

int sigsuspend(const sigset_t *mask) 
{
  return notimpl("sigsuspend");
}

int sigemptyset(sigset_t *set)
{
  *set = 0;
  return 0;
}

int sigaddset(sigset_t *set, int signum)
{
  *set |= (1 << signum);
  return 0;
}

int sigaction(int signum, const struct sigaction *act, struct sigaction *oldact)
{
  return notimpl("sigaction");
}


int raise(int signum)
{
  return notimpl("raise");
}

int kill(pid_t pid, int signum)
{
  return notimpl("kill");
}

int getpid()
{
  return 0;
}

int sendsig(handle_t thread, int signum)
{
  return notimpl("sendsig");
}

void sigexit(struct siginfo *info, int action)
{
  notimpl("sigexit");
}

unsigned alarm(unsigned seconds)
{
  notimpl("alarm");
  return -1;
}


char *getenv(const char *name)
{
  int i;
  int len;

  if (!name) return NULL;
  len = strlen(name);
  for (i = 0; envtab[i]; i++)
  {
    if (strncmp(envtab[i], name, len) == 0 && envtab[i][len] == '=') return envtab[i] + len + 1;
  }

  return NULL;

}

int setenv(const char *name, const char *value, int rewrite)
{
  if (rewrite || getenv(name) == NULL) 
  {
    SetEnvironmentVariable(name, value);
    setup_env();
  }
  return 0;
}

int putenv(const char *str)
{
  return notimpl("putenv");
}

time_t time(time_t *timeptr)
{
  FILETIME ft;
  time_t t;

  GetSystemTimeAsFileTime(&ft);
  t = ft2time(&ft);
  if (timeptr) *timeptr = t;
  return t;
}

int gettimeofday(struct timeval *tv, void *tzp)
{
  FILETIME ft;

  GetSystemTimeAsFileTime(&ft);
  tv->tv_sec = (long) ft2time(&ft);
  tv->tv_usec = ft.dwLowDateTime / 10;
  return tv->tv_usec;
}

int settimeofday(struct timeval *tv)
{
  return notimpl("settimeofday");
}

clock_t clock()
{
  return GetTickCount();
}

void openlog(char *ident, int option, int facility)
{
  notimpl("openlog");
}

void closelog()
{
  notimpl("closelog");
}

int setlogmask(int mask)
{
  return notimpl("setlogmask");
}

void syslog(int pri, const char *fmt, ...)
{
  va_list args;

  va_start(args, fmt);
  vsyslog(pri, fmt, args);
}

void vsyslog(int pri, const char *fmt, va_list args)
{
  char buffer[1024];
  DWORD written;

  vsprintf(buffer, fmt, args);
  WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), buffer, lstrlen(buffer), &written, NULL);
  WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), "\n", 1, &written, NULL);
}

void panic(const char *msg)
{
  DWORD written;
  WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), "panic: ", 7, &written, NULL);
  WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), msg, strlen(msg), &written, NULL);
  WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), "\n", 1, &written, NULL);
  ExitProcess(1);
}

char *strerror(int errnum)
{
  return "Error";
}

int canonicalize(const char *filename, char *buffer, int size)
{
  // TODO change
  char *basename = (char *) filename;
  char *p = (char *) filename;

  while (*p)
  {
    *buffer = *p;
    if (*buffer == PS1 || *buffer == PS2) basename = buffer + 1;
    buffer++;
    p++;
  }
  *buffer = 0;
  return 0;
}

void mkcs(critsect_t cs)
{
  cs->count = -1;
  cs->recursion = 0;
  cs->owner = NOHANDLE;
  cs->event = mkevent(0, 0);
}

void csfree(critsect_t cs)
{
  close(cs->event);
}

void enter(critsect_t cs)
{
  tid_t tid = GetCurrentThreadId();

  if (cs->owner == tid)
  {
    cs->recursion++;
  }
  else 
  {    
    if (atomic_add(&cs->count, 1) > 0) waitone(cs->event, INFINITE);
    cs->owner = tid;
  }
}

void leave(critsect_t cs)
{
  if (cs->owner != GetCurrentThreadId()) return;
  if (cs->recursion > 0)
  {
    cs->recursion--;
  }
  else
  {
    cs->owner = NOHANDLE;
    if (atomic_add(&cs->count, -1) >= 0) eset(cs->event);
  }
}

void *malloc(size_t size)
{
  return HeapAlloc(GetProcessHeap(), 0, size);
}

void *realloc(void *mem, size_t size)
{
  return mem ? HeapReAlloc(GetProcessHeap(), 0, mem, size) : HeapAlloc(GetProcessHeap(), 0, size);
}

void *calloc(size_t num, size_t size)
{
  char *p;

  p = HeapAlloc(GetProcessHeap(), 0, size * num);
  if (p) memset(p, 0, size * num);
  return p;
}

void free(void *p)
{
  HeapFree(GetProcessHeap(), 0, p);
}

struct mallinfo mallinfo()
{
  struct mallinfo mallinfo;
  memset(&mallinfo, 0, sizeof mallinfo);
  notimpl("mallinfo");
  return mallinfo;
}

void *_lmalloc(size_t size)
{
  return malloc(size);
}

void *_lrealloc(void *mem, size_t size)
{
  return realloc(mem, size);
}

void *_lcalloc(size_t num, size_t size)
{
  return calloc(num, size);
}

void _lfree(void *p)
{
  free(p);
}

hmodule_t dlopen(const char *name, int mode)
{
  char fn[MAXPATH];

  make_external_filename(name, fn);
  return LoadLibrary(fn);
}

int dlclose(hmodule_t hmod)
{
  return FreeLibrary(hmod) ? 0 : -EINVAL;
}

void *dlsym(hmodule_t hmod, const char *procname)
{
  return GetProcAddress(hmod, procname);
}

char *dlerror()
{
  notimpl("dlerror");
  return NULL;
}

hmodule_t getmodule(const char *name)
{
  char fn[MAXPATH];

  make_external_filename(name, fn);
  return GetModuleHandle(fn);
}

int getmodpath(hmodule_t hmod, char *buffer, int size)
{
  // TODO: convert filename to internal filename
  GetModuleFileName(hmod, buffer, size);
  return 0;
}

int exec(hmodule_t hmod, const char *args, char **env)
{
  IMAGE_DOS_HEADER *doshdr;
  IMAGE_HEADER *imghdr;
  void *entrypoint;

  doshdr = (IMAGE_DOS_HEADER *) hmod;
  imghdr = (IMAGE_HEADER *) (((char *) hmod) + doshdr->e_lfanew);
  entrypoint = ((char *) hmod) + imghdr->optional.AddressOfEntryPoint;

  return ((int (*)(hmodule_t, char *, char **)) entrypoint)(hmod, (char *) args, env);
}

void *getresdata(hmodule_t hmod, int type, char *name, int lang, int *len)
{
  notimpl("getresdata");
  return NULL;
}

int getreslen(hmodule_t hmod, int type, char *name, int lang)
{
  return notimpl("getreslen");
}

tls_t tlsalloc()
{
  return TlsAlloc();
}

void tlsfree(tls_t index)
{
  TlsFree(index);
}

void *tlsget(tls_t index)
{
  return TlsGetValue(index);
}

int tlsset(tls_t index, void *value)
{
  TlsSetValue(index, value);
  return 0;
}

int accept(int s, struct sockaddr *addr, int *addrlen)
{
  return sockcall(_accept(hget(s, HANDLE_SOCKET), addr, addrlen));
}

int bind(int s, const struct sockaddr *name, int namelen)
{
  return sockcall(_bind(hget(s, HANDLE_SOCKET), name, namelen));
}

int connect(int s, const struct sockaddr *name, int namelen)
{
  return sockcall(_connect(hget(s, HANDLE_SOCKET), name, namelen));
}

int getpeername(int s, struct sockaddr *name, int *namelen)
{
  return sockcall(_getpeername(hget(s, HANDLE_SOCKET), name, namelen));
}

int getsockname(int s, struct sockaddr *name, int *namelen)
{
  return sockcall(_getsockname(hget(s, HANDLE_SOCKET), name, namelen));
}

int getsockopt(int s, int level, int optname, char *optval, int *optlen)
{
  return sockcall(_getsockopt(hget(s, HANDLE_SOCKET), level, optname, optval, optlen));
}

int listen(int s, int backlog)
{
  return sockcall(_listen(hget(s, HANDLE_SOCKET), backlog));
}

int recv(int s, void *data, int size, unsigned int flags)
{
  return sockcall(_recv(hget(s, HANDLE_SOCKET), data, size, flags));
}

int recvfrom(int s, void *data, int size, unsigned int flags, struct sockaddr *from, int *fromlen)
{
  return sockcall(_recvfrom(hget(s, HANDLE_SOCKET), data, size, flags, from, fromlen));
}

int recvmsg(int s, struct msghdr *hdr, unsigned int flags)
{
  return notimpl("recvmsg");
}

int send(int s, const void *data, int size, unsigned int flags)
{
  return sockcall(_send(hget(s, HANDLE_SOCKET), data, size, flags));
}

int sendto(int s, const void *data, int size, unsigned int flags, const struct sockaddr *to, int tolen)
{
  return sockcall(_sendto(hget(s, HANDLE_SOCKET), data, size, flags, to, tolen));
}

int sendmsg(int s, struct msghdr *hdr, unsigned int flags)
{
  return notimpl("sendmsg");
}

int setsockopt(int s, int level, int optname, const char *optval, int optlen)
{
  return sockcall(_setsockopt(hget(s, HANDLE_SOCKET), level, optname, optval, optlen));
}

int shutdown(int s, int how)
{
  return sockcall(_shutdown(hget(s, HANDLE_SOCKET), how));
}

int socket(int domain, int type, int protocol)
{
  SOCKET sock;

  sock = _socket(domain, type, protocol);
  if (sock == (SOCKET) -1) return winerr();
  return halloc(HANDLE_SOCKET, sock);
}

int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, const struct timeval *timeout)
{
  return notimpl("select");
}

int res_send(const char *buf, int buflen, char *answer, int anslen)
{
  return notimpl("res_send");
}

int res_query(const char *dname, int cls, int type, unsigned char *answer, int anslen)
{
  return notimpl("res_query");
}

int res_search(const char *name, int cls, int type, unsigned char *answer, int anslen)
{
  return notimpl("res_search");
}

int res_querydomain(const char *name, const char *domain, int cls, int type, unsigned char *answer, int anslen)
{
  return notimpl("res_querydomain");
}

int res_mkquery(int op, const char *dname, int cls, int type, char *data, int datalen, unsigned char *newrr, char *buf, int buflen)
{
  return notimpl("res_mkquery");
}

int dn_comp(const char *src, unsigned char *dst, int dstsiz, unsigned char **dnptrs, unsigned char **lastdnptr)
{
  return notimpl("dn_comp");
}

int dn_expand(const unsigned char *msg, const unsigned char *eom, const unsigned char *src,  char *dst, int dstsiz)
{
  return notimpl("dn_expand");
}

struct hostent *gethostbyname(const char *name)
{
  struct hostent *ret = _gethostbyname(name);
  if (!ret) winerr();
  return ret;
}

struct hostent *gethostbyaddr(const char *addr, int len, int type)
{
  struct hostent *ret = _gethostbyaddr(addr, len, type);
  if (!ret) winerr();
  return ret;
}

char *inet_ntoa(struct in_addr in)
{
  return _inet_ntoa(in);
}

unsigned long inet_addr(const char *cp)
{
  return _inet_addr(cp);
}

int gethostname(char *name, int namelen)
{
  return sockcall(_gethostname(name, namelen));
}

struct protoent *getprotobyname(const char *name)
{
  struct protoent *ret = _getprotobyname(name);
  if (!ret) winerr();
  return ret;
}

struct protoent *getprotobynumber(int proto)
{
  struct protoent *ret = _getprotobynumber(proto);
  if (!ret) winerr();
  return ret;
}

struct servent *getservbyname(const char *name, const char *proto)
{
  struct servent *ret = _getservbyname(name, proto);
  if (!ret) winerr();
  return ret;
}

struct servent *getservbyport(int port, const char *proto)
{
  struct servent *ret = _getservbyport(port, proto);
  if (!ret) winerr();
  return ret;
}

int *_errno()
{
  return &(gettib()->errnum);
}

char ***_environ()
{
  return &mainproc->env;
}

int *_fmode()
{
  return &peb->fmodeval;
}

int __stdcall dllmain(hmodule_t hmodule, unsigned long reason, void *reserved)
{
  switch (reason)
  {
    case DLL_PROCESS_ATTACH:
      init();
      break;

    case DLL_PROCESS_DETACH:
      term();
      break;
  }

  return TRUE;
}
