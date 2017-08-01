#ifndef SESSION_H
#define SESSION_H

#if defined(__WIN32__)
#include <winsock2.h>
#elif defined(__linux__)
#include <sys/types.h>
#include <sys/socket.h>
#include <mutex>
#endif


class Session
{
public:
    Session();
    ~Session();
#if 0
    void setSockID(int sockfd);

    void setIpAddress(unsigned int IpAddress);

    void setPort(unsigned short Port);

    void setSessionTimeout(int timeout);

    void setRecvDelay(int interval);

    void setListenSockfd(int sockfd);
#endif
    Session & operator = (Session &s2)
    {
        mClientSockfd    = s2.mClientSockfd;
        mClientIpAddress = s2.mClientIpAddress;
        mClientPort      = s2.mClientPort;
        mSessionTimeout  = s2.mSessionTimeout;

        return (*this);
    }



    int getClientSocketID() {return mClientSockfd;}

    void setClientSocketID(int __Sockfd) {mClientSockfd = __Sockfd;}


    unsigned int getClientIpAddress() {return mClientIpAddress;}

    void setClientIpAddress(unsigned int __IpAddress) {mClientIpAddress = __IpAddress;}


    unsigned short getClientPort() {return mClientPort;}

    void setClientPort(unsigned short __Port) {mClientPort = __Port;}


    int getSessionTimeout() {return mSessionTimeout;}

    void setSessionTimeout(int __Timeout) {mSessionTimeout = __Timeout;}


    int getServerListenSocket() {return mListenSockfd;}

    void setServerListenSockfd(int __Sockfd) {mListenSockfd = __Sockfd;}

    std::mutex& mutex() {
		return mutex_;
	}

private:

    int mClientSockfd;
    unsigned int mClientIpAddress;
    unsigned short mClientPort;
    int mSessionTimeout;
    int mListenSockfd;
    std::mutex mutex_;

};

#endif // SESSION_H
