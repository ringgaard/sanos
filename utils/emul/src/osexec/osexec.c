#define handle_t oshandle_t
#include <windows.h>
#undef handle_t

#include "types.h"
#include <vfs.h>

#define OSEXEC
#include <os.h>
#include <syscall.h>
#include <sockcall.h>

#define SECTORSIZE 512

#define IMAGEDIR "d:\\sanos\\image\\"
#define IMAGEFILE "d:\\sanos\\sanos.img"

//#define USEDFS

HANDLE heap;
HANDLE hdev;

void (*user_break_handler)(int type);

#define PAGESIZE 4096

#define MILLITIMESCALE  10000                  // 1 ms resolution
#define MILLIEPOC       11644473600000         // 00:00:00 GMT on January 1, 1970 (in millisecs)

#define SECTIMESCALE    10000000               // 1 sec resolution
#define SECEPOC         11644473600            // 00:00:00 GMT on January 1, 1970 (in secs) 

#define OK    0
#define EFAIL (-1)

void panic(char *msg);

#ifdef _DEBUG

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

#endif

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

#define HANDLE_FREE    0x1000
#define HANDLE_DEV     0x1001
#define HANDLE_THREAD  0x1002
#define HANDLE_SEM     0x1003
#define HANDLE_EVENT   0x1004
#define HANDLE_FILE    0x1005
#define HANDLE_DIR     0x1006

#define MAXHANDLES 1024

struct handle
{
  int type;
  void *data;
};

struct handle htab[MAXHANDLES];
DWORD threadtls;

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

int atoi(const char *s)
{
  int n;
  int sign;

  if (!s) return -1;
  while (*s == ' ') s++;

  sign = *s;
  if (*s == '-' || *s == '+') s++;

  n = 0;
  if (*s < '0' || *s > '9') return -1;
  while (*s >= '0' && *s <= '9') n = 10 * n + (*s++ - '0');

  return sign == '-' ? -n : n;
}

int dev_read(devno_t devno, void *buffer, size_t count, blkno_t blkno) 
{
  DWORD bytes;

  //printf("read block %d, %d bytes\n", blkno, count);

  if (SetFilePointer((HANDLE) devno, blkno * SECTORSIZE, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER) panic("unable to set file pointer");
  if (!ReadFile((HANDLE) devno, buffer, count, &bytes, NULL)) panic("error reading from device");
  return count;
}

int dev_write(devno_t devno, void *buffer, size_t count, blkno_t blkno)
{
  DWORD bytes;

  //printf("write block %d, %d bytes\n", blkno, count);

  if (SetFilePointer((HANDLE) devno, blkno * SECTORSIZE, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER) panic("unable to set file pointer");
  if (!WriteFile((HANDLE) devno, buffer, count, &bytes, NULL)) panic("error writing to device");
  return count;
}

unsigned int dev_getsize(devno_t devno)
{
  return GetFileSize((HANDLE) devno, NULL) / SECTORSIZE;
}

void init_handles()
{
  int i;

  for (i = 0; i < MAXHANDLES; i++)
  {
    htab[i].type = HANDLE_FREE;
    htab[i].data = NULL;
  }

  htab[0].type = HANDLE_DEV;
  htab[0].data = GetStdHandle(STD_INPUT_HANDLE);

  htab[1].type = HANDLE_DEV;
  htab[1].data = GetStdHandle(STD_OUTPUT_HANDLE);

  htab[2].type = HANDLE_DEV;
  htab[2].data = GetStdHandle(STD_ERROR_HANDLE);

  threadtls = TlsAlloc();
}

handle_t halloc(int type)
{
  int i;

  for (i = 0; i < MAXHANDLES; i++)
  {
    if (htab[i].type == HANDLE_FREE) 
    {
      htab[i].type = type;
      return i;
    }
  }

  panic("no more handles");
  return NOHANDLE;
}

void hfree(handle_t h)
{
  htab[h].type = HANDLE_FREE;
}

static time_t ft2time(FILETIME *ft)
{
  return (time_t) (div64x32(*(unsigned __int64 *) ft, SECTIMESCALE) - SECEPOC);
}

void *kmalloc(int size)
{
  return HeapAlloc(heap, 0, size);
}

void kfree(void *p)
{
  if (p) HeapFree(heap, 0, p);
}

void make_external_filename(char *name, char *fn)
{
  if (name[0] && name[1] == ':') name += 2;
  if (*name == PS1 || *name == PS2) name++;
  strcpy(fn, IMAGEDIR);
  strcpy(fn + strlen(IMAGEDIR), name);
}

void panic(char *msg)
{
  DWORD written;
  WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), "panic: ", 7, &written, NULL);
  WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), msg, strlen(msg), &written, NULL);
  WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), "\n", 1, &written, NULL);
  ExitProcess(1);
}

void print_string(char *s)
{
  DWORD written;
  WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), s, strlen(s), &written, NULL);
}

void set_break_handler(void (*handler)(int type))
{
  user_break_handler = handler;
}

void *mmap(void *addr, unsigned long size, int type, int protect)
{
  return VirtualAlloc(addr, size, type, protect);
}

int munmap(void *addr, unsigned long size, int type)
{
  if (type == MEM_DECOMMIT)
    return VirtualFree(addr, size, MEM_DECOMMIT) ? 0 : EFAIL;
  else if (type == MEM_RELEASE)
    return VirtualFree(addr, 0, MEM_RELEASE) ? 0 : EFAIL;
  else
    return EFAIL;
}

int close_handle(handle_t handle)
{
  if (handle == NOHANDLE) return -1;

  switch (htab[handle].type)
  {
    case HANDLE_FREE:
      panic("trying to close free handle");

    case HANDLE_DEV:
    case HANDLE_THREAD:
    case HANDLE_SEM:
    case HANDLE_EVENT:
      if (!CloseHandle((HANDLE) htab[handle].data)) return EFAIL;
      hfree(handle);
      return OK;

    case HANDLE_FILE:
#ifdef USEDFS
      if (close((struct file *) htab[handle].data) < 0) return EFAIL;
      hfree(handle);
      return OK;
#else
      if (!CloseHandle((HANDLE) htab[handle].data)) return EFAIL;
      hfree(handle);
      return OK;
#endif

    case HANDLE_DIR:
#ifdef USEDFS
      if (close((struct file *) htab[handle].data) < 0) return EFAIL;
      hfree(handle);
      return OK;
#else
      if (!FindClose(((struct finddata *) htab[handle].data)->fhandle)) return EFAIL;
      kfree(htab[handle].data);
      hfree(handle);
      return OK;
#endif

    default:
      panic("unknown handle type");
  }

  return -1;
}

handle_t dup_handle(handle_t handle)
{
  handle_t newhandle = halloc(htab[handle].type);

  switch (htab[handle].type)
  {
    case HANDLE_FREE:
      panic("trying to dup free handle");

    case HANDLE_DEV:
    case HANDLE_THREAD:
    case HANDLE_SEM:
    case HANDLE_EVENT:
      if (!DuplicateHandle(GetCurrentProcess(), htab[handle].data, GetCurrentProcess(), &htab[newhandle].data, 0, FALSE, DUPLICATE_SAME_ACCESS)) return NOHANDLE;
      break;

    case HANDLE_FILE:
    case HANDLE_DIR:
      panic("file/dir not implemented (dup)");

    default:
      panic("unknown handle type");
  }

  return newhandle;
}

handle_t get_std_handle(int handlenum)
{
  return handlenum;
}

char *get_file_path(char *filename, char *buffer, int size)
{
  //char *filepart = NULL;
  //GetFullPathName(filename, size, buffer, &filepart);
  //return filepart;

  char *basename = filename;
  char *p = filename;

  while (*p)
  {
    *buffer = *p;
    if (*buffer == PS1 || *buffer == PS2) basename = buffer + 1;
    buffer++;
    p++;
  }
  *buffer = 0;
  return basename;
}

int get_file_stat(char *filename, struct stat *buffer)
{
#ifdef USEDFS
  return stat(filename, buffer);
#else
  WIN32_FILE_ATTRIBUTE_DATA fdata;
  char fn[MAXPATH];

  make_external_filename(filename, fn);

  if (!GetFileAttributesEx(fn, GetFileExInfoStandard, &fdata)) return -1;

  if (buffer)
  {
    buffer->atime = ft2time(&fdata.ftLastAccessTime);
    buffer->ctime = ft2time(&fdata.ftCreationTime);
    buffer->mtime = ft2time(&fdata.ftCreationTime);
  
    buffer->quad.size_low = fdata.nFileSizeLow;
    buffer->quad.size_high = fdata.nFileSizeHigh;

    buffer->mode = 0;
    if (fdata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) buffer->mode |= FILE_STAT_DIRECTORY;
  }

  return fdata.nFileSizeLow;
#endif
}

int get_handle_stat(handle_t f, struct stat *buffer)
{
#ifdef USEDFS
  if (htab[f].type != HANDLE_FILE) panic("file expected in get_handle_stat");
  return fstat((struct file *) htab[f].data, buffer);
#else
  BY_HANDLE_FILE_INFORMATION fi;

  if (f == NOHANDLE) return -1;
  if (htab[f].type != HANDLE_FILE) panic("handle stat on non-file handle");

  if (!GetFileInformationByHandle(htab[f].data, &fi)) return -1;

  if (buffer)
  {
    buffer->atime = ft2time(&fi.ftLastAccessTime);
    buffer->ctime = ft2time(&fi.ftCreationTime);
    buffer->mtime = ft2time(&fi.ftLastWriteTime);
  
    buffer->quad.size_low = fi.nFileSizeLow;
    buffer->quad.size_high = fi.nFileSizeHigh;

    buffer->mode = 0;
  }

  return fi.nFileSizeLow;
#endif
}

int make_dir(char *dirname)
{
#ifdef USEDFS
  return mkdir(dirname);
#else
  char fn[MAXPATH];

  make_external_filename(dirname, fn);
  if (!CreateDirectory(fn, NULL)) return -1;
  return 0;
#endif
}

handle_t open_dir(char *filename)
{
#ifdef USEDFS
  handle_t h;
  struct file *filp;

  filp = opendir(filename);
  if (!filp) return EFAIL;

  h = halloc(HANDLE_DIR);
  htab[h].data = filp;
  return h;
#else
  char fn[MAXPATH];
  struct finddata *finddata;
  handle_t h;

  make_external_filename((char *) filename, fn);
  strcpy(fn + strlen(fn), "\\*");

  finddata = (struct finddata *) kmalloc(sizeof(struct finddata));
  finddata->fhandle = FindFirstFile(fn, &finddata->fdata);
  if (finddata->fhandle == INVALID_HANDLE_VALUE) return NOHANDLE;
  finddata->first = TRUE;

  h = halloc(HANDLE_DIR);
  htab[h].data = finddata;
  return h;
#endif
}

int read_dir(handle_t find, struct dirent *dirp, int count)
{
#ifdef USEDFS
  return readdir((struct file *) htab[find].data, dirp, count);
#else
  struct finddata *finddata = (struct finddata *) htab[find].data;

  if (!finddata->first)
  {
    if (!FindNextFile(finddata->fhandle, &finddata->fdata)) return 0;
  }

  finddata->first = FALSE;
  dirp->ino = -1;
  dirp->namelen = strlen(finddata->fdata.cFileName);
  dirp->reclen = sizeof(struct dirent) + dirp->namelen;
  strcpy(dirp->name, finddata->fdata.cFileName);
  return 1;
#endif
}

handle_t open_file(char *name, int mode)
{
#ifdef USEDFS
  handle_t f;
  struct file *filp;

  filp = open(name, mode);
  if (!filp) return NOHANDLE;

  f = halloc(HANDLE_FILE);
  htab[f].data = filp;
  
  return f;
#else
  handle_t f;
  HANDLE hfile;
  DWORD access;
  DWORD disp;
  char fn[MAXPATH];

  make_external_filename(name, fn);

  if (mode & O_RDONLY)
    access = GENERIC_READ;
  else if (mode & O_WRONLY)
    access = GENERIC_WRITE;
  else
    access = GENERIC_READ | GENERIC_WRITE;

  switch (mode & (O_CREAT | O_EXCL | O_TRUNC))
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
      return 0;
  }

  hfile = CreateFile(fn, access, 0, NULL, disp, FILE_ATTRIBUTE_NORMAL, NULL);
  if (hfile == INVALID_HANDLE_VALUE) return NOHANDLE;

  f = halloc(HANDLE_FILE);
  htab[f].data = hfile;
  
  return f;
#endif
}

int close_file(handle_t f)
{
  return close_handle(f);
}

int read_file(handle_t f, void *data, int len)
{
#ifdef USEDFS
  if (htab[f].type == HANDLE_FILE) 
    return read((struct file *) htab[f].data, data, len);
  else if (htab[f].type == HANDLE_DEV) 
  {
    DWORD bytes;

    if (!ReadFile((HANDLE) htab[f].data, data, len, &bytes, NULL)) return EFAIL;
    return bytes;
  }
  else
  {
    panic("file/device expected in read_file");
    return -1;
  }
#else
  DWORD bytes;

  if (!ReadFile((HANDLE) htab[f].data, data, len, &bytes, NULL)) return EFAIL;
  return bytes;
#endif
}

int write_file(handle_t f, void *data, int len)
{
#ifdef USEDFS
  if (htab[f].type == HANDLE_FILE) 
    return write((struct file *) htab[f].data, data, len);
  else if (htab[f].type == HANDLE_DEV) 
  {
    DWORD bytes;

    if (!WriteFile((HANDLE) htab[f].data, data, len, &bytes, NULL)) return EFAIL;
    return bytes;
  }
  else
  {
    panic("file/device expected in write_file");
    return -1;
  }
#else
  DWORD bytes;

  if (!WriteFile((HANDLE) htab[f].data, data, len, &bytes, NULL)) return EFAIL;
  return bytes;
#endif
}

int set_file_pointer(handle_t f, int offset, int origin)
{
#ifdef USEDFS
  if (htab[f].type != HANDLE_FILE) panic("file expected in set_file_pointer");
  return lseek((struct file *) htab[f].data, offset, origin);
#else
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
      return EFAIL;
  }

  return SetFilePointer((HANDLE) htab[f].data, offset, NULL, method);
#endif
}

handle_t begin_thread(void (__stdcall *startaddr)(void *), unsigned stacksize, void *arg, int suspended, int *threadid)
{
  handle_t h = halloc(HANDLE_THREAD);

  htab[h].data = CreateThread(NULL, stacksize, (unsigned long (__stdcall *)(void *)) startaddr, arg, suspended ? CREATE_SUSPENDED : 0, (DWORD *) &threadid);
  return h;
}

int suspend_thread(handle_t thread)
{
  return SuspendThread(htab[thread].data);
}

int resume_thread(handle_t thread)
{
  return ResumeThread(htab[thread].data);
}

void end_thread(int retval)
{
  ExitThread(retval);
}

handle_t get_cur_thread()
{
  handle_t h = (handle_t) TlsGetValue(threadtls);
  if (h == 0)
  {
    h = halloc(HANDLE_THREAD);
    htab[h].data = GetCurrentThread();
    TlsSetValue(threadtls, (void *) h);
  }

  return h;
}

unsigned long get_cur_thread_id()
{
  return GetCurrentThreadId();
}

int get_thread_context(handle_t thread, void *context)
{
  if (!GetThreadContext(htab[thread].data, context)) return -1;
  return 0;
}

int get_thread_priority(handle_t thread)
{
  return GetThreadPriority(htab[thread].data);
}

int set_thread_priority(handle_t thread, int priority)
{
  if (!SetThreadPriority(htab[thread].data, priority)) return -1;
  return 0;
}

void sleep(int millisecs)
{
  Sleep(millisecs);
}

int wait_for_object(handle_t obj, unsigned long timeout)
{
  return WaitForSingleObject(htab[obj].data, timeout);
}

int wait_for_objects(handle_t *objs, int count, int waitall, unsigned long timeout)
{
  HANDLE hobjs[4];
  int i;

  if (count > 4) panic("wait for more than four objects");
  for (i = 0; i < count; i++) hobjs[i] = htab[objs[i]].data;

  return WaitForMultipleObjects(count, hobjs, waitall, timeout);
}

tls_t alloc_tls()
{
  return TlsAlloc();
}

void free_tls(tls_t index)
{
  TlsFree(index);
}

void *get_tls(tls_t index)
{
  return TlsGetValue(index);
}

void set_tls(tls_t index, void *value)
{
  TlsSetValue(index, value);
}

handle_t create_event(int manual_reset, int initial_state)
{
  handle_t h = halloc(HANDLE_EVENT);

  htab[h].data = CreateEvent(NULL, manual_reset, initial_state, NULL);
  return h;
}

void set_event(handle_t event)
{
  SetEvent(htab[event].data);
}

void reset_event(handle_t event)
{
  ResetEvent(htab[event].data);
}

handle_t create_sem(unsigned int initial_count)
{
  handle_t h = halloc(HANDLE_SEM);

  htab[h].data = CreateSemaphore(NULL, initial_count, 0x7FFFFFFF, NULL);
  return h;
}

unsigned int release_sem(handle_t sem, unsigned int count)
{
  unsigned int prevcount;

  ReleaseSemaphore(htab[sem].data, count, &prevcount);
  return prevcount;
}

unsigned long get_tick_count()
{
  return GetTickCount();
}

systime_t get_time()
{
  FILETIME ft;

  GetSystemTimeAsFileTime(&ft);
  return div64x32(*(unsigned __int64 *) &ft, MILLITIMESCALE) - MILLIEPOC;
}

void systemtime_to_filetime(void *st, void *ft)
{
  SystemTimeToFileTime((const SYSTEMTIME *) st, (FILETIME *) ft);
}

void get_system_time(void *st)
{
  GetSystemTime((SYSTEMTIME *) st);
}

socket_t winsock_socket(int af, int type, int protocol)
{
  return socket(af, type, protocol);
}

int winsock_connect(socket_t s, const struct sockaddr *name, int namelen)
{
  return connect(s, name, namelen);
}

int winsock_listen(socket_t s, int backlog)
{
  return listen(s, backlog);
}

int winsock_bind(socket_t s, const struct sockaddr *name, int namelen)
{
  return bind(s, name, namelen);
}

socket_t winsock_accept(socket_t s, struct sockaddr *addr, int *addrlen)
{
  return accept(s, addr, addrlen);
}

int winsock_closesocket(socket_t s)
{
  return closesocket(s);
}

int winsock_shutdown(socket_t s, int how)
{
  return shutdown(s, how);
}

int winsock_select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, const struct timeval *timeout)
{
  return select(nfds, readfds, writefds, exceptfds, timeout);
}

int winsock_recv(socket_t s,char *buf, int len, int flags)
{
  return recv(s, buf, len, flags);
}

int winsock_send(socket_t s, const char *buf, int len, int flags)
{
  return send(s, buf, len, flags);
}

int winsock_recvfrom(socket_t s, char *buf, int len, int flags, struct sockaddr *from, int *fromlen)
{
  return recvfrom(s, buf, len, flags, from, fromlen);
}

int winsock_sendto(socket_t s, const char *buf, int len, int flags, const struct sockaddr *to, int tolen)
{
  return sendto(s, buf, len, flags, to, tolen);
}

int winsock_gethostname(char *name, int namelen)
{
  return gethostname(name, namelen);
}

struct hostent *winsock_gethostbyaddr(const char *addr, int len, int type)
{
  return gethostbyaddr(addr, len, type);
}

struct hostent *winsock_gethostbyname(const char *name)
{
  return gethostbyname(name);
}

struct protoent *winsock_getprotobyname(const char *name)
{
  return getprotobyname(name);
}

unsigned short winsock_htons(unsigned short hostshort)
{
  return htons(hostshort);
}

unsigned long winsock_htonl(unsigned long hostlong)
{
  return htonl(hostlong);
}

unsigned short winsock_ntohs(unsigned short netshort)
{
  return ntohs(netshort);
}

unsigned long winsock_ntohl(unsigned long netlong)
{
  return ntohl(netlong);
}

int winsock_getsockopt(socket_t s, int level, int optname, char *optval, int *optlen)
{
  return getsockopt(s, level, optname, optval, optlen);
}

int winsock_setsockopt(socket_t s, int level, int optname, const char *optval, int optlen)
{
  return setsockopt(s, level, optname, optval, optlen);
}

int winsock_getsockname(socket_t s, struct sockaddr *name, int *namelen)
{
  return getsockname(s, name, namelen);
}

int winsock_ioctlsocket(socket_t s, long cmd, unsigned long *argp)
{
  return ioctlsocket(s, cmd, argp);
}

int  winsock___WSAFDIsSet(socket_t s, fd_set *fd)
{
  return __WSAFDIsSet(s, fd);
}

int winsock_WSAGetLastError()
{
  return WSAGetLastError();
}

struct sockcallops sockcalls =
{
  winsock_socket,
  winsock_connect,
  winsock_listen,
  winsock_bind,
  winsock_accept,
  winsock_closesocket,
  winsock_shutdown,

  winsock_select,

  winsock_recv,
  winsock_send,
  winsock_recvfrom,
  winsock_sendto,

  winsock_gethostname,
  winsock_gethostbyaddr,
  winsock_gethostbyname,
  winsock_getprotobyname,

  winsock_htons,
  winsock_htonl,
  winsock_ntohs,
  winsock_ntohl,

  winsock_getsockopt,
  winsock_setsockopt,

  winsock_getsockname,
  winsock_ioctlsocket,

  winsock___WSAFDIsSet,
  winsock_WSAGetLastError
};

struct sockcallops *get_sock_calls()
{
  return &sockcalls;
}

struct syscallops syscalls =
{
  panic,
  print_string,
  set_break_handler,

  mmap,
  munmap,

  close_handle,
  dup_handle,
  get_std_handle,

  get_file_path,
  get_file_stat,
  get_handle_stat,

  make_dir,
  open_dir,
  read_dir,

  open_file,
  close_file,
  read_file,
  write_file,
  set_file_pointer,

  begin_thread,
  suspend_thread,
  resume_thread,
  end_thread,
  get_cur_thread,
  get_cur_thread_id,
  get_thread_context,
  get_thread_priority,
  set_thread_priority,
  sleep,

  wait_for_object,
  wait_for_objects,

  alloc_tls,
  free_tls,
  get_tls,
  set_tls,

  create_event,
  set_event,
  reset_event,
  create_sem,
  release_sem,

  get_tick_count,
  get_time,

  get_sock_calls,

  systemtime_to_filetime,
  get_system_time
};

time_t time(time_t *timer)
{
  return (time_t) div64x32(get_time(), 1000);
}

EXCEPTION_DISPOSITION __cdecl except_handler(struct _EXCEPTION_RECORD *ExceptionRecord, void *EstablisherFrame, struct _CONTEXT *ContextRecord, void *DispatcherContext)
{
  EXCEPTION_DISPOSITION disposition = ExceptionContinueSearch;

#if 0
  if (ExceptionRecord->ExceptionFlags == 0)
  {
    LPTOP_LEVEL_EXCEPTION_FILTER handler = get_global_exception_handler();
 
    if (handler != NULL)
    {
      EXCEPTION_POINTERS excpt_info;
      excpt_info.ExceptionRecord = ExceptionRecord;
      excpt_info.ContextRecord = ContextRecord;

      LONG rc = handler(&excpt_info);
      if (rc == EXCEPTION_CONTINUE_EXECUTION) disposition = ExceptionContinueExecution;
    }
  }
#endif
  panic("Unhandled exception\n");

  return disposition;
}

int execute(void *startaddr, void *hmod, struct syscallops *syscalls, char *cmdline)
{
  int rc;

#if 0
  DWORD __haddr = (DWORD) except_handler;

  __asm { push __haddr }
  __asm { push fs:[0] }
  __asm { mov  fs:[0], esp }
#endif

  rc = ((int (__stdcall *)(void *, struct syscallops *, char *)) startaddr)(hmod, syscalls, cmdline);
  
#if 0
  __asm { mov  eax, esp }
  __asm { mov  fs:[0], eax }
  __asm { add esp, 8 }
#endif

  return rc;
}

void close_all()
{
  int i;

  for (i = 0; i < MAXHANDLES; i++)
  {
    if (htab[i].type == HANDLE_FILE || htab[i].type == HANDLE_DIR) close_handle(i);
  }
}

BOOL WINAPI break_handler(DWORD ctrl_type)
{
  if (ctrl_type == CTRL_BREAK_EVENT || ctrl_type ==  CTRL_C_EVENT)
  {
    print_string("break\n");
    if (user_break_handler) user_break_handler(ctrl_type);

#ifdef USEDFS
  print_string("sync filsystem\n");
  close_all();
  unmount_all();
  CloseHandle(hdev);
#endif
    
    ExitProcess(1);
    //return TRUE;
  }
  else
    return FALSE;
}

void dfs_init();

int main(char *cmdline)
{
  HANDLE hfile;
  DWORD bytes;
  DWORD filesize;
  IMAGE_DOS_HEADER doshdr;
  IMAGE_HEADER imghdr;
  char *image;
  char *entrypoint;
  int rc;
  WSADATA wsadata;

  // Get handle to process heap
  heap = GetProcessHeap();

  // Initialize handles
  init_handles();

  // Initialize filesystem
#ifdef USEDFS
  dfs_init();
  hdev = CreateFile(IMAGEFILE, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, 0, NULL);
  if (hdev == INVALID_HANDLE_VALUE) panic("unable to open device");
  mount("dfs", "/", (devno_t) hdev, NULL);
#endif

  // Initialize winsock
  if (WSAStartup(MAKEWORD(1,1), &wsadata) != 0) return 3;

  // Open os.dll
  hfile = CreateFile(IMAGEDIR "/os/os.dll", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
  if (hfile == INVALID_HANDLE_VALUE) panic("unable to find os.dll");

  // Get file size
  filesize = GetFileSize(hfile, NULL);

  // Read DOS header
  ReadFile(hfile, &doshdr, sizeof(doshdr), &bytes, NULL);
  
  // Read image header
  SetFilePointer(hfile, doshdr.e_lfanew, NULL, FILE_BEGIN);
  ReadFile(hfile, &imghdr, sizeof(imghdr), &bytes, NULL);

  // Reserve and allocate memory for os.dll
  image = (char *) VirtualAlloc((void *) imghdr.optional.ImageBase, imghdr.optional.SizeOfImage, MEM_RESERVE, PAGE_EXECUTE_READWRITE);
  if (image == NULL) panic("unable to reserve memory region for os.dll");

  image = VirtualAlloc(image, imghdr.optional.SizeOfImage, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
  if (image == NULL) panic("unable to commit memory region for os.dll");

  // Read os.dll image into memory
  SetFilePointer(hfile, 0, NULL, FILE_BEGIN);
  ReadFile(hfile, image, filesize, &bytes, NULL);

  // Close os.dll
  CloseHandle(hfile);

  // Find entry point for os.dll
  entrypoint = image + imghdr.optional.AddressOfEntryPoint;

  // install ctrl-c handler
  SetConsoleCtrlHandler(break_handler, TRUE);

  // Call entry point for os.dll
  rc = execute(entrypoint, image, &syscalls, cmdline);

  // Clean up winsock
  WSACleanup();

  // Return exit code
  return rc;
}

void __stdcall start()
{
  int rc = main(GetCommandLine());
  ExitProcess(rc);
}
