//
// smbcache.c
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// SMB directory entry cache
//

#include <os/krnl.h>
#include "smb.h"

void smb_add_to_cache(struct smb_share *share, char *path, char *filename, struct stat *statbuf)
{
  int idx = share->next_cacheidx;

  strcpy(share->dircache[idx].path, path);
  strcat(share->dircache[idx].path, "\\");
  strcat(share->dircache[idx].path, filename);

  memcpy(&share->dircache[idx].statbuf, statbuf, sizeof(struct stat));

  if (++share->next_cacheidx == SMB_DENTRY_CACHESIZE) share->next_cacheidx = 0;
}

struct smb_dentry *smb_find_in_cache(struct smb_share *share, char *path)
{
  int idx;

  if (share->dircache[share->next_cacheidx].path[0] && strcmp(share->dircache[share->next_cacheidx].path, path) == 0) return &share->dircache[share->next_cacheidx];

  for (idx = 0; idx < SMB_DENTRY_CACHESIZE; idx++)
  {
    if (share->dircache[idx].path[0] && strcmp(share->dircache[idx].path, path) == 0)
    {
      return &share->dircache[idx];
    }
  }
  
  return NULL;
}

void smb_clear_cache(struct smb_share *share)
{
  int idx;

  for (idx = 0; idx < SMB_DENTRY_CACHESIZE; idx++) share->dircache[idx].path[0] = 0;
}
