//
// sh.c
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Shell
//

#include <os.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

unsigned char buffer[4096];

unsigned char orig[4096];
unsigned char pattern[4096];

static void list_file(char *filename)
{
  int count;
  int file;

  if ((file = open(filename, 0)) < 0)
  {
    printf("%s: file not found\n", filename);
    return;
  }

  while ((count = read(file, buffer, 4096)) > 0)
  {
    write(stdout, buffer, count);
  }

  close(file);
}

static void copy_file(char *srcfn, char *dstfn)
{
  char *data;
  int count;
  int f1;
  int f2;

  if ((f1 = open(srcfn, 0)) < 0)
  {
    printf("%s: file not found\n", srcfn);
    return;
  }

  if ((f2 = open(dstfn, O_CREAT)) < 0)
  {
    close(f1);
    printf("%s: unable to create file\n", dstfn);
    return;
  }

  data = malloc(64 * K);
  while ((count = read(f1, data, 64 * K)) > 0)
  {
    if (write(f2, data, count) != count)
    {
      printf("%s: error writing data\n", dstfn);
      break;
    }
  }
  free(data);

  if (count < 0) printf("%s: error reading data\n", srcfn);
  
  close(f1);
  close(f2);
}

static void list_dir(int argc, char **argv)
{
  char *dirname;
  int verbose;
  int dir;
  struct dirent dirp;
  struct stat buf;
  struct tm tm;
  char path[MAXPATH];

  verbose = 0;
  if (argc == 1)
    dirname = "/";
  else if (argc == 2)
  {
    if (strcmp(argv[1], "-l") == 0)
    {
      verbose = 1;
      dirname = "/";
    }
    else
      dirname = argv[1];
  }
  else if (argc == 3)
  {
    if (strcmp(argv[1], "-l") != 0)
    {
      printf("usage: ls [-l] <dir>\n");
      return;
    }
    verbose = 1;
    dirname = argv[2];
  }
  else
  {
    printf("usage: ls [-l] <dir>\n");
    return;
  }

  if ((dir = opendir(dirname)) < 0)
  {
    printf("%s: directory not found\n", dirname);
    return;
  }

  while (readdir(dir, &dirp, 1) > 0)
  {
    strcpy(path, dirname);
    strcat(path, "/");
    strcat(path, dirp.name);

    if (stat(path, &buf) < 0) memset(&buf, 0, sizeof(struct stat));

    if (verbose)
    {
      printf("%8d %4d %1d %2d ", buf.quad.size_low, buf.ino, buf.nlink, buf.devno);

      gmtime(&buf.ctime, &tm);
      printf("%02d/%02d/%04d %02d:%02d:%02d ", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, tm.tm_hour, tm.tm_min, tm.tm_sec);
    }
    else
    {
      if (buf.mode & FS_DIRECTORY)
        printf("         ");
      else
      {
	int size;

	if (buf.quad.size_low >= K)
	  size = buf.quad.size_low / K;
	else if (buf.quad.size_low > 0)
	  size = 1;
	else
	  size = 0;

        printf("%6dKB ", size);
      }
    }

    gmtime(&buf.mtime, &tm);
    printf("%02d/%02d/%04d %02d:%02d:%02d ", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, tm.tm_hour, tm.tm_min, tm.tm_sec);

    printf("%s", dirp.name);
    if (buf.mode & FS_DIRECTORY) printf("/", dirp.name);

    printf("\n");
  }

  close(dir);
}

static void remove_file(char *filename)
{
  if (unlink(filename) < 0)
  {
    printf("%s: file not found\n", filename);
    return;
  }
}

static void move_file(char *oldname, char *newname)
{
  if (rename(oldname, newname) < 0)
  {
    printf("%s: unable to rename file to %s\n", oldname, newname);
    return;
  }
}

static void make_dir(char *filename)
{
  if (mkdir(filename) < 0)
  {
    printf("%s: cannot make directory\n", filename);
    return;
  }
}

static void remove_dir(char *filename)
{
  if (rmdir(filename) < 0)
  {
    printf("%s: cannot delete directory\n", filename);
    return;
  }
}

static void display_file(char *filename)
{
  int count;
  int file;
  int line;
  char *start;
  char *end;
  char ch;

  if ((file = open(filename, 0)) < 0)
  {
    printf("%s: file not found\n", filename);
    return;
  }

  line = 0;
  while ((count = read(file, buffer, 4096)) > 0)
  {
    start = end = buffer;
    while (end < buffer + count)
    {
      if (line == 24)
      {
	write(stdout, start, end - start);
	while (read(stdin, &ch, 1) <= 0);
	start = end;
        line = 0;
      }
      if (*end++ == '\n') line++;
    }
    write(stdout, start, end - start);
  }

  close(file);
}

static void show_date()
{
  time_t t;
  struct tm tm;

  t = time();
  gmtime(&t, &tm);
  printf("Time is %04d/%02d/%02d %02d:%02d:%02d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
}

static void test_read_file(char *filename)
{
  int count;
  int file;
  char *data;
  clock_t start;
  clock_t time;
  int bytes;

  if ((file = open(filename, 0)) < 0)
  {
    printf("%s: file not found\n", filename);
    return;
  }

  data = malloc(64 * K);

  bytes = 0;
  start = clock();
  while ((count = read(file, data, 64 * K)) > 0)
  {
    bytes += count;
  }
  time = clock() - start;
  if (time == 0) time = 1;

  printf("%s: read %dKB in %d ms, %dKB/s\n", filename, bytes / K, time, bytes / time);
  
  free(data);

  if (count < 0) printf("%s: error reading file\n", filename);

  close(file);
}

static void test_write_file(char *filename, int size)
{
  int count;
  int file;
  char *data;
  clock_t start;
  clock_t time;
  int bytes;

  if ((file = open(filename, O_CREAT)) < 0)
  {
    printf("%s: error creating file\n", filename);
    return;
  }

  data = malloc(64 * K);

  bytes = 0;
  start = clock();
  while (bytes < size)
  {
    if ((count = write(file, data, 64 * K)) <= 0)
    {
      printf("%s: error writing file\n", filename);
      break;
    }

    bytes += count;
  }
  time = clock() - start;
  if (time == 0) time = 1;

  printf("%s: wrote %dKB in %d ms, %dKB/s\n", filename, bytes / K, time, bytes / time);
  
  free(data);

  close(file);
}

static void set_loglevel(char *arg, int level)
{
  int ll = 0;

  while (*arg)
  {
    if (*arg == 'm') ll |= LOG_MODULE;
    if (*arg == 'h') ll |= LOG_HEAP;
    if (*arg == 't') ll |= LOG_APITRACE;
    if (*arg == 'a') ll |= LOG_AUX;
    if (*arg == 'x') ll |= LOG_SUBSYS_MASK;

    arg++;
  }

  loglevel = ll | level;
}

static void idle_sleep(int ms)
{
  time_t t;
  struct tm tm;
  clock_t c;

  t = time();
  c = clock();
  gmtime(&t, &tm);
  printf("Time is %04d/%02d/%02d %02d:%02d:%02d Clock is %d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, c);

  printf("Sleep %d ms\n", ms);
  sleep(ms);

  t = time();
  c = clock();
  gmtime(&t, &tm);
  printf("Time is %04d/%02d/%02d %02d:%02d:%02d Clock is %d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, c);
}

static void start_program(int argc, char **argv)
{
  char *pgm;
  char *args = "";
  hmodule_t hmod;
  int rc;

  if (argc < 2)
  {
    printf("usage: start <pgm> [<args>]\n");
    return;
  }
  pgm = argv[1];

  if (argc > 2) args = argv[2];

  hmod = load(pgm);
  if (hmod == NULL)
  {
    printf("%s: unable to load module\n", pgm);
    return;
  }

  rc = exec(hmod, args);

  if (rc != 0) printf("Exitcode: %d\n", rc);
  //unload(hmod);
}

static void mount_device(int argc, char **argv)
{
  char *devname;
  char *path;
  char *type = "dfs";
  char *opts = NULL;
  int rc;

  if (argc < 3)
  {
    printf("usage: mount <device> <path> [<type> [<options>]]\n");
    return;
  }

  devname = argv[1];
  path = argv[2];
  if (argc > 3) type = argv[3];
  if (argc > 4) opts = argv[4];

  rc = mount(type, path, devname, opts); 
  if (rc < 0)
  {
    printf("mount: error %d mounting %s\n", rc, devname);
    return;
  }
}

static void unmount_device(int argc, char **argv)
{
  char *path;
  int rc;

  if (argc < 2)
  {
    printf("usage: unmount <path>\n");
    return;
  }

  path = argv[1];

  rc = unmount(path);
  if (rc < 0)
  {
    printf("unmount: error %d mounting %s\n", rc, path);
    return;
  }
}

static void disk_usage(int argc, char **argv)
{
  int count;
  int rc;
  int n;
  struct statfs *buf;
  struct statfs *b;

  count = getfsstat(NULL, 0);
  if (count < 0)
  {
    printf("du: error %d in getfsstat\n", count);
    return;
  }

  buf = (struct statfs *) malloc(count * sizeof(struct statfs));
  rc = getfsstat(buf, count * sizeof(struct statfs));
  if (rc < 0)
  {
    printf("du: error %d in getfsstat\n", rc);
    return;
  }

  printf("type  mounted on    mounted from    cache    total     used    avail files\n");
  printf("----  ------------- -------------- ------ -------- -------- -------- -----\n");

  for (n = 0; n < count; n++)
  {
    b = buf + n;

    printf("%-6s%-14s%-14s", b->fstype, b->mntonname, b->mntfromname);
    if (b->blocks != -1)
    {
      printf("%6dK", b->cachesize / K);
      printf("%8dK", b->blocks * (b->bsize / 512) / 2);
      printf("%8dK", (b->blocks - b->bfree) * (b->bsize / 512) / 2);
      printf("%8dK", b->bfree * (b->bsize / 512) / 2);

      printf(" %d/%d", b->files - b->ffree, b->files);
    }

    printf("\n");
  }

  free(buf);
}

static void lookup(char *name)
{
  struct hostent *hp;
  int i;

  if (!name || !*name) return;

  if (*name >= '0' && *name <= '9')
  {
    struct in_addr addr;
    addr.s_addr = inet_addr(name);
    if (addr.s_addr == INADDR_NONE)
    {
      printf("%s: invalid address\n", name);
      return;
    }

    hp = gethostbyaddr((char *) &addr, sizeof(struct in_addr), AF_INET);
  }
  else
    hp = gethostbyname(name);

  if (!hp)
  {
    printf("%s: error %d resolving name\n", name, errno);
    return;
  }

  if (hp->h_addrtype != AF_INET)
  {
    printf("unknown address type %d\n", hp->h_addrtype);
    return;
  }

  if (hp->h_length != sizeof(struct in_addr))
  {
    printf("unknown address length %d\n", hp->h_length);
    return;
  }

  printf("name: %s\n", hp->h_name);
  for (i = 0; hp->h_aliases[i] != NULL; i++) printf("alias: %s\n", hp->h_aliases[i]);
  for (i = 0; hp->h_addr_list[i] != NULL; i++) 
  {
    struct in_addr *addr = (struct in_addr *) (hp->h_addr_list[i]);
    printf("address: %s\n", inet_ntoa(*addr));
  }
}

static void http(int argc, char **argv)
{
  char *server;
  char *path;
  struct hostent *hp;
  struct sockaddr_in sin;
  int s;
  int rc;
  int n;
  char buf[256];

  server = "192.168.12.1";
  path = "/";
  if (argc >= 2) server = argv[1];
  if (argc >= 3) path = argv[2];

  hp = gethostbyname(server);
  if (!hp)
  {
    printf("%s: host not found\n", server);
    return;
  }

  printf("address: %s\n", inet_ntoa(*(struct in_addr *) hp->h_addr_list[0]));

  s = socket(AF_INET, SOCK_STREAM, 0);
  if (s < 0)
  {
    printf("socket: error %d\n", s);
    return;
  }

  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  memcpy(&sin.sin_addr, hp->h_addr_list[0], hp->h_length);
  sin.sin_port = htons(80);

  printf("connect to %s port %d\n", inet_ntoa(sin.sin_addr), ntohs(sin.sin_port));
  rc = connect(s, (struct sockaddr *) &sin, sizeof(sin));
  if (rc  < 0)
  {
    printf("connect: error %d\n", rc);
  }

  printf("connected\n");

  sprintf(buf, "GET %s HTTP/1.1\r\n\r\n", path);
  rc = send(s, buf, strlen(buf), 0);
  if (rc < 0)
  {
    printf("send: error %d\n", rc);
    return;
  }

  printf("receive data\n");

  while ((n = recv(s, buf, 128, 0)) > 0)
  {
    printf("reveived %d bytes\n", n);
  }

  if (rc < 0)
  {
    printf("recv: error %d\n", n);
  }
  sleep(10000);

  printf("closing\n");
  close(s);
}

static void test(int argc, char **argv)
{
  int dev;
  int n;
  int rc;

  dev = open(argv[1], O_RDWR | O_DIRECT);
  if (dev < 0) 
  {
    printf("Error %d opening device\n", dev);
    return;
  }

  for (n = 1; n < atoi(argv[2]); n++)
  {
    printf("checking block %d\n", n);
    
    rc = lseek(dev, n * 4096, SEEK_SET);
    if (rc < 0) printf("lseek returned %d\n", rc);

    rc = read(dev, orig, 4096);
    if (rc < 0) printf("read returned %d\n", rc);

    rc = lseek(dev, n * 4096, SEEK_SET);
    if (rc < 0) printf("lseek returned %d\n", rc);

    memset(pattern, n & 0xFF, 4096);
    rc = write(dev, pattern, 4096);
    if (rc < 0) printf("write returned %d\n", rc);

    rc = lseek(dev, n * 4096, SEEK_SET);
    if (rc < 0) printf("lseek returned %d\n", rc);

    memset(buffer, 0, 4096);
    rc = read(dev, buffer, 4096);
    if (rc < 0) printf("read returned %d\n", rc);

    if (memcmp(pattern, buffer, 4096) != 0) printf("error in test pattern\n");

    rc = lseek(dev, n * 4096, SEEK_SET);
    if (rc < 0) printf("lseek returned %d\n", rc);

    rc = write(dev, orig, 4096);
    if (rc < 0) printf("write returned %d\n", rc);
  
    rc = lseek(dev, n * 4096, SEEK_SET);
    if (rc < 0) printf("lseek returned %d\n", rc);

    memset(buffer, 0, 4096);
    rc = read(dev, buffer, 4096);
    if (rc < 0) printf("read returned %d\n", rc);

    if (memcmp(orig, buffer, 4096) != 0) printf("error in verify\n");
  }

  close(dev);
}

__inline long __declspec(naked) rdtscl()
{
  __asm { rdtsc }
  __asm { ret }
}

static void nop()
{
  long before;
  long after;
  long cycles;
  int i;
  long min;
  long max;
  long sum;

  if (!peb->fast_syscalls_supported)
  {
    printf("error: processor does not support sysenter/sysexit\n");
    return;
  }

  peb->fast_syscalls_supported = 0;
  min = 0x7FFFFFFF;
  max = 0;
  sum = 0;

  for (i = 0; i < 1000; i++)
  {
    before = rdtscl();
    syscall(0,0);
    after = rdtscl();

    cycles = after - before;
    if (cycles < min) min = cycles;
    if (cycles > max) max = cycles;
    sum += cycles;
  }

  printf("syscall(0,0) with int 48: min/avg/max %d/%d/%d cycles\n", min, sum / 1000, max);

  peb->fast_syscalls_supported = 1;
  min = 0x7FFFFFFF;
  max = 0;
  sum = 0;

  for (i = 0; i < 1000; i++)
  {
    before = rdtscl();
    syscall(0,0);
    after = rdtscl();

    cycles = after - before;
    if (cycles < min) min = cycles;
    if (cycles > max) max = cycles;
    sum += cycles;
  }

  printf("syscall(0,0) with sysenter: min/avg/max %d/%d/%d cycles\n", min, sum / 1000, max);
}

void shell()
{
  char cmd[256];
  int argc;
  char **argv;

  while (1)
  {
    printf("$ ");
    gets(cmd);

    argc = parse_args(cmd, NULL);
    if (argc)
    {
      argv = (char **) malloc(argc * sizeof(char *));
      parse_args(cmd, argv);
    
      if (strcmp(argv[0], "exit") == 0) 
	break;
      else if (strcmp(argv[0], "ls") == 0 || strcmp(argv[0], "dir") == 0)
	list_dir(argc, argv);
      else if (strcmp(argv[0], "cat") == 0 || strcmp(argv[0], "type") == 0)
	list_file(argv[1]);
      else if (strcmp(argv[0], "cp") == 0 || strcmp(argv[0], "copy") == 0)
	copy_file(argv[1], argv[2]);
      else if (strcmp(argv[0], "rm") == 0 || strcmp(argv[0], "del") == 0)
	remove_file(argv[1]);
      else if (strcmp(argv[0], "mv") == 0 || strcmp(argv[0], "move") == 0)
	move_file(argv[1], argv[2]);
      else if (strcmp(argv[0], "mkdir") == 0 || strcmp(argv[0], "md") == 0)
	make_dir(argv[1]);
      else if (strcmp(argv[0], "rmdir") == 0 || strcmp(argv[0], "rd") == 0)
	remove_dir(argv[1]);
      else if (strcmp(argv[0], "mount") == 0)
	mount_device(argc, argv);
      else if (strcmp(argv[0], "unmount") == 0)
	unmount_device(argc, argv);
      else if (strcmp(argv[0], "du") == 0)
	disk_usage(argc, argv);
      else if (strcmp(argv[0], "date") == 0)
	show_date();
      else if (strcmp(argv[0], "read") == 0)
	test_read_file(argv[1]);
      else if (strcmp(argv[0], "more") == 0)
	display_file(argv[1]);
      else if (strcmp(argv[0], "write") == 0)
	test_write_file(argv[1], atoi(argv[2]) * K);
      else if (strcmp(argv[0], "cls") == 0)
	printf("\f");
      else if (strcmp(argv[0], "log") == 0)
	set_loglevel(argv[1], atoi(argv[2]));
      else if (strcmp(argv[0], "start") == 0)
	start_program(argc, argv);
      else if (strcmp(argv[0], "break") == 0)
	dbgbreak();
      else if (strcmp(argv[0], "nop") == 0)
	nop();
      else if (strcmp(argv[0], "sleep") == 0)
	idle_sleep(atoi(argv[1]));
      else if (strcmp(argv[0], "lookup") == 0 || strcmp(argv[0], "nslookup") == 0)
	lookup(argv[1]);
      else if (strcmp(argv[0], "http") == 0)
	http(argc, argv);
      else if (strcmp(argv[0], "test") == 0)
	test(argc, argv);
      else
	printf("%s: unknown command\n", argv[0]);

      free_args(argc, argv);
    }
  }
}

void __stdcall shelltask(void *arg)
{
  char *devname = (char *) arg;
  handle_t f;

  f = open(devname, O_RDWR);
  if (f < 0) 
  {
    syslog(LOG_INFO, "Error %d starting shell on device %s\n", f, devname);
    endthread(1);
  }

  syslog(LOG_INFO, "sh: starting shell on device %s\n", devname);

  gettib()->in = f;
  gettib()->out = f;
  gettib()->err = f;

  shell();

  close(f);
}

int __stdcall main(hmodule_t hmod, char *cmdline, void *env)
{
  //beginthread(shelltask, 0, "/dev/com1", 0, NULL);
  shell();
  return 0;
}
