//
// ftpd.c
//
// FTP daemon
//
// Copyright (C) 2002 Michael Ringgaard. All rights reserved.
// Portions Copyright (C) 1995-2000 Trolltech AS. 
// Portions Copyright (C) 2001 Arnt Gulbrandsen.
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
#include <inifile.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <dirent.h>
#include <time.h>
#include <pwd.h>

#define STOPPED 0
#define RUNNING 1

int port;

int port;
int state = STOPPED;
int sock = NOHANDLE;
int logging = 2;
int timeout = 900 * 1000;

struct reply {
  struct reply *next;
  char line[1];
};

struct ftpstate {
  int ctrlsock;
  int datasock;
  int replycode;
  struct reply *firstreply;
  struct reply *lastreply;
  FILE *in;
  FILE *out;
  struct sockaddr_in peer;
  char cmd[MAXPATH + 32];
  char wd[MAXPATH + 1];
  char *renamefrom;
  int uid;
  int epsvall;
  int loggedin;
  int guest;
  int restartat;
  int debug;
  int idletime;
  int passive;
  int dataport;
  int type;
};

int login(struct ftpstate *fs, struct passwd *pw) {
  if (initgroups(pw->pw_name, pw->pw_gid) < 0) return -1;
  if (setgid(pw->pw_gid) < 0) return -1;
  if (setuid(pw->pw_uid) < 0) return -1;

  fs->uid = pw->pw_uid;
  strcpy(fs->wd, pw->pw_dir);

  return 0;
}

int convert(struct ftpstate *fs, char *filename, char *buf) {
  char buffer[2 * MAXPATH];
  char *p;

  if (*filename == '/' || *filename == '\\') {
    strcpy(buffer, filename);
  } else {
    strcpy(buffer, fs->wd);
    strcat(buffer, "/");
    strcat(buffer, filename);
  }

  if (realpath(buffer, buf) == NULL) return -1;

  p = buf;
  while (*p) {
    if (*p == '\\') *p = '/';
    p++;
  }

  return 0;
}

void addreply(struct ftpstate *fs, int code, const char *line, ...) {
  char buf[MAXPATH + 128];
  va_list ap;
  char *s;
  int l;
  struct reply *r;

  if (code) fs->replycode = code;
  va_start(ap, line);
  vsnprintf(buf, MAXPATH + 50, line, ap);
  va_end(ap);

  s = buf;
  while (*s) {
    char *e = s;
    while (*e && *e != '\n') e++;
    l = e - s;

    r = (struct reply *) malloc(sizeof(struct reply) + l);
    if (!r) return;
    memcpy(r->line, s, l);
    r->line[l] = 0;
    r->next = NULL;

    if (fs->lastreply) {
      fs->lastreply->next = r;
    } else {
      fs->firstreply = r;
    }

    fs->lastreply = r;

    s = e;
    if (*s) s++;
  }
}

void error(struct ftpstate *fs, int code, char *msg) {
  int err = errno;
  if (err == 0) {
    syslog(LOG_ERR, "%s", msg);
    addreply(fs, code, "%s", msg);
  } else {
    char *errmsg = strerror(errno);
    syslog(LOG_ERR, "%s: %s", msg, errmsg);
    addreply(fs, code, "%s: %s", msg, errmsg);
  }
}

void doreply(struct ftpstate *fs) {
  struct reply *r = fs->firstreply;
  while (r) {
    struct reply *next = r->next;
    fprintf(fs->out, "%03d %s\r\n", fs->replycode, r->line);
    if (logging > 1) syslog(LOG_DEBUG, "%03d %s", fs->replycode, r->line);
    free(r);
    r = next;
  }

  fs->firstreply = fs->lastreply = NULL;
  fflush(fs->out);
}

int opendata(struct ftpstate *fs) {
  struct sockaddr_in sin;
  int sock;
  int len;

  if (fs->datasock < 0) {
    addreply(fs, 425, "No data connection");
    return -1;
  }

  if (fs->passive) {
    fd_set rs;
    struct timeval tv;

    FD_ZERO(&rs);
    FD_SET(fs->datasock, &rs);
    tv.tv_sec = fs->idletime;
    tv.tv_usec = 0;
    if (select(fs->datasock + 1, &rs, NULL, NULL, &tv) < 0) {
      addreply(fs, 421, "timeout (no connection for %d seconds)", fs->idletime);
      return -1;
    }

    len = sizeof(sin);
    sock = accept(fs->datasock, (struct sockaddr *) &sin, &len);
    if (sock < 0) {
      error(fs, 421, "accept failed");
      close(fs->datasock);
      fs->datasock = -1;
      return -1;
    }

    if (!fs->guest && sin.sin_addr.s_addr != fs->peer.sin_addr.s_addr) {
      addreply(fs, 425, "Connection must originate at %s", inet_ntoa(fs->peer.sin_addr));
      close(sock);
      close(fs->datasock);
      fs->datasock = -1;
      return -1;
    }

    addreply(fs, 150, "Accepted data connection from %s:%d", inet_ntoa(sin.sin_addr), ntohs(sin.sin_port));
  } else {
    sin.sin_addr.s_addr = fs->peer.sin_addr.s_addr;
    sin.sin_port = htons(fs->dataport);
    sin.sin_family = AF_INET;

    if (connect(fs->datasock, (struct sockaddr *) &sin, sizeof(sin)) < 0) {
      addreply(fs, 425, "Could not open data connection to %s port %d: %s", inet_ntoa(sin.sin_addr), fs->dataport, strerror(errno));
      close(fs->datasock);
      fs->datasock = -1;
      return -1;
    }

    sock = fs->datasock;
    fs->datasock = -1;
    addreply(fs, 150, "Connecting to %s:%d", inet_ntoa(sin.sin_addr), fs->dataport);
  }

  return sock;
}

void douser(struct ftpstate *fs, char *username) {
  struct passwd *pw;

  if (fs->loggedin) {
    if (username) {
      if (!fs->guest) {
        addreply(fs, 530, "You're already logged in.");
      } else {
        addreply(fs, 230, "Anonymous user logged in.");
      }
    }
    return;
  }

  if (username && strcmp(username, "ftp") != 0 && strcmp(username, "anonymous") != 0) {
    pw = getpwnam(username);
    if (pw == NULL) {
      addreply(fs, 331, "User %s unknown.", username);
    } else {
      fs->uid = pw->pw_uid;
      addreply(fs, 331, "User %s OK.  Password required.", pw->pw_name);
    }
    
    fs->loggedin = 0;
  } else {
    pw = getpwnam("ftp");
    if (!pw) {
      addreply(fs, 530, "Anonymous access not allowed.");
    } else {
      if (login(fs, pw) < 0) {
        addreply(fs, 530, "Could not login anonymous user.");
      } else {
        addreply(fs, 230, "Anonymous user logged in.");
        fs->loggedin = fs->guest = 1;
        syslog(LOG_INFO, "guest logged in");
      }
    }
  }
}

void dopass(struct ftpstate *fs, char *password) {
  struct passwd *pw;

  if (fs->uid < 0) {
    addreply(fs, 332, "Need account for login.");
  } else if ((pw = getpwuid(fs->uid)) == NULL) {
    addreply(fs, 331, "Unknow user.");
  } else if (strcmp(pw->pw_passwd, crypt(password, pw->pw_passwd)) == 0) {
    if (login(fs, pw) < 0) {
      addreply(fs, 530, "Could not login user.");
    } else {
      fs->loggedin = 1;
      addreply(fs, 230, "OK. Current directory is %s", fs->wd);
      syslog(LOG_INFO, "%s logged in", pw->pw_name);
    }
  } else {
    addreply(fs, 530, "Sorry");
  }
}

void docwd(struct ftpstate *fs, char *dir) {
  char newwd[MAXPATH];
  struct stat st;

  if (convert(fs, dir, newwd) < 0 || stat(newwd, &st) < 0) {
    addreply(fs, 530, "Cannot change directory to %s: %s", dir, strerror(errno));
    return;
  }

  if (!S_ISDIR(st.st_mode)) {
    addreply(fs, 530, "Not a directory");
    return;
  }

  strcpy(fs->wd, newwd);
  addreply(fs, 250, "Changed to %s", fs->wd);
}

void doretr(struct ftpstate *fs, char *name) {
  char filename[MAXPATH];
  int f;
  int sock;
  struct stat st;
  clock_t started;
  clock_t ended;
  int ofs;
  int n;
  char buf[4096];
  double t;
  double speed;

  if (convert(fs, name, filename) < 0) {
    error(fs, 550, name);
    return;
  }

  f = open(filename, O_RDONLY);
  if (f < 0) {
    char buffer[MAXPATH + 40];
    snprintf(buffer, MAXPATH + 39, "Can't open %s", name);
    error(fs, 550, buffer);
    return;
  }

  if (fstat(f, &st)) {
    close(f);
    error(fs, 451, "can't find file size");
    return;
  }

  if (fs->restartat && fs->restartat > st.st_size) {
    addreply(fs, 451, "Restart offset %d is too large for file size %d.\nRestart offset reset to 0.", fs->restartat, st.st_size);
    return;
  }

  if (!S_ISREG(st.st_mode)) {
    close(f);
    addreply(fs, 450, "Not a regular file");
    return;
  }

  sock = opendata(fs);
  if (sock < 0) {
    close(f);
    return;
  }

  if (fs->restartat == st.st_size) {
    close(f);
    close(sock);
    addreply(fs, 226, "Nothing left to download.  Restart offset reset to 0.");
    return;
  }

  //if (fs->type == 1) addreply(fs, 0, "NOTE: ASCII mode requested, but binary mode used");
  //if (st.st_size - fs->restartat > 4096) addreply(fs, 0, "%.1f kbytes to download", (st.st_size - fs->restartat) / 1024.0);

  doreply(fs);

  started = clock();
  ofs = fs->restartat;
  if (ofs != 0) lseek(f, ofs, SEEK_SET);

  while (ofs < st.st_size) {
    n = st.st_size - ofs;
    if (n > sizeof(buf)) n = sizeof(buf);

    n  = read(f, buf, n);
    if (n <= 0) {
      if (n == 0) {
        addreply(fs, 451, "unexpected end of file");
      } else {
        error(fs, 451, "error reading file");
      }

      close(f);
      close(sock);
      return;
    }

    if (send(sock, buf, n, 0) < 0) {
      addreply(fs, 426, "Transfer aborted");
      close(f);
      close(sock);
      return;
    }

    ofs += n;
  }

  ended = clock();

  t = (ended - started) / 1000.0;
  addreply(fs, 226, "File written successfully");

  if (t != 0.0 && st.st_size - fs->restartat > 0) {
    speed = (st.st_size - fs->restartat) / t;
  } else {
    speed = 0.0;
  }

  //addreply(fs, 0, "%.3f seconds (measured by the server), %.2f %sb/s", t, speed > 524288 ? speed / 1048576 : speed / 1024, speed > 524288 ? "M" : "K");

  close(f);
  close(sock);

  if (fs->restartat != 0) {
    fs->restartat = 0;
    addreply(fs, 0, "Restart offset reset to 0.");
  }
}

void dorest(struct ftpstate *fs, char *name) {
  char *endptr;

  fs->restartat = strtoul(name, &endptr, 10);
  if (*endptr) {
    fs->restartat = 0;
    addreply(fs, 501, "RESTART needs numeric parameter.\nRestart offset set to 0.");
  } else {
    syslog(LOG_NOTICE, "info: restart %d", fs->restartat);
    addreply(fs, 350, "Restarting at %ld. Send STOR or RETR to initiate transfer.", fs->restartat);
  }
}

void dodele(struct ftpstate *fs, char *name) {
  char filename[MAXPATH];

  if (convert(fs, name, filename) < 0) {
    error(fs, 550, name);
    return;
  }

  if (fs->guest) {
    addreply(fs, 550, "Anonymous users can not delete files.");
  } else if (unlink(filename) < 0) {
    addreply(fs, 550, "Could not delete %s: %s", name, strerror(errno));
  } else {
    addreply(fs, 250, "Deleted %s", name);
  }
}

void dostor(struct ftpstate *fs, char *name) {
  char filename[MAXPATH];
  struct stat st;
  int f;
  int sock;
  char buf[4096];
  int n;

  if (convert(fs, name, filename) < 0) {
    error(fs, 550, name);
    return;
  }

  if (fs->type < 1) {
    addreply(fs, 503, "Only ASCII and binary modes are supported");
    return;
  }

  if (stat(filename, &st) >= 0) {
    if (fs->guest) {
      addreply(fs, 553, "Anonymous users may not overwrite existing files");
      return;
    }
  } else if (errno != ENOENT) {
    error(fs, 553, "Can't check for file presence");
    return;
  }

  f = open(filename, O_CREAT | O_TRUNC | O_WRONLY, 0600);
  if (f < 0) {
    error(fs, 553, "Can't open file");
    return;
  }

  if (fs->restartat && lseek(f, fs->restartat, SEEK_SET) < 0) {
    close(f);
    error(fs, 451, "can't seek");
    return;
  }

  sock = opendata(fs);
  if (sock < 0) {
    close(f);
    return;
  }
  doreply(fs);

  while (1) {
    n = recv(sock, buf, sizeof(buf), 0);
    if (n < 0) {
      error(fs, 451, "Error during read from data connection");
      close(f);
      close(sock);
      addreply(fs, 451, "%s %s", name, unlink(filename) ? "partially uploaded" : "removed");
      return;
    }

    if (n == 0) break;

    if (write(f, buf, n) < 0) {
      error(fs, 450, "Error during write to file");
      close(f);
      close(sock);
      addreply(fs, 450, "%s %s", name, unlink(filename) ? "partially uploaded" : "removed");
      return;
    }
  }

  fchmod(f, 0644);
  addreply(fs, 226, "File written successfully");

  close(f);
  close(sock);

  if (fs->restartat) {
    fs->restartat = 0;
    addreply(fs, 0, "Restart offset reset to 0.");
  }
}

void domkd(struct ftpstate *fs, char *name) {
  char filename[MAXPATH];

  if (convert(fs, name, filename) < 0) {
    error(fs, 550, name);
    return;
  }

  if (fs->guest) {
    addreply(fs, 550, "Sorry, anonymous users are not allowed to make directories.");
  } else if (mkdir(filename, 0755) < 0) {
    error(fs, 550, "Can't create directory");
  } else {
    addreply(fs, 257, "MKD command successful.");
  }
}

void dormd(struct ftpstate *fs, char *name) {
  char filename[MAXPATH];

  if (convert(fs, name, filename) < 0) {
    error(fs, 550, name);
    return;
  }

  if (fs->guest) {
    addreply(fs, 550, "Sorry, anonymous users are not allowed to remove directories.");
  } else if (rmdir(filename) < 0) {
    error(fs, 550, "Can't remove directory");
  } else {
    addreply(fs, 250, "RMD command successful.");
  }
}

void domdtm(struct ftpstate *fs, char *name) {
  char filename[MAXPATH];
  struct stat st;
  struct tm *t;

  if (convert(fs, name, filename) < 0) {
    error(fs, 550, name);
    return;
  }

  if (stat(filename, &st) < 0) {
    error(fs, 550, "Unable to stat()");
  } else if (!S_ISREG(st.st_mode)) {
    addreply(fs, 550, "Not a regular file");
  } else {
    t = gmtime(& st.st_mtime);
    if (!t) {
      addreply(fs, 550, "gmtime() returned NULL");
    } else {
      addreply(fs, 213, "%04d%02d%02d%02d%02d%02d", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
    }
  }
}

void dosize(struct ftpstate *fs, char *name) {
  char filename[MAXPATH];
  struct stat st;

  if (convert(fs, name, filename) < 0) {
    error(fs, 550, name);
    return;
  }

  if (stat(filename, &st ) < 0) {
    addreply(fs, 550, "Unable to stat()");
  } else if (!S_ISREG(st.st_mode)) {
    addreply(fs, 550, "Not a regular file");
  } else {
    addreply(fs, 213, "%ld", st.st_size);
  }
}

void doport(struct ftpstate *fs, unsigned int ip, unsigned int port) {
  struct sockaddr_in sin;

  if (fs->datasock != -1) {
    close(fs->datasock);
    fs->datasock = -1;
  }

  fs->datasock = socket(AF_INET, SOCK_STREAM, 0);
  if (fs->datasock < 0) {
    error(fs, 425, "Can't make data socket");
    return;
  }

  //int on = 1;
  //if (setsockopt(fs->datasock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) {
  //  error(fs, 421, "setsockopt");
  //  return;
  //}

  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = INADDR_ANY;
  sin.sin_port = htons(20); // FTP data connection port
  if (bind(fs->datasock, (struct sockaddr *) &sin, sizeof(sin)) < 0) {
    error(fs, 220, "bind");
    close(fs->datasock);
    fs->datasock = -1;
    return;
  }

  if (fs->debug) addreply(fs, 0, "My data connection endpoint is %s:%d", inet_ntoa(sin.sin_addr), ntohs(sin.sin_port));

  fs->dataport = port;

  if (htonl(ip) != fs->peer.sin_addr.s_addr) {
    addreply(fs, 425, "Will not open connection to %d.%d.%d.%d (only to %s)",
             (ip >> 24) & 255, (ip >> 16) & 255, (ip >> 8) & 255, ip & 255,
             inet_ntoa(fs->peer.sin_addr));
    close(fs->datasock);
    fs->datasock = -1;
    return;
  }

  fs->passive = 0;

  addreply(fs, 200, "PORT command successful");
}

void dopasv(struct ftpstate *fs, int useepsv) {
  unsigned int a;
  unsigned int p;
  struct sockaddr_in sin;
  unsigned int len;

  if (fs->datasock != -1) {
    close(fs->datasock);
    fs->datasock = -1;
  }

  len = sizeof(sin);
  if (getsockname(fs->ctrlsock, (struct sockaddr *) &sin, &len) < 0) {
    error(fs, 425, "Can't getsockname");
    return;
  }

  fs->datasock = socket(AF_INET, SOCK_STREAM, 0);
  if (fs->datasock < 0) {
    error(fs, 425, "Can't open passive connection");
    return;
  }

  sin.sin_port = 0;
  if (bind(fs->datasock, (struct sockaddr *) &sin, sizeof(sin)) < 0) {
    error(fs, 425, "Can't bind to socket");
    return;
  }

  len = sizeof(sin);
  if (getsockname(fs->datasock, (struct sockaddr *) &sin, &len) < 0) {
    error(fs, 425, "Can't getsockname");
    return;
  }

  listen(fs->datasock, 1);

  a = ntohl(sin.sin_addr.s_addr);
  p = ntohs(sin.sin_port);
  if (useepsv) {
    addreply(fs, 229, "Extended Passive mode OK (|||%d|)", p);
  } else {
    addreply(fs, 227, "Passive mode OK (%d,%d,%d,%d,%d,%d)", (a >> 24) & 255, (a >> 16) & 255, (a >> 8) & 255, a & 255, (p >> 8) & 255, p & 255);
  }
  
  fs->passive = 1;
}

void domode(struct ftpstate *fs, char *arg) {
  if (!arg || !*arg) {
    addreply(fs, 500, "No arguments\nNot that it matters, only MODE S is supported");
  } else if (strcmp(arg, "S") != 0) {
    addreply(fs, 504, "MODE %s is not supported\nOnly S(tream) is supported", arg);
  } else {
    addreply(fs, 200, "S OK");
  }
}

void dostru(struct ftpstate *fs, char *arg) {
  if (!arg || !*arg) {
    addreply(fs, 500, "No arguments\nNot that it matters, only STRU F is supported");
  } else if (strcmp(arg, "F") != 0) {
    addreply(fs, 504, "STRU %s is not supported\nOnly F(ile) is supported", arg);
  } else {
    addreply(fs, 200, "F OK");
  }
}

void dotype(struct ftpstate *fs, char *arg) {
  fs->replycode = 200;

  if (!arg || !*arg) {
    addreply(fs, 501, "TYPE needs an argument\nOnly A(scii), I(mage) and L(ocal) are supported");
  } else if (tolower(*arg) == 'a') {
    fs->type = 1;
  } else if (tolower(*arg) == 'i') {
    fs->type = 2;
  } else if (tolower(*arg) == 'l') {
    if (arg[1] == '8') {
      fs->type = 2;
    } else if (isdigit(arg[1])) {
      addreply(fs, 504, "Only 8-bit bytes are supported");
    } else {
      addreply(fs, 0, "Byte size not specified");
      fs->type = 2;
    }
  } else {
    addreply(fs, 504, "Unknown TYPE: %s", arg);
  }

  addreply(fs, 0, "TYPE is now %s", (fs->type > 1) ? "8-bit binary" : "ASCII");
}

void dornfr(struct ftpstate *fs, char *name) {
  char filename[MAXPATH];
  struct stat st;

  if (convert(fs, name, filename) < 0) {
    error(fs, 550, name);
    return;
  }

  if (fs->guest) {
    addreply(fs, 550, "Sorry, anonymous users are not allowed to rename files.");
  } else if (stat(filename, &st) < 0) {
    addreply(fs, 550, "File does not exist");
  } else {
    if (fs->renamefrom) {
      addreply(fs, 0, "Aborting previous rename operation.");
      free(fs->renamefrom);
    }
    fs->renamefrom = strdup(filename);
    addreply(fs, 350, "RNFR accepted - file exists, ready for destination.");
  }
}

void dornto(struct ftpstate *fs, char *name) {
  char filename[MAXPATH];
  struct stat st;

  if (convert(fs, name, filename) < 0) {
    error(fs, 550, name);
    return;
  }

  if (fs->guest) {
    addreply(fs, 550, "Sorry, anonymous users are not allowed to rename files.");
  } else if (stat(filename, &st) == 0) {
    addreply(fs, 550, "RENAME Failed - destination file already exists.");
  } else if (!fs->renamefrom) {
    addreply(fs, 503, "Need RNFR before RNTO");
  } else if (rename(fs->renamefrom, filename) < 0) {
    addreply(fs, 550, "Rename failed: %s", strerror(errno));
  } else {
    addreply(fs, 250, "File renamed.");
  }

  if (fs->renamefrom) free(fs->renamefrom);
  fs->renamefrom = NULL;
}

void donlist(struct ftpstate *fs, char *arg) {
  static char *months[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

  char dirname[MAXPATH];
  DIR *dir;
  struct dirent *de;
  int sock;
  int matches;
  int opt_l = 0;
  time_t now = time(NULL);

  while (isspace(*arg)) arg++;

  while (*arg == '-') {
    while (isalnum(*++arg)) {
      switch (*arg) {
        case 'l':
        case 'a':
          opt_l = 1;
          break;
      }
    }

    while (isspace(*arg)) arg++;
  }

  if (convert(fs, arg, dirname) < 0) {
    error(fs, 550, arg);
    return;
  }

  dir = opendir(dirname);
  if (!dir) {
    error(fs, 550, dirname);
    return;
  }

  sock = opendata(fs);
  if (sock < 0) {
    closedir(dir);
    return;
  }
  doreply(fs);

  matches = 0;
  while ((de = readdir(dir)) != NULL) {
    char buf[MAXPATH + 128];

    if (opt_l) {
      char fn[MAXPATH * 2 + 2];
      struct stat st;
      char perm[11];
      char timestr[6];
      struct passwd *pwd;
      struct group *grp;
      struct tm *tm;

      strcpy(fn, dirname);
      strcat(fn, "/");
      strcat(fn, de->d_name);

      if (stat(fn, &st) < 0) continue;
      if ((tm = localtime(&st.st_mtime)) == NULL) continue;

      strcpy(perm, " ---------");
      switch (st.st_mode & S_IFMT) {
        case S_IFREG: perm[0] = '-'; break;
        case S_IFLNK: perm[0] = 'l'; break;
        case S_IFDIR: perm[0] = 'd'; break;
        case S_IFBLK: perm[0] = 'b'; break;
        case S_IFCHR: perm[0] = 'c'; break;
        case S_IFPKT: perm[0] = 'p'; break;
      }

      if (st.st_mode & 0400) perm[1] = 'r';
      if (st.st_mode & 0200) perm[2] = 'w';
      if (st.st_mode & 0100) perm[3] = 'x';
      if (st.st_mode & 0040) perm[4] = 'r';
      if (st.st_mode & 0020) perm[5] = 'w';
      if (st.st_mode & 0010) perm[6] = 'x';
      if (st.st_mode & 0004) perm[7] = 'r';
      if (st.st_mode & 0002) perm[8] = 'w';
      if (st.st_mode & 0001) perm[9] = 'x';

      pwd = getpwuid(st.st_uid);
      grp = getgrgid(st.st_gid);
      if (!pwd || !grp) continue;

      if (now - st.st_mtime > 180 * 24 * 60 * 60) {
        snprintf(timestr, 6, "%5d", tm->tm_year + 1900);
      } else {
        snprintf(timestr, 6, "%02d:%02d", tm->tm_hour, tm->tm_min);
      }

      snprintf(buf, sizeof(buf), "%s %3d %s %s %7d %s %2d %s %s\r\n",
               perm, st.st_nlink, pwd->pw_name, grp->gr_name,
               st.st_size, months[tm->tm_mon], tm->tm_mday, timestr, de->d_name);
    } else {
      sprintf(buf, "%s\r\n", de->d_name);
    }
    
    send(sock, buf, strlen(buf), 0);
    matches++;
  }

  addreply(fs, 226, "%d matches total", matches);

  close(sock);
  closedir(dir);
}

int docmd(struct ftpstate *fs) {
  char *arg;
  char *cmd;
  int cmdsize;
  int n = 0;

  if (!fgets(fs->cmd, sizeof(fs->cmd), fs->in)) {
    if (errno == ETIMEOUT) addreply(fs, 421, "Timeout (%d seconds).", fs->idletime / 1000);
    return -1;
  }
  cmd = fs->cmd;
  cmdsize = strlen(cmd);

  if (fs->debug) addreply(fs, 0, "%s", cmd);

  n = 0;
  while (isalpha(cmd[n]) && n < cmdsize) {
    cmd[n] = tolower(cmd[n]);
    n++;
  }

  if (!n) {
    addreply(fs, 221, "Goodbye.");
    return 0;
  }

  while (isspace(cmd[n]) && n < cmdsize) cmd[n++] = '\0';
  arg = cmd + n;

  while (cmd[n] && n < cmdsize) n++;
  n--;

  while (isspace(cmd[n])) cmd[n--] = '\0';

  if (logging > 0) syslog(LOG_DEBUG, "CMD %s %s", cmd, strcmp(cmd, "pass") ? arg : "<password>");

  if (strlen(cmd) > 10) {
    addreply(fs, 500, "Unknown command.");
  } else if (strlen(arg) >= MAXPATH) { // ">=" on purpose.
    addreply(fs, 501, "Cannot handle %d-character file names", strlen(arg));
  } else if (strcmp(cmd, "user") == 0) {
    douser(fs, arg);
  } else if (strcmp(cmd, "pass") == 0) {
    dopass(fs, arg);
  } else if (strcmp(cmd, "quit") == 0) {
     addreply(fs, 221, "Goodbye");
     return 0;
  } else if (strcmp(cmd, "noop") == 0) {
    addreply(fs, 200, "NOOP command successful");
  } else if (strcmp(cmd, "syst") == 0) {
    addreply(fs, 215, "UNIX Type: L8");
  } else if (strcmp(cmd, "feat") == 0) {
    addreply(fs, 500, "Unsupported command");
  } else if (strcmp(cmd, "port") == 0|| strcmp(cmd, "eprt") == 0) {
    // Don't auto-login for PORT or PASV, but do auto-login
    // for the command which uses the data connection
    unsigned int a1, a2, a3, a4, p1, p2;

    if (fs->epsvall) {
      addreply(fs, 501, "Cannot use PORT/EPRT after EPSV ALL");
    } else if (cmd[0] == 'e' && strncmp(arg, "|2|", 3) == 0) {
      addreply(fs, 522, "IPv6 not supported, use IPv4 (1)");
    } else if (cmd[0] == 'e' && 
                sscanf(arg, "|1|%u.%u.%u.%u|%u|", &a1, &a2, &a3, &a4, &p1) == 5  &&
                a1 < 256 && a2 < 256 && a3 < 256 && a4 < 256 && p1 < 65536) {
      doport(fs, (a1 << 24) + (a2 << 16) + (a3 << 8) + a4, p1);
    } else if (cmd[0] == 'p' &&
               sscanf(arg, "%u,%u,%u,%u,%u,%u", &a1, &a2, &a3, &a4, &p1, &p2) == 6 &&
               a1 < 256 && a2 < 256 && a3 < 256 && a4 < 256 && p1 < 256 && p2 < 256) {
      doport(fs, (a1 << 24) + (a2 << 16) + (a3 << 8) + a4, ((p1 << 8) + p2));
    } else {
      addreply(fs, 501, "Syntax error.");
    }
  } else if (strcmp(cmd, "pasv") == 0) {
    dopasv(fs, 0);
  } else if (strcmp(cmd, "epsv") == 0) {
    if (strcmp(arg, "all") == 0) {
      addreply(fs, 220, "OK; will reject non-EPSV data connections");
      fs->epsvall++;
    } else if (strcmp(arg, "2") == 0) {
      addreply(fs, 522, "IPv6 not supported, use IPv4 (1)");
    } else if (strlen(arg) == 0 || strcmp(arg, "1") == 0) {
      dopasv(fs, 1);
    }
  } else if (strcmp(cmd, "pwd") == 0 || strcmp(cmd, "xpwd") == 0) {
    if (fs->loggedin) {
      addreply(fs, 257, "\"%s\"", fs->wd);
    } else {
      addreply(fs, 550, "Not logged in");
    }
  } else if (strcmp(cmd, "auth") == 0) {
    // RFC 2228 Page 5 Authentication/Security mechanism (AUTH)
    addreply(fs, 502, "Security extensions not implemented");
  } else {
    // From this point, all commands trigger an automatic login
    douser(fs, NULL);

    if (strcmp(cmd, "cwd") == 0) {
      docwd(fs, arg);
    } else if (strcmp(cmd, "cdup") == 0) {
      docwd(fs, "..");
    } else if (strcmp(cmd, "retr") == 0) {
      if (arg && *arg) {
        doretr(fs, arg);
      } else {
        addreply(fs, 501, "No file name");
      }
    } else if (strcmp(cmd, "rest") == 0) {
      if (arg && *arg) {
        dorest(fs, arg);
      } else {
        addreply(fs, 501, "No restart point");
      }
    } else if (strcmp(cmd, "dele") == 0) {
      if (arg && *arg) {
        dodele(fs, arg);
      } else {
        addreply(fs, 501, "No file name");
      }
    } else if (strcmp(cmd, "stor") == 0) {
      if (arg && *arg) {
        dostor(fs, arg);
      } else {
        addreply(fs, 501, "No file name");
      }
    } else if (strcmp(cmd, "mkd") == 0 || strcmp(cmd, "xmkd") == 0) {
      if (arg && *arg) {
        domkd(fs, arg);
      } else {
        addreply(fs, 501, "No directory name");
      }
    } else if (strcmp(cmd, "rmd") == 0 || strcmp(cmd, "xrmd") == 0) {
      if (arg && *arg) {
        dormd(fs, arg);
      } else {
        addreply(fs, 550, "No directory name");
      }
    } else if (strcmp(cmd, "list") == 0 || strcmp(cmd, "nlst") == 0) {
      donlist(fs, (arg && *arg) ? arg : "-l");
    } else if (strcmp(cmd, "type") == 0) {
      dotype(fs, arg);
    } else if (strcmp(cmd, "mode") == 0) {
      domode(fs, arg);
    } else if (strcmp(cmd, "stru") == 0) {
      dostru(fs, arg);
    } else if (strcmp(cmd, "abor") == 0) {
      addreply(fs, 226, "ABOR succeeded.");
    } else if (strcmp(cmd, "site") == 0) {
      char *sitearg;

      sitearg = arg;
      while (sitearg && *sitearg && !isspace(*sitearg)) sitearg++;
      if (sitearg) *sitearg++ = '\0';

      if (strcmp(arg, "idle") == 0) {
        if (!*sitearg) {
          addreply(fs, 501, "SITE IDLE: need argument");
        } else {
          unsigned long int i = 0;

          i = strtoul(sitearg, &sitearg, 10);
          if (sitearg && *sitearg) {
            addreply(fs, 501, "Garbage (%s) after value (%u)", sitearg, i);
          } else {
            if (i > 7200) i = 7200;
            if (i < 10) i = 10;
            fs->idletime = i * 1000;
            setsockopt(fs->ctrlsock, SOL_SOCKET, SO_RCVTIMEO, (char *) &fs->idletime, 4);
            setsockopt(fs->ctrlsock, SOL_SOCKET, SO_SNDTIMEO, (char *) &fs->idletime, 4);

            addreply(fs, 200, "Idle time set to %u seconds", fs->idletime / 1000);
          }
        }
      } else if (arg && *arg) {
        addreply(fs, 500, "SITE %s unknown", arg);
      } else {
        addreply(fs, 500, "SITE: argument needed");
      }
    } else if (strcmp(cmd, "xdbg") == 0) {
      fs->debug++;
      addreply(fs, 200, "XDBG command succeeded, debug level is now %d.", fs->debug);
    } else if (strcmp(cmd, "mdtm") == 0) {
      domdtm(fs, (arg && *arg) ? arg :  "");
    } else if (strcmp(cmd, "size") == 0) {
      dosize(fs, (arg && *arg) ? arg :  "");
    } else if (strcmp(cmd, "rnfr") == 0) {
      if (arg && *arg) {
        dornfr(fs, arg);
      } else {
        addreply(fs, 550, "No file name given.");
      }
    } else if (strcmp(cmd, "rnto") == 0) {
      if (arg && *arg) {
        dornto(fs, arg);
      } else {
        addreply(fs, 550, "No file name given.");
      }
    } else if (strcmp(cmd, "rest") == 0) {
      fs->restartat = 0;
    } else {
      addreply(fs, 500, "Unknown command.");
    }
  }

  return 1;
}

void __stdcall ftp_task(void *arg) {
  struct ftpstate ftpstate;
  struct ftpstate *fs = &ftpstate;
  struct process *proc = gettib()->proc;
  int s = (int) arg;
  int len;

  // Initialize client state
  memset(fs, 0, sizeof(struct ftpstate));
  fs->ctrlsock = s;
  fs->datasock = -1;
  fs->uid = -1;

  len = sizeof(fs->peer);
  getpeername(s, (struct sockaddr *) &fs->peer, &len);

  // Set process identifer
  if (!proc->ident && !proc->cmdline) {
    struct sockaddr_in sin;
    int sinlen = sizeof sin;

    getpeername(s, (struct sockaddr *) &sin, &sinlen);
    proc->ident = strdup("ftp");
    proc->cmdline = strdup(inet_ntoa(sin.sin_addr));
  }

  // Set idle timeout for connection
  setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char *) &timeout, sizeof(timeout));
  setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, (char *) &timeout, sizeof(timeout));
  fs->idletime = timeout;

  // Open data input and out file descriptors
  fs->in = fdopen(s, "r");
  if (!fs->in) {
    close(s);
    return;
  }

  fs->out = fdopen(dup(s), "w");
  if (!fs->out) {
    fclose(fs->in);
    return;
  }

  // Handle commands
  addreply(fs, 220, "FTP server ready");
  while (1) {
    doreply(fs);
    if (docmd(fs) <= 0) break;
  }
  doreply(fs);

  // Close connection
  if (fs->renamefrom) free(fs->renamefrom);
  if (fs->in) fclose(fs->in);
  if (fs->out) fclose(fs->out);
  if (fs->datasock != -1) close(fs->datasock);
}

int main(int argc, char *argv[]) {
  int s;
  int rc;
  struct sockaddr_in sin;
  int hthread;

  if (argc == 2) {
    if (strcmp(argv[1], "start") == 0) {
      char path[MAXPATH];
      
      getmodpath(NULL, path, MAXPATH);
      hthread = spawn(P_NOWAIT | P_DETACH, path, "", NULL, NULL);
      if (hthread < 0) syslog(LOG_ERR, "error %d (%s) in spawn", errno, strerror(errno));
      close(hthread);
      return 0;
    } else if (strcmp(argv[1], "stop") == 0) {
      state = STOPPED;
      close(sock);
      return 0;
    }
  }

  port = get_numeric_property(osconfig(), "ftpd", "port", getservbyname("ftp", "tcp")->s_port);

  sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0) {
    syslog(LOG_ERR, "error %d (%s) in socket", errno, strerror(errno));
    return 1;
  }

  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = INADDR_ANY;
  sin.sin_port = htons(port);
  rc = bind(sock, (struct sockaddr *) &sin, sizeof sin);
  if (rc < 0) {
    syslog(LOG_ERR, "error %d (%s) in bind", errno, strerror(errno));
    return 1;
  }

  rc = listen(sock, 5);
  if (rc < 0) {
    syslog(LOG_ERR, "error %d (%s) in listen", errno, strerror(errno));
    return 1;
  }

  syslog(LOG_INFO, "ftpd started on port %d", port);
  state = RUNNING;
  while (1) {
    struct sockaddr_in sin;

    s = accept(sock, (struct sockaddr *) &sin, NULL);
    if (state == STOPPED) break;
    if (s < 0) {
      syslog(LOG_ERR, "error %d (%s) in accept", errno, strerror(errno));
      return 1;
    }

    syslog(LOG_INFO, "FTP client connected from %a", &sin.sin_addr);

    hthread = beginthread(ftp_task, 0, (void *) s, CREATE_NEW_PROCESS | CREATE_DETACHED, "ftp", NULL);
    close(hthread);
  }

  syslog(LOG_INFO, "ftpd stopped");
}
