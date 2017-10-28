#include "socketsource.h"
#include "utils.h"
#include <iostream>
#include <errno.h>
#include <assert.h>
#include <string.h>
#if defined(__linux__)
#include <netdb.h>
#endif

int SocketSource::SetNonBlock(int sockfd) {

#if defined(__linux__)
  int flag;
  flag = fcntl(sockfd, F_GETFD, 0);
  fcntl(sockfd, F_SETFD, flag | O_NONBLOCK);
#elif defined(__WIN32__)
  int ioMode = 1; //0 block, 1 nonblock
  ioctlsocket(sockfd, FIONBIO, (u_long FAR*) &ioMode);
#endif

  return 0;

}

int SocketSource::SetBlock(int sockfd) {
#if defined(__linux__)
  int flag;
  flag = fcntl(sockfd, F_GETFD, 0);
  flag &= ~O_NONBLOCK;
  fcntl(sockfd, F_SETFD, flag);
#elif defined(__WIN32__)
  int ioMode = 0; //0 block, 1 nonblock
  ioctlsocket(sockfd, FIONBIO, (u_long FAR*) &ioMode);
#endif

  return 0;
}

int SocketSource::SocketCreate() {
  int sockfd;

#if defined(__WIN32__)
  WSADATA Ws;
  if (WSAStartup(MAKEWORD(2, 2), &Ws) != 0) {
    std::cout << "Windows system enable network device fail." << std::endl;
    return -1;
  }
#endif

  sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (sockfd < 0) {
    std::cout << "Create socket fail." << std::endl;
    return -1;
  }

  return sockfd;
}

int SocketSource::SocketClose(int sockfd) {

  if (sockfd >= 0) {
#if defined(__WIN32__)
    closesocket(sockfd);
#elif defined(__linux__)
    close(sockfd);
#endif
  }

  return 0;
}

/* get netif info */
unsigned int SocketSource::GetLocalAddress() {
  int sock_get_ip;
  unsigned int IpAddress = 0;

  struct sockaddr_in *local;
  struct ifreq ifr_ip;

  if ((sock_get_ip = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    return 0;
  }

  memset(&ifr_ip, 0, sizeof(ifr_ip));
  strncpy(ifr_ip.ifr_name, "eth0", sizeof(ifr_ip.ifr_name) - 1);

  if (ioctl(sock_get_ip, SIOCGIFADDR, &ifr_ip) < 0) {
    return 0;
  }

  local = (struct sockaddr_in *) &ifr_ip.ifr_addr;
  IpAddress = local->sin_addr.s_addr;

  close(sock_get_ip);

  return IpAddress;

}

unsigned int SocketSource::Domain2IpAddress(const char *host) {
  struct in_addr ipArray;
  unsigned int ipv4;

  if (host == NULL)
    return -1;

#if defined(__WIN32__)
  WORD v = MAKEWORD(1, 1);
  WSADATA wsaData;
  WSAStartup(v, &wsaData);
#endif

  /* only get the first ip */
  struct hostent *hostinfo = NULL;
  hostinfo = gethostbyname(host);
  if (hostinfo != NULL) {
    ipArray = *((struct in_addr *) hostinfo->h_addr);
  }

  ipv4 = ipArray.s_addr;

  return ipv4;

}

int SocketSource::CheckRecvBuffer(int sockfd) {
  int nbytes = 0;
  unsigned char buffer[4096];

  do {
    nbytes = recv(sockfd, (char *) buffer, sizeof(buffer), MSG_PEEK);
    if (nbytes == -1 && errno == EINTR)
      continue;

  } while (0);

  return nbytes;
}

int SocketSource::IOMonitor(int *SockQueue, int QueueSize, int timeout,
    int option, fd_set &optionfds) {
  struct timeval select_timeout;
  int result;

  unsigned int timestamp = Utils::getCurrentTime();

  if (timeout == 0) {
    timeout = 100;
  }

  int maxfds = Utils::GetMaxValue(SockQueue, QueueSize) + 1;

  select_timeout.tv_sec = 0;
  select_timeout.tv_usec = 100 * 1000;

  while (1) {
    if (Utils::getCurrentTime() - timestamp >= timeout)
      break;

    FD_ZERO(&optionfds);
    for (int index = 0; index < QueueSize; index++) {
      FD_SET(SockQueue[index], &optionfds);
    }

    switch (option) {
    case READFDS_TYPE:
      result = select(maxfds, &optionfds, NULL, NULL, &select_timeout);
      break;

    case WRITEFDS_TYPE:
      result = select(maxfds, NULL, &optionfds, NULL, &select_timeout);
      break;

    case EXCEPTFDS_TYPE:
      result = select(maxfds, NULL, NULL, &optionfds, &select_timeout);
      break;

    default:
      result = select(maxfds, &optionfds, NULL, NULL, &select_timeout);
      break;
    }

    if (result > 0) {
      return result;
    } else if (result == 0) {
      continue;
    }

  }

  return -1;
}

int SocketSource::CheckSockError(int sockfd) {
  int result;
  int errcode = 0;
  socklen_t errsize = sizeof(errcode);

  result = getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &errcode, &errsize);
  if (result < 0) {
    perror("getsockopt");
    return -1;
  }

  return errcode;
}

int SocketSource::TcpServerCreate(const char *lhost, unsigned short lport) {
  int listenfd;
  struct sockaddr_in ServAddr;

  listenfd = SocketCreate();
  assert(listenfd != -1);

  memset(&ServAddr, 0, sizeof(ServAddr));
  ServAddr.sin_family = AF_INET;
  ServAddr.sin_port = htons(lport);

  if (lhost != NULL) {

    if (INADDR_NONE == inet_addr(lhost)) {
      /* this host is not ip address string */
      struct hostent *phost;
      phost = gethostbyname(lhost);
      if (NULL == phost) {
        std::cout << strerror(errno) << std::endl;
        exit(1);
      }

      ServAddr.sin_addr = *(struct in_addr*) phost->h_addr;
    }
    else {
      ServAddr.sin_addr.s_addr = inet_addr(lhost);
    }
  }
  else {
    ServAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  }

  int opt = 1;
  /* re-use port */
  setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const char *) &opt,
      sizeof(opt));
  assert(
      0
          == bind(listenfd, (struct sockaddr * )&ServAddr,
              sizeof(struct sockaddr)));

  return listenfd;

}

int SocketSource::TcpListen(int sockfd, int maxcnt) {
  listen(sockfd, maxcnt);

  return 0;
}

int SocketSource::TcpAccept(int ServSocket, struct sockaddr *client,
    int timeout) {
  fd_set readfds;
  struct timeval select_timeout;
  int result;
  int client_sd;
  socklen_t socklen = sizeof(struct sockaddr);
  unsigned int timestamp = Utils::getCurrentTime();

  if (timeout == 0) {
    timeout = 500;
  }

  select_timeout.tv_sec = 0;
  select_timeout.tv_usec = 500 * 1000;

  do {

    if (Utils::getCurrentTime() - timestamp >= timeout) {
      break;
    }

    FD_ZERO(&readfds);
    FD_SET(ServSocket, &readfds);

    result = select(ServSocket + 1, &readfds, NULL, NULL, &select_timeout);
    if (result > 0) {
      if (FD_ISSET(ServSocket, &readfds)) {
        memset(client, 0, sizeof(struct sockaddr));
        client_sd = accept(ServSocket, client, &socklen);
        if (client_sd != INVALID_SOCKET) {
          SetNonBlock(client_sd);
          return client_sd;
        } else {
          continue;
        }
      }
    } else if (result == 0) {
      continue;
    }

  } while (1);

  return -1;
}

int SocketSource::TcpConnect(const char *host, unsigned short port,
    int timeout) {
  if (host == NULL)
    return -1;

  struct sockaddr_in remote;

  memset(&remote, 0, sizeof(remote));

  if (INADDR_NONE == inet_addr(host)) {
    /* this host is not ip address string */
    struct hostent *phost = NULL;
    phost = gethostbyname(host);
    if (NULL == phost) {
      std::cout << strerror(errno) << std::endl;
      return -1;
    }

    remote.sin_addr = *(struct in_addr*) phost->h_addr;
  }
  else {
    remote.sin_addr.s_addr = inet_addr(host);
  }

  if (remote.sin_addr.s_addr == 0)
    return -1;

  remote.sin_family = AF_INET;
  remote.sin_port = port;

  int sockfd = SocketCreate();
  if (sockfd < 0)
    return -1;

  bool is_connect_ok = false;

  fd_set fds;

  SetNonBlock(sockfd);

  remote.sin_addr.s_addr = inet_addr("192.168.0.102");

  do {

    if (0
        == connect(sockfd, (struct sockaddr *) &remote,
            sizeof(struct sockaddr))) {
      std::cout << "Connect ok. sockfd = " << sockfd << std::endl;
      is_connect_ok = true;
      break;
    } else {
      if (errno != EINPROGRESS)
        break;
    }

    if (IOMonitor(&sockfd, 1, timeout, WRITEFDS_TYPE, fds) <= 0)
      break;

    if (FD_ISSET(sockfd, &fds)) {
      if (CheckSockError(sockfd) != 0)
        break;
    }

    is_connect_ok = true;

  } while (0);

  if (is_connect_ok)
    return sockfd;

  SocketClose(sockfd);

  return -1;
}

int SocketSource::TcpRecv(int sockfd, unsigned char *data, int nbytes) {
  int rbytes;
  int total_bytes = 0;
  unsigned char *pData = data;

  do {

    rbytes = read(sockfd, pData, nbytes - total_bytes);
    if (rbytes == 0) {
      return 0;
    } else if (rbytes == -1) {
      if (errno == EINTR) {
        continue;
      } else {
        return -1;
      }
    }

    total_bytes += rbytes;
    pData += rbytes;

    if (total_bytes < nbytes)
      continue;
    else if (total_bytes == nbytes)
      break;

  } while (1);

  return total_bytes;
}

int SocketSource::TcpSend(int sockfd, unsigned char *data, int nbytes) {
  int wbytes;
  int total_bytes = 0;
  unsigned char *pData = data;

  do {

    wbytes = write(sockfd, pData, nbytes - total_bytes);
    if (wbytes == -1 && errno == EINTR) {
      continue;
    }

    total_bytes += wbytes;
    pData += wbytes;

    if (total_bytes < nbytes) {
      continue;
    } else if (total_bytes == nbytes) {
      break;
    }

  } while (1);

  return total_bytes;
}

int SocketSource::TcpReadOneLine(int sockfd, char *data, int maxsize) {
  int rbytes;
  int total_bytes = 0;
  char *pData = data;

  do {

    rbytes = read(sockfd, pData, maxsize - total_bytes);
    if (rbytes == 0) {
      return 0;
    } else if (rbytes == -1) {
      if (errno == EINTR) {
        continue;
      } else {
        return -1;
      }
    }

    total_bytes += rbytes;
    for (int index = 0; index < rbytes; index++) {
      if (pData[index] == '\r')
        return total_bytes;
    }
    pData += rbytes;

    if (total_bytes < maxsize)
      continue;

  } while (1);

  return -1;
}

int SocketSource::UdpObjectCreate(const char *lhost, int lport) {
  int opt = 1;
  int udp_sd;
  int ipv4;
  struct sockaddr_in local;

  udp_sd = SocketCreate();
  if (udp_sd < 0)
    return -1;

  memset(&local, 0, sizeof(struct sockaddr_in));

  if (inet_addr(lhost) == INADDR_NONE) {
    ipv4 = Domain2IpAddress(lhost);
  } else {
    ipv4 = inet_addr(lhost);
  }

  local.sin_family = AF_INET;
  local.sin_port = ntohs(lport);
  local.sin_addr.s_addr = ipv4;

  SetNonBlock(udp_sd);
  setsockopt(udp_sd, SOL_SOCKET, SO_REUSEADDR, (char *) &opt, sizeof(opt));

  if (0 != bind(udp_sd, (struct sockaddr *) &local, sizeof(struct sockaddr)))
    return -1;

  return udp_sd;
}

int SocketSource::UdpRecv(int sockfd, sockaddr *from, unsigned char *data,
    int nbytes) {
  int rbytes;
  int total_bytes = 0;
  socklen_t socklen = sizeof(struct sockaddr);
  unsigned char *pData = data;

  do {

    rbytes = recvfrom(sockfd, (char *) pData, nbytes - total_bytes, 0, from,
        &socklen);
    if (rbytes == 0) {
      return 0;
    } else if (rbytes == -1) {
      if (errno == EINTR)
        continue;
      else
        break;
    }

    total_bytes += rbytes;
    pData += rbytes;

    if (total_bytes < nbytes)
      continue;
    else if (total_bytes == nbytes) {
      return total_bytes;
    }

  } while (1);

  return -1;
}

int SocketSource::UdpSend(int sockfd, sockaddr *dest, unsigned char *data,
    int nbytes) {
  int wbytes;
  int total_bytes = 0;
  int socklen = sizeof(struct sockaddr);
  unsigned char *pData = data;

  do {

    wbytes = sendto(sockfd, (char *) pData, nbytes - total_bytes, 0, dest,
        socklen);
    if (wbytes == -1 && errno == EINTR)
      continue;

    total_bytes += wbytes;
    pData += wbytes;

    if (total_bytes < nbytes)
      continue;
    else if (total_bytes == nbytes)
      break;

  } while (1);

  return total_bytes;
}

