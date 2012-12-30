#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <ctype.h>

#ifdef WIN32
#include <direct.h>
#include <io.h>
#include <windows.h>
#endif

#ifdef __linux__
#include <dirent.h>
#include <unistd.h>
#endif

#ifndef __linux__
#include "getopt.h"
#endif

#include "blockdev.h"

#include "types.h"
#include "bitops.h"
#include "buf.h"
#include "vfs.h"
#include "dfs.h"
#include "mbr.h"

#ifdef WIN32
#define S_IFMT _S_IFMT
#define S_IFDIR _S_IFDIR
#endif

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
int verbose = 0;

void panic(char *reason)
{
  printf("panic: %s\n", reason);
  exit(1);
}

int dev_read(vfs_devno_t devno, void *buffer, size_t count, vfs_blkno_t blkno)
{
  struct blockdevice *blkdev = (struct blockdevice *) devno;
  if (bdrv_read(blkdev, blkno + part_offset, buffer, count / SECTORSIZE) < 0) panic("error reading from device"); 
  return count;
}

int dev_write(vfs_devno_t devno, void *buffer, size_t count, vfs_blkno_t blkno)
{
  struct blockdevice *blkdev = (struct blockdevice *) devno;
  if (bdrv_write(blkdev, blkno + part_offset, buffer, count / SECTORSIZE) < 0) panic("error writing to device"); 
  return count;
}

unsigned int dev_getsize(vfs_devno_t devno)
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

size_t filesize(int fd)
{
  struct stat st;
  if (fstat(fd, &st) == -1) panic("cannot get file size");
  return st.st_size;
}

void read_boot_sector(char *name)
{
  int fd;
  size_t bytes;

  fd = open(name, O_BINARY);
  if (fd == -1) panic("unable to read boot sector");

  bytes = read(fd, bootsect, SECTORSIZE);
  if (bytes != SECTORSIZE) panic("bad bootsector");

  close(fd);
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
  int fd;
  size_t size;
  size_t count;
  struct filsys *fs;

  unsigned int i;
  unsigned int blocks;
  char *image;

  // Read loader
  fd = open(ldrfile, O_BINARY);
  if (fd == -1) panic("unable to read os loader");
  size = filesize(fd);
  image = malloc(size);
  count = read(fd, image, size);
  close(fd);
  if (count != size) panic("error reading os loader");

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

int isdir(char *filename)
{
  struct stat st;
  if (stat(filename, &st) == -1) 
  {
    perror(filename);
    panic("cannot stat file");
  }
  return (st.st_mode & S_IFMT) == S_IFDIR;
}

void set_time(int fd, struct file *f)
{
  struct stat st;
  struct vfs_utimbuf t;

  fstat(fd, &st);
  t.atime = st.st_atime;
  t.ctime = st.st_ctime;
  t.mtime = st.st_mtime;
  if (vfs_futime(f, &t) < 0) panic("error setting file time");
}

void install_kernel()
{
  int fd;
  size_t size;
  size_t count;

  struct file *file;
  char buf[4096];

  fd = open(krnlfile, O_BINARY);
  size = filesize(fd);

  vfs_mkdir("/boot", 0755);
  file = vfs_open("/boot/krnl.dll", VFS_O_SPECIAL | (DFS_INODE_KERNEL << 24) | O_WRONLY, 0644);
  if (file == NULL) panic("error creating kernel file");

  count = read(fd, buf, 4096);
  while (count > 0)
  {
    vfs_write(file, buf, count);
    count = read(fd, buf, 4096);
  }
  
  set_time(fd, file);
  vfs_close(file);
  close(fd);
}

void make_directory(char *dirname)
{
  vfs_mkdir(dirname, 0755);
}

void transfer_file(char *dstfn, char *srcfn)
{
  int fd;
  size_t size;
  size_t count;

  struct file *file;
  char buf[4096];
  char *ext;
  int executable = 0;

  fd = open(srcfn, O_BINARY);
  size = filesize(fd);

  ext = dstfn + strlen(dstfn) - 1;
  while (ext > dstfn && *ext != '.' && *ext != PS1 && *ext != PS2) ext--;
  if (strcmp(ext, ".dll") == 0 || 
      strcmp(ext, ".exe") == 0 || 
      strcmp(ext, ".sys") == 0 ||
      strcmp(ext, ".sh") == 0) {
    executable = 1;
  }

  file = vfs_open(dstfn, VFS_O_CREAT, executable ? 0755 : 0644);

  if (verbose) printf("%s -> %s (%d bytes)\n", srcfn, dstfn, size);

  count = read(fd, buf, 4096);
  while (count > 0)
  {
    vfs_write(file, buf, count);
    count = read(fd, buf, 4096);
  }
  
  set_time(fd, file);
  vfs_close(file);
  close(fd);
}

#ifdef WIN32
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
#else
void transfer_files(char *dstdir, char *srcdir)
{
  char srcfn[255];
  char dstfn[255];
  struct dirent *dp;

  DIR *dir = opendir(srcdir);
  if (dir == NULL) return;
  while ((dp = readdir(dir)) != NULL) 
  {
    strcpy(srcfn, srcdir);
    strcat(srcfn, "/");
    strcat(srcfn, dp->d_name);

    strcpy(dstfn, dstdir);
    strcat(dstfn, "/");
    strcat(dstfn, dp->d_name);

    if (isdir(srcfn))
    {
      if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0)
      {
        make_directory(dstfn);
        transfer_files(dstfn, srcfn);
      }
    }
    else
      transfer_file(dstfn, srcfn);
  }
  closedir(dir);
}
#endif

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
  fprintf(stderr, "  -v (verbose output)\n");
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
  while ((c = getopt(argc, argv, "ad:b:c:ifk:l:t:vwp:qB:C:F:I:K:P:S:T:?")) != EOF)
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

      case 'v':
        verbose++;
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
    if (vfs_format((vfs_devno_t) &blkdev, "dfs", options) < 0) panic("error formatting device");
  }

  // Initialize the file system
  printf("Mounting device\n");
  if (vfs_mount("dfs", "/", (vfs_devno_t) &blkdev, NULL) < 0) panic("error mounting device");

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
    dev_write((vfs_devno_t) &blkdev, bootsect, SECTORSIZE, 0);
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
      if (source) chdir(source);
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
  vfs_unmount_all();

  // Close device file
  printf("Closing device\n");
  bdrv_close(&blkdev);

  return 0;
}
