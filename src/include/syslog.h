//
// syslog.h
//
// System error logging
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

#ifndef SYSLOG_H
#define SYSLOG_H

#include <sys/types.h>

//
// Option flags for openlog
//

#define LOG_PID         0x01
#define LOG_CONS        0x02
#define LOG_ODELAY      0x04
#define LOG_NDELAY      0x08
#define LOG_NOWAIT      0x10
#define LOG_PERROR      0x20

//
// Arguments to setlogmask
//

#define LOG_MASK(pri)   (1 << (pri))            // Mask for one priority
#define LOG_UPTO(pri)   ((1 << ((pri)+1)) - 1)  // All priorities through pri

//
// Logging facility codes
//

#ifdef LOG_KERN

#define LOG_KERN        (0<<3)  // Kernel messages
#define LOG_USER        (1<<3)  // Random user-level messages
#define LOG_MAIL        (2<<3)  // Mail system
#define LOG_DAEMON      (3<<3)  // System daemons
#define LOG_AUTH        (4<<3)  // Security/authorization messages
#define LOG_SYSLOG      (5<<3)  // Messages generated internally by syslogd
#define LOG_LPR         (6<<3)  // Line printer subsystem
#define LOG_NEWS        (7<<3)  // Network news subsystem
#define LOG_UUCP        (8<<3)  // UUCP subsystem
#define LOG_CRON        (9<<3)  // Clock daemon
#define LOG_AUTHPRIV    (10<<3) // Security/authorization messages (private)
#define LOG_FTP         (11<<3) // FTP daemon

#define LOG_LOCAL0      (16<<3) // Reserved for local use
#define LOG_LOCAL1      (17<<3) // Reserved for local use
#define LOG_LOCAL2      (18<<3) // Reserved for local use
#define LOG_LOCAL3      (19<<3) // Reserved for local use
#define LOG_LOCAL4      (20<<3) // Reserved for local use
#define LOG_LOCAL5      (21<<3) // Reserved for local use
#define LOG_LOCAL6      (22<<3) // Reserved for local use
#define LOG_LOCAL7      (23<<3) // Reserved for local use

#endif

#define LOG_FACMASK     0x03f8
#define LOG_FAC(p)      ((p) & LOG_FACMASK)

//
// Logging priority codes
//

#ifdef LOG_EMERG

#define LOG_EMERG       0       // System is unusable
#define LOG_ALERT       1       // Action must be taken immediately
#define LOG_CRIT        2       // Critical conditions
#define LOG_ERR         3       // Error conditions
#define LOG_WARNING     4       // Warning conditions
#define LOG_NOTICE      5       // Normal but significant condition
#define LOG_INFO        6       // Informational
#define LOG_DEBUG       7       // Debug-level messages

#endif

#define LOG_PRIMASK             0x07    
#define LOG_PRI(p)              ((p) & LOG_PRIMASK)
#define LOG_MAKEPRI(fac, pri)   ((fac) | (pri))

#ifdef  __cplusplus
extern "C" {
#endif

osapi void openlog(char *ident, int option, int facility);
osapi void closelog();
osapi int setlogmask(int mask);

osapi void syslog(int pri, const char *fmt, ...);
osapi void vsyslog(int pri, const char *fmt, va_list args);

#ifdef  __cplusplus
}
#endif

#endif
