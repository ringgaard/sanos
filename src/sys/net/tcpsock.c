//
// tcpsock.c
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// TCP socket interface
//

#include <net/net.h>

static err_t recv_tcp(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err);
static err_t sent_tcp(void *arg, struct tcp_pcb *pcb, unsigned short len);
static void err_tcp(void *arg, err_t err);

static struct socket *accept_connection(struct tcp_pcb *pcb)
{
  struct socket *s;

  if (socket(AF_INET, SOCK_STREAM, IPPROTO_TCP, &s) < 0) return NULL;

  s->state = SOCKSTATE_CONNECTED;
  s->tcp.pcb = pcb;
  tcp_arg(pcb, s);
  tcp_recv(pcb, recv_tcp);
  tcp_sent(pcb, sent_tcp);
  tcp_err(pcb, err_tcp);

  return s;
}

static err_t accept_tcp(void *arg, struct tcp_pcb *newpcb, err_t err)
{
  struct socket *s = arg;
  struct socket *newsock;
  struct sockreq *req = s->waithead;

  while (req)
  {
    if (req->type == SOCKREQ_ACCEPT)
    {
      if (err < 0)
      {
	release_socket_request(req, err);
	return 0;
      }

      req->newsock = accept_connection(newpcb);
      release_socket_request(req, err);
      return 0;
    }

    req = req->next;
  }

  if (s->tcp.numpending < s->tcp.backlog)
  {
    if (err < 0) return err;
    newsock = accept_connection(newpcb);
    if (newsock < 0) return -ENOMEM;

    s->tcp.pending[s->tcp.numpending++] = newsock;
    return 0;
  }

  return -EABORT;
}

static err_t connected_tcp(void *arg, struct tcp_pcb *pcb, err_t err)
{
  struct socket *s = arg;
  struct sockreq *req = s->waithead;

  while (req)
  {
    if (req->type == SOCKREQ_CONNECT)
    {
      s->state = SOCKSTATE_CONNECTED;
      release_socket_request(req, err);
      return 0;
    }

    req = req->next;
  }

  return -EABORT;
}

static err_t recv_tcp(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
{
  struct socket *s = arg;
  struct sockreq *req;
  struct sockreq *next;
  int bytes;
  int waitrecv;
  int bytesrecv = 0;

  if (p)
  {
    if (s->tcp.recvtail)
    {
      pbuf_chain(s->tcp.recvtail, p);
      s->tcp.recvtail = p;
    }
    else
    {
      s->tcp.recvhead = p;
      s->tcp.recvtail = p;
    }
  }
  else
    s->state = SOCKSTATE_CLOSING;

  if (s->state == SOCKSTATE_CLOSING && s->tcp.recvhead == NULL)
  {
    req = s->waithead;
    while (req)
    {
      next = req->next;
      if (req->type == SOCKREQ_RECV || req->type == SOCKREQ_WAITRECV) release_socket_request(req, 0);
      req = next;
    }

    return 0;
  }

  while (1)
  {
    p = s->tcp.recvhead;
    if (!p) break;

    req = s->waithead;
    waitrecv = 0;

    while (req)
    {
      if (req->type == SOCKREQ_RECV) break;
      if (req->type == SOCKREQ_WAITRECV) waitrecv++;
      req = req->next;
    }

    if (!req) 
    {
      if (waitrecv)
      {
	req = s->waithead;
	while (req)
	{
	  next = req->next;
	  if (req->type == SOCKREQ_WAITRECV) release_socket_request(req, 0);
	  req = next;
	}
      }

      break;
    }

    bytes = req->len;
    if (bytes > p->len) bytes = p->len;

    memcpy(req->data, p->payload, bytes);
    req->data += bytes;
    req->len -= bytes;
    req->err += bytes;

    bytesrecv += bytes;

    release_socket_request(req, req->err);

    pbuf_header(p, -bytes);
    if (p->len == 0)
    {
      s->tcp.recvhead = pbuf_dechain(p);
      if (!s->tcp.recvhead) s->tcp.recvtail = NULL;
      pbuf_free(p);
    }
  }

  if (bytesrecv) tcp_recved(pcb, bytesrecv);
  return 0;
}

static err_t poll_tcp(void *arg, struct tcp_pcb *pcb)
{
  struct socket *s = arg;
  return 0;
}

static err_t sent_tcp(void *arg, struct tcp_pcb *pcb, unsigned short len)
{
  struct socket *s = arg;
  struct sockreq *req;
  int rc;
  int bytes;

  while (1)
  {
    if (tcp_sndbuf(pcb) == 0) return 0;

    req = s->waithead;
    while (req)
    {
      if (req->type == SOCKREQ_SEND) break;
      req = req->next;
    }

    if (!req) return 0;

    bytes = req->len;
    if (bytes > tcp_sndbuf(pcb)) bytes = tcp_sndbuf(pcb);

    rc = tcp_write(pcb, req->data, (unsigned short) bytes);
    if (rc < 0)
    {
      release_socket_request(req, rc);
      return rc;
    }

    req->data += bytes;
    req->len -= bytes;
    req->err += bytes;

    if (req->len == 0) release_socket_request(req, req->err);
  }
}

static void err_tcp(void *arg, err_t err)
{
  struct socket *s = arg;
  struct sockreq *req = s->waithead;
  struct sockreq *next;

  while (req)
  {
    next = req->next;
    release_socket_request(req, err);
    req->socket->tcp.pcb = NULL;
    req = next;
  }
}

static err_t alloc_pcb(struct socket *s)
{
  s->tcp.pcb = tcp_new();
  if (!s->tcp.pcb) return -ENOMEM;
  
  tcp_arg(s->tcp.pcb, s);
  tcp_recv(s->tcp.pcb, recv_tcp);
  tcp_sent(s->tcp.pcb, sent_tcp);
  tcp_err(s->tcp.pcb, err_tcp);

  return 0;
}

static int tcpsock_accept(struct socket *s, struct sockaddr *addr, int *addrlen, struct socket **retval)
{
  int rc;
  struct sockreq req;
  struct tcp_pcb *pcb;
  struct socket *newsock;
  struct sockaddr_in *sin;

  if (s->state != SOCKSTATE_LISTENING) return -EINVAL;

  if (s->tcp.numpending == 0)
  {
    rc = submit_socket_request(s, &req, SOCKREQ_ACCEPT, NULL, 0, INFINITE);
    if (rc < 0) return rc;
    newsock = req.newsock;
  }
  else
  {
    newsock = s->tcp.pending[0];
    if (--s->tcp.numpending > 0) 
    {
      memmove(s->tcp.pending, s->tcp.pending + 1, s->tcp.numpending * sizeof(struct socket *));
    }
  }

  if (addr) 
  {
    pcb = newsock->tcp.pcb;

    sin = (struct sockaddr_in *) addr;
    sin->sin_len = sizeof(struct sockaddr_in);
    sin->sin_family = AF_INET;
    sin->sin_port = htons(pcb->remote_port);
    sin->sin_addr.s_addr = pcb->remote_ip.addr;
  }
  if (addrlen) *addrlen = sizeof(struct sockaddr_in);
  *retval = newsock;

  return 0;
}

static int tcpsock_bind(struct socket *s, struct sockaddr *name, int namelen)
{
  int rc;
  struct sockaddr_in *sin;

  if (!name) return -EINVAL;
  if (namelen < sizeof(struct sockaddr_in)) return -EINVAL;
  sin = (struct sockaddr_in *) name;
  if (sin->sin_family != AF_INET && sin->sin_family != AF_UNSPEC) return -EPROTONOSUPPORT;

  if (s->state != SOCKSTATE_UNBOUND) return -EINVAL;

  if (!s->tcp.pcb)
  {
    rc = alloc_pcb(s);
    if (rc < 0) return rc;
  }

  rc = tcp_bind(s->tcp.pcb, (struct ip_addr *) &sin->sin_addr, ntohs(sin->sin_port));
  if (rc < 0) return rc;

  s->state = SOCKSTATE_BOUND;
  return 0;
}

static int tcpsock_close(struct socket *s)
{
  int n;
  struct sockreq *req;
  struct sockreq *next;

  s->state = SOCKSTATE_CLOSED;
  req = s->waithead;
  while (req)
  {
    next = req->next;
    release_socket_request(req, -EABORT);
    req = next;
  }

  if (s->tcp.pcb)
  {
    tcp_arg(s->tcp.pcb, NULL);
    tcp_sent(s->tcp.pcb, NULL);
    tcp_recv(s->tcp.pcb, NULL);
    tcp_accept(s->tcp.pcb, NULL);
    tcp_poll(s->tcp.pcb, NULL, 0);
    tcp_err(s->tcp.pcb, NULL);

    if (s->tcp.pcb->state == LISTEN)
    {
      tcp_close(s->tcp.pcb);
      for (n = 0; n < s->tcp.numpending; n++) tcpsock_close(s->tcp.pending[n]); 
      kfree(s->tcp.pending);
    }
    else
    {
      if (tcp_close(s->tcp.pcb) < 0)
      {
	tcp_abort(s->tcp.pcb);
      }
    }
  }

  if (s->tcp.recvhead) pbuf_free(s->tcp.recvhead);

  return 0;
}

static int tcpsock_connect(struct socket *s, struct sockaddr *name, int namelen)
{
  int rc;
  struct sockaddr_in *sin;
  struct sockreq req;

  if (!name) return -EINVAL;
  if (namelen < sizeof(struct sockaddr_in)) return -EINVAL;
  sin = (struct sockaddr_in *) name;
  if (sin->sin_family != AF_INET && sin->sin_family != AF_UNSPEC) return -EPROTONOSUPPORT;

  if (s->state != SOCKSTATE_BOUND && s->state != SOCKSTATE_UNBOUND) return -EINVAL;

  if (!s->tcp.pcb)
  {
    rc = alloc_pcb(s);
    if (rc < 0) return rc;
  }

  rc = tcp_connect(s->tcp.pcb, (struct ip_addr *) &sin->sin_addr, ntohs(sin->sin_port), connected_tcp);
  if (rc < 0) return rc;

  s->state = SOCKSTATE_CONNECTING;
  
  rc = submit_socket_request(s, &req, SOCKREQ_CONNECT, NULL, 0, INFINITE);
  if (rc < 0) return rc;

  return 0;
}

static int tcpsock_getpeername(struct socket *s, struct sockaddr *name, int *namelen)
{
  struct sockaddr_in *sin;

  if (!namelen) return -EINVAL;
  if (*namelen < sizeof(struct sockaddr_in)) return -EINVAL;
  if (s->state != SOCKSTATE_CONNECTED) return -ECONN;
  if (!s->tcp.pcb) return -ECONN;

  sin = (struct sockaddr_in *) name;
  sin->sin_len = sizeof(struct sockaddr_in);
  sin->sin_family = AF_INET;
  sin->sin_port = htons(s->tcp.pcb->remote_port);
  sin->sin_addr.s_addr = s->udp.pcb->remote_ip.addr;

  *namelen = sizeof(struct sockaddr_in);
  return 0;
}

static int tcpsock_getsockname(struct socket *s, struct sockaddr *name, int *namelen)
{
  struct sockaddr_in *sin;

  if (!namelen) return -EINVAL;
  if (*namelen < sizeof(struct sockaddr_in)) return -EINVAL;
  if (s->state != SOCKSTATE_CONNECTED) return -ECONN;
  if (!s->tcp.pcb) return -ECONN;

  sin = (struct sockaddr_in *) name;
  sin->sin_len = sizeof(struct sockaddr_in);
  sin->sin_family = AF_INET;
  sin->sin_port = htons(s->udp.pcb->local_port);
  sin->sin_addr.s_addr = s->udp.pcb->local_ip.addr;

  *namelen = sizeof(struct sockaddr_in);
  return 0;
}

static int tcpsock_getsockopt(struct socket *s, int level, int optname, char *optval, int *optlen)
{
  return -ENOSYS;
}

static int tcpsock_ioctl(struct socket *s, int cmd, void *data, size_t size)
{
  unsigned int timeout;
  int rc;
  struct sockreq req;
  
  switch (cmd)
  {
    case IOCTL_SOCKWAIT_RECV:
      if (!data || size != 4) return -EFAULT;
      timeout = *(unsigned int *) data;
      if (s->state != SOCKSTATE_CONNECTED) return -ECONN;
      if (!s->tcp.pcb) return -ECONN;
      if (s->tcp.recvhead != NULL) return 0;

      rc = submit_socket_request(s, &req, SOCKREQ_WAITRECV, NULL, 0, timeout);
      if (rc < 0) 
      {
        kprintf("tcpsock_ioctl: error %d\n", rc);
	return rc;
      }

      break;

    default:
      return -ENOSYS;
  }

  return 0;
}

static int tcpsock_listen(struct socket *s, int backlog)
{
  if (s->state != SOCKSTATE_BOUND) return -EINVAL;

  s->tcp.backlog = backlog;
  s->tcp.pending = kmalloc(sizeof(struct socket *) * backlog);
  if (!s->tcp.pending) return -ENOMEM;

  s->tcp.pcb = tcp_listen(s->tcp.pcb);
  if (!s->tcp.pcb) 
  {
    kfree(s->tcp.pending);
    return -ENOMEM;
  }
  
  tcp_accept(s->tcp.pcb, accept_tcp);
  s->state = SOCKSTATE_LISTENING;

  return 0;
}

static int tcpsock_recv(struct socket *s, void *data, int size, unsigned int flags)
{
  int rc;
  char *bufp;
  int left;
  int len;
  struct pbuf *p;
  struct sockreq req;

  if (!data) return -EFAULT;
  if (s->state != SOCKSTATE_CONNECTED && s->state != SOCKSTATE_CLOSING) return -ECONN;
  if (!s->tcp.pcb) return -ECONN;
  if (size < 0) return -EINVAL;
  if (size == 0) return 0;

  bufp = (char *) data;
  left = size;
  while (s->tcp.recvhead && left > 0)
  {
    p = s->tcp.recvhead;

    if (left < p->len)
    {
      memcpy(bufp, p->payload, left);
      pbuf_header(p, -left);
      tcp_recved(s->tcp.pcb, size);
      return size;
    }
    else
    {
      memcpy(bufp, p->payload, p->len);
      bufp += p->len;
      left -= p->len;
      
      s->tcp.recvhead = pbuf_dechain(p);
      if (!s->tcp.recvhead) s->tcp.recvtail = NULL;

      pbuf_free(p);
    }
  }

  len = size - left;
  if (len > 0) 
  {
    tcp_recved(s->tcp.pcb, len);
    return len;
  }

  if (s->state == SOCKSTATE_CLOSING) return 0;

  rc = submit_socket_request(s, &req, SOCKREQ_RECV, bufp, size, INFINITE);
  if (rc < 0) 
  {
    kprintf("tcpsock_recv: error %d\n", rc);
    return rc;
  }

  return rc; 
}

static int tcpsock_recvfrom(struct socket *s, void *data, int size, unsigned int flags, struct sockaddr *from, int *fromlen)
{
  int rc;
  struct sockaddr_in *sin;

  rc = tcpsock_recv(s, data, size, flags);
  if (rc < 0) return rc;

  if (s->tcp.pcb && from) 
  {
    sin = (struct sockaddr_in *) from;
    sin->sin_len = sizeof(struct sockaddr_in);
    sin->sin_family = AF_INET;
    sin->sin_port = htons(s->tcp.pcb->remote_port);
    sin->sin_addr.s_addr = s->tcp.pcb->remote_ip.addr;
  }
  if (fromlen) *fromlen = sizeof(struct sockaddr_in);
  
  return rc;
}

static int tcpsock_send(struct socket *s, void *data, int size, unsigned int flags)
{
  int rc;
  int len;
  struct sockreq req;

  if (!data) return -EFAULT;
  if (s->state != SOCKSTATE_CONNECTED) return -ECONN;
  if (!s->tcp.pcb) return -ECONN;
  if (size < 0) return -EINVAL;
  if (size == 0) return 0;

  len = tcp_sndbuf(s->tcp.pcb);
  if (size <= len)
  {
    rc = tcp_write(s->tcp.pcb, data, (unsigned short) size);
    if (rc < 0) return rc;
    return size;
  }
  else
  {
    if (len > 0)
    {
      rc = tcp_write(s->tcp.pcb, data, (unsigned short) len);
      if (rc < 0) return rc;
    }

    rc = submit_socket_request(s, &req, SOCKREQ_SEND, ((char *) data) + len, size - len, INFINITE);
    if (rc < 0) return rc;

    return rc + len;
  }

  return 0;
}

static int tcpsock_sendto(struct socket *s, void *data, int size, unsigned int flags, struct sockaddr *to, int tolen)
{
  return tcpsock_send(s, data, size, flags);
}

static int tcpsock_setsockopt(struct socket *s, int level, int optname, const char *optval, int optlen)
{
  return -ENOSYS;
}

static int tcpsock_shutdown(struct socket *s, int how)
{
  return -ENOSYS;
}

static int tcpsock_socket(struct socket *s, int domain, int type, int protocol)
{
  return 0;
}

struct sockops tcpops =
{
  tcpsock_accept,
  tcpsock_bind,
  tcpsock_close,
  tcpsock_connect,
  tcpsock_getpeername,
  tcpsock_getsockname,
  tcpsock_getsockopt,
  tcpsock_ioctl,
  tcpsock_listen,
  tcpsock_recv,
  tcpsock_recvfrom,
  tcpsock_send,
  tcpsock_sendto,
  tcpsock_setsockopt,
  tcpsock_shutdown,
  tcpsock_socket,
};
