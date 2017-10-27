#ifndef SOCKETSOURCE_H
#define SOCKETSOURCE_H

/*
    only for nonblock
*/


#include "session.h"

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

enum SockIO_type_e{
    READFDS_TYPE,
    WRITEFDS_TYPE,
    EXCEPTFDS_TYPE,
};



class SocketSource
{
public:
    SocketSource();

    /* socket methods */
    static int SocketCreate();

    static int SocketClose(int sockfd);

    static int SocketTryClose(int sockfd);

    static int SetNonBlock(int sockfd);

    static int SetBlock(int sockfd);

    static int IOMonitor(int *, int , int , int , fd_set &);

    static int CheckSockError(int sockfd);

    /* check how many valid bytes space can be read, usually for tcp */
    static int CheckRecvBuffer(int sockfd);

    /* return value : big endian ip address(only ipv4) */
    static unsigned int Domain2IpAddress(const char *host);

    static unsigned int GetLocalAddress(void);


    /* TCP */
    static int TcpServerCreate(const char *lhost, unsigned short lport);

    static int TcpAccept(int ServSocket, struct sockaddr *client, int timeout);

    static int TcpConnect(const char *host, unsigned short port, int timeout);

    static int TcpListen(int sockfd, int maxcnt);

    static int TcpRecv(int sockfd, unsigned char *data, int nbytes);

    static int TcpSend(int sockfd, unsigned char *data, int nbytes);

    static int TcpReadOneLine(int sockfd, char *data, int maxsize);


    /* UDP */
    static int UdpObjectCreate(const char *lhost, int lport);

    static int UdpRecv(int sockfd, struct sockaddr *from, unsigned char *data, int nbytes);

    static int UdpSend(int sockfd, struct sockaddr *dest, unsigned char *data, int nbytes);
};

#endif // SOCKETSOURCE_H
