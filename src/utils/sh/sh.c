//
// sh.c
//
// Shell
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

#include <os.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <limits.h>
#include <os/version.h>
#include <crtbase.h>

#include <inifile.h>
#include <httpd.h>

unsigned char buffer[4096];

unsigned char orig[4096];
unsigned char pattern[4096];

static void list_file(char *filename)
{
  int count;
  int file;

  if ((file = open(filename, O_BINARY)) < 0)
  {
    printf("%s: %s\n", filename, strerror(file));
    return;
  }

  while ((count = read(file, buffer, 4096)) > 0)
  {
    write(fdout, buffer, count);
  }

  close(file);
}

static void copy_file(char *srcfn, char *dstfn)
{
  char *data;
  int count;
  int f1;
  int f2;

  if ((f1 = open(srcfn, O_BINARY)) < 0)
  {
    printf("%s: %s\n", srcfn, strerror(f1));
    return;
  }

  if ((f2 = open(dstfn, O_CREAT | O_BINARY, S_IREAD | S_IWRITE)) < 0)
  {
    close(f1);
    printf("%s: %s\n", dstfn, strerror(f2));
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

  if (count < 0) printf("%s: %s\n", srcfn, strerror(count));
  
  close(f1);
  close(f2);
}

static void list_dir(int argc, char **argv)
{
  char *dirname;
  int verbose;
  int i;
  int dir;
  struct dirent dirp;
  struct stat64 buf;
  struct tm tm;
  char path[MAXPATH];
  char *arg;
  int col;

  verbose = 0;
  dirname = ".";
  for (i = 1; i < argc; i++)
  {
    arg = argv[i];

    if (*arg == '-')
    {
      while (*++arg)
      {
	if (*arg == 'l') verbose = 1;
	if (*arg == 'w') verbose = 2;
      }
    }
    else
      dirname = arg;
  }

  if ((dir = opendir(dirname)) < 0)
  {
    printf("%s: %s\n", dirname, strerror(dir));
    return;
  }

  col = 0;
  while (readdir(dir, &dirp, 1) > 0)
  {
    strcpy(path, dirname);
    strcat(path, "/");
    strcat(path, dirp.name);

    if (stat64(path, &buf) < 0) memset(&buf, 0, sizeof(struct stat64));

    if (verbose == 2)
    {
      if (col == 4)
      {
	col = 0;
	//printf("\n");
      }

      if ((buf.st_mode & S_IFMT) == S_IFDIR) strcat(dirp.name, "/");
      printf("%-20s", dirp.name);
      col++;
    }
    else
    {
      strcpy(path, dirname);
      strcat(path, "/");
      strcat(path, dirp.name);

      if (stat64(path, &buf) < 0) memset(&buf, 0, sizeof(struct stat64));

      if (verbose)
      {
	printf("%8d %4d %1d %2d ", (int) buf.st_size, buf.st_ino, buf.st_nlink, buf.st_dev);

	_gmtime(&buf.st_ctime, &tm);
	printf("%02d/%02d/%04d %02d:%02d:%02d ", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, tm.tm_hour, tm.tm_min, tm.tm_sec);
      }
      else
      {
	if ((buf.st_mode & S_IFMT) == S_IFDIR)
	  printf("         ");
	else
	{
	  if (buf.st_size < 1*K)
	    printf("%6dB  ", (int) buf.st_size);
	  else if (buf.st_size < 1*M)
	    printf("%6dKB ", (int) (buf.st_size / K));
	  else if (buf.st_size < 1073741824i64)
	    printf("%6dMB ", (int) (buf.st_size / M));
	  else
	    printf("%6dGB ", (int) (buf.st_size / 1073741824i64));
	}
      }

      _gmtime(&buf.st_mtime, &tm);
      printf("%02d/%02d/%04d %02d:%02d:%02d ", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, tm.tm_hour, tm.tm_min, tm.tm_sec);

      printf("%s", dirp.name);
      if ((buf.st_mode & S_IFMT) == S_IFDIR) printf("/", dirp.name);

      printf("\n");
    }
  }

  if (verbose == 2) printf("\n");

  close(dir);
}

static void remove_file(char *filename)
{
  int rc;

  rc = unlink(filename); 
  if (rc < 0)
  {
    printf("%s: %s\n", filename, strerror(rc));
    return;
  }
}

static void move_file(char *oldname, char *newname)
{
  int rc;

  rc = rename(oldname, newname);
  if (rc < 0)
  {
    printf("%s: %s\n", oldname, strerror(rc));
    return;
  }
}

static void change_dir(char *filename)
{
  int rc;

  rc = chdir(filename); 
  if (rc < 0)
  {
    printf("%s: %s\n", filename, strerror(rc));
    return;
  }
}

static void make_dir(char *filename)
{
  int rc;

  rc = mkdir(filename); 
  if (rc < 0)
  {
    printf("%s: %s\n", filename, strerror(rc));
    return;
  }
}

static void remove_dir(char *filename)
{
  int rc;

  rc = rmdir(filename); 
  if (rc < 0)
  {
    printf("%s: %s\n", filename, strerror(rc));
    return;
  }
}

static void display_buffer(int f, char *buffer, int size)
{
  char *p;
  char *q;
  char *end;

  p = buffer;
  end = buffer + size;
  while (p < end)
  {
    q = p;
    while (q < end && *q != '\n') q++;

    if (p != q) write(f, p, q - p);
    if (q < end) 
    {
      write(f, "\r\n", 2);
      q++;
    }
    p = q;
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

  if ((file = open(filename, O_BINARY)) < 0)
  {
    printf("%s: %s\n", filename, strerror(file));
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
	display_buffer(fdout, start, end - start);
	while (read(fdin, &ch, 1) <= 0);
	if (ch == 'x')
	{
	  close(file);
	  return;
	}
	start = end;
        line = 0;
      }
      if (*end++ == '\n') line++;
    }
    display_buffer(fdout, start, end - start);
  }

  close(file);
}

static void show_date(int argc, char *argv[])
{
  time_t t;
  struct tm tm;

  t = time(NULL);
  _gmtime(&t, &tm);

  if (argc > 1)
  {
    char buffer[256];

    strftime(buffer, 256, argv[1], &tm);
    printf("%s\n", buffer);
  }
  else
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
    printf("%s: %s\n", filename, strerror(file));
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

  if (count < 0) printf("%s: %s\n", filename, strerror(count));

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

  if ((file = open(filename, O_CREAT | O_BINARY, S_IREAD | S_IWRITE)) < 0)
  {
    printf("%s: %s\n", filename, strerror(file));
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

struct critsect testcs;
int count;
int runs;
long holdrand = 1L;

static int rand()
{
  return (((holdrand = holdrand * 214013L + 2531011L) >> 16) & 0x7fff);
}

static void __stdcall csthreadproc(void *arg)
{
  int n;

  while (1)
  {
    enter(&testcs);

    runs++;

    n = count;

    if (n != 0) printf("ERROR: count is %d\n", n);

    count++;
    if (rand() < 100) sleep(rand() / 1000);
    count--;

    leave(&testcs);
    //sleep(rand() / 10000);
  }
}

static void cstest()
{
  int n;

  mkcs(&testcs);

  runs = 0;
  for (n = 0; n < 10; n++)
  {
    beginthread(csthreadproc, 0, NULL, 0, NULL);
  }

  while (1)
  {
    sleep(1000);
    printf("%d runs\r", runs);
  }

  csfree(&testcs);
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

  t = time(NULL);
  c = clock();
  _gmtime(&t, &tm);
  printf("Time is %04d/%02d/%02d %02d:%02d:%02d Clock is %d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, c);

  printf("Sleep %d ms\n", ms);
  sleep(ms);

  t = time(NULL);
  c = clock();
  _gmtime(&t, &tm);
  printf("Time is %04d/%02d/%02d %02d:%02d:%02d Clock is %d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, c);
}

static void busy_loop(int ms)
{
  time_t t;
  struct tm tm;
  clock_t c;

  t = time(NULL);
  c = clock();
  _gmtime(&t, &tm);
  printf("Time is %04d/%02d/%02d %02d:%02d:%02d Clock is %d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, c);

  //while (clock() < c + ms);
  {
    int n;
    for (n = 0; n < 100000000; n++) c += n;
  }

  t = time(NULL);
  c = clock();
  _gmtime(&t, &tm);
  printf("Time is %04d/%02d/%02d %02d:%02d:%02d Clock is %d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, c);
}

static void exec_program(char *args)
{
  char pgm[MAXPATH];
  char *p;
  char *q;
  int rc;
  int dotseen = 0;

  p = args;
  q = pgm;
  while (*p != 0 && *p != ' ')
  {
    if (*p == '.') dotseen = 1;
    if (*p == PS1 || *p == PS2) dotseen = 0;
    *q++ = *p++;
  }
  *q++ = 0;
  if (!dotseen) strcat(pgm, ".exe");

  rc = spawn(P_WAIT, pgm, args, NULL);
  if (rc < 0)
    printf("%s: %s\n", pgm, strerror(rc));
  else if (rc > 0)
    printf("Exitcode: %d\n", rc);
}

static void launch_program(char *args)
{
  char pgm[MAXPATH];
  char *p;
  char *q;
  int h;
  int dotseen = 0;

  while (*args != 0 && *args != ' ') args++;
  while (*args == ' ') args++;

  p = args;
  q = pgm;
  while (*p != 0 && *p != ' ')
  {
    if (*p == '.') dotseen = 1;
    if (*p == PS1 || *p == PS2) dotseen = 0;
    *q++ = *p++;
  }
  *q++ = 0;
  if (!dotseen) strcat(pgm, ".exe");

  h = spawn(P_NOWAIT, pgm, args, NULL);
  if (h < 0) 
    printf("%s: %s\n", pgm, strerror(h));
  else
  {
    printf("[job %d started]\n", h);
    close(h);
  }
}

static void load_module(int argc, char **argv)
{
  char *pgm;
  hmodule_t hmod;

  if (argc < 2)
  {
    printf("usage: load <module>\n");
    return;
  }
  pgm = argv[1];

  hmod = load(pgm);
  if (hmod == NULL)
  {
    printf("%s: unable to load module\n", pgm);
    return;
  }
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
    printf("%s: %s\n", devname, strerror(rc));
    return;
  }
}

static void umount_device(int argc, char **argv)
{
  char *path;
  int rc;

  if (argc < 2)
  {
    printf("usage: umount <path>\n");
    return;
  }

  path = argv[1];

  rc = umount(path);
  if (rc < 0)
  {
    printf("%s: %s\n", path, strerror(rc));
    return;
  }
}

static void format_device(int argc, char **argv)
{
  char *devname;
  char *type = "dfs";
  char *opts = NULL;
  int rc;

  if (argc < 2)
  {
    printf("usage: format <device> [<type> [<options>]]\n");
    return;
  }

  devname = argv[1];
  if (argc > 2) type = argv[2];
  if (argc > 3) opts = argv[3];

  rc = format(devname, type, opts); 
  printf("\n");
  if (rc < 0)
  {
    printf("%s: %s\n", devname, strerror(rc));
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
    printf("du: %s\n", strerror(count));
    return;
  }

  buf = (struct statfs *) malloc(count * sizeof(struct statfs));
  rc = getfsstat(buf, count * sizeof(struct statfs));
  if (rc < 0)
  {
    printf("du: %s\n", strerror(rc));
    return;
  }

  printf("type   mounted on mounted from          cache    total     used    avail files\n");
  printf("------ ---------- -------------------- ------ -------- -------- -------- -----\n");

  for (n = 0; n < count; n++)
  {
    b = buf + n;

    printf("%-7s%-11s%-20s", b->fstype, b->mntto, b->mntfrom);
    if (b->blocks != -1)
    {
      printf("%6dK", b->cachesize / K);
      if (b->blocks * (b->bsize / 512) / 2 > 10000)
	printf("%8dM", b->blocks * (b->bsize / 512) / 2000);
      else
	printf("%8dK", b->blocks * (b->bsize / 512) / 2);

      if ((b->blocks - b->bfree) * (b->bsize / 512) / 2 > 10000)
        printf("%8dM", (b->blocks - b->bfree) * (b->bsize / 512) / 2000);
      else
        printf("%8dK", (b->blocks - b->bfree) * (b->bsize / 512) / 2);

      if (b->bfree * (b->bsize / 512) / 2 > 10000)
         printf("%8dM", b->bfree * (b->bsize / 512) / 2000);
      else
        printf("%8dK", b->bfree * (b->bsize / 512) / 2);

      if (b->files != -1 && b->ffree != -1) printf(" %d/%d", b->files - b->ffree, b->files);
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
      printf("%s: %s\n", name, strerror(errno));
      return;
    }

    hp = gethostbyaddr((char *) &addr, sizeof(struct in_addr), AF_INET);
  }
  else
    hp = gethostbyname(name);

  if (!hp)
  {
    printf("%s: %s\n", name, strerror(errno));
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

static int read_line(int f, char *buf)
{
  char c;
  int rc;
  int count;

  count = 0;
  while ((rc = read(f, &c, 1)) > 0)
  {
    if (c == '\r') continue;
    if (c == '\n')
    {
      *buf++ = 0;
      return count;
    }
    *buf++ = c;
    count++;
  }

  *buf = 0;
  return rc;
}

static int httpget(char *server, char *path, char *filename)
{
  struct hostent *hp;
  struct sockaddr_in sin;
  int s;
  int rc;
  int n;
  int count;
  int f;
  char *buf;

  hp = gethostbyname(server);
  if (!hp) return errno;

  s = socket(AF_INET, SOCK_STREAM, 0);
  if (s < 0) return s;

  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  memcpy(&sin.sin_addr, hp->h_addr_list[0], hp->h_length);
  sin.sin_port = htons(80);

  rc = connect(s, (struct sockaddr *) &sin, sizeof(sin));
  if (rc  < 0)
  {
    close(s);
    return rc;
  }

  buf = malloc(8*K);
  if (!buf) return -ENOMEM;

  sprintf(buf, "GET %s HTTP/1.0\r\nHost: %s\r\n\r\n", path, server);
  rc = send(s, buf, strlen(buf), 0);
  if (rc < 0)
  {
    close(s);
    return rc;
  }

  while ((rc = read_line(s, buf)) > 0)
  {
    //printf("hdr %s\n", buf);
  }

  if (rc < 0) return rc;

  f = open(filename, O_CREAT | O_BINARY, S_IREAD | S_IWRITE);
  if (f < 0)
  {
    close(s);
    free(buf);
    return f;
  }

  count = 0;
  while ((n = recv(s, buf, 8*K, 0)) > 0)
  {
    write(f, buf, n);
    count += n;
    //printf("len %d\n", n);
  }

  if (rc < 0)
  {
    close(s);
    close(f);
    free(buf);
    return n;
  }

  close(s);
  close(f);
  free(buf);

  return count;
}

static char *httpgetbuf(char *server, char *path)
{
  struct hostent *hp;
  struct sockaddr_in sin;
  int s;
  int rc;
  int n;
  int len;
  int left;
  char *buf;
  char *p;

  hp = gethostbyname(server);
  if (!hp) return NULL;

  s = socket(AF_INET, SOCK_STREAM, 0);
  if (s < 0) return NULL;

  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  memcpy(&sin.sin_addr, hp->h_addr_list[0], hp->h_length);
  sin.sin_port = htons(80);

  rc = connect(s, (struct sockaddr *) &sin, sizeof(sin));
  if (rc  < 0)
  {
    close(s);
    return NULL;
  }

  buf = malloc(4096);
  if (!buf) return NULL;

  sprintf(buf, "GET %s HTTP/1.0\r\n\r\n", path);
  rc = send(s, buf, strlen(buf), 0);
  if (rc < 0)
  {
    close(s);
    return NULL;
  }

  len = 0;
  while ((rc = read_line(s, buf)) > 0)
  {
    p = strchr(buf, ':');
    if (p)
    {
      *p++ = 0;
      while (*p == ' ') p++;
      if (strcmp(buf, "Content-Length") == 0) len = atoi(p);
    }
  }

  if (rc < 0) return NULL;

  printf("length is %d\n", len);
  buf = realloc(buf, len + 1);

  p = buf;
  left = len;
  while (left > 0 && (n = recv(s, p, left, 0)) > 0)
  {
    p += n;
    left -= n;
  }

  *p = 0;

  if (rc < 0)
  {
    close(s);
    free(buf);
    return NULL;
  }

  close(s);
  return buf;
}

static void http(int argc, char **argv)
{
  char *server;
  char *path;
  char *filename;
  int rc;
  clock_t t;

  server = "192.168.12.1";
  path = "/";
  filename = "/dev/null";
  
  if (argc >= 2) server = argv[1];
  if (argc >= 3) path = argv[2];
  if (argc >= 4) filename = argv[3];
  
  t = clock();
  rc = httpget(server, path, filename);
  if (rc < 0)
    printf("error %d '%s' retrieving %s from %s\n", rc, strerror(rc), path, server);
  else
  {
    t = clock() - t;
    if (t == 0) t = 1;
    printf("received %d bytes in %d ms (%d KB/s)\n", rc, t, rc / t);
  }
}

static void download(int argc, char **argv)
{
  char *server;
  char *src;
  char *dst;
  char fn[256];
  char path[256];
  char *files;
  char *p, *q;
  int dir;
  int rc;

  server = "192.168.12.1";
  src = "/sanos";
  dst = "/usr";
  
  if (argc >= 2) server = argv[1];
  if (argc >= 3) src = argv[2];
  if (argc >= 4) dst = argv[3];

  sprintf(path, "%s/FILES", src);

  files = httpgetbuf(server, path);
  if (!files)
  {
    printf("error retrieving file list from %s\n", server);
    return;
  }

  p = files;
  while (*p)
  {
    q = p;
    while (*q)
    {
      if (q[0] == '\r') break;
      if (q[0] == '/' && q[1] == '\r') break;
      q++;
    }

    if (*q == '/')
    {
      dir = 1;
      *q++ = 0;
    }
    else
      dir = 0;

    *q++ = 0;

    if (dir)
    {
      printf("create directory %s\n", p);
      sprintf(fn, "%s/%s", dst, p);
      rc = mkdir(fn);
      if (rc < 0) printf("%s: error %d '%s' creating directory\n", fn, rc, strerror(rc));
    }
    else
    {
      printf("download file %s\n", p);
      sprintf(fn, "%s/%s", dst, p);
      sprintf(path, "%s/%s", src, p);
      rc = httpget(server, path, fn);
      if (rc < 0) printf("%s: error %d '%s' downloading file\n", fn, rc, strerror(rc));
    }

    if (*q == '\n') *q++ = 0;
    p = q;
  }

  free(files);
}

static void heap_stat()
{
  struct mallinfo m;

  m = mallinfo();

  printf("Non-mmapped space allocated from system .. : %12d\n", m.arena);
  printf("Number of free chunks .................... : %12d\n", m.ordblks);
  printf("Number of fastbin blocks ................. : %12d\n", m.smblks);
  printf("Number of mmapped regions ................ : %12d\n", m.hblks);
  printf("Space in mmapped regions ................. : %12d\n", m.hblkhd);
  printf("Maximum total allocated space ............ : %12d\n", m.usmblks);
  printf("Space available in freed fastbin blocks .. : %12d\n", m.fsmblks);
  printf("Total allocated space .................... : %12d\n", m.uordblks);
  printf("Total free space ......................... : %12d\n", m.fordblks);
  printf("Top-most, releasable space ............... : %12d\n", m.keepcost);
}

static void close_handle(int argc, char **argv)
{
  int fd;
  int rc;

  if (argc != 2)
  {
    printf("usage: close <handle>\n");
    return;
  }

  fd = atoi(argv[1]);
  rc = close(fd);
  if (rc < 0) printf("close: error %d '%s' closing %d\n", rc, strerror(rc), fd);
}

static void httpinit()
{
  struct httpd_server *server;
  
  hmodule_t hmod;
  struct httpd_server *(*_httpd_initialize)(struct section *cfg); 
  struct httpd_context *(*_httpd_add_file_context)(struct httpd_server *server, char *alias, char *location, struct section *cfg);
  struct httpd_context *(*_httpd_add_resource_context)(struct httpd_server *server, char *alias, hmodule_t hmod, struct section *cfg);
  int (*_httpd_start)(struct httpd_server *server);

  hmod = load("httpd");
  if (!hmod)
  {
    printf("httpd not found\n");
    return;
  }

  _httpd_initialize = resolve(hmod, "httpd_initialize");
  _httpd_add_file_context = resolve(hmod, "httpd_add_file_context");
  _httpd_add_resource_context = resolve(hmod, "httpd_add_resource_context");
  _httpd_start = resolve(hmod, "httpd_start");

  server = _httpd_initialize(find_section(config, "httpd"));

  _httpd_add_file_context(server, "/", "/", NULL);
  _httpd_add_resource_context(server, "/icons", hmod, NULL);

  _httpd_start(server);
}

static void test1(int argc, char **argv)
{
  struct servent *sp;
  char **alias;

  sp = getservbyname("echo", "tcp");
  while (sp->s_name != NULL)
  {
    printf("%s %d/%s", sp->s_name, sp->s_port, sp->s_proto);

    alias = sp->s_aliases;
    while (*alias)
    {
      printf(" %s", *alias);
      alias++;
    }
    printf("\n");

    sp++;
  }
}

#if 0
static void test(int argc, char **argv)
{
  char *server;
  int port;
  struct hostent *hp;
  struct sockaddr_in sin;
  int sock;
  int rc;

  //server = "20.45.152.54";
  server = "192.168.123.190";
  port = 999;

  if (argc >= 2) server = argv[1];
  if (argc >= 3) port = atoi(argv[2]);

  hp = gethostbyname(server);
  if (!hp) return;

  sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0) return;

  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  memcpy(&sin.sin_addr, hp->h_addr_list[0], hp->h_length);
  sin.sin_port = htons(port);

  rc = connect(sock, (struct sockaddr *) &sin, sizeof(sin));
  if (rc  < 0)
  {
    syslog(LOG_ERR, "error %d in connect: %s\n", rc, strerror(rc));
    close(sock);
  }

  close(sock);
}
#endif

#if 0
static void test(int argc, char **argv)
{
  char *fmt = "test %%f=[%f] %%g=[%g] %%e=[%e]";

  if (argc > 1) fmt = argv[1];
  printf(fmt, -1.2345, 12345.6789, 0.0987654321, -123456);
  printf("\n");
}
#endif

void __stdcall pipereader(void *arg)
{
  int f = (int) arg;
  char buf[128];
  int rc;

  while ((rc = read(f, buf, 128)) > 0)
  {
    printf("piperead: %d bytes [", rc);
    write(fdout, buf, rc);
    printf("]\n");
  }

  printf("piperead: exit rc=%d\n", rc);
  close(f);
}

static void pipetest(int argc, char **argv)
{
  int fildes[2];
  int hthread;
  int rc;
  int i;

  rc = pipe(fildes);
  if (rc < 0)
  {
    printf("error %d in pipe()\n", rc);
    return;
  }

  hthread = beginthread(pipereader, 0, (void *) fildes[0], 0, NULL);
  close(hthread);

  for (i = 0; i < argc; i++)
  {
    printf("pipewrite: %d bytes [", strlen(argv[i]));
    write(fdout, argv[i], strlen(argv[i]));
    printf("]\n");

    write(fildes[1], argv[i], strlen(argv[i]));
  }

  close(fildes[1]);
}

static void disktest(int argc, char **argv)
{
  int dev;
  int n;
  int rc;

  dev = open(argv[1], O_RDWR | O_DIRECT | O_BINARY);
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

static void divtesthandler(int signum)
{
  struct siginfo *info = getsiginfo();

  printf("in handler\n");
  info->ctxt.ecx = 10;
}

static void divtest(int argc, char **argv)
{
  signal(SIGFPE, divtesthandler);
  printf("100/0=%d\n", 100 / atoi(argv[1]));
  signal(SIGFPE, SIG_DFL);
}

static void test(int argc, char **argv)
{
  //double x = atof(argv[1]);
  //double a, b;

  //a = modf(x, &b);
  //printf("modf(%f) = (%f,%f)\n", x, a, b);

  //printf("sin(%g)=%g\n", x, sin(x));
  //printf("cos(%g)=%g\n", x, cos(x));
  //printf("log(%g)=%g\n", x, log(x));
  //printf("exp(%g)=%g\n", x, exp(x));
  //printf("sin*sin+cos*cos=%g\n", sin(x) * sin(x) + cos(x) * cos(x));

  time_t doomsday = INT_MAX;
  time_t genesis = 0;

  printf("Doomsday %s", asctime(gmtime(&doomsday)));
  printf("Genesis %s", asctime(gmtime(&genesis)));
}

static void set_kprint(int enabled)
{
  ioctl(1, IOCTL_KPRINT_ENABLED, &enabled, 4);
}

static void beep()
{
  ioctl(1, IOCTL_BEEP, NULL, 0);
}

static void sound(int freq)
{
  ioctl(1, IOCTL_SOUND, &freq, 4);
}

static void play_file(int argc, char **argv)
{
  extern void play(char *song);

#if 0
  char buffer[4096];
  FILE *f;

  f = fopen("tunes.txt", "r");
  while (fgets(buffer, sizeof buffer, f))
  {
    printf("%s\n", buffer);
    play(buffer);
  }
  fclose(f);
#endif

  //play("AxelF:d=4, o=5, b=160:f#, 8a., 8f#, 16f#, 8a#, 8f#, 8e, f#, 8c.6, 8f#, 16f#, 8d6, 8c#6, 8a, 8f#, 8c#6, 8f#6, 16f#, 8e, 16e, 8c#, 8g#, f#.");
  //play("dualingbanjos:d=4, o=5, b=200:8c#, 8d, e, c#, d, b4, c#, d#4, b4, p, 16c#6, 16p, 16d6, 16p, 8e6, 8p, 8c#6, 8p, 8d6, 8p, 8b, 8p, 8c#6, 8p, 8a, 8p, b, p, a4, a4, b4, c#, d#4, c#, b4, p, 8a, 8p, 8a, 8p, 8b, 8p, 8c#6, 8p, 8a, 8p, 8c#6, 8p, 8b");
  play("Entertainer:d=4, o=5, b=140:8d, 8d#, 8e, c6, 8e, c6, 8e, 2c.6, 8c6, 8d6, 8d#6, 8e6, 8c6, 8d6, e6, 8b, d6, 2c6, p, 8d, 8d#, 8e, c6, 8e, c6, 8e, 2c.6, 8p, 8a, 8g, 8f#, 8a, 8c6, e6, 8d6, 8c6, 8a, 2d6");
  //play("Barbiegirl:d=4,o=5,b=125:8g#,8e,8g#,8c#6,a,p,8f#,8d#,8f#,8b,g#,8f#,8e,p,8e,8c#,f#,c#,p,8f#,8e,g#,f#");
  //play("Muppets:d=4, o=5, b=250:c6, c6, a, b, 8a, b, g, p, c6, c6, a, 8b, 8a, 8p, g., p, e, e, g, f, 8e, f, 8c6, 8c, 8d, e, 8e, 8e, 8p, 8e, g, 2p, c6, c6, a, b, 8a, b, g, p, c6, c6, a, 8b, a, g., p, e, e, g, f, 8e, f, 8c6, 8c, 8d, e, 8e, d, 8d, c");
}

static void reboot()
{
  ioctl(1, IOCTL_REBOOT, NULL, 0);
}

static void kbdtest()
{
  int escs;
  unsigned char ch;
  int rc;

  escs = 0;
  while (1)
  {
    rc = read(gettib()->job->in, &ch, 1);
    if (rc != 1)
    {
      printf("Error %d in read\n", rc);
      break;
    }

    if (ch == 0x1B) 
      escs++;
    else
      escs = 0;

    if (escs == 3) break;

    printf("[0x%02X]", ch);
  }
  printf("\n");
}

void shell()
{
  char cmd[256];
  int argc;
  char **argv;

  while (1)
  {
    printf("%s$ ", getcwd(cmd, 256));
    if (readline(fdin, cmd, 256) < 0) break;

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
      else if (strcmp(argv[0], "chdir") == 0 || strcmp(argv[0], "cd") == 0)
	change_dir(argv[1]);
      else if (strcmp(argv[0], "mkdir") == 0 || strcmp(argv[0], "md") == 0)
	make_dir(argv[1]);
      else if (strcmp(argv[0], "rmdir") == 0 || strcmp(argv[0], "rd") == 0)
	remove_dir(argv[1]);
      else if (strcmp(argv[0], "mount") == 0)
	mount_device(argc, argv);
      else if (strcmp(argv[0], "format") == 0)
	format_device(argc, argv);
      else if (strcmp(argv[0], "umount") == 0)
	umount_device(argc, argv);
      else if (strcmp(argv[0], "du") == 0)
	disk_usage(argc, argv);
      else if (strcmp(argv[0], "date") == 0)
	show_date(argc, argv);
      else if (strcmp(argv[0], "read") == 0)
	test_read_file(argv[1]);
      else if (strcmp(argv[0], "more") == 0)
	display_file(argv[1]);
      else if (strcmp(argv[0], "write") == 0)
	test_write_file(argv[1], atoi(argv[2]) * K);
      else if (strcmp(argv[0], "disktest") == 0)
	disktest(argc, argv);
      else if (strcmp(argv[0], "cls") == 0)
	printf("\f");
      else if (strcmp(argv[0], "log") == 0)
	set_loglevel(argv[1], atoi(argv[2]));
      else if (strcmp(argv[0], "heapstat") == 0)
	heap_stat();
      else if (strcmp(argv[0], "kp") == 0)
	set_kprint(atoi(argv[1]));
      else if (strcmp(argv[0], "start") == 0)
        launch_program(cmd);
      else if (strcmp(argv[0], "load") == 0)
	load_module(argc, argv);
      else if (strcmp(argv[0], "break") == 0)
	dbgbreak();
      else if (strcmp(argv[0], "nop") == 0)
	nop();
      else if (strcmp(argv[0], "sleep") == 0)
	idle_sleep(atoi(argv[1]));
      else if (strcmp(argv[0], "loop") == 0)
	busy_loop(atoi(argv[1]));
      else if (strcmp(argv[0], "lookup") == 0 || strcmp(argv[0], "nslookup") == 0)
	lookup(argv[1]);
      else if (strcmp(argv[0], "http") == 0)
	http(argc, argv);
      else if (strcmp(argv[0], "download") == 0)
	download(argc, argv);
      else if (strcmp(argv[0], "reboot") == 0)
	reboot();
      else if (strcmp(argv[0], "close") == 0)
	close_handle(argc, argv);
      else if (strcmp(argv[0], "beep") == 0)
	beep();
      else if (strcmp(argv[0], "sound") == 0)
	sound(atoi(argv[1]));
      else if (strcmp(argv[0], "play") == 0)
	play_file(argc, argv);
      else if (strcmp(argv[0], "test") == 0)
	test(argc, argv);
      else if (strcmp(argv[0], "kbd") == 0)
	kbdtest();
      else if (strcmp(argv[0], "cstest") == 0)
	cstest();
      else if (strcmp(argv[0], "httpd") == 0)
	httpinit();
      else
	exec_program(cmd);

      free_args(argc, argv);
    }
  }
}

void redirect(handle_t h, int termtype)
{
  struct job *job = gettib()->job;
  struct crtbase *crtbase = (struct crtbase *) job->crtbase;

  job->in = h;
  job->out = dup(h);
  job->err = dup(h);
  job->termtype = termtype;

  crtbase->iob[0].file = job->in;
  crtbase->iob[0].base = crtbase->iob[0].ptr = crtbase->stdinbuf;
  crtbase->iob[0].flag = _IORD | _IOEXTBUF;
  crtbase->iob[0].cnt = BUFSIZ;

  crtbase->iob[1].file = job->out;
  crtbase->iob[1].flag = _IOWR | _IONBF;

  crtbase->iob[2].file = job->err;
  crtbase->iob[2].flag = _IOWR | _IONBF;
}

void __stdcall ttyd(void *arg)
{
  char *devname = (char *) arg;
  handle_t f;
  struct serial_config cfg;

  f = open(devname, O_RDWR | O_BINARY);
  if (f < 0) 
  {
    syslog(LOG_INFO, "Error %d starting shell on device %s\n", f, devname);
    exit(1);
  }

  cfg.speed = 115200;
  cfg.databits = 8;
  cfg.stopbits = 1;
  cfg.rx_timeout = INFINITE;
  cfg.tx_timeout = INFINITE;
  cfg.parity = PARITY_NONE;

  ioctl(f, IOCTL_SERIAL_SETCONFIG, &cfg, sizeof(struct serial_config));

  redirect(f, TERM_VT100);

  syslog(LOG_INFO, "sh: starting shell on device %s\n", devname);

  shell();
}

void __stdcall telnet_task(void *arg)
{
  int s = (int) arg;

  redirect(s, TERM_VT100);

#ifdef DEBUG
  printf("%s version %s (Debug Build %s %s)\n", OSNAME, OSVERSION, __DATE__, __TIME__);
#else
  printf("%s version %s (Build %s %s)\n", OSNAME, OSVERSION, __DATE__, __TIME__);
#endif
  printf("%s\n\n", COPYRIGHT);

  shell();
}

void __stdcall telnetd(void *arg)
{
  int s;
  int client;
  int rc;
  struct sockaddr_in sin;
  int hthread;

  s = socket(AF_INET, SOCK_STREAM, 0);
  if (s < 0)
  {
    printf("telnetd: error %d in socket\n", s);
    return;
  }

  sin.sin_len = sizeof(struct sockaddr_in);
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = INADDR_ANY;
  sin.sin_port = htons(23);
  rc = bind(s, (struct sockaddr *) &sin, sizeof sin);
  if (rc < 0)
  {
    printf("telnetd: error %d in bind\n", rc);
    return;
  }

  rc = listen(s, 5);
  if (rc < 0)
  {
    printf("telnetd: error %d in listen\n", rc);
    return;
  }

  while (1)
  {
    struct sockaddr_in sin;

    client = accept(s, (struct sockaddr *) &sin, NULL);
    if (client < 0)
    {
      printf("telnetd: error %d in accept\n", client);
      return;
    }

    syslog(LOG_INFO, "telnetd: client connected from %a\n", &sin.sin_addr);

    hthread = beginthread(telnet_task, 0, (void *) client, CREATE_NEW_JOB | CREATE_DETACHED, NULL);
    close(hthread);
  }
}

int main(int argc, char *argv[])
{
  if (sizeof(struct tib) != PAGESIZE) printf("warning: tib is %d bytes (%d expected)\n", sizeof(struct tib), PAGESIZE);

  //beginthread(ttyd, 0, "/dev/com4", CREATE_NEW_JOB | CREATE_DETACHED, NULL);
  if (peb->ipaddr.s_addr != INADDR_ANY) beginthread(telnetd, 0, NULL, 0, NULL);

  shell();

  return 0;
}
