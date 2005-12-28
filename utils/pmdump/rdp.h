//
// rdp.h
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Remote debugging protocol
//

#ifndef RDP_H
#define RDP_H

typedef unsigned long tid_t;
typedef void *hmodule_t;

//
// Context structure from os.h
//

struct context
{
  unsigned long es, ds;
  unsigned long edi, esi, ebp, ebx, edx, ecx, eax;
  unsigned long traptype;
  unsigned long errcode;

  unsigned long eip, ecs;
  unsigned long eflags;
  unsigned long esp, ess;
};

#include <seg.h>
#include <trap.h>
#include <cpu.h>
#include <dbg.h>

#define GLOBAL_PROCID      1
#define GLOBAL_HPROC       0x0123

#define MAX_DBG_CHUNKSIZE  4096
#define MAX_DBG_PACKETLEN  (MAX_DBG_CHUNKSIZE + sizeof(union dbg_body))

struct dbg_session
{
  HANDLE target;
  unsigned char next_reqid;

  struct dbg_event *event_head;
  struct dbg_event *event_tail;

  struct dbg_connect conn;

  struct dbg_hdr hdr;
  char dbg_data[MAX_DBG_PACKETLEN];
  union dbg_body *body;
};

struct dbg_session *dbg_create_session(char *port);
void dbg_close_session(struct dbg_session *s, int resume);

struct dbg_event *dbg_next_event(struct dbg_session *s);
void dbg_release_event(struct dbg_event *e);

int dbg_continue(struct dbg_session *s);

int dbg_read_memory(struct dbg_session *s, void *addr, int size, void *buffer);
int dbg_write_memory(struct dbg_session *s, void *addr, int size, void *buffer);

int dbg_get_context(struct dbg_session *s, tid_t tid, struct context *ctxt);
int dbg_set_context(struct dbg_session *s, tid_t tid, struct context *ctxt);

int dbg_suspend_threads(struct dbg_session *s, tid_t *threadids, int count);
int dbg_resume_threads(struct dbg_session *s, tid_t *threadids, int count);

int dbg_get_selector(struct dbg_session *s, int sel, struct segment *seg);

#endif
