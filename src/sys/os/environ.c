//
// environ.c
//
// Environment variables
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
#include <string.h>
#include <inifile.h>

extern struct critsect env_lock;

//
// The environment block is a null terminated array of strings 
// of the form NAME=VALUE.
//

char ***_environ() {
  return &gettib()->proc->env;
}

static char *findenv(char **env, const char *name, int *offset) {
  int i;
  int len;

  if (!env || !name) return NULL;

  len = strlen(name);
  for (i = 0; env[i]; i++) {
    if (strncmp(env[i], name, len) == 0 && env[i][len] == '=') {
      if (offset) *offset = i;
      return env[i] + len + 1;
    }
  }

  return NULL;
}

char **copyenv(char **env) {
  int n;
  char **newenv;

  if (!env) return NULL;

  enter(&env_lock);
  for (n = 0; env[n]; n++);
  newenv = (char **) malloc((n + 1) * sizeof(char *));
  if (newenv) {
    newenv[n] = NULL;
    for (n = 0; env[n]; n++) newenv[n] = strdup(env[n]);
  }
  leave(&env_lock);

  return newenv;
}

void freeenv(char **env) {
  int n;
  
  if (!env) return;
  enter(&env_lock);
  for (n = 0; env[n]; n++) free(env[n]);
  free(env);
  leave(&env_lock);
}

char **initenv(struct section *sect) {
  int n;
  char **env;
  struct property *prop;

  if (!sect) return NULL;
  n = get_section_size(sect);
  env = (char **) malloc((n + 1) * sizeof(char *));
  if (!env) return NULL;
  env[n] = NULL;

  for (n = 0, prop = sect->properties; prop; n++, prop = prop->next) {
    int len = strlen(prop->name) + 1;
    if (prop->value) len += strlen(prop->value);
    env[n] = (char *) malloc(len + 1);
    if (env[n]) {
      strcpy(env[n], prop->name);
      strcat(env[n], "=");
      if (prop->value) strcat(env[n], prop->value);
    }
  }

  return env;
}

char *getenv(const char *name) {
  char *value;

  enter(&env_lock);
  value = findenv(environ, name, NULL);
  leave(&env_lock);

  return value;
}

int setenv(const char *name, const char *value, int rewrite) {
  char **env;
  char *p;
  char *buf;
  int offset;
  size_t len;

  if (!name) {
    errno = EINVAL;
    return 0;
  }

  for (p = (char *) name; *p && *p != '='; p++);
  if (*p == '=') {
    errno = EINVAL;
    return 0;
  }

  if (*value == '=') value++;

  enter(&env_lock);
  env = environ;

  len = strlen(value);
  if ((p = findenv(env, name, &offset))) {
    if (!rewrite) {
      leave(&env_lock);
      return 0;
    }

    if (strlen(p) >= len) {
      // Old value is larger; copy over
      while ((*p++ = *value++) != 0);
      leave(&env_lock);
      return 0;
    }
  } else {
    int n;

    for (n = 0; env && env[n]; n++);
    env = (char **) realloc(env, (n + 2) * sizeof(char *));
    if (!env) {
      leave(&env_lock);
      return -1;
    }
    env[n + 1] = NULL;
    offset = n;
    environ = env;
  }

  buf = (char *) malloc(strlen(name) + len + 2);
  if (!buf) {
    leave(&env_lock);
    return -1;
  }

  env[offset] = buf;
  p = (char *) name;
  while (*p) *buf++ = *p++;
  *buf++ = '=';
  p = (char *) value;
  while (*p) *buf++ = *p++;
  *buf = 0;

  leave(&env_lock);

  return 0;
}

void unsetenv(const char *name) {
  char **p;
  char **env;
  int offset;

  enter(&env_lock);
  env = environ;
  
  while (findenv(env, name, &offset)) {
    for (p = &(env[offset]);; p++) {
      if (!(*p = *(p + 1))) break;
    }
  }

  leave(&env_lock);
}

int putenv(const char *str) {
  char *p, *equal;
  int rc;

  p = strdup(str);
  if (!p) return 1;

  if ((equal = strchr(p, '='))) {
    *equal = '\0';
    rc = setenv(p, equal + 1, 1);
  } else {
    unsetenv(p);
    rc = 0;
  }

  free(p);
  return rc;
}
