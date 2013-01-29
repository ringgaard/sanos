//
// hndl.c
//
// Object handle manager
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

#define HANDLES_PER_PAGE (PAGESIZE / sizeof(handle_t))

handle_t *htab = (handle_t *) HTABBASE;
handle_t hfreelist = HEND;
int htabsize = 0;

static int expand_htab() {
  unsigned long pfn;
  handle_t h;

  if (htabsize == HTABSIZE / sizeof(struct object *)) return -ENFILE;
  pfn = alloc_pageframe('HTAB');
  map_page(htab + htabsize, pfn, PT_WRITABLE | PT_PRESENT);

  for (h = htabsize + HANDLES_PER_PAGE - 1; h >= htabsize; h--) {
    htab[h] = hfreelist;
    hfreelist = h;
  }

  htabsize += HANDLES_PER_PAGE;
  return 0;
}

static int remove_from_freelist(handle_t h) {
  if (h == hfreelist) {
    hfreelist = htab[h];
    return 0;
  } else  {
    handle_t fl = hfreelist;

    while (fl != HEND) {
      if (h == htab[fl]) {
        htab[fl] = htab[h];
        return 0;
      }
    }

    return -EBADF;
  }
}

//
// hlookup
//
// Lookup object in handle table
//

struct object *hlookup(handle_t h) {
  if (h < 0 || h >= htabsize) return NULL;
  if (!HUSED(htab[h])) return NULL;
  return HOBJ(htab[h]);
}

//
// halloc
//
// Allocate handle
//

handle_t halloc(struct object *o) {
  handle_t h;
  int rc;

  // Expand handle table if full
  if (hfreelist == HEND) {
    rc = expand_htab();
    if (rc < 0) return rc;
  }

  h = hfreelist;
  hfreelist = htab[h];
  htab[h] = (handle_t) o;
  o->handle_count++;

  return h;
}

//
// hassign
//
// Assign handle to object. If the handle is in use the handle is closed 
// before beeing assigned
//

int hassign(struct object *o, handle_t h) {
  int rc;

  if (h > HTABSIZE / sizeof(handle_t)) return -EBADF;

  while (htabsize <= h) {
    rc = expand_htab();
    if (rc < 0) return rc;
  }

  if (!HUSED(htab[h])) {
    // Not allocated, remove from freelist
    rc = remove_from_freelist(h);
    if (rc < 0) return rc;
  } else {
    // Handle already allocated, free handle
    struct object *oo = HOBJ(htab[h]);
    if (HPROT(htab[h])) return -EACCES;
    if (--oo->handle_count == 0) {
      rc = close_object(oo);
      if (oo->lock_count == 0) destroy_object(oo);
      if (rc < 0) return rc;
    }
  }

  // Assign handle to object
  htab[h] = (handle_t) o;
  o->handle_count++;

  return 0;
}

//
// hfree
//
// Free handle
//

int hfree(handle_t h) {
  struct object *o;
  int rc;

  o = hlookup(h);
  if (!o) return -EBADF;
  if (HPROT(htab[h])) return -EACCES;

  htab[h] = hfreelist;
  hfreelist = h;

  if (--o->handle_count > 0) return 0;
  
  rc = close_object(o);

  if (o->lock_count == 0) destroy_object(o);

  return rc;
}

//
// hprotect
//
// Protect handle from being closed
//

int hprotect(handle_t h) {
  if (!hlookup(h)) return -EBADF;
  htab[h] |= HPROTECT;
  return 0;
}

//
// hunprotect
//
// Remove protection handle
//

int hunprotect(handle_t h) {
  if (!hlookup(h)) return -EBADF;
  htab[h] &= ~HPROTECT;
  return 0;
}

//
// olock
//
// Lock object
//

struct object *olock(handle_t h, int type) {
  struct object *o;

  o = hlookup(h);
  if (!o) return NULL;
  if (o->type != type && type != OBJECT_ANY) return NULL;
  if (o->handle_count == 0) return NULL;
  o->lock_count++;
  return o;
}

//
// orel
//
// Release lock on object
//

int orel(object_t hobj) {
  struct object *o = (struct object *) hobj;

  if (--o->lock_count == 0 && o->handle_count == 0) {
    return destroy_object(o);
  } else {
    return 0;
  }
}

//
// handles_proc
//

#define FRAQ 2310

static int handles_proc(struct proc_file *pf, void *arg) {
  static char *objtype[OBJECT_TYPES] = {"THREAD", "EVENT", "TIMER", "MUTEX", "SEM", "FILE", "SOCKET", "IOMUX", "FILEMAP"};

  int h;
  int i;
  struct object *o;
  int objcount[OBJECT_TYPES];

  for (i = 0; i < OBJECT_TYPES; i++) objcount[i] = 0;

  pprintf(pf, "handle addr     s p type    count locks\n");
  pprintf(pf, "------ -------- - - ------- ----- -----\n");
  for (h = 0; h < htabsize; h++) {
    if (!HUSED(htab[h])) continue;
    o = HOBJ(htab[h]);
    
    pprintf(pf, "%6d %8X %d %d %-7s %5d %5d\n", 
      h, o, o->signaled,  HPROT(htab[h]), objtype[o->type], 
      o->handle_count, o->lock_count);

    if (o->handle_count != 0) objcount[o->type] += FRAQ / o->handle_count;
  }

  pprintf(pf, "\n");
  for (i = 0; i < OBJECT_TYPES; i++) {
    if (objcount[i] > 0) {
      pprintf(pf, "%s:%d ", objtype[i], objcount[i] / FRAQ);
    }
  }
  pprintf(pf, "\n");

  return 0;
}

//
// init_handles
//

void init_handles() {
  register_proc_inode("handles", handles_proc, NULL);
}
