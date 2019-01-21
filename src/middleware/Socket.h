#ifndef SOCKETSOURCE_H
#define SOCKETSOURCE_H

/*
 only for nonblock
 */

#include "core/Session.h"

#if defined(__WIN32__)
#include <winsock2.h>
#elif defined(__linux__)
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <ifaddrs.h>
#include <net/if.h>
#endif
#include <unistd.h>

#ifndef INVALID_SOCKET
#define INVALID_SOCKET (int)(~0)
#endif

enum SockIO_type_e {
  READFDS_TYPE,
  WRITEFDS_TYPE,
  EXCEPTFDS_TYPE,
};

class Socket {
 public:
  /* socket methods */
  static int Create();

  static int Close(int sockfd);

  static int TryClose(int sockfd);

  static int SetNonBlock(int sockfd);

  static int SetBlock(int sockfd);

  static int Select(int *, int, int, int, fd_set *);

  static bool HasMessageArrived(int *sockfd_list, const int count,
                                const int timeout);

  static int CheckSockError(int sockfd);

  /* check how many valid bytes space can be read, usually for tcp */
  static int CheckRecvBuffer(int sockfd);

  /* return value : big endian ip address(only ipv4) */
  static unsigned int Domain2IpAddress(const char *host);

  static unsigned int GetLocalAddress(void);

  static unsigned int GetBindIpAddress(const int sockfd);

  static unsigned short GetBindPort(const int sockfd);

  /* TCP */
  static int TcpServerCreate(const char *lhost, unsigned short lport);

  static int TcpAccept(int ServSocket, struct sockaddr *client, int timeout);

  static int TcpConnect(const char *host, unsigned short port, int timeout);

  static int TcpListen(int sockfd, int maxcnt);

  static int ServerContact(int listen_sockfd, unsigned int* ip_address,
                           unsigned short* port);

  static int ClientContact(const unsigned int ip_address,
                           const unsigned short port);

  static int TcpRecv(int sockfd, unsigned char *data, int nbytes);

  static int TcpSend(int sockfd, unsigned char *data, int nbytes);

  static int TcpReadLine(int sockfd, char *data, int maxsize);

  /* UDP */
  static int UdpObjectCreate(const char *lhost, int lport);

  static int UdpRecv(int sockfd, struct sockaddr *from, unsigned char *data,
                     int nbytes);

  static int UdpSend(int sockfd, struct sockaddr *dest, unsigned char *data,
                     int nbytes);
};

#endif // SOCKETSOURCE_H
