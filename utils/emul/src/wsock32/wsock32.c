#include <os.h>
#include <win32.h>
#include <sockcall.h>

struct sockcallops *sockcalls;

//#define TRACEAPI

#ifdef TRACEAPI
#define TRACE(s) print_string(s " called\n");
#else
#define TRACE(s)
#endif

//#define sockapi __declspec(dllexport)
#define sockapi

#define WSADESCRIPTION_LEN      256
#define WSASYS_STATUS_LEN       128

typedef socket_t SOCKET;

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
  return sockcalls->recv(s, buf, len, flags);
}

sockapi int __stdcall winsock_send(SOCKET s, const char *buf, int len, int flags)
{
  TRACE("send");
  return sockcalls->send(s, buf, len, flags);
}

sockapi int __stdcall winsock_listen(SOCKET s, int backlog)
{
  TRACE("listen");
  return sockcalls->listen(s, backlog);
}

sockapi int __stdcall winsock_bind(SOCKET s, const struct sockaddr *name, int namelen)
{
  TRACE("bind");
  return sockcalls->bind(s, name, namelen);
}

sockapi SOCKET __stdcall winsock_accept(SOCKET s, struct sockaddr *addr, int *addrlen)
{
  TRACE("accept");
  return sockcalls->accept(s, addr, addrlen);
}

sockapi int __stdcall winsock_recvfrom(SOCKET s, char *buf, int len, int flags, struct sockaddr *from, int *fromlen)
{
  TRACE("recvfrom");
  return sockcalls->recvfrom(s, buf, len, flags, from, fromlen);
}

sockapi int __stdcall winsock_sendto(SOCKET s, const char *buf, int len, int flags, const struct sockaddr *to, int tolen)
{
  TRACE("sendto");
  return sockcalls->sendto(s, buf, len, flags, to, tolen);
}

sockapi int __stdcall winsock_select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, const struct timeval *timeout)
{
  TRACE("select");
  return sockcalls->select(nfds, readfds, writefds, exceptfds, timeout);
}

sockapi int __stdcall winsock_connect(SOCKET s, const struct sockaddr *name, int namelen)
{
  TRACE("connect");
  return sockcalls->connect(s, name, namelen);
}

sockapi int __stdcall winsock_closesocket(SOCKET s)
{
  TRACE("closesocket");
  return sockcalls->closesocket(s);
}

sockapi int __stdcall winsock_shutdown(SOCKET s, int how)
{
  TRACE("shutdown");
  return sockcalls->shutdown(s, how);
}

sockapi int __stdcall winsock_gethostname(char *name, int namelen)
{
  TRACE("gethostname");
  return sockcalls->gethostname(name, namelen);
}

sockapi struct hostent * __stdcall winsock_gethostbyaddr(const char *addr, int len, int type)
{
  TRACE("gethostbyaddr");
  return sockcalls->gethostbyaddr(addr, len, type);
}

sockapi struct hostent * __stdcall winsock_gethostbyname(const char *name)
{
  TRACE("gethostbyname");
  return sockcalls->gethostbyname(name);
}

sockapi unsigned short __stdcall winsock_htons(unsigned short hostshort)
{
  TRACE("htons");
  return sockcalls->htons(hostshort);
}

sockapi unsigned long __stdcall winsock_htonl(unsigned long hostlong)
{
  TRACE("htonl");
  return sockcalls->htonl(hostlong);
}

sockapi unsigned short __stdcall winsock_ntohs(unsigned short netshort)
{
  TRACE("ntohs");
  return sockcalls->ntohs(netshort);
}

sockapi unsigned long __stdcall winsock_ntohl(unsigned long netlong)
{
  TRACE("ntohl");
  return sockcalls->ntohl(netlong);
}

sockapi int __stdcall winsock_getsockopt(SOCKET s, int level, int optname, char *optval, int *optlen)
{
  TRACE("getsockopt");
  return sockcalls->getsockopt(s, level, optname, optval, optlen);
}

sockapi int winsock_setsockopt(SOCKET s, int level, int optname, const char *optval, int optlen)
{
  TRACE("setsockopt");
  return sockcalls->setsockopt(s, level, optname, optval, optlen);
}

sockapi struct protoent * __stdcall winsock_getprotobyname(const char *name)
{
  TRACE("getprotobyname");
  return sockcalls->getprotobyname(name);
}

sockapi int __stdcall winsock_getsockname(SOCKET s, struct sockaddr *name, int *namelen)
{
  TRACE("getsockname");
  return sockcalls->getsockname(s, name, namelen);
}

sockapi SOCKET __stdcall winsock_socket(int af, int type, int protocol)
{
  TRACE("socket");
  return sockcalls->socket(af, type, protocol);
}

#if 0
sockapi int __stdcall WSASendDisconnect(SOCKET s, LPWSABUF lpOutboundDisconnectData)
{
  panic("WSASendDisconnect not implemented");
  return -1;
}
#endif

sockapi int __stdcall winsock_ioctlsocket(SOCKET s, long cmd, unsigned long *argp)
{
  TRACE("ioctlsocket");
  return sockcalls->ioctlsocket(s, cmd, argp);
}

sockapi int __stdcall  __WSAFDIsSet(SOCKET s, fd_set *fd)
{
  TRACE("__WSAFDIsSet");
  return sockcalls->__WSAFDIsSet(s, fd);
}

sockapi int __stdcall WSAGetLastError()
{
  TRACE("WSAGetLastError");
  return sockcalls->WSAGetLastError();
}

sockapi int __stdcall WSACleanup()
{
  TRACE("WSACleanup");
  return 0;
}

int __stdcall DllMain(handle_t hmod, int reason, void *reserved)
{
  sockcalls = get_sock_calls();
  return TRUE;
}
