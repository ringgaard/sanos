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
#include <netdb.h>
#include <inifile.h>
#include <shlib.h>
#include <dirent.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/stat.h>

#define STRLEN 1024
#define DEFAULT_PKG_REPO "http://www.jbox.dk/pkg"
#define PKGDB_FILE       "/usr/share/pkg/db"

struct pkg {
  char *name;
  char *inffile;
  struct section *manifest;
  int removed;
  struct pkg *next;
};

struct pkgdb {
  struct pkg *head;
  struct pkg *tail;
  char *repo;
  int dirty;
  int verbose;
  int update;
  int onlyfetch;
  int onlybuild;
};

static int parse_url(char *url, char **host, int *port, char **path) {
  *host = NULL;
  *port = 80;
  *path = NULL;

  // Parse protocol
  if (strncmp(url, "file:", 5) == 0) {
    *port = -1;
    url += 5;
  } else if (strncmp(url, "http:", 5) == 0) {
    url += 5;
  } else if (strncmp(url, "https:", 6) == 0 || strncmp(url, "ftp:", 4)) {
    return -1;
  }

  // Parse server and port
  if (strncmp(url, "//", 2) == 0) {
    char *start;
    int len;

    url += 2;
    start = url;
    while (*url && *url != '/' && *url != ':') url++;
    len = url - start;
    *host = malloc(len + 1);
    memcpy(*host, start, len);
    (*host)[len] = 0;

    if (*url == ':') {
      *port = 0;
      url++;
      while (*url && *url != '/') {
        if (*url < '0' || *url > '9') return -1;
        *port = *port * 10 + (*url++ - '0');
      }
    }
  }

  // The remaining URL is the path
  *path = strdup(*url ? url : "/");

  return 0;
}

static FILE *open_url(char *url) {
  char *host;
  int port;
  char *path;
  struct hostent *hp;
  struct sockaddr_in sin;
  int s;
  int rc;
  FILE *f;
  int status;
  char header[STRLEN];

  // Parse URL  
  if (parse_url(url, &host, &port, &path) < 0) {
    fprintf(stderr, "%s: invalid url\n", url);
    free(host);
    free(path);
    return NULL;
  }

  // Check for file-based URL
  if (port == -1) {
    f = fopen(path, "r");
    if (!f) perror(path);
    free(host);
    free(path);
    return f;
  }

  // Lookup host
  hp = gethostbyname(host);
  if (!hp) {
    perror(host);
    free(host);
    free(path);
    return NULL;
  }
  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  memcpy(&sin.sin_addr, hp->h_addr_list[0], hp->h_length);
  sin.sin_port = htons(port);

  // Connect to host
  s = socket(AF_INET, SOCK_STREAM, 0);
  if (s < 0) {
    perror(host);
    free(host);
    free(path);
    return NULL;
  }

  rc = connect(s, (struct sockaddr *) &sin, sizeof(sin));
  if (rc  < 0) {
    perror(host);
    free(path);
    close(s);
    return NULL;
  }

  // Open file for socket
  f = fdopen(s, "a+");
  if (!f) {
    perror(host);
    free(host);
    free(path);
    close(s);
    return NULL;
  }

  // Send request to host
  fprintf(f, "GET %s HTTP/1.0\r\nHost: %s\r\n\r\n", path, host);
  fseek(f, 0, SEEK_CUR);

  // Read headers
  status = -1;
  while (fgets(header, sizeof(header), f)) {
    // Remove trailing cr/nl
    char *p = header;
    while (*p && *p != '\r' && *p != '\n') p++;
    *p = 0;

    // Check status code
    if (status == -1) {
      p = header;
      if (strncmp(p, "HTTP/1.0 ", 9) == 0 || strncmp(p, "HTTP/1.1 ", 9) == 0) p += 9;
      status = atoi(p);
      if (status != 200) fprintf(stderr, "%s: %s\n", url, p);
    }
    
    // Check for end of header
    if (!*header) break;
    //printf("hdr: %s\n", header);
  }

  // Check status code
  if (status != 200) {
    if (status == -1) {
      fprintf(stderr, "%s: invalid response from server\n", url);
    }
    free(host);
    free(path);
    fclose(f);
    return NULL;
  }

  // Return file
  free(host);
  free(path);
  return f;
}

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

static int extract_files(char *url, FILE *f, char *manifest, int verbose) {
  unsigned char buffer[TAR_BLKSIZ];
  unsigned char fn[FILENAME_MAX];
  struct tarhdr *hdr;
  unsigned int checksum;
  unsigned int sum;
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
    if (verbose) printf("extracting  '%s' size: %d mode: %o\n", fn, size, mode);

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

static struct pkg *add_package(struct pkgdb *db, char *name) {
  struct pkg *pkg = calloc(1, sizeof(struct pkg));
  pkg->name = strdup(name);
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

static int read_pkgdb(struct pkgdb *db) {
  FILE *f;
  char line[STRLEN];

  f = fopen(PKGDB_FILE, "r");
  if (!f) return 0;
  while (fgets(line, STRLEN, f)) {
    char *p = line;
    char *name = NULL;
    char *inffile = NULL;
    name = strsep(&p, "\t\n");
    inffile = strsep(&p, "\t\n");

    if (name) {
      struct pkg *pkg = add_package(db, name);
      if (inffile) pkg->inffile = strdup(inffile);
    }
  }
  fclose(f);
  db->dirty = 0;
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
      fprintf(f, "%s\t%s\n", pkg->name, pkg->inffile);
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
    free_properties(pkg->manifest);
    free(pkg);
    pkg = next;
  }
  db->head = db->tail = NULL;
}

static int install_package(char *pkgname, struct pkgdb *db, int file, int dependency) {
  char url[STRLEN];
  FILE *f;
  int rc;
  char inffile[STRLEN];
  struct section *manifest;
  struct section *dep;
  struct section *build;
  struct pkg *pkg;

  // Check if package is already installed
  if (!file) {
    if (find_package(db, pkgname)) {
      if (dependency) return 0;
      if (db->update) {
        if (db->verbose) printf("updating package %s\n", pkgname);
      } else {
        fprintf(stderr, "%s: is already installed\n", pkgname);
        return 1;
      }
    }
  }

  // Open package file
  printf("Fetching %s\n", pkgname);
  if (file) {
    snprintf(url, sizeof(url), "file:///%s", pkgname);
  } else {
    snprintf(url, sizeof(url), "%s/%s.pkg", db->repo, pkgname);
  }
  if (db->verbose) printf("fetching package %s from %s\n", pkgname, url);
  f = open_url(url);
  if (!f) return 1;

  // Extract file from package file
  rc = extract_files(url, f, inffile, db->verbose);
  fclose(f);
  if (rc != 0) return rc;

  // Read manifest
  if (db->verbose) printf("reading manifest from %s\n", inffile);
  manifest = read_properties(inffile);
  if (!manifest) {
    fprintf(stderr, "%s: unable to read manifest\n", inffile);
    return 1;
  }

  // Add package to package database
  if (db->verbose) printf("adding package %s to database\n", pkgname);
  pkgname = get_property(manifest, "package", "name", pkgname);
  pkg = add_package(db, pkgname);
  pkg->inffile = strdup(inffile);
  pkg->manifest = manifest;

  // Install package dependencies
  dep = find_section(manifest, "dependencies");
  if (dep) {
    struct property *p;
    for (p = dep->properties; p; p = p->next) {
      if (db->verbose) printf("package %s depends on %s\n", pkgname, p->name);
      rc = install_package(p->name, db, 0, 1);
      if (rc != 0) return rc;
    }
  }
  if (db->onlyfetch && !dependency) return 0;

  // Run package build/installation commands
  if (!db->onlyfetch || dependency) {
    build = find_section(manifest, db->onlybuild && !dependency ? "build" : "install");
    if (build) {
      struct property *p;
      printf(db->onlybuild && !dependency ? "Building %s\n" : "Installing %s\n", pkgname);
      for (p = build->properties; p; p = p->next) {
        if (db->verbose) printf("%s\n", p->name);
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

static void remove_path(char *path, int verbose) {
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
    int rc;

    // Remove files in directory recursively
    dirp = opendir(path);
    if (!dirp) {
      perror(path);
      return;
    }
    while ((dp = readdir(dirp))) {
      fn = join_path(path, dp->d_name);
      if (!fn) {
        fprintf(stderr, "error: out of memory\n");
        closedir(dirp);
        return;
      }
      remove_path(fn, verbose);
    }
    closedir(dirp);

    // Remove directory
    if (verbose) printf("removing directory %s\n", path);
    if (rmdir(path) < 0) perror(path);
  } else {
    // Remove file
    if (verbose) printf("removing file %s\n", path);
    if (unlink(path) < 0) perror(path);
  }
}

static int remove_package(char *pkgname, struct pkgdb *db) {
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
    if (db->verbose) printf("reading manifest from %s\n", pkg->inffile);
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
      remove_path(p->name, db->verbose);
    }
  }

  // Remove manifest file
  unlink(pkg->inffile);

  // Remove package from package database
  pkg->removed = 1;
  db->dirty = 1;

  return 0;
}

static int query_repository(char **keywords, struct pkgdb *db) {
  char url[STRLEN];
  char line[STRLEN];
  FILE *f;

  snprintf(url, sizeof(url), "%s/packages.lst", db->repo);
  f = open_url(url);
  if (!f) return 1;

  while (fgets(line, sizeof(line), f)) {
    int match = 0;
    if (*keywords) {
      char **k = keywords;
      while (*k) {
        if (strstr(line, *k++)) {
          match = 1;
          break;
        }
      }
    } else {
      match = 1;
    }
    if (match) fputs(line, stdout);
  }
  fclose(f);

  return 0;
}

static void usage() {
  fprintf(stderr, "usage: pkg [OPTIONS] PACKAGE... \n\n");
  fprintf(stderr, "  -f      Install from package file\n");
  fprintf(stderr, "  -R URL  Package repository URL (default: %s)\n", DEFAULT_PKG_REPO);
  fprintf(stderr, "  -B      Build but do not install packages\n");
  fprintf(stderr, "  -F      Fetch but do not build packages\n");
  fprintf(stderr, "  -u      Update packages\n");
  fprintf(stderr, "  -d      Delete package\n");
  fprintf(stderr, "  -l      List installed packages\n");
  fprintf(stderr, "  -q      Query packages in repository\n");
  fprintf(stderr, "  -v      Output verbose installation information\n");
}

int main(int argc, char *argv[]) {
  struct pkgdb db;
  int c;
  int i;
  int rc;
  int file = 0;
  int remove = 0;
  int list = 0;
  int query = 0;

  // Parse command line options
  memset(&db, 0, sizeof(struct pkgdb));
  db.repo = DEFAULT_PKG_REPO;
  while ((c = getopt(argc, argv, "dflquvBFR:?")) != EOF) {
    switch (c) {
      case 'd':
        remove = 1;
        break;

      case 'f':
        file = 1;
        break;

      case 'l':
        list = 1;
        break;

      case 'q':
        query = 1;
        break;

      case 'u':
        db.update = 1;
        break;

      case 'v':
        db.verbose = 1;
        break;

      case 'B':
        db.onlybuild = 1;
        break;

      case 'F':
        db.onlyfetch = 1;
        break;

      case 'R':
        db.repo = optarg;
        break;

      case '?':
      default:
        usage();
    }
  }
  if (optind == argc && !list && !query) usage();

  // Read package database
  read_pkgdb(&db);

  if (list) {
    // List installed packages
    struct pkg *pkg;
    for (pkg = db.head; pkg; pkg = pkg->next) puts(pkg->name);
    rc = 0;
  } else if (query) {
    // Query package repository
    rc = query_repository(argv + optind, &db);
  } else {
    // Install/remove packages
    for (i = optind; i < argc; i++) {
      if (remove) {
        rc = remove_package(argv[i], &db);
      } else {
        rc = install_package(argv[i], &db, file, 0);
      }
      if (rc != 0) break;
    }
  }

  // Write package database
  if (rc == 0 && db.dirty) write_pkgdb(&db);
  delete_pkgdb(&db);

  return rc;
}
