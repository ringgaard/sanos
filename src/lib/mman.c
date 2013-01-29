//
// mman.c
//
// Memory-mapped files
//
// Copyright (C) 2013 Michael Ringgaard. All rights reserved.
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
#include <sys/mman.h>

static int map_protect(int prot) {
  switch (prot) {
    case PROT_NONE: 
      return PAGE_NOACCESS;

    case PROT_READ: 
      return PAGE_READONLY;

    case PROT_WRITE:
    case PROT_READ | PROT_WRITE:
      return PAGE_READWRITE;

    case PROT_EXEC:
      return PAGE_EXECUTE;

    case PROT_EXEC | PROT_READ:
      return PAGE_EXECUTE_READ;

    case PROT_EXEC | PROT_WRITE:
    case PROT_EXEC | PROT_READ | PROT_WRITE:
      return PAGE_EXECUTE_READWRITE;
  }
  
  return -1;
}

void *mmap(void *addr, size_t size, int prot, int flags, handle_t f, off_t offset) {
  if (flags & MAP_ANONYMOUS) {
    addr = vmalloc(addr, size, MEM_RESERVE | MEM_COMMIT, map_protect(prot), 'MMAP');
  } else {
    addr = vmmap(addr, size, map_protect(prot), f, offset);
    if (!addr) return MAP_FAILED;
  }

  return addr;
}

int munmap(void *addr, size_t size) {
  return vmfree(addr, size, MEM_DECOMMIT | MEM_RELEASE);
}

int msync(void *addr, size_t size, int flags) {
  return vmsync(addr, size);
}

int mprotect(void *addr, size_t size, int prot) {
  return vmprotect(addr, size, map_protect(prot));
}

int mlock(const void *addr, size_t size) {
  return vmlock((void *) addr, size);
}

int munlock(const void *addr, size_t size) {
  return vmunlock((void *) addr, size);
}

int mlockall(int flags) {
  errno = ENOSYS;
  return -1;
}

int munlockall(void) {
  errno = ENOSYS;
  return -1;
}
