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

//
// Debugger commands
//

#define DBGCMD_READ_MEMORY        1
#define DBGCMD_WRITE_MEMORY       2
#define DBGCMD_SUSPEND_THREAD     3
#define DBGCMD_RESUME_THREAD      4
#define DBGCMD_GET_THREAD_CONTEXT 5
#define DBGCMD_SET_THREAD_CONTEXT 6
#define DBGCMD_GET_SELECTOR       7
#define DBGCMD_GET_DEBUG_EVENT    8
#define DBGCMD_CONTINUE           9

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

static void dbg_send(unsigned char *buffer, int count)
{
  while (count-- > 0)
  {
    while ((_inp(DEBUGPORT + 5) & 0x20) == 0);
    _outp(DEBUGPORT, *buffer++);
  }
}

static void dbg_recv(unsigned char *buffer, int count)
{
  while (count-- > 0)
  {
    while ((_inp(DEBUGPORT + 5) & 0x01) == 0);
    *buffer++ = _inp(DEBUGPORT) & 0xFF;
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

  dbg_send((unsigned char *) &hdr, sizeof(struct dbghdr));
  dbg_send((unsigned char *) data, len);
}

static void dbg_send_error(unsigned char errcode, unsigned char id)
{
  dbg_send_packet(errcode, id, NULL, 0);
}

static int dcg_recv_packet(struct dbghdr *hdr, void *data)
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

void dbg_enter()
{
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
