//
// syslog.c
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

#include <os.h>
#include <syslog.h>
#include <stdarg.h>
#include <string.h>
#include <inifile.h>
#include <time.h>

extern const char *_months_abbrev[]; // in time.c 

int vsprintf(char *buf, const char *fmt, va_list args);
int sprintf(char *buf, const char *fmt, ...);

unsigned long logmask = LOG_UPTO(LOG_DEBUG);

static handle_t syslogfd = -1;
static int syslogsock = -1;
static int syslogcons = 1;

void openlog(char *ident, int option, int facility) {
  struct process *proc = gettib()->proc;

  if (ident && strlen(ident) < 64) {
    if (proc->ident)  {
      char *oldident = proc->ident;
      proc->ident = strdup(ident);
      free(oldident);
    } else {
      proc->ident = strdup(ident);
    }
  }

  proc->facility = facility;
}

void closelog() {
}

int setlogmask(int mask) {
  int oldmask = logmask;
  if (mask != 0) logmask = mask;
  return oldmask;
}

static void add_to_syslog(int pri, char *msg, int msglen, char *ident, int id, int display) {
  struct peb *peb = getpeb();
  char buffer[1024];
  char pribuf[32];
  char *bufend;
  char *hostname;
  time_t now;
  struct tm tm;
  struct iovec iov[4];

  if ((logmask & LOG_MASK(LOG_PRI(pri))) == 0) return;
  
  if (*peb->hostname) {
    hostname = peb->hostname;
  } else {
    hostname = get_property(osconfig(), "os", "hostname", NULL);
    if (!hostname && peb->ipaddr.s_addr != INADDR_ANY) hostname = inet_ntoa(peb->ipaddr);
    if (!hostname) hostname = "localhost";
  }

  now = time(NULL);
  localtime_r(&now, &tm);

  sprintf(pribuf, "<%d>", pri);

  sprintf(buffer, "%s %2d %02d:%02d:%02d %s ", _months_abbrev[tm.tm_mon], tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, hostname);
  bufend = buffer + strlen(buffer);

  if (ident) {
    if (id != -1) {
      sprintf(bufend, "%s[%d]: ", ident, id);
    } else {
      sprintf(bufend, "%s: ", ident);
    }

    bufend += strlen(bufend);
  }

  iov[0].iov_base = pribuf;
  iov[0].iov_len = strlen(pribuf);

  iov[1].iov_base = buffer;
  iov[1].iov_len = bufend - buffer;

  iov[2].iov_base = msg;
  iov[2].iov_len = msglen;

  iov[3].iov_base = "\n";
  iov[3].iov_len = 1;

  if (display) writev(syslogcons, iov + 1, 3);
  if (syslogfd >= 0) writev(syslogfd, iov, 4);
  if (syslogsock >= 0) writev(syslogsock, iov, 3);
}

void vsyslog(int pri, const char *fmt, va_list args) {
  char buffer[1024];
  char *ident;
  int priority;
  int facility;
  struct tib *tib;

  priority = LOG_PRI(pri);
  facility = LOG_FAC(pri);

  if ((logmask & LOG_MASK(priority)) == 0) return;

  tib = gettib();

  if (facility == 0) {
    if (tib->proc) {
      facility = tib->proc->facility;
    } else {
      facility = LOG_USER;
    }
  }
  
  if (tib->proc && tib->proc->ident) {
    ident = tib->proc->ident;
  } else {
    ident = "thread";
  }

  vsprintf(buffer, fmt, args);

  add_to_syslog(LOG_MAKEPRI(facility, priority), buffer, strlen(buffer), ident, tib->tid, 1);
}

void syslog(int pri, const char *fmt, ...) {
  va_list args;

  va_start(args, fmt);
  vsyslog(pri, fmt, args);
}

void add_to_klog(char *msg, int msglen) {
  int pri = LOG_DEBUG;

  if (msglen >= 3 && msg[0] == '<' && msg[1] >= '0' && msg[1] <= '7' && msg[2] == '>') {
    pri = msg[1] - '0';
    msg += 3;
    msglen -= 3;
  }

  add_to_syslog(LOG_MAKEPRI(LOG_KERN, pri), msg, msglen, NULL, -1, 0);
}

void __stdcall klogd(void *arg) {
  int klog;
  char buffer[512];
  char line[512];
  int size;
  int linelen;

  klog = open("/dev/klog", O_BINARY);
  if (klog < 0) return;

  linelen = 0;
  while (1) {
    size = read(klog, buffer, sizeof buffer);
    if (size < 0) {
      break;
    } else if (size > 0) {
      int i;

      for (i = 0; i < size; i++) {
        if (linelen == sizeof(line)) {
          add_to_klog(line, linelen);
          linelen = 0;
        }

        if (buffer[i] == '\n') {
          if (linelen > 0) add_to_klog(line, linelen);
          linelen = 0;
        } else {
          line[linelen++] = buffer[i];
        }
      }
    } else {
      if (ioctl(klog, IOCTL_KLOG_WAIT, NULL, 0) < 0) break;
    }
  }

  close(klog);
}

void start_syslog() {
  char *logfn;
  char *loghost;

  logmask = LOG_UPTO(get_numeric_property(osconfig(), "os", "loglevel", LOG_DEBUG));
  syslogcons = fderr;

  logfn = get_property(osconfig(), "os", "logfile", NULL);
  if (logfn != NULL) {
    syslogfd = open(logfn, O_CREAT, S_IREAD | S_IWRITE);
    if (syslogfd >= 0) lseek(syslogfd, 0, SEEK_END);
  }

  loghost = get_property(osconfig(), "os", "loghost", NULL);
  if (loghost != NULL) {
    struct hostent *hp;

    hp = gethostbyname(loghost);
    if (hp) {
      struct sockaddr_in sin;

      memset(&sin, 0, sizeof(sin));
      sin.sin_family = AF_INET;
      memcpy(&sin.sin_addr, hp->h_addr_list[0], hp->h_length);
      sin.sin_port = htons(514);

      syslogsock = socket(AF_INET, SOCK_DGRAM, 0);
      connect(syslogsock, (struct sockaddr *) &sin, sizeof(sin));
    } else {
      syslog(LOG_ERR | LOG_SYSLOG, "unknown syslog server %s", loghost);
    }
  }

  close(beginthread(klogd, 0, NULL, 0, "syslogd", NULL));
}

void stop_syslog() {
  if (syslogfd >= 0) {
    close(syslogfd);
    syslogfd = -1;
  }

  if (syslogsock > 0) {
    close(syslogsock);
    syslogsock = -1;
  }
}
