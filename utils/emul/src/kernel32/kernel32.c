#include <os.h>
#include <win32.h>

//#define WINDIR "C:\\WINDOWS"
//#define SYSDIR "C:\\WINDOWS\\SYSTEM32"
//#define TMPDIR "C:\\TEMP"
//#define CURDIR "C:\\"

#define WINDIR "\\os"
#define SYSDIR "\\os"
#define TMPDIR "\\tmp"
#define CURDIR "\\"

#define NOFINDHANDLE ((HANDLE) (-2))

#define LOCALE 0x0406

#if 0

#define DAY_SEC           (24L * 60L * 60L)    // secs in a day
#define YEAR_SEC          (365L * DAY_SEC)     // secs in a year
#define FOUR_YEAR_SEC     (1461L * DAY_SEC)    // secs in a 4 year interval
#define DEC_SEC           315532800L           // secs in 1970-1979
#define BASE_YEAR         70L                  // 1970 is the base year
#define BASE_DOW          4                    // 01-01-70 was a Thursday
#define LEAP_YEAR_ADJUST  17L                  // Leap years 1900 - 1970
#define MAX_YEAR          138L                 // 2038 is the max year

int lpdays[] = {-1, 30, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365};
int days[] = {-1, 30, 58, 89, 119, 150, 180, 211, 242, 272, 303, 333, 364};

#endif

#define MILLITIMESCALE  10000                  // 1 ms resolution
#define MILLIEPOC       11644473600000         // 00:00:00 GMT on January 1, 1970 (in millisecs)

#define SECTIMESCALE    10000000               // 1 sec resolution
#define SECEPOC         11644473600            // 00:00:00 GMT on January 1, 1970 (in secs) 

struct winfinddata
{
  handle_t fhandle;
  char dir[MAXPATH];
  char *mask;
};

__declspec(naked) void _allmul()
{
  __asm
  {

    mov     eax,[esp+8]     ; HIWORD(A)
    mov     ecx,[esp+16]    ; HIWORD(B)
    or      ecx,eax         ; test for both hiwords zero.
    mov     ecx,[esp+12]    ; LOWORD(B)
    jnz     short hard      ; both are zero, just mult ALO and BLO

    mov     eax,[esp+4]     ; LOWORD(A)
    mul     ecx

    ret     16              ; callee restores the stack

  hard:
    push    ebx

    mul     ecx             ; eax has AHI, ecx has BLO, so AHI * BLO
    mov     ebx,eax         ; save result

    mov     eax,[esp+8]     ; LOWORD(A2)
    mul     dword ptr [esp+20] ; ALO * BHI
    add     ebx,eax         ; ebx = ((ALO * BHI) + (AHI * BLO))

    mov     eax,[esp+8]     ; ecx = BLO
    mul     ecx             ; so edx:eax = ALO*BLO
    add     edx,ebx         ; now edx has all the LO*HI stuff

    pop     ebx

    ret     16              ; callee restores the stack
  }
}

BOOL WINAPI CloseHandle
(
  HANDLE hObject)
{
  if (close_handle((handle_t) hObject) < 0) return FALSE;
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
  if (lpName != NULL) panic("named event not supported");
  return (HANDLE) create_event(bManualReset, bInitialState);
}

HANDLE WINAPI CreateFileA
(
  LPCTSTR lpFileName, 
  DWORD dwDesiredAccess, 
  DWORD dwShareMode, 
  LPSECURITY_ATTRIBUTES lpSecurityAttributes, 
  DWORD dwCreationDisposition, 
  DWORD dwFlagsAndAttributes, 
  HANDLE hTemplateFile)
{
  panic("CreateFileA not implemented");
  return 0;
}

BOOL WINAPI CreatePipe
(
  PHANDLE hReadPipe, 
  PHANDLE hWritePipe, 
  LPSECURITY_ATTRIBUTES lpPipeAttributes, 
  DWORD nSize)
{
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
  if (lpName != NULL) panic("named semaphores not supported");
  syslog(LOG_DEBUG, "create sem with (%d,%d)\n", lInitialCount, lMaximumCount);
  return (HANDLE) create_sem(lInitialCount);
}

VOID WINAPI DebugBreak(VOID)
{
  panic("DebugBreak not implemented");
}

BOOL WINAPI DeleteFileA
(
  LPCTSTR lpFileName
)
{
  panic("DeleteFileA not implemented");
  return FALSE;
}

BOOL WINAPI DisableThreadLibraryCalls
(
  HMODULE hModule
)
{
  // panic("DisableThreadLibraryCalls not implemented");
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
  *lpTargetHandle = (HANDLE) dup_handle((handle_t) hSourceHandle);
  return TRUE;
}

VOID WINAPI EnterCriticalSection
(
  LPCRITICAL_SECTION lpCriticalSection
)
{
  enter_critsect(lpCriticalSection);
}

static void fill_find_data(LPWIN32_FIND_DATA fd, char *filename, struct stat *statbuf)
{
  memset(fd, 0, sizeof(WIN32_FIND_DATA));

  strcpy(fd->cFileName, filename);

  if (statbuf->mode & FILE_STAT_DIRECTORY) 
    fd->dwFileAttributes |= FILE_ATTRIBUTE_DIRECTORY;
  else
    fd->dwFileAttributes |= FILE_ATTRIBUTE_NORMAL;

  fd->nFileSizeLow = statbuf->quad.size_low;
  fd->nFileSizeHigh = statbuf->quad.size_high;

  *(__int64 *) &(fd->ftCreationTime) = ((__int64) statbuf->ctime + SECEPOC) * SECTIMESCALE;
  *(__int64 *) &(fd->ftLastAccessTime) = ((__int64) statbuf->mtime + SECEPOC) * SECTIMESCALE;
  *(__int64 *) &(fd->ftLastWriteTime) = ((__int64) statbuf->mtime + SECEPOC) * SECTIMESCALE;
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

  if (hFindFile == NOFINDHANDLE)
    return TRUE;
  else if (hFindFile == INVALID_HANDLE_VALUE)
    return FALSE;
  else
  {
    finddata = (struct winfinddata *) hFindFile;

    close_handle(finddata->fhandle);
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
    if (finddata->fhandle == NOHANDLE)
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

        if (get_file_stat(fn, &statbuf) < 0) 
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
    if (get_file_stat((char *) lpFileName, &statbuf) < 0) return INVALID_HANDLE_VALUE;

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

        if (get_file_stat(fn, &statbuf) < 0) return FALSE;

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
  panic("FlushFileBuffers not implemented");
  return FALSE;
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
  // TODO: generate mote descriptive message
  format(lpBuffer, "Error Message %p.\n", dwMessageId);
  return strlen(lpBuffer);
}

BOOL WINAPI FreeLibrary
(
  HMODULE hModule
)
{
  panic("FreeLibrary not implemented");
  return FALSE;
}

DWORD WINAPI GetCurrentDirectoryA
(
  DWORD nBufferLength,
  LPTSTR lpBuffer
)
{
  strcpy(lpBuffer, CURDIR);
  return strlen(CURDIR);
}

HANDLE WINAPI GetCurrentProcess(VOID)
{
  //panic("GetCurrentProcess not implemented");
  return 0;
}

HANDLE WINAPI GetCurrentThread(VOID)
{
  return (HANDLE) get_current_thread();
}

DWORD WINAPI GetCurrentThreadId(VOID)
{
  return get_current_thread_id();
  //return (DWORD) get_current_thread();
}

DWORD WINAPI GetEnvironmentVariableA
(
  LPCTSTR lpName,
  LPTSTR lpBuffer,
  DWORD nSize
)
{
  syslog(LOG_DEBUG, "GetEnvironmentVariable(%s)\n", lpName);
  memset(lpBuffer, 0, nSize);
  return 0;
}

BOOL WINAPI GetExitCodeProcess
(
  HANDLE hProcess,
  LPDWORD lpExitCode
)
{
  panic("GetExitCodeProcess not implemented");
  return 0;
}

DWORD WINAPI GetFileAttributesA
(
  LPCTSTR lpFileName
)
{
  struct stat fs;

  if (get_file_stat((char *) lpFileName, &fs) == -1) return -1;

  // TODO: set other attributes for files (hidden, system, readonly etc.)
  if (fs.mode & FILE_STAT_DIRECTORY) 
    return 0x00000010;
  else
    return 0x00000080;
}

DWORD WINAPI GetLastError(VOID)
{
  // TODO: implement better error reporting, for now just return E_FAIL
  return 0x80000008L;
}

DWORD WINAPI GetLogicalDrives(VOID)
{
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
  return get_module_filename((hmodule_t) hModule, lpFilename, nSize);
}

BOOL WINAPI GetNumberOfConsoleInputEvents
(
  HANDLE hConsoleInput,
  LPDWORD lpcNumberOfEvents
)
{
  panic("GetNumberOfConsoleInputEvents not implemented");
  return FALSE;
}

FARPROC WINAPI GetProcAddress
(
  HMODULE hModule,
  LPCSTR lpProcName
)
{
  return get_proc_address((hmodule_t) hModule, (char *) lpProcName);
}

HANDLE WINAPI GetStdHandle
(
  DWORD nStdHandle
)
{
  panic("GetStdHandle not implemented");
  return 0;
}

UINT WINAPI GetSystemDirectoryA
(
  LPTSTR lpBuffer,
  UINT uSize
)
{
  //panic("GetSystemDirectoryA not implemented");
  strcpy(lpBuffer, SYSDIR);
  return strlen(SYSDIR);
}

VOID WINAPI GetSystemInfo
(
  LPSYSTEM_INFO lpSystemInfo
)
{
  memset(lpSystemInfo, 0, sizeof(SYSTEM_INFO));
  lpSystemInfo->dwNumberOfProcessors = 1;
  lpSystemInfo->dwPageSize = PAGESIZE;
  lpSystemInfo->dwProcessorType = 586;;
}

VOID WINAPI GetSystemTime
(
  LPSYSTEMTIME lpSystemTime
)
{
  get_system_time(lpSystemTime);
}

VOID GetSystemTimeAsFileTime
(
  LPFILETIME lpSystemTimeAsFileTime
)
{
  *(unsigned __int64 *) lpSystemTimeAsFileTime = (get_time() + MILLIEPOC) * MILLITIMESCALE;
}

DWORD WINAPI GetTempPathA
(
  DWORD nBufferLength,
  LPTSTR lpBuffer
)
{
  strcpy(lpBuffer, TMPDIR);
  return strlen(TMPDIR);
}

BOOL WINAPI GetThreadContext
(
  HANDLE hThread,
  LPCONTEXT lpContext
)
{
  if (get_thread_context((handle_t) hThread, lpContext) < 0) return FALSE;
  return TRUE;
}

LCID WINAPI GetThreadLocale(void)
{
  return LOCALE;
}

int WINAPI GetThreadPriority
(
  HANDLE hThread
)
{
  return get_thread_priority((handle_t) hThread);
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
  panic("GetThreadTimes not implemented");
  return FALSE;
}

DWORD WINAPI GetTimeZoneInformation
(
  LPTIME_ZONE_INFORMATION lpTimeZoneInformation
)
{
  // TODO: return real time zone information (used by win32\native\java\util\TimeZone_md.c)
  return -1;
}

BOOL WINAPI GetVersionExA
(
  LPOSVERSIONINFO lpVersionInfo
)
{
  lpVersionInfo->dwMajorVersion = 5;
  lpVersionInfo->dwMinorVersion = 0;
  lpVersionInfo->dwPlatformId = 2; // VER_PLATFORM_WIN32_NT
  strcpy(lpVersionInfo->szCSDVersion, "Emulation");

  return TRUE;
}

UINT WINAPI GetWindowsDirectoryA
(
  LPTSTR lpBuffer,
  UINT uSize
)
{
  //panic("GetWindowsDirectoryA not implemented");
  strcpy(lpBuffer, WINDIR);
  return strlen(WINDIR);
}

VOID WINAPI InitializeCriticalSection
(
  LPCRITICAL_SECTION lpCriticalSection
)
{
  init_critsect(lpCriticalSection);
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
  // TODO: test is lead byte
  //syslog(LOG_DEBUG, "IsDBCSLeadByte called");
  return FALSE;
}

VOID WINAPI LeaveCriticalSection
(
  LPCRITICAL_SECTION lpCriticalSection
)
{
  leave_critsect(lpCriticalSection);
}

HMODULE WINAPI LoadLibraryA
(
  LPCTSTR lpFileName
)
{
  return (HMODULE) load_module((char *) lpFileName);
}

BOOL WINAPI PeekConsoleInputA
(
  HANDLE hConsoleInput,
  PINPUT_RECORD lpBuffer,
  DWORD nLength,
  LPDWORD lpNumberOfEventsRead
)
{
  panic("PeekConsoleInputA not implemented");
  return FALSE;
}

BOOL WINAPI QueryPerformanceCounter
(
  LARGE_INTEGER *lpPerformanceCount
)
{
  lpPerformanceCount->HighPart = 0;
  lpPerformanceCount->LowPart = get_tick_count();

  return TRUE;
}

BOOL WINAPI QueryPerformanceFrequency
(
  LARGE_INTEGER *lpFrequency
)
{
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
  if (!lpPreviousCount) lpPreviousCount = &dummy;
  *lpPreviousCount = release_sem((handle_t) hSemaphore, lReleaseCount);
  return TRUE;
}

BOOL WINAPI RemoveDirectoryA
(
  LPCTSTR lpPathName
)
{
  panic("RemoveDirectoryA not implemented");
  return FALSE;
}

BOOL WINAPI ResetEvent
(
  HANDLE hEvent
)
{
  reset_event((handle_t) hEvent);
  return TRUE;
}

DWORD WINAPI ResumeThread
(
  HANDLE hThread
)
{
  return resume_thread((handle_t) hThread);
}

BOOL WINAPI SetConsoleCtrlHandler
(
  PHANDLER_ROUTINE HandlerRoutine,
  BOOL Add
)
{
  syslog(LOG_DEBUG, "warning: SetConsoleCtrlHandler not implemented, ignored\n");
  return TRUE;
}

BOOL WINAPI SetEndOfFile
(
  HANDLE hFile
)
{
  panic("SetEndOfFile not implemented");
  return FALSE;
}

BOOL WINAPI SetEvent
(
  HANDLE hEvent
)
{
  set_event((handle_t) hEvent);
  return TRUE;
}

BOOL WINAPI SetFileAttributesA
(
  LPCTSTR lpFileName,
  DWORD dwFileAttributes
)
{
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
  panic("SetThreadContext not implemented");
  return FALSE;
}

BOOL WINAPI SetThreadPriority
(
  HANDLE hThread,
  int nPriority
)
{
  if (set_thread_priority((handle_t) hThread, nPriority) < 0) return FALSE;
  return TRUE;
}

LPTOP_LEVEL_EXCEPTION_FILTER WINAPI SetUnhandledExceptionFilter
(
  LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter 
)
{
  syslog(LOG_DEBUG, "warning: SetUnhandledExceptionFilter not implemented, ignored\n");
  return NULL;
}

VOID WINAPI Sleep
(
  DWORD dwMilliseconds
)
{
  sleep(dwMilliseconds);
}

DWORD WINAPI SuspendThread
(
  HANDLE hThread
)
{
  return suspend_thread((handle_t) hThread);
}

BOOL WINAPI SystemTimeToFileTime
(
  CONST SYSTEMTIME *lpSystemTime,
  LPFILETIME lpFileTime
)
{
  systemtime_to_filetime((void *) lpSystemTime, (void *) lpFileTime);
  return TRUE;
}

BOOL WINAPI TerminateProcess
(
  HANDLE hProcess,
  UINT uExitCode
)
{
  panic("TerminateProcess not implemented");
  return FALSE;
}

DWORD WINAPI TlsAlloc(VOID)
{
  return alloc_tls();
}

LPVOID WINAPI TlsGetValue
(
  DWORD dwTlsIndex
)
{
  return get_tls(dwTlsIndex);
}

BOOL WINAPI TlsSetValue
(
  DWORD dwTlsIndex,
  LPVOID lpTlsValue
)
{
  set_tls(dwTlsIndex, lpTlsValue);
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
  return wait_for_objects((handle_t *) lpHandles, nCount, bWaitAll, dwMilliseconds);
}

DWORD WINAPI WaitForSingleObject
(
  HANDLE hHandle,
  DWORD dwMilliseconds
)
{
  return wait_for_object((handle_t) hHandle, dwMilliseconds);
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
  panic("WideCharToMultiByte not implemented");
  return 0;
}

int __stdcall DllMain(handle_t hmod, int reason, void *reserved)
{
  return TRUE;
}
