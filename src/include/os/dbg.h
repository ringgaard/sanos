//
// dbg.h
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Remote debugging support
//

#ifndef DBG_H
#define DBG_H

void dumpregs(struct context *ctxt);

void dbg_enter(struct context *ctxt, void *addr);

void dbg_notify_create_thread(struct thread *t, void *startaddr);
void dbg_notify_exit_thread(struct thread *t);
void dbg_notify_load_module(hmodule_t hmod);
void dbg_notify_unload_module(hmodule_t hmod);
void dbg_output(char *msg);

#endif
