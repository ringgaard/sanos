//
// udpsock.c
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// UDP socket interface
//

#include <net/net.h>

static void recv_udp(void *arg, struct udp_pcb *pcb, struct pbuf *p, struct ip_addr *addr, unsigned short port)
{
  struct socket *s = arg;
  struct sockreq *req = s->waithead;
  int len;

  if (req)
  {
    if (req->len > p->len)
      len = req->len;
    else
      len = p->len;

    memcpy(req->data, p->payload, len);
    req->err = len;

    req->addr.sin_len = sizeof(struct sockaddr_in);
    req->addr.sin_family = AF_INET;
    req->addr.sin_port = port;
    req->addr.sin_addr.s_addr = addr->addr;

    pbuf_free(p);

    release_socket_request(req);
  }
  else if (s->udp.recvtail)
  {
    pbuf_chain(s->udp.recvtail, p);
    s->udp.recvtail = p;
  }
  else
  {
    s->udp.recvhead = p;
    s->udp.recvtail = p;
  }
}

static int udpsock_accept(struct socket *s, struct sockaddr *addr, int *addrlen, struct socket **retval)
{
  return -EINVAL;
}

static int udpsock_bind(struct socket *s, struct sockaddr *name, int namelen)
{
  int rc;
  struct sockaddr_in *sin;

  if (!name) return -EINVAL;
  if (namelen < sizeof(struct sockaddr_in)) return -EINVAL;
  sin = (struct sockaddr_in *) name;
  if (sin->sin_family != AF_INET) return -EPROTONOSUPPORT;

  if (!s->udp.pcb)
  {
    s->udp.pcb = udp_new();
    if (!s->udp.pcb) return -ENOMEM;
    udp_recv(s->udp.pcb, recv_udp, s);
  }

  rc = udp_bind(s->udp.pcb, (struct ip_addr *) &sin->sin_addr, sin->sin_port);
  if (rc < 0) return rc;

  s->state = SOCKSTATE_BOUND;
  return 0;
}

static int udpsock_close(struct socket *s)
{
  struct sockreq *req;

  s->state = SOCKSTATE_CLOSED;
  req = s->waithead;
  while (req)
  {
    req->err = EABORT;
    release_socket_request(req);
    req = req->next;
  }

  if (s->udp.pcb)
  {
    s->udp.pcb->recv_arg = NULL;
    udp_remove(s->udp.pcb);
  }

  if (s->udp.recvhead) pbuf_free(s->udp.recvhead);

  return 0;
}

static int udpsock_connect(struct socket *s, struct sockaddr *name, int namelen)
{
  int rc;
  struct sockaddr_in *sin;

  if (!name) return -EINVAL;
  if (namelen < sizeof(struct sockaddr_in)) return -EINVAL;
  sin = (struct sockaddr_in *) name;
  if (sin->sin_family != AF_INET) return -EPROTONOSUPPORT;
  if (s->state == SOCKSTATE_CLOSED) return -EINVAL;

  if (!s->udp.pcb)
  {
    s->udp.pcb = udp_new();
    if (!s->udp.pcb) return -ENOMEM;
    udp_recv(s->udp.pcb, recv_udp, s);
  }

  rc = udp_connect(s->udp.pcb, (struct ip_addr *) &sin->sin_addr, sin->sin_port);
  if (rc < 0) return rc;

  s->state = SOCKSTATE_CONNECTED;
  return 0;
}

static int udpsock_listen(struct socket *s, int backlog)
{
  return -EINVAL;
}

static int udpsock_recvfrom(struct socket *s, void *data, int size, unsigned int flags, struct sockaddr *from, int *fromlen)
{
  struct pbuf *p;
  struct udp_hdr *udphdr;
  struct ip_hdr *iphdr;
  void *msg;
  int len;
  struct sockaddr_in *sin;
  struct sockreq req;

  p = s->udp.recvhead;
  if (p)
  {
    s->udp.recvhead = pbuf_dechain(p);
    if (!s->udp.recvhead) s->udp.recvtail = NULL; 

    msg = p->payload;
    len = p->len;

    pbuf_header(p, UDP_HLEN);
    udphdr = p->payload;

    pbuf_header(p, IP_HLEN);
    iphdr = p->payload;

    if (len > size) len = size;
    memcpy(data, msg, len);

    if (from)
    {
      sin = (struct sockaddr_in *) from;
      sin->sin_len = sizeof(struct sockaddr_in);
      sin->sin_family = AF_INET;
      sin->sin_port = udphdr->src;
      sin->sin_addr.s_addr = iphdr->src.addr;
    }
    if (fromlen) *fromlen = sizeof(struct sockaddr_in);

    pbuf_free(p);

    return len;
  }
  else
  {
    len = submit_socket_request(s, &req, SOCKREQ_RECV, data, size);

    if (len >= 0)
    {
      if (from) memcpy(from, &req.addr, sizeof(struct sockaddr_in));
      if (fromlen) *fromlen = sizeof(struct sockaddr_in);
    }

    return len;
  }

  return 0;
}

static int udpsock_recv(struct socket *s, void *data, int size, unsigned int flags)
{
  return udpsock_recvfrom(s, data, size, flags, NULL, NULL);
}

static int udpsock_send(struct socket *s, void *data, int size, unsigned int flags)
{
  struct pbuf *p;
  int rc;

  if (!data) return -EFAULT;
  if (s->state != SOCKSTATE_CONNECTED) return -EINVAL;
  if (!size) return 0;

  p = pbuf_alloc(PBUF_TRANSPORT, size, PBUF_RW);
  if (!p) return -ENOMEM;

  memcpy(p->payload, data, size);
  
  rc = udp_send(s->udp.pcb, p, NULL);
  if (rc < 0)
  {
    pbuf_free(p);
    return rc;
  }
  
  pbuf_free(p);
  
  return size;
}

static int udpsock_sendto(struct socket *s, void *data, int size, unsigned int flags, struct sockaddr *to, int tolen)
{
  int rc;

  rc = udpsock_connect(s, to, tolen);
  if (rc < 0) return rc;

  rc = udpsock_send(s, data, size, flags);
  if (rc < 0) return rc;

  return 0;
}

static int udpsock_socket(struct socket *s, int domain, int type, int protocol)
{
  return 0;
}

struct sockops udpops =
{
  udpsock_accept,
  udpsock_bind,
  udpsock_close,
  udpsock_connect,
  udpsock_listen,
  udpsock_recv,
  udpsock_recvfrom,
  udpsock_send,
  udpsock_sendto,
  udpsock_socket,
};
