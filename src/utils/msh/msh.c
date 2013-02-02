//
// msh.c
//
// Mini shell
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
#include <glob.h>
#include <os/version.h>
#include <inifile.h>

#include <pwd.h>
#include <grp.h>

#define BUFSIZE 4096

int execute_script(char *cmdfile);

typedef int (*cmdproc_t)(int argc, char *argv[]);

struct command {
  char *cmdname;
  cmdproc_t proc;
  char *help;
};

extern struct command cmdtab[];

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
  if (rc  < 0) {
    close(s);
    return rc;
  }

  buf = malloc(8 * 1024);
  if (!buf) return -ENOMEM;

  sprintf(buf, "GET %s HTTP/1.0\r\nHost: %s\r\n\r\n", path, server);
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

int cmd_beep(int argc, char *argv[]) {
  return ioctl(1, IOCTL_BEEP, NULL, 0);
}

int cmd_break(int argc, char *argv[]) {
  dbgbreak();
  return 0;
}

int cmd_cat(int argc, char *argv[]) {
  int count;
  int fd;
  char *filename;
  char buffer[BUFSIZE];

  if (argc != 2) {
    printf("usage: cat <filename>\n");
    return -EINVAL;
  }
  filename = argv[1];

  if ((fd = open(filename, O_BINARY)) < 0) {
    printf("%s: %s\n", filename, strerror(fd));
    return fd;
  }

  while ((count = read(fd, buffer, sizeof buffer)) > 0) {
    fwrite(buffer, 1, count, stdout);
  }

  close(fd);
  return 0;
}

int cmd_chdir(int argc, char *argv[]) {
  char *path;

  if (argc != 2) {
    printf("usage: chdir <path>\n");
    return -EINVAL;
  }
  path = argv[1];

  if (chdir(path) < 0) {
    printf("%s: %s\n", path, strerror(errno));
    return -1;
  }

  return 0;
}

int cmd_chmod(int argc, char *argv[]) {
  int mode;
  char *path;
  char *p;

  if (argc != 3) {
    printf("usage: chmod <mode> <path>\n");
    return -EINVAL;
  }

  path = argv[2];

  mode = 0;
  p = argv[1];
  while (*p) {
    if (*p < '0' || *p > '7') {
      printf("Illegal mode\n");
      return -EINVAL;
    }
    mode = (mode << 3) + (*p++ - '0');
  }

  if (chmod(path, mode) < 0) perror(path);

  return 0;
}

int cmd_chown(int argc, char *argv[]) {
  struct passwd *pwd = NULL;
  struct group *grp = NULL;
  char *path;
  char *p;

  if (argc != 3) {
    printf("usage: chown <owner>:<group> <path>\n");
    return -EINVAL;
  }

  path = argv[2];

  p = strchr(argv[1], ':');
  if (p) {
    *p++ = 0;
    pwd = getpwnam(argv[1]);
    grp = getgrnam(p);

    if (!pwd && !grp) {
      printf("Unknown user/group\n");
      return -EINVAL;
    }
  } else {
    pwd = getpwnam(argv[1]);
    if (!pwd) {
      printf("Unknown user\n");
      return -EINVAL;
    }
  }

  if (chown(path, pwd ? pwd->pw_uid : -1, grp ? grp->gr_gid : -1) < 0) perror(path);

  return 0;
}

int cmd_cls(int argc, char *argv[]) {
  printf("\f");
  return 0;
}

int cmd_cp(int argc, char *argv[]) {
  char *data;
  int count;
  int fd1;
  int fd2;
  char *srcfn;
  char *dstfn;
  struct stat st;

  if (argc != 3) {
    printf("usage: cp <source file> <dest file>\n");
    return -EINVAL;
  }
  srcfn = argv[1];
  dstfn = argv[2];

  if ((fd1 = open(srcfn, O_BINARY)) < 0) {
    printf("%s: %s\n", srcfn, strerror(errno));
    return -1;
  }
  fstat(fd1, &st);

  if ((fd2 = open(dstfn, O_CREAT | O_TRUNC | O_BINARY, st.st_mode)) < 0) {
    close(fd1);
    printf("%s: %s\n", dstfn, strerror(errno));
    return -1;
  }

  data = malloc(64 * 1024);
  while ((count = read(fd1, data, 64 * 1024)) > 0) {
    if (write(fd2, data, count) != count) {
      printf("%s: error writing data (%s)\n", dstfn, strerror(errno));
      break;
    }
  }
  free(data);

  if (count < 0) {
    printf("%s: %s\n", srcfn, strerror(errno));
    close(fd1);
    close(fd2);
    return -1;
  }
  
  close(fd1);
  close(fd2);
  return 0;
}

int cmd_ln(int argc, char *argv[]) {
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

int cmd_crypt(int argc, char *argv[]) {
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

int cmd_date(int argc, char *argv[]) {
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

int cmd_debug(int argc, char *argv[]) {
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

int cmd_df(int argc, char *argv[]) {
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

int cmd_exec(int argc, char *argv[]) {
  char *filename;

  if (argc != 2) {
    printf("usage: dump <file>\n");
    return -EINVAL;
  }
  
  filename = argv[1];
  return execute_script(filename);
}

int cmd_dump(int argc, char *argv[]) {
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

int cmd_grabcon(int argc, char *argv[]) {
  dup2(fdin, 0);
  dup2(fdout, 1);
  dup2(fderr, 2);

  return 0;
}

int cmd_id(int argc, char *argv[]) {
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

int cmd_ifconfig(int argc, char *argv[]) {
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

int cmd_keyb(int argc, char *argv[]) {
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

int cmd_heapstat(int argc, char *argv[]) {
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

int cmd_help(int argc, char *argv[]) {
  struct command *command;
  int lineno;
  int ok;
  int n;

  lineno = 0;
  for (command = cmdtab; command->cmdname; command++) {
    ok = 1;
    if (argc > 1) {
      ok = 0;
      for (n = 1; n < argc; n++) {
        if (strstr(command->cmdname, argv[n]) ||
            command->help && strstr(command->help, argv[n])) {
          ok = 1;
          break;
        }
      }
    }

    if (ok && command->help) {
      printf("%-10.10s %s\n", command->cmdname, command->help);
      if (++lineno == 24) {
        fflush(stdout);
        getchar();
        lineno = 0;
      }
    }
  }

  return 0;
}

int cmd_httpget(int argc, char *argv[]) {
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

int cmd_ps(int argc, char *argv[]) {
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

int cmd_kbd(int argc, char *argv[]) {
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

int cmd_kill(int argc, char *argv[]) {
  int pid = getpid();
  int signum = SIGINT;
  int rc;
  
  if (argc > 1) pid = atoi(argv[1]);
  if (argc > 2) signum = atoi(argv[2]);

  rc = kill(pid, signum);
  if (rc < 0) perror("kill");

  return 0;
}

int cmd_klog(int argc, char *argv[]) {
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

int cmd_load(int argc, char *argv[]) {
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

int cmd_loglevel(int argc, char *argv[]) {
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

int cmd_ls(int argc, char *argv[]) {
  char *dirname;
  int verbose;
  int i;
  int dir;
  struct direntry dirp;
  struct stat buf;
  struct tm tm;
  char path[MAXPATH];
  char *arg;
  int col;
  char perm[11];
  struct passwd *pwd;
  struct group *grp;

  verbose = 0;
  dirname = ".";
  for (i = 1; i < argc; i++) {
    arg = argv[i];

    if (*arg == '-') {
      while (*++arg) {
        if (*arg == 'l') verbose = 1;
        if (*arg == 'w') verbose = 2;
      }
    } else {
      dirname = arg;
    }
  }

  if ((dir = _opendir(dirname)) < 0) {
    printf("%s: %s\n", dirname, strerror(errno));
    return dir;
  }

  col = 0;
  while (_readdir(dir, &dirp, 1) > 0) {
    strcpy(path, dirname);
    strcat(path, "/");
    strcat(path, dirp.name);

    if (stat(path, &buf) < 0) memset(&buf, 0, sizeof(struct stat));

    if (verbose == 2) {
      if (col == 4) col = 0;
      if ((buf.st_mode & S_IFMT) == S_IFDIR) strcat(dirp.name, "/");
      printf("%-20s", dirp.name);
      col++;
    } else {
      strcpy(path, dirname);
      strcat(path, "/");
      strcat(path, dirp.name);

      if (stat(path, &buf) < 0) memset(&buf, 0, sizeof(struct stat));

      if (verbose) {
        printf("%8d %4d %1d %2d ", (int) buf.st_size, buf.st_ino, buf.st_nlink, buf.st_dev);

        gmtime_r(&buf.st_ctime, &tm);
        printf("%02d/%02d/%04d %02d:%02d:%02d ", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, tm.tm_hour, tm.tm_min, tm.tm_sec);
      } else {
        strcpy(perm, " ---------");
        switch (buf.st_mode & S_IFMT) {
          case S_IFREG: perm[0] = '-'; break;
          case S_IFLNK: perm[0] = 'l'; break;
          case S_IFDIR: perm[0] = 'd'; break;
          case S_IFBLK: perm[0] = 'b'; break;
          case S_IFCHR: perm[0] = 'c'; break;
          case S_IFPKT: perm[0] = 'p'; break;
        }

        if (buf.st_mode & 0400) perm[1] = 'r';
        if (buf.st_mode & 0200) perm[2] = 'w';
        if (buf.st_mode & 0100) perm[3] = 'x';
        if (buf.st_mode & 0040) perm[4] = 'r';
        if (buf.st_mode & 0020) perm[5] = 'w';
        if (buf.st_mode & 0010) perm[6] = 'x';
        if (buf.st_mode & 0004) perm[7] = 'r';
        if (buf.st_mode & 0002) perm[8] = 'w';
        if (buf.st_mode & 0001) perm[9] = 'x';
        printf("%s ", perm);

        pwd = getpwuid(buf.st_uid);
        grp = getgrgid(buf.st_gid);
        printf("%-8s %-8s", pwd ? pwd->pw_name : "?", grp ? grp->gr_name : "?");

        if ((buf.st_mode & S_IFMT) == S_IFDIR) {
          printf("           ");
        } else {
          printf("%10d ", buf.st_size);
        }
      }

      gmtime_r(&buf.st_mtime, &tm);
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

int cmd_mkdir(int argc, char *argv[]) {
  char *path;
  int rc;

  if (argc != 2) {
    printf("usage: mkdir <path>\n");
    return -EINVAL;
  }
  path = argv[1];

  rc = mkdir(path, 0755); 
  if (rc < 0) {
    printf("%s: %s\n", path, strerror(errno));
    return -1;
  }

  return 0;
}

int cmd_mkfs(int argc, char *argv[]) {
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

int cmd_more(int argc, char *argv[]) {
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

  if ((fd = open(filename, O_BINARY)) < 0) {
    printf("%s: %s\n", filename, strerror(errno));
    return fd;
  }

  line = 0;
  while ((count = read(fd, buffer, sizeof buffer)) > 0) {
    start = end = buffer;
    while (end < buffer + count) {
      if (line == 24) {
        display_buffer(stdout, start, end - start);
        fflush(stdout);
        ch = getchar();
        if (ch == 'x') {
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

int cmd_mount(int argc, char *argv[]) {
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

int cmd_mv(int argc, char *argv[]) {
  char *oldname;
  char *newname;
  int rc;

  if (argc != 3) {
    printf("usage: mv <old name> <new name>\n");
    return -EINVAL;
  }
  oldname = argv[1];
  newname = argv[2];

  rc = rename(oldname, newname);
  if (rc < 0) {
    printf("%s: %s\n", oldname, strerror(errno));
    return -1;
  }

  return 0;
}

int cmd_nslookup(int argc, char *argv[]) {
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

int cmd_play(int argc, char *argv[]) {
  char *filename;
  extern void play(char *song);

  //play("AxelF:d=4, o=5, b=160:f#, 8a., 8f#, 16f#, 8a#, 8f#, 8e, f#, 8c.6, 8f#, 16f#, 8d6, 8c#6, 8a, 8f#, 8c#6, 8f#6, 16f#, 8e, 16e, 8c#, 8g#, f#.");
  //play("dualingbanjos:d=4, o=5, b=200:8c#, 8d, e, c#, d, b4, c#, d#4, b4, p, 16c#6, 16p, 16d6, 16p, 8e6, 8p, 8c#6, 8p, 8d6, 8p, 8b, 8p, 8c#6, 8p, 8a, 8p, b, p, a4, a4, b4, c#, d#4, c#, b4, p, 8a, 8p, 8a, 8p, 8b, 8p, 8c#6, 8p, 8a, 8p, 8c#6, 8p, 8b");
  //play("Entertainer:d=4, o=5, b=140:8d, 8d#, 8e, c6, 8e, c6, 8e, 2c.6, 8c6, 8d6, 8d#6, 8e6, 8c6, 8d6, e6, 8b, d6, 2c6, p, 8d, 8d#, 8e, c6, 8e, c6, 8e, 2c.6, 8p, 8a, 8g, 8f#, 8a, 8c6, e6, 8d6, 8c6, 8a, 2d6");
  //play("Barbiegirl:d=4,o=5,b=125:8g#,8e,8g#,8c#6,a,p,8f#,8d#,8f#,8b,g#,8f#,8e,p,8e,8c#,f#,c#,p,8f#,8e,g#,f#");
  //play("Muppets:d=4, o=5, b=250:c6, c6, a, b, 8a, b, g, p, c6, c6, a, 8b, 8a, 8p, g., p, e, e, g, f, 8e, f, 8c6, 8c, 8d, e, 8e, 8e, 8p, 8e, g, 2p, c6, c6, a, b, 8a, b, g, p, c6, c6, a, 8b, a, g., p, e, e, g, f, 8e, f, 8c6, 8c, 8d, e, 8e, d, 8d, c");

  char buffer[4096];
  FILE *f;

  if (argc != 2) {
    printf("usage: play <rttl file>\n");
    return -EINVAL;
  }
  filename = argv[1];

  f = fopen(filename, "r");
  if (!f) {
    printf("%s: %s\n", filename, strerror(errno));
    return -1;
  }
  
  while (fgets(buffer, sizeof buffer, f)) {
    printf("%s\n", buffer);
    play(buffer);
  }
  fclose(f);

  return 0;
}

int cmd_read(int argc, char *argv[]) {
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

int cmd_reboot(int argc, char *argv[]) {
  exitos(EXITOS_REBOOT);
  return 0;
}

int cmd_rm(int argc, char *argv[]) {
  char *filename;
  int rc;
  int i;

  if (argc < 2) {
    printf("usage: rm <file(s)>\n");
    return -EINVAL;
  }

  for (i = 1; i < argc; i++) {
    filename = argv[i];
    rc = unlink(filename); 
    if (rc < 0) {
      printf("%s: %s\n", filename, strerror(errno));
      return -1;
    }
  }

  return 0;
}

int cmd_rmdir(int argc, char *argv[]) {
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

int cmd_set(int argc, char *argv[]) {
  char **env = environ;
  int n;

  if (argc <= 1) {
    if (env) for (n = 0; env[n]; n++) printf("%s\n", env[n]);
  } else {
    for (n = 1; n < argc; n++) putenv(argv[n]);
  }

  return 0;
}

int cmd_sleep(int argc, char *argv[]) {
  int ms;

  if (argc != 2) {
    printf("usage: sleep <millis>\n");
    return -EINVAL;
  }
  ms = atoi(argv[1]);

  msleep(ms);
  return 0;
}

int cmd_sound(int argc, char *argv[]) {
  int freq;

  if (argc != 2) {
    printf("usage: sound <freq> (0=silence)\n");
    return -EINVAL;
  }
  freq = atoi(argv[1]);

  return ioctl(1, IOCTL_SOUND, &freq, 4);
}

int cmd_shutdown(int argc, char *argv[]) {
  exitos(EXITOS_POWEROFF);
  return 0;
}

int cmd_sysinfo(int argc, char *argv[]) {
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
         mem.physmem_total / 1024, mem.physmem_avail / 1024, mem.virtmem_total / 1024, mem.virtmem_avail / 1024, mem.pagesize);

  printf("load uptime: %d s user: %d%% sys: %d%% intr: %d%% idle: %d%%\n", 
         load.uptime, load.load_user, load.load_system, load.load_intr, load.load_idle);

  return 0;
}

int cmd_test(int argc, char *argv[]) {
  int rc;
  
  errno = 0;

  rc = access(argv[1], F_OK);
  printf("access(%s,F_OK)=%d %d %s\n", argv[1], rc, errno, strerror(errno));

  rc = access(argv[1], R_OK);
  printf("access(%s,R_OK)=%d %d %s\n", argv[1], rc, errno, strerror(errno));

  rc = access(argv[1], W_OK);
  printf("access(%s,W_OK)=%d %d %s\n", argv[1], rc, errno, strerror(errno));

  rc = access(argv[1], X_OK);
  printf("access(%s,X_OK)=%d %d %s\n", argv[1], rc, errno, strerror(errno));

  return 0;
}

int cmd_glob(int argc, char *argv[]) {
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

int cmd_uname(int argc, char *argv[]) {
  struct utsname buf;
  
  if (uname(&buf) < 0) {
    perror("uname");
    return -1;
  }

  printf("%s %s %s %s %s\n", buf.sysname, buf.nodename, buf.release, buf.version, buf.machine);
  return 0;
}

int cmd_umount(int argc, char *argv[]) {
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

int cmd_whoami(int argc, char *argv[]) {
  struct passwd *pwd;

  pwd = getpwuid(getuid());
  if (pwd) {
    printf("%s\n", pwd->pw_name);
  } else {
    printf("uid%d\n", getuid);
  }

  return 0;
}

int cmd_write(int argc, char *argv[]) {
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

struct command cmdtab[] = {
  {"?",        cmd_help,     "This help"},
  {"beep",     cmd_beep,     "Play beep in speaker"},
  {"break",    cmd_break,    "Debug breakpoint"},
  {"bye",      cmd_shutdown, "Shutdown computer and power off"},
  {"cat",      cmd_cat,      "Display file"},
  {"cd",       cmd_chdir,    "Change current directory"},
  {"chdir",    cmd_chdir,    "Change current directory"},
  {"chmod",    cmd_chmod,    "Change file permissions"},
  {"chown",    cmd_chown,    "Change file owner"},
  {"copy",     cmd_cp,       "Copy file"},
  {"cls",      cmd_cls,      "Clear screen"},
  {"cp",       cmd_cp,       "Copy file"},
  {"crypt",    cmd_crypt,    "Encrypt password"},
  {"date",     cmd_date,     "Display date"},
  {"debug",    cmd_debug,    "Enable/disable debug mode"},
  {"del",      cmd_rm,       "Delete file"},
  {"df",       cmd_df,       "Display file system usage"},
  {"dir",      cmd_ls,       "List directory"},
  {"dump",     cmd_dump,     "Display file in hex format"},
  {"exec",     cmd_exec,     "Execute shell script"},
  {"exit",     NULL,         "Exit shell"},
  {"glob",     cmd_glob,     "Expand filename arguments"},
  {"grabcon",  cmd_grabcon,  "Grab the main console file descriptors"},
  {"id",       cmd_id,       "Display real and effective user and group ids"},
  {"ifconfig", cmd_ifconfig, "Display network interface configuration"},
  {"ipconfig", cmd_ifconfig, "Display network interface configuration"},
  {"keyb",     cmd_keyb,     "Change keyboard map"},
  {"heapstat", cmd_heapstat, "Display heap statistics"},
  {"help",     cmd_help,     "This help"},
  {"httpget",  cmd_httpget,  "Retrieve file via http"},
  {"jobs",     cmd_ps,       "Display process list"},
  {"kbd",      cmd_kbd,      "Keyboard test"},
  {"kill",     cmd_kill,     "Send signal"},
  {"klog",     cmd_klog,     "Enable/disable kernel log messages"},
  {"load",     cmd_load,     "Load module"},
  {"loglevel", cmd_loglevel, "Set syslog tracing mask and level"},
  {"lookup",   cmd_nslookup, "Lookup hostname or IP address using DNS"},
  {"ln",       cmd_ln,       "Create link to file or directory"},
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
  {"ps",       cmd_ps,       "Display process list"},
  {"read",     cmd_read,     "Read file from disk"},
  {"reboot",   cmd_reboot,   "Reboot computer"},
  {"rm",       cmd_rm,       "Delete file"},
  {"rd",       cmd_rmdir,    "Remove directory"},
  {"rmdir",    cmd_rmdir,    "Remove directory"},
  {"set",      cmd_set,      "Set or display environment variables"},
  {"sleep",    cmd_sleep,    "Sleep for a number of milliseconds"},
  {"sound",    cmd_sound,    "Play sound in speaker"},
  {"shutdown", cmd_shutdown, "Shutdown computer and power off"},
  {"sysinfo",  cmd_sysinfo,  "Display system info"},
  {"test",     cmd_test,     "Dummy command for misc. tests"},
  {"type",     cmd_cat,      "Display file"},
  {"uname",    cmd_uname,    "Print system information"},
  {"umount",   cmd_umount,   "Unmount file system"},
  {"whoami",   cmd_whoami,   "Display logged in user"},
  {"write",    cmd_write,    "Write zero filled file to disk"},
  {NULL, NULL, NULL}
};

static void exec_program(char *args) {
  int rc;

  rc = spawn(P_WAIT, NULL, args, NULL, NULL);
  if (rc < 0) perror("error");
}

static void launch_program(char *args) {
  int h;

  h = spawn(P_NOWAIT, NULL, args, NULL, NULL);
  if (h < 0) {
    perror("error");
  } else {
    printf("[process %d started]\n", h);
    close(h);
  }
}

static int exec_builtin(int argc, char *argv[], int *found) {
  struct command *command;

  for (command = cmdtab; command->cmdname; command++) {
    if (strcmp(argv[0], command->cmdname) == 0) break;
  }

  if (!command->cmdname) {
    if (found) *found = 0;
    return -ENOENT;
  }

  if (found) *found = 1;
  if (!command->proc) return -ENOSYS;
  return command->proc(argc, argv);
}

static int parse_args(char *args, char **argv) {
  char *p;
  int argc;
  char *start;
  char *end;
  char *buf;
  int delim;

  p = args;
  argc = 0;
  while (*p) {
    while (*p == ' ') p++;
    if (!*p) break;

    if (*p == '"' || *p == '\'') {
      delim = *p++;
      start = p;
      while (*p && *p != delim) p++;
      end = p;
      if (*p == delim) p++;
    } else {
      start = p;
      while (*p && *p != ' ') p++;
      end = p;
    }

    if (argv) {
      buf = (char *) malloc(end - start + 1);
      if (!buf) break;
      memcpy(buf, start, end - start);
      buf[end - start] = 0;
      argv[argc] = buf;
    }
    
    argc++;
  }

  return argc;
}

static void free_args(int argc, char **argv) {
  int i;
  
  for (i = 0; i < argc; ++i) free(argv[i]);
  free(argv);
}

static int exec_command(char *cmdline) {
  int argc;
  char **argv;
  int found;
  int rc;

  argc = parse_args(cmdline, NULL);
  if (!argc) return 0;

  argv = (char **) malloc(argc * sizeof(char *));
  parse_args(cmdline, argv);

  rc = exec_builtin(argc, argv, &found);
  if (found) {
    free_args(argc, argv);
    return rc;
  }
  
  if (strcmp(argv[0], "start") == 0) {
    char *args = cmdline;
    while (*args != 0 && *args != ' ') args++;
    while (*args == ' ') args++;
    launch_program(args);
  } else if (strcmp(argv[argc - 1], "&") == 0) {
    char *buf = strdup(cmdline);
    buf[strlen(buf) - 1] = 0;
    launch_program(buf);
    free(buf);
  } else {
    exec_program(cmdline);
  }

  free_args(argc, argv);
  return 0;
}

void shell() {
  char curdir[MAXPATH];
  char cmdline[256];
  char *prompt = get_property(osconfig(), "shell", "prompt", "msh %s$ ");
  int rc;

  while (1) {
    printf(prompt, getcwd(curdir, sizeof curdir));
    rc = readline(cmdline, sizeof cmdline);
    if (rc < 0) {
      if (errno != EINTR) break;
    } else {
      if (stricmp(cmdline, "exit") == 0) break;
      fflush(stdout);
      exec_command(cmdline);
    }
  }
}

int execute_script(char *cmdfile) {
  FILE *f;
  char cmdline[256];
  int echo;

  echo = get_numeric_property(osconfig(), "shell", "echo", 0);

  f = fopen(cmdfile, "r");
  if (!f) {
    perror(cmdfile);
    return -errno;
  }

  while (fgets(cmdline, sizeof cmdline, f)) {
    if (*cmdline == ';' || *cmdline == '#') continue;
    if (echo) fwrite(cmdline, strlen(cmdline), 1, stdout);

    if (strchr(cmdline, '\r')) *strchr(cmdline, '\r') = 0;
    if (strchr(cmdline, '\n')) *strchr(cmdline, '\n') = 0;

    if (exec_command(cmdline) < 0) break;
  }

  fclose(f);
  return 0;
}

int main(int argc, char *argv[]) {
  if (gettib()->proc->term->type == TERM_VT100) setvbuf(stdout, NULL, 0, 8192);

  if (argc > 1) {
    char *cmdline = gettib()->proc->cmdline;
    while (*cmdline != ' ') {
      if (!*cmdline) return 0;
      cmdline++;
    }
    while (*cmdline == ' ') cmdline++;

    return exec_command(cmdline);
  } else {
    shell();
  }

  setbuf(stdout, NULL);

  return 0;
}
