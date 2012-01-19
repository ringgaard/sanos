//
// dbggw.c
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Remote debugging gateway
//

#include <windows.h>
#include <stdio.h>
#include <process.h>

typedef unsigned long tid_t;
typedef void *hmodule_t;

#include <seg.h>
#include <intr.h>
#include <dbg.h>

#define DRPC_SIGNATURE  0x43505244
#define DRPC_STARTMARK  0x00286308
#define DRPC_REPLY      0x00010000

#define DRPC_DEBUG_BREAK          0x0002C028
#define DRPC_GET_SYSTEM_VERSION   0x0004C004
#define DRPC_GET_HOST_INFO        0x0004C005
#define DRPC_GET_CPU_INFO         0x0004C006
#define DRPC_GET_PROCESS_LIST     0x0004C008
#define DRPC_GET_PROCESS_NAME     0x0004C00A
#define DRPC_DEBUG_PROCESS        0x0004C00B
#define DRPC_OPEN_PROCESS         0x0004C00C
#define DRPC_CREATE_PROCESS       0x0004C00E
#define DRPC_READ_PROCESS_MEMORY  0x0004C017
#define DRPC_WRITE_PROCESS_MEMORY 0x0004C018
#define DRPC_SUSPEND_THREAD       0x0004C01E
#define DRPC_RESUME_THREAD        0x0004C01F
#define DRPC_GET_THREAD_CONTEXT   0x0004C020
#define DRPC_SET_THREAD_CONTEXT   0x0004C021
#define DRPC_GET_PEB_ADDRESS      0x0004C022
#define DRPC_GET_THREAD_SELECTOR  0x0004C024
#define DRPC_GET_DEBUG_EVENT      0x0004C029
#define DRPC_CONTINUE_DEBUG_EVENT 0x0004C02A

#define DEBUGGER_PORT 24502
#define DEBUGGEE_PORT "COM1"
#define MAX_DBG_CHUNKSIZE 4096

#define MAX_DBG_PACKETLEN (MAX_DBG_CHUNKSIZE + sizeof(union dbg_body))

HANDLE debugee;
SOCKET listener;
unsigned char next_reqid = 1;

struct dbg_hdr hdr;
char dbg_data[MAX_DBG_PACKETLEN];
union dbg_body *body;

struct dbg_event *event_head = NULL;
struct dbg_event *event_tail = NULL;

FILE *logfile;

static char *dbgevtnames[] = 
{
  "0", 
  "Exception", 
  "CreateThread", 
  "CreateProcess", 
  "ExitThread", 
  "ExitProcess", 
  "LoadModule", 
  "UnloadModule",
  "OutputDebugString",
  "RIP"
};

struct session
{
  SOCKET debugger;
  HANDLE debuggee;
  unsigned long startmark;
};

struct drpc_packet
{
  unsigned long startmark;
  unsigned long reserved1;    // Always zero
  unsigned long cmd;          // Command (or with 0x00010000 in reply)
  unsigned long reqlen;       // Request data length
  unsigned long rsplen;       // Response data length
  unsigned long result;       // Result (success=0)
  unsigned long reqid;        // Request id
  unsigned long reserved2;    // Always zero
};

struct drpc_processor_identification
{
  unsigned long family;
  unsigned long model;
  unsigned long stepping;
  unsigned long vendor[16];
};

struct drpc_system_version
{
  unsigned long unknown1;    // 0x14c
  unsigned long processors;  // 1
  unsigned long platformid;  // 2
  unsigned long build;       // 0xA28
  unsigned long spnum;       // 0
  char spname[0x104];
  char buildname[0x110];
};

void panic(char *msg)
{
  printf("panic: %s\n", msg);
  exit(1);
}

void logmsg(const char *fmt, ...)
{
  va_list args;

  va_start(args, fmt);
  if (logfile) vfprintf(logfile, fmt, args);
  vprintf(fmt, args);
  va_end(args);
}

void dump_data(FILE *f, unsigned char *buf, int bytes, int wordsize, unsigned char *cmpbuf)
{
  int i;
  int start;
  int addr;

  start = 0;
  while (start < bytes)
  {
    if (start > 0) fprintf(f, "         ");

    for (i = 0; i < 32 / wordsize; i++) 
    {
      addr = start + i * wordsize;

      switch (wordsize)
      {
        case 4:
          if (addr >= bytes)
            fprintf(f, "         ");
          else if (cmpbuf && *(unsigned long *)(buf + addr) == *(unsigned long *)(cmpbuf + addr))
            fprintf(f, " XXXXXXXX");
          else
            fprintf(f, " %08X", *(unsigned long *)(buf + addr)); 
          break;

        case 2: 
          if (addr >= bytes)
            fprintf(f, "     ");
          else
            fprintf(f, " %04X", *(unsigned short *)(buf + addr)); 
          break;

        case 1: 
          if (addr >= bytes)
            fprintf(f, "   ");
          else
            fprintf(f, " %02X", *(unsigned char *)(buf + addr)); 
          break;
      }
    }
    fprintf(f, " ");

    for (i = 0; i < 32; i++) 
    {
      addr = start + i;

      if (addr >= bytes)
        fprintf(f, " ");
      else if (cmpbuf && cmpbuf[addr] == buf[addr])
        fprintf(f, "?");
      else if (buf[addr] < 32 || buf[addr] > 127)
        fprintf(f, ".");
      else
        fprintf(f, "%c", buf[addr]);
    }
    fprintf(f, "\n");

    start += 32;
  }
}

void convert_to_sanos_context(struct context *ctxt, CONTEXT *ctx)
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

void convert_from_sanos_context(struct context *ctxt, CONTEXT *ctx)
{
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

void enqueue_event(struct dbg_event *e)
{
  if (event_tail) event_tail->next = e;
  e->next = NULL;
  if (!event_head) event_head = e;
}

struct dbg_event *dequeue_event()
{
  struct dbg_event *e = event_head;

  if (!e) return NULL;
  event_head = e->next;
  if (!event_head) event_tail = NULL;

  return e;
}

int handshake(struct session *sess)
{
  int bytes;
  unsigned long buf[128];

  // Receive DRPC hello request
  bytes = recv(sess->debugger, (char *) buf, 512, 0);
  if (bytes == 0 || bytes < 0) return -1;

  if (buf[0] != DRPC_SIGNATURE)
  {
    logmsg("unexpected signature %08X\n");
    return -1;
  }

  logmsg("debugger ");
  dump_data(stdout, (unsigned char *) buf, bytes, 4, NULL);
  dump_data(logfile, (unsigned char *) buf, bytes, 4, NULL);

  // Send DRPC hello response
  sess->startmark = DRPC_STARTMARK;
  memset(buf, 0, 128);
  buf[0] = DRPC_SIGNATURE;
  buf[1] = 2;
  buf[6] = sess->startmark;

  send(sess->debugger, (char *) buf, 128, 0);
  
  return 0;
}

#if 0
void decode_debug_event(char *cmd, struct drpc_packet *rsp, char *buf)
{
  DEBUG_EVENT *evt;

  evt = (DEBUG_EVENT *) buf;
  logmsg("Response %s: event=%s pid=%d tid=%d\n", cmd, dbgevtnames[evt->dwDebugEventCode], evt->dwProcessId, evt->dwThreadId);
  switch (evt->dwDebugEventCode)
  {
    case EXCEPTION_DEBUG_EVENT:
      logmsg("  Exception Code: %08X\n", evt->u.Exception.ExceptionRecord.ExceptionCode);
      logmsg("  Exception Address: %08X\n", evt->u.Exception.ExceptionRecord.ExceptionAddress);
      logmsg("  Num Parameters: %08X\n", evt->u.Exception.ExceptionRecord.NumberParameters);
      logmsg("  Exception Record: %08X\n", evt->u.Exception.ExceptionRecord.ExceptionRecord);
      break;

    case CREATE_THREAD_DEBUG_EVENT:
      logmsg("  Thread Handle: %08X (%d)\n", evt->u.CreateThread.hThread, evt->u.CreateThread.hThread);
      logmsg("  Thread Local Base: %08X\n", evt->u.CreateThread.lpThreadLocalBase);
      logmsg("  Start Address: %08X\n", evt->u.CreateThread.lpStartAddress);
      break;

    case CREATE_PROCESS_DEBUG_EVENT:
      logmsg("  File Handle: %08X\n", evt->u.CreateProcessInfo.hFile);
      logmsg("  Process Handle: %08X (%d)\n", evt->u.CreateProcessInfo.hProcess, evt->u.CreateProcessInfo.hProcess);
      logmsg("  Thread Handle: %08X (%d)\n", evt->u.CreateProcessInfo.hThread, evt->u.CreateProcessInfo.hThread);
      logmsg("  Base of Image: %08X\n", evt->u.CreateProcessInfo.lpBaseOfImage);
      logmsg("  Debug Info File Offset: %08X\n", evt->u.CreateProcessInfo.dwDebugInfoFileOffset);
      logmsg("  Debug Info Size: %08X\n", evt->u.CreateProcessInfo.nDebugInfoSize);
      logmsg("  Thread Local Base: %08X\n", evt->u.CreateProcessInfo.lpThreadLocalBase);
      logmsg("  Start Address: %08X\n", evt->u.CreateProcessInfo.lpStartAddress);
      logmsg("  Image Name: %08X\n", evt->u.CreateProcessInfo.lpImageName);
      logmsg("  Unicode: %08X\n", evt->u.CreateProcessInfo.fUnicode);
      break;

    case EXIT_THREAD_DEBUG_EVENT:
      logmsg("  Exit Code: %08X\n", evt->u.ExitThread.dwExitCode);
      break;

    case EXIT_PROCESS_DEBUG_EVENT:
      logmsg("  Exit Code: %08X\n", evt->u.ExitProcess.dwExitCode);
      break;

    case LOAD_DLL_DEBUG_EVENT:
      logmsg("  File Handle: %08X\n", evt->u.LoadDll.hFile);
      logmsg("  Base of DLL: %08X\n", evt->u.LoadDll.lpBaseOfDll);
      logmsg("  Debug Info File Offset: %08X\n", evt->u.LoadDll.dwDebugInfoFileOffset);
      logmsg("  Debug Info Size: %08X\n", evt->u.LoadDll.nDebugInfoSize);
      logmsg("  Image Name: %08X\n", evt->u.LoadDll.lpImageName);
      logmsg("  Unicode: %08X\n", evt->u.LoadDll.fUnicode);
      break;

    case UNLOAD_DLL_DEBUG_EVENT:
      logmsg("  Base of DLL: %08X\n", evt->u.UnloadDll.lpBaseOfDll);
      break;

    case OUTPUT_DEBUG_STRING_EVENT:
      logmsg("  String Data: %08X\n", evt->u.DebugString.lpDebugStringData);
      logmsg("  Unicode: %08X\n", evt->u.DebugString.fUnicode);
      logmsg("  String Data Length: %08X\n", evt->u.DebugString.nDebugStringLength);
      break;

    case RIP_EVENT:
      // fall through

    default:
      logmsg("data:    ");
      dump_data(stdout, buf, rsp->rsplen > 256 ? 256 : rsp->rsplen, 4, NULL);
      dump_data(logfile, buf, rsp->rsplen, 4, NULL);
  }
}

void decode_request(struct drpc_packet *req, char *buf)
{
  unsigned long pid;
  unsigned long tid;
  unsigned long addr;
  unsigned long len;
  unsigned long sel;
  unsigned long flags;
  unsigned long n;
  CONTEXT *ctxt;

  switch (req->cmd)
  {
    case DRPC_GET_SYSTEM_VERSION:
      logmsg("COMMAND GetSystemVersion: spnamelen=%d, buildnamelen=%d\n", *(unsigned long *) (buf + 0), *(unsigned long *) (buf + 8));
      break;

    case DRPC_DEBUG_BREAK:
      tid = *(unsigned long *) (buf + 0);
      logmsg("COMMAND DebugBreak hthread=%08X\n", tid);
      break;

    case DRPC_GET_HOST_INFO:
      logmsg("COMMAND GetHostInfo: hostlen=%d, transportlen=%d\n", *(unsigned long *) (buf + 0), *(unsigned long *) (buf + 8));
      break;

    case DRPC_GET_CPU_INFO:
      logmsg("COMMAND GetCpuInfo: maxlen=%d\n", *(unsigned long *) buf);
      break;

    case DRPC_GET_PROCESS_LIST:
      logmsg("COMMAND GetProcessList: maxpids=%d\n", *(unsigned long *) buf);
      break;

    case DRPC_GET_PROCESS_NAME:
      pid = *(unsigned long *) buf;
      logmsg("COMMAND GetProcessName pid=%d:\n", pid);
      break;

    case DRPC_DEBUG_PROCESS:
      pid = *(unsigned long *) buf;
      logmsg("COMMAND DebugProcess pid=%d (%08X):\n", pid, pid);
      break;

    case DRPC_OPEN_PROCESS:
      pid = *(unsigned long *) buf;
      flags = *(unsigned long *) (buf + 8);
      logmsg("COMMAND OpenProcess pid=%d (%08X) flags=%08X:\n", pid, pid, flags);
      break;

    case DRPC_CREATE_PROCESS:
      pid = *(unsigned long *) buf;
      logmsg("COMMAND CreateProcess: cmd=%S flags=%08X:\n", buf, *(unsigned long *)(buf + req->reqlen - 4));
      break;

    case DRPC_READ_PROCESS_MEMORY:
      pid = *(unsigned long *) (buf + 0);
      addr = *(unsigned long *) (buf + 8);
      len = *(unsigned long *) (buf + 16);
      logmsg("COMMAND ReadProcessMemory hproc=%08X addr=%08x len=%d:\n", pid, addr, len);
      break;

    case DRPC_WRITE_PROCESS_MEMORY:
      pid = *(unsigned long *) (buf + 0);
      addr = *(unsigned long *) (buf + 8);
      len = *(unsigned long *) (buf + 16);
      logmsg("COMMAND WriteProcessMemory hproc=%08X addr=%08x len=%d:\n", pid, addr, len);
      break;

    case DRPC_SUSPEND_THREAD:
      len = *(unsigned long *) (buf + 0);
      logmsg("COMMAND SuspendThread: hthread=");
      for (n = 0; n < len; n++)
      {
        tid = *(unsigned long *) (buf + 8 + n * 8);
        logmsg(" %08X", tid);
      }
      logmsg("\n");
      break;

    case DRPC_RESUME_THREAD:
      len = *(unsigned long *) (buf + 0);
      logmsg("COMMAND ResumeThread: hthread=");
      for (n = 0; n < len; n++)
      {
        tid = *(unsigned long *) (buf + 8 + n * 8);
        logmsg(" %08X", tid);
      }
      logmsg("\n");
      break;

    case DRPC_GET_THREAD_CONTEXT:
      tid = *(unsigned long *) (buf + 0);
      flags = *(unsigned long *) (buf + 8);
      len = *(unsigned long *) (buf + 16);
      logmsg("COMMAND GetThreadContext hthread=%08X context=%08x len=%d:\n", tid, flags, len);
      break;

    case DRPC_SET_THREAD_CONTEXT:
      tid = *(unsigned long *) (buf + 0); 
      len = *(unsigned long *) (buf + 8);
      ctxt = (CONTEXT *) (buf + 12);
      logmsg("COMMAND SetThreadContext hthread=%08X eax=%08x len=%d:\n", tid, ctxt->Eax, len);
      break;

    case DRPC_GET_PEB_ADDRESS:
      pid = *(unsigned long *) (buf + 0);
      logmsg("COMMAND GetPebAddress hproc=%08X:\n", pid);
      break;

    case DRPC_GET_THREAD_SELECTOR:
      tid = *(unsigned long *) (buf + 0);
      sel = *(unsigned long *) (buf + 8);
      len = *(unsigned long *) (buf + 12);
      logmsg("COMMAND GetThreadSelector hthread=%08X selector=%08x len=%08x:\n", tid, sel, len);
      break;

    case DRPC_GET_DEBUG_EVENT:
      logmsg("COMMAND GetDebugEvent %08X %08X:\n", *(unsigned long *) buf, *(unsigned long *) (buf + 4));
      break;

    case DRPC_CONTINUE_DEBUG_EVENT:
      flags = *(unsigned long *) buf;
      if (flags == 0x00010001)
        logmsg("COMMAND ContinueDebugEvent: DBG_EXCEPTION_HANDLED\n");
      else if (flags == 0x00010002)
        logmsg("COMMAND ContinueDebugEvent: DBG_CONTINUE\n");
      else
        logmsg("COMMAND ContinueDebugEvent: %08X\n", flags);
      break;

    default:
      logmsg("COMMAND %08X (reqlen=%d, rsplen=%d):\n", req->cmd, req->reqlen, req->rsplen);
      if (req->reqlen > 0)
      {
        logmsg("reqdata: ");
        dump_data(stdout, buf, req->reqlen > 256 ? 256 : req->reqlen, 4, NULL);
        dump_data(logfile, buf, req->reqlen, 4, NULL);
      }
  }
}

void decode_response(struct drpc_packet *rsp, char *buf)
{
  int i;
  int n;
  int len;
  unsigned long sesid;
  struct drpc_processor_identification *cpuinfo;
  struct drpc_system_version *vers;
  CONTEXT *ctxt;

  if (rsp->result != 0) logmsg("Debuggee returned error %08X\n", rsp->result);

  switch (rsp->cmd & ~DRPC_REPLY)
  {
    case DRPC_GET_SYSTEM_VERSION:
      vers = (struct drpc_system_version *) buf;

#if 0
      memset(vers, 0, sizeof(struct drpc_system_version));
      vers->unknown1 = 0x14c;
      vers->processors = 1;
      vers->platformid = 2;
      vers->build = 2000;
      vers->spnum = 1;
      strcpy(vers->spname, "SanOS");
      strcpy(vers->buildname, "Version 0.0.4 (Build 15 Mar 2002)");
#endif  
      logmsg("RESPONSE GetSystemVersion: unknown1=%d, processors=%d platformid=%d version=%d sp=%d spname='%s' buildname='%s'\n",
             vers->unknown1, vers->processors, vers->platformid, vers->build, vers->spnum, vers->spname, vers->buildname);
      break;
    
    case DRPC_GET_HOST_INFO:
      logmsg("RESPONSE GetHostInfo: computer=%s, transport=%s\n", buf, buf + 16);
      break;

    case DRPC_GET_CPU_INFO:
      cpuinfo = (struct drpc_processor_identification *) buf;
      len = *(unsigned long *) (buf + 32);
      logmsg("RESPONSE GetCpuInfo: family=%d model=%d stepping=%d vendor=%s len=%d\n", cpuinfo->family, cpuinfo->model, cpuinfo->stepping, cpuinfo->vendor, len);
      break;

    case DRPC_GET_PROCESS_LIST:
      logmsg("RESPONSE GetProcessList:");
      n = *(int *) (buf + rsp->rsplen - 4);
      for (i = 0; i < n; i++) logmsg(" %4d", *(unsigned long *) (buf + i * 4));
      logmsg("\n");
      break;

    case DRPC_GET_PROCESS_NAME:
      logmsg("RESPONSE GetProcessName: procname=%S\n", buf);
      break;

    case DRPC_DEBUG_PROCESS:
      sesid = *(unsigned long *) (buf + 0);
      logmsg("RESPONSE DebugProcess: sesid=%d (%08x) param=%d\n", sesid, sesid, *(unsigned long *) (buf + 8));
      break;

    case DRPC_READ_PROCESS_MEMORY:
      logmsg("RESPONSE ReadProcessMemory\n");
      //logmsg("data:    ");
      //dump_data(logfile, buf, rsp->rsplen, 4, NULL );
      //dump_data(stdout, buf, rsp->rsplen > 256 ? 256 : rsp->rsplen, 4, NULL);
      break;

    case DRPC_WRITE_PROCESS_MEMORY:
      len = *(unsigned long *) (buf);
      logmsg("RESPONSE WriteProcessMemory: %d bytes written\n", len);
      break;

    case DRPC_SUSPEND_THREAD:
      logmsg("RESPONSE SuspendThread: suspendcount=");
      for (n = 0; n < (int) rsp->rsplen; n += 4) logmsg("%d ", *(unsigned long *) (buf + n));
      logmsg("\n");
      break;

    case DRPC_RESUME_THREAD:
      logmsg("RESPONSE ResumeThread: suspendcount=");
      for (n = 0; n < (int) rsp->rsplen; n += 4) logmsg("%d ", *(unsigned long *) (buf + n));
      logmsg("\n");
      break;

    case DRPC_GET_THREAD_CONTEXT:
      ctxt = (CONTEXT *) buf;
      logmsg("Response GetThreadContext\n");
      logmsg("  SegGs: %08X SegFs: %08X SegEs: %08X SegDs: %08X\n", ctxt->SegGs, ctxt->SegFs, ctxt->SegEs, ctxt->SegDs);
      logmsg("  Eax: %08X Ebx: %08X Ecx: %08X Edx: %08X\n", ctxt->Eax, ctxt->Ebx, ctxt->Ecx, ctxt->Edx);
      logmsg("  Edi: %08X Esi: %08X Ebp: %08X Eip: %08X\n", ctxt->Edi, ctxt->Esi, ctxt->Ebp, ctxt->Eip);
      logmsg("  Esp: %08X SegSs: %08X SegCs: %08X EFlags: %08X\n", ctxt->Esp, ctxt->SegSs, ctxt->SegCs, ctxt->EFlags);
      break;

    case DRPC_SET_THREAD_CONTEXT:
      len = *(unsigned long *) (buf + 0);
      logmsg("Response SetThreadContext: len=%d\n", len);
      break;

    case DRPC_GET_PEB_ADDRESS:
      logmsg("Response GetPebAddress: PEB %08x\n", *(unsigned long *) (buf + 0));
      break;

    case DRPC_GET_THREAD_SELECTOR:
      logmsg("Response GetSelector: LDT %08x %08x\n", *(unsigned long *) (buf + 0), *(unsigned long *) (buf + 4));
      break;

    case DRPC_GET_DEBUG_EVENT:
      if (rsp->result == 0) decode_debug_event("GetDebugEvent", rsp, buf);
      break;

    case DRPC_CONTINUE_DEBUG_EVENT:
      if (rsp->result == 0 && rsp->rsplen > 0) decode_debug_event("GetDebugEvent", rsp, buf);
      break;

    //case 0x0004C004:
    //  memset(buf, 0, rsp->rsplen);

    default:
      if (rsp->rsplen > 0)
      {
        logmsg("rspdata: ");
        dump_data(stdout, buf, rsp->rsplen > 256 ? 256 : rsp->rsplen, 4, NULL /*prevrsp*/);
        dump_data(logfile, buf, rsp->rsplen, 4, NULL /*prevrsp*/);
      }
  }
}
#endif

void get_process_list(struct session *sess, struct drpc_packet *pkt, char *buf)
{
  // Only one process
  *(int *) (buf + 0) = 1;
  *(int *) (buf + pkt->rsplen - 4) = 1; 
}

void get_process_name(struct session *sess, struct drpc_packet *pkt, char *buf)
{
  wcscpy((wchar_t *) buf, L"SanOS Kernel");
}

void handle_command(struct session *sess, struct drpc_packet *pkt, char *buf)
{
  unsigned long pid;
  unsigned long tid;
  unsigned long addr;
  unsigned long len;
  unsigned long sel;
  unsigned long flags;
  unsigned long n;
  CONTEXT *ctxt;

  // Validate request
  if (pkt->startmark != sess->startmark) logmsg("WARNING: unexpected startmark %08X\n", pkt->startmark);
  if (pkt->reserved1 != 0) logmsg("WARNING: reserved1 is %08X\n", pkt->reserved1);
  if (pkt->reserved2 != 0) logmsg("WARNING: reserved2 is %08X\n", pkt->reserved2);
  if (pkt->result != 0) logmsg("WARNING: result is %08X\n", pkt->result);

  // Execute command
  switch (pkt->cmd)
  {
    case DRPC_GET_SYSTEM_VERSION:
      logmsg("COMMAND GetSystemVersion: spnamelen=%d, buildnamelen=%d\n", *(unsigned long *) (buf + 0), *(unsigned long *) (buf + 8));
      break;

    case DRPC_DEBUG_BREAK:
      tid = *(unsigned long *) (buf + 0);
      logmsg("COMMAND DebugBreak hthread=%08X\n", tid);
      break;

    case DRPC_GET_HOST_INFO:
      logmsg("COMMAND GetHostInfo: hostlen=%d, transportlen=%d\n", *(unsigned long *) (buf + 0), *(unsigned long *) (buf + 8));
      break;

    case DRPC_GET_CPU_INFO:
      logmsg("COMMAND GetCpuInfo: maxlen=%d\n", *(unsigned long *) buf);
      break;

    case DRPC_GET_PROCESS_LIST:
      logmsg("COMMAND GetProcessList: maxpids=%d\n", *(unsigned long *) buf);
      get_process_list(sess, pkt, buf);
      break;

    case DRPC_GET_PROCESS_NAME:
      pid = *(unsigned long *) buf;
      logmsg("COMMAND GetProcessName pid=%d:\n", pid);
      get_process_name(sess, pkt, buf);
      break;

    case DRPC_DEBUG_PROCESS:
      pid = *(unsigned long *) buf;
      logmsg("COMMAND DebugProcess pid=%d (%08X):\n", pid, pid);
      break;

    case DRPC_OPEN_PROCESS:
      pid = *(unsigned long *) buf;
      flags = *(unsigned long *) (buf + 8);
      logmsg("COMMAND OpenProcess pid=%d (%08X) flags=%08X:\n", pid, pid, flags);
      break;

    case DRPC_CREATE_PROCESS:
      pid = *(unsigned long *) buf;
      logmsg("COMMAND CreateProcess: cmd=%S flags=%08X:\n", buf, *(unsigned long *)(buf + pkt->reqlen - 4));
      break;

    case DRPC_READ_PROCESS_MEMORY:
      pid = *(unsigned long *) (buf + 0);
      addr = *(unsigned long *) (buf + 8);
      len = *(unsigned long *) (buf + 16);
      logmsg("COMMAND ReadProcessMemory hproc=%08X addr=%08x len=%d:\n", pid, addr, len);
      break;

    case DRPC_WRITE_PROCESS_MEMORY:
      pid = *(unsigned long *) (buf + 0);
      addr = *(unsigned long *) (buf + 8);
      len = *(unsigned long *) (buf + 16);
      logmsg("COMMAND WriteProcessMemory hproc=%08X addr=%08x len=%d:\n", pid, addr, len);
      break;

    case DRPC_SUSPEND_THREAD:
      len = *(unsigned long *) (buf + 0);
      logmsg("COMMAND SuspendThread: hthread=");
      for (n = 0; n < len; n++)
      {
        tid = *(unsigned long *) (buf + 8 + n * 8);
        logmsg(" %08X", tid);
      }
      logmsg("\n");
      break;

    case DRPC_RESUME_THREAD:
      len = *(unsigned long *) (buf + 0);
      logmsg("COMMAND ResumeThread: hthread=");
      for (n = 0; n < len; n++)
      {
        tid = *(unsigned long *) (buf + 8 + n * 8);
        logmsg(" %08X", tid);
      }
      logmsg("\n");
      break;

    case DRPC_GET_THREAD_CONTEXT:
      tid = *(unsigned long *) (buf + 0);
      flags = *(unsigned long *) (buf + 8);
      len = *(unsigned long *) (buf + 16);
      logmsg("COMMAND GetThreadContext hthread=%08X context=%08x len=%d:\n", tid, flags, len);
      break;

    case DRPC_SET_THREAD_CONTEXT:
      tid = *(unsigned long *) (buf + 0); 
      len = *(unsigned long *) (buf + 8);
      ctxt = (CONTEXT *) (buf + 12);
      logmsg("COMMAND SetThreadContext hthread=%08X eax=%08x len=%d:\n", tid, ctxt->Eax, len);
      break;

    case DRPC_GET_PEB_ADDRESS:
      pid = *(unsigned long *) (buf + 0);
      logmsg("COMMAND GetPebAddress hproc=%08X:\n", pid);
      break;

    case DRPC_GET_THREAD_SELECTOR:
      tid = *(unsigned long *) (buf + 0);
      sel = *(unsigned long *) (buf + 8);
      len = *(unsigned long *) (buf + 12);
      logmsg("COMMAND GetThreadSelector hthread=%08X selector=%08x len=%08x:\n", tid, sel, len);
      break;

    case DRPC_GET_DEBUG_EVENT:
      logmsg("COMMAND GetDebugEvent %08X %08X:\n", *(unsigned long *) buf, *(unsigned long *) (buf + 4));
      break;

    case DRPC_CONTINUE_DEBUG_EVENT:
      flags = *(unsigned long *) buf;
      if (flags == 0x00010001)
        logmsg("COMMAND ContinueDebugEvent: DBG_EXCEPTION_HANDLED\n");
      else if (flags == 0x00010002)
        logmsg("COMMAND ContinueDebugEvent: DBG_CONTINUE\n");
      else
        logmsg("COMMAND ContinueDebugEvent: %08X\n", flags);
      break;

    default:
      logmsg("COMMAND %08X (reqlen=%d, rsplen=%d):\n", pkt->cmd, pkt->reqlen, pkt->rsplen);
      if (pkt->reqlen > 0)
      {
        logmsg("reqdata: ");
        dump_data(stdout, buf, pkt->reqlen > 256 ? 256 : pkt->reqlen, 4, NULL);
        dump_data(logfile, buf, pkt->reqlen, 4, NULL);
      }
  }

  pkt->cmd |= 0x00010000;
  logmsg("\n");
}

void handle_session(void *arg)
{
  struct session *sess = (struct session *) arg;
  struct drpc_packet pkt;
  int bytes;
  char *buf = NULL;
  unsigned int buflen = 0;
  char fn[256];

  printf("----------------------------------------------------------------------------------------------\n");

  sprintf(fn, "dbgspy%8d.log", GetTickCount());
  logfile = fopen(fn, "w");

  if (handshake(sess) < 0)
  {
    CloseHandle(sess->debuggee);
    closesocket(sess->debugger);
    return;
  }

  while (1)
  {
    // Receive header from debugger
    bytes = recv(sess->debugger, (char *) &pkt, sizeof(struct drpc_packet), 0);
    if (bytes == 0 || bytes < 0)
    {
      CloseHandle(sess->debuggee);
      closesocket(sess->debugger);
      break;
    }
      
    // Allocate data
    if (pkt.reqlen > 0 && pkt.reqlen > buflen) 
    {
      buf = realloc(buf, pkt.reqlen);
      buflen = pkt.reqlen;
    }

    if (pkt.rsplen > 0 && pkt.rsplen > buflen) 
    {
      buf = realloc(buf, pkt.rsplen);
      buflen = pkt.rsplen;
    }

    // Receive request data
    if (pkt.reqlen > 0) recv(sess->debugger, buf, pkt.reqlen, 0);

    // Handle command
    handle_command(sess, &pkt, buf);

    // Send response to debugger
    send(sess->debugger, (char *) &pkt, sizeof(struct drpc_packet), 0);
    if (pkt.rsplen > 0) send(sess->debugger, buf, pkt.rsplen, 0);
  }

  printf("terminating session\n");
  fclose(logfile);
  logfile = NULL;
}

void handle_connection(SOCKET s)
{
  struct session *sess;

  printf("starting new session\n");
  sess = (struct session *) malloc(sizeof(struct session));
  sess->debugger = s;

  _beginthread(handle_session, 0, sess);
}

BOOL WINAPI break_handler(DWORD ctrl_type)
{
  if (ctrl_type == CTRL_BREAK_EVENT || ctrl_type ==  CTRL_C_EVENT)
  {
    closesocket(listener);
    return TRUE;
  }
  else
    return FALSE;
}

void main0()
{ 
  SOCKET s;
  WSADATA wsadata;
  SOCKADDR_IN sin;
  int rc;

  // Initialize winsock
  rc = WSAStartup(MAKEWORD(2, 2), &wsadata);
  if (rc != 0) panic("error in WSAStartup");

  // Create listen socket
  listener = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
  if (listener == INVALID_SOCKET) panic("error in socket");

  // Bind the socket to the service port
  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = INADDR_ANY;
  sin.sin_port = htons(DEBUGGER_PORT);
  if (bind(listener, (LPSOCKADDR) &sin, sizeof(sin)) == SOCKET_ERROR) panic("error in bind");

  // Listen for incoming connections on the socket
  if (listen(listener, 5) == SOCKET_ERROR) panic("error in listen");

  // Wait and accept new connection from client
  while (1)
  {
    s = accept(listener, NULL, NULL);
    if (s == INVALID_SOCKET) panic("error in accept");

    handle_connection(s);
  }

  closesocket(listener);
}

int setup_comport(int baudrate)
{
  DCB dcb;

  if (!GetCommState(debugee, &dcb)) return -1;

  dcb.BaudRate = baudrate;
  dcb.ByteSize = DATABITS_8;
  dcb.StopBits = ONESTOPBIT;
  dcb.Parity = NOPARITY;

  if (!SetCommState(debugee, &dcb)) return -1;
  
  return 0;
}

static void dbg_send(void *buffer, int count)
{
  unsigned long bytes;

  WriteFile(debugee, buffer, count, &bytes, NULL);
}

static void dbg_recv(void *buffer, int count)
{
  unsigned long bytes;
  char *p = buffer;

  while (count > 0)
  {
    if (!ReadFile(debugee, p, count, &bytes, NULL)) 
    {
      printf("Error %d reading from com port\n", GetLastError());
      return;
    }

    count -= bytes;
    p += bytes;
  }
}

static void dbg_send_packet(int cmd, unsigned char id, void *data, unsigned int len)
{
  unsigned int n;
  struct dbg_hdr hdr;
  unsigned char checksum;
  unsigned char *p;

  hdr.signature = DBG_SIGNATURE;
  hdr.cmd = (unsigned char) cmd;
  hdr.len = len;
  hdr.id = id;
  hdr.checksum = 0;

  checksum = 0;
  p = (unsigned char *) &hdr;
  for (n = 0; n < sizeof(struct dbg_hdr); n++) checksum += *p++;
  p = (unsigned char *) data;
  for (n = 0; n < len; n++) checksum += *p++;
  hdr.checksum = -checksum;

  dbg_send(&hdr, sizeof(struct dbg_hdr));
  dbg_send(data, len);
}

static int dbg_recv_packet(struct dbg_hdr *hdr, void *data)
{
  unsigned int n;
  unsigned char checksum;
  unsigned char *p;

  while (1)
  {
    dbg_recv(&hdr->signature, 1);
    if (hdr->signature == DBG_SIGNATURE) break;
  }

  dbg_recv(&hdr->cmd, 1);
  dbg_recv(&hdr->id, 1);
  dbg_recv(&hdr->checksum, 1);
  dbg_recv((unsigned char *) &hdr->len, 4);
  if (hdr->len > MAX_DBG_PACKETLEN) return -1;
  dbg_recv(data, hdr->len);

  checksum = 0;
  p = (unsigned char *) hdr;
  for (n = 0; n < sizeof(struct dbg_hdr); n++) checksum += *p++;
  p = (unsigned char *) data;
  for (n = 0; n < hdr->len; n++) checksum += *p++;
  if (checksum != 0) return -2;

  return hdr->len;
}

int dbg_xact(unsigned char cmd, void *reqdata, unsigned int len, void *rspdata)
{
  struct dbg_hdr hdr;
  int rc;
  unsigned char reqid = next_reqid++;

  dbg_send_packet(cmd, reqid, reqdata, len);
  rc = dbg_recv_packet(&hdr, rspdata);
  if (rc < 0)
  {
    printf("Error %d receiving response for command 0x%x\n", rc);
    return rc;
  }

  if (hdr.id != reqid)
  {
    printf("Error: reqid %d expected, got %d\n", reqid, hdr.id);
    return -4;
  }

  if (hdr.cmd >= 0x80)
  {
    printf("Error: debuggee returned error %d %s\n", hdr.cmd, hdr.len > 0 ? (char *) rspdata : "");
    return -hdr.cmd;
  }

  return hdr.cmd;
}

int dbg_read_mem(void *addr, void *buffer, unsigned long size)
{
  struct dbg_memory mem;

  mem.addr = addr;
  mem.size = size;
  return dbg_xact(DBGCMD_READ_MEMORY, &mem, sizeof(struct dbg_memory), buffer);
}

void read_mem(unsigned long addr, unsigned long len)
{
  struct dbg_memory mem;

  if (len <= 0 || len >= 4096) len = 4096;
  mem.addr = (void *) addr;
  mem.size = len;

  if (dbg_xact(DBGCMD_READ_MEMORY, &mem, sizeof(struct dbg_memory), body) > 0)
  {
    printf("data:    ");
    dump_data(stdout, body->data, len, 1, NULL);
  }
}

void get_mod_list()
{
  int n;
  char name[256];


  if (dbg_xact(DBGCMD_GET_MODULES, NULL, 0, body) < 0) return;

  for (n = 0; n < body->mod.count; n++)
  {
    if (body->mod.mods[n].name != NULL)
      dbg_read_mem(body->mod.mods[n].name, name, 256);
    else
      *name = 0;

    printf("hmod %08X name: %08X %s\n", body->mod.mods[n].hmod, body->mod.mods[n].name, name);
  }
}

void get_thread_list()
{
  int n;

  if (dbg_xact(DBGCMD_GET_THREADS, NULL, 0, body) < 0) return;

  for (n = 0; n < body->thr.count; n++)
  {
    printf("thread id %04X\n", body->thr.threadids[n]);
  }
}

void get_context(unsigned long tid)
{
  struct dbg_context ctx;
  struct context *ctxt = &ctx.ctxt;

  ctx.tid = tid;

  if (dbg_xact(DBGCMD_GET_THREAD_CONTEXT, &ctx, 4, &ctx) < 0) return;

  printf("EAX  = %08X EBX  = %08X ECX  = %08X EDX  = %08X\n", ctxt->eax, ctxt->ebx, ctxt->ecx, ctxt->edx);
  printf("EDI  = %08X ESI  = %08X EBP  = %08X ESP  = %08X\n", ctxt->edi, ctxt->esi, ctxt->ebp, ctxt->esp);
  printf("CS   = %08X DS   = %08X ES   = %08X SS   = %08X\n", ctxt->ecs, ctxt->ds, ctxt->es, ctxt->ess);
  printf("EIP  = %08X EFLG = %08X TRAP = %08X ERR  = %08X\n", ctxt->eip, ctxt->eflags, ctxt->traptype, ctxt->errcode);
}

void handle_event(struct dbg_hdr *hdr, union dbg_body *body)
{
  char msg[1024];

  switch (hdr->cmd)
  {
    case DBGEVT_TRAP:
      printf("break: thread %d trap %d eip %08X addr %08X\n", 
             body->trap.tid, body->trap.traptype, body->trap.eip, body->trap.addr);
      return;

    case DBGEVT_CREATE_THREAD:
      printf("create: thread %04X tib %08X entry %08X\n", body->create.tid, body->create.tib, body->create.startaddr);
      break;

    case DBGEVT_EXIT_THREAD:
      printf("exit: thread %04X exitcode %d\n", body->exit.tid, body->exit.exitcode);
      break;

    case DBGEVT_LOAD_MODULE:
      printf("load: module %08X\n", body->load.hmod);
      break;

    case DBGEVT_UNLOAD_MODULE:
      printf("unload: module %08X\n", body->unload.hmod);
      break;

    case DBGEVT_OUTPUT:
      dbg_read_mem(body->output.msgptr, msg, body->output.msglen);
      printf("%s", msg);
      break;

    default:
      printf("unknown: cmd %d\n", hdr->cmd);

  }
}

void go()
{
  struct dbg_hdr hdr;

  while (1)
  {
    if (dbg_xact(DBGCMD_CONTINUE, NULL, 0, body) < 0) return;
    dbg_recv_packet(&hdr, body);
    handle_event(&hdr, body);
  }
}

void main()
{
  char cmd[256];
  char command;
  unsigned long arg1;
  unsigned long arg2;
  int done;

  debugee = CreateFile("COM1", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
  if (debugee == INVALID_HANDLE_VALUE)
  {
    printf("Error %d opening com port\n", GetLastError());
    return;
  }

  if (setup_comport(115200) < 0)
  {
    printf("Error configuring com port\n");
    return;
  }

  printf("Connecting to debugger...\n");
  body = (union dbg_body *) dbg_data;

  body->conn.version = DRPC_VERSION;
  dbg_send_packet(DBGCMD_CONNECT, next_reqid++, body, 4);

  if (dbg_recv_packet(&hdr, body) < 0)
  {
    printf("Unable to connect to debugger\n");
    return;
  }

  if (hdr.cmd == (DBGCMD_CONNECT | DBGCMD_REPLY))
  {
    printf("break: thread %d trap %d eip %08X addr %08X\n", 
           body->conn.tid, body->conn.traptype, body->conn.eip, body->conn.addr);
  }
  else
    handle_event(&hdr, body);

  done = 0;
  while (!done)
  {
    printf("> ");
    gets(cmd);
    command = ' ';
    arg1 = arg2 = 0;
    sscanf(cmd, "%c %x %x", &command, &arg1, &arg2);
    printf("command: %c %08X %08X\n", command, arg1, arg2);

    switch (command)
    {
      case 'r':
        read_mem(arg1, arg2);
        break;

      case 'm':
        get_mod_list();
        break;

      case 't':
        get_thread_list();
        break;

      case 'c':
        get_context(arg1);
        break;

      case 'g':
        go();
        break;

      case 'x':
        dbg_xact(DBGCMD_CONTINUE, NULL, 0, body);
        done = 1;
        break;
    }
  }

  CloseHandle(debugee);
}
