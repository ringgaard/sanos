//
// fpu.h
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Floating point unit
//

#ifndef FPU_H
#define FPU_H

struct fpu 
{
  unsigned long ocw;
  unsigned long osw;
  unsigned long otw;
  unsigned long oip;
  unsigned long ocs;
  unsigned long ooo;
  unsigned long oos;
  unsigned long ost[20];
};

void fpu_enable(struct fpu *state);
void fpu_disable(struct fpu *state);

void init_fpu();

#endif
