//
// sockets.h
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// BSD socket interface
//

#ifndef SOCKETS_H
#define SOCKETS_H

//
// Socket types
//

#define SOCKTYPE_TCP          0
#define SOCKTYPE_UDP          1

#define SOCKTYPE_NUM          2

//
// Socket state
//

#define SOCKSTATE_UNBOUND     0
#define SOCKSTATE_BOUND       1
#define SOCKSTATE_CLOSED      2
#define SOCKSTATE_CONNECTING  3
#define SOCKSTATE_CONNECTED   4
#define SOCKSTATE_LISTENING   5

//
// Socket request
//

#define SOCKREQ_ACCEPT        0
#define SOCKREQ_CONNECT       1
#define SOCKREQ_RECV          2
#define SOCKREQ_RECVFROM      3
#define SOCKREQ_SEND          4
#define SOCKREQ_SENDTO        5
#define SOCKREQ_CLOSE         6

struct sockreq;

struct sockops
{
  int (*accept)(struct socket *s, struct sockaddr *addr, int *addrlen, struct socket **retval);
  int (*bind)(struct socket *s, struct sockaddr *name, int namelen);
  int (*close)(struct socket *s);
  int (*connect)(struct socket *s, struct sockaddr *name, int namelen);
  int (*getpeername)(struct socket *s, struct sockaddr *name, int *namelen);
  int (*getsockname)(struct socket *s, struct sockaddr *name, int *namelen);
  int (*getsockopt)(struct socket *s, int level, int optname, char *optval, int *optlen);
  int (*ioctl)(struct socket *s, int cmd, void *data, size_t size);
  int (*listen)(struct socket *s, int backlog);
  int (*recv)(struct socket *s, void *data, int size, unsigned int flags);
  int (*recvfrom)(struct socket *s, void *data, int size, unsigned int flags, struct sockaddr *from, int *fromlen);
  int (*send)(struct socket *s, void *data, int size, unsigned int flags);
  int (*sendto)(struct socket *s, void *data, int size, unsigned int flags, struct sockaddr *to, int tolen);
  int (*setsockopt)(struct socket *s, int level, int optname, const char *optval, int optlen);
  int (*shutdown)(struct socket *s, int how);
  int (*socket)(struct socket *s, int domain, int type, int protocol);
};

struct tcpsocket
{
  struct tcp_pcb *pcb;

  int backlog;
  int numpending;
  struct tcp_pcb **pending;

  struct pbuf *recvhead;
  struct pbuf *recvtail;
};

struct udpsocket
{
  struct udp_pcb *pcb;
  struct pbuf *recvhead;
  struct pbuf *recvtail;
};

struct socket
{
  struct object object;
  int type;
  int state;

  struct sockreq *waithead;
  struct sockreq *waittail;

  union 
  {
    struct tcpsocket tcp;
    struct udpsocket udp;
  };
};

struct sockreq
{
  struct sockreq *next;
  struct sockreq *prev;
  struct socket *socket;
  struct thread *thread;
  int type;
  err_t err;
  char *data;
  int len;
  struct sockaddr_in addr;
  struct tcp_pcb *pcb;
};

err_t submit_socket_request(struct socket *s, struct sockreq *req, int type, char *data, int len);
void release_socket_request(struct sockreq *req);
void socket_init();

int accept(struct socket *s, struct sockaddr *addr, int *addrlen, struct socket **retval);
int bind(struct socket *s, struct sockaddr *name, int namelen);
int closesocket(struct socket *s);
int connect(struct socket *s, struct sockaddr *name, int namelen);
int getpeername(struct socket *s, struct sockaddr *name, int *namelen);
int getsockname(struct socket *s, struct sockaddr *name, int *namelen);
int getsockopt(struct socket *s, int level, int optname, char *optval, int *optlen);
int ioctlsocket(struct socket *s, int cmd, void *data, size_t size);
int listen(struct socket *s, int backlog);
int recv(struct socket *s, void *data, int size, unsigned int flags);
int recvfrom(struct socket *s, void *data, int size, unsigned int flags, struct sockaddr *from, int *fromlen);
int send(struct socket *s, void *data, int size, unsigned int flags);
int sendto(struct socket *s, void *data, int size, unsigned int flags, struct sockaddr *to, int tolen);
int setsockopt(struct socket *s, int level, int optname, const char *optval, int optlen);
int shutdown(struct socket *s, int how);
int socket(int domain, int type, int protocol, struct socket **retval);

#endif
