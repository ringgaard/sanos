//
// stdio.h
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Standard I/O routines
//

#ifndef STDIO_H
#define STDIO_H

typedef void *FILE;

#define stdin  (gettib()->in)
#define stdout (gettib()->out)
#define stderr (gettib()->err)

void printf(const char *fmt,...);
char *gets(char *buf);

#endif