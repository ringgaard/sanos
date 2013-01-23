//
// rm.c
//
// Remove files
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
#include <stdlib.h>
#include <string.h>
#include <shlib.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>

struct options {
  int recurse;
  int verbose;
  int force;
};

static int remove_file(char *path, struct options *opts) {
  struct stat st;

  // Stat file
  if (stat(path, &st) < 0) {
    perror(path);
    return 1;
  }

  // Recurse into sub-directories if requested
  if (opts->recurse && S_ISDIR(st.st_mode)) {
    struct dirent *dp;
    DIR *dirp;
    char *fn;
    int rc;
    int more;

    dirp = opendir(path);
    if (!dirp) {
      perror(path);
      return 1;
    }

    // Keep scanning directory until it is empty
    more = 1;
    while (more) {
      more = 0;
      while ((dp = readdir(dirp))) {
        more = 1;
        fn = join_path(path, dp->d_name);
        if (!fn) {
          fprintf(stderr, "error: out of memory\n");
          closedir(dirp);
          return 1;
        }
        rc = remove_file(fn, opts);
        free(fn);
        if (rc != 0) {
          closedir(dirp);
          return 1;
        }
      }
      if (more) rewinddir(dirp);
    }
    closedir(dirp);
    
    // Remove directory
    if (opts->verbose) printf("removed directory '%s'\n", path);
    if (rmdir(path) < 0 && !opts->force) {
      perror(path);
      return 1;
    }
  } else {
    // Remove file
    if (opts->verbose) printf("removed file '%s'\n", path);
    if (unlink(path) < 0 && !opts->force) {
      perror(path);
      return 1;
    }
  }
  
  return 0;
}

static void usage() {
  fprintf(stderr, "usage: rm [OPTIONS] FILE...\n\n");
  fprintf(stderr, "  -r, -R  Recursively remove files\n");
  fprintf(stderr, "  -f      Force removal of files\n");
  fprintf(stderr, "  -v      Print out removed files\n");
  exit(1);
}

shellcmd(rm) {
  struct options opts;
  int c;
  int i;
  int rc;

  // Parse command line options
  memset(&opts, 0, sizeof(struct options));
  while ((c = getopt(argc, argv, "fRrv?")) != EOF) {
    switch (c) {
      case 'f':
        opts.force = 1;
        break;

      case 'R':
      case 'r':
        opts.recurse = 1;
        break;

      case 'v':
        opts.verbose = 1;
        break;

      case '?':
      default:
        usage();
    }
  }

  // Remove files
  for (i = optind; i < argc; i++) {
    rc = remove_file(argv[i], &opts);
    if (rc != 0) return 1;
  }

  return 0;
}

