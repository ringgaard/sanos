//
// smbcache.c
//
// SMB directory entry cache
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

#include <os/krnl.h>
#include "smb.h"

void smb_add_to_cache(struct smb_share *share, char *path, char *filename, struct stat64 *statbuf) {
  int idx = share->next_cacheidx;

  strcpy(share->dircache[idx].path, path);
  strcat(share->dircache[idx].path, "\\");
  strcat(share->dircache[idx].path, filename);

  memcpy(&share->dircache[idx].statbuf, statbuf, sizeof(struct stat64));

  if (++share->next_cacheidx == SMB_DENTRY_CACHESIZE) share->next_cacheidx = 0;
}

struct smb_dentry *smb_find_in_cache(struct smb_share *share, char *path) {
  int idx;

  if (share->dircache[share->next_cacheidx].path[0] && strcmp(share->dircache[share->next_cacheidx].path, path) == 0) return &share->dircache[share->next_cacheidx];

  for (idx = 0; idx < SMB_DENTRY_CACHESIZE; idx++) {
    if (share->dircache[idx].path[0] && strcmp(share->dircache[idx].path, path) == 0) {
      return &share->dircache[idx];
    }
  }
  
  return NULL;
}

void smb_clear_cache(struct smb_share *share) {
  int idx;

  for (idx = 0; idx < SMB_DENTRY_CACHESIZE; idx++) share->dircache[idx].path[0] = 0;
}
