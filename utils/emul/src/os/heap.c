//
// heap.c
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Heap memory management routines
// Ported from Doug Lea's malloc
//

#include <os.h>

#define assert(x)

#define ALIGNMENT (2 * sizeof(size_t))
#define ALIGNMASK (ALIGNMENT - 1)

//
// Trimming options
//

#define HAVE_MMAP 1
#define MMAP_AS_MORECORE_SIZE (8 * 1024 * 1024)
#define HAVE_MREMAP 0

#define DEFAULT_MXFAST         64
#define DEFAULT_TRIM_THRESHOLD (1024 * 1024)
#define DEFAULT_TOP_PAD        (0)
#define DEFAULT_MMAP_THRESHOLD (1024 * 1024)
#define DEFAULT_MMAP_MAX       (65536)

//
// Chunk representations
//
// This struct declaration is misleading (but accurate and necessary).
// It declares a "view" into memory allowing access to necessary
// fields at known offsets from a given base. See explanation below.
//

struct chunk 
{
  size_t prev_size;          // Size of previous chunk (if free).
  size_t size;               // Size in bytes, including overhead.

  struct chunk *fd;          // Double links -- used only if free.
  struct chunk *bk;
};

typedef struct chunk* mchunkptr;

//
//  Chunk details
//
//  (The following includes lightly edited explanations by Colin Plumb.)
//  
//  Chunks of memory are maintained using a `boundary tag' method as
//  described in e.g., Knuth or Standish.  (See the paper by Paul
//  Wilson ftp://ftp.cs.utexas.edu/pub/garbage/allocsrv.ps for a
//  survey of such techniques.)  Sizes of free chunks are stored both
//  in the front of each chunk and at the end.  This makes
//  consolidating fragmented chunks into bigger chunks very fast.  The
//  size fields also hold bits representing whether chunks are free or
//  in use.
//  
//  An allocated chunk looks like this:
//  
//  
//     chunk-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//             |             Size of previous chunk, if allocated            | |
//             +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//             |             Size of chunk, in bytes                         |P|
//       mem-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//             |             User data starts here...                          .
//             .                                                               .
//             .             (malloc_usable_space() bytes)                     .
//             .                                                               |
// nextchunk-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//             |             Size of chunk                                     |
//             +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  
//  
//  Where "chunk" is the front of the chunk for the purpose of most of
//  the malloc code, but "mem" is the pointer that is returned to the
//  user.  "Nextchunk" is the beginning of the next contiguous chunk.
//  
//  Chunks always begin on even word boundries, so the mem portion
//  (which is returned to the user) is also on an even word boundary, and
//  thus at least double-word aligned.
//  
//  Free chunks are stored in circular doubly-linked lists, and look like this:
//  
//     chunk-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//             |             Size of previous chunk                            |
//             +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//     'head:' |             Size of chunk, in bytes                         |P|
//       mem-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//             |             Forward pointer to next chunk in list             |
//             +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//             |             Back pointer to previous chunk in list            |
//             +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//             |             Unused space (may be 0 bytes long)                .
//             .                                                               .
//             .                                                               |
// nextchunk-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//     'foot:' |             Size of chunk, in bytes                           |
//             +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  
//  The P (PREV_INUSE) bit, stored in the unused low-order bit of the
//  chunk size (which is always a multiple of two words), is an in-use
//  bit for the *previous* chunk.  If that bit is *clear*, then the
//  word before the current chunk size contains the previous chunk
//  size, and can be used to find the front of the previous chunk.
//  The very first chunk allocated always has this bit set,
//  preventing access to non-existent (or non-owned) memory. If
//  prev_inuse is set for any given chunk, then you CANNOT determine
//  the size of the previous chunk, and might even get a memory
//  addressing fault when trying to do so.
//  
//  Note that the `foot' of the current chunk is actually represented
//  as the prev_size of the NEXT chunk. This makes it easier to
//  deal with alignments etc but can be very confusing when trying
//  to extend or adapt this code.
//  
//  The two exceptions to all this are
//  
//   1. The special chunk `top' doesn't bother using the
//      trailing size field since there is no next contiguous chunk
//      that would have to index off it. After initialization, `top'
//      is forced to always exist.  If it would become less than
//      MINSIZE bytes long, it is replenished.
//  
//   2. Chunks allocated via mmap, which have the second-lowest-order
//      bit (IS_MMAPPED) set in their size fields.  Because they are
//      allocated one-by-one, each must contain its own trailing size field.
//  

//
// Size and alignment checks and conversions
//

// Conversion from malloc headers to user pointers, and back

#define chunk2mem(p)   ((void *) ((char *) (p) + 2 * sizeof(size_t)))
#define mem2chunk(mem) ((mchunkptr) ((char *) (mem) - 2 * sizeof(size_t)))

// The smallest possible chunk

#define MIN_CHUNK_SIZE (sizeof(struct chunk))

// The smallest size we can malloc is an aligned minimal chunk

#define MINSIZE  (size_t) (((MIN_CHUNK_SIZE + ALIGNMASK) & ~ALIGNMASK))

// Check if m has acceptable alignment

#define aligned(m) (((size_t) ((m)) & (ALIGNMASK)) == 0)

// Pad request bytes into a usable size -- internal version

#define request2size(req)                                         \
   (((req) + sizeof(size_t) + ALIGNMASK < MINSIZE)  ?             \
   MINSIZE :                                                      \
   ((req) + sizeof(size_t) + ALIGNMASK) & ~ALIGNMASK)

//
// Physical chunk operations
//

// Size field is or'ed with PREV_INUSE when previous adjacent chunk in use

#define PREV_INUSE 0x1

// Extract inuse bit of previous chunk

#define prev_inuse(p) ((p)->size & PREV_INUSE)

// Size field is or'ed with IS_MMAPPED if the chunk was obtained with mmap()

#define IS_MMAPPED 0x2

// check for mmap()'ed chunk

#define chunk_is_mmapped(p) ((p)->size & IS_MMAPPED)

//
// Bits to mask off when extracting size 
// 
// Note: IS_MMAPPED is intentionally not masked off from size field in
// macros for which mmapped chunks should never be seen. This should
// cause helpful core dumps to occur if it is tried by accident by
// people extending or adapting this malloc.
// 

#define SIZE_BITS (PREV_INUSE | IS_MMAPPED)

// Get size, ignoring use bits

#define chunksize(p) ((p)->size & ~(SIZE_BITS))

// Ptr to next physical chunk

#define next_chunk(p) ((mchunkptr) (((char *) (p)) + ((p)->size & ~PREV_INUSE)))

// Ptr to previous physical chunk

#define prev_chunk(p) ((mchunkptr) (((char *) (p)) - ((p)->prev_size)))

// Treat space at ptr + offset as a chunk

#define chunk_at_offset(p, s)  ((mchunkptr) (((char *) (p)) + (s)))

// Extract p's inuse bit

#define inuse(p) ((((mchunkptr) (((char *) (p))+((p)->size & ~PREV_INUSE)))->size) & PREV_INUSE)

// set/clear chunk as being inuse without otherwise disturbing

#define set_inuse(p) ((mchunkptr) (((char *) (p)) + ((p)->size & ~PREV_INUSE)))->size |= PREV_INUSE
#define clear_inuse(p) ((mchunkptr) (((char *) (p)) + ((p)->size & ~PREV_INUSE)))->size &= ~(PREV_INUSE)

// Check/set/clear inuse bits in known places

#define inuse_bit_at_offset(p, s)       (((mchunkptr) (((char *) (p)) + (s)))->size & PREV_INUSE)
#define set_inuse_bit_at_offset(p, s)   (((mchunkptr) (((char *) (p)) + (s)))->size |= PREV_INUSE)
#define clear_inuse_bit_at_offset(p, s) (((mchunkptr) (((char *) (p)) + (s)))->size &= ~(PREV_INUSE))

// Set size at head, without disturbing its use bit

#define set_head_size(p, s) ((p)->size = (((p)->size & PREV_INUSE) | (s)))

// Set size/use field 

#define set_head(p, s) ((p)->size = (s))

// Set size at footer (only when chunk is not in use)

#define set_foot(p, s) (((mchunkptr) ((char *) (p) + (s)))->prev_size = (s))

//
// Internal data structures
// 
// All internal state is held in an instance of malloc_state defined
// below. 
// 
// Beware of lots of tricks that minimize the total bookkeeping space
// requirements. The result is a little over 1K bytes.
//

// 
// Bins
// 
// An array of bin headers for free chunks. Each bin is doubly
// linked.  The bins are approximately proportionally (log) spaced.
// There are a lot of these bins (128). This may look excessive, but
// works very well in practice.  Most bins hold sizes that are
// unusual as malloc request sizes, but are more usual for fragments
// and consolidated sets of chunks, which is what these bins hold, so
// they can be found quickly.  All procedures maintain the invariant
// that no consolidated chunk physically borders another one, so each
// chunk in a list is known to be preceeded and followed by either
// inuse chunks or the ends of memory.
// 
// Chunks in bins are kept in size order, with ties going to the
// approximately least recently used chunk. Ordering isn't needed
// for the small bins, which all contain the same-sized chunks, but
// facilitates best-fit allocation for larger chunks. These lists
// are just sequential. Keeping them in order almost never requires
// enough traversal to warrant using fancier ordered data
// structures.  
// 
// Chunks of the same size are linked with the most
// recently freed at the front, and allocations are taken from the
// back.  This results in LRU (FIFO) allocation order, which tends
// to give each chunk an equal opportunity to be consolidated with
// adjacent freed chunks, resulting in larger free chunks and less
// fragmentation.
// 
// To simplify use in double-linked lists, each bin header acts
// as a chunk. This avoids special-casing for headers. But to conserve 
// space and improve locality, we allocate only the fd/bk pointers 
// of bins, and then use repositioning tricks to treat these as the 
// fields of a chunk.  
// 

typedef struct chunk *mbinptr;

// Addressing -- note that bin_at(0) does not exist

#define bin_at(m, i) ((mbinptr) ((char *) &((m)->bins[(i) << 1]) - (sizeof(size_t) << 1)))

// Analog of ++bin

#define next_bin(b)  ((mbinptr)((char *) (b) + (sizeof(mchunkptr) << 1)))

// Reminders about list directionality within bins

#define first(b) ((b)->fd)
#define last(b) ((b)->bk)

// Take a chunk off a bin list

#define unlink(p, b, f)  \
{                        \
  f = p->fd;             \
  b = p->bk;             \
  f->bk = b;             \
  b->fd = f;             \
}

//
// Indexing
// 
// Bins for sizes < 512 bytes contain chunks of all the same size, spaced
// 8 bytes apart. Larger bins are approximately logarithmically spaced:
// 
//  64 bins of size       8
//  32 bins of size      64
//  16 bins of size     512
//  8 bins of size    4096
//  4 bins of size   32768
//  2 bins of size  262144
//  1 bin  of size what's left
// 
// There is actually a little bit of slop in the numbers in bin_index
// for the sake of speed. This makes no difference elsewhere.
// 
// The bins top out around 1MB because we expect to service large
// requests via mmap.
// 

#define NBINS             128
#define NSMALLBINS         64
#define SMALLBIN_WIDTH      8
#define MIN_LARGE_SIZE    512

#define in_smallbin_range(sz) ((sz) < MIN_LARGE_SIZE)

#define smallbin_index(sz) ((sz) >> 3)

#define largebin_index(sz)                 \
((((sz) >>  6) <= 32)?  56 + ((sz) >>  6): \
 (((sz) >>  9) <= 20)?  91 + ((sz) >>  9): \
 (((sz) >> 12) <= 10)? 110 + ((sz) >> 12): \
 (((sz) >> 15) <=  4)? 119 + ((sz) >> 15): \
 (((sz) >> 18) <=  2)? 124 + ((sz) >> 18): \
                                        126)

#define bin_index(sz) ((in_smallbin_range(sz)) ? smallbin_index(sz) : largebin_index(sz))

//
// Unsorted chunks
// 
// All remainders from chunk splits, as well as all returned chunks,
// are first placed in the "unsorted" bin. They are then placed
// in regular bins after malloc gives them ONE chance to be used before
// binning. So, basically, the unsorted_chunks list acts as a queue,
// with chunks being placed on it in free (and heap_consolidate),
// and taken off (to be either used or placed in bins) in malloc.
// 

// The otherwise unindexable 1-bin is used to hold unsorted chunks.

#define unsorted_chunks(m) (bin_at(m, 1))

// 
// Top
// 
// The top-most available chunk (i.e., the one bordering the end of
// available memory) is treated specially. It is never included in
// any bin, is used only if no other chunk is available, and is
// released back to the system if it is very large (see
// M_TRIM_THRESHOLD).  Because top initially
// points to its own bin with initial zero size, thus forcing
// extension on the first malloc request, we avoid having any special
// code in malloc to check whether it even exists yet. But we still
// need to do so when getting memory from system, so we make
// initial_top treat the bin as a legal but unusable chunk during the
// interval between initialization and the first call to
// sysalloc. (This is somewhat delicate, since it relies on
// the 2 preceding words to be zero during this interval as well.)
//

// Conveniently, the unsorted bin can be used as dummy top on first call

#define initial_top(m) (unsorted_chunks(m))

// 
// Binmap
// 
// To help compensate for the large number of bins, a one-level index
// structure is used for bin-by-bin searching.  `binmap' is a
// bitvector recording whether bins are definitely empty so they can
// be skipped over during during traversals.  The bits are NOT always
// cleared as soon as bins are empty, but instead only
// when they are noticed to be empty during traversal in malloc.
// 

#define BINMAPSHIFT      5
#define BITSPERMAP       (1U << BINMAPSHIFT)
#define BINMAPSIZE       (NBINS / BITSPERMAP)

#define idx2block(i)     ((i) >> BINMAPSHIFT)
#define idx2bit(i)       ((1U << ((i) & ((1U << BINMAPSHIFT)-1))))

#define mark_bin(m,i)    ((m)->binmap[idx2block(i)] |=  idx2bit(i))
#define unmark_bin(m,i)  ((m)->binmap[idx2block(i)] &= ~(idx2bit(i)))
#define get_binmap(m,i)  ((m)->binmap[idx2block(i)] &   idx2bit(i))

// 
// Fastbins
// 
// An array of lists holding recently freed small chunks.  Fastbins
// are not doubly linked.  It is faster to single-link them, and
// since chunks are never removed from the middles of these lists,
// double linking is not necessary. Also, unlike regular bins, they
// are not even processed in FIFO order (they use faster LIFO) since
// ordering doesn't much matter in the transient contexts in which
// fastbins are normally used.
// 
// Chunks in fastbins keep their inuse bit set, so they cannot
// be consolidated with other free chunks. heap_consolidate
// releases all chunks in fastbins and consolidates them with
// other free chunks. 
// 

typedef struct chunk *mfastbinptr;

// Offset 2 to use otherwise unindexable first 2 bins

#define fastbin_index(sz) ((((unsigned int) (sz)) >> 3) - 2)

// The maximum fastbin request size we support

#define MAX_FAST_SIZE     80

#define NFASTBINS  (fastbin_index(request2size(MAX_FAST_SIZE)) + 1)

// 
// FASTBIN_CONSOLIDATION_THRESHOLD is the size of a chunk in free()
// that triggers automatic consolidation of possibly-surrounding
// fastbin chunks. This is a heuristic, so the exact value should not
// matter too much. It is defined at half the default trim threshold as a
// compromise heuristic to only attempt consolidation if it is likely
// to lead to trimming. However, it is not dynamically tunable, since
// consolidation reduces fragmentation surrounding large chunks even 
// if trimming is not used.
// 

#define FASTBIN_CONSOLIDATION_THRESHOLD  (65536UL)

//
// Since the lowest 2 bits in max_fast don't matter in size comparisons, 
// they are used as flags.
// 
// FASTCHUNKS_BIT held in max_fast indicates that there are probably
// some fastbin chunks. It is set true on entering a chunk into any
// fastbin, and cleared only in heap_consolidate.
// 
// The truth value is inverted so that have_fastchunks will be true
// upon startup (since statics are zero-filled), simplifying
// initialization checks.
// 

#define FASTCHUNKS_BIT (1U)

#define have_fastchunks(m)     (((m)->max_fast &  FASTCHUNKS_BIT) == 0)
#define clear_fastchunks(m)    ((m)->max_fast |=  FASTCHUNKS_BIT)
#define set_fastchunks(m)      ((m)->max_fast &= ~FASTCHUNKS_BIT)

// 
// NONCONTIGUOUS_BIT indicates that MORECORE does not return contiguous
// regions.  Otherwise, contiguity is exploited in merging together,
// when possible, results from consecutive MORECORE calls.
// 
// The initial value comes from MORECORE_CONTIGUOUS, but is
// changed dynamically if mmap is ever used as an sbrk substitute.
// 

#define NONCONTIGUOUS_BIT     (2U)

#define contiguous(m)          (((m)->max_fast &  NONCONTIGUOUS_BIT) == 0)
#define noncontiguous(m)       (((m)->max_fast &  NONCONTIGUOUS_BIT) != 0)
#define set_noncontiguous(m)   ((m)->max_fast |=  NONCONTIGUOUS_BIT)
#define set_contiguous(m)      ((m)->max_fast &= ~NONCONTIGUOUS_BIT)

//  
// Set value of max_fast. 
// Use impossibly small value if 0.
// Precondition: there are no existing fastbin chunks.
// Setting the value clears fastchunk bit but preserves noncontiguous bit.
// 

#define set_max_fast(m, s) \
  (m)->max_fast = (((s) == 0)? SMALLBIN_WIDTH: request2size(s)) | \
  FASTCHUNKS_BIT | \
  ((m)->max_fast &  NONCONTIGUOUS_BIT)


//
// Internal heap state representation and initialization
//

struct heap
{
  size_t       max_fast;            // The maximum chunk size to be eligible for fastbin (low 2 bits used as flags)
  mfastbinptr  fastbins[NFASTBINS]; // Fastbins
  mchunkptr    top;                 // Base of the topmost chunk -- not otherwise kept in a bin
  mchunkptr    last_remainder;      // The remainder from the most recent split of a small request
  mchunkptr    bins[NBINS * 2];     // Normal bins packed as described above
  unsigned int binmap[BINMAPSIZE];  // Bitmap of bins

  // Tunable parameters
  unsigned long    trim_threshold;
  size_t           top_pad;
  size_t           mmap_threshold;

  // Memory map support
  int              n_mmaps;
  int              n_mmaps_max;
  int              max_n_mmaps;
};

// 
// There is exactly one instance of this struct in this malloc.
// If you are adapting this malloc in a way that does NOT use a static
// malloc_state, you MUST explicitly zero-fill it before using. This
// malloc relies on the property that malloc_state is initialized to
// all zeroes (as is true of C statics).
// 

static struct heap mainheap;  // never directly referenced

// 
// All uses of av_ are via get_malloc_state().
// At most one "call" to get_malloc_state is made per invocation of
// the public versions of malloc and free, but other routines
// that in turn invoke malloc and/or free may call more then once. 
// Also, it is called in check* routines if DEBUG is set.
// 

#define get_malloc_state() (&(mainheap))

//
// Initialize a malloc_state struct.
// 
// This is called only from within heap_consolidate, which needs
// be called in the same contexts anyway.  It is never called directly
// outside of heap_consolidate because some optimizing compilers try
// to inline it at all call points, which turns out not to be an
// optimization at all. (Inlining it in heap_consolidate is fine though.)
// 

static heap_init(struct heap *av)
{
  int     i;
  mbinptr bin;
  
  // Establish circular links for normal bins
  for (i = 1; i < NBINS; i++)
  { 
    bin = bin_at(av,i);
    bin->fd = bin->bk = bin;
  }

  av->top_pad        = DEFAULT_TOP_PAD;
  av->n_mmaps_max    = DEFAULT_MMAP_MAX;
  av->mmap_threshold = DEFAULT_MMAP_THRESHOLD;
  av->trim_threshold = DEFAULT_TRIM_THRESHOLD;

  //set_noncontiguous(av);

  set_max_fast(av, DEFAULT_MXFAST);

  av->top            = initial_top(av);
}

//
// heap_consolidate
// 
// heap_consolidate is a specialized version of free() that tears
// down chunks held in fastbins.  Free itself cannot be used for this
// purpose since, among other things, it might place chunks back onto
// fastbins.  So, instead, we need to use a minor variant of the same
// code.
// 
// Also, because this routine needs to be called the first time through
// malloc anyway, it turns out to be the perfect place to trigger
// initialization code.

static void heap_consolidate(struct heap *av)
{
  mfastbinptr *   fb;                 // Current fastbin being consolidated
  mfastbinptr *   maxfb;              // Last fastbin (for loop control)
  mchunkptr       p;                  // Current chunk being consolidated
  mchunkptr       nextp;              // Next chunk to consolidate
  mchunkptr       unsorted_bin;       // Bin header
  mchunkptr       first_unsorted;     // chunk to link to

  // These have same use as in free()
  mchunkptr       nextchunk;
  size_t          size;
  size_t          nextsize;
  size_t          prevsize;
  int             nextinuse;
  mchunkptr       bck;
  mchunkptr       fwd;

  // If max_fast is 0, we know that av hasn't yet been initialized, in which case do so below
  if (av->max_fast != 0) 
  {
    clear_fastchunks(av);
    unsorted_bin = unsorted_chunks(av);

    // Remove each chunk from fast bin and consolidate it, placing it
    // then in unsorted bin. Among other reasons for doing this,
    // placing in unsorted bin avoids needing to calculate actual bins
    // until malloc is sure that chunks aren't immediately going to be
    // reused anyway.
    
    maxfb = &(av->fastbins[fastbin_index(av->max_fast)]);
    fb = &(av->fastbins[0]);
    do 
    {
      if ((p = *fb) != 0) 
      {
        *fb = 0;
        
        do 
	{
          nextp = p->fd;
          
          // Slightly streamlined version of consolidation code in free()
          size = p->size & ~PREV_INUSE;
          nextchunk = chunk_at_offset(p, size);
          nextsize = chunksize(nextchunk);
          
          if (!prev_inuse(p))
	  {
            prevsize = p->prev_size;
            size += prevsize;
            p = chunk_at_offset(p, -((long) prevsize));
            unlink(p, bck, fwd);
          }
          
          if (nextchunk != av->top) 
	  {
            nextinuse = inuse_bit_at_offset(nextchunk, nextsize);
            set_head(nextchunk, nextsize);
            
            if (!nextinuse) 
	    {
              size += nextsize;
              unlink(nextchunk, bck, fwd);
            }
            
            first_unsorted = unsorted_bin->fd;
            unsorted_bin->fd = p;
            first_unsorted->bk = p;
            
            set_head(p, size | PREV_INUSE);
            p->bk = unsorted_bin;
            p->fd = first_unsorted;
            set_foot(p, size);
          }
          else 
	  {
            size += nextsize;
            set_head(p, size | PREV_INUSE);
            av->top = p;
          }
        } while ( (p = nextp) != 0);
      }
    } while (fb++ != maxfb);
  }
  else 
    heap_init(av);
}

//
// sysalloc handles malloc cases requiring more memory from the system.
// On entry, it is assumed that av->top does not have enough
// space to service request for nb bytes, thus requiring that av->top
// be extended or replaced.
// 

#define REGION_SIZE (32 * 1024 * 1024)
#define GROUP_SIZE  (128 * 1024)

static char *region = NULL;
static char *wilderness = NULL;
static char *heapend = NULL;

static void *sysalloc(size_t nb, struct heap *av)
{
  mchunkptr       top;             // incoming value of av->top
  size_t          size;            // its size
  size_t          newsize;
  mchunkptr       p;               // the allocated/returned chunk
  mchunkptr       remainder;       // remainder from allocation
  unsigned long   remainder_size;  // its size
  size_t expand;

  // Allocate big chunks directly from system
  if (nb >= av->mmap_threshold && av->n_mmaps < av->n_mmaps_max) 
  {
    char *mem;

    size = (nb + sizeof(size_t) + ALIGNMASK + PAGESIZE - 1) & ~(PAGESIZE - 1);

    mem = (char *) mmap(NULL, size, MEM_COMMIT, PAGE_READWRITE);
    if (mem != NULL)
    {
      p = (mchunkptr) mem;
      set_head(p, size | IS_MMAPPED);
      syslog(LOG_DEBUG | LOG_HEAP, "Big allocation %dK\n", size / 1024);

      return chunk2mem(p);
    }
  }

  // Allocate memory from the system
  if (region == NULL)
  {
    region = (char *) mmap(NULL, REGION_SIZE, MEM_RESERVE, PAGE_READWRITE);
    if (!region) panic("unable to reserve heap region");
    wilderness = region;
    heapend = region + REGION_SIZE;
  }

  top = av->top;
  size = chunksize(top);

  // Commit one or more groups from region
  expand = (nb + MINSIZE - size + GROUP_SIZE - 1)  & ~(GROUP_SIZE - 1);
  syslog(LOG_HEAP | LOG_DEBUG, "Expand heap: request = %d, remaining = %d, expansion = %d\n", nb, size, expand);
  if (wilderness + expand > heapend) panic("out of memory");

  if (mmap(wilderness, expand, MEM_COMMIT, PAGE_READWRITE) == 0) panic("unable to expand heap");
  wilderness += expand;

  if (size == 0)
  {
    syslog(LOG_HEAP | LOG_DEBUG, "Reserve heap region\n");
    av->top = (mchunkptr) region;
    newsize = expand;
  }
  else
    newsize = size + expand;

  set_head(av->top, newsize | PREV_INUSE);

  syslog(LOG_HEAP | LOG_DEBUG, "Expand heap to %d KB (%d)\n", (wilderness - region) / 1024, expand);

  // Do the allocation
  p = av->top;
  size = chunksize(p);

  remainder_size = size - nb;
  remainder = chunk_at_offset(p, nb);
  av->top = remainder;
  set_head(p, nb | PREV_INUSE);
  set_head(remainder, remainder_size | PREV_INUSE);

  return chunk2mem(p);
}

//
// systrim is an inverse of sorts to sysalloc.  It gives memory back
// to the system (via negative arguments to sbrk) if there is unused
// memory at the `high' end of the malloc pool. It is called
// automatically by free() when top space exceeds the trim
// threshold. It is also called by the public malloc_trim routine.  It
// returns 1 if it actually released any memory, else 0.
//

static int systrim(size_t pad, struct heap *av)
{
  syslog(LOG_HEAP | LOG_DEBUG, "Heap Trim called: top remaining = %d\n", chunksize(av->top));
  return 0;
}

//
// heap_alloc
//

void *heap_alloc(size_t bytes)
{
  struct heap *av = get_malloc_state();

  size_t          nb;               // normalized request size
  unsigned int    idx;              // associated bin index
  mbinptr         bin;              // associated bin
  mfastbinptr*    fb;               // associated fastbin

  mchunkptr       victim;           // inspected/selected chunk
  size_t          size;             // its size
  int             victim_index;     // its bin index

  mchunkptr       remainder;        // remainder from a split
  unsigned long   remainder_size;   // its size

  unsigned int    block;            // bit map traverser
  unsigned int    bit;              // bit map traverser
  unsigned int    map;              // current word of binmap

  mchunkptr       fwd;              // misc temp for linking
  mchunkptr       bck;              // misc temp for linking

  // Convert request size to internal form by adding sizeof(size_t) bytes
  // overhead plus possibly more to obtain necessary alignment and/or
  // to obtain a size of at least MINSIZE, the smallest allocatable
  // size. 

  nb = request2size(bytes);

  // If the size qualifies as a fastbin, first check corresponding bin.
  // This code is safe to execute even if av is not yet initialized, so we
  // can try it without checking, which saves some time on this fast path.

  if (nb <= av->max_fast)
  { 
    fb = &(av->fastbins[(fastbin_index(nb))]);
    if ( (victim = *fb) != 0) 
    {
      *fb = victim->fd;
      return chunk2mem(victim);
    }
  }

  // If a small request, check regular bin.  Since these "smallbins"
  // hold one size each, no searching within bins is necessary.
  // (For a large request, we need to wait until unsorted chunks are
  // processed to find best fit. But for small ones, fits are exact
  // anyway, so we can check now, which is faster.)

  if (in_smallbin_range(nb)) 
  {
    idx = smallbin_index(nb);
    bin = bin_at(av,idx);

    if ((victim = last(bin)) != bin) 
    {
      if (victim == 0) // initialization check
        heap_consolidate(av);
      else 
      {
        bck = victim->bk;
        set_inuse_bit_at_offset(victim, nb);
        bin->bk = bck;
        bck->fd = bin;
        
        return chunk2mem(victim);
      }
    }
  }

  // If this is a large request, consolidate fastbins before continuing.
  // While it might look excessive to kill all fastbins before
  // even seeing if there is space available, this avoids
  // fragmentation problems normally associated with fastbins.
  // Also, in practice, programs tend to have runs of either small or
  // large requests, but less often mixtures, so consolidation is not 
  // invoked all that often in most programs. And the programs that
  // it is called frequently in otherwise tend to fragment.

  else 
  {
    idx = largebin_index(nb);
    if (have_fastchunks(av)) heap_consolidate(av);
  }

  // Process recently freed or remaindered chunks, taking one only if
  // it is exact fit, or, if this a small request, the chunk is remainder from
  // the most recent non-exact fit.  Place other traversed chunks in
  // bins.  Note that this step is the only place in any routine where
  // chunks are placed in bins.
  // 
  // The outer loop here is needed because we might not realize until
  // near the end of malloc that we should have consolidated, so must
  // do so and retry. This happens at most once, and only when we would
  // otherwise need to expand memory to service a "small" request.
    
  for(;;) 
  {    
    
    while ((victim = unsorted_chunks(av)->bk) != unsorted_chunks(av)) 
    {
      bck = victim->bk;
      size = chunksize(victim);

      // If a small request, try to use last remainder if it is the
      // only chunk in unsorted bin.  This helps promote locality for
      // runs of consecutive small requests. This is the only
      // exception to best-fit, and applies only when there is
      // no exact fit for a small chunk.

      if (in_smallbin_range(nb) && bck == unsorted_chunks(av) && victim == av->last_remainder && size > nb + MINSIZE)
      {
        // split and reattach remainder
        remainder_size = size - nb;
        remainder = chunk_at_offset(victim, nb);
        unsorted_chunks(av)->bk = unsorted_chunks(av)->fd = remainder;
        av->last_remainder = remainder; 
        remainder->bk = remainder->fd = unsorted_chunks(av);
        
        set_head(victim, nb | PREV_INUSE);
        set_head(remainder, remainder_size | PREV_INUSE);
        set_foot(remainder, remainder_size);
        
        return chunk2mem(victim);
      }

      // Remove from unsorted list
      unsorted_chunks(av)->bk = bck;
      bck->fd = unsorted_chunks(av);
      
      // Take now instead of binning if exact fit
      if (size == nb) 
      {
        set_inuse_bit_at_offset(victim, size);
        return chunk2mem(victim);
      }
      
      // Place chunk in bin
      if (in_smallbin_range(size)) 
      {
        victim_index = smallbin_index(size);
        bck = bin_at(av, victim_index);
        fwd = bck->fd;
      }
      else 
      {
        victim_index = largebin_index(size);
        bck = bin_at(av, victim_index);
        fwd = bck->fd;

        // Maintain large bins in sorted order
        if (fwd != bck) 
	{
	  // Or with inuse bit to speed comparisons
          size |= PREV_INUSE;

          // If smaller than smallest, bypass loop below
          if (size <= bck->bk->size) 
	  {
            fwd = bck;
            bck = bck->bk;
          }
          else 
	  {
            while (size < fwd->size) fwd = fwd->fd;
            bck = fwd->bk;
          }
        }
      }
      
      mark_bin(av, victim_index);
      victim->bk = bck;
      victim->fd = fwd;
      fwd->bk = victim;
      bck->fd = victim;
    }
   
    // If a large request, scan through the chunks of current bin in
    // sorted order to find smallest that fits.  This is the only step
    // where an unbounded number of chunks might be scanned without doing
    // anything useful with them. However the lists tend to be short.
      
    if (!in_smallbin_range(nb)) 
    {
      bin = bin_at(av, idx);

      // Skip scan if empty or largest chunk is too small
      if ((victim = last(bin)) != bin && first(bin)->size >= nb) 
      {
        while ((size = chunksize(victim)) < nb) victim = victim->bk;

        remainder_size = size - nb;
        unlink(victim, bck, fwd);
        
        // Exhaust
        if (remainder_size < MINSIZE)  
	{
          set_inuse_bit_at_offset(victim, size);
          return chunk2mem(victim);
        }
        else 
	{
          // Split
          remainder = chunk_at_offset(victim, nb);
          unsorted_chunks(av)->bk = unsorted_chunks(av)->fd = remainder;
          remainder->bk = remainder->fd = unsorted_chunks(av);
          set_head(victim, nb | PREV_INUSE);
          set_head(remainder, remainder_size | PREV_INUSE);
          set_foot(remainder, remainder_size);
          return chunk2mem(victim);
        } 
      }
    }    

    // Search for a chunk by scanning bins, starting with next largest
    // bin. This search is strictly by best-fit; i.e., the smallest
    // (with ties going to approximately the least recently used) chunk
    // that fits is selected.
    
    // The bitmap avoids needing to check that most blocks are nonempty.
    // The particular case of skipping all bins during warm-up phases
    // when no chunks have been returned yet is faster than it might look.
    
    idx++;
    bin = bin_at(av,idx);
    block = idx2block(idx);
    map = av->binmap[block];
    bit = idx2bit(idx);
    
    for (;;) 
    {
      // Skip rest of block if there are no more set bits in this block.
      if (bit > map || bit == 0) 
      {
        do { if (++block >= BINMAPSIZE) goto use_top; } while ((map = av->binmap[block]) == 0);
        bin = bin_at(av, (block << BINMAPSHIFT));
        bit = 1;
      }
      
      // Advance to bin with set bit. There must be one.
      while ((bit & map) == 0) 
      {
        bin = next_bin(bin);
        bit <<= 1;
        assert(bit != 0);
      }
      
      // Inspect the bin. It is likely to be non-empty
      victim = last(bin);
      
      //  If a false alarm (empty bin), clear the bit.
      if (victim == bin) 
      {
        av->binmap[block] = map &= ~bit; // Write through
        bin = next_bin(bin);
        bit <<= 1;
      }
      else 
      {
        size = chunksize(victim);

        //  We know the first chunk in this bin is big enough to use.
        assert(size >= nb);

        remainder_size = size - nb;
        
        // Unlink
        bck = victim->bk;
        bin->bk = bck;
        bck->fd = bin;
        
        // Exhaust
        if (remainder_size < MINSIZE) 
	{
          set_inuse_bit_at_offset(victim, size);
          return chunk2mem(victim);
        }
        else 
	{
        
          // Split
          remainder = chunk_at_offset(victim, nb);
          
          unsorted_chunks(av)->bk = unsorted_chunks(av)->fd = remainder;
          remainder->bk = remainder->fd = unsorted_chunks(av);
          
	  // Advertise as last remainder
          if (in_smallbin_range(nb)) av->last_remainder = remainder; 
          
          set_head(victim, nb | PREV_INUSE);
          set_head(remainder, remainder_size | PREV_INUSE);
          set_foot(remainder, remainder_size);
          return chunk2mem(victim);
        }
      }
    }

  use_top:    
    // If large enough, split off the chunk bordering the end of memory
    // (held in av->top). Note that this is in accord with the best-fit
    // search rule.  In effect, av->top is treated as larger (and thus
    // less well fitting) than any other available chunk since it can
    // be extended to be as large as necessary (up to system
    // limitations).
    // 
    // We require that av->top always exists (i.e., has size >=
    // MINSIZE) after initialization, so if it would otherwise be
    // exhuasted by current request, it is replenished. (The main
    // reason for ensuring it exists is that we may need MINSIZE space
    // to put in fenceposts in sysmalloc.)

    victim = av->top;
    size = chunksize(victim);
   
    if (size >= nb + MINSIZE)
    {
      remainder_size = size - nb;
      remainder = chunk_at_offset(victim, nb);
      av->top = remainder;
      set_head(victim, nb | PREV_INUSE);
      set_head(remainder, remainder_size | PREV_INUSE);

      return chunk2mem(victim);
    }

    // If there is space available in fastbins, consolidate and retry,
    // to possibly avoid expanding memory. This can occur only if nb is
    // in smallbin range so we didn't consolidate upon entry.

    else if (have_fastchunks(av)) 
    {
      assert(in_smallbin_range(nb));
      heap_consolidate(av);
      idx = smallbin_index(nb); // restore original bin index
    }
    else 
       // Otherwise, relay to handle system-dependent cases 
      return sysalloc(nb, av);
  }
}

//
// heap_free
//

void heap_free(void *mem)
{
  struct heap *av = get_malloc_state();

  mchunkptr       p;           // chunk corresponding to mem
  size_t          size;        // its size
  mfastbinptr*    fb;          // associated fastbin
  mchunkptr       nextchunk;   // next contiguous chunk
  size_t          nextsize;    // its size
  int             nextinuse;   // true if nextchunk is used
  size_t          prevsize;    // size of previous contiguous chunk
  mchunkptr       bck;         // misc temp for linking
  mchunkptr       fwd;         // misc temp for linking

  // free(0) has no effect
  if (mem != 0) 
  {
    p = mem2chunk(mem);
    size = chunksize(p);

    // If eligible, place chunk on a fastbin so it can be found
    // and used quickly in malloc.

    if (size <= av->max_fast) 
    {

      set_fastchunks(av);
      fb = &(av->fastbins[fastbin_index(size)]);
      p->fd = *fb;
      *fb = p;
    }
    else if (!chunk_is_mmapped(p)) 
    {
      // Consolidate other non-mmapped chunks as they arrive.
      nextchunk = chunk_at_offset(p, size);
      nextsize = chunksize(nextchunk);

      // Consolidate backward
      if (!prev_inuse(p)) 
      {
        prevsize = p->prev_size;
        size += prevsize;
        p = chunk_at_offset(p, -((long) prevsize));
        unlink(p, bck, fwd);
      }

      if (nextchunk != av->top) 
      {
        // Get and clear inuse bit
        nextinuse = inuse_bit_at_offset(nextchunk, nextsize);
        set_head(nextchunk, nextsize);

        // Consolidate forward 
        if (!nextinuse) 
	{
          unlink(nextchunk, bck, fwd);
          size += nextsize;
        }

        // Place the chunk in unsorted chunk list. Chunks are
        // not placed into regular bins until after they have
        // been given one chance to be used in malloc.

        bck = unsorted_chunks(av);
        fwd = bck->fd;
        p->bk = bck;
        p->fd = fwd;
        bck->fd = p;
        fwd->bk = p;

        set_head(p, size | PREV_INUSE);
        set_foot(p, size);
      }
      else 
      {
	// If the chunk borders the current high end of memory,
	// consolidate into top

        size += nextsize;
        set_head(p, size | PREV_INUSE);
        av->top = p;
      }

      // If freeing a large space, consolidate possibly-surrounding
      // chunks. Then, if the total unused topmost memory exceeds trim
      // threshold, ask malloc_trim to reduce top.
      // 
      // Unless max_fast is 0, we don't know if there are fastbins
      // bordering top, so we cannot tell for sure whether threshold
      // has been reached unless fastbins are consolidated.  But we
      // don't want to consolidate on each free.  As a compromise,
      // consolidation is performed if FASTBIN_CONSOLIDATION_THRESHOLD
      // is reached.

      if (size >= FASTBIN_CONSOLIDATION_THRESHOLD) 
      { 
        if (have_fastchunks(av)) heap_consolidate(av);

        if (chunksize(av->top) >= av->trim_threshold)
	{
          systrim(av->top_pad, av);
	}
      }

    }

    // If the chunk was allocated via mmap, release via munmap()
    // Note that if HAVE_MMAP is false but chunk_is_mmapped is
    // true, then user must have overwritten memory. There's nothing
    // we can do to catch this error unless DEBUG is set, in which case
    // check_inuse_chunk (above) will have triggered error.

    else 
    {
      int ret;
      size_t offset = p->prev_size;
      av->n_mmaps--;
      syslog(LOG_DEBUG | LOG_HEAP, "Free big chunk %dK (%d)\n", size / 1024, offset);
      ret = munmap(p - offset, size + offset, MEM_RELEASE);
      // munmap returns non-zero on failure
      assert(ret == 0);
    }
  }
}

//
// heap_realloc
//

void *heap_realloc(void *oldmem, size_t bytes)
{
  struct heap *av = get_malloc_state();

  size_t           nb;              // padded request size

  mchunkptr        oldp;            // chunk corresponding to oldmem
  size_t           oldsize;         // its size

  mchunkptr        newp;            // chunk to return
  size_t           newsize;         // its size
  void *           newmem;          // corresponding user mem

  mchunkptr        next;            // next contiguous chunk after oldp

  mchunkptr        remainder;       // extra space at end of newp
  unsigned long    remainder_size;  // its size

  mchunkptr        bck;             // misc temp for linking
  mchunkptr        fwd;             // misc temp for linking

  if (bytes == 0)
  {
    heap_free(oldmem);
    return 0;
  }

  // Realloc of null is supposed to be same as malloc
  if (oldmem == 0) return heap_alloc(bytes);

  nb = request2size(bytes);

  oldp = mem2chunk(oldmem);
  oldsize = chunksize(oldp);

  if (!chunk_is_mmapped(oldp)) 
  {

    if (oldsize >= nb) 
    {
      // Already big enough; split below
      newp = oldp;
      newsize = oldsize;
    }
    else 
    {
      next = chunk_at_offset(oldp, oldsize);

      // Try to expand forward into top
      if (next == av->top && (newsize = oldsize + chunksize(next)) >= nb + MINSIZE) 
      {
        set_head_size(oldp, nb);
        av->top = chunk_at_offset(oldp, nb);
        set_head(av->top, (newsize - nb) | PREV_INUSE);
        return chunk2mem(oldp);
      }

      // Try to expand forward into next chunk;  split off remainder below
      else if (next != av->top && !inuse(next) && (newsize = oldsize + chunksize(next)) >= nb) 
      {
        newp = oldp;
        unlink(next, bck, fwd);
      }
      else 
      {
        // Allocate, copy, free
        newmem = heap_alloc(nb - ALIGNMASK);
        if (newmem == 0) return 0; // Propagate failure
      
        newp = mem2chunk(newmem);
        newsize = chunksize(newp);
        
        // Avoid copy if newp is next chunk after oldp.
        if (newp == next) 
	{
          newsize += oldsize;
          newp = oldp;
        }
        else 
	{
          memcpy(newmem, oldmem, oldsize - sizeof(size_t));
          heap_free(oldmem);
          return chunk2mem(newp);
        }
      }
    }

    // If possible, free extra space in old or extended chunk
    assert(newsize >= nb);
    remainder_size = newsize - nb;

    if (remainder_size < MINSIZE) 
    {
      // not enough extra to split off
      set_head_size(newp, newsize);
      set_inuse_bit_at_offset(newp, newsize);
    }
    else 
    { 
      // split remainder
      remainder = chunk_at_offset(newp, nb);
      set_head_size(newp, nb);
      set_head(remainder, remainder_size | PREV_INUSE);
      
      // Mark remainder as inuse so free() won't complain
      set_inuse_bit_at_offset(remainder, remainder_size);
      heap_free(chunk2mem(remainder)); 
    }

    return chunk2mem(newp);
  }
  else 
  {
    // Handle mmap cases
#if HAVE_MREMAP
    size_t offset = oldp->prev_size;
    size_t pagemask = av->pagesize - 1;
    char *cp;
    unsigned long sum;
    
    // Note the extra sizeof(size_t) overhead
    newsize = (nb + offset + sizeof(size_t) + pagemask) & ~pagemask;

    // don't need to remap if still within same page
    if (oldsize == newsize - offset) return oldmem;

    cp = (char *) mremap((char *) oldp - offset, oldsize + offset, newsize, 1);
    if (cp != (char*)MORECORE_FAILURE) 
    {
      newp = (mchunkptr)(cp + offset);
      set_head(newp, (newsize - offset) | IS_MMAPPED);
      
      assert(aligned(chunk2mem(newp)));
      assert((newp->prev_size == offset));
      
      // update statistics
      sum = av->mmapped_mem += newsize - oldsize;
      if (sum > av->max_mmapped_mem) av->max_mmapped_mem = sum;
      sum += av->sbrked_mem;
      if (sum > av->max_total_mem) av->max_total_mem = sum;
      return chunk2mem(newp);
    }
#endif

    // Note the extra sizeof(size_t) overhead.
    if (oldsize >= nb + sizeof(size_t)) 
      newmem = oldmem; // do nothing
    else 
    {
      // Must alloc, copy, free.
      newmem = heap_alloc(nb - ALIGNMASK);
      if (newmem != 0) 
      {
        memcpy(newmem, oldmem, oldsize - 2 * sizeof(size_t));
        heap_free(oldmem);
      }
    }
    return newmem;
  }
}

//
// heap_calloc
//

void *heap_calloc(size_t n_elements, size_t elem_size)
{
  mchunkptr p;

  void *mem = heap_alloc(n_elements * elem_size);

  if (mem != 0) 
  {
    p = mem2chunk(mem);

    // Don't need to clear mmapped space
    if (!chunk_is_mmapped(p))  memset(mem, 0, chunksize(p) - sizeof(size_t));
  }

  return mem;
}
