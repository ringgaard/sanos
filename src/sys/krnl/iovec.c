//
// iovec.c
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Scatter/gather utility routines
//

#include <os/krnl.h>

int check_iovec(struct iovec *iov, int iovlen)
{
  if (iov)
  {
    if (iovlen < 0) return -EINVAL;
    if (KERNELSPACE(iov)) return -EFAULT;
    if (!mem_mapped(iov, iovlen * sizeof(struct iovec))) return -EFAULT;
    while (iovlen > 0)
    {
      if (iov->iov_len < 0) return -EINVAL;
      if (iov->iov_base)
      {
	if (KERNELSPACE(iov->iov_base)) return -EFAULT;
	if (!mem_mapped(iov->iov_base, iov->iov_len)) return -EFAULT;
      }
      else if (iov->iov_len != 0) 
	return -EFAULT;

      iov++;
      iovlen--;
    }
  }
  else
  {
    if (iovlen != 0) return -EFAULT;
  }

  return 0;
}

size_t get_iovec_size(struct iovec *iov, int iovlen)
{
  size_t size = 0;

  while (iovlen > 0)
  {
    size += iov->iov_len;
    iov++;
    iovlen--;
  }

  return size;
}

struct iovec *dup_iovec(struct iovec *iov, int iovlen)
{
  struct iovec *newiov;

  newiov = (struct iovec *) kmalloc(iovlen * sizeof(struct iovec));
  if (!newiov) return NULL;
  memcpy(newiov, iov, iovlen * sizeof(struct iovec));

  return newiov;
}

int read_iovec(struct iovec *iov, int iovlen, char *buf, size_t count)
{
  size_t read = 0;
  size_t len;

  while (count > 0 && iovlen > 0)
  {
    if (iov->iov_len > 0)
    {
      len = iov->iov_len;
      if (count < iov->iov_len) len = count;

      memcpy(buf, iov->iov_base, len);
      read += len;

      (char *) iov->iov_base += len;
      iov->iov_len -= len;

      buf += len;
      count -= len;
    }

    iov++;
    iovlen--;
  }

  return read;
}

int write_iovec(struct iovec *iov, int iovlen, char *buf, size_t count)
{
  size_t written = 0;
  size_t len;

  while (count > 0 && iovlen > 0)
  {
    if (iov->iov_len > 0)
    {
      len = iov->iov_len;
      if (count < iov->iov_len) len = count;

      memcpy(iov->iov_base, buf, len);
      written += len;

      (char *) iov->iov_base += len;
      iov->iov_len -= len;

      buf += len;
      count -= len;
    }

    iov++;
    iovlen--;
  }

  return written;
}
