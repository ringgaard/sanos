#ifndef SOCKCALL_H
#define SOCKCALL_H

#ifdef OSEXEC

typedef SOCKET socket_t;

#else

#define FD_SETSIZE 64

typedef unsigned int *socket_t;

struct sockaddr 
{
  unsigned short sa_family;
  char sa_data[14];
};

struct in_addr 
{
  union 
  {
    struct { unsigned char s_b1,s_b2,s_b3,s_b4; } S_un_b;
    struct { unsigned short s_w1,s_w2; } S_un_w;
    unsigned long S_addr;
  } S_un;
};

struct sockaddr_in 
{
  short sin_family;
  unsigned short sin_port;
  struct in_addr sin_addr;
  char sin_zero[8];
};

typedef struct fd_set 
{
  unsigned int fd_count;
  socket_t fd_array[FD_SETSIZE];
} fd_set;

struct timeval 
{
  long tv_sec;
  long tv_usec;
};

struct hostent
{
  char *h_name;
  char **h_aliases;
  short h_addrtype;
  short h_length;
  char **h_addr_list;
};

struct protoent 
{
  char *p_name;
  char **p_aliases;
  short p_proto;
};

#endif

struct sockcallops
{
  socket_t (*socket)(int af, int type, int protocol);
  int (*connect)(socket_t s, const struct sockaddr *name, int namelen);
  int (*listen)(socket_t s, int backlog);
  int (*bind)(socket_t s, const struct sockaddr *name, int namelen);
  socket_t (*accept)(socket_t s, struct sockaddr *addr, int *addrlen);
  int (*closesocket)(socket_t s);
  int (*shutdown)(socket_t s, int how);

  int (*select)(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, const struct timeval *timeout);

  int (*recv)(socket_t s,char *buf, int len, int flags);
  int (*send)(socket_t s, const char *buf, int len, int flags);
  int (*recvfrom)(socket_t s, char *buf, int len, int flags, struct sockaddr *from, int *fromlen);
  int (*sendto)(socket_t s, const char *buf, int len, int flags, const struct sockaddr *to, int tolen);
  
  int (*gethostname)(char *name, int namelen);
  struct hostent *(*gethostbyaddr)(const char *addr, int len, int type);
  struct hostent *(*gethostbyname)(const char *name);
  struct protoent *(*getprotobyname)(const char *name);

  unsigned short (*htons)(unsigned short hostshort);
  unsigned long (*htonl)(unsigned long hostlong);
  unsigned short (*ntohs)(unsigned short netshort);
  unsigned long (*ntohl)(unsigned long netlong);

  int (*getsockopt)(socket_t s, int level, int optname, char *optval, int *optlen);
  int (*setsockopt)(socket_t s, int level, int optname, const char *optval, int optlen);

  int (*getsockname)(socket_t s, struct sockaddr *name, int *namelen);
  int (*ioctlsocket)(socket_t s, long cmd, unsigned long *argp);

  int  (*__WSAFDIsSet)(socket_t s, fd_set *fd);
  int (*WSAGetLastError)();
};

#endif
