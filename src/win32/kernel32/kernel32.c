//
// kernel32.c
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Win32 KERNEL32 emulation
//

#include <os.h>
#include <os/trap.h>
#include <win32.h>
#include <string.h>
#include <inifile.h>

#define EPOC            116444736000000000     // 00:00:00 GMT on January 1, 1970

#define MICROTIMESCALE  10                     // 1 us resolution
#define MILLITIMESCALE  10000                  // 1 ms resolution
#define SECTIMESCALE    10000000               // 1 sec resolution

#define NOFINDHANDLE ((HANDLE) (-2))

struct winfinddata
{
  handle_t fhandle;
  char dir[MAXPATH];
  char *mask;
};

void convert_from_win32_context(struct context *ctxt, CONTEXT *ctx)
{
  if (ctx->ContextFlags & CONTEXT_CONTROL)
  {
    ctxt->ess = ctx->SegSs;
    ctxt->esp = ctx->Esp;
    ctxt->ecs = ctx->SegCs;
    ctxt->eip = ctx->Eip;
    ctxt->eflags = ctx->EFlags;
    ctxt->ebp = ctx->Ebp;
  }

  if (ctx->ContextFlags & CONTEXT_INTEGER)
  {
    ctxt->eax = ctx->Eax;
    ctxt->ebx = ctx->Ebx;
    ctxt->ecx = ctx->Ecx;
    ctxt->edx = ctx->Edx;
    ctxt->esi = ctx->Esi;
    ctxt->edi = ctx->Edi;
  }

  if (ctx->ContextFlags & CONTEXT_SEGMENTS)
  {
    ctxt->ds = ctx->SegDs;
    ctxt->es = ctx->SegEs;
    // fs missing
    // gs missing
  }

  if (ctx->ContextFlags & CONTEXT_FLOATING_POINT)
  {
    // fpu state missing
  }
 
  if (ctx->ContextFlags & CONTEXT_FLOATING_POINT)
  {
    // fpu state missing
  }

  if (ctx->ContextFlags & CONTEXT_DEBUG_REGISTERS)
  {
    // debug registers missing
  }

  ctxt->traptype = 0;
  ctxt->errcode = 0;
}

void convert_to_win32_context(struct context *ctxt, CONTEXT *ctx)
{
  memset(ctx, 0, sizeof(CONTEXT));
  ctx->ContextFlags = CONTEXT_CONTROL | CONTEXT_INTEGER | CONTEXT_SEGMENTS;

  ctx->SegSs = ctxt->ess;
  ctx->Esp = ctxt->esp;
  ctx->SegCs = ctxt->ecs;
  ctx->Eip = ctxt->eip;
  ctx->EFlags = ctxt->eflags;
  ctx->Ebp = ctxt->ebp;

  ctx->Eax = ctxt->eax;
  ctx->Ebx = ctxt->ebx;
  ctx->Ecx = ctxt->ecx;
  ctx->Edx = ctxt->edx;
  ctx->Esi = ctxt->esi;
  ctx->Edi = ctxt->edi;

  ctx->SegDs = ctxt->ds;
  ctx->SegEs = ctxt->es;
  ctx->SegFs = 0;
  ctx->SegGs = 0;
}

BOOL WINAPI CloseHandle
(
  HANDLE hObject
)
{
  TRACE("CloseHandle");
  if (close((handle_t) hObject) < 0) return FALSE;
  return TRUE;
}

HANDLE WINAPI CreateEventA
(
  LPSECURITY_ATTRIBUTES lpEventAttributes,
  BOOL bManualReset,
  BOOL bInitialState,
  LPCTSTR lpName
)
{
  TRACE("CreateEventA");
  if (lpName != NULL) panic("named event not supported");
  return (HANDLE) mkevent(bManualReset, bInitialState);
}

HANDLE WINAPI CreateFileA
(
  LPCTSTR lpFileName, 
  DWORD dwDesiredAccess, 
  DWORD dwShareMode, 
  LPSECURITY_ATTRIBUTES lpSecurityAttributes, 
  DWORD dwCreationDisposition, 
  DWORD dwFlagsAndAttributes, 
  HANDLE hTemplateFile
)
{
  TRACE("CreateFileA");
  panic("CreateFileA not implemented");
  return 0;
}

BOOL WINAPI CreatePipe
(
  PHANDLE hReadPipe, 
  PHANDLE hWritePipe, 
  LPSECURITY_ATTRIBUTES lpPipeAttributes, 
  DWORD nSize
)
{
  TRACE("CreatePipe");
  panic("CreatePipe not implemented");
  return FALSE;
}

BOOL WINAPI CreateProcessA
(
  LPCTSTR lpApplicationName,
  LPTSTR lpCommandLine,
  LPSECURITY_ATTRIBUTES lpProcessAttributes,
  LPSECURITY_ATTRIBUTES lpThreadAttributes,
  BOOL bInheritHandles,
  DWORD dwCreationFlags,
  LPVOID lpEnvironment,
  LPCTSTR lpCurrentDirectory,
  LPSTARTUPINFO lpStartupInfo,
  LPPROCESS_INFORMATION lpProcessInformation
)
{
  TRACE("CreateProcessA");
  panic("CreateProcessA not implemented");
  return FALSE;
}

HANDLE WINAPI CreateSemaphoreA
(
  LPSECURITY_ATTRIBUTES lpSemaphoreAttributes,
  LONG lInitialCount,
  LONG lMaximumCount,
  LPCTSTR lpName
)
{
  TRACE("CreateSemaphoreA");
  if (lpName != NULL) panic("named semaphores not supported");
  syslog(LOG_DEBUG, "create sem with (%d,%d)\n", lInitialCount, lMaximumCount);
  return (HANDLE) mksem(lInitialCount);
}

VOID WINAPI DebugBreak(VOID)
{
  TRACE("DebugBreak");
  panic("DebugBreak not implemented");
}

BOOL WINAPI DeleteFileA
(
  LPCTSTR lpFileName
)
{
  TRACE("DeleteFileA");
  if (unlink(lpFileName) < 0) return FALSE;
  return TRUE;
}

BOOL WINAPI DisableThreadLibraryCalls
(
  HMODULE hModule
)
{
  TRACE("DisableThreadLibraryCalls");
  // ignore
  return TRUE;
}

BOOL WINAPI DuplicateHandle
(
  HANDLE hSourceProcessHandle,
  HANDLE hSourceHandle,
  HANDLE hTargetProcessHandle,
  LPHANDLE lpTargetHandle,
  DWORD dwDesiredAccess,
  BOOL bInheritHandle,
  DWORD dwOptions
)
{
  TRACE("DuplicateHandle");
  *lpTargetHandle = (HANDLE) dup((handle_t) hSourceHandle);
  return TRUE;
}

VOID WINAPI EnterCriticalSection
(
  LPCRITICAL_SECTION lpCriticalSection
)
{
  TRACEX("EnterCriticalSection");
  enter(lpCriticalSection);
}

static void fill_find_data(LPWIN32_FIND_DATA fd, char *filename, struct stat *statbuf)
{
  memset(fd, 0, sizeof(WIN32_FIND_DATA));
  strcpy(fd->cFileName, filename);

  if (statbuf->mode & FS_DIRECTORY) 
    fd->dwFileAttributes |= FILE_ATTRIBUTE_DIRECTORY;
  else
    fd->dwFileAttributes |= FILE_ATTRIBUTE_NORMAL;

  fd->nFileSizeLow = statbuf->quad.size_low;
  fd->nFileSizeHigh = statbuf->quad.size_high;

  *(__int64 *) &(fd->ftCreationTime) = (__int64) statbuf->ctime * SECTIMESCALE + EPOC;
  *(__int64 *) &(fd->ftLastAccessTime) = (__int64) statbuf->mtime * SECTIMESCALE + EPOC;
  *(__int64 *) &(fd->ftLastWriteTime) = (__int64) statbuf->mtime * SECTIMESCALE + EPOC;
}

static int like(char *str, char *mask)
{
  if (!str) str = "";
  if (!mask) mask = "";

  while (*mask)
  {
    if (*mask == '*')
    {
      while (*mask == '*') mask++;
      if (*mask == 0) return 1;
      while (*str)
      {
        if (like(str, mask)) return 1;
        str++;
      }
      return 0;
    }
    else if (*mask == *str || *mask == '?' && *str != 0)
    {
      str++;
      mask++;
    }
    else
      return 0;
  }

  return *str == 0;
}

BOOL WINAPI FindClose
(
  HANDLE hFindFile
)
{
  struct winfinddata *finddata;

  TRACE("FindClose");
  if (hFindFile == NOFINDHANDLE)
    return TRUE;
  else if (hFindFile == INVALID_HANDLE_VALUE)
    return FALSE;
  else
  {
    finddata = (struct winfinddata *) hFindFile;

    close(finddata->fhandle);
    free(finddata);

    return TRUE;
  }
}

HANDLE WINAPI FindFirstFileA
(
  LPCTSTR lpFileName,
  LPWIN32_FIND_DATA lpFindFileData
)
{
  struct winfinddata *finddata;
  struct dirent dirent;
  struct stat statbuf;
  char *p;
  char *base;
  char fn[MAXPATH];

  TRACE("FindFirstFileA");
  syslog(LOG_DEBUG | LOG_AUX, "FindFirstFile %s\n", lpFileName);
  p = (char *) lpFileName;
  while (*p != 0 && *p != '*' && *p != '?') p++;
  
  if (*p)
  {
    finddata = (struct winfinddata *) malloc(sizeof(struct winfinddata));
    if (!finddata) return INVALID_HANDLE_VALUE;

    strcpy(finddata->dir, lpFileName);
    base = NULL;
    p = finddata->dir;
    while (*p)
    {
      if (*p == PS1 || *p == PS2) base = p + 1;
      p++;
    }
    if (!base)
    {
      free(finddata);
      return INVALID_HANDLE_VALUE;
    }
    base[-1] = 0;
    finddata->mask = base;

    if (finddata->mask[0] == '*' && finddata->mask[1] == '.' && finddata->mask[2] == '*' && finddata->mask[3] == 0) 
    {
      finddata->mask[1] = 0;
    }

    finddata->fhandle = opendir(finddata->dir);
    if (finddata->fhandle < 0)
    {
      free(finddata);
      return INVALID_HANDLE_VALUE;
    }

    while (readdir(finddata->fhandle, &dirent, 1) > 0)
    {
      syslog(LOG_DEBUG | LOG_AUX, "match %s with %s\n", dirent.name, finddata->mask);
      if (like(dirent.name, finddata->mask))
      {
	strcpy(fn, finddata->dir);
	strcpy(fn + strlen(fn), "\\");
	strcpy(fn + strlen(fn), dirent.name);

        if (stat(fn, &statbuf) < 0) 
	{
          free(finddata);
	  return INVALID_HANDLE_VALUE;
	}

        fill_find_data(lpFindFileData, dirent.name, &statbuf);
	return (HANDLE) finddata;
      }
    }

    free(finddata);
    return INVALID_HANDLE_VALUE;
  }
  else
  {
    if (stat((char *) lpFileName, &statbuf) < 0) return INVALID_HANDLE_VALUE;

    p = (char *) lpFileName;
    base = p;
    while (*p)
    {
      if (*p == PS1 || *p == PS2) base = p + 1;
      p++;
    }

    fill_find_data(lpFindFileData, base, &statbuf);

    return NOFINDHANDLE;
  }
}

BOOL WINAPI FindNextFileA
(
  HANDLE hFindFile,
  LPWIN32_FIND_DATA lpFindFileData
)
{
  struct winfinddata *finddata;
  struct dirent dirent;
  struct stat statbuf;
  char fn[MAXPATH];

  TRACE("FindNextFileA");
  if (hFindFile == NOFINDHANDLE)
    return FALSE;
  else
  {
    finddata = (struct winfinddata *) hFindFile;

    while (readdir(finddata->fhandle, &dirent, 1) > 0)
    {
      syslog(LOG_DEBUG | LOG_AUX, "match next %s with %s\n", dirent.name, finddata->mask);
      if (like(dirent.name, finddata->mask))
      {
	strcpy(fn, finddata->dir);
	strcpy(fn + strlen(fn), "\\");
	strcpy(fn + strlen(fn), dirent.name);

        if (stat(fn, &statbuf) < 0) return FALSE;

        fill_find_data(lpFindFileData, dirent.name, &statbuf);
	return TRUE;
      }
    }

    return FALSE;
  }
}

BOOL WINAPI FlushFileBuffers
(
  HANDLE hFile
)
{
  TRACE("FlushFileBuffers");
  if (flush((handle_t) hFile) < 0) return FALSE;
  return TRUE;
}

DWORD WINAPI FormatMessageA
(
  DWORD dwFlags,
  LPCVOID lpSource,
  DWORD dwMessageId,
  DWORD dwLanguageId,
  LPTSTR lpBuffer,
  DWORD nSize,
  va_list *Arguments
)
{
  TRACE("FormatMessageA");
  // TODO: generate mote descriptive message
  sprintf(lpBuffer, "Error Message %p.\n", dwMessageId);
  return strlen(lpBuffer);
}

BOOL WINAPI FreeLibrary
(
  HMODULE hModule
)
{
  TRACE("FreeLibrary");
  if (unload((hmodule_t) hModule) < 0) return FALSE;
  return TRUE;
}

DWORD WINAPI GetCurrentDirectoryA
(
  DWORD nBufferLength,
  LPTSTR lpBuffer
)
{
  TRACE("GetCurrentDirectoryA");
  strcpy(lpBuffer, get_property(config, "win32", "curdir", "c:\\"));
  return strlen(lpBuffer);
}

HANDLE WINAPI GetCurrentProcess(VOID)
{
  TRACE("GetCurrentProcess");
  return (HANDLE) 1;
}

HANDLE WINAPI GetCurrentThread(VOID)
{
  TRACE("GetCurrentThread");
  return (HANDLE) self();
}

DWORD WINAPI GetCurrentThreadId(VOID)
{
  TRACE("GetCurrentThreadId");
  return gettid();
}

DWORD WINAPI GetEnvironmentVariableA
(
  LPCTSTR lpName,
  LPTSTR lpBuffer,
  DWORD nSize
)
{
  char *value;

  TRACE("GetEnvironmentVariableA");
  syslog(LOG_DEBUG, "GetEnvironmentVariable(%s)\n", lpName);
  
  value = get_property(config, "env", (char *) lpName, NULL);
  if (value)
  {
    strcpy(lpBuffer, value);
    return strlen(value);
  }
  else
  {
    memset(lpBuffer, 0, nSize);
    return 0;
  }
}

BOOL WINAPI GetExitCodeProcess
(
  HANDLE hProcess,
  LPDWORD lpExitCode
)
{
  TRACE("GetExitCodeProcess");
  panic("GetExitCodeProcess not implemented");
  return 0;
}

DWORD WINAPI GetFileAttributesA
(
  LPCTSTR lpFileName
)
{
  struct stat fs;

  TRACE("GetFileAttributesA");
  if (stat((char *) lpFileName, &fs) < 0) return -1;

  // TODO: set other attributes for files (hidden, system, readonly etc.)
  if (fs.mode & FS_DIRECTORY) 
    return 0x00000010;
  else
    return 0x00000080;
}

DWORD WINAPI GetLastError(VOID)
{
  TRACE("GetLastError");
  // TODO: implement better error reporting, for now just return E_FAIL
  return 0x80000008L;
}

DWORD WINAPI GetLogicalDrives(VOID)
{
  TRACE("GetLogicalDrives");
  panic("GetLogicalDrives not implemented");
  return 0;
}

DWORD WINAPI GetModuleFileNameA
(
  HMODULE hModule,
  LPTSTR lpFilename,
  DWORD nSize
)
{
  int rc;

  TRACE("GetModuleFileNameA");
  rc = getmodpath((hmodule_t) hModule, lpFilename, nSize);
  syslog(LOG_DEBUG, "Module filename for %p is %s\n", hModule, lpFilename);
  return rc;
}

BOOL WINAPI GetNumberOfConsoleInputEvents
(
  HANDLE hConsoleInput,
  LPDWORD lpcNumberOfEvents
)
{
  TRACE("GetNumberOfConsoleInputEvents");
  panic("GetNumberOfConsoleInputEvents not implemented");
  return FALSE;
}

FARPROC WINAPI GetProcAddress
(
  HMODULE hModule,
  LPCSTR lpProcName
)
{
  TRACE("GetProcAddress");
  return resolve((hmodule_t) hModule, (char *) lpProcName);
}

HANDLE WINAPI GetStdHandle
(
  DWORD nStdHandle
)
{
  TRACE("GetStdHandle");
  panic("GetStdHandle not implemented");
  return 0;
}

UINT WINAPI GetSystemDirectoryA
(
  LPTSTR lpBuffer,
  UINT uSize
)
{
  TRACE("GetSystemDirectoryA");
  strcpy(lpBuffer, get_property(config, "win32", "sysdir", "c:\\os"));
  return strlen(lpBuffer);
}

VOID WINAPI GetSystemInfo
(
  LPSYSTEM_INFO lpSystemInfo
)
{
  TRACE("GetSystemInfo");
  memset(lpSystemInfo, 0, sizeof(SYSTEM_INFO));
  lpSystemInfo->dwNumberOfProcessors = 1;
  lpSystemInfo->dwPageSize = PAGESIZE;
  lpSystemInfo->dwProcessorType = 586;
}

VOID WINAPI GetSystemTime
(
  LPSYSTEMTIME lpSystemTime
)
{
  struct timeval tv;
  struct tm tm;

  TRACE("GetSystemTime");
  gettimeofday(&tv);
  gmtime(&tv.tv_sec, &tm);

  lpSystemTime->wYear = tm.tm_year + 1900; 
  lpSystemTime->wMonth = tm.tm_mon + 1; 
  lpSystemTime->wDayOfWeek = tm.tm_wday;
  lpSystemTime->wDay = tm.tm_mday; 
  lpSystemTime->wHour = tm.tm_hour; 
  lpSystemTime->wMinute = tm.tm_min; 
  lpSystemTime->wSecond = tm.tm_sec; 
  lpSystemTime->wMilliseconds = tv.tv_usec / 1000;
}

VOID GetSystemTimeAsFileTime
(
  LPFILETIME lpSystemTimeAsFileTime
)
{
  struct timeval tv;

  TRACE("GetSystemTimeAsFileTime");
  gettimeofday(&tv);
  *(unsigned __int64 *) lpSystemTimeAsFileTime = ((__int64) (tv.tv_sec) * 1000000 + (__int64) (tv.tv_usec)) * MICROTIMESCALE + EPOC;
}

DWORD WINAPI GetTempPathA
(
  DWORD nBufferLength,
  LPTSTR lpBuffer
)
{
  TRACE("GetTempPathA");
  strcpy(lpBuffer, get_property(config, "win32", "tmpdir", "c:\\tmp"));
  return strlen(lpBuffer);
}

BOOL WINAPI GetThreadContext
(
  HANDLE hThread,
  LPCONTEXT lpContext
)
{
  struct context ctxt;

  TRACE("GetThreadContext");
  if (getcontext((handle_t) hThread, &ctxt) < 0) return FALSE;
  convert_to_win32_context(&ctxt, lpContext);
  return TRUE;
}

LCID WINAPI GetThreadLocale(void)
{
  TRACE("GetThreadLocale");
  return get_numeric_property(config, "win32", "locale", 0x0406);
}

int WINAPI GetThreadPriority
(
  HANDLE hThread
)
{
  int prio;

  TRACE("GetThreadPriority");

  prio = getprio((handle_t) hThread);
  if (prio < 0) return 0x7FFFFFFF;

  switch (prio)
  {
    case PRIORITY_IDLE: return THREAD_PRIORITY_IDLE;
    case PRIORITY_LOWEST: return THREAD_PRIORITY_LOWEST;
    case PRIORITY_BELOW_NORMAL: return THREAD_PRIORITY_BELOW_NORMAL;
    case PRIORITY_NORMAL: return THREAD_PRIORITY_NORMAL;
    case PRIORITY_ABOVE_NORMAL: return THREAD_PRIORITY_ABOVE_NORMAL;
    case PRIORITY_HIGHEST: return THREAD_PRIORITY_HIGHEST;
    case PRIORITY_TIME_CRITICAL: return THREAD_PRIORITY_TIME_CRITICAL;
  }

  return 0x7FFFFFFF;
}

BOOL WINAPI GetThreadTimes
(
  HANDLE hThread,
  LPFILETIME lpCreationTime,
  LPFILETIME lpExitTime,
  LPFILETIME lpKernelTime,
  LPFILETIME lpUserTime
)
{
  TRACE("GetThreadTimes");
  panic("GetThreadTimes not implemented");
  return FALSE;
}

DWORD WINAPI GetTimeZoneInformation
(
  LPTIME_ZONE_INFORMATION lpTimeZoneInformation
)
{
  TRACE("GetTimeZoneInformation");
  // TODO: return real time zone information (used by win32\native\java\util\TimeZone_md.c)
  return -1;
}

BOOL WINAPI GetVersionExA
(
  LPOSVERSIONINFO lpVersionInfo
)
{
  TRACE("GetVersionExA");
  lpVersionInfo->dwMajorVersion = 5;
  lpVersionInfo->dwMinorVersion = 0;
  lpVersionInfo->dwPlatformId = 2; // VER_PLATFORM_WIN32_NT
  strcpy(lpVersionInfo->szCSDVersion, "sanos");

  return TRUE;
}

UINT WINAPI GetWindowsDirectoryA
(
  LPTSTR lpBuffer,
  UINT uSize
)
{
  TRACE("GetWindowsDirectoryA");
  strcpy(lpBuffer, get_property(config, "win32", "windir", "c:\\os"));
  return strlen(lpBuffer);
}

VOID WINAPI InitializeCriticalSection
(
  LPCRITICAL_SECTION lpCriticalSection
)
{
  TRACE("InitializeCriticalSection");
  mkcs((struct critsect *) lpCriticalSection);
}

__declspec(naked) LONG WINAPI InterlockedDecrement
(
  LPLONG volatile lpAddend
)
{
  __asm
  {
    mov   ecx,dword ptr [esp+4]
    mov   eax,0FFFFFFFFh
    nop
    xadd  dword ptr [ecx],eax
    dec   eax
    ret   4
  }
}

__declspec(naked) LONG WINAPI InterlockedIncrement
(
  LPLONG volatile lpAddend
)
{
  __asm
  {
    mov   ecx,dword ptr [esp+4]
    mov   eax,1
    nop
    xadd  dword ptr [ecx],eax
    inc   eax
    ret   4
  }
}

BOOL WINAPI IsDBCSLeadByte
(
  BYTE TestChar
)
{
  TRACEX("IsDBCSLeadByte");
  // TODO: test is lead byte
  //syslog(LOG_DEBUG, "IsDBCSLeadByte called");
  return FALSE;
}

VOID WINAPI LeaveCriticalSection
(
  LPCRITICAL_SECTION lpCriticalSection
)
{
  TRACEX("LeaveCriticalSection");
  leave(lpCriticalSection);
}

HMODULE WINAPI LoadLibraryA
(
  LPCTSTR lpFileName
)
{
  TRACE("LoadLibraryA");
  return (HMODULE) load((char *) lpFileName);
}

BOOL WINAPI PeekConsoleInputA
(
  HANDLE hConsoleInput,
  PINPUT_RECORD lpBuffer,
  DWORD nLength,
  LPDWORD lpNumberOfEventsRead
)
{
  TRACE("PeekConsoleInputA");
  panic("PeekConsoleInputA not implemented");
  return FALSE;
}

BOOL WINAPI QueryPerformanceCounter
(
  LARGE_INTEGER *lpPerformanceCount
)
{
  TRACE("QueryPerformanceCounter");
  lpPerformanceCount->HighPart = 0;
  lpPerformanceCount->LowPart = clock();

  return TRUE;
}

BOOL WINAPI QueryPerformanceFrequency
(
  LARGE_INTEGER *lpFrequency
)
{
  TRACE("QueryPerformanceFrequency");
  // Tick count is in milliseconds
  lpFrequency->HighPart = 0;
  lpFrequency->LowPart = 1000;

  return TRUE;
}

BOOL WINAPI PeekNamedPipe
(
  HANDLE hNamedPipe,
  LPVOID lpBuffer,
  DWORD nBufferSize,
  LPDWORD lpBytesRead,
  LPDWORD lpTotalBytesAvail,
  LPDWORD lpBytesLeftThisMessage
)
{
  TRACE("PeekNamedPipe");
  panic("PeekNamedPipe not implemented");
  return FALSE;
}

BOOL WINAPI ReleaseSemaphore
(
  HANDLE hSemaphore,
  LONG lReleaseCount,
  LPLONG lpPreviousCount
)
{
  LONG dummy;

  TRACE("ReleaseSemaphore");
  if (!lpPreviousCount) lpPreviousCount = &dummy;
  *lpPreviousCount = semrel((handle_t) hSemaphore, lReleaseCount);
  return TRUE;
}

BOOL WINAPI RemoveDirectoryA
(
  LPCTSTR lpPathName
)
{
  TRACE("RemoveDirectoryA");
  if (rmdir(lpPathName) < 0) return FALSE;
  return TRUE;
}

BOOL WINAPI ResetEvent
(
  HANDLE hEvent
)
{
  TRACE("ResetEvent");
  ereset((handle_t) hEvent);
  return TRUE;
}

DWORD WINAPI ResumeThread
(
  HANDLE hThread
)
{
  TRACE("ResumeThread");
  return resume((handle_t) hThread);
}

BOOL WINAPI SetConsoleCtrlHandler
(
  PHANDLER_ROUTINE HandlerRoutine,
  BOOL Add
)
{
  TRACE("SetConsoleCtrlHandler");
  syslog(LOG_DEBUG, "warning: SetConsoleCtrlHandler not implemented, ignored\n");
  return TRUE;
}

BOOL WINAPI SetEndOfFile
(
  HANDLE hFile
)
{
  TRACE("SetEndOfFile");
  if (chsize((handle_t) hFile, tell((handle_t) hFile)) < 0) return FALSE;
  return TRUE;
}

BOOL WINAPI SetEvent
(
  HANDLE hEvent
)
{
  TRACE("SetEvent");
  eset((handle_t) hEvent);
  return TRUE;
}

BOOL WINAPI SetFileAttributesA
(
  LPCTSTR lpFileName,
  DWORD dwFileAttributes
)
{
  TRACE("SetFileAttributesA");
  panic("SetFileAttributesA not implemented");
  return FALSE;
}

DWORD WINAPI SetFilePointer
(
  HANDLE hFile,
  LONG lDistanceToMove,
  PLONG lpDistanceToMoveHigh,
  DWORD dwMoveMethod
)
{
  TRACE("SetFilePointer");
  panic("SetFilePointer not implemented");
  return 0;
}

BOOL WINAPI SetFileTime
(
  HANDLE hFile,
  CONST FILETIME *lpCreationTime,
  CONST FILETIME *lpLastAccessTime,
  CONST FILETIME *lpLastWriteTime
)
{
  TRACE("SetFileTime");
  panic("SetFileTime not implemented");
  return FALSE;
}

BOOL WINAPI SetHandleInformation
(
  HANDLE hObject,
  DWORD dwMask,
  DWORD dwFlags
)
{
  TRACE("SetHandleInformation");
  // Ignored, only used for setting handle inheritance
  syslog(LOG_DEBUG, "warning: SetHandleInformation ignored\n");
  return TRUE;
}

BOOL WINAPI SetThreadContext
(
  HANDLE hThread,
  CONST CONTEXT *lpContext
)
{
  struct context ctxt;

  TRACE("SetThreadContext");
  convert_from_win32_context(&ctxt, (CONTEXT *) lpContext);
  if (setcontext((handle_t) hThread, &ctxt) < 0) return FALSE;
  return TRUE;
}

BOOL WINAPI SetThreadPriority
(
  HANDLE hThread,
  int nPriority
)
{
  int prio;

  TRACE("SetThreadPriority");

  switch (nPriority)
  {
    case THREAD_PRIORITY_IDLE:
      prio = PRIORITY_IDLE;
      break;

    case THREAD_PRIORITY_LOWEST:
      prio = PRIORITY_LOWEST;
      break;

    case THREAD_PRIORITY_BELOW_NORMAL:
      prio = PRIORITY_BELOW_NORMAL;
      break;

    case THREAD_PRIORITY_NORMAL:
      prio = PRIORITY_NORMAL;
      break;

    case THREAD_PRIORITY_ABOVE_NORMAL:
      prio = PRIORITY_ABOVE_NORMAL;
      break;

    case THREAD_PRIORITY_HIGHEST:
      prio = PRIORITY_HIGHEST;
      break;

    case THREAD_PRIORITY_TIME_CRITICAL:
      prio = PRIORITY_TIME_CRITICAL;
      break;

    default:
      return FALSE;
  }

  if (setprio((handle_t) hThread, prio) < 0) return FALSE;
  return TRUE;
}

LPTOP_LEVEL_EXCEPTION_FILTER WINAPI SetUnhandledExceptionFilter
(
  LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter 
)
{
  TRACE("SetUnhandledExceptionFilter");
  syslog(LOG_DEBUG, "warning: SetUnhandledExceptionFilter not implemented, ignored\n");
  return NULL;
}

VOID WINAPI Sleep
(
  DWORD dwMilliseconds
)
{
  TRACE("Sleep");
  sleep(dwMilliseconds);
}

DWORD WINAPI SuspendThread
(
  HANDLE hThread
)
{
  TRACE("SuspendThread");
  return suspend((handle_t) hThread);
}

BOOL WINAPI SystemTimeToFileTime
(
  CONST SYSTEMTIME *lpSystemTime,
  LPFILETIME lpFileTime
)
{
  struct tm tm;
  time_t time;

  TRACE("SystemTimeToFileTime");
  memset(&tm, 0, sizeof(tm));
  tm.tm_yday = lpSystemTime->wYear - 1900;
  tm.tm_mon = lpSystemTime->wMonth - 1;
  tm.tm_mday = lpSystemTime->wDay;
  tm.tm_hour = lpSystemTime->wHour;
  tm.tm_min = lpSystemTime->wMinute;
  tm.tm_sec = lpSystemTime->wSecond;

  time = mktime(&tm);
    
  *(unsigned __int64 *) lpFileTime = ((__int64) time * 1000000 + (__int64) (lpSystemTime->wMilliseconds) * 1000) * MICROTIMESCALE + EPOC;
  return TRUE;
}

BOOL WINAPI TerminateProcess
(
  HANDLE hProcess,
  UINT uExitCode
)
{
  TRACE("TerminateProcess");
  panic("TerminateProcess not implemented");
  return FALSE;
}

DWORD WINAPI TlsAlloc(VOID)
{
  TRACE("TlsAlloc");
  return tlsalloc();
}

LPVOID WINAPI TlsGetValue
(
  DWORD dwTlsIndex
)
{
  TRACEX("TlsGetValue");
  return tlsget(dwTlsIndex);
}

BOOL WINAPI TlsSetValue
(
  DWORD dwTlsIndex,
  LPVOID lpTlsValue
)
{
  TRACEX("TlsSetValue");
  tlsset(dwTlsIndex, lpTlsValue);
  return TRUE;
}

LPVOID WINAPI VirtualAlloc
(
  LPVOID lpAddress,
  SIZE_T dwSize,
  DWORD flAllocationType,
  DWORD flProtect
)
{
  TRACE("VirtualAlloc");
  syslog(LOG_DEBUG, "VirtualAlloc %d bytes (%p,%p)\n", dwSize, flAllocationType, flProtect);
  return mmap(lpAddress, dwSize, flAllocationType, flProtect);
}

BOOL WINAPI VirtualFree
(
  LPVOID lpAddress,
  SIZE_T dwSize,
  DWORD dwFreeType
)
{
  TRACE("VirtualFree");
  syslog(LOG_DEBUG, "VirtualFree %d bytes (%p)\n", dwSize, dwFreeType);
  munmap(lpAddress, dwSize, dwFreeType);
  return TRUE;
}

DWORD WINAPI VirtualQuery
(
  LPCVOID lpAddress,
  PMEMORY_BASIC_INFORMATION lpBuffer,
  SIZE_T dwLength
)
{
  TRACE("VirtualQuery");
  // Only used in win32\hpi\src\threads_md.c to check for stack guard pages
  lpBuffer->BaseAddress = (void *) ((unsigned long) lpAddress & ~(PAGESIZE - 1));
  lpBuffer->Protect = PAGE_READWRITE;

  return dwLength;
}

DWORD WINAPI WaitForMultipleObjects
(
  DWORD nCount,
  CONST HANDLE *lpHandles,
  BOOL bWaitAll,
  DWORD dwMilliseconds
)
{
  TRACE("WaitForMultipleObjects");
  panic("WaitForMultipleObjects not implemented");
  return 0;
  //return wait_for_objects((handle_t *) lpHandles, nCount, bWaitAll, dwMilliseconds);
}

DWORD WINAPI WaitForSingleObject
(
  HANDLE hHandle,
  DWORD dwMilliseconds
)
{
  TRACE("WaitForSingleObject");
  return wait((handle_t) hHandle, dwMilliseconds);
}

int WINAPI WideCharToMultiByte
(
  UINT CodePage,
  DWORD dwFlags,
  LPCWSTR lpWideCharStr,
  int cchWideChar,
  LPSTR lpMultiByteStr,
  int cbMultiByte,
  LPCSTR lpDefaultChar,
  LPBOOL lpUsedDefaultChar
)
{
  TRACE("WideCharToMultiByte");
  panic("WideCharToMultiByte not implemented");
  return 0;
}

int __stdcall DllMain(handle_t hmod, int reason, void *reserved)
{
  return TRUE;
}
