//
// grep.c
//
// Search files for a pattern
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
#include <regex.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>

#define MAXLINE 512

struct options {
  int extended;
  int nocase;
  int output_filename;
  int output_linenum;
  int nolines;
  int matchcount;
  int invert;
  int recurse;
  regex_t re;
};

int search_file(FILE *f, char *filename, struct options *opts) {
  int match;
  int n;
  int linenum;
  char line[MAXLINE];

  // Handle simple match or no match case
  if (opts->nolines || opts->matchcount) {
    // Count number of matches
    int matches = 0;
    while (fgets(line, sizeof(line), f) != NULL) {
      n = strlen(line);
      if (n > 0 && line[n - 1] == '\n') line[--n] = 0;
      if (n > 0 && line[n - 1] == '\r') line[--n] = 0;

      match = regexec(&opts->re, line, 0, NULL, 0) != REG_NOMATCH;
      if (opts->invert) match = !match;
      if (match) {
        matches++;
        if (!opts->matchcount) break;
      }
    }

    // Output filename (and matches) if any matches was found
    if (opts->matchcount) {
      if (opts->output_filename) printf("%s:", filename);
      printf("%d\n", matches);
    } else if (matches) {
      printf("%s\n", filename);
    }
    return 0;
  }

  // Find matches in file
  linenum = 1;
  while (fgets(line, sizeof(line), f) != NULL) {
    n = strlen(line);
      if (n > 0 && line[n - 1] == '\n') line[--n] = 0;
      if (n > 0 && line[n - 1] == '\r') line[--n] = 0;

    match = regexec(&opts->re, line, 0, NULL, 0) != REG_NOMATCH;
    if (opts->invert) match = !match;

    if (match) {
      if (opts->output_filename) printf("%s:", filename);
      if (opts->output_linenum) printf("%d:", linenum);
      printf("%s\n", line);
    }

    linenum++;
  }
  return 0;
}

int search_directory(char *path, struct options *opts) {
  struct dirent *dp;
  DIR *dirp;
  char *fn;
  int rc;
  struct stat st;

  dirp = opendir(path);
  if (!dirp) {
    perror(path);
    return 1;
  }
  while ((dp = readdir(dirp))) {
    fn = join_path(path, dp->d_name);
    if (!fn) {
      fprintf(stderr, "error: out of memory\n");
      closedir(dirp);
      return 1;
    }

    if (stat(fn, &st) >= 0 && S_ISDIR(st.st_mode)) {
      rc = search_directory(fn, opts);
    } else {
      FILE *f = fopen(fn, "r");
      if (!f) {
        perror(fn);
        rc = 1;
      } else {
        rc = search_file(f, fn, opts);
        fclose(f);
      }
    }

    free(fn);
    if (rc != 0) {
      closedir(dirp);
      return 1;
    }
  }
  closedir(dirp);

  return 0;
}

static void usage() {
  fprintf(stderr, "usage: grep [OPTIONS] PATTERN FILE...\n\n");
  fprintf(stderr, "  -E      Match using extended regular expressions\n");
  fprintf(stderr, "  -c      Print number of lines matching\n");
  fprintf(stderr, "  -i      Case-insensitive search\n");
  fprintf(stderr, "  -l      Print only names of files with matches\n");
  fprintf(stderr, "  -n      Print line numbers\n");
  fprintf(stderr, "  -r, -R  Search sub-directories\n");
  fprintf(stderr, "  -v      Print lines that do not match\n");
  exit(1);
}

shellcmd(grep) {
  struct options opts;
  int c;
  char *pattern;
  int reflags;
  int rc;
  FILE *f;
  int i;
  struct stat st;

  // Parse command line options
  memset(&opts, 0, sizeof(struct options));
  while ((c = getopt(argc, argv, "RrEcilnv?")) != EOF) {
    switch (c) {
      case 'E':
        opts.extended = 1;
        break;

      case 'R':
      case 'r':
        opts.recurse = 1;
        break;

      case 'c':
        opts.matchcount = 1;
        break;

      case 'i':
        opts.nocase = 1;
        break;

      case 'l':
        opts.nolines = 1;
        break;

      case 'n':
        opts.output_linenum = 1;
        break;

      case 'v':
        opts.invert = 1;
        break;

      case '?':
      default:
        usage();
    }
  }

  // Get pattern
  if (optind == argc) usage();
  pattern = argv[optind++];

  // Convert patttern to regex
  reflags = opts.extended ? REG_EXTENDED : REG_BASIC;
  if (opts.nocase) reflags |= REG_ICASE;
  rc = regcomp(&opts.re, pattern, reflags);
  if (rc != 0) {
    char errmsg[128];
    regerror(rc, &opts.re, errmsg, sizeof(errmsg));
    fprintf(stderr, "%s: regular expression error '%s'\n", pattern, errmsg);
    return 1;
  }

  // Search files
  if (optind == argc) {
    rc = search_file(stdin, "<stdin>", &opts);
  } else {
    opts.output_filename = argc - optind > 1 || opts.recurse;
    for (i = optind; i < argc; i++) {
      char *fn = argv[i];
      if (opts.recurse && stat(fn, &st) >= 0 && S_ISDIR(st.st_mode)) {
        rc = search_directory(fn, &opts);
        if (rc != 0) break;
      } else {
        f = fopen(fn, "r");
        if (!f) {
          perror(fn);
          rc = 1;
          break;
        } else {
          rc = search_file(f, fn, &opts);
          fclose(f);
          if (rc != 0) break;
        }
      }
    }
  }
  regfree(&opts.re);
  return rc;
}

