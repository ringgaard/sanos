//
// tcpsock.c
//
// TCP socket interface
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

static err_t recv_tcp(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err);
static err_t sent_tcp(void *arg, struct tcp_pcb *pcb, unsigned short len);
static void err_tcp(void *arg, err_t err);

static int fill_sndbuf(struct socket *s, struct iovec *iov, int iovlen) {
  int left;
  int bytes;
  int len;
  int rc;

  left = tcp_sndbuf(s->tcp.pcb);

  if (iovlen == 1) {
    if (iov->iov_len == 0) return 0;

    len = iov->iov_len;
    if (len > left) len = left;

    rc = tcp_write(s->tcp.pcb, iov->iov_base, len, (s->flags & SOCK_NODELAY) ? TCP_WRITE_FLUSH : TCP_WRITE_NAGLE);
    if (rc < 0) return rc;

    (char *) iov->iov_base += len;
    iov->iov_len -= len;

    return len;
  } else {
    bytes = 0;
    while (left > 0 && iovlen > 0) {
      if (iov->iov_len > 0) {
        len = iov->iov_len;
        if (len > left) len = left;

        rc = tcp_write(s->tcp.pcb, iov->iov_base, len, TCP_WRITE_NOFLUSH);
        if (rc < 0) return rc;

        (char *) iov->iov_base += len;
        iov->iov_len -= len;

        left -= len;
        bytes += len;
      }

      iov++;
      iovlen--;
    }

    rc = tcp_write(s->tcp.pcb, NULL, 0, (s->flags & SOCK_NODELAY) ? TCP_WRITE_FLUSH : TCP_WRITE_NAGLE);
    if (rc < 0) return rc;

    return bytes;
  }
}

static int fetch_rcvbuf(struct socket *s, struct iovec *iov, int iovlen) {
  int left;
  int recved;
  int rc;
  struct pbuf *p;
  
  left = get_iovec_size(iov, iovlen);
  recved = 0;
  while (s->tcp.recvhead && left > 0) {
    p = s->tcp.recvhead;

    if (left < p->len) {
      rc = write_iovec(iov, iovlen, p->payload, left);
      if (rc < 0) return rc;

      recved += rc;
      left -= rc;

      pbuf_header(p, -rc);
    } else {
      rc = write_iovec(iov, iovlen, p->payload, p->len);
      if (rc < 0) return rc;

      recved += rc;
      left -= rc;

      s->tcp.recvhead = pbuf_dechain(p);
      if (!s->tcp.recvhead) s->tcp.recvtail = NULL;
      pbuf_free(p);
    }
  }

  return recved;
}

static int recv_ready(struct socket *s) {
  struct pbuf *p = s->tcp.recvhead;
  int bytes = 0;

  while (p) {
    bytes += p->len;
    p = p->next;
  }

  return bytes;
}

static struct socket *accept_connection(struct tcp_pcb *pcb) {
  struct socket *s;

  if (socket(AF_INET, SOCK_STREAM, IPPROTO_TCP, &s) < 0) return NULL;

  s->state = SOCKSTATE_CONNECTED;
  s->tcp.pcb = pcb;
  tcp_arg(pcb, s);
  tcp_recv(pcb, recv_tcp);
  tcp_sent(pcb, sent_tcp);
  tcp_err(pcb, err_tcp);

  set_io_event(&s->iob, IOEVT_WRITE);

  return s;
}

static err_t accept_tcp(void *arg, struct tcp_pcb *newpcb, err_t err) {
  struct socket *s = arg;
  struct socket *newsock;
  struct sockreq *req = s->waithead;

  while (req) {
    if (req->type == SOCKREQ_ACCEPT) {
      if (err < 0) {
        release_socket_request(req, err);
        return 0;
      }

      req->newsock = accept_connection(newpcb);
      release_socket_request(req, err);
      return 0;
    }

    req = req->next;
  }

  if (s->tcp.numpending < s->tcp.backlog) {
    if (err < 0) return err;
    newsock = accept_connection(newpcb);
    if (!newsock) return -ENOMEM;

    s->tcp.pending[s->tcp.numpending++] = newsock;
    set_io_event(&s->iob, IOEVT_ACCEPT);

    return 0;
  }

  return -EABORT;
}

static err_t connected_tcp(void *arg, struct tcp_pcb *pcb, err_t err) {
  struct socket *s = arg;
  struct sockreq *req = s->waithead;

  while (req) {
    if (req->type == SOCKREQ_CONNECT) {
      s->state = SOCKSTATE_CONNECTED;
      set_io_event(&s->iob, IOEVT_WRITE);
      release_socket_request(req, err);
      return 0;
    }

    req = req->next;
  }

  s->state = SOCKSTATE_CONNECTED;
  set_io_event(&s->iob, IOEVT_CONNECT | IOEVT_WRITE);
  
  return 0;
}

static err_t recv_tcp(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err) {
  struct socket *s = arg;
  struct sockreq *req;
  struct sockreq *next;
  int bytes;
  int waitrecv;
  int bytesrecv = 0;

  if (p) {
    if (s->tcp.recvtail) {
      pbuf_chain(s->tcp.recvtail, p);
      s->tcp.recvtail = p;
    } else {
      s->tcp.recvhead = p;
      s->tcp.recvtail = p;
    }
  } else {
    s->state = SOCKSTATE_CLOSING;
    set_io_event(&s->iob, IOEVT_CLOSE);
  }

  if (s->state == SOCKSTATE_CLOSING && s->tcp.recvhead == NULL) {
    req = s->waithead;
    while (req) {
      next = req->next;
      if (req->type == SOCKREQ_RECV || req->type == SOCKREQ_WAITRECV) release_socket_request(req, 0);
      req = next;
    }

    return 0;
  }

  while (1) {
    if (!s->tcp.recvhead) break;

    req = s->waithead;
    waitrecv = 0;

    while (req) {
      if (req->type == SOCKREQ_RECV) break;
      if (req->type == SOCKREQ_WAITRECV) waitrecv++;
      req = req->next;
    }

    if (!req) {
      if (waitrecv) {
        req = s->waithead;
        while (req) {
          next = req->next;
          if (req->type == SOCKREQ_WAITRECV) release_socket_request(req, 0);
          req = next;
        }
      }

      break;
    }

    bytes = fetch_rcvbuf(s, req->msg->msg_iov, req->msg->msg_iovlen);
    if (bytes > 0) {
      bytesrecv += bytes;
      req->rc += bytes;
      release_socket_request(req, req->rc);
    }
  }

  if (bytesrecv) tcp_recved(pcb, bytesrecv);
  
  if (s->tcp.recvhead) {
    set_io_event(&s->iob, IOEVT_READ);
  } else {
    clear_io_event(&s->iob, IOEVT_READ);
  }

  return 0;
}

static err_t poll_tcp(void *arg, struct tcp_pcb *pcb) {
  struct socket *s = arg;
  return 0;
}

static err_t sent_tcp(void *arg, struct tcp_pcb *pcb, unsigned short len) {
  struct socket *s = arg;
  struct sockreq *req;
  int rc;

  while (1) {
    if (tcp_sndbuf(pcb) == 0) break;

    req = s->waithead;
    while (req) {
      if (req->type == SOCKREQ_SEND) break;
      req = req->next;
    }

    if (!req) break;

    rc = fill_sndbuf(s, req->msg->msg_iov, req->msg->msg_iovlen);
    if (rc < 0) {
      release_socket_request(req, rc);
      return rc;
    }

    req->rc += rc;

    if (get_iovec_size(req->msg->msg_iov, req->msg->msg_iovlen) == 0) release_socket_request(req, req->rc);
  }

  if (tcp_sndbuf(pcb) > 0) {
    set_io_event(&s->iob, IOEVT_WRITE);
  } else {
    clear_io_event(&s->iob, IOEVT_WRITE);
  }

  return 0;
}

static void err_tcp(void *arg, err_t err) {
  struct socket *s = arg;
  struct sockreq *req = s->waithead;
  struct sockreq *next;

  s->tcp.pcb = NULL;
  while (req) {
    next = req->next;
    release_socket_request(req, err);
    req = next;
  }

  set_io_event(&s->iob, err == -ERST ? IOEVT_CLOSE : IOEVT_ERROR);
}

static err_t alloc_pcb(struct socket *s) {
  s->tcp.pcb = tcp_new();
  if (!s->tcp.pcb) return -ENOMEM;
  
  tcp_arg(s->tcp.pcb, s);
  tcp_recv(s->tcp.pcb, recv_tcp);
  tcp_sent(s->tcp.pcb, sent_tcp);
  tcp_err(s->tcp.pcb, err_tcp);

  return 0;
}

static int tcpsock_accept(struct socket *s, struct sockaddr *addr, int *addrlen, struct socket **retval) {
  int rc;
  struct sockreq req;
  struct tcp_pcb *pcb;
  struct socket *newsock;
  struct sockaddr_in *sin;

  if (s->state != SOCKSTATE_LISTENING) return -EINVAL;

  if (s->tcp.numpending == 0) {
    if (s->flags & SOCK_NBIO) return -EAGAIN;

    rc = submit_socket_request(s, &req, SOCKREQ_ACCEPT, NULL, INFINITE);
    if (rc < 0) return rc;
    newsock = req.newsock;
  } else {
    newsock = s->tcp.pending[0];
    if (--s->tcp.numpending > 0) {
      memmove(s->tcp.pending, s->tcp.pending + 1, s->tcp.numpending * sizeof(struct socket *));
    }

    if (s->tcp.numpending == 0) clear_io_event(&s->iob, IOEVT_ACCEPT);
  }

  if (addr) {
    pcb = newsock->tcp.pcb;

    sin = (struct sockaddr_in *) addr;
    sin->sin_family = AF_INET;
    sin->sin_port = htons(pcb->remote_port);
    sin->sin_addr.s_addr = pcb->remote_ip.addr;
  }
  if (addrlen) *addrlen = sizeof(struct sockaddr_in);

  *retval = newsock;
  return 0;
}

static int tcpsock_bind(struct socket *s, struct sockaddr *name, int namelen) {
  int rc;
  struct sockaddr_in *sin;

  if (!name) return -EFAULT;
  if (namelen < sizeof(struct sockaddr_in)) return -EFAULT;
  sin = (struct sockaddr_in *) name;
  if (sin->sin_family != AF_INET && sin->sin_family != AF_UNSPEC) return -EAFNOSUPPORT;

  if (s->state != SOCKSTATE_UNBOUND) return -EINVAL;

  if (!s->tcp.pcb) {
    rc = alloc_pcb(s);
    if (rc < 0) return rc;
  }

  rc = tcp_bind(s->tcp.pcb, (struct ip_addr *) &sin->sin_addr, ntohs(sin->sin_port));
  if (rc < 0) return rc;

  s->state = SOCKSTATE_BOUND;
  return 0;
}

static int tcpsock_close(struct socket *s) {
  int n;
  struct sockreq *req;
  struct sockreq *next;

  s->state = SOCKSTATE_CLOSED;
  req = s->waithead;
  while (req) {
    next = req->next;
    release_socket_request(req, -EABORT);
    req = next;
  }
  
  set_io_event(&s->iob, IOEVT_CLOSE);

  if (s->tcp.pcb) {
    tcp_arg(s->tcp.pcb, NULL);
    tcp_sent(s->tcp.pcb, NULL);
    tcp_recv(s->tcp.pcb, NULL);
    tcp_accept(s->tcp.pcb, NULL);
    tcp_err(s->tcp.pcb, NULL);

    if (s->tcp.pcb->state == LISTEN) {
      tcp_close(s->tcp.pcb);
      for (n = 0; n < s->tcp.numpending; n++) tcpsock_close(s->tcp.pending[n]); 
      kfree(s->tcp.pending);
    } else {
      if (tcp_close(s->tcp.pcb) < 0) {
        tcp_abort(s->tcp.pcb);
      }
    }
  }

  if (s->tcp.recvhead) pbuf_free(s->tcp.recvhead);

  return 0;
}

static int tcpsock_connect(struct socket *s, struct sockaddr *name, int namelen) {
  int rc;
  struct sockaddr_in *sin;
  struct sockreq req;

  if (!name) return -EFAULT;
  if (namelen < sizeof(struct sockaddr_in)) return -EFAULT;
  sin = (struct sockaddr_in *) name;
  if (sin->sin_family != AF_INET && sin->sin_family != AF_UNSPEC) return -EAFNOSUPPORT;

  if (s->state == SOCKSTATE_CONNECTED) return -EISCONN;
  if (s->state == SOCKSTATE_CONNECTING) return -EALREADY;
  if (s->state != SOCKSTATE_BOUND && s->state != SOCKSTATE_UNBOUND) return -EINVAL;

  if (!s->tcp.pcb) {
    rc = alloc_pcb(s);
    if (rc < 0) return rc;
  }

  s->state = SOCKSTATE_CONNECTING;

  rc = tcp_connect(s->tcp.pcb, (struct ip_addr *) &sin->sin_addr, ntohs(sin->sin_port), connected_tcp);
  if (rc < 0) return rc;

  if (s->state != SOCKSTATE_CONNECTED) {
    if (s->flags & SOCK_NBIO) return -EAGAIN;
  
    rc = submit_socket_request(s, &req, SOCKREQ_CONNECT, NULL, INFINITE);
    if (rc < 0) return rc;
  }

  return 0;
}

static int tcpsock_getpeername(struct socket *s, struct sockaddr *name, int *namelen) {
  struct sockaddr_in *sin;

  if (!namelen) return -EFAULT;
  if (*namelen < sizeof(struct sockaddr_in)) return -EFAULT;
  if (s->state != SOCKSTATE_CONNECTED) return -ENOTCONN;
  if (!s->tcp.pcb) return -ENOTCONN;

  sin = (struct sockaddr_in *) name;
  sin->sin_family = AF_INET;
  sin->sin_port = htons(s->tcp.pcb->remote_port);
  sin->sin_addr.s_addr = s->tcp.pcb->remote_ip.addr;

  *namelen = sizeof(struct sockaddr_in);
  return 0;
}

static int tcpsock_getsockname(struct socket *s, struct sockaddr *name, int *namelen) {
  struct sockaddr_in *sin;

  if (!namelen) return -EFAULT;
  if (*namelen < sizeof(struct sockaddr_in)) return -EFAULT;
  if (s->state != SOCKSTATE_CONNECTED && s->state != SOCKSTATE_BOUND) return -ENOTCONN;
  if (!s->tcp.pcb) return -ENOTCONN;

  sin = (struct sockaddr_in *) name;
  sin->sin_family = AF_INET;
  sin->sin_port = htons(s->tcp.pcb->local_port);
  sin->sin_addr.s_addr = s->tcp.pcb->local_ip.addr;

  *namelen = sizeof(struct sockaddr_in);
  return 0;
}

static int tcpsock_getsockopt(struct socket *s, int level, int optname, char *optval, int *optlen) {
  return -ENOSYS;
}

static int tcpsock_ioctl(struct socket *s, int cmd, void *data, size_t size) {
  unsigned int timeout;
  int rc;
  struct sockreq req;
  
  switch (cmd) {
    case SIOWAITRECV:
      if (!data || size != 4) return -EFAULT;
      timeout = *(unsigned int *) data;
      if (s->state != SOCKSTATE_CONNECTED) return -ENOTCONN;
      if (!s->tcp.pcb) return -ENOTCONN;
      if (s->tcp.recvhead != NULL) return 0;

      rc = submit_socket_request(s, &req, SOCKREQ_WAITRECV, NULL, timeout);
      if (rc < 0) return rc;
      break;

    case FIONREAD:
      if (!data || size != 4) return -EFAULT;
      *(int *) data = recv_ready(s);
      break;

    case FIONBIO:
      if (!data || size != 4) return -EFAULT;
      if (*(int *) data)
        s->flags |= SOCK_NBIO;
      else
        s->flags &= ~SOCK_NBIO;
      break;

    default:
      return -ENOSYS;
  }

  return 0;
}

static int tcpsock_listen(struct socket *s, int backlog) {
  if (s->state == SOCKSTATE_CONNECTED) return -EISCONN;
  if (s->state != SOCKSTATE_BOUND) return -EINVAL;

  s->tcp.backlog = backlog;
  s->tcp.pending = kmalloc(sizeof(struct socket *) * backlog);
  if (!s->tcp.pending) return -ENOMEM;

  s->tcp.pcb = tcp_listen(s->tcp.pcb);
  if (!s->tcp.pcb)  {
    kfree(s->tcp.pending);
    return -ENOMEM;
  }
  
  tcp_accept(s->tcp.pcb, accept_tcp);
  s->state = SOCKSTATE_LISTENING;

  return 0;
}

static int tcpsock_recvmsg(struct socket *s, struct msghdr *msg, unsigned int flags) {
  int rc;
  int size;
  struct sockaddr_in *sin;
  struct sockreq req;

  if (s->state != SOCKSTATE_CONNECTED && s->state != SOCKSTATE_CLOSING) return -ENOTCONN;
  if (!s->tcp.pcb) return -ERST;

  if (s->tcp.pcb && msg->msg_name) {
    if (msg->msg_namelen < sizeof(struct sockaddr_in)) return -EFAULT;
    sin = (struct sockaddr_in *) msg->msg_name;
    sin->sin_family = AF_INET;
    sin->sin_port = htons(s->tcp.pcb->remote_port);
    sin->sin_addr.s_addr = s->tcp.pcb->remote_ip.addr;
  }
  msg->msg_namelen = sizeof(struct sockaddr_in);

  size = get_iovec_size(msg->msg_iov, msg->msg_iovlen);
  if (size < 0) return -EINVAL;
  if (size == 0) return 0;

  rc = fetch_rcvbuf(s, msg->msg_iov, msg->msg_iovlen);
  if (rc < 0) return rc;

  if (!s->tcp.recvhead) clear_io_event(&s->iob, IOEVT_READ);

  if (rc > 0) {
    tcp_recved(s->tcp.pcb, rc);
    return rc;
  }
 
  if (s->state == SOCKSTATE_CLOSING) return 0;
  if (s->flags & SOCK_NBIO) return -EAGAIN;

  rc = submit_socket_request(s, &req, SOCKREQ_RECV, msg, s->rcvtimeo);
  if (rc < 0) return rc;

  return rc; 
}

static int tcpsock_sendmsg(struct socket *s, struct msghdr *msg, unsigned int flags) {
  int rc;
  int size;
  struct sockreq req;
  int bytes;

  if (s->state != SOCKSTATE_CONNECTED) return -ENOTCONN;
  if (!s->tcp.pcb) return -ERST;

  size = get_iovec_size(msg->msg_iov, msg->msg_iovlen);
  if (size == 0) return 0;

  rc = fill_sndbuf(s, msg->msg_iov, msg->msg_iovlen);
  if (rc < 0) return rc;
  bytes = rc;

  if (tcp_sndbuf(s->tcp.pcb) == 0) clear_io_event(&s->iob, IOEVT_WRITE);

  if (bytes < size && (s->flags & SOCK_NBIO) == 0) {
    rc = submit_socket_request(s, &req, SOCKREQ_SEND, msg, s->sndtimeo);
    if (rc < 0) return rc;

    bytes += rc;
  }

  return bytes;
}

static int tcpsock_setsockopt(struct socket *s, int level, int optname, const char *optval, int optlen) {
  if (level == SOL_SOCKET) {
    struct linger *l;

    switch (optname) {
      case SO_LINGER:
        if (!optval || optlen != sizeof(struct linger)) return -EFAULT;
        l = (struct linger *) optval;
        s->tcp.lingertime = l->l_linger * HZ;
        if (l->l_onoff) {
          s->flags |= SOCK_LINGER;
        } else {
          s->flags &= ~SOCK_LINGER;
        }
        break;

      case SO_DONTLINGER:
        if (!optval || optlen != 4) return -EFAULT;
        if (*(int *) optval) {
          s->flags &= ~SOCK_LINGER;
        } else {
          s->flags |= SOCK_LINGER;
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
  } else if (level == IPPROTO_TCP) {
    switch (optname) {
      case TCP_NODELAY:
        if (!optval || optlen != 4) return -EFAULT;
        if (*(int *) optval) {
          s->flags |= SOCK_NODELAY;
        } else {
          s->flags &= ~SOCK_NODELAY;
        }
        break;

      default:
        return -ENOPROTOOPT;
    }
  } else {
    return -ENOPROTOOPT;
  }

  return 0;
}

static int tcpsock_shutdown(struct socket *s, int how) {
  return -ENOSYS;
}

static int tcpsock_socket(struct socket *s, int domain, int type, int protocol) {
  return 0;
}

struct sockops tcpops = {
  tcpsock_accept,
  tcpsock_bind,
  tcpsock_close,
  tcpsock_connect,
  tcpsock_getpeername,
  tcpsock_getsockname,
  tcpsock_getsockopt,
  tcpsock_ioctl,
  tcpsock_listen,
  tcpsock_recvmsg,
  tcpsock_sendmsg,
  tcpsock_setsockopt,
  tcpsock_shutdown,
  tcpsock_socket,
};
