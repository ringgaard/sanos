//
// limits.h
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Implementation dependent limits 
//

#ifndef LIMITS_H
#define LIMITS_H


#define CHAR_BIT      8			// Number of bits in a char
#define SCHAR_MIN   (-128)		// Minimum signed char value
#define SCHAR_MAX     127		// Maximum signed char value
#define UCHAR_MAX     0xff		// Maximum unsigned char value

#ifndef _CHAR_UNSIGNED
#define CHAR_MIN    SCHAR_MIN		// Mimimum char value
#define CHAR_MAX    SCHAR_MAX		// Maximum char value
#else
#define CHAR_MIN      0
#define CHAR_MAX    UCHAR_MAX
#endif

#define MB_LEN_MAX    2			// Max. # bytes in multibyte char
#define SHRT_MIN    (-32768)		// Minimum (signed) short value
#define SHRT_MAX      32767		// Maximum (signed) short value
#define USHRT_MAX     0xffff		// Maximum unsigned short value
#define INT_MIN     (-2147483647 - 1)	// Minimum (signed) int value
#define INT_MAX       2147483647	// Maximum (signed) int value
#define UINT_MAX      0xffffffff	// Maximum unsigned int value
#define LONG_MIN    (-2147483647L - 1)	// Minimum (signed) long value
#define LONG_MAX      2147483647L	// Maximum (signed) long value
#define ULONG_MAX     0xffffffffUL	// Maximum unsigned long value

#if     _INTEGRAL_MAX_BITS >= 8
#define _I8_MIN     (-127i8 - 1)	// Minimum signed 8 bit value
#define _I8_MAX       127i8		// Maximum signed 8 bit value
#define _UI8_MAX      0xffui8		// Maximum unsigned 8 bit value
#endif

#if     _INTEGRAL_MAX_BITS >= 16
#define _I16_MIN    (-32767i16 - 1)	// Minimum signed 16 bit value
#define _I16_MAX      32767i16		// Maximum signed 16 bit value
#define _UI16_MAX     0xffffui16	// Maximum unsigned 16 bit value
#endif

#if     _INTEGRAL_MAX_BITS >= 32
#define _I32_MIN    (-2147483647i32 - 1) // Minimum signed 32 bit value
#define _I32_MAX      2147483647i32	// Maximum signed 32 bit value
#define _UI32_MAX     0xffffffffui32	// Maximum unsigned 32 bit value
#endif

#if     _INTEGRAL_MAX_BITS >= 64
#define _I64_MIN    (-9223372036854775807i64 - 1)
#define _I64_MAX      9223372036854775807i64
#define _UI64_MAX     0xffffffffffffffffui64
#endif

#if     _INTEGRAL_MAX_BITS >= 128
#define _I128_MIN   (-170141183460469231731687303715884105727i128 - 1)
#define _I128_MAX     170141183460469231731687303715884105727i128
#define _UI128_MAX    0xffffffffffffffffffffffffffffffffui128
#endif

#ifdef  _POSIX_

#define _POSIX_ARG_MAX      4096
#define _POSIX_CHILD_MAX    6
#define _POSIX_LINK_MAX     8
#define _POSIX_MAX_CANON    255
#define _POSIX_MAX_INPUT    255
#define _POSIX_NAME_MAX     14
#define _POSIX_NGROUPS_MAX  0
#define _POSIX_OPEN_MAX     16
#define _POSIX_PATH_MAX     255
#define _POSIX_PIPE_BUF     512
#define _POSIX_SSIZE_MAX    32767
#define _POSIX_STREAM_MAX   8
#define _POSIX_TZNAME_MAX   3

#define ARG_MAX             14500
#define LINK_MAX            1024
#define MAX_CANON           _POSIX_MAX_CANON
#define MAX_INPUT           _POSIX_MAX_INPUT
#define NAME_MAX            255
#define NGROUPS_MAX         16
#define OPEN_MAX            32
#define PATH_MAX            512
#define PIPE_BUF            _POSIX_PIPE_BUF
#define SSIZE_MAX           _POSIX_SSIZE_MAX
#define STREAM_MAX          20
#define TZNAME_MAX          10

#endif

#endif
