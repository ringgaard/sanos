//
// input.c
//
// C formatted input, used by scanf, etc.
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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <sys/types.h>

#define GETCH()           (++charcount, inc(stream))
#define UNGETCH(ch)       (--charcount, uninc(ch, stream))
#define SKIPWS()          skipws(&charcount, stream)

#define RANGESETSIZE  32             // # of bytes needed to hold 256 bits
#define DECIMALPOINT  '.'
#define LEFT_BRACKET  ('[' | ('a' - 'A')) // 'lowercase' version of [

static char sbrackset[] = " \t-\r]"; // Use range-style list
static char cbrackset[] = "]";


static int hextodec(int ch) {
  return isdigit(ch) ? ch : (ch & ~('a' - 'A')) - 'A' + 10 + '0';
}

static int inc(FILE *stream) {
  return getc(stream);
}

static void uninc(int ch, FILE *stream) {
  if (ch != EOF) ungetc(ch, stream);
}

static int skipws(int *counter, FILE *stream) {
  int ch;

  while (1) {
    ch = inc(stream);
    *counter++;
    if (!isspace(ch)) return ch;
  }
}

int _input(FILE *stream, const unsigned char *format, va_list arglist) {
  char table[RANGESETSIZE];           // Which chars allowed for %[], %s
  char fltbuf[CVTBUFSIZE + 1];        // ASCII buffer for floats
  unsigned long number;               // Temp hold-value
  unsigned __int64 num64;             // Temp for 64-bit integers
  void *pointer;                      // Points to user data receptacle
  void *start;                        // Indicate non-empty string

  unsigned char *scanptr;             // For building "table" data
  int ch;

  int charcount;                      // Total number of chars read
  int comchr;                         // Holds designator type
  int count;                          // Return value. # of assignments

  int started;                        // Indicate good number
  int width;                          // Width of field
  int widthset;                       // User has specified width

  char done_flag;                     // General purpose loop monitor
  char longone;                       // 0 = short, 1 = long, 2 = long double
  int integer64;                      // 1 for 64-bit integer, 0 otherwise
  char reject;                        // %[^ABC] instead of %[ABC]
  char negative;                      // Flag for '-' detected
  char suppress;                      // Don't assign anything
  char match;                         // Flag: !0 if any fields matched
  va_list arglistsave;                // Save arglist value

  unsigned char rngch;                // Used while scanning range
  unsigned char last;                 // Also for %[a-z]
  unsigned char prevchar;             // For %[a-z]

  // count = # fields assigned
  // charcount = # chars read
  // match = flag indicating if any fields were matched

  // Note that we need both count and match.  For example, a field
  // may match a format but have assignments suppressed.  In this case,
  // match will get set, but 'count' will still equal 0.  We need to
  // distinguish 'match vs no-match' when terminating due to EOF.

  count = charcount = match = 0;

  while (*format) {
    if (isspace(*format)) {
      UNGETCH(SKIPWS()); // Put first non-space char back

      while ((isspace)(*++format));
      // Careful: isspace macro may evaluate argument more than once!
    }

    if (*format == '%') {
      number = 0;
      prevchar = 0;
      width = widthset = started = 0;
      done_flag = suppress = negative = reject = 0;
      longone = 1;
      integer64 = 0;

      while (!done_flag) {
        comchr = *++format;
        if (isdigit(comchr)) {
          ++widthset;
          width = width * 10 + (comchr - '0');
        } else {
          switch (comchr) {
            case 'F':
            case 'N':
              // Near and far pointer modifiers ignored
              break;

            case 'h':
              --longone;
              break;

            case 'I':
              if ((*(format + 1) == '6') && (*(format + 2) == '4')) {
                format += 2;
                ++integer64;
                num64 = 0;
                break;
              }
              goto default_label;

            case 'L':
            case 'l':
              ++longone;
              break;

            case '*':
              ++suppress;
              break;

            default:
default_label:
              ++done_flag;
              break;
          }
        }
      }

      if (!suppress) {
        arglistsave = arglist;
        pointer = va_arg(arglist, void *);
      }

      done_flag = 0;

      // Switch to lowercase to allow %E,%G, and to keep the switch table small
      comchr = *format | ('a' - 'A');

      if (comchr != 'n') {
        if (comchr != 'c' && comchr != LEFT_BRACKET) {
          ch = SKIPWS();
        } else {
          ch = GETCH();
        }
      }

      if (!widthset || width) {
        switch(comchr) {
          case 'c':
            if (!widthset) {
              ++widthset;
              ++width;
            }
            scanptr = cbrackset;
            --reject;
            goto scanit2;

          case 's':
            scanptr = sbrackset;
            --reject;
            goto scanit2;

          case LEFT_BRACKET:
            scanptr = (char *)(++format);

            if (*scanptr == '^') {
              ++scanptr;
              --reject;
            }
scanit2:
            memset(table, 0, RANGESETSIZE);

            if (LEFT_BRACKET == comchr) {
              if (*scanptr == ']') {
                prevchar = ']';
                ++scanptr;
                table[']' >> 3] = 1 << (']' & 7);
              }
            }

            while (*scanptr != ']') {
              rngch = *scanptr++;

              if (rngch != '-'|| !prevchar || *scanptr == ']') {
                table[(prevchar = rngch) >> 3] |= 1 << (rngch & 7);
              } else  {  
                // Handle a-z type set
                rngch = *scanptr++; // Get end of range

                if (prevchar < rngch) {
                  // %[a-z]
                  last = rngch;
                } else {
                  // %[z-a]
                  last = prevchar;
                  prevchar = rngch;
                }
                
                for (rngch = prevchar; rngch <= last; ++rngch) table[rngch >> 3] |= 1 << (rngch & 7);
                prevchar = 0;
              }
            }


            if (!*scanptr) goto error_return;      // Truncated format string

            // Scanset completed. Now read string
            if (LEFT_BRACKET == comchr) format = scanptr;
            start = pointer;

            // Execute the format directive. That is, scan input
            // characters until the directive is fulfilled, eof
            // is reached, or a non-matching character is
            // encountered.
            // 
            // It is important not to get the next character
            // unless that character needs to be tested! Other-
            // wise, reads from line-buffered devices (e.g.,
            // scanf()) would require an extra, spurious, newline
            // if the first newline completes the current format
            // directive.

            UNGETCH(ch);

            while (!widthset || width--) {
              ch = GETCH();
              if (((table[ch >> 3] ^ reject) & (1 << (ch & 7)))) {
                if (!suppress) {
                  *(char *) pointer = (char) ch;
                  pointer = (char *) pointer + 1;
                } else {
                  // Just indicate a match
                  start = (char *) start + 1;
                }
              } else {
                UNGETCH(ch);
                break;
              }
            }

            // Make sure something has been matched and, if
            // assignment is not suppressed, null-terminate
            // output string if comchr != c

            if (start != pointer) {
              if (!suppress) {
                ++count;
                if (comchr != 'c') {
                  // Null-terminate strings
                  *(char *) pointer = '\0';
                }
              }
            } else {
              goto error_return;
            }

            break;

          case 'i':
            comchr = 'd';

          case 'x':
            if (ch == '-') {
              ++negative;
              goto x_incwidth;
            } else if (ch == '+') {
x_incwidth:
              if (!--width && widthset) {
                ++done_flag;
              } else {
                ch = GETCH();
              }
            }

            if (ch == '0') {
              if ((ch = GETCH()) == 'x' || ch == 'X') {
                ch = GETCH();
                comchr = 'x';
              } else {
                ++started;
                if (comchr != 'x') {
                  comchr = 'o';
                } else {
                  // Scanning a hex number that starts
                  // with a 0. Push back the character
                  // currently in ch and restore the 0
                  UNGETCH(ch);
                  ch = '0';
                }
              }
            }
            goto getnum;

          case 'p':
            // Force %hp to be treated as %p
            longone = 1;

          case 'o':
          case 'u':
          case 'd':
            if (ch == '-') {
              ++negative;
              goto d_incwidth;
            } else if (ch == '+') {
d_incwidth:
              if (!--width && widthset) {
                ++done_flag;
              } else {
                ch = GETCH();
              }
            }
getnum:
            if (integer64) {
              while (!done_flag) {
                if (comchr == 'x') {
                  if (isxdigit(ch)) {
                    num64 <<= 4;
                    ch = hextodec(ch);
                  } else {
                    ++done_flag;
                  }
                } else if (isdigit(ch)) {
                  if (comchr == 'o') {
                    if (ch < '8') {
                      num64 <<= 3;
                    } else {
                      ++done_flag;
                    }
                  } else {
                    // comchr == 'd'
                    num64 = num64 * 10;
                  }
                } else {
                  ++done_flag;
                }

                if (!done_flag) {
                  ++started;
                  num64 += ch - '0';

                  if (widthset && !--width) {
                    ++done_flag;
                  } else {
                    ch = GETCH();
                  }
                } 
                else {
                  UNGETCH(ch);
                }
              }

              if (negative) num64 = (unsigned __int64 ) (-(__int64) num64);
            } else {
              while (!done_flag) {
                if (comchr == 'x' || comchr == 'p') {
                  if (isxdigit(ch))  {
                    number = (number << 4);
                    ch = hextodec(ch);
                  } else {
                    ++done_flag;
                  }
                } else if (isdigit(ch)) {
                  if (comchr == 'o') {
                    if (ch < '8') {
                      number = (number << 3);
                    } else {
                      ++done_flag;
                    }
                  } else {
                    // comchr == 'd'
                    number = number * 10;
                  }
                } else {
                  ++done_flag;
                }

                if (!done_flag) {
                  ++started;
                  number += ch - '0';

                  if (widthset && !--width) {
                    ++done_flag;
                  } else {
                    ch = GETCH();
                  }
                } else {
                  UNGETCH(ch);
                }
              }

              if (negative) number = (unsigned long) (-(long) number);
            }

            if (comchr == 'F') started = 0; // Expected ':' in long pointer

            if (started) {
              if (!suppress) {
                ++count;
assign_num:
                if (integer64) {
                  *(__int64 *) pointer = (unsigned __int64) num64;
                } else if (longone) {
                  *(long *) pointer = (unsigned long) number;
                } else {
                  *(short *) pointer = (unsigned short) number;
                }
              }
            } else {
              goto error_return;
            }

            break;

          case 'n':      
            // Char count, don't inc return value
            number = charcount;
            if (!suppress) goto assign_num; // Found in number code above
            break;

          case 'e':
          case 'f':
          case 'g': 
            // Scan a float
            scanptr = fltbuf;

            if (ch == '-') {
              *scanptr++ = '-';
              goto f_incwidth;
            } else if (ch == '+') {
f_incwidth:
              --width;
              ch = GETCH();
            }

            if (!widthset || width > CVTBUFSIZE) width = CVTBUFSIZE;

            // Now get integral part
            while (isdigit(ch) && width--) {
              ++started;
              *scanptr++ = (char)ch;
              ch = GETCH();
            }

            // Now check for decimal
            if (ch == DECIMALPOINT && width--) {
              ch = GETCH();
              *scanptr++ = DECIMALPOINT;

              while (isdigit(ch) && width--) {
                ++started;
                *scanptr++ = (char) ch;
                ch = GETCH();
              }
            }

            // Now check for exponent
            if (started && (ch == 'e' || ch == 'E') && width--) {
              *scanptr++ = 'e';

              if ((ch = GETCH()) == '-') {
                *scanptr++ = '-';
                goto f_incwidth2;
              } else if (ch == '+') {
f_incwidth2:
                if (!width--) {
                  ++width;
                } else {
                  ch = GETCH();
                }
              }

              while (isdigit(ch) && width--) {
                ++started;
                *scanptr++ = (char)ch;
                ch = GETCH();
              }
            }

            UNGETCH(ch);

            if (started) {
              if (!suppress) {
                double d;

                ++count;
                *scanptr = '\0';

                d = strtod(fltbuf, NULL);

                if (longone) {
                  *(double *) pointer = d;
                } else {
                  *(float *) pointer = (float) d;
                }
              }
            } else {
              goto error_return;
            }

            break;

          default:
            // Either found '%' or something else
            if ((int) *format != (int) ch) {
              UNGETCH(ch);
              goto error_return;
            } else {
              match--; // % found, compensate for inc below
            }

            if (!suppress) arglist = arglistsave;
        }

        // Matched a format field - set flag
        match++;        
      } else {  
        // Zero-width field in format string
        UNGETCH(ch);
        goto error_return;
      }

      // Skip to next char
      ++format;
    } else {
      // *format != '%'
      if ((int) *format++ != (int) (ch = GETCH())) {
        UNGETCH(ch);
        goto error_return;
      }
    }

    if ((ch == EOF) && ((*format != '%') || (*(format + 1) != 'n')))  break;
  }

error_return:

  if (ch == EOF) {
    // If any fields were matched or assigned, return count
    return (count || match) ? count : EOF;
  } else {
    return count;
  }
}
