#ifndef WIN32_H
#define WIN32_H

#define WINAPI __declspec(dllexport) __stdcall

#define CONST const

#define MAX_PATH          260
#define INVALID_HANDLE_VALUE ((HANDLE) -1)

typedef void *HANDLE;
typedef HANDLE *PHANDLE;
typedef HANDLE *LPHANDLE;

typedef void VOID;
typedef void *PVOID;
typedef const void *LPCVOID;

typedef char TCHAR;
typedef char CHAR;
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;
typedef unsigned int UINT;
typedef long LONG;
typedef unsigned __int64 LONGLONG;

typedef int BOOL;
typedef unsigned int SIZE_T;

typedef char *LPSTR;
typedef char *LPTSTR;
typedef const char *LPCTSTR;
typedef const char *LPCSTR;

typedef const unsigned short *LPCWSTR;

typedef BYTE *LPBYTE;
typedef LONG *LPLONG;
typedef LONG *PLONG;
typedef DWORD *LPDWORD;
typedef BOOL *LPBOOL;
typedef void *LPVOID;
typedef handle_t HMODULE;
typedef void *FARPROC;
typedef int LCID;
typedef int HKEY;
typedef HKEY *PHKEY;
typedef int REGSAM;
typedef int HWND;
typedef int MMRESULT;

typedef void *LPCRITICAL_SECTION;
typedef void *LPSECURITY_ATTRIBUTES;
typedef void *LPSTARTUPINFO;
typedef void *LPPROCESS_INFORMATION;
typedef void *LPTIME_ZONE_INFORMATION;
typedef void *PINPUT_RECORD;
typedef void *PHANDLER_ROUTINE;
typedef void *LPTOP_LEVEL_EXCEPTION_FILTER;

typedef struct CONTEXT 
{ 
  DWORD dummy;
} CONTEXT, *PCONTEXT, *LPCONTEXT; 

typedef union _LARGE_INTEGER { 
  struct 
  {
    DWORD LowPart; 
    LONG  HighPart; 
  };
  LONGLONG QuadPart;
} LARGE_INTEGER, *PLARGE_INTEGER; 

typedef struct _FILETIME 
{ 
  DWORD dwLowDateTime;
  DWORD dwHighDateTime;
} FILETIME, *PFILETIME, *LPFILETIME; 

typedef struct _SYSTEMTIME 
{ 
    WORD wYear; 
    WORD wMonth; 
    WORD wDayOfWeek; 
    WORD wDay; 
    WORD wHour; 
    WORD wMinute; 
    WORD wSecond; 
    WORD wMilliseconds; 
} SYSTEMTIME, *PSYSTEMTIME, *LPSYSTEMTIME;

typedef struct _SYSTEM_INFO 
{ 
  union 
  { 
    DWORD  dwOemId; 
    struct 
    { 
      WORD wProcessorArchitecture; 
      WORD wReserved; 
    }; 
  }; 
  DWORD  dwPageSize; 
  LPVOID lpMinimumApplicationAddress; 
  LPVOID lpMaximumApplicationAddress; 
  DWORD *dwActiveProcessorMask; 
  DWORD dwNumberOfProcessors; 
  DWORD dwProcessorType; 
  DWORD dwAllocationGranularity; 
  WORD wProcessorLevel; 
  WORD wProcessorRevision; 
} SYSTEM_INFO, *LPSYSTEM_INFO; 

typedef struct _OSVERSIONINFO
{ 
  DWORD dwOSVersionInfoSize; 
  DWORD dwMajorVersion; 
  DWORD dwMinorVersion; 
  DWORD dwBuildNumber; 
  DWORD dwPlatformId; 
  TCHAR szCSDVersion[128];
} OSVERSIONINFO, *LPOSVERSIONINFO; 

typedef struct _MEMORY_BASIC_INFORMATION 
{
  PVOID BaseAddress; 
  PVOID AllocationBase; 
  DWORD AllocationProtect; 
  SIZE_T RegionSize; 
  DWORD State; 
  DWORD Protect; 
  DWORD Type; 
} MEMORY_BASIC_INFORMATION, *PMEMORY_BASIC_INFORMATION; 

#define FILE_ATTRIBUTE_DIRECTORY            0x00000010
#define FILE_ATTRIBUTE_NORMAL               0x00000080

typedef struct WIN32_FIND_DATA 
{
  DWORD dwFileAttributes;
  FILETIME ftCreationTime;
  FILETIME ftLastAccessTime;
  FILETIME ftLastWriteTime;
  DWORD nFileSizeHigh;
  DWORD nFileSizeLow;
  DWORD dwReserved0;
  DWORD dwReserved1;
  CHAR cFileName[MAX_PATH];
  CHAR cAlternateFileName[14];
} WIN32_FIND_DATA, *PWIN32_FIND_DATA, *LPWIN32_FIND_DATA;

typedef char *va_list;

#endif
