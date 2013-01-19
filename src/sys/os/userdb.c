//
// userdb.c
//
// User database functions
//
// Copyright (C) 2002 Michael Ringgaard. All rights reserved.
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

#include <os.h>
#include <stdlib.h>
#include <string.h>

#include <pwd.h>
#include <grp.h>

static char *passwd;
static int passwd_len;
static struct passwd *passwdtab;
static int passwd_cnt;

static char *group;
static int group_len;
static struct group *grouptab;
static int group_cnt;

static struct passwd defpasswd = {"root", "", 0, 0, "root", "/", "/bin/sh.exe"};
static char *defgrpmem[] = {"root", NULL};
static struct group defgroup = {"root", "", 0, defgrpmem};

static char *skip(char *p, int c) {
  while (*p && *p != c) p++;
  if (*p) *p++ = 0;
  return p;
}

static char *nextline(char *p) {
  while (*p && *p != '\r' && *p != '\n') p++;
  if (*p == '\r') *p++ = 0;
  if (*p) *p++ = 0;
  return p;
}

static int linecount(char *data) {
  int cnt = 0;
  char *p = data;
  while (*p) if (*p++ == '\n') cnt++;
  if (p > data && *(p - 1) != '\n') cnt++;
  return cnt;
}

static char *read_file(char *name, int *size) {
  int f;
  struct stat st;
  char *data;
  int len;

  // Determine size of file
  if (stat(name, &st) < 0) return NULL;
  len = (int) st.st_size;

  // Allocate memory for file image
  data = vmalloc(NULL, len + 1, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE, 'UDB');
  if (!data) return NULL;

  // Read file into memory
  f = open(name, 0);
  if (f < 0)  {
    vmfree(data, len + 1, MEM_RELEASE);
    return NULL;
  }

  if (read(f, data, len) != len) {
    vmfree(data, len + 1, MEM_RELEASE);
    return NULL;
  }

  data[len] = 0;
  close(f);

  *size = len;
  return data;
}

static int read_passwd() {
  char *p;
  int n;
  int passwdtab_len;

  // Read password file into memory
  passwd = read_file("/etc/passwd", &passwd_len);
  if (!passwd) return -1;

  // Calculate the number of entries in password file
  n = linecount(passwd);

  // Allocate memory for password table
  passwdtab_len = n * sizeof(struct passwd);
  passwdtab = vmalloc(NULL, passwdtab_len, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE, 'UDB');
  if (!passwdtab) return -1;

  // Build password table entries
  p = passwd;
  n = 0;
  while (*p) {
    char *next = nextline(p);

    passwdtab[n].pw_name = p;
    p = skip(p, ':');
    passwdtab[n].pw_passwd = p;
    p = skip(p, ':');
    passwdtab[n].pw_uid = atoi(p);
    p = skip(p, ':');
    passwdtab[n].pw_gid = atoi(p);
    p = skip(p, ':');
    passwdtab[n].pw_gecos = p;
    p = skip(p, ':');
    passwdtab[n].pw_dir = p;
    p = skip(p, ':');
    passwdtab[n].pw_shell = p;

    p = next;
    n++;
  }

  // Protect password memory
  if (vmprotect(passwd, passwd_len, PAGE_READONLY) < 0) return -1;
  if (vmprotect(passwdtab, passwdtab_len, PAGE_READONLY) < 0) return -1;
  passwd_cnt = n;

  return 0;
}

static int read_group() {
  char *p;
  int n, m;
  int grouptab_len;
  int commas;
  char **grmem;

  // Read user group file into memory
  group = read_file("/etc/group", &group_len);
  if (!group) return -1;

  // Calculate the number of entries in user group file
  n = linecount(group);

  // The total number of group members entries is at most the number of commas
  // plus the number of lines
  commas = 0;
  p = group;
  while (*p) if (*p++ == ',') commas++;

  // Allocate memory for user group table
  grouptab_len = n * sizeof(struct group) + (commas + n * 2) * sizeof(char *);
  grouptab = vmalloc(NULL, grouptab_len, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE, 'UDB');
  if (!grouptab) return -1;
  grmem = (char **) (grouptab + n * sizeof(struct group));

  // Build user group table entries
  p = group;
  n = 0;
  m = 0;
  while (*p) {
    char *next = nextline(p);

    grouptab[n].gr_name = p;
    p = skip(p, ':');
    grouptab[n].gr_passwd = p;
    p = skip(p, ':');
    grouptab[n].gr_gid = atoi(p);
    p = skip(p, ':');
    grouptab[n].gr_mem = &grmem[m];
    while (*p) {
      grmem[m++] = p;
      p = skip(p, ',');
    }
    grmem[m++] = NULL;

    p = next;
    n++;
  }

  // Protect group memory
  if (vmprotect(group, passwd_len, PAGE_READONLY) < 0) return -1;
  if (vmprotect(grouptab, grouptab_len, PAGE_READONLY) < 0) return -1;
  group_cnt = n;

  return 0;
}

void init_userdb() {
  read_passwd();
  if (passwd_cnt == 0) {
    passwdtab = &defpasswd;
    passwd_cnt = 1;
  }

  read_group();
  if (group_cnt == 0) {
    grouptab = &defgroup;
    group_cnt = 1;
  }
}

struct passwd *getpwnam(const char *name) {
  int i;

  for (i = 0; i < passwd_cnt; i++) {
    if (strcmp(name, passwdtab[i].pw_name) == 0)  return &passwdtab[i];
  }
  return NULL;
}

struct passwd *getpwuid(uid_t uid) {
  int i;

  for (i = 0; i < passwd_cnt; i++) {
    if (uid == passwdtab[i].pw_uid) return &passwdtab[i];
  }
  return NULL;
}

struct group *getgrnam(const char *name) {
  int i;

  for (i = 0; i < group_cnt; i++) {
    if (strcmp(name, grouptab[i].gr_name) == 0)  return &grouptab[i];
  }
  return NULL;
}

struct group *getgrgid(gid_t gid) {
  int i;

  for (i = 0; i < group_cnt; i++) {
    if (gid == grouptab[i].gr_gid) return &grouptab[i];
  }
  return NULL;
}

int initgroups(const char *user, gid_t basegid) {
  gid_t groups[NGROUPS_MAX];
  int ngroups = 0;
  int i;
  char **mem;

  groups[ngroups++] = basegid;
  for (i = 0; i < group_cnt; i++) {
    if (grouptab[i].gr_gid == basegid) continue;
    mem = grouptab[i].gr_mem;
    while (*mem && strcmp(*mem, user) != 0) mem++;
    if (*mem && ngroups < NGROUPS_MAX) groups[ngroups++] = grouptab[i].gr_gid;
  }

  return setgroups(ngroups, groups);
}
