//
// sysapi.h
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Operating System API
//

#include <os.h>
#include <os/syscall.h>
#include <os/cpu.h>

__declspec(naked) int syscall(int syscallno, void *params)
{
  __asm
  {
    mov   eax, dword ptr ds:[PEB_ADDRESS + 4]
    test  eax, eax
    jz    slow_syscall

    push  ebp
    mov	  ebp, esp

    mov	  eax, 8[ebp]
    mov	  edx, 12[ebp]
    mov   ecx, offset sys_return

    sysenter

sys_return:
    pop   ebp
    ret

slow_syscall:
    push  ebp
    mov	  ebp, esp

    mov	  eax, 8[ebp]
    mov	  edx, 12[ebp]

    int	  48

    leave
    ret
  }
}

int format(const char *devname, const char *type, const char *opts)
{
  return syscall(SYSCALL_FORMAT, (void *) &devname);
}

int mount(const char *type, const char *mntto, const char *mntfrom, const char *opts)
{
  return syscall(SYSCALL_MOUNT, (void *) &type);
}

int unmount(const char *path)
{
  return syscall(SYSCALL_UNMOUNT, (void *) &path);
}

int getfsstat(struct statfs *buf, size_t size)
{
  return syscall(SYSCALL_GETFSSTAT, (void *) &buf);
}

int fstatfs(handle_t f, struct statfs *buf)
{
  return syscall(SYSCALL_FSTATFS, (void *) &f);
}

int statfs(const char *name, struct statfs *buf)
{
  return syscall(SYSCALL_STATFS, (void *) &name);
}

handle_t open(const char *name, int mode)
{
  return syscall(SYSCALL_OPEN, (void *) &name);
}

int close(handle_t h)
{
  return syscall(SYSCALL_CLOSE, &h);
}

int flush(handle_t f)
{
  return syscall(SYSCALL_FLUSH, &f);
}

int read(handle_t f, void *data, size_t size)
{
  return syscall(SYSCALL_READ, &f);
}

int write(handle_t f, const void *data, size_t size)
{
  return syscall(SYSCALL_WRITE, &f);
}

int ioctl(handle_t f, int cmd, const void *data, size_t size)
{
  return syscall(SYSCALL_IOCTL, &f);
}

loff_t tell(handle_t f)
{
  return syscall(SYSCALL_TELL, &f);
}

loff_t lseek(handle_t f, loff_t offset, int origin)
{
  return syscall(SYSCALL_LSEEK, &f);
}

int chsize(handle_t f, loff_t size)
{
  return syscall(SYSCALL_CHSIZE, &f);
}

int futime(handle_t f, struct utimbuf *times)
{
  return syscall(SYSCALL_FUTIME, &f);
}

int utime(const char *name, struct utimbuf *times)
{
  return syscall(SYSCALL_UTIME, (void *) &name);
}

int fstat(handle_t f, struct stat *buffer)
{
  return syscall(SYSCALL_FSTAT, &f);
}

int stat(const char *name, struct stat *buffer)
{
  return syscall(SYSCALL_STAT, (void *) &name);
}

int mkdir(const char *name)
{
  return syscall(SYSCALL_MKDIR, (void *) &name);
}

int rmdir(const char *name)
{
  return syscall(SYSCALL_RMDIR, (void *) &name);
}

int rename(const char *oldname, const char *newname)
{
  return syscall(SYSCALL_RENAME, (void *) &oldname);
}

int link(const char *oldname, const char *newname)
{
  return syscall(SYSCALL_LINK, (void *) &oldname);
}

int unlink(const char *name)
{
  return syscall(SYSCALL_UNLINK, (void *) &name);
}

handle_t opendir(const char *name)
{
  return syscall(SYSCALL_OPENDIR, (void *) &name);
}

int readdir(handle_t f, struct dirent *dirp, int count)
{
  return syscall(SYSCALL_READDIR, &f);
}

void *mmap(void *addr, unsigned long size, int type, int protect)
{
  return (void *) syscall(SYSCALL_MMAP, &addr);
}

int munmap(void *addr, unsigned long size, int type)
{
  return syscall(SYSCALL_MUNMAP, &addr);
}

void *mremap(void *addr, unsigned long oldsize, unsigned long newsize, int type, int protect)
{
  return (void *) syscall(SYSCALL_MREMAP, &addr);
}

int mprotect(void *addr, unsigned long size, int protect)
{
  return syscall(SYSCALL_MPROTECT, &addr);
}

int mlock(void *addr, unsigned long size)
{
  return syscall(SYSCALL_MLOCK, &addr);
}

int munlock(void *addr, unsigned long size)
{
  return syscall(SYSCALL_MUNLOCK, &addr);
}

int wait(handle_t h, int timeout)
{
  return syscall(SYSCALL_WAIT, &h);
}

int waitall(handle_t *h, int count, int timeout)
{
  return syscall(SYSCALL_WAITALL, &h);
}

int waitany(handle_t *h, int count, int timeout)
{
  return syscall(SYSCALL_WAITANY, &h);
}

handle_t mkevent(int manual_reset, int initial_state)
{
  return syscall(SYSCALL_MKEVENT, &manual_reset);
}

int epulse(handle_t h)
{
  return syscall(SYSCALL_EPULSE, &h);
}

int eset(handle_t h)
{
  return syscall(SYSCALL_ESET, &h);
}

int ereset(handle_t h)
{
  return syscall(SYSCALL_ERESET, &h);
}

handle_t self()
{
  return syscall(SYSCALL_SELF, NULL);
}

void exit(int status)
{
  syscall(SYSCALL_EXIT, &status);
}

handle_t dup(handle_t h)
{
  return syscall(SYSCALL_DUP, &h);
}

time_t time()
{
  return syscall(SYSCALL_TIME, NULL);
}

int gettimeofday(struct timeval *tv)
{
  return syscall(SYSCALL_GETTIMEOFDAY, &tv);
}

int settimeofday(struct timeval *tv)
{
  return syscall(SYSCALL_SETTIMEOFDAY, &tv);
}

clock_t clock()
{
  return syscall(SYSCALL_CLOCK, NULL);
}

handle_t mksem(int initial_count)
{
  return syscall(SYSCALL_MKSEM, &initial_count);
}

int semrel(handle_t h, int count)
{
  return syscall(SYSCALL_SEMREL, &h);
}

int accept(int s, struct sockaddr *addr, int *addrlen)
{
  return syscall(SYSCALL_ACCEPT, &s);
}

int bind(int s, const struct sockaddr *name, int namelen)
{
  return syscall(SYSCALL_BIND, &s);
}

int connect(int s, const struct sockaddr *name, int namelen)
{
  return syscall(SYSCALL_CONNECT, &s);
}

int getpeername(int s, struct sockaddr *name, int *namelen)
{
  return syscall(SYSCALL_GETPEERNAME, &s);
}

int getsockname(int s, struct sockaddr *name, int *namelen)
{
  return syscall(SYSCALL_GETSOCKNAME, &s);
}

int getsockopt(int s, int level, int optname, char *optval, int *optlen)
{
  return syscall(SYSCALL_GETSOCKOPT, &s);
}

int listen(int s, int backlog)
{
  return syscall(SYSCALL_LISTEN, &s);
}

int recv(int s, void *data, int size, unsigned int flags)
{
  return syscall(SYSCALL_RECV, &s);
}

int recvfrom(int s, void *data, int size, unsigned int flags, struct sockaddr *from, int *fromlen)
{
  return syscall(SYSCALL_RECVFROM, &s);
}

int send(int s, const void *data, int size, unsigned int flags)
{
  return syscall(SYSCALL_SEND, &s);
}

int sendto(int s, const void *data, int size, unsigned int flags, const struct sockaddr *to, int tolen)
{
  return syscall(SYSCALL_SENDTO, &s);
}

int setsockopt(int s, int level, int optname, const char *optval, int optlen)
{
  return syscall(SYSCALL_SETSOCKOPT, &s);
}

int shutdown(int s, int how)
{
  return syscall(SYSCALL_SHUTDOWN, &s);
}

int socket(int domain, int type, int protocol)
{
  return syscall(SYSCALL_SOCKET, &domain);
}
