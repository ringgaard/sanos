//
// hash.h
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// A hashed lookup mechanism
//

#ifndef HASH_H
#define HASH_H

//
// This is the root of the hashed object.
//

struct hash 
{
  int hashsize;			// Width of bucket array
  unsigned long hashmask;       // Bit mask to match array size
  struct hash_node *buckets[0];	// Chains under each hash value
};

//
// Hash collision chains.  An internal data structure.
//

struct hash_node 
{
  struct hash_node *next;	// Next on hash chain
  unsigned long key;		// Key for this node
  void *data;			//  ...corresponding value
};

typedef int (*enumfunc_t)(int key, void *val, void *arg);

//
// Hash routines
//

struct hash *hash_alloc(int hashsize);
int hash_insert(struct hash *h, unsigned long key, void *val);
int hash_delete(struct hash *h, unsigned long key);
void *hash_lookup(struct hash *h, unsigned long key);
void hash_dealloc(struct hash *h);
int hash_foreach(struct hash *h, enumfunc_t f, void *arg);
int hash_size(struct hash *h);

#endif
