//
// output.c
//
// Print formatting routines 
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
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <limits.h>
#include <math.h>

#define MAXBUFFER 512

// Flag definitions

#define FL_SIGN       0x00001   // Put plus or minus in front
#define FL_SIGNSP     0x00002   // Put space or minus in front
#define FL_LEFT       0x00004   // Left justify
#define FL_LEADZERO   0x00008   // Pad with leading zeros
#define FL_LONG       0x00010   // Long value given
#define FL_SHORT      0x00020   // Short value given
#define FL_SIGNED     0x00040   // Signed data given
#define FL_ALTERNATE  0x00080   // Alternate form requested
#define FL_NEGATIVE   0x00100   // Value is negative
#define FL_FORCEOCTAL 0x00200   // Force leading '0' for octals

static void cfltcvt(double value, char *buffer, char fmt, int precision, int capexp) {
  int decpt, sign, exp, pos;
  char *digits = NULL;
  char cvtbuf[CVTBUFSIZE];
  int magnitude;

  if (fmt == 'g') {
    digits = ecvtbuf(value, precision, &decpt, &sign, cvtbuf);
    magnitude = decpt - 1;
    if (magnitude < -4  ||  magnitude > precision - 1) {
      fmt = 'e';
      precision -= 1;
    } else {
      fmt = 'f';
      precision -= decpt;
    }
  }

  if (fmt == 'e') {
    digits = ecvtbuf(value, precision + 1, &decpt, &sign, cvtbuf);

    if (sign) *buffer++ = '-';
    *buffer++ = *digits;
    if (precision > 0) *buffer++ = '.';
    memcpy(buffer, digits + 1, precision);
    buffer += precision;
    *buffer++ = capexp ? 'E' : 'e';

    if (decpt == 0) {
      if (value == 0.0) {
        exp = 0;
      } else {
        exp = -1;
      }
    } else {
      exp = decpt - 1;
    }

    if (exp < 0) {
      *buffer++ = '-';
      exp = -exp;
    } else {
      *buffer++ = '+';
    }

    buffer[2] = (exp % 10) + '0';
    exp = exp / 10;
    buffer[1] = (exp % 10) + '0';
    exp = exp / 10;
    buffer[0] = (exp % 10) + '0';
    buffer += 3;
  } else if (fmt == 'f') {
    digits = fcvtbuf(value, precision, &decpt, &sign, cvtbuf);
    if (sign) *buffer++ = '-';
    if (*digits) {
      if (decpt <= 0) {
        *buffer++ = '0';
        *buffer++ = '.';
        for (pos = 0; pos < -decpt; pos++) *buffer++ = '0';
        while (*digits) *buffer++ = *digits++;
      } else {
        pos = 0;
        while (*digits) {
          if (pos++ == decpt) *buffer++ = '.';
          *buffer++ = *digits++;
        }
      }
    } else {
      *buffer++ = '0';
      if (precision > 0) {
        *buffer++ = '.';
        for (pos = 0; pos < precision; pos++) *buffer++ = '0';
      }
    }
  }

  *buffer = '\0';
}

static void forcdecpt(char *buffer) {
  while (*buffer) {
    if (*buffer == '.') return;
    if (*buffer == 'e' || *buffer == 'E') break;
    buffer++;
  }

  if (*buffer) {
    int n = strlen(buffer);
    while (n > 0)  {
      buffer[n + 1] = buffer[n];
      n--;
    }

    *buffer = '.';
  } else {
    *buffer++ = '.';
    *buffer = '\0';
  }
}

static void cropzeros(char *buffer) {
  char *stop;

  while (*buffer && *buffer != '.') buffer++;
  if (*buffer++) {
    while (*buffer && *buffer != 'e' && *buffer != 'E') buffer++;
    stop = buffer--;
    while (*buffer == '0') buffer--;
    if (*buffer == '.') buffer--;
    while (*++buffer = *stop++);
  }
}

int _output(FILE *stream, const char *format, va_list args) {
  char *text;
  int textlen;
  int ch;
  int cnt;
  int radix, hexadd, padding;
  double fltval;
  char buffer[MAXBUFFER];
  char *fmt = (char *) format;

  cnt = 0;
  while (*fmt) {
    // Initialize formating state
    char prefix[2];
    int prefixlen = 0;
    int flags = 0;
    int precision = -1;
    int qualifier = -1;
    int width = 0;
    int capexp = 0;

    // Go forward until next format specifier or end of string
    while (*fmt != 0 && *fmt != '%') {
      //if ((stream->flag & _IOCRLF) && *fmt == '\n') putc('\r', stream);
      putc(*fmt, stream);
      fmt++;
      cnt++;
    }
    if (*fmt++ == 0) break;

    // Check for flags
    switch (*fmt++) {
      case '-': flags |= FL_LEFT;      break;  // '-' => Left justify
      case '+': flags |= FL_SIGN;      break;  // '+' => Force sign indicator
      case ' ': flags |= FL_SIGNSP;    break;  // ' ' => Force sign or space
      case '#': flags |= FL_ALTERNATE; break;  // '#' => Alternate form
      case '0': flags |= FL_LEADZERO;  break;  // '0' => Pad with leading zeros
      default : fmt--; // No format flag
    }

    // Check for width value
    if (*fmt == '*') {
      // Get width from arg list
      width = va_arg(args, int);
      if (width < 0) {
        // ANSI says neg fld width means '-' flag and pos width
        flags |= FL_LEFT;
        width = -width;
      }
      fmt++;
    } else {
      // Get field width
      while (*fmt >= '0' && *fmt <= '9') width = width * 10 + (*fmt++ - '0');
    }
    
    // Check for precision
    if (*fmt == '.') {
      // Zero the precision, since dot with no number means 0, not default, according to ANSI
      precision = 0;
      fmt++;

      if (*fmt == '*') {
        // Get precision from arg list
        precision = va_arg(args, int);
        if (precision < 0) precision = -1; // Negative precision means default
        fmt++;
      } else {
        // Get precision
        while (*fmt >= '0' && *fmt <= '9') precision = precision * 10 + (*fmt++ - '0');
      }
    }
    
    // Get the conversion qualifier
    if (*fmt == 'h' || *fmt == 'l' || *fmt == 'L') {
      qualifier = *fmt;
      fmt++;
    } else if (*fmt == 'I' && *(fmt + 1) == '6' && *(fmt + 2) == '4') {
      qualifier = 'I';
      fmt += 3;
    }

    // Format output according to type specifier    
    ch = *fmt++;
    switch (ch) {
      case 'c': // One character
        buffer[0] = va_arg(args, char);
        text = buffer;
        textlen = 1;
        break;
     
      case 's': // Zero terminated character string
        text = va_arg(args, char *);
        if (text == NULL) {
          text = "(null)";
          textlen = 6;
        } else {
          char *p;

          int maxlen = precision == -1 ? INT_MAX : precision;
          textlen = 0;
          p = text;
          while (maxlen-- && *p) p++;
          textlen = p - text;
        }
        break;

      case 'n': // Number of characters successfully written so far
        if (qualifier == 'l') {
          long *ip = va_arg(args, long *);
          *ip = cnt;
        } else {
          int *ip = va_arg(args, int *);
          *ip = cnt;
        }
        continue;

      case 'd': // Signed decimal integer
      case 'i':
        flags |= FL_SIGNED;
        radix = 10;
        goto case_int;

      case 'u': // Unsigned decimal integer
        radix = 10;
        goto case_int;

      case 'p': // Pointer
        precision = 2 * sizeof(void *);  // Number of hex digits needed
        // Drop through to hex formatting

      case 'X': // Unsigned upper hex integer
         hexadd = 'A' - '9' - 1; // Set hexadd for uppercase hex
         goto case_hex;

      case 'x': // Unsigned lower hex integer
         hexadd = 'a' - '9' - 1; // Set hexadd for lowercase hex
         // Drop through to hex formatting
         
      case_hex:
         radix = 16;
         if (flags & FL_ALTERNATE) {
           // Alternate form means '0x' prefix
           prefix[0] = '0';
           prefix[1] = 'x' - 'a' + '9' + 1 + hexadd;  // 'x' or 'X'
           prefixlen = 2;
         }
         goto case_int;

      case 'o': // Unsigned octal integer
        radix = 8;
        if (flags & FL_ALTERNATE) {
          // Alternate form means force a leading 0
          flags |= FL_FORCEOCTAL;
        }
        // Drop through to int formatting

      case_int: {
        unsigned int number;       // Number to convert (32 bit)
        unsigned __int64 number64; // Number to convert (64 bit)
        int digit;                 // ASCII value of digit

        // Read argument and check for negative
        if (qualifier == 'I') {
          if (flags & FL_SIGNED) {
            __int64 n = va_arg(args, __int64);
            if (n < 0) {
              number64 = -n;
              flags |= FL_NEGATIVE;
            } else {
              number64 = n;
            }
          } else {
            number64 = va_arg(args, unsigned __int64);
          }
        } else if (qualifier == 'l' || qualifier == 'L') {
          if (flags & FL_SIGNED) {
            long n = va_arg(args, long);
            if (n < 0) {
              number = -n;
              flags |= FL_NEGATIVE;
            } else {
              number = n;
            }
          } else {
            number = va_arg(args, unsigned long);
          }
        } else if (qualifier == 'h') {
          if (flags & FL_SIGNED) {
            short n = va_arg(args, short);
            if (n < 0) {
              number = -n;
              flags |= FL_NEGATIVE;
            } else {
              number = n;
            }
          } else {
            number = va_arg(args, unsigned short);
          }
        } else if (flags & FL_SIGNED) {
          int n = va_arg(args, int);
          if (n < 0) {
            number = -n;
            flags |= FL_NEGATIVE;
          } else {
            number = n;
          }
        } else {
          number = va_arg(args, unsigned int);
        }

        // Check precision value for default; non-default turns off 0 flag, according to ANSI
        if (precision < 0) {
          precision = 1;  // Default precision
        } else {
          flags &= ~FL_LEADZERO;
        }

        // Convert data to ASCII -- note if precision is zero and number is zero, we get no digits at all
        text = &buffer[MAXBUFFER - 1]; // Last digit at end of buffer
        if (qualifier == 'I') {
          // Check if data is 0; if so, turn off hex prefix
          if (number64 == 0) prefixlen = 0;

          while (precision-- > 0 || number64 != 0) {
            digit = (int) (number64 % radix) + '0';
            number64 /= radix; // Reduce number
            if (digit > '9') digit += hexadd;
            *text-- = digit;
          }
        } else {
          // Check if data is 0; if so, turn off hex prefix
          if (number == 0) prefixlen = 0;

          while (precision-- > 0 || number != 0) {
            digit = (int) (number % radix) + '0';
            number /= radix; // Reduce number
            if (digit > '9') digit += hexadd;
            *text-- = digit;
          }
        }

        textlen = &buffer[MAXBUFFER - 1] - text; // Compute length of number
        text++; // Text points to first digit now

        // Force a leading zero if FORCEOCTAL flag set */
        if ((flags & FL_FORCEOCTAL) && (*text != '0' || textlen == 0)) {
          *--text = '0';
          textlen++;
        }
        break;
      }
    
      case 'E':
      case 'G':
        capexp = 1;      // Capitalize exponent
        ch += 'a' - 'A'; // Convert format char to lower
        // Drop through to float formatting

      case 'e':
      case 'f':
      case 'g':
        flags |= FL_SIGNED; // Floating point is signed conversion
        text = buffer;      // Put result in buffer

        // Get floating point value
        fltval = va_arg(args, double);

        if (isfinite(fltval)) {
          // Compute the precision value
          if (precision < 0) {
            precision = 6; // Default precision: 6
          } else if (precision == 0 && ch == 'g') {
            precision = 1; // ANSI specified
          }

          // Convert floating point number to text
          cfltcvt(fltval, text, ch, precision, capexp);

          // '#' and precision == 0 means force a decimal point
          if ((flags & FL_ALTERNATE) && precision == 0) forcdecpt(text);

          // 'g' format means crop zero unless '#' given
          if (ch == 'g' && !(flags & FL_ALTERNATE)) cropzeros(text);
        } else if (isnan(fltval)) {
          strcpy(buffer, "NaN");
        } else if (fltval == HUGE_VAL) {
          strcpy(buffer, "+INF");
        } else if (fltval == -HUGE_VAL) {
          strcpy(buffer, "-INF");
        } else {
          strcpy(buffer, "?FLT?");
        }

        // Check if result was negative, save '-' for later and point to positive part (this is for '0' padding)
        if (*text == '-') {
          flags |= FL_NEGATIVE;
          text++;
        }
        textlen = strlen(text); // Compute length of text
        break;

      case '%':
        text = "%";
        textlen = 1;
        break;
        
      default:
        text = "";
        textlen = 0;
    }

    // Check for signs
    if (flags & FL_SIGNED) {
      if (flags & FL_NEGATIVE) {
        prefix[0] = '-';
        prefixlen = 1;
      } else if (flags & FL_SIGN) {
        prefix[0] = '+';
        prefixlen = 1;
      } else if (flags & FL_SIGNSP) {
        prefix[0] = ' ';
        prefixlen = 1;
      }
    }

    // Calculate amount of padding -- might be negative, but this will just mean zero
    padding = width - textlen - prefixlen;

    // Put out the padding, prefix, and text, in the correct order
    if (!(flags & (FL_LEFT | FL_LEADZERO))) {
      // Pad on left with blanks
      while (padding > 0) {
        putc(' ', stream);
        cnt++;
        padding--;
      }
    }

    // Write prefix
    if (prefixlen > 0) {
      int n;

      for (n = 0; n < prefixlen; n++) {
        putc(prefix[n], stream);
        cnt++;
      }
    }

    // Put out leading zeroes
    if ((flags & FL_LEADZERO) && !(flags & FL_LEFT)) {
      // Write leading zeros
      while (padding > 0) {
        putc('0', stream);
        cnt++;
        padding--;
      }
    }

    // Put out text
    while (textlen > 0) {
      putc(*text, stream);
      text++;
      textlen--;
      cnt++;
    }

    // Put out trailling blanks
    if (flags & FL_LEFT) {
      // Pad on right with blanks
      while (padding > 0) {
        putc(' ', stream);
        cnt++;
        padding--;
      }
    }
  }
  
  return cnt;
}
