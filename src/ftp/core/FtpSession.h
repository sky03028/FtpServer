/*
 * DefaultSession.h
 *
 *  Created on: 2019年1月17日
 *      Author: xueda
 */

#ifndef SRC_FTP_CORE_FTPSESSION_H_
#define SRC_FTP_CORE_FTPSESSION_H_

#include "core/Context.h"
#include "model/DefaultSession.h"

namespace ftp {

class FtpSession : public model::DefaultSession {
 public:
  FtpSession()
      : ipc_sockfd_(-1),
        peer_pid_(0),
        curr_dir_("/home"),
        root_dir_("/"),
        has_abort_(false),
        type_(model::SessionType::kTypeFTP) {
  }
  virtual ~FtpSession() {
    Destory();
  }

  void set_ipc_sockfd(const int sockfd) {
    ipc_sockfd_ = sockfd;
  }
  int ipc_sockfd() const {
    return ipc_sockfd_;
  }

  void set_peer_pid(const int pid) {
    peer_pid_ = pid;
  }
  int peer_pid() const {
    return peer_pid_;
  }

  void set_curr_dir(const std::string& directory) {
    curr_dir_ = directory;
  }
  const std::string& curr_dir() const {
    return curr_dir_;
  }

  void set_root_dir(const std::string& directory) {
    root_dir_ = directory;
  }
  const std::string& root_dir() const {
    return root_dir_;
  }

  void set_abort_flag(const bool flag) {
    has_abort_ = flag;
  }
  bool has_abort() const {
    return has_abort_;
  }

  virtual int type() const {
    return type_;
  }

  virtual int IpcRecv(model::Context* context) {
    if (ipc_sockfd() <= 0) {
      return -1;
    }
    unsigned char buffer[4096];
    memset(buffer, 0, sizeof(buffer));
    int nbytes = DefaultSession::Recv(ipc_sockfd(), buffer, 4096);
    if (nbytes > 0) {
      context->set_content((char*) buffer);
    }
    return nbytes;
  }

  virtual int IpcSend(model::Context* context) {
    if (ipc_sockfd() <= 0) {
      return -1;
    }
    return DefaultSession::Send(ipc_sockfd(),
                                (unsigned char*) context->content().c_str(),
                                context->content().size());
  }

  virtual void Destory() {
    DefaultSession::Destory();
    DefaultSession::Close(ipc_sockfd());
    set_ipc_sockfd(-1);
  }

  bool HasMessageArrived() {
    int sockfd_list[2] = { sockfd(), ipc_sockfd() };
    return DefaultSession::HasMessageArrived(sockfd_list, sizeof(sockfd_list), 3000);
  }

 private:
  int ipc_sockfd_;
  int peer_pid_;
  std::string curr_dir_;
  std::string root_dir_;
  bool has_abort_;
  int type_;
};

}

#endif /* SRC_FTP_CORE_FTPSESSION_H_ */
