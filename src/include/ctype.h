//
// ctype.h
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Character types
//

#ifndef CTYPE_H
#define CTYPE_H

//
// Character types
//

#define _UPPER          0x1     // Upper case letter
#define _LOWER          0x2     // Lower case letter
#define _DIGIT          0x4     // Digit[0-9]
#define _SPACE          0x8     // Tab, carriage return, newline, vertical tab or form feed
#define _PUNCT          0x10    // Punctuation character
#define _CONTROL        0x20    // Control character
#define _BLANK          0x40    // Space char
#define _HEX            0x80    // Hexadecimal digit

#define _LEADBYTE       0x8000                      // Multibyte leadbyte
#define _ALPHA          (0x0100 | _UPPER| _LOWER)  // Alphabetic character


extern unsigned short *_pctype; // pointer to table for char's

int _isctype(int c, int mask);

#ifdef NO_CTYPE_MACROS

int isspace(int c);
int isupper(int c);
int islower(int c);
int isdigit(int c);
int isxdigit(int c);
int ispunct(int c);
int isalpha(int c);
int isalnum(int c);
int isprint(int c);
int isgraph(int c);
int iscntrl(int c);

int toupper(int c);
int tolower(int c);

#else

#define isalpha(c)     (_pctype[c] & (_UPPER | _LOWER))
#define isupper(c)     (_pctype[c] & _UPPER)
#define islower(c)     (_pctype[c] & _LOWER)
#define isdigit(c)     (_pctype[c] & _DIGIT)
#define isxdigit(c)    (_pctype[c] & _HEX)
#define isspace(c)     (_pctype[c] & _SPACE)
#define ispunct(c)     (_pctype[c] & _PUNCT)
#define isalnum(c)     (_pctype[c] & (_UPPER | _LOWER | _DIGIT))
#define isprint(c)     (_pctype[c] & (_BLANK | _PUNCT | _UPPER | _LOWER | _DIGIT))
#define isgraph(c)     (_pctype[c] & (_PUNCT | _UPPER | _LOWER | _DIGIT))
#define iscntrl(c)     (_pctype[c] & _CONTROL)

#define tolower(c)     ((c) - 'A' + 'a')
#define toupper(c)     ((c) - 'a' + 'A')

#endif

#endif
