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

int telnetd_started = 0;

#define BUFSIZE 4096

typedef int (*cmdproc_t)(int argc, char *argv[]);

struct command
{
  char *cmdname;
  cmdproc_t proc;
  char *help;
};

struct command cmdtab[];

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
  if (!hp) return -errno;

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

int cmd_beep(int argc, char *argv[])
{
  return ioctl(1, IOCTL_BEEP, NULL, 0);
}

int cmd_break(int argc, char *argv[])
{
  dbgbreak();
  return 0;
}

int cmd_cat(int argc, char *argv[])
{
  int count;
  int fd;
  char *filename;
  char buffer[BUFSIZE];

  if (argc != 2)
  {
    printf("usage: cat <filename>\n");
    return -EINVAL;

  }
  filename = argv[1];

  if ((fd = open(filename, O_BINARY)) < 0)
  {
    printf("%s: %s\n", filename, strerror(fd));
    return fd;
  }

  while ((count = read(fd, buffer, sizeof buffer)) > 0)
  {
    write(fdout, buffer, count);
  }

  close(fd);
  return 0;
}

int cmd_chdir(int argc, char *argv[])
{
  char *path;
  int rc;

  if (argc != 2)
  {
    printf("usage: chdir <path>\n");
    return -EINVAL;
  }
  path = argv[1];

  rc = chdir(path); 
  if (rc < 0)
  {
    printf("%s: %s\n", path, strerror(rc));
    return rc;
  }

  return 0;
}

int cmd_cls(int argc, char *argv[])
{
  printf("\f");
  return 0;
}

int cmd_cp(int argc, char *argv[])
{
  char *data;
  int count;
  int fd1;
  int fd2;
  char *srcfn;
  char *dstfn;

  if (argc != 3)
  {
    printf("usage: cp <source file> <dest file>\n");
    return -EINVAL;
  }
  srcfn = argv[1];
  dstfn = argv[2];

  if ((fd1 = open(srcfn, O_BINARY)) < 0)
  {
    printf("%s: %s\n", srcfn, strerror(fd1));
    return fd1;
  }

  if ((fd2 = open(dstfn, O_CREAT | O_BINARY, S_IREAD | S_IWRITE)) < 0)
  {
    close(fd1);
    printf("%s: %s\n", dstfn, strerror(fd2));
    return fd2;
  }

  data = malloc(64 * K);
  while ((count = read(fd1, data, 64 * K)) > 0)
  {
    if (write(fd2, data, count) != count)
    {
      printf("%s: error writing data\n", dstfn);
      break;
    }
  }
  free(data);

  if (count < 0) 
  {
    printf("%s: %s\n", srcfn, strerror(count));
    close(fd1);
    close(fd2);
    return count;
  }
  
  close(fd1);
  close(fd2);
  return 0;
}

int cmd_date(int argc, char *argv[])
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

  return 0;
}

int cmd_df(int argc, char *argv[])
{
  int count;
  int rc;
  int n;
  struct statfs *buf;
  struct statfs *b;

  count = getfsstat(NULL, 0);
  if (count < 0)
  {
    printf("df: %s\n", strerror(count));
    return count;
  }

  buf = (struct statfs *) malloc(count * sizeof(struct statfs));
  rc = getfsstat(buf, count * sizeof(struct statfs));
  if (rc < 0)
  {
    printf("df: %s\n", strerror(rc));
    return rc;
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
  return 0;
}

int cmd_dump(int argc, char *argv[])
{
  char *filename;
  int fd;
  unsigned char buf[16];
  int pos;
  int count;
  int line;
  int i;
  char ch;

  if (argc != 2)
  {
    printf("usage: dump <file>\n");
    return -EINVAL;
  }
  filename = argv[1];

  if ((fd = open(filename, O_BINARY)) < 0)
  {
    printf("%s: %s\n", filename, strerror(fd));
    return -EINVAL;
  }

  line = 0;
  pos = 0;
  while ((count = read(fd, buf, 16)) > 0)
  {
    printf("%08X ", pos);
    for (i = 0; i < count; i++) printf("%02X ", buf[i]);
    for (i = count; i < 16; i++) printf("   ");
    for (i = 0; i < count; i++) printf("%c", buf[i] < 0x20 ? '.' : buf[i]);
    printf("\n");

    if (line == 23)
    {
      while (read(fdin, &ch, 1) <= 0);
      if (ch == 'x')
      {
	close(fd);
	return 0;
      }
      line = 0;
    }
    else
      line++;

    pos += count;
  }

  close(fd);
  return 0;
}

int cmd_grabcon(int argc, char *argv[])
{
  dup2(fdin, 0);
  dup2(fdout, 1);
  dup2(fderr, 2);

  return 0;
}

int cmd_heapstat(int argc, char *argv[])
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

  return 0;
}

int cmd_help(int argc, char *argv[])
{
  struct command *command;
  int lineno;
  int ok;
  int n;

  lineno = 0;
  for (command = cmdtab; command->cmdname; command++)
  {
    ok = 1;
    if (argc > 1)
    {
      ok = 0;
      for (n = 1; n < argc; n++)
      {
	if (strstr(command->cmdname, argv[n]) ||
	    command->help && strstr(command->help, argv[n]))
	{
	  ok = 1;
	  break;
	}
      }
    }

    if (ok && command->help) 
    {
      printf("%-10.10s %s\n", command->cmdname, command->help);
      if (++lineno == 24)
      {
	getchar();
	lineno = 0;
      }
    }
  }

  return 0;
}

int cmd_httpget(int argc, char *argv[])
{
  char *server;
  char *path;
  char *filename;
  int rc;
  clock_t t;

  server = "localhost";
  path = "/";
  filename = "/dev/null";

  if (argc == 1)
  {
    printf("usage: httpget <server> [<url> [<filename>]]\n");
    return -EINVAL;
  }

  if (argc >= 2) server = argv[1];
  if (argc >= 3) path = argv[2];
  if (argc >= 4) filename = argv[3];
  
  t = clock();
  rc = httpget(server, path, filename);
  if (rc < 0)
  {
    printf("Error %d '%s' retrieving %s from %s\n", rc, strerror(rc), path, server);
    return rc;
  }
   
  t = clock() - t;
  if (t == 0) t = 1;
  printf("Received %d bytes in %d ms (%d KB/s)\n", rc, t, rc / t);

  return 0;
}

int cmd_jobs(int argc, char *argv[])
{
  struct job *job = peb->firstjob;

  printf("hmod     threads in  out err name\n");
  printf("-------- ------- --- --- --- ------------------------------------------------\n");

  while (job)
  {
    char *name;
    char modname[MAXPATH];

    if (!job->cmdline || !*job->cmdline)
    {
      getmodpath(job->hmod, modname, MAXPATH);
      name = modname;
    }
    else
      name = job->cmdline;

    printf("%08x%8d%4d%4d%4d %s\n", job->hmod, job->threadcnt, job->in, job->out, job->err, name);
    job = job->nextjob;
  }

  return 0;
}

int cmd_kbd(int argc, char *argv[])
{
  int escs;
  unsigned char ch;
  int rc;

  printf("(press esc three times to exit)\n");

  escs = 0;
  while (1)
  {
    rc = read(fdin, &ch, 1);
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
  return 0;
}

int cmd_klog(int argc, char *argv[])
{
  int enabled;

  if (argc != 2)
  {
    printf("usage: klog [on|off]\n");
  }

  enabled = strcmp(argv[1], "on") == 0;
  return ioctl(1, IOCTL_KPRINT_ENABLED, &enabled, 4);
}

int cmd_load(int argc, char *argv[])
{
  char *pgm;
  hmodule_t hmod;

  if (argc != 2)
  {
    printf("usage: load <module>\n");
    return -EINVAL;
  }
  pgm = argv[1];

  hmod = load(pgm);
  if (hmod == NULL)
  {
    printf("%s: unable to load module\n", pgm);
    return -errno;
  }

  return 0;
}

int cmd_loglevel(int argc, char *argv[])
{
  char *arg; 
  int level;
  int ll = 0;

  if (argc != 3)
  {
    printf("usage: logevel [m|h|t|a|*]+ <loglevel>\n");
    return -EINVAL;
  }
  arg = argv[1];
  level = atoi(argv[2]);

  while (*arg)
  {
    if (*arg == 'm') ll |= LOG_MODULE;
    if (*arg == 'h') ll |= LOG_HEAP;
    if (*arg == 't') ll |= LOG_APITRACE;
    if (*arg == 'a') ll |= LOG_AUX;
    if (*arg == '*') ll |= LOG_SUBSYS_MASK;

    arg++;
  }

  loglevel = ll | level;
  return 0;
}

int cmd_ls(int argc, char *argv[])
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
    return dir;
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
  return 0;
}

int cmd_mkdir(int argc, char *argv[])
{
  char *path;
  int rc;

  if (argc != 2)
  {
    printf("usage: mkdir <path>\n");
    return -EINVAL;
  }
  path = argv[1];

  rc = mkdir(path, 0666); 
  if (rc < 0)
  {
    printf("%s: %s\n", path, strerror(rc));
    return rc;
  }

  return 0;
}

int cmd_mkfs(int argc, char *argv[])
{
  char *devname;
  char *type = "dfs";
  char *opts = NULL;
  int rc;

  if (argc < 2)
  {
    printf("usage: mkfs <device> [<type> [<options>]]\n");
    return -EINVAL;
  }

  devname = argv[1];
  if (argc > 2) type = argv[2];
  if (argc > 3) opts = argv[3];

  rc = mkfs(devname, type, opts); 
  printf("\n");
  if (rc < 0)
  {
    printf("%s: %s\n", devname, strerror(rc));
    return rc;
  }

  return 0;
}

int cmd_more(int argc, char *argv[])
{
  char *filename;
  int count;
  int fd;
  int line;
  char *start;
  char *end;
  char ch;
  char buffer[BUFSIZE];

  if (argc != 2)
  {
    printf("usage: more <file>\n");
    return -EINVAL;
  }
  filename = argv[1];

  if ((fd = open(filename, O_BINARY)) < 0)
  {
    printf("%s: %s\n", filename, strerror(fd));
    return fd;
  }

  line = 0;
  while ((count = read(fd, buffer, sizeof buffer)) > 0)
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
	  close(fd);
	  return 0;
	}
	start = end;
        line = 0;
      }
      if (*end++ == '\n') line++;
    }
    display_buffer(fdout, start, end - start);
  }

  close(fd);
  return 0;
}

int cmd_mount(int argc, char *argv[])
{
  char *devname;
  char *path;
  char *type = "dfs";
  char *opts = NULL;
  int rc;

  if (argc < 3)
  {
    printf("usage: mount <device> <path> [<type> [<options>]]\n");
    return -EINVAL;
  }

  devname = argv[1];
  path = argv[2];
  if (argc > 3) type = argv[3];
  if (argc > 4) opts = argv[4];

  rc = mount(type, path, devname, opts); 
  if (rc < 0)
  {
    printf("%s: %s\n", devname, strerror(rc));
    return rc;
  }

  return 0;
}

int cmd_mv(int argc, char *argv[])
{
  char *oldname;
  char *newname;
  int rc;

  if (argc != 3)
  {
    printf("usage: mv <old name> <new name>\n");
    return -EINVAL;
  }
  oldname = argv[1];
  newname = argv[2];

  rc = rename(oldname, newname);
  if (rc < 0)
  {
    printf("%s: %s\n", oldname, strerror(rc));
    return rc;
  }

  return 0;
}

int cmd_nslookup(int argc, char *argv[])
{
  char *name;
  struct hostent *hp;
  int i;

  if (argc != 2)
  {
    printf("usage: nslookup [<hostname>|<ipaddress>]\n");
    return -EINVAL;
  }
  name = argv[1];

  if (!*name) return 0;

  if (*name >= '0' && *name <= '9')
  {
    struct in_addr addr;
    addr.s_addr = inet_addr(name);
    if (addr.s_addr == INADDR_NONE)
    {
      printf("%s: %s\n", name, strerror(errno));
      return -errno;
    }

    hp = gethostbyaddr((char *) &addr, sizeof(struct in_addr), AF_INET);
  }
  else
    hp = gethostbyname(name);

  if (!hp)
  {
    printf("%s: %s\n", name, strerror(errno));
    return -errno;
  }

  if (hp->h_addrtype != AF_INET)
  {
    printf("unknown address type %d\n", hp->h_addrtype);
    return 0;
  }

  if (hp->h_length != sizeof(struct in_addr))
  {
    printf("unknown address length %d\n", hp->h_length);
    return 0;
  }

  printf("name: %s\n", hp->h_name);
  for (i = 0; hp->h_aliases[i] != NULL; i++) printf("alias: %s\n", hp->h_aliases[i]);
  for (i = 0; hp->h_addr_list[i] != NULL; i++) 
  {
    struct in_addr *addr = (struct in_addr *) (hp->h_addr_list[i]);
    printf("address: %s\n", inet_ntoa(*addr));
  }

  return 0;
}

int cmd_play(int argc, char *argv[])
{
  char *filename;
  extern void play(char *song);

  //play("AxelF:d=4, o=5, b=160:f#, 8a., 8f#, 16f#, 8a#, 8f#, 8e, f#, 8c.6, 8f#, 16f#, 8d6, 8c#6, 8a, 8f#, 8c#6, 8f#6, 16f#, 8e, 16e, 8c#, 8g#, f#.");
  //play("dualingbanjos:d=4, o=5, b=200:8c#, 8d, e, c#, d, b4, c#, d#4, b4, p, 16c#6, 16p, 16d6, 16p, 8e6, 8p, 8c#6, 8p, 8d6, 8p, 8b, 8p, 8c#6, 8p, 8a, 8p, b, p, a4, a4, b4, c#, d#4, c#, b4, p, 8a, 8p, 8a, 8p, 8b, 8p, 8c#6, 8p, 8a, 8p, 8c#6, 8p, 8b");
  //play("Entertainer:d=4, o=5, b=140:8d, 8d#, 8e, c6, 8e, c6, 8e, 2c.6, 8c6, 8d6, 8d#6, 8e6, 8c6, 8d6, e6, 8b, d6, 2c6, p, 8d, 8d#, 8e, c6, 8e, c6, 8e, 2c.6, 8p, 8a, 8g, 8f#, 8a, 8c6, e6, 8d6, 8c6, 8a, 2d6");
  //play("Barbiegirl:d=4,o=5,b=125:8g#,8e,8g#,8c#6,a,p,8f#,8d#,8f#,8b,g#,8f#,8e,p,8e,8c#,f#,c#,p,8f#,8e,g#,f#");
  //play("Muppets:d=4, o=5, b=250:c6, c6, a, b, 8a, b, g, p, c6, c6, a, 8b, 8a, 8p, g., p, e, e, g, f, 8e, f, 8c6, 8c, 8d, e, 8e, 8e, 8p, 8e, g, 2p, c6, c6, a, b, 8a, b, g, p, c6, c6, a, 8b, a, g., p, e, e, g, f, 8e, f, 8c6, 8c, 8d, e, 8e, d, 8d, c");

  char buffer[4096];
  FILE *f;

  if (argc != 2)
  {
    printf("usage: play <rttl file>\n");
    return -EINVAL;
  }
  filename = argv[1];

  f = fopen(filename, "r");
  if (!f)
  {
    printf("%s: %s\n", filename, strerror(errno));
    return -errno;
  }
  
  while (fgets(buffer, sizeof buffer, f))
  {
    printf("%s\n", buffer);
    play(buffer);
  }
  fclose(f);

  return 0;
}

int cmd_read(int argc, char *argv[])
{
  char *filename;
  int count;
  int fd;
  char *data;
  clock_t start;
  clock_t time;
  int bytes;

  if (argc != 2)
  {
    printf("usage: read <file>\n");
    return -EINVAL;
  }
  filename = argv[1];

  if ((fd = open(filename, 0)) < 0)
  {
    printf("%s: %s\n", filename, strerror(fd));
    return fd;
  }

  data = malloc(64 * K);

  bytes = 0;
  start = clock();
  while ((count = read(fd, data, 64 * K)) > 0)
  {
    bytes += count;
  }
  time = clock() - start;
  if (time == 0) time = 1;

  printf("%s: read %dKB in %d ms, %dKB/s\n", filename, bytes / K, time, bytes / time);
  
  free(data);

  if (count < 0) printf("%s: %s\n", filename, strerror(count));

  close(fd);
  return 0;
}

int cmd_reboot(int argc, char *argv[])
{
  return ioctl(1, IOCTL_REBOOT, NULL, 0);  
}

int cmd_rm(int argc, char *argv[])
{
  char *filename;
  int rc;

  if (argc != 2)
  {
    printf("usage: rm <file>\n");
    return -EINVAL;
  }
  filename = argv[1];

  rc = unlink(filename); 
  if (rc < 0)
  {
    printf("%s: %s\n", filename, strerror(rc));
    return rc;
  }

  return 0;
}

int cmd_rmdir(int argc, char *argv[])
{
  char *path;
  int rc;

  if (argc != 2)
  {
    printf("usage: rmdir <path>\n");
    return -EINVAL;
  }
  path = argv[1];

  rc = rmdir(path); 
  if (rc < 0)
  {
    printf("%s: %s\n", path, strerror(rc));
    return rc;
  }

  return 0;
}

int cmd_sound(int argc, char *argv[])
{
  int freq;

  if (argc != 2)
  {
    printf("usage: sound <freq> (0=silence)\n");
    return -EINVAL;
  }
  freq = atoi(argv[1]);

  return ioctl(1, IOCTL_SOUND, &freq, 4);
}

int cmd_sysinfo(int argc, char *argv[])
{
  int rc;
  struct cpuinfo cpu;
  struct meminfo mem;
  struct loadinfo load;

  rc = sysinfo(SYSINFO_CPU, &cpu, sizeof(cpu));
  if (rc < 0) return rc;

  rc = sysinfo(SYSINFO_MEM, &mem, sizeof(mem));
  if (rc < 0) return rc;

  rc = sysinfo(SYSINFO_LOAD, &load, sizeof(load));
  if (rc < 0) return rc;

  printf("cpu vendor: %d family: %d model: %d stepping: %d mhz: %d feat: %08X pagesize: %d\n", 
         cpu.cpu_vendor, cpu.cpu_family, cpu.cpu_model, cpu.cpu_stepping, cpu.cpu_mhz, cpu.cpu_features, cpu.pagesize);

  printf("mem phys total: %d K avail: %d K virt total: %d K avail: %d K pagesize: %d\n", 
         mem.physmem_total / K, mem.physmem_avail / K, mem.virtmem_total / K, mem.virtmem_avail / K, mem.pagesize);

  printf("load uptime: %d s user: %d%% sys: %d%% intr: %d%% idle: %d%%\n", 
         load.uptime, load.load_user, load.load_system, load.load_intr, load.load_idle);

  return 0;
}

int cmd_test(int argc, char *argv[])
{
#if 0
  struct hostent *hp;
  struct sockaddr_in sin;
  int s;
  int rc;

  hp = gethostbyname(argv[1] ? argv[1] : "192.168.174.2");
  if (!hp) return errno;

  s = socket(AF_INET, SOCK_STREAM, 0);
  if (s < 0) return s;

  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  memcpy(&sin.sin_addr, hp->h_addr_list[0], hp->h_length);
  sin.sin_port = htons(2000);

  rc = connect(s, (struct sockaddr *) &sin, sizeof(sin));
  if (rc  < 0)
  {
    close(s);
    return rc;
  }

  printf("connected socket %d\n", s);
  return 0;
#endif

  char buffer[64000];
  memset(buffer, 0, sizeof buffer);
  return buffer[0];
}

int cmd_umount(int argc, char *argv[])
{
  char *path;
  int rc;

  if (argc != 2)
  {
    printf("usage: umount <path>\n");
    return -EINVAL;
  }
  path = argv[1];

  rc = umount(path);
  if (rc < 0)
  {
    printf("%s: %s\n", path, strerror(rc));
    return rc;
  }

  return 0;
}

int cmd_write(int argc, char *argv[])
{
  char *filename;
  int size;
  int count;
  int fd;
  char *data;
  clock_t start;
  clock_t time;
  int bytes;

  if (argc != 3)
  {
    printf("usage: write <file> <size (in kb)>\n");
    return -EINVAL;
  }
  filename = argv[1];
  size = atoi(argv[2]) * 1024;

  if ((fd = open(filename, O_CREAT | O_BINARY, S_IREAD | S_IWRITE)) < 0)
  {
    printf("%s: %s\n", filename, strerror(fd));
    return fd;
  }

  data = malloc(64 * K);

  bytes = 0;
  start = clock();
  while (bytes < size)
  {
    if ((count = write(fd, data, 64 * K)) <= 0)
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

  close(fd);
  return 0;
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
    if (q - pgm == MAXPATH - 1) break;
    *q++ = *p++;
  }
  *q++ = 0;
  if (!dotseen && strlen(pgm) + 5 < MAXPATH) strcat(pgm, ".exe");

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
    if (q - pgm == MAXPATH - 1) break;
    *q++ = *p++;
  }
  *q++ = 0;
  if (!dotseen && strlen(pgm) + 5 < MAXPATH) strcat(pgm, ".exe");

  h = spawn(P_NOWAIT, pgm, args, NULL);
  if (h < 0) 
    printf("%s: %s\n", pgm, strerror(h));
  else
  {
    printf("[job %d started]\n", h);
    close(h);
  }
}

struct command cmdtab[] =
{
  {"?",        cmd_help,     "This help"},
  {"beep",     cmd_beep,     "Play beep in speaker"},
  {"break",    cmd_break,    "Debug breakpoint"},
  {"cat",      cmd_cat,      "Display file"},
  {"cd",       cmd_chdir,    "Change current directory"},
  {"chdir",    cmd_chdir,    "Change current directory"},
  {"copy",     cmd_cp,       "Copy file"},
  {"cls",      cmd_cls,      "Clear screen"},
  {"cp",       cmd_cp,       "Copy file"},
  {"date",     cmd_date,     "Display date"},
  {"del",      cmd_rm,       "Delete file"},
  {"df",       cmd_df,       "Display file system usage"},
  {"dir",      cmd_ls,       "List directory"},
  {"dump",     cmd_dump,     "Display file in hex format"},
  {"exit",     NULL,         "Exit shell"},
  {"grabcon",  cmd_grabcon,  "Grab the main console file descriptors"},
  {"heapstat", cmd_heapstat, "Display heap statistics"},
  {"help",     cmd_help,     "This help"},
  {"httpget",  cmd_httpget,  "Retrieve file via http"},
  {"jobs",     cmd_jobs,     "Display job list"},
  {"kbd",      cmd_kbd,      "Keyboard test"},
  {"klog",     cmd_klog,     "Enable/disable kernel log messages"},
  {"less",     cmd_more,     "Display file paginated"},
  {"load",     cmd_load,     "Load module"},
  {"loglevel", cmd_loglevel, "Set syslog tracing mask and level"},
  {"lookup",   cmd_nslookup, "Lookup hostname or IP address using DNS"},
  {"ls",       cmd_ls,       "List directory"},
  {"md",       cmd_mkdir,    "Make new directory"},
  {"mkdir",    cmd_mkdir,    "Make new directory"},
  {"mkfs",     cmd_mkfs,     "Format device for new file system"},
  {"more",     cmd_more,     "Display file paginated"},
  {"mount",    cmd_mount,    "Mount file system"},
  {"move",     cmd_mv,       "Move file"},
  {"mv",       cmd_mv,       "Move file"},
  {"nslookup", cmd_nslookup, "Lookup hostname or IP address using DNS"},
  {"play",     cmd_play,     "Play RTTTL file in speaker"},
  {"read",     cmd_read,     "Read file from disk"},
  {"reboot",   cmd_reboot,   "Reboot computer"},
  {"rm",       cmd_rm,       "Delete file"},
  {"rd",       cmd_rmdir,    "Remove directory"},
  {"rmdir",    cmd_rmdir,    "Remove directory"},
  {"sound",    cmd_sound,    "Play sound in speaker"},
  {"sysinfo",  cmd_sysinfo,  "Display system info"},
  {"test",     cmd_test,     "Dummy command for misc. tests"},
  {"type",     cmd_cat,      "Display file"},
  {"umount",   cmd_umount,   "Unmount file system"},
  {"write",    cmd_write,    "Write zero filled file to disk"},
  {NULL, NULL, NULL}
};

void shell()
{
  char curdir[MAXPATH];
  char cmdline[256];
  int argc;
  char **argv;
  struct command *command;

  while (1)
  {
    printf("%s$ ", getcwd(curdir, sizeof curdir));
    if (readline(fdin, cmdline, sizeof cmdline) < 0) break;

    argc = parse_args(cmdline, NULL);
    if (!argc) continue;

    argv = (char **) malloc(argc * sizeof(char *));
    parse_args(cmdline, argv);

    for (command = cmdtab; command->cmdname; command++)
      if (strcmp(argv[0], command->cmdname) == 0) break;

    if (command->cmdname)
    {
      if (!command->proc) 
      {
	free_args(argc, argv);
	break;
      }
      command->proc(argc, argv);
    }
    else if (strcmp(argv[0], "start") == 0)
      launch_program(cmdline);
    else
      exec_program(cmdline);

    free_args(argc, argv);
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
  crtbase->iob[1].flag = _IOWR | _IONBF | _IOCRLF;

  crtbase->iob[2].file = job->err;
  crtbase->iob[2].flag = _IOWR | _IONBF | _IOCRLF;
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

  if (peb->ipaddr.s_addr != INADDR_ANY) 
  {
    if (get_numeric_property(osconfig, "shell", "telnetd", 0) && !telnetd_started)
    {
      beginthread(telnetd, 0, NULL, 0, NULL);
      telnetd_started = 1;
    }
  }

  shell();

  return 0;
}
