//
// hash.h
//
// A hashed lookup mechanism
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

#ifndef HASH_H
#define HASH_H

//
// This is the root of the hashed object.
//

struct hash  {
  int hashsize;                 // Width of bucket array
  unsigned long hashmask;       // Bit mask to match array size
  struct hash_node *buckets[0]; // Chains under each hash value
};

//
// Hash collision chains.  An internal data structure.
//

struct hash_node {
  struct hash_node *next;       // Next on hash chain
  unsigned long key;            // Key for this node
  void *data;                   //  ...corresponding value
};

typedef int (*enumfunc_t)(int key, void *val, void *arg);

//
// Hash routines
//

#ifdef  __cplusplus
extern "C" {
#endif

struct hash *hash_alloc(int hashsize);
int hash_insert(struct hash *h, unsigned long key, void *val);
int hash_delete(struct hash *h, unsigned long key);
void *hash_lookup(struct hash *h, unsigned long key);
void hash_dealloc(struct hash *h);
int hash_foreach(struct hash *h, enumfunc_t f, void *arg);
int hash_size(struct hash *h);

#ifdef  __cplusplus
}
#endif

#endif
