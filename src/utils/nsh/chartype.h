//
// chartype.h
//
// Character classification for shell
//
// Copyright (C) 2011 Michael Ringgaard. All rights reserved.
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

#ifndef CHARTYPE_H
#define CHARTYPE_H

//
// Character classification table 
//

#define CHAR_BITS  (sizeof(unsigned char) * 8)
#define CHAR_RANGE (1 << CHAR_BITS)

extern const unsigned short ctype[CHAR_RANGE];

//
// Character classes
//

#define C_UNDEF   0x000 // Undefined character class
#define C_SPACE   0x001 // A whitespace character
#define C_DIGIT   0x002 // A numerical digit
#define C_UPPER   0x004 // An uppercase char
#define C_LOWER   0x008 // A lowercase char
#define C_NAME    0x010 // An alphanumeric or underscore
#define C_SPCL    0x020 // Special parameter char
#define C_CTRL    0x040 // Control operator char
#define C_ESC     0x080 // Characters to escape before expansion
#define C_DESC    0x100 // Escapable characters in double quotation mode
#define C_QUOT    0x200 // Quotes

//
// Character class macros 
//

#define is_ctype(c, mask) (ctype[(int)(unsigned char)(c)] & (mask))

// Matches [ \t\n]
#define is_space(c) is_ctype((c), C_SPACE)

// Matches [0-9]
#define is_digit(c) is_ctype((c), C_DIGIT)

// Matches [A-Z]
#define is_upper(c) is_ctype((c), C_UPPER)

// Matches [a-z]
#define is_lower(c) is_ctype((c), C_LOWER)

// Matches [a-zA-Z]
#define is_alpha(c) is_ctype((c), C_LOWER | C_UPPER)

// Matches [a-zA-Z0-9]
#define is_alnum(c) is_ctype((c), C_LOWER | C_UPPER | C_DIGIT)

// Matches [@#?!$0-*] (special parameters)
#define is_spcl(c)  is_ctype((c), C_SPCL)

// Matches [;&|()] (control operator)
#define is_ctrl(c)  is_ctype((c), C_CTRL)

// Matches [*?[]\] (glob expansion escape stuff)
#define is_esc(c)   is_ctype((c), C_ESC)

// Matches [$`"] (double quote escape stuff)
#define is_desc(c)  is_ctype((c), C_DESC)

// Matches alpha, digit or underscore
#define is_name(c)  is_ctype((c), C_LOWER | C_UPPER | C_DIGIT | C_NAME)

// Matches either alpha, digit or underscore or special parameter
#define is_param(c) is_ctype((c), C_LOWER | C_UPPER | C_DIGIT | C_NAME | C_SPCL)

#endif
