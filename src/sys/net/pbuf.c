//
// pbuf.c
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Packet buffer management
//

#include <net/net.h>

static struct pbuf *pbuf_pool = NULL;
static struct pbuf *pbuf_pool_alloc_cache = NULL;
static struct pbuf *pbuf_pool_free_cache = NULL;

static int pbuf_pool_free_lock, pbuf_pool_alloc_lock;

//
// pbuf_init
//
// Initializes the pbuf module. A large part of memory is allocated
// for holding the pool of pbufs. The size of the individual pbufs in
// the pool is given by the size parameter, and the number of pbufs in
// the pool by the num parameter.
//
// After the memory has been allocated, the pbufs are set up. The
// next pointer in each pbuf is set up to point to the next pbuf in
// the pool.
//

void pbuf_init()
{
  struct pbuf *p, *q;
  int i;

  // Allocate buffer pool
  pbuf_pool = (struct pbuf *) kmalloc(PBUF_POOL_SIZE * (PBUF_POOL_BUFSIZE + sizeof(struct pbuf)));
  stats.pbuf.avail = PBUF_POOL_SIZE;
  
  // Set up next pointers to link the pbufs of the pool together
  p = pbuf_pool;
  
  for (i = 0; i < PBUF_POOL_SIZE; i++)
  {
    p->next = (struct pbuf *) ((char *) p + PBUF_POOL_BUFSIZE + sizeof(struct pbuf));
    p->len = p->tot_len = PBUF_POOL_BUFSIZE;
    p->payload = (void *) ((char *) p + sizeof(struct pbuf));
    q = p;
    p = p->next;
  }
  
  // The next pointer of last pbuf is NULL to indicate that there are no more pbufs in the pool
  q->next = NULL;

  pbuf_pool_alloc_lock = 0;
  pbuf_pool_free_lock = 0;
  //pbuf_pool_free_sem = sys_sem_new(1);
}

static struct pbuf *pbuf_pool_alloc()
{
  struct pbuf *p = NULL;

  // First, see if there are pbufs in the cache
  if (pbuf_pool_alloc_cache) 
  {
    p = pbuf_pool_alloc_cache;
    if (p) pbuf_pool_alloc_cache = p->next; 
  } 
  else 
  {
    // Next, check the actual pbuf pool, but if the pool is locked, we
    // pretend to be out of buffers and return NULL
    if (pbuf_pool_free_lock) 
    {
      stats.pbuf.alloc_locked++;
      return NULL;
    }
    
    pbuf_pool_alloc_lock++;
    if (!pbuf_pool_free_lock)
    {
      p = pbuf_pool;
      if(p) pbuf_pool = p->next; 
    } 
    else
      stats.pbuf.alloc_locked++;

    pbuf_pool_alloc_lock--;
  }

  if (p != NULL) 
  {
    stats.pbuf.used++;
    if (stats.pbuf.used > stats.pbuf.max) stats.pbuf.max = stats.pbuf.used;
  }

  return p;   
}

static void pbuf_pool_free(struct pbuf *p)
{
  struct pbuf *q;

  for (q = p; q != NULL; q = q->next) stats.pbuf.used--;
  
  if (pbuf_pool_alloc_cache == NULL) 
  {
    pbuf_pool_alloc_cache = p;
  }
  else 
  {
    for (q = pbuf_pool_alloc_cache; q->next != NULL; q = q->next);
    q->next = p;    
  }
}

//
// pbuf_alloc
//
// Allocates a pbuf at protocol layer. The actual memory allocated
// for the pbuf is determined by the layer at which the pbuf is
// allocated and the requested size (from the size parameter). The
// flag parameter decides how and where the pbuf should be allocated
// as follows:
//
// * PBUF_RW:    buffer memory for pbuf is allocated as one large
//               chunk. This includes protocol headers as well. 
// * PBUF_RO:    no buffer memory is allocated for the pbuf, even for
//               protocol headers. Additional headers must be prepended
//               by allocating another pbuf and chain in to the front of
//               the ROM pbuf.	       
// * PBUF_POOL:  the pbuf is allocated as a pbuf chain, with pbufs from
//               the pbuf pool that is allocated during pbuf_init.
//

struct pbuf *pbuf_alloc(int layer, int size, int flag)
{
  struct pbuf *p, *q, *r;
  int offset;
  int rsize;

  //kprintf("pbuf_alloc: alloc %d bytes layer=%d flags=%d\n", size, layer, flag);

  offset = 0;
  switch (layer)
  {
    case PBUF_TRANSPORT:
      offset += PBUF_TRANSPORT_HLEN;
      // FALLTHROUGH

    case PBUF_IP:
      offset += PBUF_IP_HLEN;
      offset += PBUF_LINK_HLEN;
      // FALLTHROUGH

    case PBUF_LINK:
      break;

    case PBUF_RAW:
      break;

    default:
      panic("pbuf_alloc: bad pbuf layer");
  }

  switch (flag) 
  {
    case PBUF_POOL:
      // Allocate head of pbuf chain into p
      p = pbuf_pool_alloc();
      if (p == NULL) 
      {
	stats.pbuf.err++;
	return NULL;
      }
      p->next = NULL;
    
      // Set the payload pointer so that it points offset bytes into pbuf data memory
      p->payload = (void *) ((char *) p + sizeof(struct pbuf) + offset);

      // The total length of the pbuf is the requested size
      p->tot_len = size;

      // Set the length of the first pbuf in the chain
      p->len = size > PBUF_POOL_BUFSIZE - offset ? PBUF_POOL_BUFSIZE - offset: size;

      p->flags = PBUF_FLAG_POOL;
    
      // Allocate the tail of the pbuf chain
      r = p;
      rsize = size - p->len;
      while(rsize > 0) 
      {
	q = pbuf_pool_alloc();
	if (q == NULL) 
	{
	  stats.pbuf.err++;
	  pbuf_pool_free(p);
	  return NULL;
	}
	q->next = NULL;
	r->next = q;
	q->len = rsize > PBUF_POOL_BUFSIZE ? PBUF_POOL_BUFSIZE: rsize;
	q->flags = PBUF_FLAG_POOL;
	q->payload = (void *) ((char *) q + sizeof(struct pbuf));
	r = q;
	q->ref = 1;
	q = q->next;
	rsize -= PBUF_POOL_BUFSIZE;
      }
      r->next = NULL;
      break;

    case PBUF_RW:
      // If pbuf is to be allocated in RAM, allocate memory for it
      p = (struct pbuf *) kmalloc(sizeof(struct pbuf) + size + offset);
      if (p == NULL) return NULL;

      // Set up internal structure of the pbuf.
      p->payload = (void *) ((char *) p + sizeof(struct pbuf) + offset);
      p->len = p->tot_len = size;
      p->next = NULL;
      p->flags = PBUF_FLAG_RW;
      stats.pbuf.rwbufs++;
      break;

    case PBUF_RO:
      // If the pbuf should point to ROM, we only need to allocate memory for the pbuf structure
      p = (struct pbuf *) kmalloc(sizeof(struct pbuf));
      if (p == NULL) return NULL;

      p->payload = NULL;
      p->len = p->tot_len = size;
      p->next = NULL;
      p->flags = PBUF_FLAG_RO;
      break;

    default:
      panic("pbuf_alloc: erroneous flag");
  }

  //kprintf("pbuf: %d bufs\n", stats.pbuf.rwbufs);
  p->ref = 1;
  return p;
}

//
// pbuf_refresh
//
// Moves free buffers from the pbuf_pool_free_cache to the pbuf_pool
// list (if possible).
//

void pbuf_refresh()
{
  struct pbuf *p;

  //sys_sem_wait(pbuf_pool_free_sem);
  
  if (pbuf_pool_free_cache != NULL) 
  {
    pbuf_pool_free_lock++;
    if (!pbuf_pool_alloc_lock) 
    {
      if (pbuf_pool == NULL)
      {
	pbuf_pool = pbuf_pool_free_cache;	
      } 
      else 
      {  
	for(p = pbuf_pool; p->next != NULL; p = p->next);
	p->next = pbuf_pool_free_cache;
      }

      pbuf_pool_free_cache = NULL;
    } 
    else 
    {
      stats.pbuf.refresh_locked++;
    }
    
    pbuf_pool_free_lock--;
  }
  
  //sys_sem_signal(pbuf_pool_free_sem);
}

//
// pbuf_realloc:
//
// Reallocates the memory for a pbuf. If the pbuf is RO, this as
// simple as to adjust the tot_len and len fields. If the pbuf is
// a pbuf chain, as it might be with both pbufs in dynamically
// allocated RAM and for pbufs from the pbuf pool, we have to step
// through the chain until we find the new endpoint in the pbuf chain.
// Then the pbuf that is right on the endpoint is resized and any
// further pbufs on the chain are deallocated.
//

void pbuf_realloc(struct pbuf *p, int size)
{
  struct pbuf *q, *r;
  int rsize;

  if (p->tot_len <= size) return;

  switch (p->flags) 
  {
    case PBUF_FLAG_POOL:
      // First, step over any pbufs that should still be in the chain
      rsize = size;
      q = p;  
      while (rsize > q->len)
      {
	rsize -= q->len;
	q = q->next;
      }
      
      // Adjust the length of the pbuf that will be halved
      q->len = rsize;

      // And deallocate any left over pbufs
      r = q->next;
      q->next = NULL;
      q = r;
      while (q != NULL) 
      {
	r = q->next;

        //sys_sem_wait(pbuf_pool_free_sem);
        q->next = pbuf_pool_free_cache;
        pbuf_pool_free_cache = q;
        //sys_sem_signal(pbuf_pool_free_sem);

	stats.pbuf.used--;
	q = r;
      }
      break;

    case PBUF_FLAG_RO:
      p->len = size;
      break;

    case PBUF_FLAG_RW:
      // First, step over the pbufs that should still be in the chain.
      rsize = size;
      q = p;
      while (rsize > q->len) 
      {
	rsize -= q->len;
	q = q->next;
      }

      if (q->flags == PBUF_FLAG_RW)
      {
	// Reallocate and adjust the length of the pbuf that will be halved
	// TODO: we cannot reallocate the buffer without relinking it, we just leave it for now
        // mem_realloc(q, (u8_t *)q->payload - (u8_t *)q + rsize/sizeof(u8_t));
      }
    
      q->len = rsize;
    
      // And deallocate any left over pbufs
      r = q->next;
      q->next = NULL;
      pbuf_free(r);
      break;
  }

  p->tot_len = size;
  pbuf_refresh();
}

//
// pbuf_header
//
// Adjusts the payload pointer so that space for a header appears in
// the pbuf. Also, the tot_len and len fields are adjusted.
//

int pbuf_header(struct pbuf *p, int header_size)
{
  void *payload;

  payload = p->payload;
  p->payload = (char *) (p->payload) - header_size;
  if ((char *) p->payload < (char *) p + sizeof(struct pbuf))
  {
    p->payload = payload;
    return -1;
  }
  p->len += header_size;
  p->tot_len += header_size;
  
  return 0;
}

//
// pbuf_free
//
// Decrements the reference count and deallocates the pbuf if the
// reference count is zero. If the pbuf is a chain all pbufs in the
// chain are deallocated.
//

int pbuf_free(struct pbuf *p)
{
  struct pbuf *q;
  int count = 0;
    
  if (p == NULL) return 0;

  // Decrement reference count
  p->ref--;

  // If reference count is zero, actually deallocate pbuf
  if (p->ref == 0) 
  {
    q = NULL;
    while (p != NULL) 
    {
      // Check if this is a pbuf from the pool
      if (p->flags == PBUF_FLAG_POOL) 
      {
	p->len = p->tot_len = PBUF_POOL_BUFSIZE;
	p->payload = (void *)((char *) p + sizeof(struct pbuf));
	q = p->next;

        //sys_sem_wait(pbuf_pool_free_sem);
        p->next = pbuf_pool_free_cache;
        pbuf_pool_free_cache = p;
        //sys_sem_signal(pbuf_pool_free_sem);

	stats.pbuf.used--;
      } 
      else if (p->flags == PBUF_FLAG_RO)
      {
	q = p->next;
	kfree(p);
      } 
      else 
      {
	q = p->next;
        stats.pbuf.rwbufs--;
	kfree(p);
      }

      p = q;
      count++;
    }
  }

  //kprintf("pbuf: %d bufs\n", stats.pbuf.rwbufs);
  pbuf_refresh();
  return count;
}

//
// pbuf_clen
//
// Returns the length of the pbuf chain.
//

int pbuf_clen(struct pbuf *p)
{
  int len;

  if (!p) return 0;
  
  for (len = 0; p != NULL; p = p->next) ++len;

  return len;
}

//
// pbuf_ref
//
// Increments the reference count of the pbuf
//

void pbuf_ref(struct pbuf *p)
{
  if (p == NULL) return;
  p->ref++;
}

//
// pbuf_chain
//
// Chains the two pbufs h and t together. The tot_len field of the
// first pbuf (h) is adjusted.
//

void pbuf_chain(struct pbuf *h, struct pbuf *t)
{
  struct pbuf *p;

  for (p = h; p->next != NULL; p = p->next);
  p->next = t;
  h->tot_len += t->tot_len;  
}

//
// pbuf_dechain
//
// Adjusts the tot_len field of the pbuf and returns the tail (if
// any) of the pbuf chain.
//

struct pbuf *pbuf_dechain(struct pbuf *p)
{
  struct pbuf *q;
  
  q = p->next;
  if (q) q->tot_len = p->tot_len - p->len;
  p->tot_len = p->len;
  p->next = NULL;
  return q;
}
