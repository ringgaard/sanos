//
// wsock32.c
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Windows Socket Library
//

#define TRACEAPI

#include <os.h>
#include <win32.h>
#include <string.h>
#include <inifile.h>

//#define sockapi __declspec(dllexport)
#define sockapi

#define WSADESCRIPTION_LEN      256
#define WSASYS_STATUS_LEN       128

typedef handle_t SOCKET;

typedef struct WSAData 
{
  WORD wVersion;
  WORD wHighVersion;
  char szDescription[WSADESCRIPTION_LEN + 1];
  char szSystemStatus[WSASYS_STATUS_LEN + 1];
  unsigned short iMaxSockets;
  unsigned short iMaxUdpDg;
  char *lpVendorInfo;
} WSADATA, *LPWSADATA; 

typedef struct _WSABUF 
{
  unsigned long len;
  char *buf;
} WSABUF, *LPWSABUF;

sockapi int __stdcall WSAStartup(WORD wVersionRequested, LPWSADATA lpWSAData)
{
  TRACE("WSAStartup");
  // TODO: fill lpWSAData with data
  return 0;
}

sockapi int __stdcall winsock_recv(SOCKET s,char *buf, int len, int flags)
{
  TRACE("recv");
  return recv(s, buf, len, flags);
}

sockapi int __stdcall winsock_send(SOCKET s, const char *buf, int len, int flags)
{
  TRACE("send");
  return send(s, buf, len, flags);
}

sockapi int __stdcall winsock_listen(SOCKET s, int backlog)
{
  int rc;

  TRACE("listen");
  rc = listen(s, backlog);
  if (rc < 0)
  {
    syslog(LOG_DEBUG, "error %d in listen\n", rc);
    dbgbreak();
  }

  return rc;
}

sockapi int __stdcall winsock_bind(SOCKET s, const struct sockaddr *name, int namelen)
{
  int rc;

  TRACE("bind");
  rc = bind(s, name, namelen);
  if (rc < 0)
  {
    syslog(LOG_DEBUG, "error %d in bind\n", rc);
    dbgbreak();
  }

  return rc;
}

sockapi SOCKET __stdcall winsock_accept(SOCKET s, struct sockaddr *addr, int *addrlen)
{
  TRACE("accept");
  return accept(s, addr, addrlen);
}

sockapi int __stdcall winsock_recvfrom(SOCKET s, char *buf, int len, int flags, struct sockaddr *from, int *fromlen)
{
  TRACE("recvfrom");
  return recvfrom(s, buf, len, flags, from, fromlen);
}

sockapi int __stdcall winsock_sendto(SOCKET s, const char *buf, int len, int flags, const struct sockaddr *to, int tolen)
{
  TRACE("sendto");
  return sendto(s, buf, len, flags, to, tolen);
}

sockapi int __stdcall winsock_select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, const struct timeval *timeout)
{
  unsigned int millisecs;
  int rc;

  TRACE("select");

  if (!readfds || readfds->fd_count != 1 || !timeout || writefds || exceptfds) panic("winsock select not implemented");

  millisecs = timeout->tv_sec * 1000 + timeout->tv_usec / 1000;

  rc = ioctl(readfds->fd_array[0], IOCTL_SOCKWAIT_RECV, &millisecs, 4);

  if (rc == -ETIMEOUT) return 0;
  if (rc == -EABORT) return -2;
  if (rc < 0) return -1;

  return 1;
}

sockapi int __stdcall winsock_connect(SOCKET s, const struct sockaddr *name, int namelen)
{
  TRACE("connect");
  return connect(s, name, namelen);
}

sockapi int __stdcall winsock_closesocket(SOCKET s)
{
  TRACE("closesocket");
  return close(s);
}

sockapi int __stdcall winsock_shutdown(SOCKET s, int how)
{
  TRACE("shutdown");
  return shutdown(s, how);
}

sockapi int __stdcall winsock_gethostname(char *name, int namelen)
{
  TRACE("gethostname");
  return gethostname(name, namelen);
}

sockapi struct hostent * __stdcall winsock_gethostbyaddr(const char *addr, int len, int type)
{
  TRACE("gethostbyaddr");
  return gethostbyaddr(addr, len, type);
}

sockapi struct hostent * __stdcall winsock_gethostbyname(const char *name)
{
  TRACE("gethostbyname");
  return gethostbyname(name);
}

sockapi unsigned short __stdcall winsock_htons(unsigned short hostshort)
{
  TRACE("htons");
  return htons(hostshort);
}

sockapi unsigned long __stdcall winsock_htonl(unsigned long hostlong)
{
  TRACE("htonl");
  return htonl(hostlong);
}

sockapi unsigned short __stdcall winsock_ntohs(unsigned short netshort)
{
  TRACE("ntohs");
  return ntohs(netshort);
}

sockapi unsigned long __stdcall winsock_ntohl(unsigned long netlong)
{
  TRACE("ntohl");
  return ntohl(netlong);
}

sockapi int __stdcall winsock_getsockopt(SOCKET s, int level, int optname, char *optval, int *optlen)
{
  TRACE("getsockopt");
  return getsockopt(s, level, optname, optval, optlen);
}

sockapi int winsock_setsockopt(SOCKET s, int level, int optname, const char *optval, int optlen)
{
  TRACE("setsockopt");
  return setsockopt(s, level, optname, optval, optlen);
}

sockapi struct protoent * __stdcall winsock_getprotobyname(const char *name)
{
  TRACE("getprotobyname");
  return getprotobyname(name);
}

sockapi int __stdcall winsock_getsockname(SOCKET s, struct sockaddr *name, int *namelen)
{
  TRACE("getsockname");
  return getsockname(s, name, namelen);
}

sockapi SOCKET __stdcall winsock_socket(int af, int type, int protocol)
{
  TRACE("socket");
  return socket(af, type, protocol);
}

sockapi int __stdcall winsock_ioctlsocket(SOCKET s, long cmd, unsigned long *argp)
{
  TRACE("ioctlsocket");
  panic("winsock ioctlsocket not implemented");
  //return ioctl(s, cmd, argp, 0);
  return 0;
}

sockapi int __stdcall  __WSAFDIsSet(SOCKET s, fd_set *fd)
{
  TRACE("__WSAFDIsSet");
  panic("winsock __WSAFDIsSet not implemented");
  //return sockcalls->__WSAFDIsSet(s, fd);
  return 0;
}

sockapi int __stdcall WSAGetLastError()
{
  TRACE("WSAGetLastError");
  return gettib()->err;
}

sockapi int __stdcall WSACleanup()
{
  TRACE("WSACleanup");
  return 0;
}

int __stdcall DllMain(handle_t hmod, int reason, void *reserved)
{
  return TRUE;
}
