#include <windows.h>

#include <stdio.h>
#include <time.h>

#include "getopt.h"

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
int devsize = 1440 * 2;
int devcap = 1440 * 2;
int blocksize = 4096;
int inoderatio = 4096;
int doshell = 0;
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

int get_tick_count()
{
  return (int) GetTickCount();
}

void panic(char *reason)
{
  printf("panic: %s\n", reason);
  exit(1);
}

int dev_read(devno_t devno, void *buffer, size_t count, blkno_t blkno)
{
  DWORD bytes;

  //printf("read block %d, %d bytes\n", blkno, count);

  if (SetFilePointer((HANDLE) devno, (blkno + part_offset) * SECTORSIZE, NULL, FILE_BEGIN) == -1) panic("unable to set file pointer");
  if (!ReadFile((HANDLE) devno, buffer, count, &bytes, NULL)) panic("error reading from device");
  return count;
}

int dev_write(devno_t devno, void *buffer, size_t count, blkno_t blkno)
{
  DWORD bytes;

  //printf("write block %d, %d bytes\n", blkno, count);

  if (SetFilePointer((HANDLE) devno, (blkno + part_offset) * SECTORSIZE, NULL, FILE_BEGIN) == -1) panic("unable to set file pointer");
  if (!WriteFile((HANDLE) devno, buffer, count, &bytes, NULL)) panic("error writing to device");
  return count;
}

unsigned int dev_getsize(devno_t devno)
{
  unsigned int size;

  //size = GetFileSize((HANDLE) devno, NULL) / SECTORSIZE;
  size = devsize;
  printf("device size is %d sectors (%dMB)\n", size, size / 2048);
  return size;
}

void create_device(char *devname, int devsize)
{
  DWORD bytes;
  HANDLE hdev;
  char sector[SECTORSIZE];
  int i;

  hdev = CreateFile(devname, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
  if (hdev == INVALID_HANDLE_VALUE) panic("unable to create device file");

  memset(sector, 0, SECTORSIZE);
  for (i = 0; i < devsize; i++)
  {
    if (!WriteFile(hdev, sector, SECTORSIZE, &bytes, NULL)) panic("error writing to device");
  }

  CloseHandle(hdev);
}

void clear_device(HANDLE hdev, int devsize)
{
  DWORD bytes;
  char sector[SECTORSIZE];
  int i;

  memset(sector, 0, SECTORSIZE);
  for (i = 0; i < devsize; i++)
  {
    if (!WriteFile(hdev, sector, SECTORSIZE, &bytes, NULL)) panic("error writing to device");
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

void read_mbr(HANDLE hdev)
{
  struct master_boot_record mbr;
  DWORD bytes;

  if (SetFilePointer(hdev, 0, NULL, FILE_BEGIN) == -1) panic("unable to set file pointer");
  if (!ReadFile(hdev, &mbr, sizeof mbr, &bytes, NULL)) panic("error reading mbr from device");
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

  return mktime(&tm);
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

  mkdir("/bin", 0755);
  file = open("/bin/krnl.dll", O_SPECIAL | (DFS_INODE_KERNEL << 24) | O_WRONLY, 0644);
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

void copy_file(char *srcfn, char *dstfn)
{
  size_t count;
  char buf[4096];
  struct file *f1;
  struct file *f2;

  f1 = open(srcfn, 0, 0);
  if (!f1)
  {
    printf("%s: file not found\n", srcfn);
    return;
  }

  f2 = open(dstfn, O_CREAT, 0);
  if (!f2)
  {
    close(f1);
    printf("%s: unable to create file\n", dstfn);
    return;
  }

  while ((count = read(f1, buf, 4096)) > 0)
  {
    write(f2, buf, count);
  }

  close(f1);
  close(f2);
}

void list_dir(char *dirname)
{
  struct file *dir;
  struct dirent dirp;
  struct stat buf;
  struct tm *tm;
  char path[MAXPATH];

  dir = opendir(dirname);
  if (!dir)
  {
    printf("%s: directory not found\n", dirname);
    return;
  }

  while (readdir(dir, &dirp, 1) > 0)
  {
    strcpy(path, dirname);
    strcat(path, "/");
    strcat(path, dirp.name);

    stat(path, &buf);
    tm = gmtime(&buf.ctime);

    printf("%8d %8d %02d/%02d/%04d %02d:%02d:%02d ", buf.ino, buf.quad.size_low, tm->tm_mday, tm->tm_mon, tm->tm_year + 1900, tm->tm_hour, tm->tm_min, tm->tm_sec);
    if (buf.mode & FS_DIRECTORY) 
      printf("[%s]", dirp.name);
    else
      printf("%s", dirp.name);

    printf("\n");
  }

  close(dir);
}

void remove_file(char *filename)
{
  if (xunlink(filename) < 0)
  {
    printf("%s: file not found\n", filename);
    return;
  }
}

void make_dir(char *filename)
{
  if (mkdir(filename, 0755) < 0)
  {
    printf("%s: cannot make directory\n", filename);
    return;
  }
}

void remove_dir(char *filename)
{
  if (rmdir(filename) < 0)
  {
    printf("%s: cannot delete directory\n", filename);
    return;
  }
}

void disk_usage(char *filename)
{
  struct fs *fs;
  struct filsys *filsys;

  fs = fslookup(filename, NULL);
  if (!fs)
  {
    printf("%s: unknown file system\n", filename);
  }

  filsys = (struct filsys *) fs->data;
  printf("block size %d\n", filsys->blocksize);
  printf("blocks %d\n", filsys->super->block_count);
  printf("inodes %d\n", filsys->super->inode_count);
  printf("free blocks %d\n", filsys->super->free_block_count);
  printf("free inodes %d\n", filsys->super->free_inode_count);

  printf("%dKB used, %dKB free, %dKB total\n", 
    filsys->blocksize * (filsys->super->block_count - filsys->super->free_block_count) / 1024, 
    filsys->blocksize * filsys->super->free_block_count / 1024, 
    filsys->blocksize * filsys->super->block_count / 1024);
}

static void test_read_file(char *filename)
{
  int count;
  struct file *file;
  char *data;
  int start;
  int time;
  int bytes;

  file = open(filename, 0, 0);
  if (!file)
  {
    printf("%s: file not found\n", filename);
    return;
  }

  data = malloc(64 * K);

  bytes = 0;
  start = get_tick_count();
  while ((count = read(file, data, 64 * K)) > 0)
  {
    bytes += count;
  }
  time = (get_tick_count() - start) * 10;
  printf("%s: read %dKB in %d ms, %dKB/s\n", filename, bytes / K, time, bytes / time);
  
  free(data);

  if (count < 0) printf("%s: error reading file\n", filename);

  close(file);
}

static void test_write_file(char *filename, int size)
{
  int count;
  struct file *file;
  char *data;
  int start;
  int time;
  int bytes;

  file = open(filename, O_CREAT, 0644);
  if (!file)
  {
    printf("%s: error creating file\n", filename);
    return;
  }

  data = malloc(64 * K);

  bytes = 0;
  start = get_tick_count();
  while (bytes < size)
  {
    if ((count = write(file, data, 64 * K)) <= 0)
    {
      printf("%s: error writing file\n", filename);
      break;
    }

    bytes += count;
  }
  time = (get_tick_count() - start) * 10;
  printf("%s: wrote %dKB in %d ms, %dKB/s\n", filename, bytes / K, time, bytes / time);
  
  free(data);

  close(file);
}

void shell()
{
  char cmd[256];
  char *arg;
  char *arg2;

  while (1)
  {
    printf("> ");
    gets(cmd);

    arg = cmd;
    while (*arg && *arg != ' ') arg++;
    while (*arg == ' ') *arg++ = 0;

    arg2 = arg;
    while (*arg2 && *arg2 != ' ') arg2++;
    while (*arg2 == ' ') *arg2++ = 0;

    if (strcmp(cmd, "exit") == 0) 
      break;
    else if (strcmp(cmd, "ls") == 0)
      list_dir(arg);
    else if (strcmp(cmd, "cat") == 0)
      list_file(arg);
    else if (strcmp(cmd, "cp") == 0)
      copy_file(arg, arg2);
    else if (strcmp(cmd, "rm") == 0)
      remove_file(arg);
    else if (strcmp(cmd, "mkdir") == 0)
      make_dir(arg);
    else if (strcmp(cmd, "rmdir") == 0)
      remove_dir(arg);
    else if (strcmp(cmd, "du") == 0)
      disk_usage(arg);
    else if (strcmp(cmd, "read") == 0)
      test_read_file(arg);
    else if (strcmp(cmd, "write") == 0)
      test_write_file(arg, atoi(arg2) * K);
    else if (*cmd)
      printf("%s: unknown command\n", cmd);
  }
}

void usage()
{
  fprintf(stderr, "usage: mkdfs [options]\n\n");
  fprintf(stderr, "  -a (use alternative filenames from file list)\n");
  fprintf(stderr, "  -d <devname>\n");
  fprintf(stderr, "  -b <bootfile>\n");
  fprintf(stderr, "  -c <device capacity> (capacity in kilobytes)\n");
  fprintf(stderr, "  -i (initialize device)\n");
  fprintf(stderr, "  -f (format device)\n");
  fprintf(stderr, "  -k <kernel image>\n");
  fprintf(stderr, "  -l <loader image>\n");
  fprintf(stderr, "  -s (shell)\n");
  fprintf(stderr, "  -w (wipe out device, i.e. zero all sectors first)\n");
  fprintf(stderr, "  -p <partition>\n");
  fprintf(stderr, "  -P <partition start sector>\n");
  fprintf(stderr, "  -q (quick format device)\n");
  fprintf(stderr, "  -B <block size> (default 4096)\n");
  fprintf(stderr, "  -C <capacity> (capacity in kilobytes used for creating new device file)\n");
  fprintf(stderr, "  -F <file list file>\n");
  fprintf(stderr, "  -I <inode ratio> (default 1 inode per 4K)\n");
  fprintf(stderr, "  -K <kernel options>\n");
  fprintf(stderr, "  -S <source directory or file>\n");
  fprintf(stderr, "  -T <target directory or file>\n");
}

int main(int argc, char **argv)
{
  HANDLE hdev;
  int c;

  // Parse command line options
  while ((c = getopt(argc, argv, "ad:b:c:ifk:l:swp:qB:C:F:I:K:P:S:T:?")) != EOF)
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
	devsize = devcap = atoi(optarg) * 2;
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

      case 's':
	doshell = !doshell;
	break;

      case 'w':
	dowipe = !dowipe;
	break;

      case 'q':
	quick = !quick;
	break;

      case 'B':
	blocksize = atoi(optarg);
	break;

      case 'C':
	devcap = atoi(optarg) * 2;
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
    printf("Creating device file %s %dKB\n", devname, devcap / 2);
    create_device(devname, devcap);
  }

  // Open device file
  hdev = CreateFile(devname, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_WRITE_THROUGH | FILE_FLAG_NO_BUFFERING, NULL);
  if (hdev == INVALID_HANDLE_VALUE) 
  {
    printf("Error %d opening device %s\n", GetLastError(), devname);
    panic("unable to open device");
  }

  // Read master boot record
  if (part != -1)
  {
    read_mbr(hdev);
  }

  // Clear device
  if (dowipe) 
  {
    printf("Clearing device %s\n", devname);
    clear_device(hdev, devsize);
  }

  // Create filesystem
  if (doformat)
  {
    char options[256];

    sprintf(options, "blocksize=%d,inoderatio=%d%s", blocksize, inoderatio, quick ? ",quick" : "");
    printf("Formating device (%s)...\n", options);
    if (format((devno_t) hdev, "dfs", options) < 0) panic("error formatting device");
  }

  // Initialize the file system
  printf("Mounting device\n");
  if (mount("dfs", "/", (devno_t) hdev, NULL) < 0) panic("error mounting device");

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
    dev_write((devno_t) hdev, bootsect, SECTORSIZE, 0);
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

  // Execute shell
  if (doshell)
  {
    printf("Executing shell\n");
    shell();
  }

  // Close file system
  printf("Unmounting device\n");
  unmount_all();

  // Close device file
  printf("Closing device\n");
  CloseHandle(hdev);

  return 0;
}
