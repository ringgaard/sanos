//
// stdlib.h
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Standard library routines
//

#ifndef STDLIB_H
#define STDLIB_H

int parse_args(char *args, char **argv);
void free_args(int argc, char **argv);

char *get_option(char *opts, char *name, char *buffer, int size, char *defval);
int get_num_option(char *opts, char *name, int defval);

int abs(int number);

#endif