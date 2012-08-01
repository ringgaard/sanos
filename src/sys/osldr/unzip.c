//
// unzip.c
//
// Decompression routines
//
// Copyright (C) 2002 Michael Ringgaard. All rights reserved.
// Portions Copyright (C) 1995-2002 Jean-loup Gailly and Mark Adler
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
// Inflate deflated (PKZIP's method 8 compressed) data.  The compression
// method searches for as much of the current string of bytes (up to a
// length of 258) in the previous 32 K bytes.  If it doesn't find any
// matches (of at least length 3), it codes the next byte.  Otherwise, it
// codes the length of the matched string and its distance backwards from
// the current position.  There is a single Huffman code that codes both
// single bytes (called "literals") and match lengths.  A second Huffman
// code codes the distance information, which follows a length code.  Each
// length or distance code actually represents a base value and a number
// of "extra" (sometimes zero) bits to get to add to the base value.  At
// the end of each deflated block is a special end-of-block (EOB) literal/
// length code.  The decoding process is basically: get a literal/length
// code; if EOB then done; if a literal, emit the decoded byte; if a
// length then get the distance and emit the referred-to bytes from the
// sliding window of previously emitted data.
//
// There are (currently) three kinds of inflate blocks: stored, fixed, and
// dynamic.  The compressor deals with some chunk of data at a time, and
// decides which method to use on a chunk-by-chunk basis.  A chunk might
// typically be 32 K or 64 K.  If the chunk is incompressible, then the
// "stored" method is used.  In this case, the bytes are simply stored as
// is, eight bits per byte, with none of the above coding.  The bytes are
// preceded by a count, since there is no longer an EOB code.
//
// If the data is compressible, then either the fixed or dynamic methods
// are used.  In the dynamic method, the compressed data is preceded by
// an encoding of the literal/length and distance Huffman codes that are
// to be used to decode this block.  The representation is itself Huffman
// coded, and so is preceded by a description of that code.  These code
// descriptions take up a little space, and so for small blocks, there is
// a predefined set of codes, called the fixed codes.  The fixed method is
// used if the block codes up smaller that way (usually for quite small
// chunks), otherwise the dynamic method is used.  In the latter case, the
// codes are customized to the probabilities in the current block, and so
// can code it much better than the pre-determined fixed codes.
// 
// The Huffman codes themselves are decoded using a multi-level table
// lookup, in order to maximize the speed of decoding plus the speed of
// building the decoding tables.  See the comments below that precede the
// lbits and dbits tuning parameters.
//
// Notes beyond the 1.93a appnote.txt:
//
//  1. Distance pointers never point before the beginning of the output
//     stream.
//  2. Distance pointers can point back across blocks, up to 32k away.
//  3. There is an implied maximum of 7 bits for the bit length table and
//     15 bits for the actual data.
//  4. If only one code exists, then it is encoded using one bit.  (Zero
//     would be more efficient, but perhaps a little confusing.)  If two
//     codes exist, they are coded using one bit each (0 and 1).
//  5. There is no way of sending zero distance codes -- a dummy must be
//     sent if there are none.  (History: a pre 2.0 version of PKZIP would
//     store blocks with no distance codes, but this was discovered to be
//     too harsh a criterion.)  Valid only for 1.93a.  2.04c does allow
//     zero distance codes, which is sent as one code of zero bits in
//     length.
//  6. There are up to 286 literal/length codes.  Code 256 represents the
//     end-of-block.  Note however that the static length tree defines
//     288 codes just to fill out the Huffman codes.  Codes 286 and 287
//     cannot be used though, since there is no length base or extra bits
//     defined for them.  Similarly, there are up to 30 distance codes.
//     However, static trees define 32 codes (all 5 bits) to fill out the
//     Huffman codes, but the last two had better not show up in the data.
//  7. Unzip can check dynamic Huffman blocks for complete code sets.
//     The exception is that a single code would not be complete (see #4).
//  8. The five bits following the block type is really the number of
//     literal codes sent minus 257.
//  9. Length codes 8,16,16 are interpreted as 13 length codes of 8 bits
//     (1+6+6).  Therefore, to output three times the length, you output
//     three codes (1+1+1), whereas to output four times the same length,
//     you only need two codes (1+3).  Hmm.
// 10. In the tree reconstruction algorithm, Code = Code + Increment
//     only if BitLength(i) is not zero.  (Pretty obvious.)
// 11. Correction: 4 Bits: # of Bit Length codes - 4     (4 - 19)
// 12. Note: length code 284 can represent 227-258, but length code 285
//     really is 258.  The last length deserves its own, short code
//     since it gets used a lot in very redundant files.  The length
//     258 is special since 258 - 3 (the min match length) is 255.
// 13. The literal/length and distance code bit lengths are read as a
//     single stream of lengths.  It is possible (and advantageous) for
//     a repeat code (16, 17, or 18) to go across the boundary between
//     the two sets of lengths.
//

#include <string.h>

void panic(char *msg);

//
// GZIP flag byte
//

#define ASCII_FLAG   0x01       // Bit 0: File probably ASCII text
#define CONTINUATION 0x02       // Bit 1: Continuation of multi-part gzip file
#define EXTRA_FIELD  0x04       // Bit 2: Extra field present
#define ORIG_NAME    0x08       // Bit 3: Original file name present
#define COMMENT      0x10       // Bit 4: File comment present
#define ENCRYPTED    0x20       // Bit 5: File is encrypted
#define RESERVED     0xC0       // Bit 6,7: Reserved

//
// Globals
//

static unsigned char *inptr;    // Input pointer
static unsigned char *inend;    // End of input buffer

static unsigned char *outptr;   // Output pointer
static unsigned char *outend;   // End of output buffer

static char *heap_start;        // Start of heap area
static char *heap_end;          // End of heap area
static char *heap_ptr;          // Pointer to first free heap location

//
// Input/output buffer management
//

static unsigned __inline char getbyte() {
  if (inptr == inend) panic("unzip: input buffer underrun");
  return *inptr++;
}

static void __inline ungetbyte() {
  inptr--;
}

static void __inline putbyte(unsigned char c) {
  if (outptr == outend) panic("unzip: output buffer overrun");
  *outptr++ = c;
}

static void __inline copybytes(unsigned dist, unsigned len) {
  if (outptr + len > outend) panic("unzip: output buffer overrun");

  if (dist >= len) {
    memcpy(outptr, outptr - dist, len);
    outptr += len;
  } else {
    unsigned char *p = outptr - dist;
    while (len > 0) {
      *outptr++ = *p++;
      len--;
    }
  }
}

//
// Heap management
//

static void *gzip_malloc(int size) {
  void *p;
  
  heap_ptr = (char *) (((unsigned long) heap_ptr + 3) & ~3);    // Align
  p = (void *) heap_ptr;
  heap_ptr += size;
  if (heap_ptr >= heap_end) return NULL;
  return p;
}

static void gzip_free(void *p) {
  // Ignore
}

static void gzip_mark(void **ptr) {
  *ptr = (void *) heap_ptr;
}

static void gzip_release(void **ptr) {
  heap_ptr = (char *) *ptr;
}

//
// Huffman code lookup table entry.
// Valid extra bits are 0..13.  e == 15 is EOB (end of block), e == 16
// means that v is a literal, 16 < e < 32 means that v is a pointer to
// the next table, which codes e - 16 bits, and lastly e == 99 indicates
// an unused code.  If a code with e == 99 is looked up, this implies an
// error in the data.
//

struct huft  {
  unsigned char e;      // Number of extra bits or operation
  unsigned char b;      // Number of bits in this code or subcode
  union {
    unsigned short n;   // Literal, length base, or distance base
    struct huft *t;     // Pointer to next level of table
  } v;
};

static int huft_free(struct huft *);

// Tables for deflate from PKZIP's appnote.txt.

// Order of the bit length code lengths

static const unsigned border[] = {
  16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15
};

// Copy lengths for literal codes 257..285
// note: see note #13 above about the 258 in this list.

static const unsigned short cplens[] = {
  3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31,
  35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258, 0, 0
};

// Extra bits for literal codes 257..285

static const unsigned short cplext[] = {
  0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2,
  3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0, 99, 99  // 99: invalid
};

// Copy offsets for distance codes 0..29

static const unsigned short cpdist[] = {
  1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193,
  257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145,
  8193, 12289, 16385, 24577
};

// Extra bits for distance codes

static const unsigned short cpdext[] = {
  0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6,
  7, 7, 8, 8, 9, 9, 10, 10, 11, 11,
  12, 12, 13, 13
};

//
// Macros for inflate() bit peeking and grabbing.
// The usage is:
// 
//      NEEDBITS(j)
//      x = b & mask_bits[j];
//      DUMPBITS(j)
//
// where NEEDBITS makes sure that b has at least j bits in it, and
// DUMPBITS removes the bits from b.  The macros use the variable k
// for the number of bits in b.  Normally, b and k are register
// variables for speed, and are initialized at the beginning of a
// routine that uses these macros from a global bit buffer and count.
//
// If we assume that EOB will be the longest code, then we will never
// ask for bits with NEEDBITS that are beyond the end of the stream.
// So, NEEDBITS should not read any more bytes than are needed to
// meet the request.  Then no bytes need to be "returned" to the buffer
// at the end of the last block.
//
// However, this assumption is not true for fixed blocks -- the EOB code
// is 7 bits, but the other literal/length codes can be 8 or 9 bits.
// (The EOB code is shorter than other codes because fixed blocks are
// generally short.  So, while a block always has an EOB, many other
// literal/length codes have a significantly lower probability of
// showing up at all.)  However, by making the first table have a
// lookup of seven bits, the EOB code will be found in that first
// lookup, and so will not require that too many bits be pulled from
// the stream.
//

static unsigned long bb;      // Bit buffer
static unsigned bk;           // Bits in bit buffer

static const unsigned short mask_bits[] = {
  0x0000,
  0x0001, 0x0003, 0x0007, 0x000f, 0x001f, 0x003f, 0x007f, 0x00ff,
  0x01ff, 0x03ff, 0x07ff, 0x0fff, 0x1fff, 0x3fff, 0x7fff, 0xffff
};

#define NEXTBYTE()  getbyte()
#define NEEDBITS(n) {while (k < (n)) {b |= ((unsigned long) NEXTBYTE()) << k; k += 8;}}
#define DUMPBITS(n) {b >>= (n); k-= (n);}

//
// Huffman code decoding is performed using a multi-level table lookup.
// The fastest way to decode is to simply build a lookup table whose
// size is determined by the longest code.  However, the time it takes
// to build this table can also be a factor if the data being decoded
// is not very long.  The most common codes are necessarily the
// shortest codes, so those codes dominate the decoding time, and hence
// the speed.  The idea is you can have a shorter table that decodes the
// shorter, more probable codes, and then point to subsidiary tables for
// the longer codes.  The time it costs to decode the longer codes is
// then traded against the time it takes to make longer tables.
//
// This results of this trade are in the variables lbits and dbits
// below.  lbits is the number of bits the first level table for literal/
// length codes can decode in one step, and dbits is the same thing for
// the distance codes.  Subsequent tables are also less than or equal to
// those sizes.  These values may be adjusted either when all of the
// codes are shorter than that, in which case the longest code length in
// bits is used, or when the shortest code is *longer* than the requested
// table size, in which case the length of the shortest code in bits is
// used.
//
// There are two different values for the two tables, since they code a
// different number of possibilities each.  The literal/length table
// codes 286 possible values, or in a flat code, a little over eight
// bits.  The distance table codes 30 possible values, or a little less
// than five bits, flat.  The optimum values for speed end up being
// about one bit more than those, so lbits is 8+1 and dbits is 5+1.
// The optimum values may differ though from machine to machine, and
// possibly even between compilers.  Your mileage may vary.
//

static const int lbits = 9;   // Bits in base literal/length lookup table
static const int dbits = 6;   // Bits in base distance lookup table

// If BMAX needs to be larger than 16, then h and x[] should be unsigned long.

#define BMAX 16               // Maximum bit length of any code (16 for explode)
#define N_MAX 288             // Maximum number of codes in any set

//
// Given a list of code lengths and a maximum table size, make a set of
// tables to decode that set of codes.  Return zero on success, one if
// the given code set is incomplete (the tables are still built in this
// case), two if the input is invalid (all zero length codes or an
// oversubscribed set of lengths), and three if not enough memory.
//
//   b    Code lengths in bits (all assumed <= BMAX)
//   n    Number of codes (assumed <= N_MAX)
//   s    Number of simple-valued codes (0..s-1)
//   d    List of base values for non-simple codes
//   e    List of extra bits for non-simple codes
//   t    Result: starting table
//   m    Maximum lookup bits, returns actual
//

static int huft_build(unsigned *b, unsigned n, unsigned s, const unsigned short *d, const unsigned short *e, struct huft **t, int *m) {
  unsigned a;                   // Counter for codes of length k
  unsigned c[BMAX + 1];         // Bit length count table
  unsigned f;                   // i repeats in table every f entries
  int g;                        // Maximum code length
  int h;                        // Table level
  unsigned i;                   // Counter, current code
  unsigned j;                   // Counter
  int k;                        // Number of bits in current code
  int l;                        // Bits per table (returned in m)
  unsigned *p;                  // Pointer into c[], b[], or v[]
  struct huft *q;               // Points to current table
  struct huft r;                // Table entry for structure assignment
  struct huft *u[BMAX];         // Table stack
  unsigned v[N_MAX];            // Values in order of bit length
  int w;                        // Bits before this table == (l * h)
  unsigned x[BMAX + 1];         // Bit offsets, then code stack
  unsigned *xp;                 // Pointer into x
  int y;                        // Number of dummy codes added
  unsigned z;                   // Number of entries in current table

  // Generate counts for each bit length
  memset(c, 0, sizeof(c));
  p = b;  i = n;
  do {
    c[*p]++;                    // Assume all entries <= BMAX
    p++;
  } while (--i);
  if (c[0] == n) {              // Null input -- all zero length codes
    *t = NULL;
    *m = 0;
    return 0;
  }

  // Find minimum and maximum length, bound *m by those
  l = *m;
  for (j = 1; j <= BMAX; j++) if (c[j]) break;

  k = j;                        // Minimum code length
  if ((unsigned) l < j) l = j;

  for (i = BMAX; i; i--) if (c[i]) break;

  g = i;                        // Maximum code length
  if ((unsigned) l > i) l = i;
  *m = l;

  // Adjust last length count to fill out codes, if needed
  for (y = 1 << j; j < i; j++, y <<= 1) {
    if ((y -= c[j]) < 0) return 2;  // Bad input: more codes than bits
  }
  if ((y -= c[i]) < 0) return 2;
  c[i] += y;

  // Generate starting offsets into the value table for each length
  x[1] = j = 0;
  p = c + 1;  
  xp = x + 2;
  while (--i)  {
    // Note that i == g from above
    *xp++ = (j += *p++);
  }

  // Make a table of values in order of bit lengths
  p = b;  
  i = 0;
  do {
    if ((j = *p++) != 0) v[x[j]++] = i;
  } while (++i < n);

  // Generate the Huffman codes and for each, make the table entries
  x[0] = i = 0;                 // First Huffman code is zero
  p = v;                        // Grab values in bit order
  h = -1;                       // No tables yet--level -1
  w = -l;                       // Bits decoded == (l * h)
  u[0] = NULL;                  // Just to keep compilers happy
  q = NULL;                     // Ditto
  z = 0;                        // Ditto

  // Go through the bit lengths (k already is bits in shortest code)
  for (; k <= g; k++) {
    a = c[k];
    while (a--) {
      // Here i is the Huffman code of length k bits for value *p
      // make tables up to required level
      while (k > w + l) {
        h++;
        w += l;                 // Previous table always l bits

        // Compute minimum size table less than or equal to l bits
        z = (z = g - w) > (unsigned) l ? l : z;  // Upper limit on table size
        if ((f = 1 << (j = k - w)) > a + 1) {     // Try a k-w bit table
          // Too few codes for k-w bit table
          f -= a + 1;           // deduct codes from patterns left
          xp = c + k;
          while (++j < z) {     // Try smaller tables up to z bits
            if ((f <<= 1) <= *++xp) break;  // Enough codes to use up j bits
            f -= *xp;           // else deduct codes from patterns
          }
        }
        z = 1 << j;             // Table entries for j-bit table

        // Allocate and link in new table
        if ((q = (struct huft *) gzip_malloc((z + 1) * sizeof(struct huft))) == NULL) {
          if (h) huft_free(u[0]);
          return 3;             // Not enough memory
        }

        *t = q + 1;             // Link to list for huft_free()
        *(t = &(q->v.t)) = NULL;
        u[h] = ++q;             // Table starts after link

        // Connect to last table, if there is one
        if (h) {
          x[h] = i;                        // Save pattern for backing up
          r.b = (unsigned char) l;         // Bits to dump before this table
          r.e = (unsigned char) (16 + j);  // Bits in this table
          r.v.t = q;                       // Pointer to this table
          j = i >> (w - l);
          u[h - 1][j] = r;                 // Connect to last table
        }
      }

      // Set up table entry in r
      r.b = (unsigned char) (k - w);
      if (p >= v + n) {
        r.e = 99;               // Out of values -- invalid code
      } else if (*p < s) {
        r.e = (unsigned char) (*p < 256 ? 16 : 15); // 256 is end-of-block code
        r.v.n = (unsigned short) (*p);              // Simple code is just the value
        p++;
      } else {
        r.e = (unsigned char) e[*p - s];   // Non-simple -- look up in lists
        r.v.n = d[*p++ - s];
      }

      // Fill code-like entries with r
      f = 1 << (k - w);
      for (j = i >> w; j < z; j += f) q[j] = r;

      // Backwards increment the k-bit code i
      for (j = 1 << (k - 1); i & j; j >>= 1) i ^= j;
      i ^= j;

      // Backup over finished tables
      while ((i & ((1 << w) - 1)) != x[h]) {
        h--;                    // Don't need to update q
        w -= l;
      }
    }
  }

  // Return true (1) if we were given an incomplete table
  return y != 0 && g != 1;
}

//
// Free the malloc'ed tables built by huft_build(), which makes a linked
// list of the tables it made, with the links in a dummy first entry of
// each table.
//

static int huft_free(struct huft *t) {
  struct huft *p, *q;

  // Go through linked list, freeing from the malloced (t[-1]) address.
  p = t;
  while (p != NULL) {
    q = (--p)->v.t;
    gzip_free(p);
    p = q;
  } 

  return 0;
}

//
// Inflate (decompress) the codes in a deflated (compressed) block.
// Return an error code or zero if it all goes ok.
//
// tl       literal/length decoder table
// td       distance decoder table
// bl       number of bits decoded by tl[]
// bd       number of bits decoded by td[]
//

static int inflate_codes(struct huft *tl, struct huft *td, int bl, int bd) {
  unsigned e;           // Table entry flag/number of extra bits
  unsigned n, d;        // Length and index for copy
  struct huft *t;       // Pointer to table entry
  unsigned ml, md;      // Masks for bl and bd bits
  unsigned long b;      // Bit buffer
  unsigned k;           // Number of bits in bit buffer

  // Make local copies of globals
  b = bb;                       // Initialize bit buffer
  k = bk;

  // inflate the coded data
  ml = mask_bits[bl];           // Precompute masks for speed
  md = mask_bits[bd];
  for (;;) {                    // Do until end of block
    NEEDBITS((unsigned)bl)
    if ((e = (t = tl + ((unsigned) b & ml))->e) > 16) {
      do {
        if (e == 99) return 1;
        DUMPBITS(t->b)
        e -= 16;
        NEEDBITS(e)
      } while ((e = (t = t->v.t + ((unsigned) b & mask_bits[e]))->e) > 16);
    }
    DUMPBITS(t->b)
    if (e == 16) {             // Then it's a literal
      putbyte((unsigned char) t->v.n);
    } else {
      // It's an EOB or a length
      // Exit if end of block
      if (e == 15) break;

      // Get length of block to copy
      NEEDBITS(e)
      n = t->v.n + ((unsigned) b & mask_bits[e]);
      DUMPBITS(e);

      // Decode distance of block to copy
      NEEDBITS((unsigned) bd)
      if ((e = (t = td + ((unsigned) b & md))->e) > 16) {
        do  {
          if (e == 99) return 1;
          DUMPBITS(t->b)
          e -= 16;
          NEEDBITS(e)
        } while ((e = (t = t->v.t + ((unsigned)b & mask_bits[e]))->e) > 16);
      }
      DUMPBITS(t->b)
      NEEDBITS(e)
      d = t->v.n + ((unsigned) b & mask_bits[e]);
      DUMPBITS(e)

      // Do the copy
      copybytes(d, n);
    }
  }

  // Restore the globals from the locals
  bb = b;                       // Restore global bit buffer
  bk = k;

  // done
  return 0;
}

//
// "Decompress" an inflated type 0 (stored) block.
//

static int inflate_stored() {
  unsigned n;           // Number of bytes in block
  unsigned long b;      // Bit buffer
  unsigned k;           // Number of bits in bit buffer

  // Make local copies of globals
  b = bb;                       // Initialize bit buffer
  k = bk;

  // go to byte boundary
  n = k & 7;
  DUMPBITS(n);

  // Get the length and its complement
  NEEDBITS(16)
  n = ((unsigned) b & 0xffff);
  DUMPBITS(16)
  NEEDBITS(16)
  if (n != (unsigned) ((~b) & 0xffff)) return 1; // Error in compressed data
  DUMPBITS(16)

  // Read and output the compressed data
  while (n--) {
    NEEDBITS(8)
    putbyte((unsigned char) b);
    DUMPBITS(8)
  }

  // Restore the globals from the locals
  bb = b;                       // Restore global bit buffer
  bk = k;

  return 0;
}

//
// Decompress an inflated type 1 (fixed Huffman codes) block.  We should
// either replace this with a custom decoder, or at least precompute the
// Huffman tables.
//

static int inflate_fixed() {
  int i;                // Temporary variable
  struct huft *tl;      // Literal/length code table
  struct huft *td;      // Distance code table
  int bl;               // Lookup bits for tl
  int bd;               // Lookup bits for td
  unsigned l[288];      // Length list for huft_build

  // set up literal table
  for (i = 0; i < 144; i++) l[i] = 8;
  for (; i < 256; i++) l[i] = 9;
  for (; i < 280; i++) l[i] = 7;
  for (; i < 288; i++) l[i] = 8;  // Make a complete, but wrong code set
    
  bl = 7;
  if ((i = huft_build(l, 288, 257, cplens, cplext, &tl, &bl)) != 0) return i;

  // Set up distance table
  for (i = 0; i < 30; i++) l[i] = 5; // Make an incomplete code set
  bd = 5;
  if ((i = huft_build(l, 30, 0, cpdist, cpdext, &td, &bd)) > 1) {
    huft_free(tl);
    return i;
  }

  // Decompress until an end-of-block code
  if (inflate_codes(tl, td, bl, bd)) return 1;

  // Free the decoding tables, return
  huft_free(tl);
  huft_free(td);

  return 0;
}

//
// Decompress an inflated type 2 (dynamic Huffman codes) block.
//

static int inflate_dynamic() {
  int i;                // Temporary variables
  unsigned j;
  unsigned l;           // Last length
  unsigned m;           // Mask for bit lengths table
  unsigned n;           // Number of lengths to get
  struct huft *tl;      // Literal/length code table
  struct huft *td;      // Distance code table
  int bl;               // Lookup bits for tl
  int bd;               // Lookup bits for td
  unsigned nb;          // Number of bit length codes
  unsigned nl;          // Number of literal/length codes
  unsigned nd;          // Number of distance codes
  unsigned ll[286 + 30];// Literal/length and distance code lengths
  unsigned long b;      // Bit buffer
  unsigned k;           // Number of bits in bit buffer

  // Make local bit buffer
  b = bb;
  k = bk;

  // Read in table lengths
  NEEDBITS(5)
  nl = 257 + ((unsigned) b & 0x1f);      // Number of literal/length codes
  DUMPBITS(5)
  NEEDBITS(5)
  nd = 1 + ((unsigned) b & 0x1f);        // Number of distance codes
  DUMPBITS(5)
  NEEDBITS(4)
  nb = 4 + ((unsigned) b & 0xf);         // Number of bit length codes
  DUMPBITS(4)
  if (nl > 286 || nd > 30) return 1;     // Bad lengths

  // Read in bit-length-code lengths
  for (j = 0; j < nb; j++) {
    NEEDBITS(3)
    ll[border[j]] = (unsigned) b & 7;
    DUMPBITS(3)
  }
  for (; j < 19; j++) ll[border[j]] = 0;

  // Build decoding table for trees -- single level, 7 bit lookup
  bl = 7;
  if ((i = huft_build(ll, 19, 19, NULL, NULL, &tl, &bl)) != 0) {
    if (i == 1) huft_free(tl);
    return i;                   // Incomplete code set
  }

  // Read in literal and distance code lengths
  n = nl + nd;
  m = mask_bits[bl];
  i = l = 0;
  while ((unsigned) i < n) {
    NEEDBITS((unsigned) bl)
    j = (td = tl + ((unsigned) b & m))->b;
    DUMPBITS(j)
    j = td->v.n;
    if (j < 16) {               // Length of code in bits (0..15)
      ll[i++] = l = j;          // Save last length in l
    } else if (j == 16) {       // Repeat last length 3 to 6 times
      NEEDBITS(2)
      j = 3 + ((unsigned) b & 3);
      DUMPBITS(2)
      if ((unsigned) i + j > n) return 1;
      while (j--) ll[i++] = l;
    } else if (j == 17) {       // 3 to 10 zero length codes
      NEEDBITS(3)
      j = 3 + ((unsigned) b & 7);
      DUMPBITS(3)
      if ((unsigned) i + j > n) return 1;
      while (j--) ll[i++] = 0;
      l = 0;
    } else {                        // j == 18: 11 to 138 zero length codes
      NEEDBITS(7)
      j = 11 + ((unsigned) b & 0x7f);
      DUMPBITS(7)
      if ((unsigned) i + j > n) return 1;
      while (j--) ll[i++] = 0;
      l = 0;
    }
  }

  // Free decoding table for trees
  huft_free(tl);

  // Restore the global bit buffer
  bb = b;
  bk = k;

  // Build the decoding tables for literal/length and distance codes
  bl = lbits;
  if ((i = huft_build(ll, nl, 257, cplens, cplext, &tl, &bl)) != 0) {
    if (i == 1) {
      // Incomplete literal tree
      huft_free(tl);
    }
    return i;                   // Incomplete code set
  }
  bd = dbits;
  if ((i = huft_build(ll + nl, nd, 0, cpdist, cpdext, &td, &bd)) != 0) {
    if (i == 1)  {
      // Incomplete distance tree
      huft_free(td);
    }
    huft_free(tl);
    return i;                   // Incomplete code set
  }

  // Decompress until an end-of-block code
  if (inflate_codes(tl, td, bl, bd)) return 1;

  // Free the decoding tables, return
  huft_free(tl);
  huft_free(td);

  return 0;
}

//
// Decompress an inflated block.
//
// e      Last block flag
//

static int inflate_block(int *e) {
  unsigned t;           // Block type
  unsigned long b;      // Bit buffer
  unsigned k;           // Number of bits in bit buffer

  // Make local bit buffer
  b = bb;
  k = bk;

  // Read in last block bit
  NEEDBITS(1)
  *e = (int) b & 1;
  DUMPBITS(1)

  // Read in block type
  NEEDBITS(2)
  t = (unsigned) b & 3;
  DUMPBITS(2)

  // Restore the global bit buffer
  bb = b;
  bk = k;

  // Inflate that block type
  if (t == 2) return inflate_dynamic();
  if (t == 0) return inflate_stored();
  if (t == 1) return inflate_fixed();

  // Bad block type
  return 2;
}

//
// Decompress an inflated entry
//

static int inflate() {
  int e;                // Last block flag
  int r;                // Result code
  void *ptr;

  // Initialize bit buffer
  bk = 0;
  bb = 0;

  // Decompress until the last block
  do  {
    gzip_mark(&ptr);
    if ((r = inflate_block(&e)) != 0) {
      gzip_release(&ptr);           
      return r;
    }
    gzip_release(&ptr);
  } while (!e);

  // Undo too much lookahead. The next read will be byte aligned so we
  // can discard unused bits in the last meaningful byte.
  while (bk >= 8) {
    bk -= 8;
    ungetbyte();
  }

  // Return success
  return 0;
}

static unsigned int crc_tab_initialized = 0;
static unsigned long crc_32_tab[256];

//
// Compute the CRC-32 table
//

static void initcrctab() {
  unsigned long c;      // crc shift register
  unsigned long e;      // polynomial exclusive-or pattern
  int i;                // counter for all possible eight bit values
  int k;                // byte being shifted into crc apparatus

  // Terms of polynomial defining this crc (except x^32):
  static const int p[] = {0, 1, 2, 4, 5, 7, 8, 10, 11, 12, 16, 22, 23, 26};

  // Make exclusive-or pattern from polynomial
  e = 0;
  for (i = 0; i < sizeof(p) / sizeof(int); i++) e |= 1L << (31 - p[i]);

  crc_32_tab[0] = 0;

  for (i = 1; i < 256; i++) {
    c = 0;
    for (k = i | 256; k != 1; k >>= 1) {
      c = c & 1 ? (c >> 1) ^ e : c >> 1;
      if (k & 1) c ^= e;
    }
    crc_32_tab[i] = c;
  }
}

//
// Compute CRC for data block
//

static unsigned long calccrc(char *data, int len) {
  unsigned long crc = 0xffffffffL;
  unsigned ch;
  int n;

  for (n = 0; n < len; n++) {
    ch = *data++;
    crc = crc_32_tab[(crc ^ ch) & 0xff] ^ (crc >> 8);
  }

  return crc ^ 0xffffffffL;
}
 
//
// Decompress the image in the source buffer into the desination buffer.
// A heap area must be supplied for memory management of the Huffman tables.
//

int unzip(void *src, unsigned long srclen, void *dst, unsigned long dstlen, char *heap, int heapsize) {
  unsigned char flags;
  unsigned char magic[2];           // Magic header
  char method;
  unsigned long orig_crc = 0;       // Original crc
  unsigned long orig_len = 0;       // Original uncompressed length
  int res;

  // Set up input buffer
  inptr  = src;
  inend = (char *) src + srclen;

  // Set up output buffer
  outptr = dst;
  outend = (char *) dst + dstlen;

  // Set up the heap;
  heap_start = heap_ptr = heap;
  heap_end = heap + heapsize;

  // Initialize CRC table
  if (!crc_tab_initialized) {
    initcrctab();
    crc_tab_initialized = 1;
  }

  // Check magic
  magic[0] = (unsigned char) getbyte();
  magic[1] = (unsigned char) getbyte();
  if (magic[0] != 037 || ((magic[1] != 0213) && (magic[1] != 0236)))  panic("unzip: bad gzip magic numbers");

  // We only support method #8, DEFLATED
  method = (unsigned char) getbyte();
  if (method != 8) panic("unzip: unsupported compression method");

  // Check for unsuported flags
  flags  = (unsigned char) getbyte();
  if ((flags & ENCRYPTED) != 0) panic("unzip: input is encrypted");
  if ((flags & CONTINUATION) != 0) panic("unzip: multi-part input not supported");
  if ((flags & RESERVED) != 0) panic("unzip: input has invalid flags");

  // Ignore timestamp
  getbyte();
  getbyte();
  getbyte();
  getbyte();

  getbyte();  // Ignore extra flags
  getbyte();  // Ignore OS type

  // Discard extra field
  if ((flags & EXTRA_FIELD) != 0) {
    unsigned len = (unsigned) getbyte();
    len |= ((unsigned) getbyte()) << 8;
    while (len--) getbyte();
  }

  // Discard original file name 
  if ((flags & ORIG_NAME) != 0) {
    while (getbyte() != 0);
  } 

  // Discard file comment if any
  if ((flags & COMMENT) != 0) {
    while (getbyte() != 0);
  }

  // Decompress
  res = inflate();
  if (res != 0) panic("unzip: error in inflate");
          
  // Get the original crc and length
  orig_crc = (unsigned long) getbyte();
  orig_crc |= (unsigned long) getbyte() << 8;
  orig_crc |= (unsigned long) getbyte() << 16;
  orig_crc |= (unsigned long) getbyte() << 24;
  
  orig_len = (unsigned long) getbyte();
  orig_len |= (unsigned long) getbyte() << 8;
  orig_len |= (unsigned long) getbyte() << 16;
  orig_len |= (unsigned long) getbyte() << 24;
  
  // Validate decompression
  if (orig_len != outptr - (unsigned char *) dst) panic("unzip: length error");
  if (orig_crc != calccrc(dst, outptr - (unsigned char *) dst)) panic("unzip: crc error");

  return orig_len;
}
