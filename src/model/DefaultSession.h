/*
 * DefaultSession.h
 *
 *  Created on: 2019年1月17日
 *      Author: xueda
 */

#ifndef SRC_MODEL_DEFAULTSESSION_H_
#define SRC_MODEL_DEFAULTSESSION_H_

#include "core/Session.h"

class DefaultSession : public Session {
 public:
  DefaultSession(const int type)
      : ipc_sockfd_(-1),
        transfer_pid_(0),
        directory_("/"),
        has_abort_(false),
        transfer_mode_(0),
        type_(type) {
  }
  virtual ~DefaultSession() = default;

  void set_ipc_sockfd(const int sockfd) {
    ipc_sockfd_ = sockfd;
  }
  int ipc_sockfd() const {
    return ipc_sockfd_;
  }

  void set_transfer_pid(const int pid) {
    transfer_pid_ = pid;
  }
  int transfer_pid() const {
    return transfer_pid_;
  }

  void set_directory(const std::string& directory) {
    directory_ = directory;
  }
  const std::string& directory() const {
    return directory_;
  }

  void set_root_directory(const std::string& directory) {
    root_directory_ = directory;
  }
  const std::string& root_directory() const {
    return directory_;
  }

  void set_abort_flag(const bool flag) {
    has_abort_ = flag;
  }
  bool has_abort() const {
    return has_abort_;
  }

  void set_transfer_mode(const int mode) {
    transfer_mode_ = mode;
  }
  int transfer_mode() const {
    return transfer_mode_;
  }

  virtual int type() const {
    return type_;
  }

  virtual int SendTo(const Context* context) {
    return 0;
  }

  virtual int RecvFrom(const Context* context) {
    return 0;
  }

 private:
  int ipc_sockfd_;
  int transfer_pid_;
  std::string directory_;
  std::string root_directory_;
  bool has_abort_;
  int transfer_mode_;
  int type_;
};

#endif /* SRC_MODEL_DEFAULTSESSION_H_ */
