//
// assert.h
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Assert macro
//

#ifndef ASSERT_H
#define ASSERT_H

#ifdef  DEBUG

void _assert(void *expr, void *filename, unsigned lineno);

#define assert(exp) (void) ((exp) || (_assert(#exp, __FILE__, __LINE__), 0))

#else

#define assert(exp) ((void) 0)

#endif

#endif
