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

#define HANDLES_PER_PAGE (PAGESIZE / sizeof(struct object *))

struct object **htab = (struct object **) HTABBASE;
handle_t hfreelist = NOHANDLE;
int htabsize = 0;

//
// halloc
//
// Allocate handle
//

handle_t halloc(struct object *o)
{
  handle_t h;

  // Expand handle table if full
  if (hfreelist == NOHANDLE)
  {
    unsigned long pfn;

    if (htabsize == HTABSIZE / sizeof(struct object *)) return -ENFILE;
    pfn = alloc_pageframe('HTAB');
    map_page(htab + htabsize, pfn, PT_WRITABLE | PT_PRESENT);

    for (h = htabsize + HANDLES_PER_PAGE - 1; h >= htabsize; h--)
    {
      htab[h] = (struct object *) hfreelist;
      hfreelist = h;
    }

    htabsize += HANDLES_PER_PAGE;
  }

  h = hfreelist;
  hfreelist = (handle_t) htab[h];
  htab[h] = o;
  o->handle_count++;

  return h;
}

//
// hfree
//
// Free handle
//

int hfree(handle_t h)
{
  struct object *o;
  int rc;

  if (h < 0 || h >= htabsize) return -EBADF;
  o = htab[h];

  if (o == (struct object *) NOHANDLE) return -EBADF;
  if (o < (struct object *) OSBASE) return -EBADF;

  htab[h] = (struct object *) hfreelist;
  hfreelist = h;

  if (--o->handle_count > 0) return 0;
  
  rc = close_object(o);

  if (o->lock_count == 0) destroy_object(o);

  return rc;
}

//
// olock
//
// Lock object
//

struct object *olock(handle_t h, int type)
{
  struct object *o;

  if (h < 0 || h >= htabsize) return NULL;
  o = htab[h];

  if (o == (struct object *) NOHANDLE) return NULL;
  if (o < (struct object *) OSBASE) return NULL;
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

int orel(object_t hobj)
{
  struct object *o = (struct object *) hobj;

  if (--o->lock_count == 0 && o->handle_count == 0) 
    return destroy_object(o);
  else
    return 0;
}

//
// handles_proc
//

#define FRAQ 2310

static int handles_proc(struct proc_file *pf, void *arg)
{
  static char *objtype[] = {"THREAD", "EVENT", "TIMER", "MUTEX", "SEM", "FILE", "SOCKET", "IOMUX"};

  int h;
  int i;
  struct object *o;
  int objcount[8];

  for (i = 0; i < 8; i++) objcount[i] = 0;

  pprintf(pf, "handle addr     s type   count locks\n");
  pprintf(pf, "------ -------- - ------ ----- -----\n");
  for (h = 0; h < htabsize; h++)
  {
    o = htab[h];

    if (o < (struct object *) OSBASE) continue;
    if (o == (struct object *) NOHANDLE) continue;
    
    pprintf(pf, "%6d %8X %d %-6s %5d %5d\n", h, o, o->signaled, objtype[o->type], o->handle_count, o->lock_count);
    if (o->handle_count != 0) objcount[o->type] += FRAQ / o->handle_count;
  }

  pprintf(pf, "\n");
  for (i = 0; i < 8; i++) pprintf(pf, "%s:%d ", objtype[i], objcount[i] / FRAQ);
  pprintf(pf, "\n");

  return 0;
}

//
// init_handles
//

void init_handles()
{
  register_proc_inode("handles", handles_proc, NULL);
}
