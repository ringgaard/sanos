//
// float.c
//
// Floating point support routines
//
// Copyright (C) 2002 Michael Ringgaard. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 
// 1. Redistributions of source code must retain the above copyright 
//    notice, this list of conditions and the following disclaimer.  
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.  
// 3. Neither the name of the project nor the names of its contributors
//    may be used to endorse or promote products derived from this software
//    without specific prior written permission. 
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
// OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
// OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF 
// SUCH DAMAGE.
// 

#include "msvcrt.h"

#define HI(x)    (*(1 + (int *) &x))
#define LO(x)    (*(int *) &x)
#define HIPTR(x) *(1+(int *) x)
#define LOPTR(x) (*(int *) x)

int _isnan(double x) {
  int hx, lx;

  hx = (HI(x) & 0x7fffffff);
  lx = LO(x);
  hx |= (unsigned) (lx | (-lx)) >> 31;
  hx = 0x7ff00000 - hx;
  return ((unsigned) (hx)) >> 31;
}

double _copysign(double x, double y) {
  HI(x) = (HI(x) & 0x7fffffff) | (HI(y) & 0x80000000);
  return x;
}

int _finite(double x) {
  int hx; 
  hx = HI(x);
  return  (unsigned) ((hx & 0x7fffffff) - 0x7ff00000) >> 31;
}

unsigned int _control87(unsigned int new, unsigned int mask) {
  // TODO: map between msvcrt fp flag definitions to i387 flags
#if 0
  unsigned int fpcw;

  __asm fnstcw fpcw;
  fpcw = ((fpcw & ~mask) | (new & mask));
  __asm fldcw fpcw;

  return fpcw;
#endif
  return 0;
}

unsigned int _controlfp(unsigned int new, unsigned int mask) {
  syslog(LOG_WARNING, "_controlcp not implemented, ignored");
  return 0;
}
