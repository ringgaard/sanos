//
// dbg.c
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Remote debugging support
//

#include <os/krnl.h>

#define DEBUGPORT  0x3F8 // COM1
#define DBG_SIGNATURE 0xDB
#define MAX_DBG_PACKETLEN 4096
#define DRPC_VERSION 1

//
// Debugger commands
//

#define DBGCMD_CONNECT            0
#define DBGCMD_CONTINUE           1
#define DBGCMD_READ_MEMORY        2
#define DBGCMD_WRITE_MEMORY       3
#define DBGCMD_SUSPEND_THREAD     4
#define DBGCMD_RESUME_THREAD      5
#define DBGCMD_GET_THREAD_CONTEXT 6
#define DBGCMD_SET_THREAD_CONTEXT 7
#define DBGCMD_GET_SELECTOR       8
#define DBGCMD_GET_DEBUG_EVENT    9
#define DBGCMD_GET_MODULES        10
#define DBGCMD_GET_THREADS        11

#define DBGERR_VERSION            128

static int debug_initialized = 0;
static char dbgdata[MAX_DBG_PACKETLEN];

struct dbghdr
{
  unsigned char signature;
  unsigned char cmd;
  unsigned char id;
  unsigned char checksum;
  unsigned int len;
};

void init_debug_port()
{
  // Turn off interrupts
  _outp(DEBUGPORT + 1, 0);
  
  // Set 9600 baud, 8 bits, no parity, one stopbit
  _outp(DEBUGPORT + 3, 0x80);
  _outp(DEBUGPORT + 0, 0x0C); // 0x0C = 9600, 0x01 = 115200
  _outp(DEBUGPORT + 1, 0x00);
  _outp(DEBUGPORT + 3, 0x03);
  _outp(DEBUGPORT + 2, 0xC7);
  _outp(DEBUGPORT + 4, 0x0B);
}

static void dbg_send(void *buffer, int count)
{
  unsigned char *p = buffer;

  while (count-- > 0)
  {
    while ((_inp(DEBUGPORT + 5) & 0x20) == 0);
    _outp(DEBUGPORT, *p++);
  }
}

static void dbg_recv(void *buffer, int count)
{
  unsigned char *p = buffer;

  while (count-- > 0)
  {
    while ((_inp(DEBUGPORT + 5) & 0x01) == 0);
    *p++ = _inp(DEBUGPORT) & 0xFF;
  }
}

static void dbg_send_packet(unsigned char cmd, unsigned char id, void *data, unsigned int len)
{
  unsigned int n;
  struct dbghdr hdr;
  unsigned char checksum;
  unsigned char *p;

  hdr.signature = DBG_SIGNATURE;
  hdr.cmd = cmd;
  hdr.len = len;
  hdr.id = id;
  hdr.checksum = 0;

  checksum = 0;
  p = (unsigned char *) &hdr;
  for (n = 0; n < sizeof(struct dbghdr); n++) checksum += *p++;
  p = (unsigned char *) data;
  for (n = 0; n < len; n++) checksum += *p++;
  hdr.checksum = -checksum;

  dbg_send(&hdr, sizeof(struct dbghdr));
  dbg_send(data, len);
}

static void dbg_send_error(unsigned char errcode, unsigned char id)
{
  dbg_send_packet(errcode, id, NULL, 0);
}

static int dbg_recv_packet(struct dbghdr *hdr, void *data)
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
  if (hdr->len > MAX_DBG_PACKETLEN) return -EBUF;
  dbg_recv(data, hdr->len);

  checksum = 0;
  p = (unsigned char *) &hdr;
  for (n = 0; n < sizeof(struct dbghdr); n++) checksum += *p++;
  p = (unsigned char *) data;
  for (n = 0; n < hdr->len; n++) checksum += *p++;
  if (checksum != 0) return -EIO;

  return hdr->len;
}

static void dbg_connect(struct dbghdr *hdr, char *data)
{
  int version = *(int *) data;

  if (version != DRPC_VERSION)
    dbg_send_error(DBGERR_VERSION, hdr->id);
  else
  {
    version = DRPC_VERSION;
    dbg_send_packet(DBGCMD_CONNECT, hdr->id, &version, 4);
  }
}

static void dbg_read_memory(struct dbghdr *hdr, char *data)
{
}

static void dbg_write_memory(struct dbghdr *hdr, char *data)
{
}

static void dbg_suspend_thread(struct dbghdr *hdr, char *data)
{
}

static void dbg_resume_thread(struct dbghdr *hdr, char *data)
{
}

static void dbg_get_thread_context(struct dbghdr *hdr, char *data)
{
}

static void dbg_set_thread_context(struct dbghdr *hdr, char *data)
{
}

static void dbg_get_selector(struct dbghdr *hdr, char *data)
{
}

static void dbg_get_debug_event(struct dbghdr *hdr, char *data)
{
}

static void dbg_get_modules(struct dbghdr *hdr, char *data)
{
}

static void handle_drpc()
{
  struct dbghdr hdr;
  int rc;

  if (!debug_initialized)
  {
    init_debug_port();
    debug_initialized = 1;
    kprintf("dbg: waiting for remote debugger...\n");
  }

  while (1)
  {
    rc = dbg_recv_packet(&hdr, dbgdata);
    if (rc < 0)
    {
      kprintf("dbg: error %d receiving debugger command\n", rc);
      continue;
    }

    switch (hdr.cmd)
    {
      case DBGCMD_CONNECT:
	dbg_connect(&hdr, dbgdata);
	break;

      case DBGCMD_CONTINUE:
        dbg_send_packet(DBGCMD_CONTINUE, hdr.id, NULL, 0);
	return;

      case DBGCMD_READ_MEMORY:
	dbg_read_memory(&hdr, dbgdata);
	break;

      case DBGCMD_WRITE_MEMORY:
	dbg_write_memory(&hdr, dbgdata);
	break;

      case DBGCMD_SUSPEND_THREAD:
	dbg_suspend_thread(&hdr, dbgdata);
	break;

      case DBGCMD_RESUME_THREAD:
	dbg_resume_thread(&hdr, dbgdata);
	break;

      case DBGCMD_GET_THREAD_CONTEXT:
	dbg_get_thread_context(&hdr, dbgdata);
	break;

      case DBGCMD_SET_THREAD_CONTEXT:
	dbg_set_thread_context(&hdr, dbgdata);
	break;

      case DBGCMD_GET_SELECTOR:
	dbg_get_selector(&hdr, dbgdata);
	break;

      case DBGCMD_GET_DEBUG_EVENT:
	dbg_get_debug_event(&hdr, dbgdata);
	break;

      case DBGCMD_GET_MODULES:
	dbg_get_modules(&hdr, dbgdata);
	break;
    }
  }
}

void dumpregs(struct context *ctxt)
{
  kprintf("EAX  = %08X EBX  = %08X ECX  = %08X EDX  = %08X\n", ctxt->eax, ctxt->ebx, ctxt->ecx, ctxt->edx);
  kprintf("EDI  = %08X ESI  = %08X EBP  = %08X ESP  = %08X\n", ctxt->edi, ctxt->esi, ctxt->ebp, ctxt->esp);
  kprintf("CS   = %08X DS   = %08X ES   = %08X SS   = %08X\n", ctxt->ecs, ctxt->ds, ctxt->es, ctxt->ess);
  kprintf("EIP  = %08X EFLG = %08X TRAP = %08X ERR  = %08X\n", ctxt->eip, ctxt->eflags, ctxt->traptype, ctxt->errcode);
}

void shell();

void dbg_enter(struct context *ctxt, void *addr)
{
  kprintf("trap %d (%p)\n", ctxt->traptype, addr);
  kprintf("enter kernel debugger\n");
  dumpregs(ctxt);
  //if (ctxt->traptype != 3) panic("system halted");
  shell();
}

void dbg_notify_create_thread(struct thread *t)
{
}

void dbg_notify_exit_thread(struct thread *t)
{
}

void dbg_notify_load_module(hmodule_t hmod)
{
}

void dbg_notify_unload_module(hmodule_t hmod)
{
}

void dbg_output(char *msg)
{
}
