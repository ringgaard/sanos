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

#endif
