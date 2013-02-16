//
// touch.c
//
// Change file access and modification times
//
// Copyright (C) 2013 Michael Ringgaard. All rights reserved.
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <shlib.h>
#include <fcntl.h>
#include <time.h>
#include <utime.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>

static char *parse_num(char *str, int len, int *result) {
  *result = 0;
  while (len > 0) {
    if (*str < '0' || *str > '9') return NULL;
    *result = *result * 10 + (*str++ - '0');
    len--;
  }
  return str;
}

static int parse_time(char *str, time_t *result) {
  time_t now;
  struct tm *tm;
  char *dot;
  int len;

  time(&now);
  tm = localtime(&now);
  tm->tm_sec = 0;

  dot = strchr(str, '.');
  len = dot ? dot - str : strlen(str);

  // Parse year
  if (len == 12) {
    str = parse_num(str, 4, &tm->tm_year);
    if (!str) return 1;
    tm->tm_year -= 1900;
  } else if (len == 10) {
    str = parse_num(str, 2, &tm->tm_year);
    if (!str) return 1;
    if (tm->tm_year < 69) tm->tm_year += 100;
  }

  // Parse month
  str = parse_num(str, 2, &tm->tm_mon);
  tm->tm_mon -= 1;
  if (!str) return 1;
  
  // Parse day
  str = parse_num(str, 2, &tm->tm_mday);
  if (!str) return 1;

  // Parse hour
  str = parse_num(str, 2, &tm->tm_hour);
  if (!str) return 1;

  // Parse minute
  str = parse_num(str, 2, &tm->tm_min);
  if (!str) return 1;
  if (*str != 0 && *str != '.') return 1;

  // Parse second
  if (dot) {
    str = parse_num(dot + 1, 2, &tm->tm_sec);
    if (!str || *str != 0) return 1;
  }

  *result = mktime(tm);
  return 0;
}

static void usage() {
  fprintf(stderr, "usage: touch [OPTIONS] FILE...\n\n");
  fprintf(stderr, "  -a      Change access time\n");
  fprintf(stderr, "  -m      Change modifcation time\n");
  fprintf(stderr, "  -c      Do not create file if it does not exist\n");
  fprintf(stderr, "  -r FILE Use the time of FILE instead of the current time\n");
  fprintf(stderr, "  -t TIME Use TIME instead of the current time ([[CC]YY]MMDDhhmm[.SS])\n");
  exit(1);
}

shellcmd(touch) {
  int c;
  int i;
  int atime = 0;
  int mtime = 1;
  int create = 1;
  char *reffile = NULL;
  char *reftime = NULL;
  time_t newtime;

  // Parse command line options
  while ((c = getopt(argc, argv, "amcr:t:?")) != EOF) {
    switch (c) {
      case 'a':
        atime = 1;
        mtime = 0;
        break;

      case 'm':
        atime = 0;
        mtime = 1;
        break;

      case 'c':
        create = 0;
        break;

      case 'r':
        reffile = optarg;
        break;

      case 't':
        reftime = optarg;
        break;

      case '?':
      default:
        usage();
    }
  }  

  if (reffile) {
    struct stat st;
    if (stat(reffile, &st) < 0) {
      perror(reffile);
      return 1;
    }
    newtime = st.st_mtime;
  } else if (reftime) {
    if (parse_time(reftime, &newtime) != 0) {
      fprintf(stderr, "%s: invalid time\n", reftime);
      return 1;
    }
  } else {
    newtime = time(NULL);
  }

  for (i = optind; i < argc; i++) {
    char *fn = argv[i];
    struct utimbuf times;

    // Create file if it does not exist
    if (access(fn, F_OK) != 0) {
      int f;
      if (errno != ENOENT || !create) {
        perror(fn);
        return 1;
      }
      f = creat(fn, 0666);
      if (f < 0) {
        perror(fn);
        return 1;
      }
      close(f);        
    }
    
    // Change modification and/or access time
    times.actime = atime ? newtime : -1;
    times.modtime = mtime ? newtime : -1;
    if (utime(fn, &times) < 0) {
      perror(fn);
      return 1;
    }
  }

  return 0;
}

