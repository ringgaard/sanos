//
// chown.c
//
// Change file ownership
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
#include <unistd.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <sys/stat.h>

struct options {
  int recurse;
  int verbose;
  char *user;
  char *group;
  int uid;
  int gid;
};

static int change_owner(char *path, struct options *opts) {
  struct stat st;

  // Get existing ownership for file
  if (stat(path, &st) < 0) {
    perror(path);
    return 1;
  }

  // Change ownership
  if (opts->verbose) {
    if (opts->uid != st.st_uid) {
      printf("owner of '%s' changed to %s\n", path, opts->user);
    } else {
      printf("ownership of '%s' retained as %s\n", path, opts->user);
    }
    if (opts->gid != -1) {
      if (opts->gid != st.st_gid) {
        printf("group owner of '%s' changed to %s\n", path, opts->group);
      } else {
        printf("group ownership of '%s' retained as %s\n", path, opts->group);
      }
    }
  }

  if (chown(path, opts->uid, opts->gid) < 0) {
    perror(path);
    return 1;
  }

  // Recurse into sub-directories if requested
  if (opts->recurse && S_ISDIR(st.st_mode)) {
    struct dirent *dp;
    DIR *dirp;
    char *subdir;
    int rc;

    dirp = opendir(path);
    if (!dirp) {
      perror(path);
      return 1;
    }
    while ((dp = readdir(dirp))) {
      subdir = join_path(path, dp->d_name);
      if (!subdir) {
        fprintf(stderr, "error: out of memory\n");
        closedir(dirp);
        return 1;
      }
      rc = change_owner(subdir, opts);
      free(subdir);
      if (rc != 0) {
        closedir(dirp);
        return 1;
      }
    }
    closedir(dirp);
  }

  return 0;
}

static void usage() {
  fprintf(stderr, "usage: chown [OPTIONS] OWNER:[GROUP] FILE...\n\n");
  fprintf(stderr, "  -R      Recursively change file ownership\n");
  fprintf(stderr, "  -v      Print out changed file ownership\n");
  exit(1);
}

shellcmd(chown) {
  struct options opts;
  int c;
  int i;
  int rc;
  struct passwd *pwd;
  struct group *grp;

  // Parse command line options
  memset(&opts, 0, sizeof(struct options));
  while ((c = getopt(argc, argv, "Rv?")) != EOF) {
    switch (c) {
      case 'R':
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

  // Get user and group
  if (optind == argc) usage();
  opts.user = argv[optind++];
  opts.group = strchr(opts.user, ':');
  if (opts.group) *opts.group++ = 0;
  pwd = getpwnam(opts.user);
  if (!pwd) {
    fprintf(stderr, "%s: unknown user\n", opts.user);
    return 1;
  }
  opts.uid = pwd->pw_uid;
  if (opts.group) {
    grp = getgrnam(opts.group);
    if (!grp) {
      fprintf(stderr, "%s: unknown group\n", opts.group);
      return 1;
    }
    opts.gid = grp->gr_gid;
  } else {
    opts.gid = -1;
  }

  // Change file ownership
  for (i = optind; i < argc; i++) {
    rc = change_owner(argv[i], &opts);
    if (rc != 0) return 1;
  }

  return 0;
}

