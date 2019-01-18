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

  virtual void Create(const int type) {
    int result = 0;
    switch (type) {
      case ConnectionType::kClient: {
        result = Socket::CreateClient(ip_address(), port());
      }
        break;
      case ConnectionType::kServer: {
        int listen_sockfd = 0;
        result = Socket::CreateServer(&listen_sockfd);
        set_listen_sockfd(listen_sockfd);
      }
        break;
      default:
        break;
    }

    if (result > 0) {
      set_sockfd(result);
    }
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
  int Send(const int sockfd, unsigned char *data, const int length) {
    return Socket::TcpSend(sockfd, data, length);
  }

  int Recv(const int sockfd, unsigned char *data, const int length) {
    return Socket::TcpRecv(sockfd, data, length, kReadTimeout);
  }

  void Close(const int sockfd) {
    if (sockfd > 0) {
      Socket::Close(sockfd);
    }
  }

 private:
  static const int kReadTimeout = 3000;  // ms

};

}

#endif /* SRC_MODEL_DEFAULTSESSION_H_ */
