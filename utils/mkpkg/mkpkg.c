#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <utime.h>
#include <sys/stat.h>
#include "inifile.h"

#define STRLEN 1024
#define TAR_BLKSIZ 512

struct tarhdr {
  char name[100];
  char mode[8];
  char uid[8];
  char gid[8];
  char size[12];
  char mtime[12];
  char chksum[8];
  char typeflag;
  char linkname[100];
  char magic[6];
  char version[2];
  char uname[32];
  char gname[32];
  char devmajor[8];
  char devminor[8];
  char prefix[155];
  char padding[12];
};

struct pkg {
  char *name;
  char *description;
  char *inffile;
  struct section *manifest;
  int removed;
  int time;
  struct pkg *next;
};

struct pkgdb {
  struct pkg *head;
  struct pkg *tail;
  char *repo;
  int dirty;
};

char *srcdir;
char *dstdir;

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

static int read_pkgdb(char *dbfile, struct pkgdb *db) {
  FILE *f;
  char line[STRLEN];

  f = fopen(dbfile, "r");
  if (!f) return 0;
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
      struct pkg *pkg = add_package(db, name);
      if (inffile) pkg->inffile = strdup(inffile);
      if (description) pkg->description = strdup(description);
      if (time) pkg->time = atoi(time);
    }
  }
  fclose(f);
  db->dirty = 0;
  return 0;
}

static int write_pkgdb(char *dbfile, struct pkgdb *db) {
  FILE *f;
  struct pkg *pkg;

  f = fopen(dbfile, "w");
  if (!f) {
    perror(dbfile);
    return 1;
  }
  for (pkg = db->head; pkg; pkg = pkg->next) {
    if (!pkg->removed) {
      fprintf(f, "%s\t%s\t%s\t%d\n", 
        pkg->name, 
        pkg->inffile ? pkg->inffile : "", 
        pkg->description ? pkg->description : "", 
        pkg->time);
    }
  }
  fclose(f);
  db->dirty = 0;

  return 0;
}

char *joinpath(char *dir, char *filename, char *path) {
  int dirlen = strlen(dir);
  while (dirlen > 0 && dir[dirlen - 1] == '/') dirlen--;
  while (*filename == '/') filename++;
  memcpy(path, dir, dirlen);
  path[dirlen] = '/';
  strcpy(path + dirlen + 1, filename);
  return path;
}

int add_file(FILE *archive, char *srcfn, char *dstfn, int *time, int prebuilt) {
  struct stat st;

  //printf(" Adding %s from %s\n", dstfn, srcfn);
  if (stat(srcfn, &st) < 0) {
    perror(srcfn);
    return 1;
  }

  if (S_ISDIR(st.st_mode)) {
    struct dirent *dp;
    DIR *dirp;
    char subsrcfn[STRLEN];
    char subdstfn[STRLEN];
    int rc;

    dirp = opendir(srcfn);
    if (!dirp) {
      perror(srcfn);
      return 1;
    }
    while ((dp = readdir(dirp))) {
      if (strcmp(dp->d_name, ".") == 0) continue;
      if (strcmp(dp->d_name, "..") == 0) continue;
      joinpath(srcfn, dp->d_name, subsrcfn);
      joinpath(dstfn, dp->d_name, subdstfn);
      rc = add_file(archive, subsrcfn, subdstfn, time, prebuilt);
      if (rc != 0) {
        closedir(dirp);
        return 1;
      }
    }
    closedir(dirp);
  } else {
    unsigned int chksum;
    int n;
    FILE *f;
    unsigned char blk[TAR_BLKSIZ];
    struct tarhdr *hdr = (struct tarhdr *) blk;
    memset(blk, 0, sizeof(blk));

    while (*dstfn == '/') dstfn++;
    strcpy(hdr->name, dstfn);
    sprintf(hdr->mode, "%07o", st.st_mode);
    sprintf(hdr->uid, "%07o", 0);
    sprintf(hdr->gid, "%07o", 0);
    sprintf(hdr->size, "%011o", st.st_size);
    sprintf(hdr->mtime, "%011o", prebuilt ? *time : st.st_mtime);
    memcpy(hdr->chksum, "        ", 8);
    hdr->typeflag = '0';
    strcpy(hdr->magic, "ustar  ");
    strcpy(hdr->uname, "sanos");
    strcpy(hdr->gname, "sanos");

    chksum = 0;
    for (n = 0; n < TAR_BLKSIZ; n++) chksum += blk[n];
    sprintf(hdr->chksum, "%06o", chksum);

    if (fwrite(blk, 1, TAR_BLKSIZ, archive) < 0) {
      perror("write");
      return 1;
    }
    f = fopen(srcfn, "r");
    if (!f) {
      perror(srcfn);
      return 1;
    }
    while ((n = fread(blk, 1, TAR_BLKSIZ, f)) > 0) {
      if (n < TAR_BLKSIZ) memset(blk + n, 0, TAR_BLKSIZ - n);
      if (fwrite(blk, 1, TAR_BLKSIZ, archive) < 0) {
        perror("write");
        fclose(f);
        return 1;
      }
    }
    fclose(f);
    if (!prebuilt) {
      if (*time == 0 || st.st_mtime > *time) *time = st.st_mtime;
    }
  }

  return 0;
}

int make_package(struct pkgdb *db, char *inffn) {
  struct section *manifest;
  struct section *source;
  struct section *prebuilt;
  struct pkg *pkg;
  char *pkgname;
  char *description;
  char dstinffn[STRLEN];
  char dstpkgfn[STRLEN];
  char srcfn[STRLEN];
  unsigned char zero[TAR_BLKSIZ];
  struct utimbuf times;
  FILE *archive;
  int rc;

  manifest = read_properties(inffn);
  if (!manifest) {
    fprintf(stderr, "Error reading manifest from %s\n", inffn);
    return 1;
  }

  pkgname = get_property(manifest, "package", "name", NULL);
  description = get_property(manifest, "package", "description", NULL);

  strcpy(dstinffn, "/usr/share/pkg/");
  strcat(dstinffn, pkgname);
  strcat(dstinffn, ".inf");

  pkg = find_package(db, pkgname);
  if (!pkg) pkg = add_package(db, pkgname);
  free(pkg->description);
  free(pkg->inffile);
  pkg->description = description ? strdup(description) : NULL;
  pkg->manifest = manifest;
  pkg->inffile = strdup(dstinffn);
  pkg->time = 0;
  db->dirty = 1;

  if (strcmp(dstdir, "-") == 0) return 0;

  joinpath(dstdir, pkgname, dstpkgfn);
  strcat(dstpkgfn, ".pkg");

  archive = fopen(dstpkgfn, "w");
  if (add_file(archive, inffn, pkg->inffile, &pkg->time, 0) != 0) {
    fclose(archive);
    unlink(dstpkgfn);
    return 1;
  }

  source = find_section(pkg->manifest, "source");
  if (source) {
    struct property *p;
    for (p = source->properties; p; p = p->next) {
      joinpath(srcdir, p->name, srcfn);
      rc = add_file(archive, srcfn, p->name, &pkg->time, 0);
      if (rc != 0) {
        fclose(archive);
        unlink(dstpkgfn);
        return 1;
      }
    }
  }

  prebuilt = find_section(pkg->manifest, "prebuilt");
  if (prebuilt) {
    struct property *p;
    for (p = prebuilt->properties; p; p = p->next) {
      joinpath(srcdir, p->name, srcfn);
      rc = add_file(archive, srcfn, p->name, &pkg->time, 1);
      if (rc != 0) {
        fclose(archive);
        unlink(dstpkgfn);
        return 1;
      }
    }
  }

  memset(zero, 0, TAR_BLKSIZ);
  fwrite(zero, 1, TAR_BLKSIZ, archive);
  fwrite(zero, 1, TAR_BLKSIZ, archive);
  fclose(archive);
  
  times.actime = times.modtime = pkg->time;
  utime(dstpkgfn, &times);

  return 0;
}

int main(int argc, char *argv[]) {
  int i;
  char dbfile[STRLEN];
  struct pkgdb db;

  if (argc < 3) {
    fprintf(stderr, "usage: mkpkg SRCDIR DSTDIR INFFILE...\n");
    return 1;
  }
  
  memset(&db, 0, sizeof(struct pkgdb));
  srcdir = argv[1];
  dstdir = argv[2];

  if (strcmp(dstdir, "-") == 0) {
    strcpy(dbfile, "db");
  } else {
    joinpath(dstdir, "db", dbfile);
  }
  read_pkgdb(dbfile, &db);
  
  for (i = 3; i < argc; i++) {
    //printf("Generating package %s\n", argv[i]);
    if (make_package(&db, argv[i]) != 0) return 1;
  }
  write_pkgdb(dbfile, &db);

  return 0;  
}
