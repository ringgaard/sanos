//
// win32.h
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Win32 definitions
//

#ifndef WIN32_H
#define WIN32_H

#define WINAPI __declspec(dllexport) __stdcall

#define CONST const

#define MAX_PATH             260
#define INVALID_HANDLE_VALUE ((HANDLE) -1)

#define WAIT_TIMEOUT         0x00000102L
#define WAIT_FAILED          0xFFFFFFFFL

#define THREAD_PRIORITY_IDLE            -15
#define THREAD_PRIORITY_LOWEST          -2
#define THREAD_PRIORITY_BELOW_NORMAL    -1
#define THREAD_PRIORITY_NORMAL          0
#define THREAD_PRIORITY_ABOVE_NORMAL    1
#define THREAD_PRIORITY_HIGHEST         2
#define THREAD_PRIORITY_TIME_CRITICAL   15

#define FD_SETSIZE 64

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

#define CONTEXT_i386    0x00010000
#define CONTEXT_i486    0x00010000

#define CONTEXT_CONTROL         (CONTEXT_i386 | 0x00000001L)
#define CONTEXT_INTEGER         (CONTEXT_i386 | 0x00000002L)
#define CONTEXT_SEGMENTS        (CONTEXT_i386 | 0x00000004L)
#define CONTEXT_FLOATING_POINT  (CONTEXT_i386 | 0x00000008L)
#define CONTEXT_DEBUG_REGISTERS (CONTEXT_i386 | 0x00000010L)
#define CONTEXT_EXTENDED_REGISTERS  (CONTEXT_i386 | 0x00000020L)

#define CONTEXT_FULL (CONTEXT_CONTROL | CONTEXT_INTEGER | CONTEXT_SEGMENTS)

#define MAXIMUM_SUPPORTED_EXTENSION     512

#define SIZE_OF_80387_REGISTERS      80

typedef struct _FLOATING_SAVE_AREA 
{
  DWORD   ControlWord;
  DWORD   StatusWord;
  DWORD   TagWord;
  DWORD   ErrorOffset;
  DWORD   ErrorSelector;
  DWORD   DataOffset;
  DWORD   DataSelector;
  BYTE    RegisterArea[SIZE_OF_80387_REGISTERS];
  DWORD   Cr0NpxState;
} FLOATING_SAVE_AREA;

typedef FLOATING_SAVE_AREA *PFLOATING_SAVE_AREA;

typedef struct _CONTEXT 
{
  DWORD ContextFlags;

  DWORD   Dr0;
  DWORD   Dr1;
  DWORD   Dr2;
  DWORD   Dr3;
  DWORD   Dr6;
  DWORD   Dr7;

  FLOATING_SAVE_AREA FloatSave;

  DWORD   SegGs;
  DWORD   SegFs;
  DWORD   SegEs;
  DWORD   SegDs;

  DWORD   Edi;
  DWORD   Esi;
  DWORD   Ebx;
  DWORD   Edx;
  DWORD   Ecx;
  DWORD   Eax;

  DWORD   Ebp;
  DWORD   Eip;
  DWORD   SegCs;
  DWORD   EFlags;
  DWORD   Esp;
  DWORD   SegSs;

  BYTE    ExtendedRegisters[MAXIMUM_SUPPORTED_EXTENSION];
} CONTEXT;

typedef CONTEXT *PCONTEXT;
typedef CONTEXT *LPCONTEXT;

typedef union _LARGE_INTEGER 
{ 
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

typedef struct fd_set 
{
  unsigned int fd_count;
  handle_t fd_array[FD_SETSIZE];
} fd_set;

#endif
