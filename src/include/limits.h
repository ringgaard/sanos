//
// limits.h
//
// Implementation dependent limits 
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

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef LIMITS_H
#define LIMITS_H

#define PATH_MAX      255
#define ARG_MAX       131072

#define CHAR_BIT      8                 // Number of bits in a char
#define SCHAR_MIN   (-128)              // Minimum signed char value
#define SCHAR_MAX     127               // Maximum signed char value
#define UCHAR_MAX     0xff              // Maximum unsigned char value

#ifndef _CHAR_UNSIGNED
#define CHAR_MIN    SCHAR_MIN           // Mimimum char value
#define CHAR_MAX    SCHAR_MAX           // Maximum char value
#else
#define CHAR_MIN      0
#define CHAR_MAX    UCHAR_MAX
#endif

#define MB_LEN_MAX    2                  // Max. # bytes in multibyte char
#define SHRT_MIN    (-32768)             // Minimum (signed) short value
#define SHRT_MAX      32767              // Maximum (signed) short value
#define USHRT_MAX     0xffff             // Maximum unsigned short value
#define INT_MIN     (-2147483647 - 1)    // Minimum (signed) int value
#define INT_MAX       2147483647         // Maximum (signed) int value
#define UINT_MAX      0xffffffff         // Maximum unsigned int value
#define LONG_MIN    (-2147483647L - 1)   // Minimum (signed) long value
#define LONG_MAX      2147483647L        // Maximum (signed) long value
#define ULONG_MAX     0xffffffffUL       // Maximum unsigned long value

#if _INTEGRAL_MAX_BITS >= 8
#define _I8_MIN     (-127i8 - 1)         // Minimum signed 8 bit value
#define _I8_MAX       127i8              // Maximum signed 8 bit value
#define _UI8_MAX      0xffui8            // Maximum unsigned 8 bit value
#endif

#if _INTEGRAL_MAX_BITS >= 16
#define _I16_MIN    (-32767i16 - 1)      // Minimum signed 16 bit value
#define _I16_MAX      32767i16           // Maximum signed 16 bit value
#define _UI16_MAX     0xffffui16         // Maximum unsigned 16 bit value
#endif

#if _INTEGRAL_MAX_BITS >= 32
#define _I32_MIN    (-2147483647i32 - 1) // Minimum signed 32 bit value
#define _I32_MAX      2147483647i32      // Maximum signed 32 bit value
#define _UI32_MAX     0xffffffffui32     // Maximum unsigned 32 bit value
#endif

#if _INTEGRAL_MAX_BITS >= 64
#define _I64_MIN    (-9223372036854775807i64 - 1)
#define _I64_MAX      9223372036854775807i64
#define _UI64_MAX     0xffffffffffffffffui64
#endif

#endif
