//
// mkfloppy.c
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Raw floppy writer utility
//

#define _WIN32_WINNT 0x500
#include <windows.h>
#include <stdio.h>

#define FLOPPY_SIZE (1440 * 1024)
#define BUFFER_SIZE (64 * 1024)

__int64 get_device_size(HANDLE hdev)
{
  GET_LENGTH_INFORMATION li;
  DWORD len;

  if (!DeviceIoControl(hdev, IOCTL_DISK_GET_LENGTH_INFO, NULL, 0, &li, sizeof(li), &len, NULL)) return -1;
  return li.Length.QuadPart;
}

int main(int argc, char *argv[])
{
  char devname[256];
  char *imgname;
  HANDLE hdev;
  HANDLE himg;
  __int64 devsize;
  DWORD bytes;
  char buffer[BUFFER_SIZE];

  if (argc != 3)
  {
    fprintf(stderr, "usage: mkfloppy [A:|B:] <floppy image>\n");
    return 1;
  }

  strcpy(devname, "\\\\.\\");
  strcat(devname, argv[1]);
  imgname = argv[2];

  hdev = CreateFile(devname, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_WRITE_THROUGH | FILE_FLAG_NO_BUFFERING, NULL);
  if (hdev == INVALID_HANDLE_VALUE) 
  {
    printf("mkfloppy: error %d opening device %s\n", GetLastError(), devname);
    return 3;
  }

  devsize = get_device_size(hdev);
  if (devsize < 0) 
  {
    printf("mkpart: error %d obtaining device size for %s\n", GetLastError(), devname);
    return 3;
  }

  if (devsize != FLOPPY_SIZE)
  {
    printf("mkfloppy: device is not a 1.44 MB floppy drive\n");
    return 3;
  }

  himg = CreateFile(imgname, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
  if (himg == INVALID_HANDLE_VALUE)
  {
    printf("mkfloppy: unable to open image file %s\n", imgname);
    return 3;
  }

  while (1)
  {
    if (!ReadFile(himg, buffer, BUFFER_SIZE, &bytes, NULL))
    {
      printf("mkfloppy: error %d reading from image file\n", GetLastError());
      break;
    }

    if (bytes == 0) break;

    if (!WriteFile(himg, buffer, bytes, &bytes, NULL))
    {
      printf("mkpart: error %d writing to device\n", GetLastError());
      break;
    }
  }

  CloseHandle(himg);
  CloseHandle(hdev);
}