//
// sockets.c
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// BSD socket interface
//

#include <net/net.h>

extern struct sockops tcpops;
extern struct sockops udpops;

struct sockops *sockops[SOCKTYPE_NUM];

void socket_init()
{
  sockops[SOCKTYPE_TCP] = &tcpops;
  sockops[SOCKTYPE_UDP] = &udpops;
}

void release_socket_request(struct sockreq *req, int err)
{
  req->err = err;

  if (req->next) req->next->prev = req->prev;
  if (req->prev) req->prev->next = req->next;
  if (req->socket)
  {
    if (req == req->socket->waithead) req->socket->waithead = req->next;
    if (req == req->socket->waittail) req->socket->waittail = req->prev;
  }

  mark_thread_ready(req->thread);
}

static void socket_timeout(void *arg)
{
  struct sockreq *req = arg;

  if (req->thread->state == THREAD_STATE_READY) return;
  release_socket_request(req, req->err > 0 ? req->err : -ETIMEOUT);
}

err_t submit_socket_request(struct socket *s, struct sockreq *req, int type, char *data, int len, unsigned int timeout)
{
  struct timer timer;

  req->socket = s;
  req->thread = current_thread();
  req->type = type;
  req->data = data;
  req->len = len;
  req->err = 0;

  req->next = NULL;
  req->prev = s->waittail;
  if (s->waittail) s->waittail->next = req;
  s->waittail = req;
  if (!s->waithead) s->waithead = req;

  if (timeout != INFINITE)
  {
    init_timer(&timer, socket_timeout, req);
    timer.expires = ticks + timeout / MSECS_PER_TICK;
    add_timer(&timer);
  }

  req->thread->state = THREAD_STATE_WAITING;
  dispatch();

  if (timeout != INFINITE) del_timer(&timer);

  return req->err;
}

int accept(struct socket *s, struct sockaddr *addr, int *addrlen, struct socket **retval)
{
  return sockops[s->type]->accept(s, addr, addrlen, retval);
}

int bind(struct socket *s, struct sockaddr *name, int namelen)
{
  return sockops[s->type]->bind(s, name, namelen);
}

int closesocket(struct socket *s)
{
  int rc;

  rc = sockops[s->type]->close(s);
  kfree(s);
  return rc;
}

int connect(struct socket *s, struct sockaddr *name, int namelen)
{
  return sockops[s->type]->connect(s, name, namelen);
}

int getpeername(struct socket *s, struct sockaddr *name, int *namelen)
{
  return sockops[s->type]->getpeername(s, name, namelen);
}

int getsockname(struct socket *s, struct sockaddr *name, int *namelen)
{
  return sockops[s->type]->getsockname(s, name, namelen);
}

int getsockopt(struct socket *s, int level, int optname, char *optval, int *optlen)
{
  return sockops[s->type]->getsockopt(s, level, optname, optval, optlen);
}

int ioctlsocket(struct socket *s, int cmd, void *data, size_t size)
{
  return sockops[s->type]->ioctl(s, cmd, data, size);
}

int listen(struct socket *s, int backlog)
{
  return sockops[s->type]->listen(s, backlog);
}

int recv(struct socket *s, void *data, int size, unsigned int flags)
{
  return sockops[s->type]->recv(s, data, size, flags);
}

int recvfrom(struct socket *s, void *data, int size, unsigned int flags, struct sockaddr *from, int *fromlen)
{
  return sockops[s->type]->recvfrom(s, data, size, flags, from, fromlen);
}

int send(struct socket *s, void *data, int size, unsigned int flags)
{
  return sockops[s->type]->send(s, data, size, flags);
}

int sendto(struct socket *s, void *data, int size, unsigned int flags, struct sockaddr *to, int tolen)
{
  return sockops[s->type]->sendto(s, data, size, flags, to, tolen);
}

int setsockopt(struct socket *s, int level, int optname, const char *optval, int optlen)
{
  return sockops[s->type]->setsockopt(s, level, optname, optval, optlen);
}

int shutdown(struct socket *s, int how)
{
  return sockops[s->type]->shutdown(s, how);
}

int socket(int domain, int type, int protocol, struct socket **retval)
{
  struct socket *s;
  int socktype;
  int rc;

  if (domain != AF_INET) return -EPROTONOSUPPORT;
  switch (type)
  {
    case SOCK_STREAM:
      if (protocol == IPPROTO_IP)
	socktype = SOCKTYPE_TCP;
      else if (protocol == IPPROTO_TCP)
	socktype = SOCKTYPE_TCP;
      else
	return -EPROTONOSUPPORT;
      break;

    case SOCK_DGRAM:
      if (protocol == IPPROTO_IP)
	socktype = SOCKTYPE_UDP;
      else if (protocol == IPPROTO_UDP)
	socktype = SOCKTYPE_UDP;
      else
	return -EPROTONOSUPPORT;
      break;

    default:
      return -EPROTONOSUPPORT;
  }

  s = (struct socket *) kmalloc(sizeof(struct socket));
  if (!s) return -ENOMEM;
  memset(s, 0, sizeof(struct socket));

  init_object(&s->object, OBJECT_SOCKET);
  s->type = socktype;
  s->state = SOCKSTATE_UNBOUND;

  rc = sockops[socktype]->socket(s, domain, type, protocol);
  if (rc < 0) return rc;

  *retval = s;
  return 0;
}
