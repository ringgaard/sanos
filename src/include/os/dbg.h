//
// dbg.h
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Remote debugging support
//

#ifndef DBG_H
#define DBG_H

#define DBG_SIGNATURE     0xDB
#define DRPC_VERSION      1

//
// Debugger commands
//

#define DBGCMD_CONNECT            0x00
#define DBGCMD_CONTINUE           0x01
#define DBGCMD_READ_MEMORY        0x02
#define DBGCMD_WRITE_MEMORY       0x03
#define DBGCMD_SUSPEND_THREAD     0x04
#define DBGCMD_RESUME_THREAD      0x05
#define DBGCMD_GET_THREAD_CONTEXT 0x06
#define DBGCMD_SET_THREAD_CONTEXT 0x07
#define DBGCMD_GET_SELECTOR       0x08
#define DBGCMD_GET_MODULES        0x09
#define DBGCMD_GET_THREADS        0x0A

#define DBGEVT_TRAP               0x20
#define DBGEVT_CREATE_THREAD      0x21
#define DBGEVT_EXIT_THREAD        0x22
#define DBGEVT_LOAD_MODULE        0x23
#define DBGEVT_UNLOAD_MODULE      0x24
#define DBGEVT_OUTPUT             0x25

#define DBGCMD_REPLY              0x40

#define DBGERR_VERSION            0x80
#define DBGERR_INVALIDCMD         0x81
#define DBGERR_INVALIDADDR        0x82
#define DBGERR_TOOBIG             0x83
#define DBGERR_INVALIDTHREAD      0x84
#define DBGERR_INVALIDSEL         0x85
#define DBGERR_NOCONTEXT          0x86

struct dbg_hdr
{
  unsigned char signature;
  unsigned char cmd;
  unsigned char id;
  unsigned char checksum;
  unsigned int len;
};

struct dbg_memory
{
  void *addr;
  unsigned long size;
  unsigned char data[0];
};

struct dbg_thread
{
  int count;
  tid_t threadids[0];
};

struct dbg_threadlist
{
  int count;
  struct { tid_t tid; void *tib; void *startaddr; } threads[0];
};

struct dbg_context
{
  tid_t tid;
  struct context ctxt;
};

struct dbg_selector
{
  int sel;
  struct segment seg;
};

struct dbg_module
{
  int count;
  struct { hmodule_t hmod; char **name; } mods[0];
};

struct dbg_evt_trap
{
  tid_t tid;
  unsigned long traptype;
  unsigned long errcode;
  unsigned long eip;
  void *addr;
};

struct dbg_evt_create_thread
{
  tid_t tid;
  void *tib;
  void *startaddr;
};

struct dbg_evt_exit_thread
{
  tid_t tid;
  int exitcode;
};

struct dbg_evt_load_module
{
  hmodule_t hmod;
  char **name;
};

struct dbg_evt_unload_module
{
  hmodule_t hmod;
};

struct dbg_evt_output
{
  char *msgptr;
  int msglen;
};

struct dbg_connect
{
  int version;
  
  struct dbg_evt_trap trap;
  struct dbg_evt_load_module mod;
  struct dbg_evt_create_thread thr;
};

union dbg_body
{
  char string[0];
  unsigned char data[0];
  struct dbg_connect conn;
  struct dbg_memory mem;
  struct dbg_thread thr;
  struct dbg_threadlist thl;
  struct dbg_context ctx;
  struct dbg_selector sel;
  struct dbg_module mod;

  struct dbg_evt_trap trap;
  struct dbg_evt_create_thread create;
  struct dbg_evt_exit_thread exit;
  struct dbg_evt_load_module load;
  struct dbg_evt_unload_module unload;
  struct dbg_evt_output output;
};

struct dbg_event
{
  tid_t tid;
  int type;
  union dbg_body evt;
  struct dbg_event *next;
};

#ifdef KERNEL

extern int debugging;

void dumpregs(struct context *ctxt);

void __inline dbg_break() { __asm int 3 };

void dbg_enter(struct context *ctxt, void *addr);

void dbg_notify_create_thread(struct thread *t, void *startaddr);
void dbg_notify_exit_thread(struct thread *t);
void dbg_notify_load_module(hmodule_t hmod, char **name);
void dbg_notify_unload_module(hmodule_t hmod);
void dbg_output(char *msg);

#endif

#endif
