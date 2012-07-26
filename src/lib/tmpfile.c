//
// tmpfile.c
//
// Temporary files
//
// Copyright (C) 2011 Michael Ringgaard. All rights reserved.
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

#include <stdio.h>
#include <string.h>

static int gentmpfn(char *path, char *prefix, int unique, char *tempfn) {
  const char *format = "%s%c%s%4.4x.tmp";
  int len;

  len = strlen(path);
  if (len > 0 && (path[len - 1] == PS1 || path[len - 1] == PS2)) len--;

  if (unique == 0) unique = clock();
  
  sprintf(tempfn, format, path, PS1, prefix, unique);
  while (access(tempfn, 0) == 0) {
    unique++;
    sprintf(tempfn, format, path, PS1, prefix, unique);
  }
  if (errno != ENOENT) return -1;

  return unique;
}

FILE *tmpfile() {
  static int unique = 0;

  FILE *stream;
  char *path;
  char tempfn[MAXPATH];
  int rc;

  path = getenv("tmp");
  if (!path) path = "/tmp";

  while (1) {
    rc = gentmpfn(path, "t", unique, tempfn);
    if (rc < 0) return NULL;
    unique = rc;

    stream = fopen(tempfn, "wb+TD");
    if (stream != NULL) break;
    if (errno != EEXIST) return NULL;
  }

  return stream;
}

char *tmpnam(char *string) {
  static int unique = 0;
  char *path;
  char *tempfn;
  int rc;

  if (string) {
    tempfn = string;
  } else {
    tempfn = gettib()->tmpnambuf;
  }
  
  path = getenv("tmp");
  if (!path) path = "/tmp";

  rc = gentmpfn(path, "s", 0, tempfn);
  if (rc < 0) return NULL;
  unique = rc;

  return tempfn;
}

char *tempnam(const char *dir, const char *prefix) {
  static int unique = 0;
  char *path;
  char *tempfn;
  int rc;

  tempfn = gettib()->tmpnambuf;
  
  path = (char *) dir;
  if (!path) path = getenv("tmp");
  if (!path) path = ".";

  rc = gentmpfn(path, (char *) prefix, unique, tempfn);
  if (rc < 0) return NULL;
  unique = rc;

  return strdup(tempfn);
}
