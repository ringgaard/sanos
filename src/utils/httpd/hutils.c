//
// hutils.c
//
// HTTP utility routines
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
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <inifile.h>

static const char hex[] = "0123456789abcdef";

char *getstrconfig(struct section *cfg, char *name, char *defval) {
  char *val;

  if (!cfg) return defval;
  val = find_property(cfg, name);
  if (!val) return defval;
  return val;
}

int getnumconfig(struct section *cfg, char *name, int defval) {
  char *val;

  if (!cfg) return defval;
  val = find_property(cfg, name);
  if (!val) return defval;
  return atoi(val);
}

static int hexdigit(int x) {
  return (x <= '9') ? x - '0' : (x & 7) + 9;
}

int decode_url(char *from, char *to) {
  char c, x1, x2;

  while ((c = *from++) != 0) {
    if (c == '%') {
      x1 = *from++;
      if (!isxdigit(x1)) {
        errno = EINVAL;
        return -1;
      }
      x2 = *from++;
      if (!isxdigit(x2)) {
        errno = EINVAL;
        return -1;
      }
      if (x1 == 0 && x2 == 0) {
        errno = EINVAL;
        return -1;
      }
      *to++ = (hexdigit(x1) << 4) + hexdigit(x2);
    } else {
      *to++ = c;
    }
  }

  *to = 0;
  return 0;
}

void encode_url(const char *from, char *to) {
  char c;

  while ((c = *from++) != 0) {
    switch (c) {
      case '%':
      case ' ':
      case '?':
      case '+':
      case '&':
        *to++ = '%';
        *to++ = hex[(c >> 4) & 15];
        *to++ = hex[c & 15];
        break;

      default:
        *to++ = c;
        break;
    }
  }
  *to = 0;
}

time_t timerfc(char *s) {
  struct tm tm;
  char month[3];
  char c;
  unsigned n;
  char flag;
  char state;
  char isctime;
  enum { D_START, D_END, D_MON, D_DAY, D_YEAR, D_HOUR, D_MIN, D_SEC };

  tm.tm_sec = 60;
  tm.tm_min = 60;
  tm.tm_hour = 24;
  tm.tm_mday = 32;
  tm.tm_year = 1969;
  isctime = 0;
  month[0] = 0;
  state = D_START;
  n = 0;
  flag = 1;
  while (*s && state != D_END) {
    c = *s++;
    switch (state) {
      case D_START:
        if (c == ' ') {
          state = D_MON;
          isctime = 1;
        } else if (c == ',') {
          state = D_DAY;
        }
        break;

      case D_MON:
        if (isalpha(c)) {
          if (n < 3) month[n++] = c;
        } else {
          if (n < 3) return -1;
          n = 0;
          state = isctime ? D_DAY : D_YEAR;
        }
        break;

      case D_DAY:
        if (c == ' ' && flag) {
        } else if (isdigit(c)) {
          flag = 0;
          n = 10 * n + (c - '0');
        } else {
          tm.tm_mday = n;
          n = 0;
          state = isctime ? D_HOUR : D_MON;
        }
        break;

      case D_YEAR:
        if (isdigit(c)) {
          n = 10 * n + (c - '0');
        } else {
          tm.tm_year = n;
          n = 0;
          state = isctime ? D_END : D_HOUR;
        }
        break;

      case D_HOUR:
        if (isdigit(c)) {
          n = 10 * n + (c - '0');
        } else {
          tm.tm_hour = n;
          n = 0;
          state = D_MIN;
        }
        break;

      case D_MIN:
        if (isdigit(c)) {
          n = 10 * n + (c - '0');
        } else {
          tm.tm_min = n;
          n = 0;
          state = D_SEC;
        }
        break;

      case D_SEC:
        if (isdigit(c)) {
          n = 10 * n + (c - '0');
        } else {
          tm.tm_sec = n;
          n = 0;
          state = isctime ? D_YEAR : D_END;
        }
        break;
    }
  }

  switch (month[0]) {
    case 'A': tm.tm_mon = (month[1] == 'p') ? 4 : 8; break;
    case 'D': tm.tm_mon = 12; break;
    case 'F': tm.tm_mon = 2; break;
    case 'J': tm.tm_mon = (month[1] == 'a') ? 1 : ((month[2] == 'l') ? 7 : 6); break;
    case 'M': tm.tm_mon = (month[2] == 'r') ? 3 : 5; break;
    case 'N': tm.tm_mon = 11; break;
    case 'O': tm.tm_mon = 10; break;
    case 'S': tm.tm_mon = 9; break;
    default: return -1;
  }
  if (tm.tm_year <= 100) tm.tm_year += (tm.tm_year < 70) ? 2000 : 1900;

  tm.tm_year -= 1900;
  tm.tm_mon--;
  tm.tm_mday--;

  return mktime(&tm);
}

char *rfctime(time_t t, char *buf) {
  struct tm *tm;

  tm = gmtime(&t);
  if (!tm) return NULL;
  strftime(buf, 31, "%a, %d %b %Y %H:%M:%S GMT", tm);
  return buf;
}
