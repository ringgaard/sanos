//
// cmds.c
//
// Shell commands
//
// Copyright (C) 2012 Michael Ringgaard. All rights reserved.
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
#include <time.h>

#include "sh.h"

#define BUFSIZE 4096

static void display_buffer(FILE *f, char *buffer, int size) {
  char *p;
  char *q;
  char *end;

  p = buffer;
  end = buffer + size;
  while (p < end) {
    q = p;
    while (q < end && *q != '\n') q++;

    if (p != q) fwrite(p, q - p, 1, f);
    if (q < end) {
      fwrite("\r\n", 2, 1, f);
      q++;
    }
    p = q;
  }
}

static int read_line(int f, char *buf) {
  char c;
  int rc;
  int count;

  count = 0;
  while ((rc = read(f, &c, 1)) > 0) {
    if (c == '\r') continue;
    if (c == '\n') {
      *buf++ = 0;
      return count;
    }
    *buf++ = c;
    count++;
  }

  *buf = 0;
  return rc;
}

static int httpget(char *server, char *path, char *filename) {
  struct hostent *hp;
  struct sockaddr_in sin;
  struct utsname sys;
  int s;
  int rc;
  int n;
  int count;
  int f;
  char *buf;

  memset(&sys, 0, sizeof(struct utsname));
  uname(&sys);
  
  hp = gethostbyname(server);
  if (!hp) return -errno;

  s = socket(AF_INET, SOCK_STREAM, 0);
  if (s < 0) return s;

  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  memcpy(&sin.sin_addr, hp->h_addr_list[0], hp->h_length);
  sin.sin_port = htons(80);

  rc = connect(s, (struct sockaddr *) &sin, sizeof(sin));
  if (rc  < 0) {
    close(s);
    return rc;
  }

  buf = malloc(8 * 1024);
  if (!buf) return -ENOMEM;

  sprintf(buf, "GET %s HTTP/1.0\r\nHost: %s\r\nUser-Agent: httpget/1.0 %s/%s (Build %s; CPU %s)\r\n\r\n", 
          path, server, sys.sysname, sys.release, sys.version, sys.machine);
  rc = send(s, buf, strlen(buf), 0);
  if (rc < 0) {
    close(s);
    return rc;
  }

  while ((rc = read_line(s, buf)) > 0) {
    //printf("hdr %s\n", buf);
  }

  if (rc < 0) return rc;

  f = open(filename, O_CREAT | O_TRUNC | O_BINARY, S_IREAD | S_IWRITE);
  if (f < 0) {
    close(s);
    free(buf);
    return f;
  }

  count = 0;
  while ((n = recv(s, buf, 8 * 1024, 0)) > 0) {
    write(f, buf, n);
    count += n;
    //printf("len %d\n", n);
  }

  if (rc < 0) {
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

shellcmd(beep) {
  return ioctl(1, IOCTL_BEEP, NULL, 0);
}

shellcmd(bye) {
  exitos(EXITOS_POWEROFF);
  return 0;
}

shellcmd(cat) {
  int count;
  int fd;
  int i;
  char *filename;
  char buffer[BUFSIZE];

  if (argc == 1) {
    while ((count = read(fdin, buffer, sizeof buffer)) > 0) {
      write(fdout, buffer, count);
    }
  } else {
    for (i = 1; i < argc; i++) {
      filename = argv[i];
      if ((fd = open(filename, O_BINARY)) < 0) {
        fprintf(stderr, "%s: %s\n", filename, strerror(fd));
        return 1;
      }
      while ((count = read(fd, buffer, sizeof buffer)) > 0) {
        fwrite(buffer, 1, count, stdout);
      }
      close(fd);
    }
  }
  return 0;
}

shellcmd(cls) {
  printf("\033c\f");
  return 0;
}

shellcmd(glob) {
  glob_t globbuf;
  int i;
  int rc;

  if (argc > 1) {
    for (i = 1; i < argc; i++) {
      rc = glob(argv[i], i > 1 ? GLOB_APPEND : 0, NULL, &globbuf);
      if (rc < 0) printf("glob(%s) returned %d\n", argv[i], rc);
    }

    for (i = 0; i < (int) globbuf.gl_pathc; i++) {
      printf("%s\n", globbuf.gl_pathv[i]);
    }
    globfree(&globbuf);
  }

  return 0;
}

shellcmd(ln) {
  char *target;
  char *linkname;

  if (argc != 3) {
    printf("usage: ln <target> <link name>\n");
    return -EINVAL;
  }
  target = argv[1];
  linkname = argv[2];

  if (link(target, linkname) < 0) {
    perror(target);
    return -1;
  }

  return 0;
}

shellcmd(crypt) {
  char *pw;
  char *salt;
  char saltc[3];

  if (argc != 2 && argc != 3) {
    printf("usage: crypt <pw> [<salt>]\n");
    return -EINVAL;
  }

  pw = argv[1];
  if (argc == 3) {
    salt = argv[2];
  } else {
    int i;
    salt = saltc;
    for (i = 0; i < 2; i++) {
      saltc[i] = (char) (random() & 077) + '.';
      if (saltc[i] > '9') saltc[i] += 7;
      if (saltc[i] > 'Z') saltc[i] += 6;
    }
    saltc[2] = 0;
  }

  printf("%s\n", crypt(pw, salt));
  return 0;
}

shellcmd(date) {
  time_t t;
  struct tm tm;

  t = time(NULL);
  gmtime_r(&t, &tm);

  if (argc > 1) {
    char buffer[256];

    strftime(buffer, 256, argv[1], &tm);
    printf("%s\n", buffer);
  } else {
    printf("Time is %04d/%02d/%02d %02d:%02d:%02d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
  }
  return 0;
}

shellcmd(debug) {
  struct peb *peb = gettib()->peb;

  if (argc > 1) {
    if (strcmp(argv[1], "on") == 0) {
      peb->debug = 1;
    } else if (strcmp(argv[1], "off") == 0) {
      peb->debug = 0;
    } else {
      printf("usage: debug [on|off]\n");
      return -EINVAL;
    }
  }

  printf("debug %s\n", peb->debug ? "on" : "off");
  return 0;
}

shellcmd(df) {
  int count;
  int rc;
  int n;
  struct statfs *buf;
  struct statfs *b;

  count = getfsstat(NULL, 0);
  if (count < 0) {
    printf("df: %s\n", strerror(errno));
    return -1;
  }

  buf = (struct statfs *) malloc(count * sizeof(struct statfs));
  rc = getfsstat(buf, count * sizeof(struct statfs));
  if (rc < 0) {
    printf("df: %s\n", strerror(errno));
    return -1;
  }

  printf("type   mounted on mounted from          cache    total     used    avail files\n");
  printf("------ ---------- -------------------- ------ -------- -------- -------- -----\n");

  for (n = 0; n < count; n++) {
    b = buf + n;

    printf("%-7s%-11s%-20s", b->fstype, b->mntto, b->mntfrom);
    if (b->blocks != -1) {
      printf("%6dK", b->cachesize / 1024);
      if (b->blocks * (b->bsize / 512) / 2 > 10000) {
        printf("%8dM", b->blocks * (b->bsize / 512) / 2000);
      } else {
        printf("%8dK", b->blocks * (b->bsize / 512) / 2);
      }

      if ((b->blocks - b->bfree) * (b->bsize / 512) / 2 > 10000) {
        printf("%8dM", (b->blocks - b->bfree) * (b->bsize / 512) / 2000);
      } else {
        printf("%8dK", (b->blocks - b->bfree) * (b->bsize / 512) / 2);
      }

      if (b->bfree * (b->bsize / 512) / 2 > 10000) {
         printf("%8dM", b->bfree * (b->bsize / 512) / 2000);
      } else {
        printf("%8dK", b->bfree * (b->bsize / 512) / 2);
      }

      if (b->files != -1 && b->ffree != -1) printf(" %d/%d", b->files - b->ffree, b->files);
    }

    printf("\n");
  }

  free(buf);
  return 0;
}

shellcmd(dump) {
  char *filename;
  int fd;
  unsigned char buf[16];
  int pos;
  int count;
  int line;
  int i;
  int ch;

  if (argc != 2) {
    printf("usage: dump <file>\n");
    return -EINVAL;
  }
  filename = argv[1];

  if ((fd = open(filename, O_BINARY)) < 0) {
    printf("%s: %s\n", filename, strerror(errno));
    return -EINVAL;
  }

  line = 0;
  pos = 0;
  while ((count = read(fd, buf, 16)) > 0) {
    printf("%08X ", pos);
    for (i = 0; i < count; i++) printf("%02X ", buf[i]);
    for (i = count; i < 16; i++) printf("   ");
    for (i = 0; i < count; i++) printf("%c", buf[i] < 0x20 ? '.' : buf[i]);
    printf("\n");

    if (line == 23) {
      fflush(stdout);
      ch = getchar();
      if (ch == 'x') {
        close(fd);
        return 0;
      }
      line = 0;
    } else {
      line++;
    }

    pos += count;
  }

  close(fd);
  return 0;
}

shellcmd(echo) {
  int i;

  for (i = 1; i < argc; i++) {
    if (i > 1) fputc(' ', stdout);
    fputs(argv[i], stdout);
  }
  fputs("\n", stdout);

  return 0;
}

shellcmd(id) {
  gid_t groups[NGROUPS_MAX];
  int ngroups;
  struct passwd *pwd = getpwuid(getuid());
  struct group *grp = getgrgid(getgid());

  if (!pwd || !grp) {
    perror("id");
    return -1;
  }

  printf("uid=%d(%s) gid=%d(%s)", pwd->pw_uid, pwd->pw_name, grp->gr_gid, grp->gr_name);

  ngroups = getgroups(NGROUPS_MAX, groups);
  if (ngroups > 0) {
    int i;

    printf(" groups");
    for (i = 0; i < ngroups; i++) {
      grp = getgrgid(groups[i]);
      printf("%c%d(%s)", (i == 0 ? '=' : ','), groups[i], grp->gr_name);
    }
  }

  printf("\n");
  return 0;


  return 0;
}

shellcmd(ifconfig) {
  struct peb *peb = gettib()->peb;
  int sock;
  struct ifcfg iflist[16];
  int n, i;

  if (*peb->hostname) {
    printf("Host Name ............... : %s\n", peb->hostname);
  }

  if (*peb->default_domain) {
    printf("Domain Name ............. : %s\n", peb->default_domain);
  }

  if (peb->primary_dns.s_addr != INADDR_ANY) {
    printf("Primary DNS Server ...... : %s\n", inet_ntoa(peb->primary_dns));
  }

  if (peb->secondary_dns.s_addr != INADDR_ANY) {
    printf("Secondary DNS Server .... : %s\n", inet_ntoa(peb->secondary_dns));
  }

  if (peb->ntp_server1.s_addr != INADDR_ANY) {
    printf("Primary NTP Server ...... : %s\n", inet_ntoa(peb->ntp_server1));
  }

  if (peb->ntp_server2.s_addr != INADDR_ANY) {
    printf("Secondary NTP Server .... : %s\n", inet_ntoa(peb->ntp_server2));
  }

  sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock < 0) return -1;

  n = ioctl(sock, SIOIFLIST, iflist, sizeof iflist);
  if (n < 0) {
    close(sock);
    return -1;
  }

  for (i = 0; i < n / (int) sizeof(struct ifcfg); i++) {
    struct sockaddr_in *addr = ((struct sockaddr_in *) &iflist[i].addr);
    struct sockaddr_in *gw = ((struct sockaddr_in *) &iflist[i].gw);
    struct sockaddr_in *mask = ((struct sockaddr_in *) &iflist[i].netmask);
    struct sockaddr_in *bcast = ((struct sockaddr_in *) &iflist[i].broadcast);
    unsigned char *hwaddr = (unsigned char *) iflist[i].hwaddr;

    printf("\n");
    printf("Network interface %s:\n", iflist[i].name);
    printf("  IP Address ......... : %s\n", inet_ntoa(addr->sin_addr));
    printf("  Subnet Mask ........ : %s\n", inet_ntoa(mask->sin_addr));
    printf("  Default Gateway .... : %s\n", inet_ntoa(gw->sin_addr));
    printf("  Broadcast Address .. : %s\n", inet_ntoa(bcast->sin_addr));

    printf("  Physical Address ... : %02x:%02x:%02x:%02x:%02x:%02x\n",
      hwaddr[0], hwaddr[1], hwaddr[2], 
      hwaddr[3], hwaddr[4], hwaddr[5]);

    printf("  Flags .............. :");
    if (iflist[i].flags & IFCFG_UP) printf(" UP");
    if (iflist[i].flags & IFCFG_DHCP) printf(" DHCP");
    if (iflist[i].flags & IFCFG_DEFAULT) printf(" DEFAULT");
    if (iflist[i].flags & IFCFG_LOOPBACK) printf(" LOOPBACK");
    printf("\n");
  }

  close(sock);
  return 0;
}

shellcmd(keyb) {
  char *keybname;
  int keymap;
  int rc;

  if (argc != 2) {
    printf("usage: keyb [us|uk|dk|fr]\n");
    return -EINVAL;
  }
  keybname = argv[1];

  if (strcmp(keybname, "us") == 0) {
    keymap = 0;
  } else if (strcmp(keybname, "dk") == 0) {
    keymap = 1;
  } else if (strcmp(keybname, "uk") == 0) {
    keymap = 2;
  } else if (strcmp(keybname, "fr") == 0) {
    keymap = 3;
  } else {
    printf("keyb: unknown keymap '%s'\n", keybname);
    return -EINVAL;
  }

  rc = ioctl(0, IOCTL_SET_KEYMAP, &keymap, sizeof(int));
  return rc;
}

shellcmd(heapstat) {
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

shellcmd(httpget) {
  char *server;
  char *path;
  char *filename;
  int rc;
  clock_t t;

  server = "localhost";
  path = "/";
  filename = "/dev/null";

  if (argc == 1) {
    printf("usage: httpget <server> [<url> [<filename>]]\n");
    return -EINVAL;
  }

  if (argc >= 2) server = argv[1];
  if (argc >= 3) path = argv[2];
  if (argc >= 4) filename = argv[3];
  
  t = clock();
  rc = httpget(server, path, filename);
  if (rc < 0) {
    printf("Error %d '%s' retrieving %s from %s\n", errno, strerror(errno), path, server);
    return -1;
  }
   
  t = clock() - t;
  if (t == 0) t = 1;
  printf("Received %d bytes in %d ms (%d KB/s)\n", rc, t, rc / t);

  return 0;
}

shellcmd(kbd) {
  int escs;
  int ch;

  printf("(press esc three times to exit)\n");

  escs = 0;
  while (1) {
    fflush(stdout);
    ch = getchar();

    if (ch == 0x1B) {
      escs++;
    } else {
      escs = 0;
    }

    if (escs == 3) break;

    printf("[0x%02X]", ch);
  }
  printf("\n");
  return 0;
}

shellcmd(kill) {
  int pid = getpid();
  int signum = SIGINT;
  int rc;
  
  if (argc > 1) pid = atoi(argv[1]);
  if (argc > 2) signum = atoi(argv[2]);

  rc = kill(pid, signum);
  if (rc < 0) perror("kill");

  return 0;
}

shellcmd(klog) {
  int enabled;
  int klog;
  int rc;

  if (argc != 2) {
    printf("usage: klog [on|off]\n");
    return -EINVAL;
  }

  enabled = strcmp(argv[1], "on") == 0;

  klog = open("/dev/klog", 0);
  rc = ioctl(klog, IOCTL_KPRINT_ENABLED, &enabled, 4);
  close(klog);

  return rc;
}

shellcmd(load) {
  char *pgm;
  hmodule_t hmod;

  if (argc != 2) {
    printf("usage: load <module>\n");
    return -EINVAL;
  }
  pgm = argv[1];

  hmod = dlopen(pgm, 0);
  if (hmod == NULL) {
    printf("%s: unable to load module\n", pgm);
    return -1;
  }

  return 0;
}

shellcmd(loglevel) {
  int level;
  int ll = 0;

  if (argc != 2) {
    printf("usage: logevel [<loglevel>]\n");
    return -EINVAL;
  }

  level = atoi(argv[1]);
  setlogmask((1 << (level + 1)) - 1);
  return 0;
}

shellcmd(mkfs) {
  char *devname;
  char *type = "dfs";
  char *opts = NULL;
  int rc;

  if (argc < 2) {
    printf("usage: mkfs <device> [<type> [<options>]]\n");
    return -EINVAL;
  }

  devname = argv[1];
  if (argc > 2) type = argv[2];
  if (argc > 3) opts = argv[3];

  rc = mkfs(devname, type, opts); 
  if (rc < 0) {
    printf("%s: %s\n", devname, strerror(errno));
    return -1;
  }

  return 0;
}

shellcmd(more) {
  char *filename;
  int count;
  int fd;
  int line;
  char *start;
  char *end;
  char ch;
  char buffer[BUFSIZE];
  struct term *term;

  if (argc == 1) {
    fd = dup(fdin);
  } else if (argc == 2) {
    filename = argv[1];
    if ((fd = open(filename, O_BINARY)) < 0) {
      printf("%s: %s\n", filename, strerror(errno));
      return fd;
    }
  } else {
    printf("usage: more <file>\n");
    return -EINVAL;
  }

  term = gettib()->proc->term;
  line = 0;
  while ((count = read(fd, buffer, sizeof buffer)) > 0) {
    start = end = buffer;
    while (end < buffer + count) {
      if (line == 24) {
        display_buffer(stdout, start, end - start);
        fflush(stdout);
        if (read(term->ttyin, &ch, 1) < 0) break;
        if (ch == 'q') {
          close(fd);
          return 0;
        }
        start = end;
        line = 0;
      }
      if (*end++ == '\n') line++;
    }
    display_buffer(stdout, start, end - start);
  }

  close(fd);
  return 0;
}

shellcmd(mount) {
  char *devname;
  char *path;
  char *type = "dfs";
  char *opts = NULL;
  int rc;

  if (argc < 3) {
    printf("usage: mount <device> <path> [<type> [<options>]]\n");
    return -EINVAL;
  }

  devname = argv[1];
  path = argv[2];
  if (argc > 3) type = argv[3];
  if (argc > 4) opts = argv[4];

  rc = mount(type, path, devname, opts); 
  if (rc < 0) {
    printf("%s: %s\n", devname, strerror(errno));
    return -1;
  }

  return 0;
}

shellcmd(nslookup) {
  char *name;
  struct hostent *hp;
  int i;

  if (argc != 2) {
    printf("usage: nslookup [<hostname>|<ipaddress>]\n");
    return -EINVAL;
  }
  name = argv[1];

  if (!*name) return 0;

  if (*name >= '0' && *name <= '9') {
    struct in_addr addr;
    addr.s_addr = inet_addr(name);
    if (addr.s_addr == INADDR_NONE) {
      printf("%s: %s\n", name, strerror(errno));
      return -1;
    }

    hp = gethostbyaddr((char *) &addr, sizeof(struct in_addr), AF_INET);
  } else {
    hp = gethostbyname(name);
  }

  if (!hp) {
    printf("%s: %s\n", name, strerror(errno));
    return -errno;
  }

  if (hp->h_addrtype != AF_INET) {
    printf("unknown address type %d\n", hp->h_addrtype);
    return 0;
  }

  if (hp->h_length != sizeof(struct in_addr)) {
    printf("unknown address length %d\n", hp->h_length);
    return 0;
  }

  printf("name: %s\n", hp->h_name);
  for (i = 0; hp->h_aliases[i] != NULL; i++) printf("alias: %s\n", hp->h_aliases[i]);
  for (i = 0; hp->h_addr_list[i] != NULL; i++) {
    struct in_addr *addr = (struct in_addr *) (hp->h_addr_list[i]);
    printf("address: %s\n", inet_ntoa(*addr));
  }

  return 0;
}

shellcmd(pwd) {
  char cwd[FILENAME_MAX];
  if (getcwd(cwd, FILENAME_MAX) == NULL) {
    perror("cwd");
    return 1;
  }
  printf("%s\n", cwd);
  return 0;
}

shellcmd(ps) {
  struct peb *peb = gettib()->peb;
  struct process *proc = peb->firstproc;

  printf("  id parent name         hmod     threads in  out err command line\n");
  printf("---- ------ ------------ -------- ------- --- --- --- -------------------------\n");

  while (proc) {
    printf("%4d %6d %-13s%08x%8d%4d%4d%4d %s\n", proc->id, proc->parent ? proc->parent->id : 0, proc->ident, proc->hmod, proc->threadcnt, proc->iob[0], proc->iob[1], proc->iob[2], proc->cmdline ? proc->cmdline : "");
    proc = proc->nextproc;
  }

  return 0;
}

shellcmd(read) {
  char *filename;
  int count;
  int fd;
  char *data;
  clock_t start;
  clock_t time;
  int bytes;

  if (argc != 2) {
    printf("usage: read <file>\n");
    return -EINVAL;
  }
  filename = argv[1];

  if ((fd = open(filename, 0)) < 0) {
    printf("%s: %s\n", filename, strerror(errno));
    return fd;
  }

  data = malloc(64 * 1024);

  bytes = 0;
  start = clock();
  while ((count = read(fd, data, 64 * 1024)) > 0) {
    bytes += count;
  }
  time = clock() - start;
  if (time == 0) time = 1;

  printf("%s: read %dKB in %d ms, %dKB/s\n", filename, bytes / 1024, time, bytes / time);
  
  free(data);

  if (count < 0) printf("%s: %s\n", filename, strerror(errno));

  close(fd);
  return 0;
}

shellcmd(reboot) {
  exitos(EXITOS_REBOOT);
  return 0;
}

shellcmd(rmdir) {
  char *path;
  int rc;

  if (argc != 2) {
    printf("usage: rmdir <path>\n");
    return -EINVAL;
  }
  path = argv[1];

  rc = rmdir(path); 
  if (rc < 0) {
    printf("%s: %s\n", path, strerror(errno));
    return -1;
  }

  return 0;
}

shellcmd(sleep) {
  int ms;

  if (argc != 2) {
    printf("usage: sleep <millis>\n");
    return -EINVAL;
  }
  ms = atoi(argv[1]);

  msleep(ms);
  return 0;
}

shellcmd(sound) {
  int freq;

  if (argc != 2) {
    printf("usage: sound <freq> (0=silence)\n");
    return -EINVAL;
  }
  freq = atoi(argv[1]);

  return ioctl(1, IOCTL_SOUND, &freq, 4);
}

shellcmd(shutdown) {
  exitos(EXITOS_POWEROFF);
  return 0;
}

shellcmd(sysinfo) {
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

  printf("cpu vendor: %d family: %d model: %d stepping: %d mhz: %d feat: %08X pagesize: %d vendorid: %s modelid: %s\n", 
         cpu.cpu_vendor, cpu.cpu_family, cpu.cpu_model, cpu.cpu_stepping, cpu.cpu_mhz, cpu.cpu_features, 
         cpu.pagesize, cpu.vendorid, cpu.modelid);

  printf("mem phys total: %d K avail: %d K virt total: %d K avail: %d K pagesize: %d\n", 
         mem.physmem_total / 1024, mem.physmem_avail / 1024, mem.virtmem_total / 1024, mem.virtmem_avail / 1024, mem.pagesize);

  printf("load uptime: %d s user: %d%% sys: %d%% intr: %d%% idle: %d%%\n", 
         load.uptime, load.load_user, load.load_system, load.load_intr, load.load_idle);

  return 0;
}

shellcmd(uname) {
  struct utsname buf;
  
  if (uname(&buf) < 0) {
    perror("uname");
    return -1;
  }

  printf("%s %s %s %s %s\n", buf.sysname, buf.nodename, buf.release, buf.version, buf.machine);
  return 0;
}

shellcmd(umount) {
  char *path;
  int rc;

  if (argc != 2) {
    printf("usage: umount <path>\n");
    return -EINVAL;
  }
  path = argv[1];

  rc = umount(path);
  if (rc < 0) {
    printf("%s: %s\n", path, strerror(errno));
    return -1;
  }

  return 0;
}

shellcmd(whoami) {
  struct passwd *pwd;

  pwd = getpwuid(getuid());
  if (pwd) {
    printf("%s\n", pwd->pw_name);
  } else {
    printf("uid%d\n", getuid);
  }

  return 0;
}

shellcmd(write) {
  char *filename;
  int size;
  int count;
  int fd;
  char *data;
  clock_t start;
  clock_t time;
  int bytes;

  if (argc != 3) {
    printf("usage: write <file> <size (in kb)>\n");
    return -EINVAL;
  }
  filename = argv[1];
  size = atoi(argv[2]) * 1024;

  if ((fd = open(filename, O_CREAT | O_BINARY, S_IREAD | S_IWRITE)) < 0) {
    printf("%s: %s\n", filename, strerror(errno));
    return -1;
  }

  data = malloc(64 * 1024);

  bytes = 0;
  start = clock();
  while (bytes < size) {
    if ((count = write(fd, data, 64 * 1024)) <= 0) {
      printf("%s: error writing file\n", filename);
      break;
    }

    bytes += count;
  }
  time = clock() - start;
  if (time == 0) time = 1;

  printf("%s: wrote %dKB in %d ms, %dKB/s\n", filename, bytes / 1024, time, bytes / time);
  
  free(data);

  close(fd);
  return 0;
}

