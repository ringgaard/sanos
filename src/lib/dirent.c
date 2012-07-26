//
// dirent.c
//
// List directory entries
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
#include <dirent.h>

DIR *opendir(const char *name) {
  int handle;
  DIR *dirp;

  handle = _opendir(name);
  if (handle < 0) return NULL;

  dirp = (DIR *) malloc(sizeof(DIR));
  if (!dirp) {
    close(handle);
    return NULL;
  }
  memset(dirp, 0, sizeof(DIR));
  dirp->handle = handle;
  strcpy(dirp->path, name);
  return dirp;
}

int closedir(DIR *dirp) {
  int rc;

  if (!dirp) {
    errno = EINVAL;
    return -1;
  }

  rc = close(dirp->handle);
  free(dirp);
  
  return rc;
}

struct dirent *readdir(DIR *dirp) {
  struct direntry dirent;
  int rc;

  if (!dirp) {
    errno = EINVAL;
    return NULL;
  }

  rc = _readdir(dirp->handle, &dirent, 1);
  if (rc <= 0) return NULL;

  dirp->entry.d_ino = dirent.ino;
  dirp->entry.d_namlen = dirent.namelen;
  memcpy(dirp->entry.d_name, dirent.name, dirent.namelen);
  dirp->entry.d_name[dirent.namelen] = 0;

  return &dirp->entry;
}

int readdir_r(DIR *dirp, struct dirent *entry, struct dirent **result) {
  struct direntry dirent;
  int rc;

  if (!dirp || !entry) {
    errno = EINVAL;
    return -1;
  }

  rc = _readdir(dirp->handle, &dirent, 1);
  if (rc <= 0) return -1;

  entry->d_ino = dirent.ino;
  entry->d_namlen = dirent.namelen;
  memcpy(entry->d_name, dirent.name, dirent.namelen);
  entry->d_name[dirent.namelen] = 0;

  *result = entry;
  return 0;
}

int rewinddir(DIR *dirp) {
  if (!dirp) {
    errno = EINVAL;
    return -1;
  }

  close(dirp->handle);
  dirp->handle = _opendir(dirp->path);
  if (dirp->handle < 0) return -1;
  return 0;
}
