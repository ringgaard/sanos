//
// sow.c
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Sanos on Win32
//

#define handle_t oshandle_t
#define STRING_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#undef handle_t
#undef STRING_H

#define OS_LIB
#include <os.h>
#include <inifile.h>

#define IMAGEDIR   "\\sanos\\image\\"
#define MAXHANDLES 1024

#define SECTIMESCALE    10000000               // 1 sec resolution
#define SECEPOC         11644473600            // 00:00:00 GMT on January 1, 1970 (in secs) 

#define HANDLE_FREE    0x1000
#define HANDLE_DEV     0x1001
#define HANDLE_THREAD  0x1002
#define HANDLE_SEM     0x1003
#define HANDLE_EVENT   0x1004
#define HANDLE_FILE    0x1005
#define HANDLE_DIR     0x1006

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

//
// Globals
//

struct handle htab[MAXHANDLES];
DWORD threadtls;
struct section *config;
unsigned long loglevel;

//
// Intrinsic functions
//

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
// Handles
//

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
  return -ENFILE;
}

void hfree(handle_t h)
{
  htab[h].type = HANDLE_FREE;
}

//
// Utility routines
//

void make_external_filename(const char *name, char *fn)
{
  if (name[0] && name[1] == ':') name += 2;
  if (*name == PS1 || *name == PS2) name++;
  strcpy(fn, IMAGEDIR);
  strcpy(fn + strlen(IMAGEDIR), name);
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
  return -ENOSYS;
}

int format(const char *devname, const char *type, const char *opts)
{
  return -ENOSYS;
}

int mount(const char *type, const char *path, const char *devname, const char *opts)
{
  return -ENOSYS;
}

int unmount(const char *path)
{
  return -ENOSYS;
}

handle_t open(const char *name, int mode)
{
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
}

int close(handle_t h)
{
  if (h == NOHANDLE) return -EINVAL;

  switch (htab[h].type)
  {
    case HANDLE_FREE:
      //panic("trying to close free handle");
      return -EBADF;

    case HANDLE_DEV:
    case HANDLE_THREAD:
    case HANDLE_SEM:
    case HANDLE_EVENT:
      if (!CloseHandle((HANDLE) htab[h].data)) return -EBADF;
      hfree(h);
      return 0;

    case HANDLE_FILE:
      if (!CloseHandle((HANDLE) htab[h].data)) return -EBADF;
      hfree(h);
      return 0;

    case HANDLE_DIR:
      if (!FindClose(((struct finddata *) htab[h].data)->fhandle)) return -EBADF;
      free(htab[h].data);
      hfree(h);
      return 0;

    default:
      //panic("unknown handle type");
      return -EBADF;
  }

  return -EBADF;
}

int flush(handle_t f)
{
  if (f == NOHANDLE) return -EINVAL;
  if (htab[f].type == HANDLE_FILE || htab[f].type == HANDLE_DEV)
  {
    if (!FlushFileBuffers((HANDLE) htab[f].data)) return -EBADF;
  }

  return 0;
}

handle_t dup(handle_t h)
{
  handle_t newh = halloc(htab[h].type);

  switch (htab[h].type)
  {
    case HANDLE_FREE:
      //panic("trying to dup free handle");
      return -EBADF;

    case HANDLE_DEV:
    case HANDLE_THREAD:
    case HANDLE_SEM:
    case HANDLE_EVENT:
    case HANDLE_FILE:
      if (!DuplicateHandle(GetCurrentProcess(), htab[h].data, GetCurrentProcess(), &htab[newh].data, 0, FALSE, DUPLICATE_SAME_ACCESS)) return -EBADF;
      break;

    case HANDLE_DIR:
      return -EBADF;
      break;

    default:
      //panic("unknown handle type");
      return -EBADF;
  }

  return newh;
}

int read(handle_t f, void *data, size_t size)
{
  DWORD bytes;

  if (!ReadFile((HANDLE) htab[f].data, data, size, &bytes, NULL)) return -EBADF;
  return bytes;
}

int write(handle_t f, const void *data, size_t size)
{
  DWORD bytes;

  if (!WriteFile((HANDLE) htab[f].data, data, size, &bytes, NULL)) return -EBADF;
  return bytes;
}

int ioctl(handle_t f, int cmd, const void *data, size_t size)
{
  return -ENOSYS;
}

loff_t tell(handle_t f)
{
  return SetFilePointer((HANDLE) htab[f].data, 0, NULL, FILE_CURRENT);
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
      return -EINVAL;
  }

  return SetFilePointer((HANDLE) htab[f].data, offset, NULL, method);
}

int chsize(handle_t f, loff_t size)
{
  BOOL ok;
  loff_t curpos;

  curpos = SetFilePointer((HANDLE) htab[f].data, size, NULL, FILE_BEGIN);
  ok = SetEndOfFile((HANDLE) htab[f].data);
  SetFilePointer((HANDLE) htab[f].data, curpos, NULL, FILE_BEGIN);

  return ok ? 0 : -EBADF;
}

int fstat(handle_t f, struct stat *buffer)
{
  BY_HANDLE_FILE_INFORMATION fi;

  if (f == NOHANDLE) return -EINVAL;
  if (htab[f].type != HANDLE_FILE) return -EBADF;

  if (!GetFileInformationByHandle(htab[f].data, &fi)) return -EBADF;

  if (buffer)
  {
    buffer->ino = 0;
    buffer->nlink = 1;
    buffer->devno = -1;
    buffer->atime = ft2time(&fi.ftLastAccessTime);
    buffer->ctime = ft2time(&fi.ftCreationTime);
    buffer->mtime = ft2time(&fi.ftLastWriteTime);
  
    buffer->quad.size_low = fi.nFileSizeLow;
    buffer->quad.size_high = fi.nFileSizeHigh;

    buffer->mode = 0;
  }

  return fi.nFileSizeLow;
}

int stat(const char *name, struct stat *buffer)
{
  WIN32_FILE_ATTRIBUTE_DATA fdata;
  char fn[MAXPATH];

  make_external_filename(name, fn);

  if (!GetFileAttributesEx(fn, GetFileExInfoStandard, &fdata)) return -ENOENT;

  if (buffer)
  {
    buffer->ino = 0;
    buffer->nlink = 1;
    buffer->devno = -1;
    buffer->atime = ft2time(&fdata.ftLastAccessTime);
    buffer->ctime = ft2time(&fdata.ftCreationTime);
    buffer->mtime = ft2time(&fdata.ftCreationTime);
  
    buffer->quad.size_low = fdata.nFileSizeLow;
    buffer->quad.size_high = fdata.nFileSizeHigh;

    buffer->mode = 0;
    if (fdata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) buffer->mode |= FS_DIRECTORY;
  }

  return fdata.nFileSizeLow;
}

int mkdir(const char *name)
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

handle_t opendir(const char *name)
{
  char fn[MAXPATH];
  struct finddata *finddata;
  handle_t h;

  make_external_filename((char *) name, fn);
  strcpy(fn + strlen(fn), "\\*");

  finddata = (struct finddata *) malloc(sizeof(struct finddata));
  finddata->fhandle = FindFirstFile(fn, &finddata->fdata);
  if (finddata->fhandle == INVALID_HANDLE_VALUE) return -ENOENT;
  finddata->first = TRUE;

  h = halloc(HANDLE_DIR);
  htab[h].data = finddata;
  return h;
}

int readdir(handle_t f, struct dirent *dirp, int count)
{
  struct finddata *finddata = (struct finddata *) htab[f].data;

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
}

void *mmap(void *addr, unsigned long size, int type, int protect)
{
  return VirtualAlloc(addr, size, type, protect);
}

int munmap(void *addr, unsigned long size, int type)
{
  if (type == MEM_DECOMMIT)
    return VirtualFree(addr, size, MEM_DECOMMIT) ? 0 : -EFAULT;
  else if (type == MEM_RELEASE)
    return VirtualFree(addr, 0, MEM_RELEASE) ? 0 : -EFAULT;
  else
    return -EINVAL;
}

void *mremap(void *addr, unsigned long oldsize, unsigned long newsize, int type, int protect)
{
  //panic("mremap not yet implemented");
  return NULL;
}

int mprotect(void *addr, unsigned long size, int protect)
{
  DWORD oldprotect;

  return VirtualProtect(addr, size, protect, &oldprotect) ? 0 : -EFAULT;
}

int mlock(void *addr, unsigned long size)
{
  return 0;
}

int munlock(void *addr, unsigned long size)
{
  return 0;
}

int wait(handle_t h, int timeout)
{
  return WaitForSingleObject(htab[h].data, timeout) == WAIT_TIMEOUT ? -ETIMEDOUT : 0;
}

handle_t mkevent(int manual_reset, int initial_state)
{
  handle_t h = halloc(HANDLE_EVENT);

  htab[h].data = CreateEvent(NULL, manual_reset, initial_state, NULL);
  return h;
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
  handle_t h = halloc(HANDLE_SEM);

  htab[h].data = CreateSemaphore(NULL, initial_count, 0x7FFFFFFF, NULL);
  return h;
}

int semrel(handle_t h, int count)
{
  unsigned int prevcount;

  ReleaseSemaphore(htab[h].data, count, &prevcount);
  return prevcount;
}

handle_t self()
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

void exit(int status)
{
  ExitProcess(status);
}

handle_t beginthread(void (__stdcall *startaddr)(void *), unsigned stacksize, void *arg, int suspended, tid_t *tid)
{
  handle_t h = halloc(HANDLE_THREAD);

  htab[h].data = CreateThread(NULL, stacksize, (unsigned long (__stdcall *)(void *)) startaddr, arg, suspended ? CREATE_SUSPENDED : 0, (DWORD *) &tid);
  return h;
}

int suspend(handle_t thread)
{
  return SuspendThread(htab[thread].data);
}

int resume(handle_t thread)
{
  return ResumeThread(htab[thread].data);
}

void endthread(int retval)
{
  ExitThread(retval);
}

tid_t gettid()
{
  return GetCurrentThreadId();
}

int getcontext(handle_t thread, void *context)
{
  if (!GetThreadContext(htab[thread].data, context)) return -EINVAL;
  return 0;
}

int getprio(handle_t thread)
{
  int prio;

  prio = GetThreadPriority(htab[thread].data);

  switch (prio)
  {
    case THREAD_PRIORITY_IDLE: 
      return PRIORITY_IDLE;

    case THREAD_PRIORITY_LOWEST: 
      return PRIORITY_LOWEST;

    case THREAD_PRIORITY_BELOW_NORMAL: 
      return PRIORITY_BELOW_NORMAL;

    case THREAD_PRIORITY_NORMAL: 
      return PRIORITY_NORMAL;

    case THREAD_PRIORITY_ABOVE_NORMAL: 
      return PRIORITY_ABOVE_NORMAL;

    case THREAD_PRIORITY_HIGHEST: 
      return PRIORITY_HIGHEST;

    case THREAD_PRIORITY_TIME_CRITICAL: 
      return PRIORITY_TIME_CRITICAL;
  }

  return -EINVAL;
}

int setprio(handle_t thread, int priority)
{
  int prio;

  switch (priority)
  {
    case PRIORITY_IDLE: 
      prio = THREAD_PRIORITY_IDLE; 
      break;

    case PRIORITY_LOWEST: 
      prio = THREAD_PRIORITY_LOWEST; 
      break;

    case PRIORITY_BELOW_NORMAL: 
      prio = THREAD_PRIORITY_BELOW_NORMAL; 
      break;

    case PRIORITY_NORMAL: 
      prio = THREAD_PRIORITY_NORMAL; 
      break;

    case PRIORITY_ABOVE_NORMAL: 
      prio = THREAD_PRIORITY_ABOVE_NORMAL; 
      break;

    case PRIORITY_HIGHEST: 
      prio = THREAD_PRIORITY_HIGHEST; 
      break;

    case PRIORITY_TIME_CRITICAL: 
      prio = THREAD_PRIORITY_TIME_CRITICAL; 
      break;

    default:
      return -EINVAL;
  }

  if (!SetThreadPriority(htab[thread].data, priority)) return -EINVAL;
  return 0;
}

void sleep(int millisecs)
{
  Sleep(millisecs);
}

struct tib *gettib()
{
  struct tib *tib;

  __asm
  {
    mov eax, fs:[TIB_SELF_OFFSET]
    mov [tib], eax
  }

  return tib;
}

time_t time()
{
  FILETIME ft;

  GetSystemTimeAsFileTime(&ft);
  return ft2time(&ft);
}

int gettimeofday(struct timeval *tv)
{
  FILETIME ft;

  GetSystemTimeAsFileTime(&ft);
  tv->tv_sec = ft2time(&ft);
  tv->tv_usec = ft.dwLowDateTime / 10;
  return tv->tv_usec;
}

clock_t clock()
{
  return GetTickCount();
}

void panic(const char *msg)
{
  DWORD written;
  WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), "panic: ", 7, &written, NULL);
  WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), msg, strlen(msg), &written, NULL);
  WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), "\n", 1, &written, NULL);
  ExitProcess(1);
}

void syslog(int priority, const char *fmt,...)
{
  // implement
}

char *canonicalize(const char *filename, char *buffer, int size)
{
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
  return basename;
}

void mkcs(critsect_t cs)
{
  InitializeCriticalSection((LPCRITICAL_SECTION) cs);
}

void csfree(critsect_t cs)
{
  DeleteCriticalSection((LPCRITICAL_SECTION) cs);
}

void enter(critsect_t cs)
{
  EnterCriticalSection((LPCRITICAL_SECTION) cs);
}

void leave(critsect_t cs)
{
  LeaveCriticalSection((LPCRITICAL_SECTION) cs);
}

void *malloc(size_t size)
{
  return HeapAlloc(GetProcessHeap(), 0, size);
}

void *realloc(void *mem, size_t size)
{
  return HeapReAlloc(GetProcessHeap(), 0, mem, size);
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

void *resolve(hmodule_t hmod, const char *procname)
{
  return GetProcAddress(hmod, procname);
}

hmodule_t getmodule(char *name)
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

hmodule_t load(const char *name)
{
  char fn[MAXPATH];

  make_external_filename(name, fn);
  return LoadLibrary(fn);
}

int unload(hmodule_t hmod)
{
  return FreeLibrary(hmod) ? 0 : -EINVAL;
}

int exec(hmodule_t hmod, char *args)
{
  IMAGE_DOS_HEADER *doshdr;
  IMAGE_HEADER *imghdr;
  void *entrypoint;

  doshdr = (IMAGE_DOS_HEADER *) hmod;
  imghdr = (IMAGE_HEADER *) (((char *) hmod) + doshdr->e_lfanew);
  entrypoint = ((char *) hmod) + imghdr->optional.AddressOfEntryPoint;

  return ((int (__stdcall *)(hmodule_t, char *, int)) entrypoint)(hmod, args, 0);
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

void tlsset(tls_t index, void *value)
{
  TlsSetValue(index, value);
}

int __stdcall dllmain(hmodule_t hmodule, unsigned long reason, void *reserved)
{
  switch (reason)
  {
    case DLL_PROCESS_ATTACH:
      init_handles();
      break;

    case DLL_PROCESS_DETACH:
      break;
  }

  return TRUE;
}
