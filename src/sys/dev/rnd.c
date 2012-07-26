//
// rnd.c
//
// A strong random number generator
//
// Copyright (C) 2002 Michael Ringgaard. All rights reserved.
// Copyright (C) 1994, 1995, 1996, 1997, 1998, 1999 Theodore Ts'o.
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

#ifdef RANDOMDEV

//
// Configuration information
//

#define DEFAULT_POOL_SIZE   512
#define SECONDARY_POOL_SIZE 128
#define BATCH_ENTROPY_SIZE  256

#define RANDOM_READ_TIMEOUT 30000

//
// A pool of size POOLWORDS is stirred with a primitive polynomial
// of degree POOLWORDS over GF(2).  The taps for various sizes are
// defined below.  They are chosen to be evenly spaced (minimum RMS
// distance from evenly spaced; the numbers in the comments are a
// scaled squared error sum) except for the last tap, which is 1 to
// get the twisting happening as fast as possible.
//

struct poolinfo  {
  int poolwords;
  int tap1, tap2, tap3, tap4, tap5;
};

static struct poolinfo poolinfo_table[] =  {
  // x^2048 + x^1638 + x^1231 + x^819 + x^411 + x + 1  -- 115
  {2048, 1638, 1231, 819,  411,  1},

  // x^1024 + x^817 + x^615 + x^412 + x^204 + x + 1 -- 290
  {1024, 817,  615,  412,  204,  1},
  
  // x^512 + x^411 + x^308 + x^208 + x^104 + x + 1 -- 225
  {512,  411,  308,  208,  104,  1},

  // x^256 + x^205 + x^155 + x^101 + x^52 + x + 1 -- 125
  {256,  205,  155,  101,  52, 1},
  
  // x^128 + x^103 + x^76 + x^51 +x^25 + x + 1 -- 105
  {128,  103,  76, 51, 25, 1},

  // x^64 + x^52 + x^39 + x^26 + x^14 + x + 1 -- 15
  {64, 52, 39, 26, 14, 1},

  // x^32 + x^26 + x^20 + x^14 + x^7 + x + 1 -- 15
  {32, 26, 20, 14, 7,  1},

  {0,  0,  0,  0,  0,  0},
};    

//
// Utility functions
//

#ifndef MIN
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#endif

__inline static unsigned long rotate_left(int i, unsigned long word) {
  return (word << i) | (word >> (32 - i));
}

__inline static unsigned long int_ln_12bits(unsigned long word) {
  // Smear msbit right to make an n-bit mask
  word |= word >> 8;
  word |= word >> 4;
  word |= word >> 2;
  word |= word >> 1;

  // Remove one bit to make this a logarithm
  word >>= 1;

  // Count the bits set in the word
  word -= (word >> 1) & 0x555;
  word = (word & 0x333) + ((word >> 2) & 0x333);
  word += (word >> 4);
  word += (word >> 8);
  return word & 15;
}

//
// OS independent entropy store.   Here are the functions which handle
// storing entropy in an entropy pool.
//

struct rand_pool_info  {
  int entropy_count;
  int buf_size;
  unsigned long buf[0];
};

struct entropy_store  {
  unsigned add_ptr;
  int entropy_count;
  int input_rotate;
  int extract_count;
  struct poolinfo poolinfo;
  unsigned long *pool;
};

static struct entropy_store *random_state; // The default global store
static struct entropy_store *sec_random_state; // Secondary store
static struct event random_read_ready;

//
// Initialize the entropy store.  The input argument is the size of
// the random pool.
// 
// Returns an negative error if there is a problem.
//

static int create_entropy_store(int size, struct entropy_store **retval) {
  struct  entropy_store *r;
  struct  poolinfo  *p;
  int poolwords;

  poolwords = (size + 3) / 4; // Convert bytes->words

  // The pool size must be a multiple of 16 32-bit words
  poolwords = ((poolwords + 15) / 16) * 16; 

  for (p = poolinfo_table; p->poolwords; p++) {
    if (poolwords == p->poolwords) break;
  }
  if (p->poolwords == 0) return -EINVAL;

  r = kmalloc(sizeof(struct entropy_store));
  if (!r) return -ENOMEM;

  memset (r, 0, sizeof(struct entropy_store));
  r->poolinfo = *p;

  r->pool = kmalloc(poolwords * 4);
  if (!r->pool) {
    kfree(r);
    return -ENOMEM;
  }
  memset(r->pool, 0, poolwords * 4);
  
  *retval = r;
  return 0;
}

//
// Clear the entropy pool and associated counters.
//

static void clear_entropy_store(struct entropy_store *r) {
  r->add_ptr = 0;
  r->entropy_count = 0;
  r->input_rotate = 0;
  r->extract_count = 0;
  memset(r->pool, 0, r->poolinfo.poolwords * 4);
}

static void free_entropy_store(struct entropy_store *r) {
  if (r->pool) kfree(r->pool);
  kfree(r);
}

//
// This function adds a byte into the entropy "pool".  It does not
// update the entropy estimate. The caller should call
// credit_entropy_store if this is appropriate.
// 
// The pool is stirred with a primitive polynomial of the appropriate
// degree, and then twisted. We twist by three bits at a time because
// it's cheap to do so and helps slightly in the expected case where
// the entropy is concentrated in the low-order bits.
//

static void add_entropy_words(struct entropy_store *r, const unsigned long *in, int num) {
  static unsigned long const twist_table[8] = {
    0x00000000, 0x3b6e20c8, 0x76dc4190, 0x4db26158,
    0xedb88320, 0xd6d6a3e8, 0x9b64c2b0, 0xa00ae278 
  };

  unsigned i;
  int new_rotate;
  unsigned long w;

  while (num--) {
    w = rotate_left(r->input_rotate, *in);
    i = r->add_ptr = (r->add_ptr - 1) & (r->poolinfo.poolwords - 1);

    //
    // Normally, we add 7 bits of rotation to the pool.
    // At the beginning of the pool, add an extra 7 bits
    // rotation, so that successive passes spread the
    // input bits across the pool evenly.
    //

    new_rotate = r->input_rotate + 14;
    if (i) new_rotate = r->input_rotate + 7;
    r->input_rotate = new_rotate & 31;

    // XOR in the various taps
    w ^= r->pool[(i + r->poolinfo.tap1) & (r->poolinfo.poolwords - 1)];
    w ^= r->pool[(i + r->poolinfo.tap2) & (r->poolinfo.poolwords - 1)];
    w ^= r->pool[(i + r->poolinfo.tap3) & (r->poolinfo.poolwords - 1)];
    w ^= r->pool[(i + r->poolinfo.tap4) & (r->poolinfo.poolwords - 1)];
    w ^= r->pool[(i + r->poolinfo.tap5) & (r->poolinfo.poolwords - 1)];
    w ^= r->pool[i];
    r->pool[i] = (w >> 3) ^ twist_table[w & 7];
  }
}

//
// Credit (or debit) the entropy store with n bits of entropy
//

static void credit_entropy_store(struct entropy_store *r, int num) {
  int max_entropy = r->poolinfo.poolwords * 32;

  if (r->entropy_count + num < 0) {
    r->entropy_count = 0;
  } else if (r->entropy_count + num > max_entropy) {
    r->entropy_count = max_entropy;
  } else {
    r->entropy_count = r->entropy_count + num;
  }
}

//
// Entropy batch input management
//
// We batch entropy to be added to avoid increasing interrupt latency
//

static unsigned long *batch_entropy_pool;
static int *batch_entropy_credit;
static int batch_max;
static int batch_head, batch_tail;
static struct task batch_task;

static void batch_entropy_process(void *arg);

// Note: the size must be a power of 2

static int batch_entropy_init(int size) {
  init_task(&batch_task);

  batch_entropy_pool = kmalloc(2 * size * sizeof(unsigned long));
  if (!batch_entropy_pool) return -ENOMEM;
  batch_entropy_credit = kmalloc(size * sizeof(int));
  if (!batch_entropy_credit) {
    kfree(batch_entropy_pool);
    return -ENOMEM;
  }
  batch_head = batch_tail = 0;
  batch_max = size;

  add_idle_task(&batch_task, batch_entropy_process, NULL);

  return 0;
}

void batch_entropy_store(unsigned long a, unsigned long b, int num) {
  int batch_new;

  if (!batch_max) return;

  //kprintf("rnd: (%lu,%lu) %d bits\n", a, b, num); 
  
  batch_entropy_pool[2 * batch_head] = a;
  batch_entropy_pool[(2 * batch_head) + 1] = b;
  batch_entropy_credit[batch_head] = num;

  batch_new = (batch_head + 1) & (batch_max - 1);

  if (batch_new != batch_tail) {
    //queue_task(NULL, &batch_task, batch_entropy_process, NULL);
    batch_head = batch_new;
  } else {
    //kprintf("random: batch entropy buffer full\n");
  }
}

static void batch_entropy_process(void *arg) {
  int num = 0;
  int max_entropy;
  struct entropy_store *r = random_state;
  struct entropy_store *p;

  if (!batch_max) return;
  if (batch_head == batch_tail) return;

  //kprintf("batch entropy task start\n");

  max_entropy = r->poolinfo.poolwords * 32;
  while (batch_head != batch_tail) {
    if (!system_idle()) break;
    add_entropy_words(r, batch_entropy_pool + 2 * batch_tail, 2);
    p = r;
    if (r->entropy_count > max_entropy && (num & 1)) r = sec_random_state;
    credit_entropy_store(r, batch_entropy_credit[batch_tail]);
    batch_tail = (batch_tail + 1) & (batch_max - 1);
    num++;
  }

  //kprintf("batch entropy task end (%d bits)\n", random_state->entropy_count);

  if (r->entropy_count >= 8) set_event(&random_read_ready);
}

//
// Entropy input management
//

struct timer_rand_state  {
  unsigned long last_time;
  long last_delta, last_delta2;
  int dont_count_entropy;
};

static struct timer_rand_state extract_timer_state;
static struct timer_rand_state dpc_timer_state;

//
// This function adds entropy to the entropy "pool" by using timing
// delays.  It uses the timer_rand_state structure to make an estimate
// of how many bits of entropy this call has added to the pool.
//
// The number "num" is also added to the pool - it should somehow describe
// the type of event which just happened.
//

static void add_timer_randomness(struct timer_rand_state *state, unsigned long num) {
  unsigned long time;
  long delta, delta2, delta3;
  int entropy = 0;

  if (cpu.features & CPU_FEATURE_TSC) {
    time = (unsigned long) rdtsc();
  } else {
    time = ticks;
  }

  //
  // Calculate number of bits of randomness we probably added.
  // We take into account the first, second and third-order deltas
  // in order to make our estimate.
  //

  if (!state->dont_count_entropy) {
    delta = time - state->last_time;
    state->last_time = time;

    delta2 = delta - state->last_delta;
    state->last_delta = delta;

    delta3 = delta2 - state->last_delta2;
    state->last_delta2 = delta2;

    if (delta < 0) delta = -delta;
    if (delta2 < 0) delta2 = -delta2;
    if (delta3 < 0) delta3 = -delta3;
    if (delta > delta2) delta = delta2;
    if (delta > delta3) delta = delta3;

    //
    // delta is now minimum absolute delta.
    // Round down by 1 bit on general principles,
    // and limit entropy entimate to 12 bits.
    //

    delta >>= 1;
    delta &= (1 << 12) - 1;

    entropy = int_ln_12bits(delta);
  }

  batch_entropy_store(num, time, entropy);
}

void add_dpc_randomness(void *dpc) {
  add_timer_randomness(&dpc_timer_state, (unsigned long) dpc);
}

//
// Hash function definition
//

#define HASH_BUFFER_SIZE 4
#define HASH_EXTRA_SIZE 0
#define HASH_TRANSFORM MD5Transform
  
//
// MD5 transform algorithm, taken from code written by Colin Plumb,
// and put into the public domain
//

// The four core functions - F1 is optimized somewhat

// #define F1(x, y, z) (x & y | ~x & z)
#define F1(x, y, z) (z ^ (x & (y ^ z)))
#define F2(x, y, z) F1(z, x, y)
#define F3(x, y, z) (x ^ y ^ z)
#define F4(x, y, z) (y ^ (x | ~z))

// This is the central step in the MD5 algorithm.
#define MD5STEP(f, w, x, y, z, data, s) ( w += f(x, y, z) + data,  w = w<<s | w>>(32-s),  w += x )

//
// The core of the MD5 algorithm, this alters an existing MD5 hash to
// reflect the addition of 16 longwords of new data.  MD5Update blocks
// the data and converts bytes into longwords for this routine.
//

static void MD5Transform(unsigned long buf[HASH_BUFFER_SIZE], unsigned long const in[16]) {
  unsigned long a, b, c, d;

  a = buf[0];
  b = buf[1];
  c = buf[2];
  d = buf[3];

  MD5STEP(F1, a, b, c, d, in[ 0]+0xd76aa478,  7);
  MD5STEP(F1, d, a, b, c, in[ 1]+0xe8c7b756, 12);
  MD5STEP(F1, c, d, a, b, in[ 2]+0x242070db, 17);
  MD5STEP(F1, b, c, d, a, in[ 3]+0xc1bdceee, 22);
  MD5STEP(F1, a, b, c, d, in[ 4]+0xf57c0faf,  7);
  MD5STEP(F1, d, a, b, c, in[ 5]+0x4787c62a, 12);
  MD5STEP(F1, c, d, a, b, in[ 6]+0xa8304613, 17);
  MD5STEP(F1, b, c, d, a, in[ 7]+0xfd469501, 22);
  MD5STEP(F1, a, b, c, d, in[ 8]+0x698098d8,  7);
  MD5STEP(F1, d, a, b, c, in[ 9]+0x8b44f7af, 12);
  MD5STEP(F1, c, d, a, b, in[10]+0xffff5bb1, 17);
  MD5STEP(F1, b, c, d, a, in[11]+0x895cd7be, 22);
  MD5STEP(F1, a, b, c, d, in[12]+0x6b901122,  7);
  MD5STEP(F1, d, a, b, c, in[13]+0xfd987193, 12);
  MD5STEP(F1, c, d, a, b, in[14]+0xa679438e, 17);
  MD5STEP(F1, b, c, d, a, in[15]+0x49b40821, 22);

  MD5STEP(F2, a, b, c, d, in[ 1]+0xf61e2562,  5);
  MD5STEP(F2, d, a, b, c, in[ 6]+0xc040b340,  9);
  MD5STEP(F2, c, d, a, b, in[11]+0x265e5a51, 14);
  MD5STEP(F2, b, c, d, a, in[ 0]+0xe9b6c7aa, 20);
  MD5STEP(F2, a, b, c, d, in[ 5]+0xd62f105d,  5);
  MD5STEP(F2, d, a, b, c, in[10]+0x02441453,  9);
  MD5STEP(F2, c, d, a, b, in[15]+0xd8a1e681, 14);
  MD5STEP(F2, b, c, d, a, in[ 4]+0xe7d3fbc8, 20);
  MD5STEP(F2, a, b, c, d, in[ 9]+0x21e1cde6,  5);
  MD5STEP(F2, d, a, b, c, in[14]+0xc33707d6,  9);
  MD5STEP(F2, c, d, a, b, in[ 3]+0xf4d50d87, 14);
  MD5STEP(F2, b, c, d, a, in[ 8]+0x455a14ed, 20);
  MD5STEP(F2, a, b, c, d, in[13]+0xa9e3e905,  5);
  MD5STEP(F2, d, a, b, c, in[ 2]+0xfcefa3f8,  9);
  MD5STEP(F2, c, d, a, b, in[ 7]+0x676f02d9, 14);
  MD5STEP(F2, b, c, d, a, in[12]+0x8d2a4c8a, 20);

  MD5STEP(F3, a, b, c, d, in[ 5]+0xfffa3942,  4);
  MD5STEP(F3, d, a, b, c, in[ 8]+0x8771f681, 11);
  MD5STEP(F3, c, d, a, b, in[11]+0x6d9d6122, 16);
  MD5STEP(F3, b, c, d, a, in[14]+0xfde5380c, 23);
  MD5STEP(F3, a, b, c, d, in[ 1]+0xa4beea44,  4);
  MD5STEP(F3, d, a, b, c, in[ 4]+0x4bdecfa9, 11);
  MD5STEP(F3, c, d, a, b, in[ 7]+0xf6bb4b60, 16);
  MD5STEP(F3, b, c, d, a, in[10]+0xbebfbc70, 23);
  MD5STEP(F3, a, b, c, d, in[13]+0x289b7ec6,  4);
  MD5STEP(F3, d, a, b, c, in[ 0]+0xeaa127fa, 11);
  MD5STEP(F3, c, d, a, b, in[ 3]+0xd4ef3085, 16);
  MD5STEP(F3, b, c, d, a, in[ 6]+0x04881d05, 23);
  MD5STEP(F3, a, b, c, d, in[ 9]+0xd9d4d039,  4);
  MD5STEP(F3, d, a, b, c, in[12]+0xe6db99e5, 11);
  MD5STEP(F3, c, d, a, b, in[15]+0x1fa27cf8, 16);
  MD5STEP(F3, b, c, d, a, in[ 2]+0xc4ac5665, 23);

  MD5STEP(F4, a, b, c, d, in[ 0]+0xf4292244,  6);
  MD5STEP(F4, d, a, b, c, in[ 7]+0x432aff97, 10);
  MD5STEP(F4, c, d, a, b, in[14]+0xab9423a7, 15);
  MD5STEP(F4, b, c, d, a, in[ 5]+0xfc93a039, 21);
  MD5STEP(F4, a, b, c, d, in[12]+0x655b59c3,  6);
  MD5STEP(F4, d, a, b, c, in[ 3]+0x8f0ccc92, 10);
  MD5STEP(F4, c, d, a, b, in[10]+0xffeff47d, 15);
  MD5STEP(F4, b, c, d, a, in[ 1]+0x85845dd1, 21);
  MD5STEP(F4, a, b, c, d, in[ 8]+0x6fa87e4f,  6);
  MD5STEP(F4, d, a, b, c, in[15]+0xfe2ce6e0, 10);
  MD5STEP(F4, c, d, a, b, in[ 6]+0xa3014314, 15);
  MD5STEP(F4, b, c, d, a, in[13]+0x4e0811a1, 21);
  MD5STEP(F4, a, b, c, d, in[ 4]+0xf7537e82,  6);
  MD5STEP(F4, d, a, b, c, in[11]+0xbd3af235, 10);
  MD5STEP(F4, c, d, a, b, in[ 2]+0x2ad7d2bb, 15);
  MD5STEP(F4, b, c, d, a, in[ 9]+0xeb86d391, 21);

  buf[0] += a;
  buf[1] += b;
  buf[2] += c;
  buf[3] += d;
}

#undef F1
#undef F2
#undef F3
#undef F4
#undef MD5STEP

//
// Entropy extraction routines
//

#define EXTRACT_ENTROPY_USER      1
#define EXTRACT_ENTROPY_SECONDARY 2

#define TMP_BUF_SIZE              (HASH_BUFFER_SIZE + HASH_EXTRA_SIZE)
#define SEC_XFER_SIZE             (TMP_BUF_SIZE * 4)

static int extract_entropy(struct entropy_store *r, void *buf, size_t nbytes, int flags);

//
// This utility function is responsible for transfering entropy
// from the primary pool to the secondary extraction pool. We pull 
// randomness under two conditions; one is if there isn't enough entropy 
// in the secondary pool.  The other is after we have extract 1024 bytes,
// at which point we do a "catastrophic reseeding".
//

static void xfer_secondary_pool(struct entropy_store *r, size_t nbytes) {
  unsigned long tmp[TMP_BUF_SIZE];

  if (r->entropy_count < (int) nbytes * 8) {
    extract_entropy(random_state, tmp, sizeof(tmp), 0);
    add_entropy_words(r, tmp, TMP_BUF_SIZE);
    credit_entropy_store(r, TMP_BUF_SIZE * 8);
  }
  
  if (r->extract_count > 1024) {
    extract_entropy(random_state, tmp, sizeof(tmp), 0);
    add_entropy_words(r, tmp, TMP_BUF_SIZE);
    r->extract_count = 0;
  }
}

//
// This function extracts randomness from the "entropy pool", and
// returns it in a buffer.  This function computes how many remaining
// bits of entropy are left in the pool, but it does not restrict the
// number of bytes that are actually obtained.
//
// Note: extract_entropy() assumes that POOLWORDS is a multiple of 16 words.
//

static int extract_entropy(struct entropy_store *r, void *buf, size_t nbytes, int flags) {
  int ret, i;
  unsigned long tmp[TMP_BUF_SIZE];
  unsigned long x;
  char *b = (char *) buf; 

  add_timer_randomness(&extract_timer_state, nbytes);
  
  // Redundant, but just in case...
  if (r->entropy_count > r->poolinfo.poolwords) r->entropy_count = r->poolinfo.poolwords;

  if (flags & EXTRACT_ENTROPY_SECONDARY) xfer_secondary_pool(r, nbytes);

  if (r->entropy_count / 8 >= (int) nbytes) {
    r->entropy_count -= nbytes * 8;
  } else {
    r->entropy_count = 0;
  }
  //if (r->entropy_count < 128) set_event(&random_write_ready);

  r->extract_count += nbytes;
  
  ret = 0;
  while (nbytes) {
    // Hash the pool to get the output
    tmp[0] = 0x67452301;
    tmp[1] = 0xefcdab89;
    tmp[2] = 0x98badcfe;
    tmp[3] = 0x10325476;

    //
    // As we hash the pool, we mix intermediate values of
    // the hash back into the pool.  This eliminates
    // backtracking attacks (where the attacker knows
    // the state of the pool plus the current outputs, and
    // attempts to find previous ouputs), unless the hash
    // function can be inverted.
    //

    for (i = 0, x = 0; i < r->poolinfo.poolwords; i += 16, x += 2) {
      HASH_TRANSFORM(tmp, r->pool + i);
      add_entropy_words(r, &tmp[x % HASH_BUFFER_SIZE], 1);
    }
    
    //
    // In case the hash function has some recognizable
    // output pattern, we fold it in half.
    //

    for (i = 0; i <  HASH_BUFFER_SIZE / 2; i++) {
      tmp[i] ^= tmp[i + (HASH_BUFFER_SIZE + 1) / 2];
     }

#if HASH_BUFFER_SIZE & 1  
    // There's a middle word to deal with
    x = tmp[HASH_BUFFER_SIZE / 2];
    x ^= (x >> 16);   // Fold it in half
    ((unsigned short *) tmp)[HASH_BUFFER_SIZE - 1] = (unsigned short) x;
#endif

    // Copy data to destination buffer
    i = MIN(nbytes, HASH_BUFFER_SIZE * sizeof(unsigned long) / 2);
    memcpy(b, (unsigned char const *) tmp, i);

    nbytes -= i;
    b += i;
    ret += i;
    add_timer_randomness(&extract_timer_state, nbytes);
  }

  // Wipe data just returned from memory
  memset(tmp, 0, sizeof(tmp));
  
  return ret;
}

//
// This function is the exported kernel interface.  It returns some
// number of good random numbers, suitable for seeding TCP sequence
// numbers, etc.
//

void get_random_bytes(void *buf, int nbytes) {
  if (sec_random_state) {
    extract_entropy(sec_random_state, (char *) buf, nbytes, EXTRACT_ENTROPY_SECONDARY);
  } else if (random_state) {
    extract_entropy(random_state, (char *) buf, nbytes, 0);
  } else {
    kprintf("random: get_random_bytes() called before random driver initialization\n");
  }
}

static int random_ioctl(struct dev *dev, int cmd, void *args, size_t size) {
  switch (cmd) {
    case IOCTL_GETDEVSIZE:
      return 0;

    case IOCTL_GETBLKSIZE:
      return 1;
  }
  
  return -ENOSYS;
}

static int random_read(struct dev *dev, void *buffer, size_t count, blkno_t blkno, int flags) {
  int n;
  
  if (count == 0) return 0;
  while (1) {
    n = count;
    if (n > SEC_XFER_SIZE) n = SEC_XFER_SIZE;
    if (n > random_state->entropy_count / 8) n = random_state->entropy_count / 8;
    if (n > 0) break;

    reset_event(&random_read_ready);
    n = wait_for_object(&random_read_ready, RANDOM_READ_TIMEOUT);
    if (n < 0) return n;
  }

  return extract_entropy(sec_random_state, buffer, n, EXTRACT_ENTROPY_USER | EXTRACT_ENTROPY_SECONDARY);
}

static int urandom_read(struct dev *dev, void *buffer, size_t count, blkno_t blkno, int flags) {
  return extract_entropy(sec_random_state, buffer, count, EXTRACT_ENTROPY_USER | EXTRACT_ENTROPY_SECONDARY);
}

static int random_write(struct dev *dev, void *buffer, size_t count, blkno_t blkno, int flags) {
  int words;

  words = count / sizeof(unsigned long);
  add_entropy_words(random_state, (unsigned long *) buffer, words);
  return words * sizeof(unsigned long);
}

struct driver random_driver = {
  "random",
  DEV_TYPE_STREAM,
  random_ioctl,
  random_read,
  random_write
};

struct driver urandom_driver = {
  "urandom",
  DEV_TYPE_STREAM,
  random_ioctl,
  urandom_read,
  random_write
};

static void init_std_data(struct entropy_store *r) {
  unsigned long words[2];

  words[0] = systemclock.tv_sec;
  words[1] = systemclock.tv_usec;
  add_entropy_words(r, words, 2);
}

int __declspec(dllexport) random() {
  int rc;

  init_event(&random_read_ready, 1, 0);

  rc = create_entropy_store(DEFAULT_POOL_SIZE, &random_state);
  if (rc < 0) return rc;

  rc = create_entropy_store(SECONDARY_POOL_SIZE, &sec_random_state);
  if (rc < 0) return rc;

  rc = batch_entropy_init(BATCH_ENTROPY_SIZE);
  if (rc < 0) return rc;

  clear_entropy_store(random_state);
  clear_entropy_store(sec_random_state);
  init_std_data(random_state);

  //memset(&dpc_timer_state, 0, sizeof(struct timer_rand_state));
  //memset(&extract_timer_state, 0, sizeof(struct timer_rand_state));
  extract_timer_state.dont_count_entropy = 1;

  dev_make("random", &random_driver, NULL, NULL);
  dev_make("urandom", &urandom_driver, NULL, NULL);

  return 0;
}

#endif
