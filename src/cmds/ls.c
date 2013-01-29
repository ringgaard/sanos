//
// ls.c
//
// List directory contents
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

#include <dirent.h>
#include <fcntl.h>
#include <grp.h>
#include <pwd.h>
#include <shlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#define HALFYEAR (365 * 24 * 60 * 60 / 2)

struct options {
  int detail;
  int onecol;
  int recurse;
  
  int width;
  int nodir;
  time_t now;
};

struct file {
  char *dir;
  char *name;
  struct stat stat;
};

struct filelist {
  struct file **begin;
  struct file **end;
  struct file **limit;
};

static void add_file(struct filelist *list, char *dir, char *name, struct stat *st) {
  struct file *file;

  // Extend list if it is full
  if (list->end == list->limit) {
    int size = list->limit - list->begin;
    int newsize = size ? size * 2 : 32;
    list->begin = (struct file **) realloc(list->begin, newsize * sizeof(struct file *));
    list->end = list->begin + size;
    list->limit = list->begin + newsize;
  }
  
  // Add new file entry to list
  file = (struct file *) malloc(sizeof(struct file));
  file->dir = strdup(dir);
  file->name = strdup(name);
  memcpy(&file->stat, st, sizeof(struct stat));
  *list->end++ = file;
}

static void delete_file_list(struct filelist *list) {
  struct file **f;

  for (f = list->begin; f != list->end; f++) {
    free((*f)->dir);
    free((*f)->name);
    free(*f);
  }
}

static int compare_filename(const void *d1, const void *d2) {
  struct file *f1 = *(struct file **) d1;
  struct file *f2 = *(struct file **) d2;
  int cmp = strcmp(f1->dir, f2->dir);
  if (!cmp) cmp = strcmp(f1->name, f2->name);
  return cmp;
}

static void sort_list(struct filelist *list) {
  qsort(list->begin, list->end - list->begin, sizeof(struct file *), compare_filename);
}

void collect_directory(struct filelist *list, char *dir, struct options *opts) {
  struct stat st;
  struct dirent *dp;
  DIR *dirp;
  char *fn;

  dirp = opendir(dir ? dir : ".");
  if (!dirp) {
    perror(dir);
    return;
  }

  while ((dp = readdir(dirp))) {
    fn = join_path(dir, dp->d_name);
    if (!fn) break;
    if (stat(fn, &st) >= 0) {
      add_file(list, dir, dp->d_name, &st);
      if (opts->recurse && S_ISDIR(st.st_mode)) {
        collect_directory(list, fn, opts);
      }
    }
    free(fn);
  }
  closedir(dirp);
}

void collect_files(struct filelist *list, char *path, struct options *opts) {
  struct stat st;
  
  // Stat file
  if (stat(path ? path : ".", &st) < 0) {
    perror(path);
    return;
  }

  if (!S_ISDIR(st.st_mode) || opts->nodir) {
    // Add single file
    add_file(list, "", path, &st);
  } else {
    // Add directory files
    collect_directory(list, path, opts);
  }
}

static int numlen(int n) {
  int l = 0;
  if (n == 0) return 1;
  while (n > 0) {
    l++;
    n /= 10;
  }
  return l;
}

void print_details(struct file *files[], int numfiles, struct options *opts) {
  char perms[11];
  char date[14];
  struct tm tm;
  int old;
  struct passwd *pwd;
  struct group *grp;
  int i, l;

  // Find width of variable size columns
  int lwidth = 1;
  int owidth = 1;
  int gwidth = 1;
  int swidth = 1;
  int lastuid = -1;
  int lastgid = -1;
  for (i = 0; i < numfiles; i++) {
    struct file *file = files[i];
    struct stat *st = &file->stat;

    l = numlen(st->st_nlink);
    if (l > lwidth) lwidth = l;

    if (st->st_uid != lastuid) {
      pwd = getpwuid(st->st_uid);
      l = pwd ? strlen(pwd->pw_name) : numlen(st->st_uid);
      if (l > owidth) owidth = l;
      lastuid = st->st_uid;
    }

    if (st->st_gid != lastgid) {
      grp = getgrgid(st->st_gid);
      l = grp ? strlen(grp->gr_name) : numlen(st->st_gid);
      if (l > gwidth) gwidth = l;
      lastgid = st->st_gid;
    }
    
    l = numlen(st->st_size);
    if (l > swidth) swidth = l;
  }

  for (i = 0; i < numfiles; i++) {
    struct file *file = files[i];
    struct stat *st = &file->stat;

    // Print permissions and number of links
    printf("%s %*d ", get_symbolic_mode(st->st_mode, perms), lwidth, st->st_nlink);

    // Print owner
    pwd = getpwuid(st->st_uid);
    if (pwd) {
      printf("%-*s ", owidth, pwd->pw_name);
    } else {
      printf("%*d ", owidth, st->st_uid);
    }
  
    // Print group
    grp = getgrgid(st->st_gid);
    if (grp) {
      printf("%-*s ", gwidth, grp->gr_name);
    } else {
      printf("%*d ", gwidth, st->st_gid);
    }

    // Print size
    printf("%*d ", swidth, st->st_size);

    // Print modification date
    gmtime_r(&st->st_mtime, &tm);
    old = opts->now - st->st_mtime > HALFYEAR;
    strftime(date, sizeof(date), old ? "%b %e  %Y" : "%b %e %H:%M", &tm);
    printf("%s ", date);
  
    // Print filename
    printf("%s%s\n", file->name, S_ISDIR(st->st_mode) ? "/" : "");
  }
}

void print_columns(struct file *files[], int numfiles, struct options *opts) {
  int i, l;
  int colwidth;
  int cols, rows;
  int r, c;

  // Find column width and number of columns
  colwidth = 1;
  for (i = 0; i < numfiles; i++) {
    struct file *file = files[i];
    l = strlen(file->name);
    if (S_ISDIR(file->stat.st_mode)) l++;
    if (l > colwidth) colwidth = l;
  }
  colwidth += 2;
  cols = opts->width / colwidth;
  if (cols < 1) cols = 1;
  rows = (numfiles + cols - 1) / cols;

  // Output rows and columns
  for (r = 0; r < rows; r++) {
    for (c = 0; c < cols; c++) {
      int n = c * rows + r;
      if (n < numfiles) {
        struct file *file = files[n];
        l = strlen(file->name);
        printf("%s", file->name);
        if (S_ISDIR(file->stat.st_mode)) {
          printf("/");
          l++;
        }
        if (c < cols - 1) {
          l = colwidth - l;
          while (l-- > 0) putchar(' ');
        }
      } else {
        break;
      }
    }
    printf("\n");
  }
}

static void print_files(struct filelist *list, struct options *opts) {
  struct file **f;

  if (opts->onecol) {
    // Print list of file with one file per line
    for (f = list->begin; f != list->end; f++) {
      struct file *file = *f;
      if (strcmp(file->dir, ".") == 0) {
        printf("%s\n", file->name);
      } else {
        printf("%s/%s\n", file->dir, file->name);
      }
    }
  } else {
    struct file **dir;
    int multidir = 0;
    int firstdir = 1;

    // Check for multiple directories
    for (f = list->begin + 1; f != list->end; f++) {
      if (strcmp(f[0]->dir, f[-1]->dir) != 0) {
        multidir = 1;
        break;
      }
    }

    // Output files for each directory
    dir = f = list->begin;
    for (f = list->begin; f <= list->end; f++) {
      if (f == list->end || strcmp((*dir)->dir, (*f)->dir) != 0) {
        if (multidir) {
          if (!firstdir) printf("\n");
          printf("%s:\n", (*dir)->dir);
          firstdir = 0;
        }
        if (opts->detail) {
          print_details(dir, f - dir, opts);
        } else {
          print_columns(dir, f - dir, opts);
        }
        dir = f;
      }
    }
  }
}

static void usage() {
  fprintf(stderr, "usage: ls [OPTIONS] FILE...\n\n");
  fprintf(stderr, "  -l      List file details\n");
  fprintf(stderr, "  -1      List files in one column\n");
  fprintf(stderr, "  -d      List directory entries instead of contents\n");
  fprintf(stderr, "  -R      Recursively list subdirectories\n");
  exit(1);
}

shellcmd(ls) {
  struct options opts;
  int c;
  int i;
  struct filelist list;

  // Parse command line options
  memset(&opts, 0, sizeof(struct options));
  opts.width = 80;
  while ((c = getopt(argc, argv, "dl1R?")) != EOF) {
    switch (c) {
      case 'd':
        opts.nodir = 1;
        break;

      case 'l':
        opts.detail = 1;
        break;

      case '1':
        opts.onecol = 1;
        break;

      case 'R':
        opts.recurse = 1;
        break;

      case '?':
      default:
        usage();
    }
  }

  // Collect files
  opts.now = time(NULL);
  memset(&list, 0, sizeof(struct filelist));
  if (optind == argc) {
    collect_files(&list, ".", &opts);
  } else {
    for (i = optind; i < argc; i++) {
      collect_files(&list, argv[i], &opts);
    }
  }

  // List files
  if (list.begin != list.end) {
    sort_list(&list);
    print_files(&list, &opts);
  }
  delete_file_list(&list);
  
  return 0;
}

