//
// nvram.c
//
// NVRAM driver
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

#define NVRAM_SIZE 128

static int nvram_ioctl(struct dev *dev, int cmd, void *args, size_t size) {
  switch (cmd) {
    case IOCTL_GETDEVSIZE:
      return NVRAM_SIZE;

    case IOCTL_GETBLKSIZE:
      return 1;
  }

  return -ENOSYS;
}

static int nvram_read(struct dev *dev, void *buffer, size_t count, blkno_t blkno, int flags) {
  unsigned int n;

  if (count == 0) return 0;
  if (blkno + count > NVRAM_SIZE) return -EFAULT;

  for (n = 0; n < count; n++) ((unsigned char *) buffer)[n] = read_cmos_reg(n + blkno);
  return count;
}

static int nvram_write(struct dev *dev, void *buffer, size_t count, blkno_t blkno, int flags) {
  unsigned int n;

  if (count == 0) return 0;
  if (blkno + count > NVRAM_SIZE) return -EFAULT;

  for (n = 0; n < count; n++) write_cmos_reg(n + blkno, ((unsigned char *) buffer)[n]);
  return count;
}

struct driver nvram_driver = {
  "nvram",
  DEV_TYPE_BLOCK,
  nvram_ioctl,
  nvram_read,
  nvram_write
};

int __declspec(dllexport) nvram() {
  dev_make("nvram", &nvram_driver, NULL, NULL);
  return 0;
}
