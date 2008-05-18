//
// mkpart.c
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Partition table utility
//

#define _WIN32_WINNT 0x500
#include <windows.h>
#include <stdio.h>

#define MBR_SIGNATURE            0xAA55

#pragma pack(push)
#pragma pack(1)

struct disk_partition 
{
  unsigned char bootid;   // Bootable?  0=no, 128=yes
  unsigned char beghead;  // Beginning head number
  unsigned char begsect;  // Beginning sector number
  unsigned char begcyl;   // 10 bit nmbr, with high 2 bits put in begsect
  unsigned char systid;   // Operating System type indicator code
  unsigned char endhead;  // Ending head number
  unsigned char endsect;  // Ending sector number
  unsigned char endcyl;   // Also a 10 bit nmbr, with same high 2 bit trick
  unsigned int relsect;   // First sector relative to start of disk
  unsigned int numsect;   // Number of sectors in partition
};

struct master_boot_record 
{
  char bootstrap[446];
  struct disk_partition parttab[4];
  unsigned short signature;
};

#pragma pack(pop)

unsigned char bootrecord[512] =
{
  0x33, 0xC0, 0x8E, 0xD0, 0xBC, 0x00, 0x7C, 0xFB, 0x50, 0x07, 0x50, 0x1F, 0xFC, 0xBE, 0x1B, 0x7C,
  0xBF, 0x1B, 0x06, 0x50, 0x57, 0xB9, 0xE5, 0x01, 0xF3, 0xA4, 0xCB, 0xBD, 0xBE, 0x07, 0xB1, 0x04,
  0x38, 0x6E, 0x00, 0x7C, 0x09, 0x75, 0x13, 0x83, 0xC5, 0x10, 0xE2, 0xF4, 0xCD, 0x18, 0x8B, 0xF5,
  0x83, 0xC6, 0x10, 0x49, 0x74, 0x19, 0x38, 0x2C, 0x74, 0xF6, 0xA0, 0xB5, 0x07, 0xB4, 0x07, 0x8B,
  0xF0, 0xAC, 0x3C, 0x00, 0x74, 0xFC, 0xBB, 0x07, 0x00, 0xB4, 0x0E, 0xCD, 0x10, 0xEB, 0xF2, 0x88,
  0x4E, 0x10, 0xE8, 0x46, 0x00, 0x73, 0x2A, 0xFE, 0x46, 0x10, 0x80, 0x7E, 0x04, 0x0B, 0x74, 0x0B,
  0x80, 0x7E, 0x04, 0x0C, 0x74, 0x05, 0xA0, 0xB6, 0x07, 0x75, 0xD2, 0x80, 0x46, 0x02, 0x06, 0x83,
  0x46, 0x08, 0x06, 0x83, 0x56, 0x0A, 0x00, 0xE8, 0x21, 0x00, 0x73, 0x05, 0xA0, 0xB6, 0x07, 0xEB,
  0xBC, 0x81, 0x3E, 0xFE, 0x7D, 0x55, 0xAA, 0x74, 0x0B, 0x80, 0x7E, 0x10, 0x00, 0x74, 0xC8, 0xA0,
  0xB7, 0x07, 0xEB, 0xA9, 0x8B, 0xFC, 0x1E, 0x57, 0x8B, 0xF5, 0xCB, 0xBF, 0x05, 0x00, 0x8A, 0x56,
  0x00, 0xB4, 0x08, 0xCD, 0x13, 0x72, 0x23, 0x8A, 0xC1, 0x24, 0x3F, 0x98, 0x8A, 0xDE, 0x8A, 0xFC,
  0x43, 0xF7, 0xE3, 0x8B, 0xD1, 0x86, 0xD6, 0xB1, 0x06, 0xD2, 0xEE, 0x42, 0xF7, 0xE2, 0x39, 0x56,
  0x0A, 0x77, 0x23, 0x72, 0x05, 0x39, 0x46, 0x08, 0x73, 0x1C, 0xB8, 0x01, 0x02, 0xBB, 0x00, 0x7C,
  0x8B, 0x4E, 0x02, 0x8B, 0x56, 0x00, 0xCD, 0x13, 0x73, 0x51, 0x4F, 0x74, 0x4E, 0x32, 0xE4, 0x8A,
  0x56, 0x00, 0xCD, 0x13, 0xEB, 0xE4, 0x8A, 0x56, 0x00, 0x60, 0xBB, 0xAA, 0x55, 0xB4, 0x41, 0xCD,
  0x13, 0x72, 0x36, 0x81, 0xFB, 0x55, 0xAA, 0x75, 0x30, 0xF6, 0xC1, 0x01, 0x74, 0x2B, 0x61, 0x60,
  0x6A, 0x00, 0x6A, 0x00, 0xFF, 0x76, 0x0A, 0xFF, 0x76, 0x08, 0x6A, 0x00, 0x68, 0x00, 0x7C, 0x6A,
  0x01, 0x6A, 0x10, 0xB4, 0x42, 0x8B, 0xF4, 0xCD, 0x13, 0x61, 0x61, 0x73, 0x0E, 0x4F, 0x74, 0x0B,
  0x32, 0xE4, 0x8A, 0x56, 0x00, 0xCD, 0x13, 0xEB, 0xD6, 0x61, 0xF9, 0xC3, 0x49, 0x6E, 0x76, 0x61,
  0x6C, 0x69, 0x64, 0x20, 0x70, 0x61, 0x72, 0x74, 0x69, 0x74, 0x69, 0x6F, 0x6E, 0x20, 0x74, 0x61,
  0x62, 0x6C, 0x65, 0x00, 0x45, 0x72, 0x72, 0x6F, 0x72, 0x20, 0x6C, 0x6F, 0x61, 0x64, 0x69, 0x6E,
  0x67, 0x20, 0x6F, 0x70, 0x65, 0x72, 0x61, 0x74, 0x69, 0x6E, 0x67, 0x20, 0x73, 0x79, 0x73, 0x74,
  0x65, 0x6D, 0x00, 0x4D, 0x69, 0x73, 0x73, 0x69, 0x6E, 0x67, 0x20, 0x6F, 0x70, 0x65, 0x72, 0x61,
  0x74, 0x69, 0x6E, 0x67, 0x20, 0x73, 0x79, 0x73, 0x74, 0x65, 0x6D, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0xAA
};

char bootsect[512];
__int64 devsize;
char partimages[4][256];

int read_boot_sector(HANDLE hdev)
{
  DWORD bytes;

  if (SetFilePointer(hdev, 0, NULL, FILE_BEGIN) == -1) return -1;
  if (!ReadFile(hdev, bootsect, 512, &bytes, NULL)) return -1;

  return 0;
}

int read_mbr_file(char *filename)
{
  HANDLE himg;
  DWORD bytes;

  himg = CreateFile(filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
  if (himg == INVALID_HANDLE_VALUE)
  {
    printf("mkpart: unable to open partition file %s\n", filename);
    return -1;
  }
  if (!ReadFile(himg, bootsect, 512, &bytes, NULL)) return -1;
  CloseHandle(himg);
  return 0;
}

int write_boot_sector(HANDLE hdev)
{
  DWORD bytes;

  if (SetFilePointer(hdev, 0, NULL, FILE_BEGIN) == -1) return -1;
  if (!WriteFile(hdev, bootsect, 512, &bytes, NULL)) return -1;

  return 0;
}

int generate_partition_info(char *fn)
{
  FILE *f;
  int i;
  struct master_boot_record *mbr;
  struct disk_partition *p;

  f = fopen(fn, "w");
  if (!f) return -1;

  fprintf(f, "%I64d\n", devsize);

  mbr = (struct master_boot_record *) bootsect;
  for (i = 0; i < 4; i++)
  {
    p = &mbr->parttab[i];

    fprintf(f, "%d %d (%d/%d/%d) (%d/%d/%d) %lu %lu -\n", 
      p->bootid, p->systid,
      p->begcyl + ((p->begsect >> 6) << 8), p->beghead, p->begsect & 0x3F,
      p->endcyl + ((p->endsect >> 6) << 8), p->endhead, p->endsect & 0x3F, 
      p->relsect, p->numsect);
  }

  fclose(f);
  return 0;
}

int set_partition_info(char *fn)
{
  FILE *f;
  int i;
  struct master_boot_record *mbr;
  struct disk_partition *p;
  int bootid, systid;
  int begcyl, beghead, begsect;
  int endcyl, endhead, endsect;

  f = fopen(fn, "r");
  if (!f) return -1;

  fscanf(f, "%I64d\n", &devsize);

  mbr = (struct master_boot_record *) bootsect;
  for (i = 0; i < 4; i++)
  {
    p = &mbr->parttab[i];

    fscanf(f, "%d %d (%d/%d/%d) (%d/%d/%d) %lu %lu %s\n",
      &bootid, &systid,
      &begcyl, &beghead, &begsect,
      &endcyl, &endhead, &endsect,
      &p->relsect, &p->numsect, partimages[i]);

    p->bootid = bootid;
    p->systid = systid;

    p->begcyl = begcyl & 0xFF;
    p->beghead = beghead;
    p->begsect = begsect | ((begcyl >> 8) << 6);

    p->endcyl = endcyl & 0xFF;
    p->endhead = endhead;
    p->endsect = endsect | ((endcyl >> 8) << 6);

    //printf("%d %d (%d/%d/%d) (%d/%d/%d) %lu %lu\n",
    //  p->bootid, p->systid,
    //  p->begcyl + ((p->begsect >> 6) << 8), p->beghead, p->begsect & 0x3F,
    //  p->endcyl + ((p->endsect >> 6) << 8), p->endhead, p->endsect & 0x3F, 
    //  p->relsect, p->numsect);
  }

  fclose(f);
  return 0;
}

__int64 get_device_size(HANDLE hdev)
{
  GET_LENGTH_INFORMATION li;
  DISK_GEOMETRY geom;
  DWORD len;

  if (DeviceIoControl(hdev, IOCTL_DISK_GET_LENGTH_INFO, NULL, 0, &li, sizeof(li), &len, NULL)) 
  {
    return li.Length.QuadPart;
  }

  if (DeviceIoControl(hdev, IOCTL_DISK_GET_DRIVE_GEOMETRY, NULL, 0, &geom, sizeof(geom), &len, NULL)) 
  {
    return geom.Cylinders.QuadPart * geom.TracksPerCylinder * geom.SectorsPerTrack * geom.BytesPerSector;
  }

  return -1;
}

void write_image(HANDLE hdev, unsigned int startsect, unsigned int numsect, char *filename)
{
  HANDLE himg;
  LARGE_INTEGER filesize;
  LARGE_INTEGER partstart;
  char buf[64 * 1024];
  unsigned int sectno;
  unsigned long bytes;

  himg = CreateFile(filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
  if (himg == INVALID_HANDLE_VALUE)
  {
    printf("mkpart: unable to open partition file %s\n", filename);
    return;
  }

  GetFileSizeEx(himg, &filesize);
  if (filesize.QuadPart / 512 != numsect)
  {
    printf("mkpart: partition image is %ul sector, %ul expected\n", (unsigned int) (filesize.QuadPart / 512), numsect);
    return;
  }

  partstart.QuadPart = startsect * 512;
  if (!SetFilePointerEx(hdev, partstart, NULL, FILE_BEGIN))
  {
    printf("mkpart: error %d position file pointer for device\n", GetLastError());
    return;
  }

  sectno = 0;
  while (sectno < numsect)
  {
    if (!ReadFile(himg, buf, sizeof buf, &bytes, NULL))
    {
      printf("mkpart: error %d reading from partition image file\n", GetLastError());
      return;
    }

    if (!WriteFile(hdev, buf, bytes, &bytes, NULL))
    {
      printf("mkpart: error %d writing to volume\n", GetLastError());
      return;
    }

    sectno += (bytes / 512);
  }

  CloseHandle(himg);
}

void write_images(HANDLE hdev)
{
  int i;
  struct master_boot_record *mbr;

  mbr = (struct master_boot_record *) bootsect;
  for (i = 0; i < 4; i++)
    if (strcmp(partimages[i], "-") != 0)
    {
      printf("writing partition image %s to partition %d\n", partimages[i], i);
      write_image(hdev, mbr->parttab[i].relsect, mbr->parttab[i].numsect, partimages[i]);
    }
}

int main(int argc, char **argv)
{
  HANDLE hdev;
  char *devname;
  char *mbrfn;
  int readmode;
  char *partinf;

  // Check arguments
  if (argc != 4 && argc != 5)
  {
    printf("usage: [-r|-w] <device> <partition info file> [<mbr>]\n");
    return 1;
  }

  if (strcmp(argv[1], "-r") == 0)
    readmode = 1;
  else if (strcmp(argv[1], "-w") == 0)
    readmode = 0;
  else
  {
    printf("mkpart: specify -r for read or -w for write partition\n");
    return 1;
  }

  devname = argv[2];
  partinf = argv[3];
  mbrfn = argc == 5 ? argv[4] : NULL;

  // Open device file
  hdev = CreateFile(devname, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_WRITE_THROUGH | FILE_FLAG_NO_BUFFERING, NULL);
  if (hdev == INVALID_HANDLE_VALUE) 
  {
    printf("mkpart: error %d opening device %s\n", GetLastError(), devname);
    return 3;
  }

  // Read partition info
  if (readmode)
  {
    devsize = get_device_size(hdev);
    if (devsize < 0) 
    {
      printf("mkpart: error %d obtaining device size for %s\n", GetLastError(), devname);
      return 3;
    }

    if (read_boot_sector(hdev) < 0)
    {
      printf("mkpart: error reading master boot record\n");
      return 3;
    }

    if (generate_partition_info(partinf) < 0)
    {
      printf("mkpart: error generating partition info from device\n");
      return 3;
    }
  }
  else
  {
    __int64 real_devsize = get_device_size(hdev);
    if (devsize < 0) 
    {
      printf("mkpart: error %d obtaining device size for %s\n", GetLastError(), devname);
      return 3;
    }

    if (mbrfn != NULL)
    {
      printf("mkpart: reading MBR from %s\n", mbrfn);
      if (read_mbr_file(mbrfn) < 0)
      {
        printf("mkpart: error %d reading MBR from %s\n", GetLastError(), mbrfn);
        return 3;
      }
    }
    else
      memcpy(bootsect, bootrecord, 512);

    if (set_partition_info(partinf) < 0)
    {
      printf("mkpart: error reading partition info from %s\n", partinf);
      return 3;
    }

    if (devsize != real_devsize)
    {
      printf("mkpart: volume size is %I64d bytes, %I64d bytes expected \n", real_devsize, devsize);
      return 3;
    }

    write_images(hdev);

    if (write_boot_sector(hdev) < 0)
    {
      printf("mkpart: error writing master boot record\n");
      return 3;
    }

    printf("partition table written to %s\n", devname);
  }

  CloseHandle(hdev);
  return 0;
}
