#include <windows.h>
#include <stdio.h>

void panic(char *msg)
{
  printf("panic: %s\n", msg);
  exit(1);
}

void main(int argc, char *argv[])
{ 
  WSADATA wsadata;
  SOCKADDR_IN sin;
  int rc;
  SOCKET listener;
  SOCKET s;
  char buf[4096];
  int len;
  int bytes;

  // Initialize winsock
  rc = WSAStartup(MAKEWORD(2, 2), &wsadata);
  if (rc != 0) panic("error in WSAStartup");

  // Create listen socket
  listener = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
  if (listener == INVALID_SOCKET) panic("error in socket");

  // Bind the socket to the service port
  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = INADDR_ANY;
  sin.sin_port = htons(80);
  if (bind(listener, (LPSOCKADDR) &sin, sizeof(sin)) == SOCKET_ERROR) panic("error in bind");

  // Listen for incoming connections on the socket
  if (listen(listener, 5) == SOCKET_ERROR) panic("error in listen");

  // Wait and accept new connection from client
  while (1)
  {
    printf("waiting for connection...\n");
    s = accept(listener, NULL, NULL);
    if (s == INVALID_SOCKET) panic("error in accept");

    printf("connected\n");

    while ((len = recv(s, buf, 4096, 0)) > 0)
    {
      printf("recv %d bytes\n", len);
      bytes = send(s, buf, len, 0);
      if (bytes != len) printf("send %d bytes, %d bytes sent\n", len, bytes);
    }

    printf("closing connection\n");
    closesocket(s);
    printf("connection closed\n");
  }

  closesocket(listener);
}
