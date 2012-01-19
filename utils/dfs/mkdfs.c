#include <windows.h>

#include <stdio.h>
#include <time.h>

#include "getopt.h"
#include "blockdev.h"

#include "types.h"
#include "bitops.h"
#include "buf.h"
#include "vfs.h"
#include "dfs.h"
#include "mbr.h"

extern struct fs *mountlist;

#define K 1024

#define SECTORSIZE       512

#define BLOCKSIZE        4096
#define INODE_RATIO      4096

#define KRNLOPTS_POSOFS      0x1A
#define KRNLOPTS_LEN         128

char bootsect[SECTORSIZE];

char *command;
char *filedir;

char *devname = "dfs.img";
char *bootfile = NULL;
char *ldrfile = NULL;
char *krnlfile = NULL;
char *filelist = NULL;
char *devtype = "raw";
int devsize = 1440 * 2; // in sectors
int devcap = 1440 * 2; // in sectors
int blocksize = 4096;
int inoderatio = 4096;
int doinit = 0;
int dowipe = 0;
int doformat = 0;
int quick = 0;
char *source = NULL;
char *target = "";
int part = -1;
int part_start = 0;
int part_offset = 0;
int ldrsize;
char *krnlopts = "";
int altfile = 0;

void panic(char *reason)
{
  printf("panic: %s\n", reason);
  exit(1);
}

int dev_read(devno_t devno, void *buffer, size_t count, blkno_t blkno)
{
  struct blockdevice *blkdev = (struct blockdevice *) devno;
  if (bdrv_read(blkdev, blkno + part_offset, buffer, count / SECTORSIZE) < 0) panic("error reading from device"); 
  return count;
}

int dev_write(devno_t devno, void *buffer, size_t count, blkno_t blkno)
{
  struct blockdevice *blkdev = (struct blockdevice *) devno;
  if (bdrv_write(blkdev, blkno + part_offset, buffer, count / SECTORSIZE) < 0) panic("error reading from device"); 
  return count;
}

unsigned int dev_getsize(devno_t devno)
{
  unsigned int size;

  size = devsize;
  printf("device size is %d sectors (%dMB)\n", size, size / 2048);
  return size;
}

void create_device(char *devname, int devsize)
{
  if (bdrv_create(devtype, devname, devsize, 0) < 0) panic("unable to create device file");
}

void clear_device(struct blockdevice *bs, int devsize)
{
  char sector[SECTORSIZE];
  int i;

  memset(sector, 0, SECTORSIZE);
  for (i = 0; i < devsize; i++)
  {
    if (bdrv_write(bs, i, sector, 1) < 0) panic("error writing to device");
  }
}

void read_boot_sector(char *name)
{
  HANDLE hfile;
  DWORD bytes;

  hfile = CreateFile(name, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
  if (hfile == INVALID_HANDLE_VALUE) panic("unable to read boot sector");

  ReadFile(hfile, bootsect, SECTORSIZE, &bytes, NULL);

  CloseHandle(hfile);
}

void read_mbr(struct blockdevice *bs)
{
  struct master_boot_record mbr;

  if (bdrv_read(bs, 0, (unsigned char *) &mbr, 1) < 0) panic("error reading mbr from device");

  if (mbr.signature != MBR_SIGNATURE) panic("invalid master boot record");

  part_start = part_offset = mbr.parttab[part].relsect;
  devsize = mbr.parttab[part].numsect;

  printf("using partition %d, offset %d, %dK\n", part, part_start, devsize / 2);
}

void install_loader()
{
  HANDLE hfile;
  DWORD size;
  size_t count;
  struct filsys *fs;

  unsigned int i;
  unsigned int blocks;
  char *image;

  // Read loader
  hfile = CreateFile(ldrfile, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
  if (hfile == INVALID_HANDLE_VALUE) panic("unable to read os loader");
  size = GetFileSize(hfile, NULL);
  image = malloc(size);
  ReadFile(hfile, image, size, &count, NULL);
  CloseHandle(hfile);

  // Patch kernel options into image
  if (krnlopts)
  {
    int optspos;

    if (strlen(krnlopts) > KRNLOPTS_LEN - 1) panic("kernel options too long");
    optspos = *(unsigned short *) (image + KRNLOPTS_POSOFS);
    strcpy(image + optspos, krnlopts);
  }

  // Write loader to device
  fs = (struct filsys *) (mountlist->data); // assume first mounted device
  blocks = size / fs->blocksize;
  ldrsize = size / SECTORSIZE;
  if (blocks > fs->super->reserved_blocks) panic("not enough space for os loader");

  for (i = 0; i < blocks; i++)
  {
    dev_write(fs->devno, image + i * fs->blocksize, fs->blocksize, (fs->super->first_reserved_block + i) * (fs->blocksize / SECTORSIZE));
  }

  free(image);
}

unsigned int ft2time(FILETIME *ft)
{
  SYSTEMTIME st;
  struct tm tm;

  FileTimeToSystemTime(ft, &st);
  tm.tm_year = st.wYear - 1900;
  tm.tm_mon = st.wMonth - 1;
  tm.tm_mday = st.wDay;
  tm.tm_hour = st.wHour;
  tm.tm_min = st.wMinute;
  tm.tm_sec = st.wSecond;
  tm.tm_isdst = 0;

  return (int) mktime(&tm);
}

int isdir(char *filename)
{
  ULONG flags = GetFileAttributes(filename);
  if (flags == (DWORD) -1) return 0;
  return (flags & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

void set_time(HANDLE hfile, struct file *f)
{
  FILETIME ctime, atime, mtime;
  struct utimbuf t;

  GetFileTime(hfile, &ctime, &atime, &mtime);
  t.atime = ft2time(&atime);
  t.ctime = ft2time(&ctime);
  t.mtime = ft2time(&mtime);

  if (futime(f, &t) < 0) panic("error setting file time");
}

void install_kernel()
{
  HANDLE hfile;
  DWORD size;
  size_t count;

  struct file *file;
  char buf[4096];

  hfile = CreateFile(krnlfile, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
  size = GetFileSize(hfile, NULL);

  mkdir("/boot", 0755);
  file = open("/boot/krnl.dll", O_SPECIAL | (DFS_INODE_KERNEL << 24) | O_WRONLY, 0644);
  if (file == NULL) panic("error creating kernel file");

  ReadFile(hfile, buf, 4096, &count, NULL);
  while (count > 0)
  {
    write(file, buf, count);
    ReadFile(hfile, buf, 4096, &count, NULL);
  }
  
  set_time(hfile, file);
  close(file);
  CloseHandle(hfile);
}

void make_directory(char *dirname)
{
  mkdir(dirname, 0755);
}

void transfer_file(char *dstfn, char *srcfn)
{
  HANDLE hfile;
  DWORD size;
  size_t count;

  struct file *file;
  char buf[4096];
  char *ext;
  int executable = 0;

  hfile = CreateFile(srcfn, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
  size = GetFileSize(hfile, NULL);

  ext = dstfn + strlen(dstfn) - 1;
  while (ext > dstfn && *ext != '.' && *ext != PS1 && *ext != PS2) ext--;
  if (strcmp(ext, ".dll") == 0 || strcmp(ext, ".exe") == 0) executable = 1;

  file = open(dstfn, O_CREAT, executable ? 0755 : 0644);

  printf("%s -> %s (%d bytes)\n", srcfn, dstfn, size);

  ReadFile(hfile, buf, 4096, &count, NULL);
  while (count > 0)
  {
    write(file, buf, count);
    ReadFile(hfile, buf, 4096, &count, NULL);
  }
  
  set_time(hfile, file);
  close(file);
  CloseHandle(hfile);
}

void transfer_files(char *dstdir, char *srcdir)
{
  WIN32_FIND_DATA finddata;
  HANDLE hfind;
  BOOL more;
  char srcfn[255];
  char dstfn[255];

  strcpy(srcfn, srcdir);
  strcat(srcfn, "\\*");

  hfind = FindFirstFile(srcfn, &finddata);
  more = hfind != INVALID_HANDLE_VALUE;
  while (more)
  {
    strcpy(srcfn, srcdir);
    strcat(srcfn, "\\");
    strcat(srcfn, finddata.cFileName);

    strcpy(dstfn, dstdir);
    strcat(dstfn, "/");
    strcat(dstfn, finddata.cFileName);

    if (finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
    {
      if (strcmp(finddata.cFileName, ".") != 0 && strcmp(finddata.cFileName, "..") != 0)
      {
        make_directory(dstfn);
        transfer_files(dstfn, srcfn);
      }
    }
    else
      transfer_file(dstfn, srcfn);

    more = FindNextFile(hfind, &finddata);
  }

  FindClose(hfind);
}

void process_filelist(FILE *f)
{
  char line[1024];
  char *p;
  char *src;
  char *dst;
  char *altsrc;

  while (fgets(line, sizeof line, f))
  {
    p = line;
    while (*p && isspace(*p)) p++;
    if (!*p || *p == ';') continue;

    dst = p;
    src = NULL;

    while (*p && !isspace(*p)) p++;
    if (*p)
    {
      *p++ = 0;
      while (*p && isspace(*p)) p++;
      src = altsrc = p;
      while (*p && !isspace(*p)) p++;
      
      if (*p)
      {
        *p++ = 0;
        while (*p && isspace(*p)) p++;
        if (*p)
        {
          altsrc = p;
          while (*p && !isspace(*p)) p++;
          *p++ = 0;
        }
      }
    }

    if (altfile && altsrc && *altsrc) src = altsrc;

    if (!src || !*src)
    {
      make_directory(dst);
    }
    else
    {
      if (isdir(src))
      {
        make_directory(dst);
        transfer_files(dst, src);
      }
      else
        transfer_file(dst, src);
    }
  }
}

void list_file(char *filename)
{
  size_t count;
  char buf[4096];
  struct file *file;

  file = open(filename, 0, 0);
  if (!file)
  {
    printf("%s: file not found\n", filename);
    return;
  }

  while ((count = read(file, buf, 4096)) > 0)
  {
    fwrite(buf, 1, count, stdout);
  }

  close(file);
}

void usage()
{
  fprintf(stderr, "usage: mkdfs [options]\n\n");
  fprintf(stderr, "  -a (use alternative filenames from file list)\n");
  fprintf(stderr, "  -d <devname>\n");
  fprintf(stderr, "  -b <bootfile>\n");
  fprintf(stderr, "  -c <filesystem capacity> (capacity in kilobytes)\n");
  fprintf(stderr, "  -i (create new device)\n");
  fprintf(stderr, "  -f (format filesystem)\n");
  fprintf(stderr, "  -k <kernel image>\n");
  fprintf(stderr, "  -l <loader image>\n");
  fprintf(stderr, "  -t <device type> (raw or vmdk)\n");
  fprintf(stderr, "  -w (wipe out device, i.e. zero all sectors first)\n");
  fprintf(stderr, "  -p <partition>\n");
  fprintf(stderr, "  -P <partition start sector>\n");
  fprintf(stderr, "  -q (quick format)\n");
  fprintf(stderr, "  -B <block size> (default 4096)\n");
  fprintf(stderr, "  -C <device capacity> (capacity in kilobytes)\n");
  fprintf(stderr, "  -F <file list file>\n");
  fprintf(stderr, "  -I <inode ratio> (default 1 inode per 4K)\n");
  fprintf(stderr, "  -K <kernel options>\n");
  fprintf(stderr, "  -S <source directory or file>\n");
  fprintf(stderr, "  -T <target directory or file>\n");
}

int str2sectors(char *str)
{
  int size = 0;

  while (*str >= '0' && *str <= '9') size = size * 10 + (*str++ - '0');
  
  if (*str == 'K' || *str == 'k')
    size *= K / SECTORSIZE;
  else if (*str == 'M' || *str == 'm')
    size *= K * K / SECTORSIZE;
  else if (*str == 'G' || *str == 'g')
    size *= K * K * K / SECTORSIZE;
  else
    size *= K / SECTORSIZE;

  return size;
}

int main(int argc, char **argv)
{
  struct blockdevice blkdev;
  int c;

  // Parse command line options
  while ((c = getopt(argc, argv, "ad:b:c:ifk:l:t:wp:qB:C:F:I:K:P:S:T:?")) != EOF)
  {
    switch (c)
    {
      case 'a':
        altfile = !altfile;
        break;

      case 'd':
        devname = optarg;
        break;

      case 'b':
        bootfile = optarg;
        break;

      case 'c':
        devsize = devcap = str2sectors(optarg);
        break;

      case 'p':
        part = atoi(optarg);
        break;

      case 'i':
        doinit = !doinit;
        break;

      case 'f':
        doformat = !doformat;
        break;

      case 'k':
        krnlfile = optarg;
        break;

      case 'l':
        ldrfile = optarg;
        break;

      case 'w':
        dowipe = !dowipe;
        break;

      case 'q':
        quick = !quick;
        break;

      case 't':
        devtype = optarg;
        break;

      case 'B':
        blocksize = atoi(optarg);
        break;

      case 'C':
        devcap = str2sectors(optarg);
        break;

      case 'F':
        filelist = optarg;
        break;

      case 'I':
        inoderatio = atoi(optarg);
        break;

      case 'K':
        krnlopts = optarg;
        break;

      case 'P':
        part_start = atoi(optarg);
        break;

      case 'S':
        source = optarg;
        break;

      case 'T':
        target = optarg;
        break;

      case '?':
      default:
        usage();
        return 1;
    }
  }

  // Register dfs filesystem
  dfs_init();

  // Create device file
  if (doinit) 
  {
    printf("Creating %s device file %s %dKB\n", devtype, devname, devcap / 2);
    create_device(devname, devcap);
  }

  // Open device file
  if (bdrv_open(&blkdev, devname, NULL) < 0) panic("unable to open device");

  // Read master boot record
  if (part != -1)
  {
    read_mbr(&blkdev);
  }

  // Clear device
  if (dowipe) 
  {
    printf("Clearing device %s\n", devname);
    clear_device(&blkdev, devsize);
  }

  // Create filesystem
  if (doformat)
  {
    char options[256];

    sprintf(options, "blocksize=%d,inoderatio=%d%s", blocksize, inoderatio, quick ? ",quick" : "");
    printf("Formating device (%s)...\n", options);
    if (format((devno_t) &blkdev, "dfs", options) < 0) panic("error formatting device");
  }

  // Initialize the file system
  printf("Mounting device\n");
  if (mount("dfs", "/", (devno_t) &blkdev, NULL) < 0) panic("error mounting device");

  // Install os loader
  if (ldrfile) 
  {
    printf("Writing os loader %s\n", ldrfile);
    install_loader();
  }

  // Install kernel
  if (krnlfile) 
  {
    printf("Writing kernel %s\n", krnlfile);
    install_kernel();
  }

  // Write boot sector
  if (bootfile) 
  {
    struct boot_sector *bsect = (struct boot_sector *) bootsect;

    printf("Writing boot sector %s\n", bootfile);
    read_boot_sector(bootfile);
    bsect->ldrstart += part_start;
    bsect->ldrsize = ldrsize;
    dev_write((devno_t) &blkdev, bootsect, SECTORSIZE, 0);
  }

  // Copy files to device
  if (filelist)
  {
    FILE *f;

    printf("Transfering files from %s to device\n", filelist);

    f = fopen(filelist, "r");
    if (!f)
    {
      perror(filelist);
      return;
    }
    else
    {
      if (source) SetCurrentDirectory(source);
      process_filelist(f);
      fclose(f);
    }
  }
  else if (source)
  {
    printf("Transfering files\n");
    if (isdir(source))
    {
      transfer_files(target, source);
    }
    else
      transfer_file(target, source);
  }

  // Close file system
  printf("Unmounting device\n");
  unmount_all();

  // Close device file
  printf("Closing device\n");
  bdrv_close(&blkdev);

  return 0;
}
