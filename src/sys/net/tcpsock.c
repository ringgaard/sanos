//
// tcpsock.c
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// TCP socket interface
//

#include <net/net.h>

static err_t accept_tcp(void *arg, struct tcp_pcb *newpcb, err_t err)
{
  struct socket *s = arg;
  struct sockreq *req = s->waithead;

  while (req)
  {
    if (req->type == SOCKREQ_ACCEPT)
    {
      req->pcb = newpcb;
      req->err = err;
      release_socket_request(req);
      return 0;
    }

    req = req->next;
  }

  if (s->tcp.numpending < s->tcp.backlog)
  {
    if (err < 0) return err;
    s->tcp.pending[s->tcp.numpending++] = newpcb;
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
      req->pcb = pcb;
      req->err = err;
      release_socket_request(req);
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
  int bytes;

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

  if (!s->tcp.recvhead)
  {
    req = s->waithead;
    while (req)
    {
      if (req->type == SOCKREQ_RECV) release_socket_request(req);
      req = req->next;
    }

    return 0;
  }

  while (1)
  {
    p = s->tcp.recvhead;
    if (!p) return 0;

    req = s->waithead;
    while (req)
    {
      if (req->type == SOCKREQ_RECV) break;
      req = req->next;
    }

    if (!req) return 0;

    bytes = req->len;
    if (bytes > p->len) bytes = p->len;

    memcpy(req->data, p->payload, bytes);
    req->data += bytes;
    req->len -= bytes;
    req->err += bytes;

    tcp_recved(pcb, (unsigned short) bytes);

    if (req->len == 0) release_socket_request(req);

    pbuf_header(p, -bytes);
    if (p->len == 0)
    {
      s->tcp.recvhead = pbuf_dechain(p);
      if (!s->tcp.recvhead) s->tcp.recvtail = NULL;
      pbuf_free(p);
    }
  }
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

    rc = tcp_write(pcb, req->data, (unsigned short) bytes, 1);
    if (rc < 0)
    {
      req->err = rc;
      release_socket_request(req);
      return rc;
    }

    req->data += bytes;
    req->len -= bytes;
    req->err += bytes;

    if (req->len == 0) release_socket_request(req);
  }
}

static void err_tcp(void *arg, err_t err)
{
  struct socket *s = arg;
  struct sockreq *req = s->waithead;

  while (req)
  {
    req->err = err;
    release_socket_request(req);
    req = req->next;
  }
}

static err_t alloc_pcb(struct socket *s)
{
  s->tcp.pcb = tcp_new();
  if (!s->tcp.pcb) return -ENOMEM;
  
  tcp_arg(s->tcp.pcb, s);
  tcp_recv(s->tcp.pcb, recv_tcp);
  tcp_sent(s->tcp.pcb, sent_tcp);
  tcp_poll(s->tcp.pcb, poll_tcp, 4);
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
    rc = submit_socket_request(s, &req, SOCKREQ_ACCEPT, NULL, 0);
    if (rc < 0) return rc;
    pcb = req.pcb;
  }
  else
  {
    pcb = s->tcp.pending[0];
    if (--s->tcp.numpending > 0) 
    {
      memmove(s->tcp.pending, s->tcp.pending + 1, s->tcp.numpending * sizeof(struct tcp_pcb *));
    }
  }

  rc = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP, &newsock);
  if (rc < 0)
  {
    tcp_abort(pcb);
    return -ENOMEM;
    
  }

  newsock->state = SOCKSTATE_CONNECTED;
  newsock->tcp.pcb = pcb;
  tcp_arg(pcb, newsock);

  if (addr) 
  {
    sin = (struct sockaddr_in *) addr;
    sin->sin_len = sizeof(struct sockaddr_in);
    sin->sin_family = AF_INET;
    sin->sin_port = pcb->dest_port;
    sin->sin_addr.s_addr = pcb->dest_ip.addr;
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
  if (sin->sin_family != AF_INET) return -EPROTONOSUPPORT;

  if (s->state != SOCKSTATE_UNBOUND) return -EINVAL;

  if (!s->tcp.pcb)
  {
    rc = alloc_pcb(s);
    if (rc < 0) return rc;
  }

  rc = tcp_bind(s->tcp.pcb, (struct ip_addr *) &sin->sin_addr, sin->sin_port);
  if (rc < 0) return rc;

  s->state = SOCKSTATE_BOUND;
  return 0;
}

static int tcpsock_close(struct socket *s)
{
  int n;
  struct sockreq *req;

  s->state = SOCKSTATE_CLOSED;
  req = s->waithead;
  while (req)
  {
    req->err = EABORT;
    release_socket_request(req);
    req = req->next;
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
      for (n = 0; n < s->tcp.numpending; n++) tcp_abort(s->tcp.pending[n]); 
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
  if (sin->sin_family != AF_INET) return -EINVAL;

  if (s->state != SOCKSTATE_BOUND && s->state != SOCKSTATE_UNBOUND) return -EINVAL;

  if (!s->tcp.pcb)
  {
    rc = alloc_pcb(s);
    if (rc < 0) return rc;
  }

  rc = tcp_connect(s->tcp.pcb, (struct ip_addr *) &sin->sin_addr, sin->sin_port, connected_tcp);
  if (rc < 0) return rc;

  s->state = SOCKSTATE_CONNECTING;
  
  rc = submit_socket_request(s, &req, SOCKREQ_CONNECT, NULL, 0);
  if (rc < 0) return rc;

  return 0;
}

static int tcpsock_listen(struct socket *s, int backlog)
{
  if (s->state != SOCKSTATE_BOUND) return -EINVAL;

  s->tcp.backlog = backlog;
  s->tcp.pending = kmalloc(sizeof(struct tcp_pcb *) * backlog);
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
  if (s->state != SOCKSTATE_CONNECTED) return -EINVAL;
  if (!size) return 0;

  bufp = (char *) data;
  left = size;
  while (s->tcp.recvhead && left > 0)
  {
    p = s->tcp.recvhead;

    if (left < p->len)
    {
      memcpy(bufp, p->payload, left);
      pbuf_header(p, -left);
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
  if (len > 0) return len;

  rc = submit_socket_request(s, &req, SOCKREQ_RECV, bufp, left);
  if (rc < 0) return rc;

  return len + rc; 
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
    sin->sin_port = s->tcp.pcb->dest_port;
    sin->sin_addr.s_addr = s->tcp.pcb->dest_ip.addr;
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
  if (s->state != SOCKSTATE_CONNECTED) return -EINVAL;
  if (!size) return 0;

  len = tcp_sndbuf(s->tcp.pcb);
  if (size <= len)
  {
    rc = tcp_write(s->tcp.pcb, data, (unsigned short) size, 1);
    if (rc < 0) return rc;
    return size;
  }
  else
  {
    if (len > 0)
    {
      rc = tcp_write(s->tcp.pcb, data, (unsigned short) len, 1);
      if (rc < 0) return rc;
    }

    rc = submit_socket_request(s, &req, SOCKREQ_SEND, ((char *) data) + len, size - len);
    if (rc < 0) return rc;

    return rc + len;
  }

  return 0;
}

static int tcpsock_sendto(struct socket *s, void *data, int size, unsigned int flags, struct sockaddr *to, int tolen)
{
  return tcpsock_send(s, data, size, flags);
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
  tcpsock_listen,
  tcpsock_recv,
  tcpsock_recvfrom,
  tcpsock_send,
  tcpsock_sendto,
  tcpsock_socket,
};
