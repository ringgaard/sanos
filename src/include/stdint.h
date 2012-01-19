//
// stdint.h
//
// Defines integer types
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

#ifndef STDINT_H
#define STDINT_H

//
// Integer types
//

#ifndef _INT_T_DEFINED
#define _INT_T_DEFINED
typedef signed char int8_t;
typedef short int16_t;
typedef int int32_t;
typedef __int64 int64_t;

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned __int64 uint64_t;
#endif

//
// Types for void * pointers
//

#ifndef _INTPTR_T_DEFINED
#define _INTPTR_T_DEFINED
typedef long intptr_t;
typedef unsigned long uintptr_t;
#endif

//
// Largest integral types
//

#ifndef _INTMAX_T_DEFINED
#define _INTMAX_T_DEFINED
typedef __int64 intmax_t;
typedef unsigned __int64 uintmax_t;
#endif

//
// Limits of integral types
//

#define INT8_MIN     (-128)
#define INT16_MIN    (-32767 - 1)
#define INT32_MIN    (-2147483647 - 1)
#define INT64_MIN    (-9223372036854775807i64 - 1)

#define INT8_MAX     (127)
#define INT16_MAX    (32767)
#define INT32_MAX    (2147483647)
#define INT64_MAX    (9223372036854775807i64)

#define UINT8_MAX    (0xffui8)
#define UINT16_MAX   (0xffffui16)
#define UINT32_MAX   (0xffffffffui32)
#define UINT64_MAX   (0xffffffffffffffffui64)

#endif
