//
// mv.c
//
// Move files
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
#include <fcntl.h>
#include <shlib.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

struct options {
  int force;
  int verbose;
};

static void usage() {
  fprintf(stderr, "usage: mv [OPTIONS] SRC DEST\n");
  fprintf(stderr, "       mv [OPTIONS] SRC... DIR\n\n");
  fprintf(stderr, "  -f      Force overwriting of destination files\n");
  fprintf(stderr, "  -v      Print files being moved\n");
  exit(1);
}

shellcmd(mv) {
  struct options opts;
  int c;
  char *dest;
  int dirdest;
  struct stat st;
  int rc;
  int i;

  // Parse command line options
  memset(&opts, 0, sizeof(struct options));
  while ((c = getopt(argc, argv, "fv?")) != EOF) {
    switch (c) {
      case 'f':
        opts.force = 1;
        break;

      case 'v':
        opts.verbose = 1;
        break;

      case '?':
      default:
        usage();
    }
  }
  if (argc - optind < 2) usage();

  // Check if destination is a directory
  dest = argv[argc - 1];
  dirdest = stat(dest, &st) >= 0 && S_ISDIR(st.st_mode);

  if (argc - optind > 2) {
    // Target must be a directory
    if (!dirdest) {
      fprintf(stderr, "%s: is not a directory\n", dest);
      return 1;
    }

    // Move source files to destination directory
    for (i = optind; i < argc - 1; i++) {
      char *destfn = join_path(dest, argv[i]);
      if (opts.force) unlink(destfn);
      if (opts.verbose) printf("move '%s' to '%s'\n", argv[i], destfn);
      rc = rename(argv[i], destfn);
      if (rc < 0) {
        perror(destfn);
        free(destfn);
        return 1;
      }
      free(destfn);
    }
  } else {
    // Rename source file to destination
    if (opts.force) unlink(argv[optind + 1]);
    if (opts.verbose) printf("move '%s' to '%s'\n", argv[optind], argv[optind + 1]);
    rc = rename(argv[optind], argv[optind + 1]);
    if (rc < 0) {
      perror(argv[optind]);
      return 1;
    }
  }

  return 0;
}

