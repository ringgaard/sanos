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

#include "rdp.h"
//
// DRPC definitions
//

#define DRPC_SIGNATURE            0x43505244
#define DRPC_STARTMARK            0x00286308
#define DRPC_REPLY                0x00010000

//
// DRPC commands
//

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

//
// DRPC packet format
//

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

struct drpc_processor_identification
{
  unsigned long family;
  unsigned long model;
  unsigned long stepping;
  char vendor[16];
};

//
// Debug gateway configiration
//

#define DEBUGGER_PORT      24502
#define DEBUGGEE_PORT      "COM1"

//
// Global variables
//

SOCKET listener;
SOCKET debugger;

int dbg_state = 1;
int first_event = 1;
struct dbg_session *session = NULL;

FILE *logfile = NULL;

unsigned long startmark;

//
// panic
//

void panic(char *msg)
{
  printf("panic: %s\n", msg);
  exit(1);
}

//
// logmsg
//

void logmsg(const char *fmt, ...)
{
  va_list args;

  va_start(args, fmt);
  if (logfile) vfprintf(logfile, fmt, args);
  vprintf(fmt, args);
  va_end(args);
}

//
// recv_fully
//

int recv_fully(SOCKET s, char *buf, int size, int flags)
{
  int bytes;
  int left;

  left = size;
  while (left > 0)
  {
    bytes = recv(s, buf, left, flags);
    if (bytes <= 0) return bytes;

    buf += bytes;
    left -= bytes;
  }

  return size;
}

//
// dump_data
//

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

//
// convert_to_sanos_context
//

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

//
// convert_from_sanos_context
//

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

//
// get_process_list
//

void get_process_list(struct drpc_packet *pkt, char *buf)
{
  // Only one process
  *(int *) (buf + 0) = GLOBAL_PROCID;
  *(int *) (buf + pkt->rsplen - 4) = 1; 
}

//
// get_process_name
//

void get_process_name(struct drpc_packet *pkt, char *buf)
{
  wcscpy((wchar_t *) buf, L"SanOS Kernel");
}

//
// get_host_info
//

void get_host_info(struct drpc_packet *pkt, char *buf)
{
  int hostlen = *(int *) (buf + 0);
  int tplen = *(int *) (buf + 16);

  strcpy(buf, "CASTOR");
  strcpy(buf + hostlen, "tcp 127.0.0.1, port 24502");
}

//
// get_system_version
//

void get_system_version(struct drpc_packet *pkt, char *buf)
{
  struct drpc_system_version *vers = (struct drpc_system_version *) buf;

  memset(vers, 0, sizeof(struct drpc_system_version));
  vers->unknown1 = 0x14c;
  vers->processors = 1;
  vers->platformid = 2;
  vers->build = 2000;
  vers->spnum = 1;
  strcpy(vers->spname, "SanOS");
  strcpy(vers->buildname, "Version 0.0.4 (Build 15 Mar 2002)");
}

//
// get_cpu_info
//

void get_cpu_info(struct drpc_packet *pkt, char *buf)
{
  int maxlen = *(int *) buf;
  struct drpc_processor_identification *cpuinfo = (struct drpc_processor_identification *) buf;

  *(int *) (buf + maxlen) = sizeof(struct drpc_processor_identification);

  cpuinfo->family = 6;
  cpuinfo->model = 5;
  cpuinfo->stepping = 2;
  strcpy(cpuinfo->vendor, "GenuineIntel");
}

//
// open_process
//

void open_process(struct drpc_packet *pkt, char *buf)
{
  session = dbg_create_session(DEBUGGEE_PORT);
  if (session == NULL) pkt->result = E_FAIL;
  first_event = 1;
}

//
// get_debug_event
//

void get_debug_event(struct drpc_packet *pkt, char *buf)
{
  int evtlen;
  struct dbg_event *e;
  DEBUG_EVENT *evt;

  evtlen = *(int *) (buf + 4);
  evt = (DEBUG_EVENT *) buf;
  memset(evt, 0, sizeof(DEBUG_EVENT));
  *(int *) (buf + evtlen) = sizeof(DEBUG_EVENT);

  if (first_event)
  {
    evt->dwDebugEventCode = CREATE_PROCESS_DEBUG_EVENT;
    evt->dwProcessId = GLOBAL_PROCID;
    evt->dwThreadId = session->conn.trap.tid;

    evt->u.CreateProcessInfo.hFile = 0;
    evt->u.CreateProcessInfo.hProcess = (HANDLE) GLOBAL_HPROC;
    evt->u.CreateProcessInfo.hThread = (HANDLE) session->conn.trap.tid;
    evt->u.CreateProcessInfo.lpBaseOfImage = session->conn.mod.hmod;
    evt->u.CreateProcessInfo.dwDebugInfoFileOffset = 0;
    evt->u.CreateProcessInfo.nDebugInfoSize = 0;
    evt->u.CreateProcessInfo.lpThreadLocalBase = session->conn.thr.tib;
    evt->u.CreateProcessInfo.lpStartAddress = session->conn.thr.startaddr;
    evt->u.CreateProcessInfo.lpImageName = NULL; //session->conn.mod.name; Should be address to string pointer !!!
    evt->u.CreateProcessInfo.fUnicode = 0;

    first_event = 0;
    return;
  }

  e = dbg_next_event(session);
  dbg_release_event(e);
}

//
// read_memory
//

void read_memory(struct drpc_packet *pkt, char *buf)
{
  void *addr;
  int size;

  addr = *(void **) (buf + 8);
  size = *(int *) (buf + 16);

  dbg_read_memory(session, addr, size, buf);
  *(int *) (buf + size) = size;
}

//
// get_peb_address
//

void get_peb_address(struct drpc_packet *pkt, char *buf)
{
  *(unsigned long *) (buf) = 0x7FFDF000;
}

//
// handle_command
//

void handle_command(struct drpc_packet *pkt, char *buf)
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
  if (pkt->startmark != startmark) logmsg("WARNING: unexpected startmark %08X\n", pkt->startmark);
  if (pkt->reserved1 != 0) logmsg("WARNING: reserved1 is %08X\n", pkt->reserved1);
  if (pkt->reserved2 != 0) logmsg("WARNING: reserved2 is %08X\n", pkt->reserved2);
  if (pkt->result != 0) logmsg("WARNING: result is %08X\n", pkt->result);

  // Execute command
  switch (pkt->cmd)
  {
    case DRPC_GET_SYSTEM_VERSION:
      logmsg("COMMAND GetSystemVersion: spnamelen=%d, buildnamelen=%d\n", *(unsigned long *) (buf + 0), *(unsigned long *) (buf + 8));
      get_system_version(pkt, buf);
      break;

    case DRPC_DEBUG_BREAK:
      tid = *(unsigned long *) (buf + 0);
      logmsg("COMMAND DebugBreak hthread=%08X\n", tid);
      break;

    case DRPC_GET_HOST_INFO:
      logmsg("COMMAND GetHostInfo: hostlen=%d, transportlen=%d\n", *(unsigned long *) (buf + 0), *(unsigned long *) (buf + 8));
      get_host_info(pkt, buf);
      break;

    case DRPC_GET_CPU_INFO:
      logmsg("COMMAND GetCpuInfo: maxlen=%d\n", *(unsigned long *) buf);
      get_cpu_info(pkt, buf);
      break;

    case DRPC_GET_PROCESS_LIST:
      logmsg("COMMAND GetProcessList: maxpids=%d\n", *(unsigned long *) buf);
      get_process_list(pkt, buf);
      break;

    case DRPC_GET_PROCESS_NAME:
      pid = *(unsigned long *) buf;
      logmsg("COMMAND GetProcessName pid=%d:\n", pid);
      get_process_name(pkt, buf);
      break;

    case DRPC_DEBUG_PROCESS:
      pid = *(unsigned long *) buf;
      logmsg("COMMAND DebugProcess pid=%d (%08X):\n", pid, pid);
      break;

    case DRPC_OPEN_PROCESS:
      pid = *(unsigned long *) buf;
      flags = *(unsigned long *) (buf + 8);
      logmsg("COMMAND OpenProcess pid=%d (%08X) flags=%08X:\n", pid, pid, flags);
      open_process(pkt, buf);
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
      read_memory(pkt, buf);
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
      get_peb_address(pkt, buf);
      break;

    case DRPC_GET_THREAD_SELECTOR:
      tid = *(unsigned long *) (buf + 0);
      sel = *(unsigned long *) (buf + 8);
      len = *(unsigned long *) (buf + 12);
      logmsg("COMMAND GetThreadSelector hthread=%08X selector=%08x len=%08x:\n", tid, sel, len);
      break;

    case DRPC_GET_DEBUG_EVENT:
      logmsg("COMMAND GetDebugEvent %08X %08X:\n", *(unsigned long *) buf, *(unsigned long *) (buf + 4));
      get_debug_event(pkt, buf);
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

    case 0x0000C001:
      logmsg("COMMAND 0000C001\n");
      pkt->result = dbg_state;
      break;

    case 0x0004C002:
      logmsg("COMMAND 0004C002\n");
      *(unsigned long *) buf = 5;
      break;

    default:
      logmsg("COMMAND %08X ??? (reqlen=%d, rsplen=%d):\n", pkt->cmd, pkt->reqlen, pkt->rsplen);
      if (pkt->reqlen > 0)
      {
	logmsg("reqdata: ");
	dump_data(stdout, buf, pkt->reqlen > 256 ? 256 : pkt->reqlen, 4, NULL);
	if (logfile) dump_data(logfile, buf, pkt->reqlen, 4, NULL);
      }
  }

  pkt->cmd |= 0x00010000;
  logmsg("\n");
}

//
// handshake
//

int handshake()
{
  int bytes;
  unsigned long buf[64];

  // Receive DRPC hello request
  bytes = recv_fully(debugger, (char *) buf, 256, 0);
  if (bytes == 0 || bytes < 0) return -1;

  if (buf[0] != DRPC_SIGNATURE)
  {
    logmsg("unexpected signature %08X\n");
    return -1;
  }

  // Send DRPC hello response
  startmark = DRPC_STARTMARK;
  memset(buf, 0, 128);
  buf[0] = DRPC_SIGNATURE;
  buf[1] = 2;
  buf[6] = startmark;
  buf[10] = 1;

  send(debugger, (char *) buf, 128, 0);
  
  return 0;
}

//
// handle_session
//

void handle_session()
{
  struct drpc_packet pkt;
  int bytes;
  char *buf = NULL;
  unsigned int buflen = 0;
  char fn[256];

  printf("starting session\n");
  sprintf(fn, "dbgspy%8d.log", GetTickCount());
  //logfile = fopen(fn, "w");

  first_event = 1;
  if (handshake() < 0)
  {
    closesocket(debugger);
    return;
  }

  while (1)
  {
    // Receive header from debugger
    bytes = recv_fully(debugger, (char *) &pkt, sizeof(struct drpc_packet), 0);
    if (bytes == 0 || bytes < 0)
    {
      closesocket(debugger);
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
    if (pkt.reqlen > 0) recv_fully(debugger, buf, pkt.reqlen, 0);

    // Handle command
    handle_command(&pkt, buf);

    // Send response to debugger
    send(debugger, (char *) &pkt, sizeof(struct drpc_packet), 0);
    if (pkt.rsplen > 0) send(debugger, buf, pkt.rsplen, 0);
  }

  printf("terminating session\n");
  if (logfile) fclose(logfile);
  logfile = NULL;
  if (buf) free(buf);
  if (session)
  {
    dbg_close_session(session);
    session = NULL;
  }
}

//
// main
//

void main()
{ 
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

  printf("waiting for debugger...\n");

  // Wait and accept new connection from client
  while (1)
  {
    debugger = accept(listener, NULL, NULL);
    if (debugger == INVALID_SOCKET) panic("error in accept");

    handle_session();
  }

  closesocket(listener);
}
