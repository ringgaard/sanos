//
// du.c
//
// Estimate file space usage
//
// Copyright (C) 2012 Michael Ringgaard. All rights reserved.
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
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>

struct options {
  int blksize;
  int all;
};

static int display_file_size(char *path, struct options *opts) {
  struct stat st;
  int total = 0;

  // Stat file
  if (stat(path ? path : ".", &st) < 0) {
    perror(path);
    return -1;
  }

  if (S_ISDIR(st.st_mode)) {
    struct dirent *dp;
    DIR *dirp;
    char *fn;
    int size;

    dirp = opendir(path);
    if (!dirp) {
      perror(path);
      return -1;
    }
    while ((dp = readdir(dirp))) {
      fn = join_path(path, dp->d_name);
      if (!fn) {
        fprintf(stderr, "error: out of memory\n");
        closedir(dirp);
        return -1;
      }
      size = display_file_size(fn, opts);
      free(fn);
      if (size == -1) {
        closedir(dirp);
        return -1;
      }
      total += size;
    }
    closedir(dirp);
  } else {
    total = st.st_size;
  }

  if (opts->all || S_ISDIR(st.st_mode)) {
    printf("%d\t%s\n", (total + opts->blksize - 1) / opts->blksize, path);
  }

  return total;
}

static void usage() {
  fprintf(stderr, "usage: du [OPTIONS] FILE...\n\n");
  fprintf(stderr, "  -a      Output both file and directories\n");
  fprintf(stderr, "  -b      Write files size in units of 512-byte blocks\n");
  fprintf(stderr, "  -k      Write files size in units of 1024-byte blocks\n");
  exit(1);
}

shellcmd(du) {
  struct options opts;
  int c;
  int rc;
  int i;

  // Parse command line options
  memset(&opts, 0, sizeof(struct options));
  opts.blksize = 1024;
  while ((c = getopt(argc, argv, "abk?")) != EOF) {
    switch (c) {
      case 'a':
        opts.all = 1;
        break;

      case 'b':
        opts.blksize = 512;
        break;

      case 'k':
        opts.blksize = 1024;
        break;

      case '?':
      default:
        usage();
    }
  }

  if (optind == argc) {
    rc = display_file_size(".", &opts);
  } else {
    for (i = optind; i < argc; i++) {
      rc = display_file_size(argv[i], &opts);
      if (rc < 0) break;
    }
  }

  return rc < 0;
}

