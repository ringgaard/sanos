//
// buildimg.c
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Build bootable OS image
//

#include <stdio.h>
#include <sys/stat.h>

#define PAGESIZE        4096
#define SECTORSIZE      512
#define TRAILINGPAGES   16

unsigned char buffer[PAGESIZE];
int len;
struct stat st;
unsigned short krnlsize;

FILE *boot;
FILE *krnl;
FILE *image;

char *bootfn;
char *imagefn;
char *krnlfn;

int main(int argc, char **argv)
{
  int i;

  // check arguments
  if (argc < 4)
  {
    fprintf(stderr, "usage: buildimg <boot> <image> <kernel modules>\n");
    return 1;
  }

  bootfn = argv[1];
  imagefn = argv[2];

  printf("Boot sector %s\n", bootfn);
  // read boot sector
  boot = fopen(bootfn, "rb");
  if (!boot)
  {
    perror(bootfn);
    return 1;
  }

  len = fread(buffer, 1, SECTORSIZE, boot);
  if (len != 512)
  {
    fprintf(stderr, "Error: boot sector %d bytes (%d bytes expected)\n", len, SECTORSIZE);
    return 1;
  }

  fclose(boot);

  // get total size of image
  krnlsize = 0;
  for (i = 3; i < argc; i++)
  {
    krnlfn = argv[i];

    if (stat(krnlfn, &st) < 0)
    {
      perror(krnlfn);
      return 1;
    }

    printf("%4dK %s\n", st.st_size / 1024, krnlfn);
    krnlsize += st.st_size / SECTORSIZE;
  }

  buffer[0x0c] = krnlsize & 0xFF;
  buffer[0x0d] = krnlsize >> 8;

  // create image file
  image = fopen(imagefn, "wb");
  if (!image)
  {
    perror(imagefn);
    return 1;
  }

  // write boot sector first
  fwrite(buffer, 1, SECTORSIZE, image);

  // write each kernel module 
  for (i = 3; i < argc; i++)
  {
    krnlfn = argv[i];

    krnl = fopen(krnlfn, "rb");
    if (!krnl)
    {
      perror(krnlfn);
      return 1;
    }

    while (len = fread(buffer, 1, PAGESIZE, krnl))
    {
      fwrite(buffer, 1, len, image);
    }
  
    fclose(krnl);
  }

  memset(buffer, 0, PAGESIZE);
  for (i = 0; i < TRAILINGPAGES; i++) fwrite(buffer, 1, PAGESIZE, image);

  fclose(boot);
  fclose(image);

  printf("Image %s build complete, %dKB\n", imagefn, krnlsize / 2);
  return 0;
}

#if 0
int log2(int n)
{
  int log = 0;
  n--;
  while (n)
  {
    log++;
    n = n / 2;
  }
  return log;
}

void main()
{
  int i;

  for (i = 1; i <= 2048; i++)
  {
    //printf("log2(%d) = %d\n", i, log2(i));
    printf("%2d,", log2(i));
    if ((i % 32) == 0) printf("\n");
  }
}
#endif
