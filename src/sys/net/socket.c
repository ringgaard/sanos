//
// sockets.c
//
// BSD socket interface
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

#include <net/net.h>

extern struct sockops tcpops;
extern struct sockops udpops;
extern struct sockops rawops;

struct sockops *sockops[SOCKTYPE_NUM];

void socket_init() {
  sockops[SOCKTYPE_TCP] = &tcpops;
  sockops[SOCKTYPE_UDP] = &udpops;
  sockops[SOCKTYPE_RAW] = &rawops;
}

void cancel_socket_request(struct sockreq *req) {
  if (req->next) req->next->prev = req->prev;
  if (req->prev) req->prev->next = req->next;
  if (req->socket) {
    if (req == req->socket->waithead) req->socket->waithead = req->next;
    if (req == req->socket->waittail) req->socket->waittail = req->prev;
  }
}

void release_socket_request(struct sockreq *req, int rc) {
  cancel_socket_request(req);
  req->rc = rc;
  mark_thread_ready(req->thread, 1, 2);
}

static void socket_timeout(void *arg) {
  struct sockreq *req = arg;

  if (req->thread->state == THREAD_STATE_READY) return;
  release_socket_request(req, req->rc > 0 ? req->rc : -ETIMEOUT);
}

err_t submit_socket_request(struct socket *s, struct sockreq *req, int type, struct msghdr *msg, unsigned int timeout) {
  struct timer timer;
  int rc;

  if (timeout == 0) return -ETIMEOUT;

  req->socket = s;
  req->thread = self();
  req->type = type;
  req->msg = msg;
  req->rc = 0;

  req->next = NULL;
  req->prev = s->waittail;
  if (s->waittail) s->waittail->next = req;
  s->waittail = req;
  if (!s->waithead) s->waithead = req;

  if (timeout != INFINITE) {
    init_timer(&timer, socket_timeout, req);
    timer.expires = ticks + timeout / MSECS_PER_TICK;
    add_timer(&timer);
  }

  rc = enter_alertable_wait(THREAD_WAIT_SOCKET);
  if (rc < 0) {
    cancel_socket_request(req);
    req->rc = rc;
  }

  if (timeout != INFINITE) del_timer(&timer);

  return req->rc;
}

int accept(struct socket *s, struct sockaddr *addr, int *addrlen, struct socket **retval) {
  return sockops[s->type]->accept(s, addr, addrlen, retval);
}

int bind(struct socket *s, struct sockaddr *name, int namelen) {
  return sockops[s->type]->bind(s, name, namelen);
}

int closesocket(struct socket *s) {
  int rc;

  rc = sockops[s->type]->close(s);
  detach_ioobject(&s->iob);
  return rc;
}

int connect(struct socket *s, struct sockaddr *name, int namelen) {
  return sockops[s->type]->connect(s, name, namelen);
}

int getpeername(struct socket *s, struct sockaddr *name, int *namelen) {
  return sockops[s->type]->getpeername(s, name, namelen);
}

int getsockname(struct socket *s, struct sockaddr *name, int *namelen) {
  return sockops[s->type]->getsockname(s, name, namelen);
}

int getsockopt(struct socket *s, int level, int optname, char *optval, int *optlen) {
  return sockops[s->type]->getsockopt(s, level, optname, optval, optlen);
}

int ioctlsocket(struct socket *s, int cmd, void *data, size_t size) {
  if (cmd == SIOIFLIST) {
    return netif_ioctl_list(data, size);
  } else if (cmd == SIOIFCFG) {
    return netif_ioctl_cfg(data, size);
  } else {
    return sockops[s->type]->ioctl(s, cmd, data, size);
  }
}

int listen(struct socket *s, int backlog) {
  return sockops[s->type]->listen(s, backlog);
}

int recv(struct socket *s, void *data, int size, unsigned int flags) {
  struct msghdr msg;
  struct iovec iov;
  int rc;

  if (!data) return -EFAULT;
  if (size < 0) return -EINVAL;

  msg.msg_name = NULL;
  msg.msg_namelen = 0;
  msg.msg_iov = &iov;
  msg.msg_iovlen = 1;
  iov.iov_base = data;
  iov.iov_len = size;

  rc = sockops[s->type]->recvmsg(s, &msg, flags);

  return rc;
}

int recvmsg(struct socket *s, struct msghdr *msg, unsigned int flags) {
  struct msghdr m;
  int rc;

  if (!msg) return -EINVAL;
  m.msg_name = msg->msg_name;
  m.msg_namelen = msg->msg_namelen;
  m.msg_iov = dup_iovec(msg->msg_iov, msg->msg_iovlen);
  m.msg_iovlen = msg->msg_iovlen;
  if (!m.msg_iov) return -ENOMEM;

  rc = sockops[s->type]->recvmsg(s, &m, flags);
  msg->msg_namelen = m.msg_namelen;
  kfree(m.msg_iov);

  return rc;
}

int recvv(struct socket *s, struct iovec *iov, int count) {
  struct msghdr msg;
  int rc;

  msg.msg_name = NULL;
  msg.msg_namelen = 0;
  msg.msg_iov = dup_iovec(iov, count);
  msg.msg_iovlen = count;
  if (!msg.msg_iov) return -ENOMEM;

  rc = sockops[s->type]->recvmsg(s, &msg, 0);
  kfree(msg.msg_iov);

  return rc;
}

int recvfrom(struct socket *s, void *data, int size, unsigned int flags, struct sockaddr *from, int *fromlen) {
  struct msghdr msg;
  struct iovec iov;
  int rc;

  if (!data) return -EFAULT;
  if (size < 0) return -EINVAL;

  msg.msg_name = from;
  msg.msg_namelen = fromlen ? *fromlen : 0;
  msg.msg_iov = &iov;
  msg.msg_iovlen = 1;
  iov.iov_base = data;
  iov.iov_len = size;

  rc = sockops[s->type]->recvmsg(s, &msg, flags);
  if (fromlen) *fromlen = msg.msg_namelen;

  return rc;
}

int send(struct socket *s, void *data, int size, unsigned int flags) {
  struct msghdr msg;
  struct iovec iov;
  int rc;

  if (!data) return -EFAULT;
  if (size < 0) return -EINVAL;

  msg.msg_name = NULL;
  msg.msg_namelen = 0;
  msg.msg_iov = &iov;
  msg.msg_iovlen = 1;
  iov.iov_base = data;
  iov.iov_len = size;

  rc = sockops[s->type]->sendmsg(s, &msg, flags);

  return rc;
}

int sendmsg(struct socket *s, struct msghdr *msg, unsigned int flags) {
  struct msghdr m;
  int rc;

  if (!msg) return -EINVAL;
  m.msg_name = msg->msg_name;
  m.msg_namelen = msg->msg_namelen;
  m.msg_iov = dup_iovec(msg->msg_iov, msg->msg_iovlen);
  m.msg_iovlen = msg->msg_iovlen;
  if (!m.msg_iov) return -ENOMEM;

  rc = sockops[s->type]->sendmsg(s, &m, flags);
  kfree(m.msg_iov);

  return rc;
}

int sendto(struct socket *s, void *data, int size, unsigned int flags, struct sockaddr *to, int tolen) {
  struct msghdr msg;
  struct iovec iov;
  int rc;

  if (!data) return -EFAULT;
  if (size < 0) return -EINVAL;

  msg.msg_name = to;
  msg.msg_namelen = tolen;
  msg.msg_iov = &iov;
  msg.msg_iovlen = 1;
  iov.iov_base = data;
  iov.iov_len = size;

  rc = sockops[s->type]->sendmsg(s, &msg, flags);

  return rc;
}

int sendv(struct socket *s, struct iovec *iov, int count) {
  struct msghdr msg;
  int rc;

  msg.msg_name = NULL;
  msg.msg_namelen = 0;
  msg.msg_iov = dup_iovec(iov, count);
  msg.msg_iovlen = count;
  if (!msg.msg_iov) return -ENOMEM;

  rc = sockops[s->type]->sendmsg(s, &msg, 0);
  kfree(msg.msg_iov);

  return rc;
}

int setsockopt(struct socket *s, int level, int optname, const char *optval, int optlen) {
  return sockops[s->type]->setsockopt(s, level, optname, optval, optlen);
}

int shutdown(struct socket *s, int how) {
  return sockops[s->type]->shutdown(s, how);
}

int socket(int domain, int type, int protocol, struct socket **retval) {
  struct socket *s;
  int socktype;
  int rc;

  if (domain != AF_INET) return -EAFNOSUPPORT;
  switch (type) {
    case SOCK_STREAM:
      if (protocol == IPPROTO_IP) {
        socktype = SOCKTYPE_TCP;
      } else if (protocol == IPPROTO_TCP) {
        socktype = SOCKTYPE_TCP;
      } else {
        return -EPROTONOSUPPORT;
      }
      break;

    case SOCK_DGRAM:
      if (protocol == IPPROTO_IP) {
        socktype = SOCKTYPE_UDP;
      } else if (protocol == IPPROTO_UDP) {
        socktype = SOCKTYPE_UDP;
      } else {
        return -EPROTONOSUPPORT;
      }
      break;

    case SOCK_RAW:
      socktype = SOCKTYPE_RAW;
      break;

    default:
      return -EPROTONOSUPPORT;
  }

  s = (struct socket *) kmalloc(sizeof(struct socket));
  if (!s) return -ENOMEM;
  memset(s, 0, sizeof(struct socket));

  init_ioobject(&s->iob, OBJECT_SOCKET);
  s->type = socktype;
  s->state = SOCKSTATE_UNBOUND;
  s->sndtimeo = INFINITE;
  s->rcvtimeo = INFINITE;

  rc = sockops[socktype]->socket(s, domain, type, protocol);
  if (rc < 0) return rc;

  *retval = s;
  return 0;
}
