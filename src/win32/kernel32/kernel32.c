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

struct winfinddata
{
  handle_t fhandle;
  char dir[MAXPATH];
  char *mask;
};

struct vad
{
  void *addr;
  unsigned long size;
};

struct vad vads[MAX_VADS];
PTOP_LEVEL_EXCEPTION_FILTER topexcptfilter = NULL;
void (*old_globalhandler)(int signum, struct siginfo *info);

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

  if (ctx->ContextFlags & CONTEXT_DEBUG_REGISTERS)
  {
    // debug registers missing
  }
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

static __inline EXCEPTION_FRAME *push_frame(EXCEPTION_FRAME *frame)
{
  struct tib *tib = gettib();
  frame->prev = (EXCEPTION_FRAME *) tib->except;
  tib->except = (struct xcptrec *) frame;
  return frame->prev;
}

static __inline EXCEPTION_FRAME *pop_frame(EXCEPTION_FRAME *frame)
{
  struct tib *tib = gettib();
  tib->except = (struct xcptrec *) frame->prev;
  return frame->prev;
}

//
// Handler for exceptions happening inside a handler
//

static EXCEPTION_DISPOSITION raise_handler(EXCEPTION_RECORD *rec, EXCEPTION_FRAME *frame, CONTEXT *ctxt, EXCEPTION_FRAME **dispatcher)
{
  if (rec->ExceptionFlags & (EH_UNWINDING | EH_EXIT_UNWIND)) return ExceptionContinueSearch;

  // We shouldn't get here so we store faulty frame in dispatcher
  *dispatcher = ((NESTED_FRAME *) frame)->prev;
  return ExceptionNestedException;
}

//
// Handler for exceptions happening inside an unwind handler.
//

static EXCEPTION_DISPOSITION unwind_handler(EXCEPTION_RECORD *rec, EXCEPTION_FRAME *frame, CONTEXT *ctxt, EXCEPTION_FRAME **dispatcher)
{
  if (!(rec->ExceptionFlags & (EH_UNWINDING | EH_EXIT_UNWIND))) return ExceptionContinueSearch;

  // We shouldn't get here so we store faulty frame in dispatcher
  *dispatcher = ((NESTED_FRAME *) frame)->prev;
  return ExceptionCollidedUnwind;
}

//
// Call an exception handler, setting up an exception frame to catch exceptions
// happening during the handler execution.
//

static EXCEPTION_DISPOSITION call_handler
(
  EXCEPTION_RECORD *record, 
  EXCEPTION_FRAME *frame,
  CONTEXT *ctxt, 
  EXCEPTION_FRAME **dispatcher,
  PEXCEPTION_HANDLER handler, 
  PEXCEPTION_HANDLER nested_handler
)
{
  NESTED_FRAME newframe;
  EXCEPTION_DISPOSITION rc;

  newframe.frame.handler = nested_handler;
  newframe.prev = frame;
  push_frame(&newframe.frame);
  syslog(LOG_DEBUG, "calling handler at %p code=%x flags=%x\n", handler, record->ExceptionCode, record->ExceptionFlags);
  rc = handler(record, frame, ctxt, dispatcher);
  syslog(LOG_DEBUG, "handler returned %x\n", rc);
  pop_frame(&newframe.frame);

  return rc;
}

void raise_exception(EXCEPTION_RECORD *rec, CONTEXT *ctxt)
{
  PEXCEPTION_FRAME frame, dispatch, nested_frame;
  EXCEPTION_RECORD newrec;
  EXCEPTION_DISPOSITION rc;
  struct tib *tib = gettib();

  syslog(LOG_DEBUG, "raise_exception: code=%lx flags=%lx addr=%p\n", rec->ExceptionCode, rec->ExceptionFlags, rec->ExceptionAddress);

  frame = (EXCEPTION_FRAME *) tib->except;
  nested_frame = NULL;
  while (frame != (PEXCEPTION_FRAME) 0xFFFFFFFF)
  {
    syslog(LOG_DEBUG, "frame: %p handler: %p\n", frame, frame->handler);
    
    // Check frame address
    if ((void *) frame < tib->stacklimit || (void *)(frame + 1) > tib->stacktop || (int) frame & 3)
    {
      rec->ExceptionFlags |= EH_STACK_INVALID;
      break;
    }

    // Call handler
    rc = call_handler(rec, frame, ctxt, &dispatch, frame->handler, raise_handler);
    if (frame == nested_frame)
    {
      // No longer nested
      nested_frame = NULL;
      rec->ExceptionFlags &= ~EH_NESTED_CALL;
    }

    switch (rc)
    {
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

void unwind(PEXCEPTION_FRAME endframe, LPVOID eip, PEXCEPTION_RECORD rec, DWORD retval, CONTEXT *ctxt)
{
  EXCEPTION_RECORD record, newrec;
  PEXCEPTION_FRAME frame, dispatch;
  EXCEPTION_DISPOSITION rc;
  struct tib *tib = gettib();

  ctxt->Eax = retval;

  // Build an exception record, if we do not have one
  if (!rec)
  {
    record.ExceptionCode    = STATUS_UNWIND;
    record.ExceptionFlags   = 0;
    record.ExceptionRecord  = NULL;
    record.ExceptionAddress = (void *) ctxt->Eip;
    record.NumberParameters = 0;
    rec = &record;
  }

  rec->ExceptionFlags |= EH_UNWINDING | (endframe ? 0 : EH_EXIT_UNWIND);

  syslog(LOG_DEBUG, "code=%lx flags=%lx\n", rec->ExceptionCode, rec->ExceptionFlags);

  // Get chain of exception frames
  frame = (EXCEPTION_FRAME *) tib->except;
  while (frame != (PEXCEPTION_FRAME) 0xffffffff && frame != endframe)
  {
    // Check frame address
    if (endframe && (frame > endframe))
    {
      newrec.ExceptionCode = STATUS_INVALID_UNWIND_TARGET;
      newrec.ExceptionFlags = EH_NONCONTINUABLE;
      newrec.ExceptionRecord = rec;
      newrec.NumberParameters = 0;
      panic("kernel32: invalid unwind target");
      //TODO: RtlRaiseException(&newrec);  // Never returns
    }

    if ((void *) frame < tib->stacklimit || (void *)(frame + 1) > tib->stacktop || (int) frame & 3)
    {
      newrec.ExceptionCode = STATUS_BAD_STACK;
      newrec.ExceptionFlags = EH_NONCONTINUABLE;
      newrec.ExceptionRecord = rec;
      newrec.NumberParameters = 0;
      panic("kernel32: bad stack in unwind");
      //TODO: RtlRaiseException(&newrec);  // Never returns
    }

    // Call handler
    rc = call_handler(rec, frame, ctxt, &dispatch, frame->handler, unwind_handler);

    switch (rc)
    {
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

void win32_globalhandler(int signum, struct siginfo *info)
{
  CONTEXT ctxt;
  EXCEPTION_RECORD rec;
  EXCEPTION_POINTERS ep;

  syslog(LOG_DEBUG, "kernel32: caught signal %d\n", signum);

  ep.ContextRecord = &ctxt;
  ep.ExceptionRecord = &rec;

  memset(&rec, 0, sizeof(EXCEPTION_RECORD));
  rec.ExceptionAddress = (void *) info->ctxt.eip;

  convert_to_win32_context(&info->ctxt, &ctxt);
  
  switch (info->ctxt.traptype)
  {
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
      rec.ExceptionCode = EXCEPTION_ACCESS_VIOLATION;
      rec.NumberParameters = 2;
      rec.ExceptionInformation[0] = (void *) ((info->ctxt.errcode & 2) ? 1 : 0);
      rec.ExceptionInformation[1] = info->addr;
      break;

    case INTR_ALIGN:
      rec.ExceptionCode = EXCEPTION_DATATYPE_MISALIGNMENT;
      break;

    default:
      // Unknown exception, dispatch signal to native handler
      old_globalhandler(signum, info);
  }

  raise_exception(&rec, &ctxt);

  convert_from_win32_context(&info->ctxt, &ctxt);

  syslog(LOG_DEBUG, "kernel32: sigexit\n", signum);
  sigexit(info, 0);
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

LONG WINAPI CompareFileTime(
  CONST FILETIME *lpFileTime1,
  CONST FILETIME *lpFileTime2
)
{
  TRACE("CompareFileTime");

  if (lpFileTime1->dwHighDateTime > lpFileTime2->dwHighDateTime)
    return 1;
  else if (lpFileTime1->dwHighDateTime < lpFileTime2->dwHighDateTime)
    return -1;
  else if (lpFileTime1->dwLowDateTime > lpFileTime2->dwLowDateTime)
    return 1;
  else if (lpFileTime1->dwLowDateTime < lpFileTime2->dwLowDateTime)
    return -1;
  else
    return 0;
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
  int flags = 0;
  int mode = 0;
  int shflags = 0;
  handle_t h;

  TRACE("CreateFile");

  if ((dwDesiredAccess & (GENERIC_READ | GENERIC_WRITE)) == (GENERIC_READ | GENERIC_WRITE))
    flags = O_RDWR;
  else if (dwDesiredAccess & GENERIC_READ)
    flags = O_RDONLY;
  else if (dwDesiredAccess & GENERIC_WRITE)
    flags = O_WRONLY;
  else if (dwDesiredAccess & GENERIC_ALL)
    flags = O_RDWR;
  else
    flags = O_RDONLY;

  if ((dwShareMode & (FILE_SHARE_READ | FILE_SHARE_WRITE)) == (FILE_SHARE_READ | FILE_SHARE_WRITE))
    shflags = SH_DENYNO;
  else if (dwShareMode & FILE_SHARE_READ)
    shflags = SH_DENYWR;
  else if (dwShareMode & FILE_SHARE_WRITE)
    shflags = SH_DENYRD;
  else
    shflags = SH_DENYRW;

  switch (dwCreationDisposition)
  {
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

  if (dwFlagsAndAttributes & FILE_ATTRIBUTE_READONLY)
    mode = S_IREAD;
  else
    mode = S_IREAD | S_IWRITE | S_IEXEC;

  if (dwFlagsAndAttributes & FILE_FLAG_OVERLAPPED)
  {
    syslog(LOG_WARNING, "CreateFile(%s): overlapped operation not supported\n", lpFileName);
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
  return (HANDLE) mksem(lInitialCount);
}

VOID WINAPI DebugBreak(VOID)
{
  TRACE("DebugBreak");
  dbgbreak();
}

VOID WINAPI DeleteCriticalSection
(
  LPCRITICAL_SECTION lpCriticalSection
)
{
  TRACE("DeleteCriticalSection");
  csfree(lpCriticalSection);
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

BOOL WINAPI FileTimeToLocalFileTime
(
  CONST FILETIME *lpFileTime,
  LPFILETIME lpLocalFileTime
)
{
  TRACEX("FileTimeToLocalFileTime");
  *lpLocalFileTime = *lpFileTime;
  return TRUE;
}

BOOL WINAPI FileTimeToSystemTime
(
  CONST FILETIME *lpFileTime,
  LPSYSTEMTIME lpSystemTime
)
{
  time_t t;
  struct tm tm;

  TRACEX("FileTimeToSystemTime");

  t = (time_t) ((*(unsigned __int64 *) lpFileTime) - EPOC) / SECTIMESCALE;

  _gmtime(&t, &tm);

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

static void fill_find_data(LPWIN32_FIND_DATA fd, char *filename, struct stat64 *statbuf)
{
  FILETIME *ft;

  memset(fd, 0, sizeof(WIN32_FIND_DATA));
  strcpy(fd->cFileName, filename);

  if ((statbuf->st_mode & S_IFMT) == S_IFDIR) 
    fd->dwFileAttributes |= FILE_ATTRIBUTE_DIRECTORY;
  else
    fd->dwFileAttributes |= FILE_ATTRIBUTE_NORMAL;

  ft = (FILETIME *) &statbuf->st_size;
  fd->nFileSizeLow = ft->dwLowDateTime;
  fd->nFileSizeHigh = ft->dwHighDateTime;

  *(unsigned __int64 *) &(fd->ftCreationTime) = (unsigned __int64) statbuf->st_ctime * SECTIMESCALE + EPOC;
  *(unsigned __int64 *) &(fd->ftLastAccessTime) = (unsigned __int64) statbuf->st_mtime * SECTIMESCALE + EPOC;
  *(unsigned __int64 *) &(fd->ftLastWriteTime) = (unsigned __int64) statbuf->st_mtime * SECTIMESCALE + EPOC;
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
  struct stat64 statbuf;
  char *p;
  char *base;
  char fn[MAXPATH];

  TRACE("FindFirstFileA");
  //syslog(LOG_DEBUG | LOG_AUX, "FindFirstFile %s\n", lpFileName);

  p = (char *) lpFileName;
  while (*p != 0 && *p != '*' && *p != '?') p++;
  
  if (*p)
  {
    finddata = (struct winfinddata *) malloc(sizeof(struct winfinddata));
    if (!finddata) 
    {
      errno = ENOMEM;
      return INVALID_HANDLE_VALUE;
    }

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
      errno = ENOTDIR;
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
      errno = ENOTDIR;
      free(finddata);
      return INVALID_HANDLE_VALUE;
    }

    while (readdir(finddata->fhandle, &dirent, 1) > 0)
    {
      //syslog(LOG_DEBUG | LOG_AUX, "match %s with %s\n", dirent.name, finddata->mask);
      if (like(dirent.name, finddata->mask))
      {
	strcpy(fn, finddata->dir);
	strcpy(fn + strlen(fn), "\\");
	strcpy(fn + strlen(fn), dirent.name);

        if (stat64(fn, &statbuf) < 0) 
	{
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
    if (stat64(finddata->dir, &statbuf) < 0) 
    {
      errno = ENOENT;
      return INVALID_HANDLE_VALUE;
    }

    fill_find_data(lpFindFileData, ".", &statbuf);
    return NOFINDHANDLE;
  }
  else
  {
    if (stat64((char *) lpFileName, &statbuf) < 0) 
    {
      errno = ENOENT;
      return INVALID_HANDLE_VALUE;
    }

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
  struct stat64 statbuf;
  char fn[MAXPATH];

  TRACE("FindNextFileA");

  if (hFindFile == NOFINDHANDLE)
  {
    errno = ESRCH;
    return FALSE;
  }
  else
  {
    finddata = (struct winfinddata *) hFindFile;

    while (readdir(finddata->fhandle, &dirent, 1) > 0)
    {
      //syslog(LOG_DEBUG | LOG_AUX, "match next %s with %s\n", dirent.name, finddata->mask);
      if (like(dirent.name, finddata->mask))
      {
	strcpy(fn, finddata->dir);
	strcpy(fn + strlen(fn), "\\");
	strcpy(fn + strlen(fn), dirent.name);

        if (stat64(fn, &statbuf) < 0) 
	{
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
  char *buf;

  TRACE("FormatMessageA");

  // TODO: generate more descriptive message
  if (dwFlags & FORMAT_MESSAGE_ALLOCATE_BUFFER)
  {
    buf = malloc(nSize > 256 ? nSize : 256);
    if (!buf) 
    {
      errno = ENOMEM;
      return 0;
    }

    *(char **) lpBuffer = buf;
  }
  else
  {
    buf = lpBuffer;
  }

  sprintf(buf, "Error 0x%08X.\n", dwMessageId);
  return strlen(buf);
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
  
  if (nBufferLength < strlen(peb->curdir) + 3) return strlen(peb->curdir) + 3;
  lpBuffer[0] = 'C';
  lpBuffer[1] = ':';
  strcpy(lpBuffer + 2, peb->curdir);
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
  //syslog(LOG_DEBUG, "GetEnvironmentVariable(%s)\n", lpName);
  
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

BOOL WINAPI GetExitCodeThread
(
  HANDLE hThread,
  LPDWORD lpExitCode
)
{
  TRACE("GetExitCodeThread");
  panic("GetExitCodeThread not implemented");
  return FALSE;
}

DWORD WINAPI GetFileAttributesA
(
  LPCTSTR lpFileName
)
{
  struct stat64 fs;

  TRACE("GetFileAttributesA");
  if (stat64((char *) lpFileName, &fs) < 0) return -1;

  // TODO: set other attributes for files (hidden, system, readonly etc.)
  if ((fs.st_mode & S_IFMT) == S_IFDIR) 
    return 0x00000010;
  else
    return 0x00000080;
}

BOOL WINAPI GetFileTime
(
  HANDLE hFile,
  LPFILETIME lpCreationTime,
  LPFILETIME lpLastAccessTime,
  LPFILETIME lpLastWriteTime
)
{
  struct utimbuf times;

  TRACE("GetFileTime");

  if (futime((handle_t) hFile, &times) < 0) return FALSE;

  if (lpCreationTime)
  {
    *(unsigned __int64 *) lpCreationTime = (unsigned __int64) times.ctime * SECTIMESCALE + EPOC;
  }

  if (lpLastAccessTime)
  {
    *(unsigned __int64 *) lpLastAccessTime = (unsigned __int64) times.atime * SECTIMESCALE + EPOC;
  }

  if (lpLastWriteTime)
  {
    *(unsigned __int64 *) lpLastWriteTime = (unsigned __int64) times.mtime * SECTIMESCALE + EPOC;
  }

  return TRUE;
}

DWORD WINAPI GetFullPathNameA
(
  LPCTSTR lpFileName,
  DWORD nBufferLength,
  LPTSTR lpBuffer,
  LPTSTR *lpFilePart
)
{
  char fn[MAXPATH];
  int rc;
  char *basename;
  char *p;

  TRACE("GetFullPathName");

  rc = canonicalize(lpFileName, fn, MAXPATH);
  if (rc < 0)
  {
    errno = -rc;
    return 0;
  }

  if (strlen(fn) + 2 >= nBufferLength) return strlen(fn) + 3;

  strcpy(lpBuffer, "C:");
  strcat(lpBuffer, fn);

  p = basename = lpBuffer;
  while (*p)
  {
    if (*p == PS1 || *p == PS2) basename = p + 1;
    p++;
  }
  *lpFilePart = basename;

  return strlen(lpBuffer);
}

DWORD WINAPI GetLastError(VOID)
{
  TRACE("GetLastError");
  // TODO: implement better error reporting
  if (errno == 0) return 0;
  if (errno == ENOENT) return ERROR_FILE_NOT_FOUND;
  if (errno == ESRCH) return ERROR_NO_MORE_FILES;
  return 0x80040000 | errno;
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
  //syslog(LOG_DEBUG, "Module filename for %p is %s\n", hModule, lpFilename);
  return rc;
}

HMODULE WINAPI GetModuleHandleA
(
  LPCTSTR lpModuleName
)
{
  hmodule_t hmod;

  TRACE("GetModuleHandleA");
  hmod = getmodule(lpModuleName);
  //syslog(LOG_DEBUG, "GetModuleHandleA(%s) returned %p\n", lpModuleName, hmod);
  return (HMODULE) hmod;
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
  //syslog(LOG_DEBUG, "GetProcAddress name: %s hmod: %08X\n", lpProcName, hModule);
  return resolve((hmodule_t) hModule, (char *) lpProcName);
}

HANDLE WINAPI GetProcessHeap()
{
  TRACE("GetProcessHeap");
  return PROCESSHEAP;
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
  strcpy(lpBuffer, get_property(config, "win32", "sysdir", "c:\\bin"));
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
  _gmtime(&tv.tv_sec, &tm);

  lpSystemTime->wYear = tm.tm_year + 1900; 
  lpSystemTime->wMonth = tm.tm_mon + 1; 
  lpSystemTime->wDayOfWeek = tm.tm_wday;
  lpSystemTime->wDay = tm.tm_mday; 
  lpSystemTime->wHour = tm.tm_hour; 
  lpSystemTime->wMinute = tm.tm_min; 
  lpSystemTime->wSecond = tm.tm_sec; 
  lpSystemTime->wMilliseconds = (WORD) (tv.tv_usec / 1000);
}

VOID WINAPI GetSystemTimeAsFileTime
(
  LPFILETIME lpSystemTimeAsFileTime
)
{
  struct timeval tv;

  TRACE("GetSystemTimeAsFileTime");
  gettimeofday(&tv);
  *(unsigned __int64 *) lpSystemTimeAsFileTime = ((unsigned __int64) (tv.tv_sec) * 1000000 + (unsigned __int64) (tv.tv_usec)) * MICROTIMESCALE + EPOC;
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
  int rc;

  TRACE("GetThreadContext");
  rc = getcontext((handle_t) hThread, &ctxt);
  if (rc < 0) 
  {
    if (rc == -EPERM)
    {
      // Thead does not have a usermode context, just return dummy context
      memset(lpContext, 0, sizeof(CONTEXT));
      return TRUE;
    }

    syslog(LOG_DEBUG, "GetThreadContext(%d) failed\n", hThread);
    return FALSE;
  }

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


  // Return priority based on win32 normal priority class scheme
  switch (prio)
  {
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
  strcpy(lpBuffer, get_property(config, "win32", "windir", "c:\\bin"));
  return strlen(lpBuffer);
}

LPVOID WINAPI HeapAlloc
(
  HANDLE hHeap,
  DWORD dwFlags,
  SIZE_T dwBytes
)
{
  TRACE("HeapAlloc");
  if (hHeap != PROCESSHEAP)
  {
    syslog(LOG_DEBUG, "warning: HeapAlloc only supported for process heap\n");
    return NULL;
  }

  return malloc(dwBytes);
}

BOOL WINAPI HeapFree
(
  HANDLE hHeap,
  DWORD dwFlags,
  LPVOID lpMem
)
{
  TRACE("HeapFree");
  if (hHeap != PROCESSHEAP)
  {
    syslog(LOG_DEBUG, "warning: HeapFree only supported for process heap\n");
    return FALSE;
  }

  free(lpMem);
  return TRUE;
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

__declspec(naked) LONG WINAPI InterlockedExchange
(
  LPLONG volatile Target,
  LONG Value
)
{
  __asm
  {
    mov ecx,dword ptr [esp+4]
    mov edx,dword ptr [esp+8]
    mov eax,dword ptr [ecx]
ileagain:
    lock cmpxchg dword ptr [ecx],edx
    jne ileagain
    ret 8
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

BOOL WINAPI MoveFileA
(
  LPCTSTR lpExistingFileName,
  LPCTSTR lpNewFileName
)
{
  TRACE("MoveFileA");
  if (rename(lpExistingFileName, lpNewFileName) < 0) return FALSE;

  return TRUE;
}

int WINAPI MultiByteToWideChar
(
  UINT CodePage,         // code page
  DWORD dwFlags,         // character-type options
  LPCSTR lpMultiByteStr, // string to map
  int cbMultiByte,       // number of bytes in string
  LPWSTR lpWideCharStr,  // wide-character buffer
  int cchWideChar        // size of buffer
)
{
  TRACE("MultiByteToWideChar");
  panic("MultiByteToWideChar not implemented");
  return 0;
}

HANDLE WINAPI OpenProcess
(
  DWORD dwDesiredAccess,
  BOOL bInheritHandle,
  DWORD dwProcessId
)
{
  TRACE("OpenProcess");
  panic("OpenProcess not implemented");
  return NULL;
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

VOID WINAPI RtlUnwind
(
  PEXCEPTION_FRAME endframe, 
  LPVOID eip, 
  PEXCEPTION_RECORD rec, 
  DWORD retval
)
{
  CONTEXT ctxt;

  TRACE("RtlUnwind");
  memset(&ctxt, 0, sizeof(CONTEXT));
  unwind(endframe, eip, rec, retval, &ctxt);
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

BOOL WINAPI SetConsoleTitleA
(
  LPCTSTR lpConsoleTitle
)
{
  TRACE("SetConsoleTitle");
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
  if (dwFileAttributes != 0) syslog(LOG_DEBUG, "warning: SetFileAttributesA(%s,%08x) not implemented, ignored\n", lpFileName, dwFileAttributes);
  return TRUE;
}

DWORD WINAPI SetFilePointer
(
  HANDLE hFile,
  LONG lDistanceToMove,
  PLONG lpDistanceToMoveHigh,
  DWORD dwMoveMethod
)
{
  int rc;
  TRACE("SetFilePointer");
  
  if (lpDistanceToMoveHigh)
  {
    LARGE_INTEGER offset;
    LARGE_INTEGER retval;

    offset.LowPart = lDistanceToMove;
    offset.HighPart = *lpDistanceToMoveHigh;

    retval.QuadPart = lseek64((handle_t) hFile, offset.QuadPart, dwMoveMethod);
    if (retval.QuadPart < 0)
    {
      errno = (int) -retval.QuadPart;
      rc = -1;
    }
    else
    {
      errno = 0;
      *lpDistanceToMoveHigh = retval.HighPart;
      rc = retval.LowPart;
    }
  }
  else
  {
    rc = lseek((handle_t) hFile, lDistanceToMove, dwMoveMethod);
    if (rc < 0) rc = -1;
  }

  return rc;
}

BOOL WINAPI SetFileTime
(
  HANDLE hFile,
  CONST FILETIME *lpCreationTime,
  CONST FILETIME *lpLastAccessTime,
  CONST FILETIME *lpLastWriteTime
)
{
  struct utimbuf times;
  int rc;

  TRACE("SetFileTime");

  if (lpCreationTime != NULL)
    times.ctime = (time_t)(((*(__int64 *) lpCreationTime) - EPOC) / SECTIMESCALE);
  else
    times.ctime = -1;

  if (lpLastAccessTime != NULL)
    times.atime = (time_t)(((*(__int64 *) lpLastAccessTime) - EPOC) / SECTIMESCALE);
  else
    times.atime = -1;

  if (lpLastWriteTime != NULL)
    times.mtime = (time_t)(((*(__int64 *) lpLastWriteTime) - EPOC) / SECTIMESCALE);
  else
    times.mtime = -1;

  rc = futime((handle_t) hFile, &times);
  if (rc < 0) return FALSE;

  return TRUE;
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

  if (lpContext->ContextFlags != (CONTEXT_CONTROL | CONTEXT_INTEGER | CONTEXT_SEGMENTS))
  {
    syslog(LOG_WARNING, "kernel32: incomplete context in SetContext, flags %lx\n", lpContext->ContextFlags); 
  }

  memset(&ctxt, 0, sizeof(struct context));
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

  // Set new thread priority based on win32 normal priority class scheme
  switch (nPriority)
  {
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

LPTOP_LEVEL_EXCEPTION_FILTER WINAPI SetUnhandledExceptionFilter
(
  LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter 
)
{
  LPTOP_LEVEL_EXCEPTION_FILTER oldfilter = topexcptfilter;

  TRACE("SetUnhandledExceptionFilter");
  
  topexcptfilter = lpTopLevelExceptionFilter;
  return oldfilter;
}

VOID WINAPI Sleep
(
  DWORD dwMilliseconds
)
{
  TRACEX("Sleep");
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

BOOL WINAPI TlsFree
(
  DWORD dwTlsIndex
)
{
  TRACE("TlsFree");
  tlsfree(dwTlsIndex);
  return TRUE;
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

BOOL WINAPI TryEnterCriticalSection
(
  LPCRITICAL_SECTION lpCriticalSection
)
{
  TRACE("TryEnterCriticalSection");
  panic("TryEnterCriticalSection not implemented");
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
  void *addr;
  int n;

  TRACE("VirtualAlloc");
  addr = mmap(lpAddress, dwSize, flAllocationType | MEM_ALIGN64K, flProtect, 'VALO');
  
  //syslog(LOG_DEBUG, "VirtualAlloc %p %dKB (%p,%p) -> %p\n", lpAddress, dwSize / K, flAllocationType, flProtect, addr);

  if (addr != NULL && (flAllocationType & MEM_RESERVE) != 0)
  {
    for (n = 0; n < MAX_VADS; n++) if (vads[n].addr == NULL) break;
    if (n != MAX_VADS)
    {
      vads[n].addr = addr;
      vads[n].size = dwSize;
    }
    else
      panic("no vad for VirtualAlloc");
  }

  return addr;
}

BOOL WINAPI VirtualFree
(
  LPVOID lpAddress,
  SIZE_T dwSize,
  DWORD dwFreeType
)
{
  int n;
  int rc;

  TRACE("VirtualFree");
  if (lpAddress != NULL && (dwFreeType & MEM_RELEASE) != 0 && dwSize == 0)
  {
    for (n = 0; n < MAX_VADS; n++) if (vads[n].addr == lpAddress) break;
    if (n != MAX_VADS)
    {
      vads[n].addr = NULL;
      dwSize = vads[n].size;
    }
    else
      syslog(LOG_WARNING, "warning: vad not found for VitualFree\n");
  }

  rc = munmap(lpAddress, dwSize, dwFreeType);

  //syslog(LOG_DEBUG, "VirtualFree %p %d bytes (%p) -> %d\n", lpAddress, dwSize, dwFreeType, rc);

  return rc == 0;
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
  int rc;

  TRACE("WaitForMultipleObjects");

  if (bWaitAll)
    rc = waitall((handle_t *) lpHandles, nCount, dwMilliseconds);
  else
    rc = waitany((handle_t *) lpHandles, nCount, dwMilliseconds);

  if (rc == -ETIMEOUT) return WAIT_TIMEOUT;
  if (rc < 0) return WAIT_FAILED;

  return rc;
}

DWORD WINAPI WaitForSingleObject
(
  HANDLE hHandle,
  DWORD dwMilliseconds
)
{
  int rc;

  TRACE("WaitForSingleObject");
  rc = wait((handle_t) hHandle, dwMilliseconds);

  if (rc == -ETIMEOUT) return WAIT_TIMEOUT;
  if (rc < 0) return WAIT_FAILED;

  return rc;
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
  if (reason == DLL_PROCESS_ATTACH)
  {
    // Register global WIN32 handler for all signals
    old_globalhandler = peb->globalhandler;
    peb->globalhandler = win32_globalhandler;
  }

  return TRUE;
}
