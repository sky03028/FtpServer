/*
 * DefaultSession.h
 *
 *  Created on: 2019年1月17日
 *      Author: xueda
 */

#ifndef SRC_MODEL_DEFAULTSESSION_H_
#define SRC_MODEL_DEFAULTSESSION_H_

#include <string.h>
#include "core/Session.h"
#include "middleware/Socket.h"

namespace model {

class DefaultSession : public Session {
 public:
  DefaultSession() = default;
  virtual ~DefaultSession() = default;

  virtual int SendTo(Context* context) {
    if (sockfd() <= 0) {
      return -1;
    }
    int nbytes = 0;
    switch (context->content_type()) {
      case ContentType::kJson:
      case ContentType::kString: {
        nbytes = Send(sockfd(), (unsigned char*) context->content().c_str(),
                      context->content().size());
      }
        break;
      case ContentType::kBinary: {
      }
        break;
      default:
        break;
    }
    return nbytes;
  }

  virtual int RecvFrom(Context* context) {
    if (sockfd() <= 0) {
      return -1;
    }

    char buffer[4096];
    memset(buffer, 0, sizeof(buffer));
    int nbytes = Recv(sockfd(), (unsigned char*) buffer, sizeof(buffer));
    if (nbytes > 0) {
      context->set_content(std::string(buffer));
    }
    return nbytes;
  }

  virtual bool Create(const int type) {
    switch (type) {
      case ConnectionType::kClient:
        break;
      case ConnectionType::kServer: {
        int listen_sockfd = Socket::TcpServerCreate(nullptr, 0);
        if (listen_sockfd == -1) {
          return false;
        }
        Socket::TcpListen(listen_sockfd, 1);
        set_listen_sockfd(listen_sockfd);
      }
        break;
      default:
        break;
    }
    return true;
  }

  virtual bool Contact() {
    int result = -1;
    if (listen_sockfd() > 0) {
      unsigned int ip_address = 0;
      unsigned short port = 0;
      result = Socket::ServerContact(listen_sockfd(), &ip_address, &port);
      set_ip_address(ip_address);
      set_port(port);
    } else {
      result = Socket::ClientContact(ip_address(), port());
    }
    if (result <= 0) {
      return false;
    }
    return true;
  }

  virtual void Destory() {
    if (listen_sockfd() > 0) {
      Close(listen_sockfd());
      set_listen_sockfd(-1);
    }
    if (sockfd() > 0) {
      Close(sockfd());
      set_sockfd(-1);
    }
  }

 protected:
  bool HasMessageArrived(int* sockfd_list, int count, const int timeout) {
    return Socket::HasMessageArrived(sockfd_list, count, timeout);
  }

  int Send(const int sockfd, unsigned char *data, const int length) {
    return Socket::TcpSend(sockfd, data, length);
  }

  int Recv(const int sockfd, unsigned char *data, const int length) {
    return Socket::TcpRecv(sockfd, data, length);
  }

  void Close(const int sockfd) {
    if (sockfd > 0) {
      Socket::Close(sockfd);
    }
  }

 private:
  static const int kReadTimeout = 100;  // ms

};

}

#endif /* SRC_MODEL_DEFAULTSESSION_H_ */
