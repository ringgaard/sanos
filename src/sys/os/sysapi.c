//
// sysapi.h
//
// Operating System API
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
#include <string.h>
#include <os/syscall.h>
#include <os/cpu.h>

int sprintf(char *buf, const char *fmt, ...);

__declspec(naked) int syscall(int syscallno, void *params) {
  __asm {
    push  ebp
    mov   ebp, esp

    mov   eax, 8[ebp]
    mov   edx, 12[ebp]
    mov   ecx, offset sys_return

    sysenter

sys_return:
    pop   ebp
    ret
  }
}

__declspec(naked) int syscall_int48(int syscallno, void *params) {
  __asm {
    push  ebp
    mov   ebp, esp

    mov   eax, 8[ebp]
    mov   edx, 12[ebp]

    int   48

    leave
    ret
  }
}

void init_syscall() {
  // If the processor does not support sysenter patch the 
  // syscall routine with a jump to syscall_int48

  if (!getpeb()->fast_syscalls_supported) {
    // Inject a 'JMP syscall_int48' at the entry of syscall
    char *sc = (char *) syscall;
    char *sc48 = (char *) syscall_int48;
    sc[0] = 0xEB;
    sc[1] = (sc48 - sc - 2);
  }
}

int mkfs(const char *devname, const char *type, const char *opts) {
  return syscall(SYSCALL_MKFS, (void *) &devname);
}

/*static*/ int _mount(const char *type, const char *mntto, const char *mntfrom, const char *opts) {
  return syscall(SYSCALL_MOUNT, (void *) &type);
}

int mount(const char *type, const char *mntto, const char *mntfrom, const char *opts) {
  if (type && strcmp(type, "smbfs") == 0) {
    char addr[256];
    char *p, *q;
    char *newopts;
    struct hostent *hp;
    struct in_addr ipaddr;
    int rc;

    if (mntfrom[0] != '/' && mntfrom[1] != '/') {
      errno = EINVAL;
      return -1;
    }
    p = (char *) mntfrom + 2;
    q = strchr(p, '/');
    if (!q) {
      errno = EINVAL;
      return -1;
    }
    if (q - p > sizeof(addr) - 1) {
      errno = EINVAL;
      return -1;
    }
    memcpy(addr, p, q - p);
    addr[q - p] = 0;
    hp = gethostbyname(addr);
    if (!hp) return -1;
    memcpy(&ipaddr, hp->h_addr_list[0], hp->h_length);
    newopts = malloc((opts ? strlen(opts) : 0) + 32);
    if (!newopts) return -1;
    if (opts) {
      sprintf(newopts, "%s,addr=0x%08X", opts, ipaddr.s_addr);
    } else {
      sprintf(newopts, "addr=0x%08X", ipaddr.s_addr);
    }

    rc = _mount(type, mntto, mntfrom, newopts);
    
    free(newopts);
    return rc;
  } else {
    return _mount(type, mntto, mntfrom, opts);
  }
}

int umount(const char *path) {
  return syscall(SYSCALL_UMOUNT, (void *) &path);
}

int getfsstat(struct statfs *buf, size_t size) {
  return syscall(SYSCALL_GETFSSTAT, (void *) &buf);
}

int fstatfs(handle_t f, struct statfs *buf) {
  return syscall(SYSCALL_FSTATFS, (void *) &f);
}

int statfs(const char *name, struct statfs *buf) {
  return syscall(SYSCALL_STATFS, (void *) &name);
}

handle_t open(const char *name, int flags, ...) {
  return syscall(SYSCALL_OPEN, (void *) &name);
}

int close(handle_t h) {
  return syscall(SYSCALL_CLOSE, &h);
}

int fsync(handle_t f) {
  return syscall(SYSCALL_FSYNC, &f);
}

int read(handle_t f, void *data, size_t size) {
  return syscall(SYSCALL_READ, &f);
}

int write(handle_t f, const void *data, size_t size) {
  return syscall(SYSCALL_WRITE, &f);
}

int pread(handle_t f, void *data, size_t size, off64_t offset) {
  return syscall(SYSCALL_PREAD, &f);
}

int pwrite(handle_t f, const void *data, size_t size, off64_t offset) {
  return syscall(SYSCALL_PWRITE, &f);
}

int ioctl(handle_t f, int cmd, const void *data, size_t size) {
  return syscall(SYSCALL_IOCTL, &f);
}

int readv(handle_t f, const struct iovec *iov, int count) {
  return syscall(SYSCALL_READV, &f);
}

int writev(handle_t f, const struct iovec *iov, int count) {
  return syscall(SYSCALL_WRITEV, &f);
}

/*static*/ loff_t _tell(handle_t f, off64_t *retval) {
  return syscall(SYSCALL_TELL, &f);
}

loff_t tell(handle_t f) {
  return _tell(f, NULL);
}

off64_t tell64(handle_t f) {
  off64_t rc;

  _tell(f, &rc);
  return rc;
}

/*static*/ int _lseek(handle_t f, off64_t offset, int origin, off64_t *retval) {
  return syscall(SYSCALL_LSEEK, &f);
}

off64_t lseek64(handle_t f, off64_t offset, int origin) {
  off64_t rc;

  _lseek(f, offset, origin, &rc);
  return rc;
}

loff_t lseek(handle_t f, loff_t offset, int origin) {
  return _lseek(f, offset, origin, NULL);
}

int ftruncate64(handle_t f, off64_t size) {
  return syscall(SYSCALL_FTRUNCATE, &f);
}

int ftruncate(handle_t f, loff_t size) {
  return ftruncate64(f, size);
}

int futime(handle_t f, struct utimbuf *times) {
  return syscall(SYSCALL_FUTIME, &f);
}

int utime(const char *name, struct utimbuf *times) {
  return syscall(SYSCALL_UTIME, (void *) &name);
}

int fstat64(handle_t f, struct stat64 *buffer) {
  return syscall(SYSCALL_FSTAT, &f);
}

int fstat(handle_t f, struct stat *buffer) {
  if (buffer) {
    struct stat64 buf64;
    int rc;

    rc = fstat64(f, &buf64);
    if (rc < 0) return rc;

    buffer->st_dev = buf64.st_dev;
    buffer->st_ino = buf64.st_ino;
    buffer->st_mode = buf64.st_mode;
    buffer->st_nlink = buf64.st_nlink;
    buffer->st_uid = buf64.st_uid;
    buffer->st_gid = buf64.st_gid;
    buffer->st_rdev = buf64.st_rdev;
    buffer->st_size = buf64.st_size > 0x7FFFFFFF ? 0x7FFFFFFF : (loff_t) buf64.st_size;
    buffer->st_atime = buf64.st_atime;
    buffer->st_mtime = buf64.st_mtime;
    buffer->st_ctime = buf64.st_ctime;

    return rc;
  } else {
    return fstat64(f, NULL);
  }
}

int stat64(const char *name, struct stat64 *buffer) {
  return syscall(SYSCALL_STAT, (void *) &name);
}

int stat(const char *name, struct stat *buffer) {
  if (buffer) {
    struct stat64 buf64;
    int rc;

    rc = stat64(name, &buf64);
    if (rc < 0) return rc;

    buffer->st_dev = buf64.st_dev;
    buffer->st_ino = buf64.st_ino;
    buffer->st_mode = buf64.st_mode;
    buffer->st_nlink = buf64.st_nlink;
    buffer->st_uid = buf64.st_uid;
    buffer->st_gid = buf64.st_gid;
    buffer->st_rdev = buf64.st_rdev;
    buffer->st_size = buf64.st_size > 0x7FFFFFFF ? 0x7FFFFFFF : (loff_t) buf64.st_size;
    buffer->st_atime = buf64.st_atime;
    buffer->st_mtime = buf64.st_mtime;
    buffer->st_ctime = buf64.st_ctime;

    return rc;
  } else {
    return stat64(name, NULL);
  }
}

int lstat(const char *name, struct stat *buffer) {
  return stat(name, buffer);
}

int lstat64(const char *name, struct stat64 *buffer) {
  return stat64(name, buffer);
}

int chmod(const char *name, int mode) {
  return syscall(SYSCALL_CHMOD, (void *) &name);
}

int fchmod(handle_t f, int mode) {
  return syscall(SYSCALL_FCHMOD, (void *) &f);
}

int mkdir(const char *name, int mode) {
  return syscall(SYSCALL_MKDIR, (void *) &name);
}

int rmdir(const char *name) {
  return syscall(SYSCALL_RMDIR, (void *) &name);
}

int rename(const char *oldname, const char *newname) {
  return syscall(SYSCALL_RENAME, (void *) &oldname);
}

int link(const char *oldname, const char *newname) {
  return syscall(SYSCALL_LINK, (void *) &oldname);
}

int unlink(const char *name) {
  return syscall(SYSCALL_UNLINK, (void *) &name);
}

handle_t _opendir(const char *name) {
  return syscall(SYSCALL_OPENDIR, (void *) &name);
}

int _readdir(handle_t f, struct direntry *dirp, int count) {
  return syscall(SYSCALL_READDIR, &f);
}

void *vmalloc(void *addr, unsigned long size, int type, int protect, unsigned long tag) {
  return (void *) syscall(SYSCALL_VMALLOC, &addr);
}

int vmfree(void *addr, unsigned long size, int type) {
  return syscall(SYSCALL_VMFREE, &addr);
}

void *vmrealloc(void *addr, unsigned long oldsize, unsigned long newsize, int type, int protect, unsigned long tag) {
  return (void *) syscall(SYSCALL_VMREALLOC, &addr);
}

int vmprotect(void *addr, unsigned long size, int protect) {
  return syscall(SYSCALL_VMPROTECT, &addr);
}

int vmlock(void *addr, unsigned long size) {
  return syscall(SYSCALL_VMLOCK, &addr);
}

int vmunlock(void *addr, unsigned long size) {
  return syscall(SYSCALL_VMUNLOCK, &addr);
}

void *vmmap(void *addr, unsigned long size, int protect, handle_t h, off64_t offset) {
  return (void *) syscall(SYSCALL_VMMAP, &addr);
}

int vmsync(void *addr, unsigned long size) {
  return syscall(SYSCALL_VMSYNC, &addr);
}

int waitone(handle_t h, int timeout) {
  return syscall(SYSCALL_WAITONE, &h);
}

int waitall(handle_t *h, int count, int timeout) {
  return syscall(SYSCALL_WAITALL, &h);
}

int waitany(handle_t *h, int count, int timeout) {
  return syscall(SYSCALL_WAITANY, &h);
}

handle_t mkevent(int manual_reset, int initial_state) {
  return syscall(SYSCALL_MKEVENT, &manual_reset);
}

int epulse(handle_t h) {
  return syscall(SYSCALL_EPULSE, &h);
}

int eset(handle_t h) {
  return syscall(SYSCALL_ESET, &h);
}

int ereset(handle_t h) {
  return syscall(SYSCALL_ERESET, &h);
}

void exitos(int mode) {
  syscall(SYSCALL_EXITOS, &mode);
}

handle_t dup(handle_t h) {
  return syscall(SYSCALL_DUP, &h);
}

time_t time(time_t *timeptr) {
  return syscall(SYSCALL_TIME, &timeptr);
}

int gettimeofday(struct timeval *tv, void *tzp) {
  return syscall(SYSCALL_GETTIMEOFDAY, &tv);
}

int settimeofday(struct timeval *tv) {
  return syscall(SYSCALL_SETTIMEOFDAY, &tv);
}

clock_t clock() {
  return syscall(SYSCALL_CLOCK, NULL);
}

handle_t mksem(int initial_count) {
  return syscall(SYSCALL_MKSEM, &initial_count);
}

int semrel(handle_t h, int count) {
  return syscall(SYSCALL_SEMREL, &h);
}

int accept(int s, struct sockaddr *addr, int *addrlen) {
  return syscall(SYSCALL_ACCEPT, &s);
}

int bind(int s, const struct sockaddr *name, int namelen) {
  return syscall(SYSCALL_BIND, &s);
}

int connect(int s, const struct sockaddr *name, int namelen) {
  return syscall(SYSCALL_CONNECT, &s);
}

int getpeername(int s, struct sockaddr *name, int *namelen) {
  return syscall(SYSCALL_GETPEERNAME, &s);
}

int getsockname(int s, struct sockaddr *name, int *namelen) {
  return syscall(SYSCALL_GETSOCKNAME, &s);
}

int getsockopt(int s, int level, int optname, char *optval, int *optlen) {
  return syscall(SYSCALL_GETSOCKOPT, &s);
}

int listen(int s, int backlog) {
  return syscall(SYSCALL_LISTEN, &s);
}

int recv(int s, void *data, int size, unsigned int flags) {
  return syscall(SYSCALL_RECV, &s);
}

int recvfrom(int s, void *data, int size, unsigned int flags, struct sockaddr *from, int *fromlen) {
  return syscall(SYSCALL_RECVFROM, &s);
}

int send(int s, const void *data, int size, unsigned int flags) {
  return syscall(SYSCALL_SEND, &s);
}

int sendto(int s, const void *data, int size, unsigned int flags, const struct sockaddr *to, int tolen) {
  return syscall(SYSCALL_SENDTO, &s);
}

int setsockopt(int s, int level, int optname, const char *optval, int optlen) {
  return syscall(SYSCALL_SETSOCKOPT, &s);
}

int shutdown(int s, int how) {
  return syscall(SYSCALL_SHUTDOWN, &s);
}

int socket(int domain, int type, int protocol) {
  return syscall(SYSCALL_SOCKET, &domain);
}

int chdir(const char *name) {
  return syscall(SYSCALL_CHDIR, (void *) &name);
}

handle_t mkiomux(int flags) {
  return syscall(SYSCALL_MKIOMUX, (void *) &flags);
}

int dispatch(handle_t iomux, handle_t h, int events, int context) {
  return syscall(SYSCALL_DISPATCH, (void *) &iomux);
}

int recvmsg(int s, struct msghdr *hdr, unsigned int flags) {
  return syscall(SYSCALL_RECVMSG, (void *) &s);
}

int sendmsg(int s, struct msghdr *hdr, unsigned int flags) {
  return syscall(SYSCALL_SENDMSG, (void *) &s);
}

int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, const struct timeval *timeout) {
  return syscall(SYSCALL_SELECT, (void *) &nfds);
}

int pipe(handle_t fildes[2]) {
  return syscall(SYSCALL_PIPE, (void *) &fildes);
}

handle_t dup2(handle_t h1, handle_t h2) {
  return syscall(SYSCALL_DUP2, (void *) &h1);
}

int setmode(handle_t f, int mode) {
  return syscall(SYSCALL_SETMODE, (void *) &f);
}

int sysinfo(int cmd, void *data, size_t size) {
  return syscall(SYSCALL_SYSINFO, (void *) &cmd);
}

handle_t mkmutex(int owned) {
  return syscall(SYSCALL_MKMUTEX, (void *) &owned);
}

int mutexrel(handle_t h) {
  return syscall(SYSCALL_MUTEXREL, (void *) &h);
}

int getuid() {
  return syscall(SYSCALL_GETUID, NULL);
}

int setuid(uid_t uid) {
  return syscall(SYSCALL_SETUID, (void *) &uid);
}

int getgid() {
  return syscall(SYSCALL_GETGID, NULL);
}

int setgid(gid_t gid) {
  return syscall(SYSCALL_SETGID, (void *) &gid);
}

int geteuid() {
  return syscall(SYSCALL_GETEUID, NULL);
}

int seteuid(uid_t uid) {
  return syscall(SYSCALL_SETEUID, (void *) &uid);
}

int getegid() {
  return syscall(SYSCALL_GETEGID, NULL);
}

int setegid(gid_t gid) {
  return syscall(SYSCALL_SETEGID, (void *) &gid);
}

int getgroups(int size, gid_t list[]) {
  return syscall(SYSCALL_GETGROUPS, (void *) &size);
}

int setgroups(int size, const gid_t list[]) {
  return syscall(SYSCALL_SETGROUPS, (void *) &size);
}

int chown(const char *name, int owner, int group) {
  return syscall(SYSCALL_CHOWN, (void *) &name);
}

int fchown(handle_t f, int owner, int group) {
  return syscall(SYSCALL_FCHOWN, (void *) &f);
}

int access(const char *name, int mode) {
  return syscall(SYSCALL_ACCESS, (void *) &name);
}

int poll(struct pollfd fds[], unsigned int nfds, int timeout) {
  return syscall(SYSCALL_POLL, (void *) &fds);
}

int _getcwd(char *buf, size_t size) {
  return syscall(SYSCALL_GETCWD, (void *) &buf);
}

char *getcwd(char *buf, size_t size) {
  if (buf) {
    if (_getcwd(buf, size) < 0) {
      return NULL;
    } else {
      return buf;
    }
  } else {
    char curdir[MAXPATH];
    size_t len;

    if (_getcwd(curdir, MAXPATH) < 0) return NULL;
    len = strlen(curdir);

    if (size == 0) {
      size = len + 1;
    } else if (len >= size) {
      errno = ERANGE;
      return NULL;
    }

    buf = malloc(size);
    if (!buf) {
      errno = ENOMEM;
      return NULL;
    }

    memcpy(buf, curdir, len + 1);
    return buf;
  }
}
