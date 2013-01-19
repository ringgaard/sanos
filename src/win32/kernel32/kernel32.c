//
// kernel32.c
//
// Win32 KERNEL32 emulation
//
// Copyright (C) 2002 Michael Ringgaard. All rights reserved.
// Portions Copyright (C) 1999 Turchanov Sergey
// Portions Copyright (C) 1999 Alexandre Julliard
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

#include <os.h>
#include <sys/types.h>
#include <os/trap.h>
#include <string.h>
#include <time.h>
#include <inifile.h>
#include <win32.h>

int sprintf(char *buf, const char *fmt, ...);

#define MAX_VADS        256

#define EPOC            116444736000000000     // 00:00:00 GMT on January 1, 1970

#define MICROTIMESCALE  10                     // 1 us resolution
#define MILLITIMESCALE  10000                  // 1 ms resolution
#define SECTIMESCALE    10000000               // 1 sec resolution

#define NOFINDHANDLE ((HANDLE) (-2))
#define PROCESSHEAP  ((HANDLE) (-3))

struct winfinddata {
  handle_t fhandle;
  char dir[MAXPATH];
  char *mask;
};

struct vad {
  void *addr;
  unsigned long size;
};

struct vad vads[MAX_VADS];
PTOP_LEVEL_EXCEPTION_FILTER topexcptfilter = NULL;
void (*old_globalhandler)(struct siginfo *info);

int convert_filename_to_unicode(const char *src, wchar_t *dst, int maxlen) {
  wchar_t *end = dst + maxlen;
  while (*src) {
    if (dst == end) {
      errno = ENAMETOOLONG;
      return -1;
    }

    *dst++ = (unsigned char) *src++;
  }
  
  if (dst == end) {
    errno = ENAMETOOLONG;
    return -1;
  }
  *dst = 0;
  return 0;
}

int convert_filename_from_unicode(const wchar_t *src, char *dst, int maxlen) {
  char *end = dst + maxlen;
  while (*src) {
    if (dst == end) {
      errno = ENAMETOOLONG;
      return -1;
    }
    if (*src & 0xFF00) {
      errno = EINVAL;
      return -1;
    }
    *dst++ = (unsigned char) *src++;
  }
  
  if (dst == end) {
    errno = ENAMETOOLONG;
    return -1;
  }
  *dst = 0;
  return 0;
}

void convert_from_win32_context(struct context *ctxt, CONTEXT *ctx) {
  if (ctx->ContextFlags & CONTEXT_CONTROL) {
    ctxt->ess = ctx->SegSs;
    ctxt->esp = ctx->Esp;
    ctxt->ecs = ctx->SegCs;
    ctxt->eip = ctx->Eip;
    ctxt->eflags = ctx->EFlags;
    ctxt->ebp = ctx->Ebp;
  }

  if (ctx->ContextFlags & CONTEXT_INTEGER) {
    ctxt->eax = ctx->Eax;
    ctxt->ebx = ctx->Ebx;
    ctxt->ecx = ctx->Ecx;
    ctxt->edx = ctx->Edx;
    ctxt->esi = ctx->Esi;
    ctxt->edi = ctx->Edi;
  }

  if (ctx->ContextFlags & CONTEXT_SEGMENTS) {
    ctxt->ds = ctx->SegDs;
    ctxt->es = ctx->SegEs;
    // fs missing
    // gs missing
  }

  if (ctx->ContextFlags & CONTEXT_FLOATING_POINT) {
    // fpu state missing
  }

  if (ctx->ContextFlags & CONTEXT_DEBUG_REGISTERS) {
    // debug registers missing
  }
}

void convert_to_win32_context(struct context *ctxt, CONTEXT *ctx) {
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

static __inline EXCEPTION_FRAME *push_frame(EXCEPTION_FRAME *frame) {
  struct tib *tib = gettib();
  frame->prev = (EXCEPTION_FRAME *) tib->except;
  tib->except = (struct xcptrec *) frame;
  return frame->prev;
}

static __inline EXCEPTION_FRAME *pop_frame(EXCEPTION_FRAME *frame) {
  struct tib *tib = gettib();
  tib->except = (struct xcptrec *) frame->prev;
  return frame->prev;
}

//
// Handler for exceptions happening inside a handler
//

static EXCEPTION_DISPOSITION raise_handler(EXCEPTION_RECORD *rec, EXCEPTION_FRAME *frame, CONTEXT *ctxt, EXCEPTION_FRAME **dispatcher) {
  if (rec->ExceptionFlags & (EH_UNWINDING | EH_EXIT_UNWIND)) return ExceptionContinueSearch;

  // We shouldn't get here so we store faulty frame in dispatcher
  *dispatcher = ((NESTED_FRAME *) frame)->prev;
  return ExceptionNestedException;
}

//
// Handler for exceptions happening inside an unwind handler.
//

static EXCEPTION_DISPOSITION unwind_handler(EXCEPTION_RECORD *rec, EXCEPTION_FRAME *frame, CONTEXT *ctxt, EXCEPTION_FRAME **dispatcher) {
  if (!(rec->ExceptionFlags & (EH_UNWINDING | EH_EXIT_UNWIND))) return ExceptionContinueSearch;

  // We shouldn't get here so we store faulty frame in dispatcher
  *dispatcher = ((NESTED_FRAME *) frame)->prev;
  return ExceptionCollidedUnwind;
}

//
// Call an exception handler, setting up an exception frame to catch exceptions
// happening during the handler execution.
//

static EXCEPTION_DISPOSITION call_handler(
    EXCEPTION_RECORD *record, 
    EXCEPTION_FRAME *frame,
    CONTEXT *ctxt, 
    EXCEPTION_FRAME **dispatcher,
    PEXCEPTION_HANDLER handler, 
    PEXCEPTION_HANDLER nested_handler) {
  NESTED_FRAME newframe;
  EXCEPTION_DISPOSITION rc;

  newframe.frame.handler = nested_handler;
  newframe.prev = frame;
  push_frame(&newframe.frame);
  //syslog(LOG_DEBUG, "calling handler at %p code=%x flags=%x", handler, record->ExceptionCode, record->ExceptionFlags);
  rc = handler(record, frame, ctxt, dispatcher);
  //syslog(LOG_DEBUG, "handler returned %x", rc);
  pop_frame(&newframe.frame);

  return rc;
}

void raise_exception(EXCEPTION_RECORD *rec, CONTEXT *ctxt) {
  PEXCEPTION_FRAME frame, dispatch, nested_frame;
  EXCEPTION_RECORD newrec;
  EXCEPTION_DISPOSITION rc;
  struct tib *tib = gettib();

  //syslog(LOG_DEBUG, "raise_exception: code=%lx flags=%lx addr=%p", rec->ExceptionCode, rec->ExceptionFlags, rec->ExceptionAddress);

  frame = (EXCEPTION_FRAME *) tib->except;
  nested_frame = NULL;
  while (frame != (PEXCEPTION_FRAME) 0xFFFFFFFF) {
    //syslog(LOG_DEBUG, "frame: %p handler: %p", frame, frame->handler);
    
    // Check frame address
    if ((void *) frame < tib->stacklimit || (void *)(frame + 1) > tib->stacktop || (int) frame & 3) {
      rec->ExceptionFlags |= EH_STACK_INVALID;
      break;
    }

    // Call handler
    rc = call_handler(rec, frame, ctxt, &dispatch, frame->handler, raise_handler);
    if (frame == nested_frame) {
      // No longer nested
      nested_frame = NULL;
      rec->ExceptionFlags &= ~EH_NESTED_CALL;
    }

    switch (rc) {
      case ExceptionContinueExecution:
        if (!(rec->ExceptionFlags & EH_NONCONTINUABLE)) return;
        newrec.ExceptionCode = STATUS_NONCONTINUABLE_EXCEPTION;
        newrec.ExceptionFlags = EH_NONCONTINUABLE;
        newrec.ExceptionRecord  = rec;
        newrec.NumberParameters = 0;
        panic("kernel32: exception not continuable");
        //TODO: RtlRaiseException(&newrec);  // Never returns
        break;

      case ExceptionContinueSearch:
        break;

      case ExceptionNestedException:
        if (nested_frame < dispatch) nested_frame = dispatch;
        rec->ExceptionFlags |= EH_NESTED_CALL;
        break;

      default:
        newrec.ExceptionCode = STATUS_INVALID_DISPOSITION;
        newrec.ExceptionFlags = EH_NONCONTINUABLE;
        newrec.ExceptionRecord = rec;
        newrec.NumberParameters = 0;
        panic("kernel32: invalid disposition returned from exception handler");
        //TODO: RtlRaiseException(&newrec);  // Never returns
    }

    frame = frame->prev;
  }
  
  // TODO: perform default handling
  // DefaultHandling(rec, context);
}

void unwind(PEXCEPTION_FRAME endframe, LPVOID eip, PEXCEPTION_RECORD rec, DWORD retval, CONTEXT *ctxt) {
  EXCEPTION_RECORD record, newrec;
  PEXCEPTION_FRAME frame, dispatch;
  EXCEPTION_DISPOSITION rc;
  struct tib *tib = gettib();

  ctxt->Eax = retval;

  // Build an exception record, if we do not have one
  if (!rec) {
    record.ExceptionCode    = STATUS_UNWIND;
    record.ExceptionFlags   = 0;
    record.ExceptionRecord  = NULL;
    record.ExceptionAddress = (void *) ctxt->Eip;
    record.NumberParameters = 0;
    rec = &record;
  }

  rec->ExceptionFlags |= EH_UNWINDING | (endframe ? 0 : EH_EXIT_UNWIND);

  //syslog(LOG_DEBUG, "code=%lx flags=%lx", rec->ExceptionCode, rec->ExceptionFlags);

  // Get chain of exception frames
  frame = (EXCEPTION_FRAME *) tib->except;
  while (frame != (PEXCEPTION_FRAME) 0xffffffff && frame != endframe) {
    // Check frame address
    if (endframe && (frame > endframe)) {
      newrec.ExceptionCode = STATUS_INVALID_UNWIND_TARGET;
      newrec.ExceptionFlags = EH_NONCONTINUABLE;
      newrec.ExceptionRecord = rec;
      newrec.NumberParameters = 0;
      panic("kernel32: invalid unwind target");
      //TODO: RtlRaiseException(&newrec);  // Never returns
    }

    if ((void *) frame < tib->stacklimit || (void *)(frame + 1) > tib->stacktop || (int) frame & 3) {
      newrec.ExceptionCode = STATUS_BAD_STACK;
      newrec.ExceptionFlags = EH_NONCONTINUABLE;
      newrec.ExceptionRecord = rec;
      newrec.NumberParameters = 0;
      panic("kernel32: bad stack in unwind");
      //TODO: RtlRaiseException(&newrec);  // Never returns
    }

    // Call handler
    rc = call_handler(rec, frame, ctxt, &dispatch, frame->handler, unwind_handler);

    switch (rc) {
      case ExceptionContinueSearch:
        break;

      case ExceptionCollidedUnwind:
        frame = dispatch;
        break;

      default:
        newrec.ExceptionCode = STATUS_INVALID_DISPOSITION;
        newrec.ExceptionFlags = EH_NONCONTINUABLE;
        newrec.ExceptionRecord = rec;
        newrec.NumberParameters = 0;
        panic("kernel32: invalid disposition returned from exception handler in unwind");
        //TODO: RtlRaiseException(&newrec);  // Never returns
    }

    frame = pop_frame(frame);
  }
}

void win32_globalhandler(struct siginfo *info) {
  CONTEXT ctxt;
  EXCEPTION_RECORD rec;
  EXCEPTION_POINTERS ep;

  //syslog(LOG_DEBUG, "kernel32: caught signal %d", signum);

  ep.ContextRecord = &ctxt;
  ep.ExceptionRecord = &rec;

  memset(&rec, 0, sizeof(EXCEPTION_RECORD));
  rec.ExceptionAddress = (void *) info->si_ctxt->eip;

  convert_to_win32_context(info->si_ctxt, &ctxt);
  
  switch (info->si_ctxt->traptype) {
    case INTR_DIV:
      rec.ExceptionCode = EXCEPTION_INT_DIVIDE_BY_ZERO;
      break;

    case INTR_OVFL:
      rec.ExceptionCode = EXCEPTION_INT_OVERFLOW;
      break;

    case INTR_BOUND:
      rec.ExceptionCode = EXCEPTION_ARRAY_BOUNDS_EXCEEDED;
      break;

    case INTR_INSTR:
      rec.ExceptionCode = EXCEPTION_ILLEGAL_INSTRUCTION;
      break;

    case INTR_STACK:
      rec.ExceptionCode = EXCEPTION_STACK_OVERFLOW;
      break;

    case INTR_PGFLT:
      if (info->si_signo == SIGSTKFLT) {
        rec.ExceptionCode = STATUS_GUARD_PAGE_VIOLATION;
      } else {
        rec.ExceptionCode = EXCEPTION_ACCESS_VIOLATION;
      }
      rec.NumberParameters = 2;
      rec.ExceptionInformation[0] = (void *) ((info->si_ctxt->errcode & 2) ? 1 : 0);
      rec.ExceptionInformation[1] = info->si_addr;
      break;

    case INTR_ALIGN:
      rec.ExceptionCode = EXCEPTION_DATATYPE_MISALIGNMENT;
      break;

    default:
      // Unknown exception, dispatch signal to native handler
      old_globalhandler(info);
  }

  raise_exception(&rec, &ctxt);

  convert_from_win32_context(info->si_ctxt, &ctxt);

  //syslog(LOG_DEBUG, "kernel32: sigexit", signum);
  sigexit(info, 0);
}

BOOL WINAPI CloseHandle(
    HANDLE hObject) {
  TRACE("CloseHandle");
  if (close((handle_t) hObject) < 0) return FALSE;
  return TRUE;
}

LONG WINAPI CompareFileTime(
    CONST FILETIME *lpFileTime1,
    CONST FILETIME *lpFileTime2) {
  TRACE("CompareFileTime");

  if (lpFileTime1->dwHighDateTime > lpFileTime2->dwHighDateTime) {
    return 1;
  } else if (lpFileTime1->dwHighDateTime < lpFileTime2->dwHighDateTime) {
    return -1;
  } else if (lpFileTime1->dwLowDateTime > lpFileTime2->dwLowDateTime) {
    return 1;
  } else if (lpFileTime1->dwLowDateTime < lpFileTime2->dwLowDateTime) {
    return -1;
  } else {
    return 0;
  }
}

BOOL WINAPI CreateDirectoryA(
    LPCTSTR lpPathName,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes) {
  int rc;

  TRACE("CreateDirectoryA");
  rc = mkdir(lpPathName, 0666);
  if (rc < 0) return FALSE;

  return TRUE;
}

BOOL WINAPI CreateDirectoryW(
    LPCWSTR lpPathName,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes) {
  char fn[MAXPATH];
  int rc;

  TRACE("CreateDirectoryW");
  
  rc = convert_filename_from_unicode(lpPathName, fn, MAXPATH);
  if (rc < 0) return FALSE;

  return CreateDirectoryA(fn, lpSecurityAttributes);
}

HANDLE WINAPI CreateEventA(
    LPSECURITY_ATTRIBUTES lpEventAttributes,
    BOOL bManualReset,
    BOOL bInitialState,
    LPCTSTR lpName) {
  TRACE("CreateEventA");
  if (lpName != NULL) panic("named event not supported");
  return (HANDLE) mkevent(bManualReset, bInitialState);
}

HANDLE WINAPI CreateFileA(
    LPCTSTR lpFileName, 
    DWORD dwDesiredAccess, 
    DWORD dwShareMode, 
    LPSECURITY_ATTRIBUTES lpSecurityAttributes, 
    DWORD dwCreationDisposition, 
    DWORD dwFlagsAndAttributes, 
    HANDLE hTemplateFile) {
  int flags = 0;
  int mode = 0;
  int shflags = 0;
  handle_t h;

  TRACE("CreateFile");

  if ((dwDesiredAccess & (GENERIC_READ | GENERIC_WRITE)) == (GENERIC_READ | GENERIC_WRITE)) {
    flags = O_RDWR;
  } else if (dwDesiredAccess & GENERIC_READ) {
    flags = O_RDONLY;
  } else if (dwDesiredAccess & GENERIC_WRITE) {
    flags = O_WRONLY;
  } else if (dwDesiredAccess & GENERIC_ALL) {
    flags = O_RDWR;
  } else {
    flags = O_RDONLY;
  }

  if ((dwShareMode & (FILE_SHARE_READ | FILE_SHARE_WRITE)) == (FILE_SHARE_READ | FILE_SHARE_WRITE)) {
    shflags = SH_DENYNO;
  } else if (dwShareMode & FILE_SHARE_READ) {
    shflags = SH_DENYWR;
  } else if (dwShareMode & FILE_SHARE_WRITE) {
    shflags = SH_DENYRD;
  } else {
    shflags = SH_DENYRW;
  }

  switch (dwCreationDisposition) {
    case CREATE_NEW:
      flags |= O_CREAT | O_EXCL;
      break;

    case CREATE_ALWAYS:
      flags |= O_CREAT | O_TRUNC;
      break;

    case OPEN_EXISTING:
      flags |= 0;
      break;

    case OPEN_ALWAYS:
      flags |= O_CREAT;
      break;

    case TRUNCATE_EXISTING:
      flags |= O_TRUNC;
      break;

    default:
      errno = EINVAL;
      return FALSE;
  }

  if (dwFlagsAndAttributes & FILE_ATTRIBUTE_READONLY) {
    mode = S_IREAD;
  } else {
    mode = S_IREAD | S_IWRITE | S_IEXEC;
  }

  if (dwFlagsAndAttributes & FILE_FLAG_OVERLAPPED) {
    syslog(LOG_WARNING, "CreateFile(%s): overlapped operation not supported", lpFileName);
    errno = EINVAL;
    return INVALID_HANDLE_VALUE;
  }

  if (dwFlagsAndAttributes & FILE_FLAG_NO_BUFFERING) flags |= O_DIRECT;
  if (dwFlagsAndAttributes & FILE_FLAG_RANDOM_ACCESS) flags |= O_RANDOM;
  if (dwFlagsAndAttributes & FILE_FLAG_SEQUENTIAL_SCAN) flags |= O_SEQUENTIAL;
  if (dwFlagsAndAttributes & FILE_FLAG_DELETE_ON_CLOSE) flags |= O_TEMPORARY;
  if (dwFlagsAndAttributes & FILE_ATTRIBUTE_TEMPORARY) flags |= O_SHORT_LIVED;

  h = sopen(lpFileName, flags, shflags, mode);
  if (h < 0) return INVALID_HANDLE_VALUE;

  return (HANDLE) h;
}

HANDLE WINAPI CreateFileMappingA(
    HANDLE hFile,
    LPSECURITY_ATTRIBUTES lpAttributes,
    DWORD flProtect,
    DWORD dwMaximumSizeHigh,
    DWORD dwMaximumSizeLow,
    LPCTSTR lpName) {
  TRACE("CreateFileMappingA");
  panic("CreateFileMappingA not implemented");
  return INVALID_HANDLE_VALUE;
}

HANDLE WINAPI CreateFileW(
    LPCWSTR lpFileName,
    DWORD dwDesiredAccess,
    DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes,
    HANDLE hTemplateFile) {
  char fn[MAXPATH];
  int rc;

  TRACE("CreateFileW");
  
  rc = convert_filename_from_unicode(lpFileName, fn, MAXPATH);
  if (rc < 0) return INVALID_HANDLE_VALUE;

  return CreateFileA(fn, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}

BOOL WINAPI CreatePipe(
    PHANDLE hReadPipe, 
    PHANDLE hWritePipe, 
    LPSECURITY_ATTRIBUTES lpPipeAttributes, 
    DWORD nSize) {
  TRACE("CreatePipe");
  panic("CreatePipe not implemented");
  return FALSE;
}

BOOL WINAPI CreateProcessA(
    LPCTSTR lpApplicationName,
    LPTSTR lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL bInheritHandles,
    DWORD dwCreationFlags,
    LPVOID lpEnvironment,
    LPCTSTR lpCurrentDirectory,
    LPSTARTUPINFO lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation) {
  TRACE("CreateProcessA");
  panic("CreateProcessA not implemented");
  return FALSE;
}

HANDLE WINAPI CreateSemaphoreA(
    LPSECURITY_ATTRIBUTES lpSemaphoreAttributes,
    LONG lInitialCount,
    LONG lMaximumCount,
    LPCTSTR lpName) {
  TRACE("CreateSemaphoreA");
  if (lpName != NULL) panic("named semaphores not supported");
  return (HANDLE) mksem(lInitialCount);
}

VOID WINAPI DebugBreak(VOID) {
  TRACE("DebugBreak");
  dbgbreak();
}

VOID WINAPI DeleteCriticalSection(
    LPCRITICAL_SECTION lpCriticalSection) {
  TRACE("DeleteCriticalSection");
  csfree(lpCriticalSection);
}

BOOL WINAPI DeleteFileA(
    LPCTSTR lpFileName) {
  TRACE("DeleteFileA");
  if (unlink(lpFileName) < 0) return FALSE;
  return TRUE;
}

BOOL WINAPI DeleteFileW(
    LPCWSTR lpFileName) {
  char fn[MAXPATH];
  int rc;

  TRACE("DeleteFileW");
  
  rc = convert_filename_from_unicode(lpFileName, fn, MAXPATH);
  if (rc < 0) return FALSE;
  
  return DeleteFileA(fn);
}

BOOL WINAPI DisableThreadLibraryCalls(
    HMODULE hModule) {
  TRACE("DisableThreadLibraryCalls");
  // ignore
  return TRUE;
}

BOOL WINAPI DuplicateHandle(
    HANDLE hSourceProcessHandle,
    HANDLE hSourceHandle,
    HANDLE hTargetProcessHandle,
    LPHANDLE lpTargetHandle,
    DWORD dwDesiredAccess,
    BOOL bInheritHandle,
    DWORD dwOptions) {
  TRACE("DuplicateHandle");
  *lpTargetHandle = (HANDLE) dup((handle_t) hSourceHandle);
  return TRUE;
}

VOID WINAPI EnterCriticalSection(
    LPCRITICAL_SECTION lpCriticalSection) {
  TRACEX("EnterCriticalSection");
  enter(lpCriticalSection);
}

BOOL WINAPI FileTimeToLocalFileTime(
    CONST FILETIME *lpFileTime,
    LPFILETIME lpLocalFileTime) {
  TRACEX("FileTimeToLocalFileTime");
  *lpLocalFileTime = *lpFileTime;
  return TRUE;
}

BOOL WINAPI FileTimeToSystemTime(
    CONST FILETIME *lpFileTime,
    LPSYSTEMTIME lpSystemTime) {
  time_t t;
  struct tm tm;

  TRACEX("FileTimeToSystemTime");

  t = (time_t) ((*(unsigned __int64 *) lpFileTime) - EPOC) / SECTIMESCALE;

  gmtime_r(&t, &tm);

  lpSystemTime->wYear = tm.tm_year + 1900; 
  lpSystemTime->wMonth = tm.tm_mon + 1; 
  lpSystemTime->wDayOfWeek = tm.tm_wday;
  lpSystemTime->wDay = tm.tm_mday; 
  lpSystemTime->wHour = tm.tm_hour; 
  lpSystemTime->wMinute = tm.tm_min; 
  lpSystemTime->wSecond = tm.tm_sec; 
  lpSystemTime->wMilliseconds = 0;

  return TRUE;
}

static void fill_find_data(LPWIN32_FIND_DATA fd, char *filename, struct stat64 *statbuf) {
  FILETIME *ft;

  memset(fd, 0, sizeof(WIN32_FIND_DATA));
  strcpy(fd->cFileName, filename);

  if ((statbuf->st_mode & S_IFMT) == S_IFDIR) {
    fd->dwFileAttributes |= FILE_ATTRIBUTE_DIRECTORY;
  } else {
    fd->dwFileAttributes |= FILE_ATTRIBUTE_NORMAL;
  }

  ft = (FILETIME *) &statbuf->st_size;
  fd->nFileSizeLow = ft->dwLowDateTime;
  fd->nFileSizeHigh = ft->dwHighDateTime;

  *(unsigned __int64 *) &(fd->ftCreationTime) = (unsigned __int64) statbuf->st_ctime * SECTIMESCALE + EPOC;
  *(unsigned __int64 *) &(fd->ftLastAccessTime) = (unsigned __int64) statbuf->st_atime * SECTIMESCALE + EPOC;
  *(unsigned __int64 *) &(fd->ftLastWriteTime) = (unsigned __int64) statbuf->st_mtime * SECTIMESCALE + EPOC;
}

static int like(char *str, char *mask) {
  if (!str) str = "";
  if (!mask) mask = "";

  while (*mask) {
    if (*mask == '*') {
      while (*mask == '*') mask++;
      if (*mask == 0) return 1;
      while (*str) {
        if (like(str, mask)) return 1;
        str++;
      }
      return 0;
    } else if (*mask == *str || *mask == '?' && *str != 0) {
      str++;
      mask++;
    } else {
      return 0;
    }
  }

  return *str == 0;
}

BOOL WINAPI FindClose(
    HANDLE hFindFile) {
  struct winfinddata *finddata;

  TRACE("FindClose");
  if (hFindFile == NOFINDHANDLE) {
    return TRUE;
  } else if (hFindFile == INVALID_HANDLE_VALUE) {
    return FALSE;
  } else {
    finddata = (struct winfinddata *) hFindFile;

    close(finddata->fhandle);
    free(finddata);

    return TRUE;
  }
}

HANDLE WINAPI FindFirstFileA(
    LPCTSTR lpFileName,
    LPWIN32_FIND_DATA lpFindFileData) {
  struct winfinddata *finddata;
  struct direntry dirent;
  struct stat64 statbuf;
  char *p;
  char *base;
  char fn[MAXPATH];

  TRACE("FindFirstFileA");
  //syslog(LOG_DEBUG | LOG_AUX, "FindFirstFile %s", lpFileName);

  p = (char *) lpFileName;
  while (*p != 0 && *p != '*' && *p != '?') p++;
  
  if (*p) {
    finddata = (struct winfinddata *) malloc(sizeof(struct winfinddata));
    if (!finddata) {
      errno = ENOMEM;
      return INVALID_HANDLE_VALUE;
    }

    strcpy(finddata->dir, lpFileName);
    base = NULL;
    p = finddata->dir;
    while (*p) {
      if (*p == PS1 || *p == PS2) base = p + 1;
      p++;
    }
    if (!base) {
      errno = ENOTDIR;
      free(finddata);
      return INVALID_HANDLE_VALUE;
    }
    base[-1] = 0;
    finddata->mask = base;

    if (finddata->mask[0] == '*' && finddata->mask[1] == '.' && finddata->mask[2] == '*' && finddata->mask[3] == 0) {
      finddata->mask[1] = 0;
    }

    finddata->fhandle = _opendir(finddata->dir);
    if (finddata->fhandle < 0) {
      free(finddata);
      return INVALID_HANDLE_VALUE;
    }

    while (_readdir(finddata->fhandle, &dirent, 1) > 0) {
      //syslog(LOG_DEBUG | LOG_AUX, "match %s with %s", dirent.name, finddata->mask);
      if (like(dirent.name, finddata->mask)) {
        strcpy(fn, finddata->dir);
        strcpy(fn + strlen(fn), "\\");
        strcpy(fn + strlen(fn), dirent.name);

        if (stat64(fn, &statbuf) < 0) {
          errno = ENOENT;
          free(finddata);
          return INVALID_HANDLE_VALUE;
        }

        fill_find_data(lpFindFileData, dirent.name, &statbuf);
        return (HANDLE) finddata;
      }
    }

    // Add dummy entry
    close(finddata->fhandle);
    free(finddata);
    if (stat64(finddata->dir, &statbuf) < 0) {
      errno = ENOENT;
      return INVALID_HANDLE_VALUE;
    }

    fill_find_data(lpFindFileData, ".", &statbuf);
    return NOFINDHANDLE;
  } else {
    if (stat64((char *) lpFileName, &statbuf) < 0) {
      errno = ENOENT;
      return INVALID_HANDLE_VALUE;
    }

    p = (char *) lpFileName;
    base = p;
    while (*p) {
      if (*p == PS1 || *p == PS2) base = p + 1;
      p++;
    }

    fill_find_data(lpFindFileData, base, &statbuf);

    return NOFINDHANDLE;
  }
}

HANDLE WINAPI FindFirstFileW(
    LPCWSTR lpFileName,
    LPWIN32_FIND_DATAW lpFindFileData) {
  char fn[MAXPATH];
  int rc;
  WIN32_FIND_DATA fd;
  HANDLE fh;

  rc = convert_filename_from_unicode(lpFileName, fn, MAXPATH);
  if (rc < 0) return INVALID_HANDLE_VALUE;

  fh = FindFirstFileA(fn, &fd);
  if (fh == INVALID_HANDLE_VALUE) return INVALID_HANDLE_VALUE;

  memcpy(lpFindFileData, &fd, sizeof(WIN32_FIND_DATA) - MAX_PATH - 14);
  convert_filename_to_unicode(fd.cFileName, lpFindFileData->cFileName, MAXPATH);
  return fh;
}

BOOL WINAPI FindNextFileA(
    HANDLE hFindFile,
    LPWIN32_FIND_DATA lpFindFileData) {
  struct winfinddata *finddata;
  struct direntry dirent;
  struct stat64 statbuf;
  char fn[MAXPATH];

  TRACE("FindNextFileA");

  if (hFindFile == NOFINDHANDLE) {
    errno = ESRCH;
    return FALSE;
  } else {
    finddata = (struct winfinddata *) hFindFile;

    while (_readdir(finddata->fhandle, &dirent, 1) > 0) {
      //syslog(LOG_DEBUG | LOG_AUX, "match next %s with %s", dirent.name, finddata->mask);
      if (like(dirent.name, finddata->mask)) {
        strcpy(fn, finddata->dir);
        strcpy(fn + strlen(fn), "\\");
        strcpy(fn + strlen(fn), dirent.name);

        if (stat64(fn, &statbuf) < 0) {
          errno = ENOENT;
          return FALSE;
        }

        fill_find_data(lpFindFileData, dirent.name, &statbuf);
        return TRUE;
      }
    }

    errno = ESRCH;
    return FALSE;
  }
}

BOOL WINAPI FindNextFileW(
    HANDLE hFindFile,
    LPWIN32_FIND_DATAW lpFindFileData) {
  WIN32_FIND_DATA fd;
  BOOL ok;

  TRACE("FindNextFileW");

  ok = FindNextFileA(hFindFile, &fd);
  if (!ok) return FALSE;

  memcpy(lpFindFileData, &fd, sizeof(WIN32_FIND_DATA) - MAX_PATH - 14);
  convert_filename_to_unicode(fd.cFileName, lpFindFileData->cFileName, MAXPATH);

  return TRUE;
}

BOOL WINAPI FlushFileBuffers(
    HANDLE hFile) {
  TRACE("FlushFileBuffers");
  if (fsync((handle_t) hFile) < 0) return FALSE;
  return TRUE;
}

BOOL WINAPI FlushViewOfFile(
    LPCVOID lpBaseAddress,
    SIZE_T dwNumberOfBytesToFlush) {
  panic("FlushViewOfFile not implemented");
  return FALSE;
}

DWORD WINAPI FormatMessageA(
    DWORD dwFlags,
    LPCVOID lpSource,
    DWORD dwMessageId,
    DWORD dwLanguageId,
    LPTSTR lpBuffer,
    DWORD nSize,
    va_list *Arguments) {
  char *buf;

  TRACE("FormatMessageA");

  // TODO: generate more descriptive message
  if (dwFlags & FORMAT_MESSAGE_ALLOCATE_BUFFER) {
    buf = malloc(nSize > 256 ? nSize : 256);
    if (!buf) {
      errno = ENOMEM;
      return 0;
    }

    *(char **) lpBuffer = buf;
  } else {
    buf = lpBuffer;
  }

  sprintf(buf, "Error 0x%08X.\n", dwMessageId);
  return strlen(buf);
}

BOOL WINAPI FreeLibrary(
    HMODULE hModule) {
  TRACE("FreeLibrary");
  if (dlclose((hmodule_t) hModule) < 0) return FALSE;
  return TRUE;
}

DWORD WINAPI GetCurrentDirectoryA(
    DWORD nBufferLength,
    LPTSTR lpBuffer) {
  char curdir[MAXPATH];

  TRACE("GetCurrentDirectoryA");
  
  if (!getcwd(curdir, MAXPATH)) return 0;
  if (nBufferLength < strlen(curdir) + 3) return strlen(curdir) + 3;
  lpBuffer[0] = 'c';
  lpBuffer[1] = ':';
  strcpy(lpBuffer + 2, curdir);
  return strlen(lpBuffer);
}

HANDLE WINAPI GetCurrentProcess(VOID) {
  TRACE("GetCurrentProcess");
  return (HANDLE) 1;
}

HANDLE WINAPI GetCurrentThread(VOID) {
  TRACE("GetCurrentThread");
  return (HANDLE) self();
}

DWORD WINAPI GetCurrentThreadId(VOID) {
  TRACE("GetCurrentThreadId");
  return gettid();
}

DWORD WINAPI GetEnvironmentVariableA(
    LPCTSTR lpName,
    LPTSTR lpBuffer,
    DWORD nSize) {
  char *value;

  TRACE("GetEnvironmentVariableA");
  //syslog(LOG_DEBUG, "GetEnvironmentVariable(%s)", lpName);
  
  value = get_property(osconfig(), "env", (char *) lpName, NULL);
  if (value) {
    strcpy(lpBuffer, value);
    return strlen(value);
  } else {
    memset(lpBuffer, 0, nSize);
    return 0;
  }
}

BOOL WINAPI GetExitCodeProcess(
    HANDLE hProcess,
    LPDWORD lpExitCode) {
  TRACE("GetExitCodeProcess");
  panic("GetExitCodeProcess not implemented");
  return 0;
}

BOOL WINAPI GetExitCodeThread(
    HANDLE hThread,
    LPDWORD lpExitCode) {
  TRACE("GetExitCodeThread");
  panic("GetExitCodeThread not implemented");
  return FALSE;
}

DWORD WINAPI GetFileAttributesA(
    LPCTSTR lpFileName) {
  struct stat64 fs;
  unsigned long attrs;

  if (stat64((char *) lpFileName, &fs) < 0) return -1;

  switch (fs.st_mode & S_IFMT) {
    case S_IFREG:
      attrs = FILE_ATTRIBUTE_NORMAL;
      break;

    case S_IFDIR:
      attrs = FILE_ATTRIBUTE_DIRECTORY;
      break;

    case S_IFSOCK:
    case S_IFBLK:
    case S_IFCHR:
    case S_IFIFO:
      attrs = FILE_ATTRIBUTE_DEVICE;
      break;

    case S_IFLNK:
      attrs = 0;
      break;

    default:
      attrs = 0;
  }

  if (!(fs.st_mode & S_IWRITE)) attrs |= FILE_ATTRIBUTE_READONLY;
  // TODO: set other attributes for files (hidden, system, etc.)

  return attrs;
}

DWORD WINAPI GetFileAttributesW(
    LPCWSTR lpFileName) {
  char fn[MAXPATH];
  int rc;

  TRACE("GetFileAttributesW");

  rc = convert_filename_from_unicode(lpFileName, fn, MAXPATH);
  if (rc < 0) return -1;

  rc = GetFileAttributesA(fn);
  //syslog(LOG_DEBUG, "GetFileAttributesW(%s)=%08X", fn, rc);
  return rc;
}

DWORD WINAPI GetFileSize(
    HANDLE hFile,
    LPDWORD lpFileSizeHigh) {
  LARGE_INTEGER size;

  TRACE("GetFileSize");

  size.QuadPart = fstat64((handle_t) hFile, NULL);
  if (size.QuadPart < 0) return INVALID_FILE_SIZE;
  if (lpFileSizeHigh) *lpFileSizeHigh = size.HighPart;
  return size.LowPart;
}

BOOL WINAPI GetFileTime(
    HANDLE hFile,
    LPFILETIME lpCreationTime,
    LPFILETIME lpLastAccessTime,
    LPFILETIME lpLastWriteTime) {
  struct stat64 statbuf;

  TRACE("GetFileTime");

  if (fstat64((handle_t) hFile, &statbuf) < 0) return FALSE;

  if (lpCreationTime) {
    *(unsigned __int64 *) lpCreationTime = (unsigned __int64) statbuf.st_ctime * SECTIMESCALE + EPOC;
  }

  if (lpLastAccessTime) {
    *(unsigned __int64 *) lpLastAccessTime = (unsigned __int64) statbuf.st_atime * SECTIMESCALE + EPOC;
  }

  if (lpLastWriteTime) {
    *(unsigned __int64 *) lpLastWriteTime = (unsigned __int64) statbuf.st_mtime * SECTIMESCALE + EPOC;
  }

  return TRUE;
}

DWORD WINAPI GetFullPathNameA(
    LPCTSTR lpFileName,
    DWORD nBufferLength,
    LPTSTR lpBuffer,
    LPTSTR *lpFilePart) {
  char fn[MAXPATH];
  int rc;
  char *basename;
  char *p;

  TRACE("GetFullPathName");

  rc = canonicalize(lpFileName, fn, MAXPATH);
  if (rc < 0) return 0;

  if (strlen(fn) + 2 >= nBufferLength) return strlen(fn) + 3;

  strcpy(lpBuffer, "c:");
  strcat(lpBuffer, fn);

  p = basename = lpBuffer;
  while (*p) {
    if (*p == PS1 || *p == PS2) basename = p + 1;
    p++;
  }
  *lpFilePart = basename;

  return strlen(lpBuffer);
}

DWORD WINAPI GetLastError(VOID) {
  TRACE("GetLastError");
  // TODO: implement better error reporting
  if (errno == 0) return 0;
  if (errno == ENOENT) return ERROR_FILE_NOT_FOUND;
  if (errno == ESRCH) return ERROR_NO_MORE_FILES;
  return 0x80040000 | errno;
}

DWORD WINAPI GetLogicalDrives(VOID) {
  TRACE("GetLogicalDrives");
  return 4; // Drive C:
}

DWORD WINAPI GetModuleFileNameA(
    HMODULE hModule,
    LPTSTR lpFilename,
    DWORD nSize) {
  int rc;

  TRACE("GetModuleFileNameA");
  rc = getmodpath((hmodule_t) hModule, lpFilename, nSize);
  //syslog(LOG_DEBUG, "Module filename for %p is %s", hModule, lpFilename);
  return rc;
}

HMODULE WINAPI GetModuleHandleA(
    LPCTSTR lpModuleName) {
  hmodule_t hmod;

  TRACE("GetModuleHandleA");
  hmod = getmodule(lpModuleName);
  //syslog(LOG_DEBUG, "GetModuleHandleA(%s) returned %p", lpModuleName, hmod);
  return (HMODULE) hmod;
}

BOOL WINAPI GetNumberOfConsoleInputEvents(
    HANDLE hConsoleInput,
    LPDWORD lpcNumberOfEvents) {
  int rc;

  TRACE("GetNumberOfConsoleInputEvents");

  // TODO: Here we assume that the handle refers to /dev/console
  rc = ioctl(fdin, IOCTL_KBHIT, NULL, 0);
  if (rc < 0) return FALSE;
  if (rc > 0) {
    *lpcNumberOfEvents = 1;
  } else {
    *lpcNumberOfEvents = 0;
  }

  return TRUE;
}

BOOL WINAPI GetOverlappedResult(
    HANDLE hFile,
    LPOVERLAPPED lpOverlapped,
    LPDWORD lpNumberOfBytesTransferred,
    BOOL bWait) {
  TRACE("GetOverlappedResult");
  panic("GetOverlappedResult not implemented");
  return FALSE;
}

FARPROC WINAPI GetProcAddress(
    HMODULE hModule,
    LPCSTR lpProcName) {
  TRACE("GetProcAddress");
  //syslog(LOG_DEBUG, "GetProcAddress name: %s hmod: %08X", lpProcName, hModule);
  return dlsym((hmodule_t) hModule, (char *) lpProcName);
}

BOOL WINAPI GetProcessAffinityMask(
    HANDLE hProcess,
    LPDWORD lpProcessAffinityMask,
    LPDWORD lpSystemAffinityMask) {
  TRACE("GetProcessAffinityMask");
  panic("GetProcessAffinityMask not implemented");
  return TRUE;
}

HANDLE WINAPI GetProcessHeap(VOID) {
  TRACE("GetProcessHeap");
  return PROCESSHEAP;
}

HANDLE WINAPI GetStdHandle(
    DWORD nStdHandle) {
  TRACE("GetStdHandle");

  switch (nStdHandle) {
    case STD_INPUT_HANDLE:  return (HANDLE) fdin;
    case STD_OUTPUT_HANDLE: return (HANDLE) fdout;
    case STD_ERROR_HANDLE:  return (HANDLE) fderr;
  }

  return INVALID_HANDLE_VALUE;
}

UINT WINAPI GetSystemDirectoryA(
    LPTSTR lpBuffer,
    UINT uSize) {
  TRACE("GetSystemDirectoryA");
  strcpy(lpBuffer, get_property(osconfig(), "win32", "sysdir", "c:\\bin"));
  return strlen(lpBuffer);
}

VOID WINAPI GetSystemInfo(
    LPSYSTEM_INFO lpSystemInfo) {
  struct cpuinfo cpu;

  TRACE("GetSystemInfo");
  
  sysinfo(SYSINFO_CPU, &cpu, sizeof(cpu));
  memset(lpSystemInfo, 0, sizeof(SYSTEM_INFO));
  lpSystemInfo->dwNumberOfProcessors = 1;

  lpSystemInfo->wProcessorLevel = cpu.cpu_family;

  if (cpu.cpu_family == 3) {
    lpSystemInfo->dwProcessorType = 386;
    lpSystemInfo->wProcessorRevision = 0xFFA0;
  } else if (cpu.cpu_family == 4) {
    lpSystemInfo->dwProcessorType = 486;
    lpSystemInfo->wProcessorRevision = 0xFFA0;
  } else {
    lpSystemInfo->dwProcessorType = 586;
    lpSystemInfo->wProcessorRevision = (cpu.cpu_model << 16) | cpu.cpu_stepping;
  }

  lpSystemInfo->dwPageSize = cpu.pagesize;
  lpSystemInfo->dwAllocationGranularity = cpu.pagesize;
  lpSystemInfo->lpMinimumApplicationAddress = (void *) 0x10000;
  lpSystemInfo->lpMaximumApplicationAddress = (void *) 0x7FFFFFFF;
}

VOID WINAPI GetSystemTime(
    LPSYSTEMTIME lpSystemTime) {
  struct timeval tv;
  struct tm tm;

  TRACE("GetSystemTime");
  gettimeofday(&tv, NULL);
  gmtime_r(&tv.tv_sec, &tm);

  lpSystemTime->wYear = tm.tm_year + 1900; 
  lpSystemTime->wMonth = tm.tm_mon + 1; 
  lpSystemTime->wDayOfWeek = tm.tm_wday;
  lpSystemTime->wDay = tm.tm_mday; 
  lpSystemTime->wHour = tm.tm_hour; 
  lpSystemTime->wMinute = tm.tm_min; 
  lpSystemTime->wSecond = tm.tm_sec; 
  lpSystemTime->wMilliseconds = (WORD) (tv.tv_usec / 1000);
}

VOID WINAPI GetSystemTimeAsFileTime(
    LPFILETIME lpSystemTimeAsFileTime) {
  struct timeval tv;

  TRACE("GetSystemTimeAsFileTime");
  gettimeofday(&tv, NULL);
  *(unsigned __int64 *) lpSystemTimeAsFileTime = ((unsigned __int64) (tv.tv_sec) * 1000000 + (unsigned __int64) (tv.tv_usec)) * MICROTIMESCALE + EPOC;
}

DWORD WINAPI GetTempPathA(
    DWORD nBufferLength,
    LPTSTR lpBuffer) {
  TRACE("GetTempPathA");
  strcpy(lpBuffer, get_property(osconfig(), "win32", "tmpdir", "c:\\tmp"));
  return strlen(lpBuffer);
}

BOOL WINAPI GetThreadContext(
    HANDLE hThread,
    LPCONTEXT lpContext) {
  struct context ctxt;
  int rc;

  TRACE("GetThreadContext");
  rc = getcontext((handle_t) hThread, &ctxt);
  if (rc < 0) {
    if (errno == EPERM) {
      // Thread does not have a usermode context, just return dummy context
      memset(lpContext, 0, sizeof(CONTEXT));
      return TRUE;
    }

    syslog(LOG_DEBUG, "GetThreadContext(%d) failed", hThread);
    return FALSE;
  }

  convert_to_win32_context(&ctxt, lpContext);
  return TRUE;
}

LCID WINAPI GetThreadLocale(VOID) {
  TRACE("GetThreadLocale");
  return get_numeric_property(osconfig(), "win32", "locale", 0x0406);
}

int WINAPI GetThreadPriority(
    HANDLE hThread) {
  int prio;

  TRACE("GetThreadPriority");

  prio = getprio((handle_t) hThread);
  if (prio < 0) return 0x7FFFFFFF;

  // Return priority based on win32 normal priority class scheme
  switch (prio) {
    case 1: return THREAD_PRIORITY_IDLE;
    case 6: return THREAD_PRIORITY_LOWEST;
    case 7: return THREAD_PRIORITY_BELOW_NORMAL;
    case 8: return THREAD_PRIORITY_NORMAL;
    case 9: return THREAD_PRIORITY_ABOVE_NORMAL;
    case 10: return THREAD_PRIORITY_HIGHEST;
    case 15: return THREAD_PRIORITY_TIME_CRITICAL;
  }

  return 0x7FFFFFFF;
}

BOOL WINAPI GetThreadTimes(
    HANDLE hThread,
    LPFILETIME lpCreationTime,
    LPFILETIME lpExitTime,
    LPFILETIME lpKernelTime,
    LPFILETIME lpUserTime) {
  TRACE("GetThreadTimes");
  panic("GetThreadTimes not implemented");
  return FALSE;
}

DWORD WINAPI GetTickCount(VOID) {
  TRACE("GetTickCount");
  return clock();
}

DWORD WINAPI GetTimeZoneInformation(
    LPTIME_ZONE_INFORMATION lpTimeZoneInformation) {
  TRACE("GetTimeZoneInformation");
  // TODO: return real time zone information (used by win32\native\java\util\TimeZone_md.c)
  return -1;
}

BOOL WINAPI GetVersionExA(
    LPOSVERSIONINFO lpVersionInfo) {
  TRACE("GetVersionExA");
  lpVersionInfo->dwMajorVersion = 5;
  lpVersionInfo->dwMinorVersion = 0;
  lpVersionInfo->dwPlatformId = 2; // VER_PLATFORM_WIN32_NT
  strcpy(lpVersionInfo->szCSDVersion, "sanos");

  return TRUE;
}

BOOL WINAPI GetVolumeInformationA(
    LPCTSTR lpRootPathName,
    LPTSTR lpVolumeNameBuffer,
    DWORD nVolumeNameSize,
    LPDWORD lpVolumeSerialNumber,
    LPDWORD lpMaximumComponentLength,
    LPDWORD lpFileSystemFlags,
    LPTSTR lpFileSystemNameBuffer,
    DWORD nFileSystemNameSize) {
  TRACE("GetVolumeInformationA");

  if (lpVolumeNameBuffer) *lpVolumeNameBuffer = 0;
  if (lpVolumeSerialNumber) *lpVolumeSerialNumber = 0;
  if (lpMaximumComponentLength) *lpMaximumComponentLength = MAXPATH - 1;
  if (lpFileSystemFlags) *lpFileSystemFlags = 0;
  if (lpFileSystemNameBuffer) *lpFileSystemNameBuffer = 0;

  return TRUE;
}

UINT WINAPI GetWindowsDirectoryA(
    LPTSTR lpBuffer,
    UINT uSize) {
  TRACE("GetWindowsDirectoryA");
  strcpy(lpBuffer, get_property(osconfig(), "win32", "windir", "c:\\bin"));
  return strlen(lpBuffer);
}

VOID WINAPI GlobalMemoryStatus(
    LPMEMORYSTATUS lpBuffer) {
  struct meminfo mem;

  TRACE("GlobalMemoryStatus");

  sysinfo(SYSINFO_MEM, &mem, sizeof(mem));

  lpBuffer->dwLength = sizeof(MEMORYSTATUS);
  lpBuffer->dwMemoryLoad = ((mem.physmem_total - mem.physmem_avail) / mem.pagesize) / (mem.physmem_total / mem.pagesize);
  lpBuffer->dwTotalPhys = mem.physmem_total;
  lpBuffer->dwAvailPhys = mem.physmem_avail;
  lpBuffer->dwTotalPageFile = mem.physmem_total;
  lpBuffer->dwAvailPageFile = mem.physmem_avail;
  lpBuffer->dwTotalVirtual = mem.virtmem_total;
  lpBuffer->dwAvailVirtual = mem.virtmem_avail;
}

LPVOID WINAPI HeapAlloc(
    HANDLE hHeap,
    DWORD dwFlags,
    SIZE_T dwBytes) {
  TRACE("HeapAlloc");
  if (hHeap != PROCESSHEAP) {
    syslog(LOG_DEBUG, "warning: HeapAlloc only supported for process heap");
    return NULL;
  }

  return malloc(dwBytes);
}

BOOL WINAPI HeapFree(
    HANDLE hHeap,
    DWORD dwFlags,
    LPVOID lpMem) {
  TRACE("HeapFree");
  if (hHeap != PROCESSHEAP) {
    syslog(LOG_DEBUG, "warning: HeapFree only supported for process heap");
    return FALSE;
  }

  free(lpMem);
  return TRUE;
}


VOID WINAPI InitializeCriticalSection(
    LPCRITICAL_SECTION lpCriticalSection) {
  TRACE("InitializeCriticalSection");
  mkcs((struct critsect *) lpCriticalSection);
}

#ifndef __TINYC__
__declspec(naked) LONG WINAPI InterlockedDecrement(
    LPLONG volatile lpAddend) {
  __asm {
    mov   ecx,dword ptr [esp+4]
    mov   eax,0FFFFFFFFh
    nop
    xadd  dword ptr [ecx],eax
    dec   eax
    ret   4
  }
}

__declspec(naked) LONG WINAPI InterlockedExchange(
    LPLONG volatile Target,
    LONG Value) {
  __asm {
    mov ecx,dword ptr [esp+4]
    mov edx,dword ptr [esp+8]
    mov eax,dword ptr [ecx]
ileagain:
    lock cmpxchg dword ptr [ecx],edx
    jne ileagain
    ret 8
  }
}

__declspec(naked) LONG WINAPI InterlockedIncrement(
    LPLONG volatile lpAddend) {
  __asm {
    mov   ecx,dword ptr [esp+4]
    mov   eax,1
    nop
    xadd  dword ptr [ecx],eax
    inc   eax
    ret   4
  }
}
#endif

BOOL WINAPI IsDBCSLeadByte(
    BYTE TestChar) {
  TRACEX("IsDBCSLeadByte");
  // TODO: test is lead byte
  //syslog(LOG_DEBUG, "IsDBCSLeadByte called");
  return FALSE;
}

BOOL WINAPI IsValidCodePage(
    UINT CodePage) {
  TRACE("IsValidCodePage");
  panic("IsValidCodePage not implemented");
  return FALSE;
}

VOID WINAPI LeaveCriticalSection(
    LPCRITICAL_SECTION lpCriticalSection) {
  TRACEX("LeaveCriticalSection");
  leave(lpCriticalSection);
}

HMODULE WINAPI LoadLibraryA(
    LPCTSTR lpFileName) {
  TRACE("LoadLibraryA");
  return (HMODULE) dlopen((char *) lpFileName, 0);
}

BOOL WINAPI LockFile(
    HANDLE hFile,
    DWORD dwFileOffsetLow,
    DWORD dwFileOffsetHigh,
    DWORD nNumberOfBytesToLockLow,
    DWORD nNumberOfBytesToLockHigh) {
  panic("LockFile not implemented");
  return FALSE;
}

BOOL WINAPI LockFileEx(
    HANDLE hFile,
    DWORD dwFlags,
    DWORD dwReserved,
    DWORD nNumberOfBytesToLockLow,
    DWORD nNumberOfBytesToLockHigh,
    LPOVERLAPPED lpOverlapped) {
  panic("LockFileEx not implemented");
  return FALSE;
}

LPVOID WINAPI MapViewOfFile(
    HANDLE hFileMappingObject,
    DWORD dwDesiredAccess,
    DWORD dwFileOffsetHigh,
    DWORD dwFileOffsetLow,
    SIZE_T dwNumberOfBytesToMap) {
  TRACE("MapViewOfFile");
  panic("MapViewOfFile not implemented");
  return NULL;
}

BOOL WINAPI MoveFileA(
    LPCTSTR lpExistingFileName,
    LPCTSTR lpNewFileName) {
  TRACE("MoveFileA");
  if (rename(lpExistingFileName, lpNewFileName) < 0) return FALSE;

  return TRUE;
}

int WINAPI MultiByteToWideChar(
    UINT CodePage,         // code page
    DWORD dwFlags,         // character-type options
    LPCSTR lpMultiByteStr, // string to map
    int cbMultiByte,       // number of bytes in string
    LPWSTR lpWideCharStr,  // wide-character buffer
    int cchWideChar) {     // size of buffer
  TRACE("MultiByteToWideChar");
  panic("MultiByteToWideChar not implemented");
  return 0;
}

HANDLE WINAPI OpenFileMappingA(
    DWORD dwDesiredAccess,
    BOOL bInheritHandle,
    LPCTSTR lpName) {
  TRACE("OpenFileMappingA");
  panic("OpenFileMappingA not implemented");
  return INVALID_HANDLE_VALUE;
}

HANDLE WINAPI OpenProcess(
    DWORD dwDesiredAccess,
    BOOL bInheritHandle,
    DWORD dwProcessId) {
  TRACE("OpenProcess");
  panic("OpenProcess not implemented");
  return NULL;
}

BOOL WINAPI PeekConsoleInputA(
    HANDLE hConsoleInput,
    PINPUT_RECORD lpBuffer,
    DWORD nLength,
    LPDWORD lpNumberOfEventsRead) {
  int rc;

  TRACE("PeekConsoleInputA");

  // TODO: Here we assume that the handle refers to /dev/console
  // Just return a bogus key event record if keyboard buffer is not empty
  rc = ioctl(fdin, IOCTL_KBHIT, NULL, 0);
  if (rc < 0) return FALSE;
  if (rc > 0) {
    KEY_EVENT_RECORD *rec = (KEY_EVENT_RECORD *) lpBuffer;
    rec->bKeyDown = TRUE;
    rec->wRepeatCount = 1;
    rec->wVirtualKeyCode = 0;
    rec->wVirtualScanCode = 0;
    rec->uChar.AsciiChar = ' ';
    rec->dwControlKeyState = 0;
    *lpNumberOfEventsRead = 1;
  } else {
    *lpNumberOfEventsRead = 0;
  }

  return TRUE;
}

BOOL WINAPI PeekNamedPipe(
    HANDLE hNamedPipe,
    LPVOID lpBuffer,
    DWORD nBufferSize,
    LPDWORD lpBytesRead,
    LPDWORD lpTotalBytesAvail,
    LPDWORD lpBytesLeftThisMessage) {
  TRACE("PeekNamedPipe");
  panic("PeekNamedPipe not implemented");
  return FALSE;
}

BOOL WINAPI QueryPerformanceCounter(
    LARGE_INTEGER *lpPerformanceCount) {
  TRACEX("QueryPerformanceCounter");
  lpPerformanceCount->HighPart = 0;
  lpPerformanceCount->LowPart = clock();

  return TRUE;
}

BOOL WINAPI QueryPerformanceFrequency(
    LARGE_INTEGER *lpFrequency) {
  TRACE("QueryPerformanceFrequency");
  // Tick count is in milliseconds
  lpFrequency->HighPart = 0;
  lpFrequency->LowPart = 1000;

  return TRUE;
}

BOOL WINAPI ReadFile(
    HANDLE hFile,
    LPVOID lpBuffer,
    DWORD nNumberOfBytesToRead,
    LPDWORD lpNumberOfBytesRead,
    LPOVERLAPPED lpOverlapped) {
  int rc;

  TRACE("ReadFile");
  if (lpOverlapped) panic("Overlapped I/O not implemented in ReadFile");

  rc = read((handle_t) hFile, lpBuffer, nNumberOfBytesToRead);
  if (rc < 0) return FALSE;

  *lpNumberOfBytesRead = rc;
  return TRUE;
}

BOOL WINAPI ReleaseSemaphore(
    HANDLE hSemaphore,
    LONG lReleaseCount,
    LPLONG lpPreviousCount) {
  LONG dummy;

  TRACE("ReleaseSemaphore");
  if (!lpPreviousCount) lpPreviousCount = &dummy;
  *lpPreviousCount = semrel((handle_t) hSemaphore, lReleaseCount);
  return TRUE;
}

BOOL WINAPI RemoveDirectoryA(
    LPCTSTR lpPathName) {
  TRACE("RemoveDirectoryA");
  if (rmdir(lpPathName) < 0) return FALSE;
  return TRUE;
}

BOOL WINAPI RemoveDirectoryW(
    LPCWSTR lpPathName) {
  char path[MAXPATH];
  int rc;

  TRACE("RemoveDirectoryW");

  rc = convert_filename_from_unicode(lpPathName, path, MAXPATH);
  if (rc < 0) return FALSE;

  return RemoveDirectoryA(path);
}

BOOL WINAPI ResetEvent(
    HANDLE hEvent) {
  TRACE("ResetEvent");
  ereset((handle_t) hEvent);
  return TRUE;
}

DWORD WINAPI ResumeThread(
    HANDLE hThread) {
  TRACE("ResumeThread");
  return resume((handle_t) hThread);
}

VOID WINAPI RtlUnwind(
    PEXCEPTION_FRAME endframe, 
    LPVOID eip, 
    PEXCEPTION_RECORD rec, 
    DWORD retval) {
  CONTEXT ctxt;

  TRACE("RtlUnwind");
  memset(&ctxt, 0, sizeof(CONTEXT));
  unwind(endframe, eip, rec, retval, &ctxt);
}

DWORD WINAPI SearchPathA(
    LPCSTR lpPath,
    LPCSTR lpFileName,
    LPCSTR lpExtension,
    DWORD nBufferLength,
    LPSTR lpBuffer,
    LPSTR *lpFilePart) {
  TRACE("SearchPathA");
  panic("SearchPathA not implemented");
  return 0;
}

BOOL WINAPI SetConsoleCtrlHandler(
    PHANDLER_ROUTINE HandlerRoutine,
    BOOL Add) {
  TRACE("SetConsoleCtrlHandler");
  //syslog(LOG_DEBUG, "warning: SetConsoleCtrlHandler not implemented, ignored");
  return TRUE;
}

BOOL WINAPI SetConsoleTitleA(
    LPCTSTR lpConsoleTitle) {
  TRACE("SetConsoleTitle");
  return TRUE;
}

BOOL WINAPI SetEndOfFile(
    HANDLE hFile)
{
  TRACE("SetEndOfFile");
  if (ftruncate((handle_t) hFile, tell((handle_t) hFile)) < 0) return FALSE;
  return TRUE;
}

BOOL WINAPI SetEvent(
    HANDLE hEvent) {
  TRACE("SetEvent");
  eset((handle_t) hEvent);
  return TRUE;
}

BOOL WINAPI SetFileAttributesA(
    LPCTSTR lpFileName,
    DWORD dwFileAttributes) {
  TRACE("SetFileAttributesA");
  if (dwFileAttributes != 0) syslog(LOG_DEBUG, "warning: SetFileAttributesA(%s,%08x) not implemented, ignored", lpFileName, dwFileAttributes);
  return TRUE;
}

BOOL WINAPI SetFileAttributesW(
    LPCWSTR lpFileName,
    DWORD dwFileAttributes) {
  char fn[MAXPATH];
  int rc;

  TRACE("SetFileAttributesW");
  
  rc = convert_filename_from_unicode(lpFileName, fn, MAXPATH);
  if (rc < 0) return 0;

  return SetFileAttributesA(fn, dwFileAttributes);
}

DWORD WINAPI SetFilePointer(
    HANDLE hFile,
    LONG lDistanceToMove,
    PLONG lpDistanceToMoveHigh,
    DWORD dwMoveMethod) {
  int rc;
  TRACE("SetFilePointer");
  
  if (lpDistanceToMoveHigh) {
    LARGE_INTEGER offset;
    LARGE_INTEGER retval;

    offset.LowPart = lDistanceToMove;
    offset.HighPart = *lpDistanceToMoveHigh;

    retval.QuadPart = lseek64((handle_t) hFile, offset.QuadPart, dwMoveMethod);
    if (retval.QuadPart < 0) {
      rc = -1;
    } else {
      *lpDistanceToMoveHigh = retval.HighPart;
      rc = retval.LowPart;
    }
  } else {
    rc = lseek((handle_t) hFile, lDistanceToMove, dwMoveMethod);
    if (rc < 0) rc = -1;
  }

  return rc;
}

BOOL WINAPI SetFileTime(
    HANDLE hFile,
    CONST FILETIME *lpCreationTime,
    CONST FILETIME *lpLastAccessTime,
    CONST FILETIME *lpLastWriteTime) {
  struct utimbuf times;
  int rc;

  TRACE("SetFileTime");

  if (lpCreationTime != NULL) {
    times.ctime = (time_t)(((*(__int64 *) lpCreationTime) - EPOC) / SECTIMESCALE);
  } else {
    times.ctime = -1;
  }

  if (lpLastAccessTime != NULL) {
    times.actime = (time_t)(((*(__int64 *) lpLastAccessTime) - EPOC) / SECTIMESCALE);
  } else {
    times.actime = -1;
  }

  if (lpLastWriteTime != NULL) {
    times.modtime = (time_t)(((*(__int64 *) lpLastWriteTime) - EPOC) / SECTIMESCALE);
  } else {
    times.modtime = -1;
  }

  rc = futime((handle_t) hFile, &times);
  if (rc < 0) return FALSE;

  return TRUE;
}

BOOL WINAPI SetHandleInformation(
    HANDLE hObject,
    DWORD dwMask,
    DWORD dwFlags) {
  TRACE("SetHandleInformation");
  // Ignored, only used for setting handle inheritance
  //syslog(LOG_DEBUG, "warning: SetHandleInformation ignored");
  return TRUE;
}

BOOL WINAPI SetThreadContext(
    HANDLE hThread,
    CONST CONTEXT *lpContext) {
  struct context ctxt;

  TRACE("SetThreadContext");

  if (lpContext->ContextFlags != (CONTEXT_CONTROL | CONTEXT_INTEGER | CONTEXT_SEGMENTS)) {
    syslog(LOG_WARNING, "kernel32: incomplete context in SetContext, flags %lx", lpContext->ContextFlags); 
  }

  memset(&ctxt, 0, sizeof(struct context));
  convert_from_win32_context(&ctxt, (CONTEXT *) lpContext);
  if (setcontext((handle_t) hThread, &ctxt) < 0) return FALSE;

  return TRUE;
}

BOOL WINAPI SetThreadPriority(
    HANDLE hThread,
    int nPriority) {
  int prio;

  TRACE("SetThreadPriority");

  // Set new thread priority based on win32 normal priority class scheme
  switch (nPriority) {
    case THREAD_PRIORITY_IDLE:
      prio = 1;
      break;

    case THREAD_PRIORITY_LOWEST:
      prio = 6;
      break;

    case THREAD_PRIORITY_BELOW_NORMAL:
      prio = 7;
      break;

    case THREAD_PRIORITY_NORMAL:
      prio = 8;
      break;

    case THREAD_PRIORITY_ABOVE_NORMAL:
      prio = 9;
      break;

    case THREAD_PRIORITY_HIGHEST:
      prio = 10;
      break;

    case THREAD_PRIORITY_TIME_CRITICAL:
      prio = 15;
      break;

    default:
      return FALSE;
  }

  if (setprio((handle_t) hThread, prio) < 0) return FALSE;
  return TRUE;
}

LPTOP_LEVEL_EXCEPTION_FILTER WINAPI SetUnhandledExceptionFilter(
    LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter) {
  LPTOP_LEVEL_EXCEPTION_FILTER oldfilter = topexcptfilter;

  TRACE("SetUnhandledExceptionFilter");
  
  topexcptfilter = lpTopLevelExceptionFilter;
  return oldfilter;
}

VOID WINAPI Sleep(
    DWORD dwMilliseconds) {
  TRACEX("Sleep");
  msleep(dwMilliseconds);
}

DWORD WINAPI SuspendThread(
    HANDLE hThread) {
  TRACE("SuspendThread");
  return suspend((handle_t) hThread);
}

BOOL WINAPI SystemTimeToFileTime(
    CONST SYSTEMTIME *lpSystemTime,
    LPFILETIME lpFileTime) {
  struct tm tm;
  time_t time;

  TRACE("SystemTimeToFileTime");
  memset(&tm, 0, sizeof(tm));
  tm.tm_year = lpSystemTime->wYear - 1900;
  tm.tm_mon = lpSystemTime->wMonth - 1;
  tm.tm_mday = lpSystemTime->wDay;
  tm.tm_hour = lpSystemTime->wHour;
  tm.tm_min = lpSystemTime->wMinute;
  tm.tm_sec = lpSystemTime->wSecond;

  time = mktime(&tm);
    
  *(unsigned __int64 *) lpFileTime = ((unsigned __int64) time * 1000000 + (unsigned __int64) (lpSystemTime->wMilliseconds) * 1000) * MICROTIMESCALE + EPOC;
  return TRUE;
}

BOOL WINAPI TerminateProcess(
    HANDLE hProcess,
    UINT uExitCode) {
  TRACE("TerminateProcess");
  panic("TerminateProcess not implemented");
  return FALSE;
}

DWORD WINAPI TlsAlloc(VOID) {
  TRACE("TlsAlloc");
  return tlsalloc();
}

BOOL WINAPI TlsFree(
    DWORD dwTlsIndex) {
  TRACE("TlsFree");
  tlsfree(dwTlsIndex);
  return TRUE;
}

LPVOID WINAPI TlsGetValue(
    DWORD dwTlsIndex) {
  TRACEX("TlsGetValue");
  return tlsget(dwTlsIndex);
}

BOOL WINAPI TlsSetValue(
    DWORD dwTlsIndex,
    LPVOID lpTlsValue) {
  TRACEX("TlsSetValue");
  tlsset(dwTlsIndex, lpTlsValue);
  return TRUE;
}

BOOL WINAPI TryEnterCriticalSection(
    LPCRITICAL_SECTION lpCriticalSection) {
  TRACE("TryEnterCriticalSection");
  panic("TryEnterCriticalSection not implemented");
  return TRUE;
}

BOOL WINAPI UnlockFile(
    HANDLE hFile,
    DWORD dwFileOffsetLow,
    DWORD dwFileOffsetHigh,
    DWORD nNumberOfBytesToUnlockLow,
    DWORD nNumberOfBytesToUnlockHigh) {
  panic("UnlockFile not implemented");
  return FALSE;
}

BOOL WINAPI UnlockFileEx(
    HANDLE hFile,
    DWORD dwReserved,
    DWORD nNumberOfBytesToUnlockLow,
    DWORD nNumberOfBytesToUnlockHigh,
    LPOVERLAPPED lpOverlapped) {
  panic("UnlockFileEx not implemented");
  return FALSE;
}

BOOL WINAPI UnmapViewOfFile(
    LPCVOID lpBaseAddress) {
  TRACE("UnmapViewOfFile");
  panic("UnmapViewOfFile not implemented");
  return FALSE;
}

LPVOID WINAPI VirtualAlloc(
    LPVOID lpAddress,
    SIZE_T dwSize,
    DWORD flAllocationType,
    DWORD flProtect) {
  void *addr;
  int n;
  //struct tib *tib = gettib();

  TRACE("VirtualAlloc");

  // Do not allow JVM to mess with the stack (this is a hack!)
  //if (lpAddress >= tib->stackbase && lpAddress < tib->stacktop) return lpAddress;

  addr = vmalloc(lpAddress, dwSize, flAllocationType | MEM_ALIGN64K, flProtect, 'VALO');

  //syslog(LOG_DEBUG, "VirtualAlloc %p %dKB (%p,%p) -> %p", lpAddress, dwSize / K, flAllocationType, flProtect, addr);

  if (addr != NULL && (flAllocationType & MEM_RESERVE) != 0) {
    for (n = 0; n < MAX_VADS; n++) if (vads[n].addr == NULL) break;
    if (n != MAX_VADS) {
      vads[n].addr = addr;
      vads[n].size = dwSize;
    } else {
      panic("no vad for VirtualAlloc");
    }
  }

  return addr;
}

BOOL WINAPI VirtualFree(
    LPVOID lpAddress,
    SIZE_T dwSize,
    DWORD dwFreeType) {
  int n;
  int rc;

  TRACE("VirtualFree");
  if (lpAddress != NULL && (dwFreeType & MEM_RELEASE) != 0 && dwSize == 0) {
    for (n = 0; n < MAX_VADS; n++) if (vads[n].addr == lpAddress) break;
    if (n != MAX_VADS) {
      vads[n].addr = NULL;
      dwSize = vads[n].size;
    } else {
      syslog(LOG_WARNING, "warning: vad not found for VitualFree");
    }
  }

  rc = vmfree(lpAddress, dwSize, dwFreeType);

  //syslog(LOG_DEBUG, "VirtualFree %p %d bytes (%p) -> %d", lpAddress, dwSize, dwFreeType, rc);

  return rc == 0;
}

BOOL WINAPI VirtualProtect(
    LPVOID lpAddress,
    SIZE_T dwSize,
    DWORD flNewProtect,
    LPDWORD lpflOldProtect) {
  int rc;
  //struct tib *tib = gettib();

  TRACE("VirtualProtect");

  // Do not allow JVM to mess with the stack (this is a hack!)
  //if (lpAddress >= tib->stackbase && lpAddress < tib->stacktop) return TRUE;

  rc = vmprotect(lpAddress, dwSize, flNewProtect);
  //syslog(LOG_DEBUG, "VirtualProtect %p %dKB %d %d", lpAddress, dwSize / K, flNewProtect, rc);
  if (rc < 0) return FALSE;

  if (lpflOldProtect) *lpflOldProtect = rc;
  return TRUE;
}

DWORD WINAPI VirtualQuery(
    LPCVOID lpAddress,
    PMEMORY_BASIC_INFORMATION lpBuffer,
    SIZE_T dwLength) {
  struct tib *tib = gettib();

  TRACE("VirtualQuery");

  // Used in win32\hpi\src\threads_md.c to check for stack guard pages
  // In JVM 1.4 also used for checking stack size in hotspot\src\os\win32\vm\os_win32.cpp
  if (lpAddress >= tib->stacklimit && lpAddress < tib->stacktop) {
    // Handle stack case
    //syslog(LOG_DEBUG, "VirtualQuery %p (in stack %p %p %p)", lpAddress, tib->stackbase, tib->stacklimit, tib->stacktop);
    lpBuffer->BaseAddress = tib->stacklimit;
    lpBuffer->RegionSize = (char *) tib->stacktop - (char *) tib->stacklimit;
    lpBuffer->AllocationBase = tib->stackbase;
    lpBuffer->Protect = PAGE_READWRITE;
  } else {
    //syslog(LOG_DEBUG, "VirtualQuery %p (not in stack!!!!)", lpAddress);
  
    // Return dummy result
    lpBuffer->BaseAddress = (void *) ((unsigned long) lpAddress & ~(PAGESIZE - 1));
    lpBuffer->Protect = PAGE_READWRITE;
  }

  return dwLength;
}

DWORD WINAPI WaitForMultipleObjects(
    DWORD nCount,
    CONST HANDLE *lpHandles,
    BOOL bWaitAll,
    DWORD dwMilliseconds) {
  int rc;

  TRACE("WaitForMultipleObjects");

  if (bWaitAll) {
    rc = waitall((handle_t *) lpHandles, nCount, dwMilliseconds);
  } else {
    rc = waitany((handle_t *) lpHandles, nCount, dwMilliseconds);
  }

  if (rc < 0) {
    if (errno == ETIMEOUT) {
      return WAIT_TIMEOUT;
    } else {
      return WAIT_FAILED;
    }
  }

  return rc;
}

DWORD WINAPI WaitForSingleObject(
    HANDLE hHandle,
    DWORD dwMilliseconds) {
  int rc;

  TRACEX("WaitForSingleObject");
  rc = waitone((handle_t) hHandle, dwMilliseconds);

  if (rc < 0) {
    if (errno == ETIMEOUT) {
      return WAIT_TIMEOUT;
    } else {
      return WAIT_FAILED;
    }
  }

  return rc;
}

int WINAPI WideCharToMultiByte(
    UINT CodePage,
    DWORD dwFlags,
    LPCWSTR lpWideCharStr,
    int cchWideChar,
    LPSTR lpMultiByteStr,
    int cbMultiByte,
    LPCSTR lpDefaultChar,
    LPBOOL lpUsedDefaultChar) {
  TRACE("WideCharToMultiByte");
  panic("WideCharToMultiByte not implemented");
  return 0;
}

BOOL WINAPI WriteFile(
    HANDLE hFile,
    LPCVOID lpBuffer,
    DWORD nNumberOfBytesToWrite,
    LPDWORD lpNumberOfBytesWritten,
    LPOVERLAPPED lpOverlapped) {
  int rc;

  TRACE("WriteFile");
  if (lpOverlapped) panic("Overlapped I/O not implemented in WriteFile");

  rc = write((handle_t) hFile, lpBuffer, nNumberOfBytesToWrite);
  if (rc < 0) return FALSE;

  *lpNumberOfBytesWritten = rc;
  return TRUE;
}

int __stdcall DllMain(handle_t hmod, int reason, void *reserved) {
  if (reason == DLL_PROCESS_ATTACH) {
    // Register global WIN32 handler for all signals
    struct peb *peb = gettib()->peb;
    old_globalhandler = peb->globalhandler;
    peb->globalhandler = win32_globalhandler;
  }

  return TRUE;
}
