//
// stdlib.h
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Standard library routines
//

#ifndef STDLIB_H
#define STDLIB_H

int atoi(const char *s);
int parse_args(char *args, char **argv);
void free_args(int argc, char **argv);

#endif