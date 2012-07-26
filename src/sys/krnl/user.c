//
// user.c
//
// User management
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

int getuid() {
  return self()->ruid;
}

int setuid(uid_t uid) {
  struct thread *thread = self();

  if (thread->euid == 0) {
    thread->ruid = thread->euid = uid;
  } else if (uid == thread->ruid) {
    thread->euid = uid;
  } else {
    return -EPERM;
  }

  return 0;
}

int getgid() {
  return self()->rgid;
}

int setgid(gid_t gid) {
  struct thread *thread = self();

  if (thread->euid == 0) {
    thread->rgid = thread->egid = gid;
  } else if (gid == thread->rgid) {
    thread->egid = gid;
  } else {
    return -EPERM;
  }

  return 0;
}

int geteuid() {
  return self()->euid;
}

int seteuid(uid_t uid) {
  struct thread *thread = self();

  if (thread->euid == 0 || uid == thread->ruid) {
    thread->euid = uid;
  } else {
    return -EPERM;
  }

  return 0;
}

int getegid() {
  return self()->egid;
}

int setegid(gid_t gid) {
  struct thread *thread = self();

  if (thread->euid == 0 || gid == thread->rgid) {
    thread->egid = gid;
  } else {
    return -EPERM;
  }

  return 0;
}

int getgroups(int size, gid_t *list) {
  struct thread *thread = self();

  if (!list || size < thread->ngroups) return -EINVAL;
  memcpy(list, thread->groups, thread->ngroups * sizeof(gid_t));
  return thread->ngroups;
}

int setgroups(int size, gid_t *list) {
  struct thread *thread = self();

  if (thread->euid != 0) return -EPERM;
  if (!list || size < 0 || size > NGROUPS_MAX) return -EINVAL;
  thread->ngroups = size;
  memcpy(thread->groups, list, size * sizeof(gid_t));

  return 0;
}

int check(int mode, uid_t uid, gid_t gid, int access) {
  struct thread *thread = self();

  if (thread->euid == 0) return 0;

  if (thread->euid != uid) {
    access >>= 3;
    if (thread->egid != gid) access >>= 3;
  }

  if ((mode & access) == access) return 0;

  return -EACCES;
}
