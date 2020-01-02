#pragma once

#define SYNC_NET_PORT 8888
#define SYNC_NET_GROUP "239.255.255.250"
#define SYNC_NET_MSG_SIZE 256

class SyncNet
{
public:
  SyncNet()
  {
    m_ready = false;

    // SETUP READ SOCKET
    m_socket_read = socket(AF_INET, SOCK_DGRAM, 0);

    if (m_socket_read == INVALID_SOCKET)
    {
      int err = WSAGetLastError();
      OutputDebugStringA((LPCSTR)"read socket\n");
      return;
    }

    ULONG mode = 1;
    UINT yes = 1;

    if (ioctlsocket(m_socket_read, FIONBIO, &mode) != NO_ERROR)
    {
      int err = WSAGetLastError();
      OutputDebugStringA((LPCSTR)"read socket nonblocking\n");
      return;
    }

    if (setsockopt(m_socket_read, SOL_SOCKET, SO_REUSEADDR, (char*)&yes, sizeof(yes)) < 0)
    {
      int err = WSAGetLastError();
      OutputDebugStringA((LPCSTR)"reuse addr\n");
      return;
    }

    memset(&m_addr_read, 0, sizeof(m_addr_read));
    m_addr_read.sin_family = AF_INET;
    m_addr_read.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
    m_addr_read.sin_port = htons(SYNC_NET_PORT);

    if (bind(m_socket_read, (struct sockaddr*) & m_addr_read, sizeof(m_addr_read)) < 0)
    {
      int err = WSAGetLastError();
      OutputDebugStringA((LPCSTR)"bind\n");
      return;
    }

    struct ip_mreq mreq;
    mreq.imr_multiaddr.S_un.S_addr = inet_addr(SYNC_NET_GROUP);
    mreq.imr_interface.S_un.S_addr = htonl(INADDR_ANY);

    if (setsockopt(m_socket_read, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mreq, sizeof(mreq)) < 0)
    {
      int err = WSAGetLastError();
      OutputDebugStringA((LPCSTR)"ip add membership\n");
      return;
    }

    // SETUP WRITE SOCKET
    m_socket_write = socket(AF_INET, SOCK_DGRAM, 0);

    if (m_socket_write == INVALID_SOCKET)
    {
      int err = WSAGetLastError();
      OutputDebugStringA((LPCSTR)"write socket\n");
      return;
    }

    memset(&m_addr_write, 0, sizeof(m_addr_write));
    m_addr_write.sin_family = AF_INET;
    m_addr_write.sin_addr.s_addr = inet_addr(SYNC_NET_GROUP);
    m_addr_write.sin_port = htons(SYNC_NET_PORT);

    m_ready = true;
  }

  BOOL ready()
  {
    return(m_ready);
  }

  int write(const char* msg)
  {
    int sz_msg = (int)strlen(msg);
    int nbytes = 0;
    nbytes = sendto(m_socket_write, msg, sz_msg, 0, (struct sockaddr*) & m_addr_write, sizeof(m_addr_write));

    if (nbytes < 0)
    {
      int err = WSAGetLastError();
      OutputDebugStringA((LPCSTR)"write socket\n");
      return(-1);
    }

    ////OutputDebugStringA((LPCSTR)"WRITE: \n");
    //OutputDebugStringA((LPCSTR)msg);
    //OutputDebugStringA((LPCSTR)"\n");

    return(nbytes);
  }

  int read(char* msg, int sz_max)
  {
    int addrlen = sizeof(m_addr_read);
    int nbytes = 0;
    nbytes = recvfrom(m_socket_read, msg, sz_max, 0, (struct sockaddr*) & m_addr_read, &addrlen);

    if (nbytes < 0)
    {
      int err = WSAGetLastError();
      OutputDebugStringA((LPCSTR)"write socket\n");
      return(-1);
    }

    msg[nbytes] = '\0';

    //OutputDebugStringA((LPCSTR)"READ: \n");
    //OutputDebugStringA((LPCSTR)msg);
    //OutputDebugStringA((LPCSTR)"\n");

    return(nbytes);
  }

protected:
  SOCKET m_socket_read, m_socket_write;
  SOCKADDR_IN m_addr_read, m_addr_write;
  BOOL m_ready;
};