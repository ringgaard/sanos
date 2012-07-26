//
// rmap.c
//
// Routines for working with a resource map
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

//
// The first entry in a resource map is interpreted as a header.
// The r_size field tells how many slots the overall data structure
// has; r_off tells how many non-empty elements exist.
//
// The list is kept in ascending order with all non-empty elements
// first.
//

#include <stdarg.h>
#include <sys/types.h>
#include <string.h>
#include <rmap.h>

unsigned int lost_elems = 0L;   // Diagnostic for space lost due to fragmentation

//
// Initialize the named resource map
//
// "size" is the total size, including the element we will use
// as the header.
//

void rmap_init(struct rmap *rmap, unsigned int size) {
  // Record # slots available, which is one less (since we use the first as our own header).
  rmap->offset = 0;
  rmap->size = size - 1;
}

//
// Insert room for a new slot at the given position
//
// Returns 1 if there isn't room to insert the element, 0 on success.
//

static int makespace(struct rmap *rmap, struct rmap *r) {
  struct rmap *rlim = &rmap[rmap->offset];

  // If no room to insert slot, return failure
  if (rmap->size == rmap->offset) return 1;
  rmap->offset += 1;

  // If inserting in middle, slide up entries
  if (r <= rlim) {
    memmove(r + 1, r, sizeof(struct rmap) * ((rlim - r) + 1));
    return 0;
  }

  return 0;
}

//
// An entry has been emptied, so make it disappear
//

static void collapse(struct rmap *rmap, struct rmap *r) {
  struct rmap *rlim = &rmap[rmap->offset];

  rmap->offset -= 1;
  if (r < rlim) memmove(r, r + 1, sizeof(struct rmap) * (rlim - r));
}

//
// Allocate some space from a resource map
//
// Returns 0 on failure.  Thus, you can't store index 0 in a resource map
//

unsigned int rmap_alloc(struct rmap *rmap, unsigned int size) {
  struct rmap *r, *rlim;
  unsigned int idx;

  // Find first slot with a fit, return failure if we run off the 
  // end of the list without finding a fit.
  rlim = &rmap[rmap->offset];
  for (r = &rmap[1]; r <= rlim; r++) {
    if (r->size >= size) break;
  }
  
  if (r > rlim) return 0;

  // Trim the resource element if it still has some left,
  // otherwise delete from the list.
  idx = r->offset;
  if (r->size > size) {
    r->offset += size;
    r->size -= size;
  } else {
    collapse(rmap, r);
  }

  return idx;
}

//
// Allocate some aligned space from a resource map
//
// Returns 0 on failure.
//

unsigned int rmap_alloc_align(struct rmap *rmap, unsigned int size, unsigned int align) {
  struct rmap *r, *rlim;
  unsigned int idx;
  unsigned int gap;

  // Find first slot with a fit, return failure if we run off the 
  // end of the list without finding a fit.
  rlim = &rmap[rmap->offset];
  for (r = &rmap[1]; r <= rlim; r++) {
    if (r->offset % align == 0) {
      gap = 0;
    } else {
      gap = align - (r->offset % align);
    }
    if (r->size >= size + gap) break;
  }
  
  if (r > rlim) return 0;
  idx = r->offset + gap;

  // Reserve region
  if (rmap_reserve(rmap, idx, size)) {
    return 0;
  } else {
    return idx;
  }
}

//
// Free some space back into the resource map
//
// The list is kept ordered, with the first free element flagged
// with a size of 0.
//

void rmap_free(struct rmap *rmap, unsigned int offset, unsigned int size) {
  struct rmap *r, *rlim;

  // Scan forward until we find the place we should be inserted.
  rlim = &rmap[rmap->offset];
  for (r = &rmap[1]; r <= rlim; r++) {
    // If the new free space abuts this entry, tack it on and return.
    if ((r->offset + r->size) == offset) {
      r->size += size;

      // If this entry now abuts the next, coalesce
      if ((r < rlim) && ((r->offset + r->size) == (r[1].offset))) {
        r->size += r[1].size;
        rmap->offset -= 1;
        r++;
        if (r < rlim) memmove(r, r + 1, sizeof(struct rmap) * (rlim - r));
      }

      return;
    }

    // If this space abuts the entry, pad it onto the beginning.
    if ((offset + size) == r->offset) {
      r->size += size;
      r->offset = offset;
      return;
    }

    if (offset < r->offset) break;
  }

  // Need to add a new element. See if it'll fit.
  if (makespace(rmap, r)) {
    // Nope. Tabulate and lose the space.
    lost_elems += size;
    return;
  }

  // Record it
  r->offset = offset;
  r->size = size;
}

//
// Reserve the requested range, or return failure if any of it is not free
//
// Returns 0 on success, 1 on failure.
//

int rmap_reserve(struct rmap *rmap, unsigned int offset, unsigned int size) {
  struct rmap *r, *rlim;
  unsigned int top, rtop;

  rlim = &rmap[rmap->offset];
  for (r = &rmap[1]; r <= rlim; r++) {
    // If we've advanced beyond the requested offset, 
    // we can never match, so end the loop.
    if (r->offset > offset) break;

    // See if this is the range which will hold our request
    top = r->offset + r->size;
    if (!((r->offset <= offset) && (top > offset))) continue;

    // Since this run encompasses the requested range, we
    // can either grab all of it, or none of it.

    // The top of our request extends beyond the block; fail.
    rtop = offset + size;
    if (rtop > top) return 1;

    // If the requested start matches our range's start, we
    // can simply shave it off the front.
    if (offset == r->offset) {
      r->offset += size;
      r->size -= size;
      if (r->size == 0) collapse(rmap, r);
      return 0;
    }

    // Similarly, if the end matches, we can shave the end
    if (rtop == top) {
      r->size -= size;

      // We know r->size > 0, since otherwise we would've
      // matched the previous case otherwise.

      return 0;
    }

    // Otherwise we must split the range
    if (makespace(rmap, r)) {
      unsigned int osize;

      // No room for further fragmentation, so chop off to the tail.
      osize = r->size;
      r->size = top - rtop;
      lost_elems += (osize - r->size) - size;
      r->offset = rtop;
      return 0;
    }

    // The current slot is everything below us
    r->size = offset - r->offset;

    // The new slot is everything above
    r++;
    r->offset = rtop;
    r->size = top - rtop;

    // OK
    return 0;
  }

  // Nope, not in our range of free entries
  return 1;
}

//
// Checks allocation status for a resource region
//
// Returns 0 if free, 1 if allocated, -1 if partially allocated.
//

int rmap_status(struct rmap *rmap, unsigned int offset, unsigned int size) {
  struct rmap *r, *rlim;
  unsigned int top, rtop;

  rlim = &rmap[rmap->offset];
  for (r = &rmap[1]; r <= rlim; r++) {
    // If we've advanced beyond the requested offset, 
    // we can never match, check for partially allocated.
    if (r->offset > offset) {
      if (offset + size > r->offset) {
        return -1;
      } else {
        return 1;
      }
    }

    // See if this is the range which will hold our request
    top = r->offset + r->size;
    if (!((r->offset <= offset) && (top > offset))) continue;

    // Check allocation status
    rtop = offset + size;
    if (rtop < top) {
      return 0;
    } else {
      return -1;
    }
  }

  // Resource region is allocated
  return 1;
}
