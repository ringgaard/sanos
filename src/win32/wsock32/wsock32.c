//
// wsock32.c
//
// Windows Socket Library
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

//#define TRACEAPI

#include <os.h>
#include <sys/types.h>
#include <win32.h>
#include <string.h>
#include <inifile.h>

//#define sockapi __declspec(dllexport)
#define sockapi

#define SIO_GET_INTERFACE_LIST  _IOCR('t', 127, unsigned long)

#define WSADESCRIPTION_LEN      256
#define WSASYS_STATUS_LEN       128

#define WSAERRBASE              10000

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

#define IFF_UP           0x00000001    // Interface is up
#define IFF_BROADCAST    0x00000002    // Broadcast is  supported
#define IFF_LOOPBACK     0x00000004    // Loopback interface
#define IFF_POINTTOPOINT 0x00000008    // Point-to-point interface
#define IFF_MULTICAST    0x00000010    // Multicast is supported

typedef struct _INTERFACE_INFO
{
  unsigned long iiFlags;               // Type and status of the interface
  struct sockaddr iiAddress;           // Interface address
  char filler1[8];
  struct sockaddr iiBroadcastAddress;  // Broadcast address
  char filler2[8];
  struct sockaddr iiNetmask;           // Network mask
  char filler3[8];
} INTERFACE_INFO;

sockapi int __stdcall WSAStartup(WORD wVersionRequested, LPWSADATA lpWSAData)
{
  TRACE("WSAStartup");

  if (lpWSAData)
  {
    lpWSAData->wVersion = 0x0101;
    lpWSAData->wHighVersion = 0x0101;
    strcpy(lpWSAData->szDescription, "Sanos Winsock 1.1");
    strcpy(lpWSAData->szSystemStatus, "Running");
    lpWSAData->iMaxSockets = 0xFFFF;
    lpWSAData->iMaxUdpDg = 1500 - 40 - 8;
    lpWSAData->lpVendorInfo = NULL;
  }

  return 0;
}

sockapi int __stdcall winsock_recv(SOCKET s,char *buf, int len, int flags)
{
  int rc;

  TRACE("recv");
  rc = recv(s, buf, len, flags);
  if (rc < 0)
  {
    errno = -rc;
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
    errno = -rc;
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
    errno = -rc;
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
    errno = -rc;
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
    errno = -rc;
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
    errno = -rc;
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
    errno = -rc;
    return -1;
  }

  return rc;
}

sockapi int __stdcall winsock_select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, const struct timeval *timeout)
{
#if 0
  unsigned int millisecs;
  int rc;

  TRACE("select");

  if (!readfds || readfds->count != 1 || !timeout || writefds || exceptfds) panic("winsock select not implemented");

  millisecs = timeout->tv_sec * 1000 + timeout->tv_usec / 1000;

  rc = ioctl(readfds->fd[0], SIOWAITRECV, &millisecs, 4);

  if (rc == -ETIMEOUT) return 0;
  if (rc == -EABORT) return -2;
  if (rc < 0) return -1;

  return 1;
#endif
  int rc;

  TRACE("select");

  rc = select(nfds, readfds, writefds, exceptfds, timeout);
  if (rc < 0)
  {
    if (rc == -ETIMEOUT) return 0;
    errno = -rc;
    return -1;
  }

  return rc;
}

sockapi int __stdcall winsock_connect(SOCKET s, const struct sockaddr *name, int namelen)
{
  int rc;

  TRACE("connect");
  rc = connect(s, name, namelen);
  if (rc < 0)
  {
    errno = -rc;
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
    errno = -rc;
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
    errno = -rc;
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
    errno = -rc;
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
    errno = -rc;
    return -1;
  }

  return rc;
}

sockapi int __stdcall winsock_setsockopt(SOCKET s, int level, int optname, const char *optval, int optlen)
{
  int rc;

  TRACE("setsockopt");

  if (level == SOL_SOCKET && optname == SO_REUSEADDR)
  {
    syslog(LOG_DEBUG, "setsockopt: SO_REUSEADDR ignored\n");
    return 0;
  }

  rc = setsockopt(s, level, optname, optval, optlen);
  if (rc < 0)
  {
    errno = -rc;
    syslog(LOG_DEBUG, "setsockopt level %d optname %d failed: %d\n", level, optname, rc);
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
    errno = -rc;
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
    errno = -rc;
    return -1;
  }

  return rc;
}

sockapi int __stdcall winsock_ioctlsocket(SOCKET s, long cmd, unsigned long *argp)
{
  int rc;

  TRACE("ioctlsocket");
  rc = ioctl(s, cmd, argp, 4);
  if (rc < 0)
  {
    errno = -rc;
    return -1;
  }

  return rc;

}

sockapi int __stdcall  __WSAFDIsSet(SOCKET s, fd_set *fd)
{
  TRACE("__WSAFDIsSet");
  return FD_ISSET(s, fd);
}

sockapi int __stdcall WSAGetLastError()
{
  int err = gettib()->errnum;

  TRACE("WSAGetLastError");
  
  if (err == 0) return 0;
  if (err == EAGAIN) return EWOULDBLOCK + WSAERRBASE;
  if (err == EBADF) return ENOTSOCK + WSAERRBASE;
  if (err < 45) return err + WSAERRBASE;
  if (err < 82) return (err - 10) + WSAERRBASE;
  if (err == EHOSTNOTFOUND) return 1001 + WSAERRBASE;
  if (err == ETRYAGAIN) return 1002 + WSAERRBASE;
  if (err == ENORECOVERY) return 1003 + WSAERRBASE;
  if (err == ENODATA) return 1004 + WSAERRBASE;

  return EIO;
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
  struct msghdr msg;
  int rc;

  TRACE("WSARecvFrom");
  
  if (lpOverlapped != NULL) panic("Overlapped I/O not implemented in WSARecvFrom");
  if (lpCompletionRoutine != NULL) panic("Completion routines not implemented in WSARecvFrom");

  msg.iov = lpBuffers;
  msg.iovlen = dwBufferCount;
  msg.name = lpFrom;
  msg.namelen = lpFromlen ? *lpFromlen : 0;

  rc = recvmsg(s, &msg, lpFlags ? *lpFlags : 0);
  if (rc < 0)
  {
    errno = -rc;
    return -1;
  }

  if (lpFromlen) *lpFromlen = msg.namelen;
  if (lpNumberOfBytesRecvd) *lpNumberOfBytesRecvd = rc;

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
  struct msghdr msg;
  int rc;

  TRACE("WSASendTo");
  
  if (lpOverlapped != NULL) panic("Overlapped I/O not implemented in WSASendTo");
  if (lpCompletionRoutine != NULL) panic("Completion routines not implemented in WSASendTo");

  msg.iov = lpBuffers;
  msg.iovlen = dwBufferCount;
  msg.name = (struct sockaddr *) lpTo;
  msg.namelen = iToLen;

  rc = sendmsg(s, &msg, dwFlags);
  if (rc < 0)
  {
    errno = -rc;
    return -1;
  }

  if (lpNumberOfBytesSent) *lpNumberOfBytesSent = rc;

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

  if (dwIoControlCode == SIO_GET_INTERFACE_LIST)
  {
    int i;
    INTERFACE_INFO *ifinfo = lpvOutBuffer;
    int numifs = cbOutBuffer / sizeof(INTERFACE_INFO);
    int bufsize = numifs * sizeof(struct ifcfg);
    struct ifcfg *iflist = malloc(bufsize);
    int rc = ioctl(s, SIOIFLIST, iflist, bufsize);
    if (rc < 0)
    {
      free(iflist);
      errno = -rc;
      return -1;
    }

    if (rc > bufsize)
    {
      errno = ENOBUFS;
      free(iflist);
      return -1;
    }

    numifs = rc / sizeof(struct ifcfg);
    memset(ifinfo, 0, numifs * sizeof(INTERFACE_INFO));

    for (i = 0; i < numifs; i++)
    {
      ifinfo[i].iiFlags = IFF_BROADCAST;
      if (iflist[i].flags & IFCFG_UP) ifinfo[i].iiFlags |= IFF_UP;
      if (iflist[i].flags & IFCFG_LOOPBACK) ifinfo[i].iiFlags |= IFF_LOOPBACK;
      memcpy(&ifinfo[i].iiAddress, &iflist[i].addr, sizeof(struct sockaddr));
      memcpy(&ifinfo[i].iiNetmask, &iflist[i].netmask, sizeof(struct sockaddr));
      memcpy(&ifinfo[i].iiBroadcastAddress, &iflist[i].broadcast, sizeof(struct sockaddr));
    }

    if (lpcbBytesReturned) *lpcbBytesReturned = numifs * sizeof(INTERFACE_INFO);
    free(iflist);
    return 0;
  }
  else
    panic("WSAIoctl not implemented");

  return 0;
}

sockapi int __stdcall WSASendDisconnect
(
  SOCKET s,
  LPWSABUF lpOutboundDisconnectData
)
{
  TRACE("WSASendDisconnect");
  panic("WSASendDisconnect not implemented");
  return -1;
}

int __stdcall DllMain(handle_t hmod, int reason, void *reserved)
{
  return TRUE;
}
