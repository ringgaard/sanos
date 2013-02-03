//
// pkg.c
//
// Package management tool
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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <tar.h>
#include <errno.h>
#include <fcntl.h>
#include <utime.h>
#include <inifile.h>
#include <shlib.h>
#include <dirent.h>
#include <sys/stat.h>

#define STRLEN 1024
#define DEFAULT_PKG_REPO "http://www.jbox.dk/pkg"
#define PKGDB_FILE       "/usr/share/pkg/db"

#define FROM_FILE   0x01
#define ONLY_FETCH  0x02
#define ONLY_BUILD  0x04
#define DEPENDENCY  0x08
#define UPDATE      0x10
#define UPGRADE     0x20
#define VERBOSE     0x40

struct pkg {
  char *name;
  char *inffile;
  char *description;
  struct section *manifest;
  time_t time;
  time_t avail;
  int removed;
  struct pkg *next;
};

struct pkgdb {
  struct pkg *head;
  struct pkg *tail;
  char *repo;
  int dirty;
};

static unsigned int parse_octal(char *str, int len) {
  unsigned int n = 0;
  while (len > 0)  {
    n = n * 8 + (*str++ - '0');
    len--;
  }
  return n;
}

static unsigned int buffersum(unsigned char *buffer, int len) {
  unsigned char *end = buffer + len;
  unsigned int sum = 0;
  while (buffer < end) sum += *buffer++;
  return sum;
}

int make_directory(char *path, int mode) {
  char buf[FILENAME_MAX];
  char *p;

  // Check if path exists
  if (access(path, X_OK) == 0) return 0;

  // Copy path to buffer
  if (strlen(path) >= FILENAME_MAX) {
    errno = ENAMETOOLONG;
    return -1;
  }
  strcpy(buf, path);

  // Create directories one level at a time
  p = path;
  while (p) {
    p = strchr(p, '/');
    if (p) *p = 0;
    if (mkdir(path, mode) < 0 && errno != EEXIST) return -1;
    if (p) *p++ = '/';
  }

  return 0;
}

static int extract_files(char *url, FILE *f, char *manifest, int verbose, int *time) {
  unsigned char buffer[TAR_BLKSIZ];
  unsigned char fn[FILENAME_MAX];
  struct tarhdr *hdr;
  unsigned int checksum;
  int n, size, left, mode, first, zeroblks;
  struct utimbuf times;
  char *slash;
  int fd;

  zeroblks = 0;
  first = 1;
  while (!feof(f)) {
    // Read next tar header
    if (fread(buffer, 1, TAR_BLKSIZ, f) != TAR_BLKSIZ) {
      fprintf(stderr, "%s: tar archive truncated\n", url);
      return 1;
    }
    hdr = (struct tarhdr *) buffer;

    // Two all-zero blocks marks the end of the archive
    if (hdr->name[0] == 0) {
      if (++zeroblks == 2) break;
      continue;
    }
    zeroblks = 0;

    // Check tar header integrity
    if (memcmp(hdr->magic, "ustar ", 6) != 0) {
      fprintf(stderr, "%s: invalid tar header (%s)\n", url, hdr->magic);
      return 1;
    }
    checksum = parse_octal(hdr->chksum, 6);
    memcpy(hdr->chksum, "        ", 8);
    if (buffersum(buffer, TAR_BLKSIZ) != checksum) {
      fprintf(stderr, "%s: corrupted tar file\n", url);
      return 1;
    }

    // Get file information
    strcpy(fn, "/");
    strcat(fn, hdr->name);
    mode = parse_octal(hdr->mode, 7);
    size = parse_octal(hdr->size, 11);
    times.modtime = parse_octal(hdr->mtime, 11);
    times.actime = -1;
    times.ctime = -1;
    if (verbose) printf("extracting '%s' size: %d mode: %o\n", fn, size, mode);
    if (times.modtime > *time) *time = times.modtime;

    if (hdr->typeflag == DIRTYPE) {
      // Create directory
      if (make_directory(fn, mode) < 0) {
        perror(fn);
        return 1;
      }
    } else if (hdr->typeflag == REGTYPE || hdr->typeflag == AREGTYPE)  {
      // Make sure target directory exists
      slash = strrchr(fn, '/');
      if (slash && slash != fn) {
        *slash = 0;
        if (make_directory(fn, mode) < 0) {
          perror(fn);
          return 1;
        }
        *slash = '/';
      }

      // Extract file
      fd = open(fn, O_CREAT | O_TRUNC, mode);
      if (fd < 0) {
        perror(fn);
        return 1;
      }
      left = size;
      while (left > 0) {
        n = fread(buffer, 1, TAR_BLKSIZ, f);
        if (n != TAR_BLKSIZ) {
          fprintf(stderr, "%s: tar archive truncated (%s)\n", url, fn);
          close(fd);
          unlink(fn);
          return 1;
        }
        if (write(fd, buffer, n > left ? left : n) < 0) {
          perror(fn);
          close(fd);
          unlink(fn);
          return 1;
        }
        left -= n;
      }

      futime(fd, &times);
      fchmod(fd, mode);
      close(fd);

      if (first) {
        strcpy(manifest, fn);
        first = 0;
      }
    } else {
      fprintf(stderr, "ignoring %s (type %c)\n", fn, hdr->typeflag);
    }
  }

  return 0;
}

static struct pkg *add_package(struct pkgdb *db, char *name, char *description) {
  struct pkg *pkg = calloc(1, sizeof(struct pkg));
  pkg->name = strdup(name);
  pkg->description = strdup(description ? description : "");
  if (!db->head) db->head = pkg;
  if (db->tail) db->tail->next = pkg;
  db->tail = pkg;
  db->dirty = 1;
  return pkg;
}

static struct pkg *find_package(struct pkgdb *db, char *name) {
  struct pkg *pkg;

  for (pkg = db->head; pkg; pkg = pkg->next) {
    if (strcmp(pkg->name, name) == 0) return pkg;
  }
  return NULL;
}

static int read_pkgdb_from_file(FILE *f, struct pkgdb *db) {
  char line[STRLEN];
  while (fgets(line, STRLEN, f)) {
    char *p = line;
    char *name = NULL;
    char *inffile = NULL;
    char *description = NULL;
    char *time = NULL;
    name = strsep(&p, "\t\n");
    inffile = strsep(&p, "\t\n");
    description = strsep(&p, "\t\n");
    time = strsep(&p, "\t\n");

    if (name) {
      struct pkg *pkg = add_package(db, name, description);
      if (inffile) pkg->inffile = strdup(inffile);
      if (time) pkg->time = atoi(time);
    }
  }
  db->dirty = 0;
  return 0;
}

static int read_pkgdb(struct pkgdb *db) {
  FILE *f;

  f = fopen(PKGDB_FILE, "r");
  if (f) {
   read_pkgdb_from_file(f, db);
    fclose(f);
  }
  return 0;
}

static int write_pkgdb(struct pkgdb *db) {
  FILE *f;
  struct pkg *pkg;

  f = fopen(PKGDB_FILE, "w");
  if (!f) {
    perror(PKGDB_FILE);
    return 1;
  }
  for (pkg = db->head; pkg; pkg = pkg->next) {
    if (!pkg->removed) {
      fprintf(f, "%s\t%s\t%s\t%d\n", pkg->name, pkg->inffile ? pkg->inffile : "", pkg->description, pkg->time);
    }
  }
  fclose(f);
  db->dirty = 0;

  return 0;
}

static int delete_pkgdb(struct pkgdb *db) {
  struct pkg *pkg = db->head;
  while (pkg) {
    struct pkg *next = pkg->next;
    free(pkg->name);
    free(pkg->inffile);
    free(pkg->description);
    free_properties(pkg->manifest);
    free(pkg);
    pkg = next;
  }
  db->head = db->tail = NULL;
  return 0;
}

static int install_package(char *pkgname, struct pkgdb *db, int options) {
  char url[STRLEN];
  FILE *f;
  int rc;
  char inffile[STRLEN];
  struct section *manifest;
  struct section *dep;
  struct section *build;
  struct pkg *pkg;
  char *description;
  int time;

  // Check if package is already installed
  if (!(options & FROM_FILE)) {
    pkg = find_package(db, pkgname);
    if (pkg) {
      if (options & UPGRADE) {
        if (pkg->time == pkg->avail) return 0;
      } else if (options & DEPENDENCY) {
        return 0;
      } else if (options & UPDATE) {
        if (options & VERBOSE) printf("updating package %s\n", pkgname);
      } else {
        printf("package %s is already installed\n", pkgname);
        return 0;
      }
    }
  }

  // Open package file
  printf("Fetching %s\n", pkgname);
  if (options & FROM_FILE) {
    snprintf(url, sizeof(url), "file:///%s", pkgname);
  } else {
    snprintf(url, sizeof(url), "%s/%s.pkg", db->repo, pkgname);
  }
  if (options & VERBOSE) printf("fetching package %s from %s\n", pkgname, url);
  f = open_url(url, "pkg");
  if (!f) return 1;

  // Extract file from package file
  time = 0;
  rc = extract_files(url, f, inffile, options & VERBOSE, &time);
  fclose(f);
  if (rc != 0) return rc;

  // Read manifest
  if (options & VERBOSE) printf("reading manifest from %s\n", inffile);
  manifest = read_properties(inffile);
  if (!manifest) {
    fprintf(stderr, "%s: unable to read manifest\n", inffile);
    return 1;
  }

  // Add package to package database
  pkgname = get_property(manifest, "package", "name", pkgname);
  description = get_property(manifest, "package", "description", NULL);
  pkg = find_package(db, pkgname);
  if (pkg) {
    if (options & VERBOSE) printf("updating package %s in database\n", pkgname);
    if (description) {
      free(pkg->description);
      pkg->description = strdup(description);
      db->dirty = 1;
    }
    if (pkg->manifest) free_properties(pkg->manifest);
    free(pkg->inffile);
  } else {
    if (options & VERBOSE) printf("adding package %s to database\n", pkgname);
    pkg = add_package(db, pkgname, description);
  }
  pkg->inffile = strdup(inffile);
  pkg->manifest = manifest;
  if (time != pkg->time) {
    pkg->time = time;
    db->dirty = 1;
  }

  // Install package dependencies
  dep = find_section(manifest, "dependencies");
  if (dep) {
    struct property *p;
    for (p = dep->properties; p; p = p->next) {
      if (options & VERBOSE) printf("package %s depends on %s\n", pkgname, p->name);
      rc = install_package(p->name, db, options | DEPENDENCY);
      if (rc != 0) return rc;
    }
  }
  if ((options & ONLY_FETCH) && !(options & DEPENDENCY)) return 0;

  // Run package build/installation commands
  if (!(options & ONLY_FETCH) || (options & DEPENDENCY)) {
    build = find_section(manifest, (options & ONLY_BUILD) && !(options & DEPENDENCY) ? "build" : "install");
    if (build) {
      struct property *p;
      printf((options & ONLY_BUILD) && !(options & DEPENDENCY) ? "Building %s\n" : "Installing %s\n", pkgname);
      for (p = build->properties; p; p = p->next) {
        if (options & VERBOSE) printf("%s\n", p->name);
        rc = system(p->name);
        if (rc != 0) {
          fprintf(stderr, "%s: build failed\n", pkgname);
          return rc;
        }
      }
    }
  }

  return 0;
}

static void remove_path(char *path, int options) {
  struct stat st;

  // Stat file
  if (stat(path, &st) < 0) {
    perror(path);
    return;
  }

  if (S_ISDIR(st.st_mode)) {
    struct dirent *dp;
    DIR *dirp;
    char *fn;
    int more;

    // Remove files in directory recursively
    if (options & VERBOSE) printf("deleting all files in %s\n", path);
    dirp = opendir(path);
    if (!dirp) {
      perror(path);
      return;
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
          return;
        }
        remove_path(fn, options);
      }
      if (more) rewinddir(dirp);
    }
    closedir(dirp);

    // Remove directory
    if (options & VERBOSE) printf("removing directory %s\n", path);
    if (rmdir(path) < 0) perror(path);
  } else {
    // Remove file
    if (options & VERBOSE) printf("removing file %s\n", path);
    if (unlink(path) < 0) perror(path);
  }
}

static int remove_package(char *pkgname, struct pkgdb *db, int options) {
  struct pkg *pkg;
  struct section *files;
  
  // Find package and load manifest
  pkg = find_package(db, pkgname);
  if (!pkg) {
    fprintf(stderr, "%s: package not found\n", pkgname);
    return 1;
  }
  if (pkg->removed) return 0;
  if (!pkg->manifest) {
    if (options & VERBOSE) printf("reading manifest from %s\n", pkg->inffile);
    pkg->manifest = read_properties(pkg->inffile);
    if (!pkg->manifest) {
      fprintf(stderr, "%s: unable to read manifest\n", pkg->inffile);
      return 1;
    }
  }

  // Remove package files
  files = find_section(pkg->manifest, "files");
  if (files) {
    struct property *p;
    for (p = files->properties; p; p = p->next) {
      remove_path(p->name, options);
    }
  }

  // Remove manifest file
  unlink(pkg->inffile);

  // Remove package from package database
  pkg->removed = 1;
  db->dirty = 1;

  return 0;
}

static int retrieve_package_timestamps(struct pkgdb *db, char *repo) {
  char url[STRLEN];
  FILE *f;
  struct pkgdb rdb;
  struct pkg *pkg;
  struct pkg *rpkg;

  snprintf(url, sizeof(url), "%s/db", repo);
  f = open_url(url, "pkg");
  if (!f) return 1;

  memset(&rdb, 0, sizeof(struct pkgdb));
  read_pkgdb_from_file(f, &rdb);
  fclose(f);

  for (pkg = db->head; pkg; pkg = pkg->next) {
    rpkg = find_package(&rdb, pkg->name);
    if (rpkg) pkg->avail = rpkg->time;
  }
  delete_pkgdb(&rdb);
  return 0;
}

static int query_repository(char **keywords, char *repo) {
  char url[STRLEN];
  FILE *f;
  struct pkgdb db;
  struct pkg *pkg;

  snprintf(url, sizeof(url), "%s/db", repo);
  f = open_url(url, "pkg");
  if (!f) return 1;

  memset(&db, 0, sizeof(struct pkgdb));
  read_pkgdb_from_file(f, &db);
  fclose(f);

  for (pkg = db.head; pkg; pkg = pkg->next) {
    int match = 0;
    if (*keywords) {
      char **k = keywords;
      while (*k) {
        match = strstr(pkg->name, *k) != NULL || strstr(pkg->description, *k) != NULL;
        if (match) break;
        k++;
      }
    } else {
      match = 1;
    }
    if (match) printf("%-15s %s\n", pkg->name, pkg->description);
  }
  delete_pkgdb(&db);

  return 0;
}

static void usage() {
  fprintf(stderr, "usage: pkg [OPTIONS] PACKAGE... \n\n");
  fprintf(stderr, "  -f      Install from package file\n");
  fprintf(stderr, "  -R URL  Package repository URL (default: %s)\n", DEFAULT_PKG_REPO);
  fprintf(stderr, "  -B      Build but do not install packages\n");
  fprintf(stderr, "  -F      Fetch but do not build packages\n");
  fprintf(stderr, "  -u      Upgrade packages if newer versions are available\n");
  fprintf(stderr, "  -U      Force update of packages\n");
  fprintf(stderr, "  -d      Delete package\n");
  fprintf(stderr, "  -l      List installed packages\n");
  fprintf(stderr, "  -q      Query packages in repository\n");
  fprintf(stderr, "  -v      Output verbose installation information\n");
  exit(1);
}

int main(int argc, char *argv[]) {
  struct pkgdb db;
  int c;
  int i;
  int rc;
  int remove = 0;
  int list = 0;
  int query = 0;
  int options = 0;

  // Parse command line options
  memset(&db, 0, sizeof(struct pkgdb));
  db.repo = DEFAULT_PKG_REPO;
  while ((c = getopt(argc, argv, "dflquvBFR:U?")) != EOF) {
    switch (c) {
      case 'd':
        remove = 1;
        break;

      case 'f':
        options |= FROM_FILE;
        break;

      case 'l':
        list = 1;
        break;

      case 'q':
        query = 1;
        break;

      case 'u':
        options |= UPGRADE;
        break;

      case 'v':
        options |= VERBOSE;
        break;

      case 'B':
        options |= ONLY_BUILD;
        break;

      case 'F':
        options |= ONLY_FETCH;
        break;

      case 'R':
        db.repo = optarg;
        break;

      case 'U':
        options |= UPDATE;
        break;

      case '?':
      default:
        usage();
    }
  }
  if (optind == argc && !list && !query && !(options & UPGRADE)) usage();

  // Read package database
  read_pkgdb(&db);

  // Get package timestamps from repository
  if (options & UPGRADE) {
    if (retrieve_package_timestamps(&db, db.repo) != 0) return 1;
  }

  if (list) {
    // List installed packages
    struct pkg *pkg;
    for (pkg = db.head; pkg; pkg = pkg->next) {
      if (options & VERBOSE) {
        printf("%-15s %s\n", pkg->name, pkg->description ? pkg->description : "");
      } else if (!(options & UPGRADE) || pkg->avail > pkg->time) {
        puts(pkg->name);
      }
    }
    rc = 0;
  } else if (query) {
    // Query package repository
    rc = query_repository(argv + optind, db.repo);
  } else if ((options & UPGRADE) && optind == argc) {
    // Upgrade all packages
    struct pkg *pkg;
    for (pkg = db.head; pkg; pkg = pkg->next) {
      rc = install_package(pkg->name, &db, options);
      if (rc != 0) break;
    }
  } else {
    // Install/remove packages
    for (i = optind; i < argc; i++) {
      if (remove) {
        rc = remove_package(argv[i], &db, options);
      } else {
        rc = install_package(argv[i], &db, options);
      }
      if (rc != 0) break;
    }
  }

  // Write package database
  if (rc == 0 && db.dirty) write_pkgdb(&db);
  delete_pkgdb(&db);

  return rc;
}
