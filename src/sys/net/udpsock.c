//
// udpsock.c
//
// UDP socket interface
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

static err_t recv_udp(void *arg, struct udp_pcb *pcb, struct pbuf *p, struct ip_addr *addr, unsigned short port) {
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
        sin->sin_port = htons(port);
        sin->sin_addr.s_addr = addr->addr;
      }
    }
    req->msg->msg_namelen = sizeof(struct sockaddr_in);

    pbuf_free(p);

    release_socket_request(req, rc);
  } else {
    if (p->next) {
      kprintf("recv_udp: fragmented pbuf not supported\n");
      return -EINVAL;
    }

    if (s->udp.recvtail) {
      pbuf_chain(s->udp.recvtail, p);
      s->udp.recvtail = p;
    } else {
      s->udp.recvhead = p;
      s->udp.recvtail = p;
    }

    set_io_event(&s->iob, IOEVT_READ);
  }

  return 0;
}

static int udpsock_accept(struct socket *s, struct sockaddr *addr, int *addrlen, struct socket **retval) {
  return -EINVAL;
}

static int udpsock_bind(struct socket *s, struct sockaddr *name, int namelen) {
  int rc;
  struct sockaddr_in *sin;

  if (!name) return -EFAULT;
  if (namelen < sizeof(struct sockaddr_in)) return -EFAULT;
  sin = (struct sockaddr_in *) name;
  if (sin->sin_family != AF_INET && sin->sin_family != AF_UNSPEC) return -EAFNOSUPPORT;

  if (!s->udp.pcb) {
    s->udp.pcb = udp_new();
    if (!s->udp.pcb) return -ENOMEM;
    if (s->flags & SOCK_BCAST) s->udp.pcb->flags |= UDP_FLAGS_BROADCAST;
    udp_recv(s->udp.pcb, recv_udp, s);
  }

  rc = udp_bind(s->udp.pcb, (struct ip_addr *) &sin->sin_addr, ntohs(sin->sin_port));
  if (rc < 0) return rc;

  s->state = SOCKSTATE_BOUND;
  return 0;
}

static int udpsock_close(struct socket *s) {
  struct sockreq *req;
  struct sockreq *next;

  s->state = SOCKSTATE_CLOSED;
  req = s->waithead;
  while (req) {
    next = req->next;
    release_socket_request(req, -EABORT);
    req = next;
  }

  if (s->udp.pcb) {
    s->udp.pcb->recv_arg = NULL;
    udp_remove(s->udp.pcb);
  }

  if (s->udp.recvhead) pbuf_free(s->udp.recvhead);

  return 0;
}

static int udpsock_connect(struct socket *s, struct sockaddr *name, int namelen) {
  int rc;
  struct sockaddr_in *sin;

  if (!name) return -EFAULT;
  if (namelen < sizeof(struct sockaddr_in)) return -EFAULT;
  sin = (struct sockaddr_in *) name;
  if (sin->sin_family != AF_INET && sin->sin_family != AF_UNSPEC) return -EAFNOSUPPORT;
  if (s->state == SOCKSTATE_CLOSED) return -EINVAL;

  if (!s->udp.pcb) {
    s->udp.pcb = udp_new();
    if (!s->udp.pcb) return -ENOMEM;
    if (s->flags & SOCK_BCAST) s->udp.pcb->flags |= UDP_FLAGS_BROADCAST;
    udp_recv(s->udp.pcb, recv_udp, s);
  }

  rc = udp_connect(s->udp.pcb, (struct ip_addr *) &sin->sin_addr, ntohs(sin->sin_port));
  if (rc < 0) return rc;

  s->state = SOCKSTATE_CONNECTED;
  return 0;
}

static int udpsock_getpeername(struct socket *s, struct sockaddr *name, int *namelen) {
  struct sockaddr_in *sin;

  if (!namelen) return -EFAULT;
  if (*namelen < sizeof(struct sockaddr_in)) return -EFAULT;
  if (s->state != SOCKSTATE_CONNECTED) return -EINVAL;

  sin = (struct sockaddr_in *) name;
  sin->sin_family = AF_INET;
  sin->sin_port = htons(s->udp.pcb->remote_port);
  sin->sin_addr.s_addr = s->udp.pcb->remote_ip.addr;

  *namelen = sizeof(struct sockaddr_in);
  return 0;
}

static int udpsock_getsockname(struct socket *s, struct sockaddr *name, int *namelen) {
  struct sockaddr_in *sin;

  if (!namelen) return -EFAULT;
  if (*namelen < sizeof(struct sockaddr_in)) return -EFAULT;
  if (s->state != SOCKSTATE_BOUND && s->state != SOCKSTATE_CONNECTED) return -EINVAL;

  sin = (struct sockaddr_in *) name;
  sin->sin_family = AF_INET;
  sin->sin_port = htons(s->udp.pcb->local_port);
  sin->sin_addr.s_addr = s->udp.pcb->local_ip.addr;

  *namelen = sizeof(struct sockaddr_in);
  return 0;
}

static int udpsock_getsockopt(struct socket *s, int level, int optname, char *optval, int *optlen) {
  return -ENOSYS;
}

static int udpsock_ioctl(struct socket *s, int cmd, void *data, size_t size) {
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

static int udpsock_listen(struct socket *s, int backlog) {
  return -EINVAL;
}

static int udpsock_recvmsg(struct socket *s, struct msghdr *msg, unsigned int flags) {
  struct pbuf *p;
  struct udp_hdr *udphdr;
  struct ip_hdr *iphdr;
  void *buf;
  int len;
  int rc;
  struct sockaddr_in *sin;
  struct sockreq req;

  if (msg->msg_name && msg->msg_namelen < sizeof(struct sockaddr_in)) return -EFAULT;
  if (!s->udp.pcb || s->udp.pcb->local_port == 0) return -EINVAL;

  p = s->udp.recvhead;
  if (p) {
    s->udp.recvhead = pbuf_dechain(p);
    if (!s->udp.recvhead) s->udp.recvtail = NULL; 
    if (!s->udp.recvhead) clear_io_event(&s->iob, IOEVT_READ);

    buf = p->payload;
    len = p->len;

    pbuf_header(p, UDP_HLEN);
    udphdr = p->payload;

    //FIXME: this does not work if there are options in the ip header
    pbuf_header(p, IP_HLEN); 
    iphdr = p->payload;

    rc = write_iovec(msg->msg_iov, msg->msg_iovlen, buf, len);
    if (rc < len) rc = -EMSGSIZE;

    if (msg->msg_name) {
      sin = (struct sockaddr_in *) msg->msg_name;
      sin->sin_family = AF_INET;
      sin->sin_port = udphdr->src;
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

static int udpsock_sendmsg(struct socket *s, struct msghdr *msg, unsigned int flags) {
  struct pbuf *p;
  int size;
  int rc;

  if (msg->msg_name && msg->msg_namelen < sizeof(struct sockaddr_in)) return -EFAULT;

  if (!s->udp.pcb || s->udp.pcb->local_port == 0) {
    struct sockaddr_in sin;

    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    rc = udpsock_bind(s, (struct sockaddr *) &sin, sizeof(sin));
    if (rc < 0) return rc;
  }

  size = get_iovec_size(msg->msg_iov, msg->msg_iovlen);

  p = pbuf_alloc(PBUF_TRANSPORT, size, PBUF_RW);
  if (!p) return -ENOMEM;

  rc = read_iovec(msg->msg_iov, msg->msg_iovlen, p->payload, size);
  if (rc < 0) return rc;

  if (msg->msg_name) {
    struct sockaddr_in *sin = (struct sockaddr_in *) msg->msg_name;
    rc = udp_send(s->udp.pcb, p, (struct ip_addr *) &sin->sin_addr, ntohs(sin->sin_port), NULL);
  } else {
    rc = udp_send(s->udp.pcb, p, NULL, 0, NULL);
  }

  if (rc < 0)
  {
    pbuf_free(p);
    return rc;
  }

  return size;
}

static int udpsock_setsockopt(struct socket *s, int level, int optname, const char *optval, int optlen) {
  if (level == SOL_SOCKET) {
    switch (optname) {
      case SO_BROADCAST:
        if (optlen != 4) return -EINVAL;
        if (*(int *) optval) {
          s->flags |= SOCK_BCAST;
          if (s->udp.pcb) s->udp.pcb->flags |= UDP_FLAGS_BROADCAST;
        } else {
          s->flags &= ~SOCK_BCAST;
          if (s->udp.pcb) s->udp.pcb->flags &= ~UDP_FLAGS_BROADCAST;
        }
        break;

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

static int udpsock_shutdown(struct socket *s, int how) {
  return -ENOSYS;
}

static int udpsock_socket(struct socket *s, int domain, int type, int protocol) {
  set_io_event(&s->iob, IOEVT_WRITE);

  return 0;
}

struct sockops udpops = {
  udpsock_accept,
  udpsock_bind,
  udpsock_close,
  udpsock_connect,
  udpsock_getpeername,
  udpsock_getsockname,
  udpsock_getsockopt,
  udpsock_ioctl,
  udpsock_listen,
  udpsock_recvmsg,
  udpsock_sendmsg,
  udpsock_setsockopt,
  udpsock_shutdown,
  udpsock_socket,
};
