//
// wsock32.c
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Windows Socket Library
//

//#define TRACEAPI

#include <os.h>
#include <win32.h>
#include <string.h>
#include <inifile.h>

//#define sockapi __declspec(dllexport)
#define sockapi

#define WSADESCRIPTION_LEN      256
#define WSASYS_STATUS_LEN       128

typedef handle_t SOCKET;
typedef struct iovec WSABUF;
typedef struct iovec *LPWSABUF;

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

sockapi int __stdcall WSAStartup(WORD wVersionRequested, LPWSADATA lpWSAData)
{
  TRACE("WSAStartup");
  // TODO: fill lpWSAData with data
  return 0;
}

sockapi int __stdcall winsock_recv(SOCKET s,char *buf, int len, int flags)
{
  int rc;

  TRACE("recv");
  rc = recv(s, buf, len, flags);
  if (rc < 0)
  {
    errno = rc;
    return -1;
  }

  return rc;
}

sockapi int __stdcall winsock_send(SOCKET s, const char *buf, int len, int flags)
{
  int rc;

  TRACE("send");
  rc = send(s, buf, len, flags);

  if (rc < 0)
  {
    errno = rc;
    return -1;
  }

  return rc;
}

sockapi int __stdcall winsock_listen(SOCKET s, int backlog)
{
  int rc;

  TRACE("listen");
  rc = listen(s, backlog);
  if (rc < 0)
  {
    errno = rc;
    return -1;
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
    errno = rc;
    return -1;
  }

  return rc;
}

sockapi SOCKET __stdcall winsock_accept(SOCKET s, struct sockaddr *addr, int *addrlen)
{
  int rc;

  TRACE("accept");
  rc = accept(s, addr, addrlen);
  if (rc < 0)
  {
    errno = rc;
    return -1;
  }

  return rc;
}

sockapi int __stdcall winsock_recvfrom(SOCKET s, char *buf, int len, int flags, struct sockaddr *from, int *fromlen)
{
  int rc;

  TRACE("recvfrom");
  
  rc = recvfrom(s, buf, len, flags, from, fromlen);
  if (rc < 0)
  {
    errno = rc;
    return -1;
  }

  return rc;
}

sockapi int __stdcall winsock_sendto(SOCKET s, const char *buf, int len, int flags, const struct sockaddr *to, int tolen)
{
  int rc;

  TRACE("sendto");
  rc = sendto(s, buf, len, flags, to, tolen);
  if (rc < 0)
  {
    errno = rc;
    return -1;
  }

  return rc;
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
  int rc;

  TRACE("connect");
  rc = connect(s, name, namelen);
  if (rc < 0)
  {
    errno = rc;
    return -1;
  }

  return rc;
}

sockapi int __stdcall winsock_closesocket(SOCKET s)
{
  int rc;

  TRACE("closesocket");
  rc = close(s);
  if (rc < 0)
  {
    errno = rc;
    return -1;
  }

  return rc;
}

sockapi int __stdcall winsock_shutdown(SOCKET s, int how)
{
  int rc;

  TRACE("shutdown");
  rc = shutdown(s, how);
  if (rc < 0)
  {
    errno = rc;
    return -1;
  }

  return rc;
}

sockapi int __stdcall winsock_gethostname(char *name, int namelen)
{
  int rc;

  TRACE("gethostname");
  rc = gethostname(name, namelen);
  if (rc < 0)
  {
    errno = rc;
    return -1;
  }

  return rc;
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
  int rc;

  TRACE("getsockopt");
  rc = getsockopt(s, level, optname, optval, optlen);
  if (rc < 0)
  {
    errno = rc;
    return -1;
  }

  return rc;
}

sockapi int winsock_setsockopt(SOCKET s, int level, int optname, const char *optval, int optlen)
{
  int rc;

  TRACE("setsockopt");
  rc = setsockopt(s, level, optname, optval, optlen);
  if (rc < 0)
  {
    errno = rc;
    return -1;
  }

  return rc;
}

sockapi struct protoent * __stdcall winsock_getprotobyname(const char *name)
{
  TRACE("getprotobyname");
  return getprotobyname(name);
}

sockapi int __stdcall winsock_getsockname(SOCKET s, struct sockaddr *name, int *namelen)
{
  int rc;

  TRACE("getsockname");
  rc = getsockname(s, name, namelen);
  if (rc < 0)
  {
    errno = rc;
    return -1;
  }

  return rc;
}

sockapi SOCKET __stdcall winsock_socket(int af, int type, int protocol)
{
  int rc;

  TRACE("socket");
  rc = socket(af, type, protocol);
  if (rc < 0)
  {
    errno = rc;
    return -1;
  }

  return rc;
}

sockapi int __stdcall winsock_ioctlsocket(SOCKET s, long cmd, unsigned long *argp)
{
  TRACE("ioctlsocket");
  syslog(LOG_DEBUG, "warning: winsock ioctlsocket not implemented\n");
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

sockapi int __stdcall WSARecvFrom
(
  SOCKET s,
  LPWSABUF lpBuffers,
  DWORD dwBufferCount,
  LPDWORD lpNumberOfBytesRecvd,
  LPDWORD lpFlags,
  struct sockaddr *lpFrom,
  LPINT lpFromlen,
  LPWSAOVERLAPPED lpOverlapped,
  LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
)
{
  TRACE("WSARecvFrom");
  panic("WSARecvFrom not implemented");
  return 0;
}

sockapi int __stdcall WSASendTo
(
  SOCKET s,
  LPWSABUF lpBuffers,
  DWORD dwBufferCount,
  LPDWORD lpNumberOfBytesSent,
  DWORD dwFlags,
  const struct sockaddr *lpTo,
  int iToLen,
  LPWSAOVERLAPPED lpOverlapped,
  LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
)
{
  TRACE("WSASendTo");
  panic("WSASendTo not implemented");
  return 0;
}

sockapi int __stdcall WSAIoctl
(
  SOCKET s,
  DWORD dwIoControlCode,
  LPVOID lpvInBuffer,
  DWORD cbInBuffer,
  LPVOID lpvOutBuffer,
  DWORD cbOutBuffer,
  LPDWORD lpcbBytesReturned,
  LPWSAOVERLAPPED lpOverlapped,
  LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
)
{
  TRACE("WSAIoctl");
  panic("WSAIoctl not implemented");
  return 0;
}

int __stdcall DllMain(handle_t hmod, int reason, void *reserved)
{
  return TRUE;
}
