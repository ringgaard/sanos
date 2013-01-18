//
// math.c
//
// Math routines
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

#include <os.h>
#include <math.h>
#include <float.h>

struct dblval {
  unsigned int manl:32;
  unsigned int manh:20;
  unsigned int exp:11;
  unsigned int sign:1;
};

int isinf(double x)  {
  unsigned __int64 *val = (unsigned __int64 *) &x;

  return *val == 0x7ff0000000000000 ? 1 : *val == 0xfff0000000000000 ? -1 : 0;
}

int isnan(double x) {
  return x != x;
}

int isfinite(double x) {
  unsigned short *val = (unsigned short *) &x;

  return (val[3] & 0x7ff0) != 0x7ff0;
}

int _fpclass(double x) {
  struct dblval *d = (struct dblval *) &x;
  
  if (d->exp == 0) {
    if (d->manh == 0 && d->manl == 0) {
      if (d->sign == 0) {
        return _FPCLASS_NZ;
      } else {
        return _FPCLASS_PZ;
      }
    } else {
      if (d->sign == 0) {
        return _FPCLASS_ND;
      } else {
        return _FPCLASS_PD;
      }
    }
  }

  if (d->exp == 0x7ff) {
    if (d->manh == 0 && d->manl == 0) {
      if (d->sign == 0) {
        return _FPCLASS_NINF;
      } else {
        return _FPCLASS_PINF;
      }
    } else if (d->manh == 0 && d->manl != 0) {
      return _FPCLASS_QNAN;
    } else {
      return _FPCLASS_SNAN;
    }
  }

  return 0;
}

int fpclassify(double x) {
  struct dblval *d = (struct dblval *) &x;

  if (d->exp == 0) {
    if ((d->manl | d->manh) == 0) return FP_ZERO;
    return FP_SUBNORMAL;
  }
  
  if (d->exp == 0x7ff) {
    if ((d->manl | d->manh) == 0) return FP_INFINITE;
    return FP_NAN;  
  }
  
  return FP_NORMAL;
}

int isless(double x, double y) {
  return isfinite(x) && isfinite(y) ? x < y : 0;
}

int isgreater(double x, double y) {
  return isfinite(x) && isfinite(y) ? x > y : 0;
}

double round(double x) {
  double t;

  if (!isfinite(x)) return x;

  if (x >= 0.0) {
    t = ceil(x);
    if (t - x > 0.5) t -= 1.0;
    return t;
  } else {
    t = ceil(-x);
    if (t + x > 0.5) t -= 1.0;
    return -t;
  }
}

double hypot(double x, double y) {
  if (x < 0.0) x = -x;
  if (y < 0.0) y = -y;
  if (y == 0) return x;
  if (x < y) {
    double tmp = x;
    x = y;
    y = tmp;
  }
  if (y == 0) return x;
  y /= x;
  return x * sqrt(1.0 + y * y);  
}

