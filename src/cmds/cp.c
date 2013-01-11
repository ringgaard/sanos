//
// cp.c
//
// Copy files
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
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <shlib.h>
#include <unistd.h>
#include <utime.h>
#include <sys/stat.h>

#define BLKSIZE 4096

struct options {
  int force;
  int recurse;
  int verbose;
  int noclobber;
  int preserve;
};

static int copy_file(char *src, char *dest, struct options *opts);

static int copy_directory(char *src, char *dest, struct options *opts) {
  struct dirent *dp;
  DIR *dirp;
  char *srcfn;
  char *destfn;
  int rc;

  // Copy all files in directory
  dirp = opendir(src);
  if (!dirp) {
    perror(src);
    return 1;
  }
  mkdir(dest, 0777);
  while ((dp = readdir(dirp))) {
    srcfn = join_path(src, dp->d_name);
    destfn = join_path(dest, dp->d_name);
    if (!srcfn || !destfn) {
      fprintf(stderr, "error: out of memory\n");
      closedir(dirp);
      return 1;
    }
    rc = copy_file(srcfn, destfn, opts);
    free(srcfn);
    free(destfn);
    if (rc != 0) {
      closedir(dirp);
      return 1;
    }
  }
  closedir(dirp);

  return 0;
}

static int copy_file(char *src, char *dest, struct options *opts) {
  int fin;
  int fout;
  struct stat st;
  char *buffer;
  int n;

  // Refuse to copy file unto itself
  if (!opts->force && strcmp(src, dest) == 0) {
    fprintf(stderr, "%s: cannot copy file unto itself\n");
    return 1;
  }

  // Open source file
  fin = open(src, O_RDONLY | O_BINARY);
  if (fin < 0  || fstat(fin, &st) < 0) {
    perror(src);
    return 1;
  }
  
  // If source is a directory copy recursively if requested
  if (S_ISDIR(st.st_mode)) {
    close(fin);
    if (opts->recurse) {
      return copy_directory(src, dest, opts);
    } else {
      errno = EISDIR;
      perror(src);
      return 1;
    }
  }

  // Copy source file to destination
  if (opts->verbose) printf("%s -> %s\n", src, dest);
  if (opts->force) unlink(dest);
  fout = open(dest, O_WRONLY | O_CREAT | (opts->noclobber ? O_EXCL : O_TRUNC) | O_BINARY, 0666);
  buffer = malloc(BLKSIZE);
  if (fout < 0 || !buffer) {
    perror(dest);
    close(fin);
    return 1;
  }
  while ((n = read(fin, buffer, BLKSIZE)) != 0) {
    if (n < 0) {
      perror(src);
      break;
    }
    if (write(fout, buffer, n) != n) {
      perror(dest);
      break;
    }
  }
  fchmod(fout, st.st_mode);

  if (opts->preserve) {
    struct utimbuf times;
    times.modtime = st.st_mtime;
    times.actime = st.st_atime;
    times.ctime = -1;
    futime(fout, &times);
    fchown(fout, st.st_uid, st.st_gid);
  }

  free(buffer);
  close(fin);
  close(fout);

  return n;
}

static int copy_file_to_dir(char *src, char *path, struct options *opts) {
  char *basename;
  char *dest;
  int rc;

  // Find base name for source file
  basename = strrchr(src, '/');
  basename = basename ? basename + 1 : src;

  // Build destination file name
  dest = join_path(path, basename);
  if (!dest) {
    perror(dest);
    return 1;
  }

  // Copy source file to destination directory
  rc = copy_file(src, dest, opts);
  free(dest);
  return rc;
}

static void usage() {
  fprintf(stderr, "usage: cp [OPTIONS] SRC DEST\n");
  fprintf(stderr, "       cp [OPTIONS] SRC... DIR\n\n");
  fprintf(stderr, "  -r, -R  Copy directories recursively\n");
  fprintf(stderr, "  -f      Force overwriting of destination files\n");
  fprintf(stderr, "  -p      Preserve modification time and owner\n");
  fprintf(stderr, "  -v      Print files being copied\n");
  fprintf(stderr, "  -n      Do not overwrite existing files\n");
  exit(1);
}

shellcmd(cp) {
  struct options opts;
  int c;
  char *dest;
  int dirdest;
  struct stat st;
  int rc;
  int i;

  // Parse command line options
  memset(&opts, 0, sizeof(struct options));
  while ((c = getopt(argc, argv, "frRvnp?")) != EOF) {
    switch (c) {
      case 'f':
        opts.force = 1;
        break;

      case 'r':
      case 'R':
        opts.recurse = 1;
        break;

      case 'v':
        opts.verbose = 1;
        break;

      case 'n':
        opts.noclobber = 1;
        break;

      case 'p':
        opts.preserve = 1;
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

  if (argc - optind > 2 || dirdest) {
    // Copy source files to destination directory
    if (!dirdest) usage();
    for (i = optind; i < argc - 1; i++) {
      rc = copy_file_to_dir(argv[i], dest, &opts);
      if (rc != 0) break;
    }
  } else {
    // Copy source file to destination file
    rc = copy_file(argv[optind], argv[optind + 1], &opts);
  }

  return rc;
}

