//
// verinfo.h
//
// Module version information
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

#ifndef VERINFO_H
#define VERINFO_H

#define VER_SIGNATURE 0xFEEF04BD
#define VER_FORMAT    0x00010000

//
// Version file flags
//

#define VER_FLAG_DEBUG          0x00000001L
#define VER_FLAG_PRERELEASE     0x00000002L
#define VER_FLAG_PATCHED        0x00000004L
#define VER_FLAG_PRIVATEBUILD   0x00000008L
#define VER_FLAG_INFOINFERRED   0x00000010L
#define VER_FLAG_SPECIALBUILD   0x00000020L

//
// File OS 
//

#define VER_OS_UNKNOWN          0x00000000L
#define VER_OS_DOS              0x00010000L
#define VER_OS_OS216            0x00020000L
#define VER_OS_OS232            0x00030000L
#define VER_OS_NT               0x00040000L
#define VER_OS_SANOS            0x00080000L

//
// File type
//

#define VER_TYPE_UNKNOWN        0x00000000L
#define VER_TYPE_APP            0x00000001L
#define VER_TYPE_DLL            0x00000002L
#define VER_TYPE_DRV            0x00000003L
#define VER_TYPE_FONT           0x00000004L
#define VER_TYPE_VXD            0x00000005L
#define VER_TYPE_STATIC_LIB     0x00000007L
#define VER_TYPE_KERNEL         0x00000010L

//
// File subtype for drivers
//

#define VER_DRV_UNKNOWN         0x00000000L
#define VER_DRV_PRINTER         0x00000001L
#define VER_DRV_KEYBOARD        0x00000002L
#define VER_DRV_LANGUAGE        0x00000003L
#define VER_DRV_DISPLAY         0x00000004L
#define VER_DRV_MOUSE           0x00000005L
#define VER_DRV_NETWORK         0x00000006L
#define VER_DRV_SYSTEM          0x00000007L
#define VER_DRV_INSTALLABLE     0x00000008L
#define VER_DRV_SOUND           0x00000009L
#define VER_DRV_COMM            0x0000000AL
#define VER_DRV_INPUTMETHOD     0x0000000BL

#ifdef  __cplusplus
extern "C" {
#endif

struct verinfo *get_version_info(hmodule_t hmod);
int get_version_value(hmodule_t hmod, char *value, char *buf, int size);

#ifdef  __cplusplus
}
#endif

#endif
