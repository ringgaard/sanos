//
// rawsock.c
//
// Raw socket interface
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

static err_t recv_raw(void *arg, struct raw_pcb *pcb, struct pbuf *p, struct ip_addr *addr) {
  struct socket *s = arg;
  struct sockreq *req = s->waithead;
  struct sockaddr_in *sin;
  int rc;

  if (req) {
    rc = write_iovec(req->msg->msg_iov, req->msg->msg_iovlen, p->payload, p->len);
    if (rc < p->len) rc = -EMSGSIZE;

    if (req->msg->msg_name) {
      if (req->msg->msg_namelen < sizeof(struct sockaddr_in)) {
        rc = -EFAULT;
      } else {
        sin = (struct sockaddr_in *) req->msg->msg_name;
        sin->sin_family = AF_INET;
        sin->sin_port = 0;
        sin->sin_addr.s_addr = addr->addr;
      }
    }
    req->msg->msg_namelen = sizeof(struct sockaddr_in);

    pbuf_free(p);

    release_socket_request(req, rc);
  } else {
    if (p->next) {
      kprintf("recv_raw: fragmented pbuf not supported\n");
      return -EINVAL;
    }

    if (s->raw.recvtail) {
      pbuf_chain(s->raw.recvtail, p);
      s->raw.recvtail = p;
    } else {
      s->raw.recvhead = p;
      s->raw.recvtail = p;
    }

    set_io_event(&s->iob, IOEVT_READ);
  }

  return 1;
}

static int rawsock_accept(struct socket *s, struct sockaddr *addr, int *addrlen, struct socket **retval) {
  return -EINVAL;
}

static int rawsock_bind(struct socket *s, struct sockaddr *name, int namelen) {
  int rc;
  struct sockaddr_in *sin;

  if (!name) return -EFAULT;
  if (namelen < sizeof(struct sockaddr_in)) return -EFAULT;
  sin = (struct sockaddr_in *) name;
  if (sin->sin_family != AF_INET && sin->sin_family != AF_UNSPEC) return -EAFNOSUPPORT;

  rc = raw_bind(s->raw.pcb, (struct ip_addr *) &sin->sin_addr);
  if (rc < 0) return rc;

  s->state = SOCKSTATE_BOUND;
  return 0;
}

static int rawsock_close(struct socket *s) {
  struct sockreq *req;
  struct sockreq *next;

  s->state = SOCKSTATE_CLOSED;
  req = s->waithead;
  while (req) {
    next = req->next;
    release_socket_request(req, -EABORT);
    req = next;
  }

  if (s->raw.pcb) {
    s->raw.pcb->recv_arg = NULL;
    raw_remove(s->raw.pcb);
  }

  if (s->raw.recvhead) pbuf_free(s->raw.recvhead);

  return 0;
}

static int rawsock_connect(struct socket *s, struct sockaddr *name, int namelen) {
  int rc;
  struct sockaddr_in *sin;

  if (!name) return -EFAULT;
  if (namelen < sizeof(struct sockaddr_in)) return -EFAULT;
  sin = (struct sockaddr_in *) name;
  if (sin->sin_family != AF_INET && sin->sin_family != AF_UNSPEC) return -EAFNOSUPPORT;
  if (s->state == SOCKSTATE_CLOSED) return -EINVAL;

  rc = raw_connect(s->raw.pcb, (struct ip_addr *) &sin->sin_addr);
  if (rc < 0) return rc;

  s->state = SOCKSTATE_CONNECTED;
  return 0;
}

static int rawsock_getpeername(struct socket *s, struct sockaddr *name, int *namelen) {
  struct sockaddr_in *sin;

  if (!namelen) return -EFAULT;
  if (*namelen < sizeof(struct sockaddr_in)) return -EFAULT;
  if (s->state != SOCKSTATE_CONNECTED) return -EINVAL;

  sin = (struct sockaddr_in *) name;
  sin->sin_family = AF_INET;
  sin->sin_port = 0;
  sin->sin_addr.s_addr = s->raw.pcb->remote_ip.addr;

  *namelen = sizeof(struct sockaddr_in);
  return 0;
}

static int rawsock_getsockname(struct socket *s, struct sockaddr *name, int *namelen) {
  struct sockaddr_in *sin;

  if (!namelen) return -EFAULT;
  if (*namelen < sizeof(struct sockaddr_in)) return -EFAULT;
  if (s->state != SOCKSTATE_BOUND && s->state != SOCKSTATE_CONNECTED) return -EINVAL;

  sin = (struct sockaddr_in *) name;
  sin->sin_family = AF_INET;
  sin->sin_port = 0;
  sin->sin_addr.s_addr = s->raw.pcb->local_ip.addr;

  *namelen = sizeof(struct sockaddr_in);
  return 0;
}

static int rawsock_getsockopt(struct socket *s, int level, int optname, char *optval, int *optlen) {
  return -ENOSYS;
}

static int rawsock_ioctl(struct socket *s, int cmd, void *data, size_t size) {
  switch (cmd) {
    case FIONBIO:
      if (!data || size != 4) return -EFAULT;
      if (*(int *) data) {
        s->flags |= SOCK_NBIO;
      } else {
        s->flags &= ~SOCK_NBIO;
      }
      break;

    default:
      return -ENOSYS;
  }

  return 0;
}

static int rawsock_listen(struct socket *s, int backlog) {
  return -EINVAL;
}

static int rawsock_recvmsg(struct socket *s, struct msghdr *msg, unsigned int flags) {
  struct pbuf *p;
  struct ip_hdr *iphdr;
  int rc;
  struct sockaddr_in *sin;
  struct sockreq req;

  p = s->raw.recvhead;
  if (p) {
    s->raw.recvhead = pbuf_dechain(p);
    if (!s->raw.recvhead) s->raw.recvtail = NULL; 
    if (!s->raw.recvhead) clear_io_event(&s->iob, IOEVT_READ);

    iphdr = p->payload;

    rc = write_iovec(msg->msg_iov, msg->msg_iovlen, p->payload, p->len);
    if (rc < p->len) rc = -EMSGSIZE;

    if (msg->msg_name) {
      if (msg->msg_namelen < sizeof(struct sockaddr_in)) return -EFAULT;
      sin = (struct sockaddr_in *) msg->msg_name;
      sin->sin_family = AF_INET;
      sin->sin_port = 0;
      sin->sin_addr.s_addr = iphdr->src.addr;
    }
    msg->msg_namelen = sizeof(struct sockaddr_in);

    pbuf_free(p);
  } else if (s->flags & SOCK_NBIO) {
    rc = -EAGAIN;
  } else {
    rc = submit_socket_request(s, &req, SOCKREQ_RECV, msg, s->rcvtimeo);
  }

  return rc;
}

static int rawsock_sendmsg(struct socket *s, struct msghdr *msg, unsigned int flags) {
  struct pbuf *p;
  int size;
  int rc;

  size = get_iovec_size(msg->msg_iov, msg->msg_iovlen);

  if (msg->msg_name) {
    rc = rawsock_connect(s, msg->msg_name, msg->msg_namelen);
    if (rc < 0) return rc;
  }

  if (s->state != SOCKSTATE_CONNECTED) return -ENOTCONN;

  p = pbuf_alloc(PBUF_TRANSPORT, size, PBUF_RW);
  if (!p) return -ENOMEM;

  rc = read_iovec(msg->msg_iov, msg->msg_iovlen, p->payload, size);
  if (rc < 0) return rc;
  
  rc = raw_send(s->raw.pcb, p);
  if (rc < 0) {
    pbuf_free(p);
    return rc;
  }

  return size;
}

static int rawsock_setsockopt(struct socket *s, int level, int optname, const char *optval, int optlen) {
  if (level == SOL_SOCKET)
  {
    switch (optname) {
      case SO_SNDTIMEO:
        if (optlen != 4) return -EINVAL;
        s->sndtimeo = *(unsigned int *) optval;
        break;

      case SO_RCVTIMEO:
        if (optlen != 4) return -EINVAL;
        s->rcvtimeo = *(unsigned int *) optval;
        break;

      default:
        return -ENOPROTOOPT;
    }
  } else {
    return -ENOPROTOOPT;
  }

  return 0;
}

static int rawsock_shutdown(struct socket *s, int how) {
  return -ENOSYS;
}

static int rawsock_socket(struct socket *s, int domain, int type, int protocol) {
  s->raw.pcb = raw_new(protocol);
  if (!s->raw.pcb) return -ENOMEM;
  raw_recv(s->raw.pcb, recv_raw, s);
  set_io_event(&s->iob, IOEVT_WRITE);

  return 0;
}

struct sockops rawops = {
  rawsock_accept,
  rawsock_bind,
  rawsock_close,
  rawsock_connect,
  rawsock_getpeername,
  rawsock_getsockname,
  rawsock_getsockopt,
  rawsock_ioctl,
  rawsock_listen,
  rawsock_recvmsg,
  rawsock_sendmsg,
  rawsock_setsockopt,
  rawsock_shutdown,
  rawsock_socket,
};
