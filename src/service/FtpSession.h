#ifndef FTPSESSION_H
#define FTPSESSION_H

#include <iostream>

#include "core/Session.h"

enum transfer_mode_e {
  PASV_MODE_ENABLE = 1,
  PORT_MODE_ENABLE = 2,
};

enum transfer_cmdtype_e {

  TRANSFER_PASV_STANDBY_REQ = 1,
  TRANSFER_PASV_STANDBY_RES,

  TRANSFER_PORT_STANDBY_REQ,
  TRANSFER_PORT_STANDBY_RES,

  TRANSFER_TRY_CONNNECT_REQ,
  TRANSFER_TRY_CONNNECT_RES,

  TRANSFER_SENDCOMMAND_REQ,
  TRANSFER_SENDCOMMAND_RES,

  TRANSFER_FILEUPLOAD_REQ,
  TRANSFER_FILEUPLOAD_RES,

  TRANSFER_FILEDOWNLOAD_REQ,
  TRANSFER_FILEDOWNLOAD_RES,

  TRANSFER_ABORT_REQ,
  TRANSFER_ABORT_RES,
};

class FtpSession : public Session {
 public:
  FtpSession()
      : pasv_listen_sockfd_(-1),
        pasv_listen_port_(0),
        port_connect_sockfd_(-1),
        port_connect_port_(0),
        transmode_(0),
        ipc_ctrl_sockfd_(-1),
        ipc_data_sockfd_(-1),
        ctrl_ip_address_(0),
        ctrl_port_(0),
        ctrl_sockfd_(-1),
        trans_ip_address_(0),
        trans_port_(0),
        trans_sockfd_(-1),
        data_pid_(0),
        interval_(0),
        abort_flag_(false) {
    directory_ = std::string("/");
  }

  virtual ~FtpSession() = default;

  FtpSession & operator()(Session &s) {
    ctrl_sockfd_ = s.sockfd();
    ctrl_ip_address_ = s.ip_address();
    ctrl_port_ = s.port();
    return (*this);
  }

  FtpSession & operator=(Session &s) {
    ctrl_sockfd_ = s.sockfd();
    ctrl_ip_address_ = s.ip_address();
    ctrl_port_ = s.port();
    return (*this);
  }

  void set_pasv_listen_sockfd(int sockfd) {
    pasv_listen_sockfd_ = sockfd;
  }
  int pasv_listen_sockfd() const {
    return pasv_listen_sockfd_;
  }

  void set_pasv_listen_port(unsigned short port) {
    pasv_listen_port_ = port;
  }
  unsigned short pasv_listen_port() const {
    return pasv_listen_port_;
  }

  void set_port_connect_sockfd(int sockfd) {
    port_connect_sockfd_ = sockfd;
  }
  int port_connect_sockfd() const {
    return port_connect_sockfd_;
  }

  void set_port_connect_port(unsigned short port) {
    port_connect_port_ = port;
  }
  unsigned short port_connect_port() const {
    return port_connect_port_;
  }

  /* transfer socket */
  void set_trans_sockfd(int sockfd) {
    trans_sockfd_ = sockfd;
  }
  int trans_sockfd() const {
    return trans_sockfd_;
  }

  /* control socket */
  void set_ctrl_sockfd(int sockfd) {
    ctrl_sockfd_ = sockfd;
  }
  int ctrl_sockfd() const {
    return ctrl_sockfd_;
  }

  /* transfer ip address */
  void set_trans_ip_address(unsigned int ip_address) {
    trans_ip_address_ = ip_address;
  }
  unsigned int trans_ip_address() const {
    return trans_ip_address_;
  }

  /* transfer port */
  void set_trans_port(unsigned short port) {
    trans_port_ = port;
  }

  unsigned short trans_port() const {
    return trans_port_;
  }

  /* IPC socket */
  void set_ipc_ctrl_sockfd(int sockfd) {
    ctrl_sockfd_ = sockfd;
  }
  int ipc_ctrl_sockfd() const {
    return ipc_ctrl_sockfd_;
  }

  void set_ipc_data_sockfd(int sockfd) {
    ipc_data_sockfd_ = sockfd;
  }
  int ipc_data_sockfd() const {
    return ipc_data_sockfd_;
  }

  /* ftp path */
  void set_directory(const std::string& Path) {
    directory_ = Path;
  }
  const std::string& directory() const {
    return directory_;
  }

  void set_root_path(const std::string& root_path) {
    root_path_ = root_path;
  }
  const std::string& root_path() const {
    return root_path_;
  }

  void set_user_name(const std::string &user_name) {
    user_name_ = user_name;
  }
  const std::string user_name() const {
    return user_name_;
  }

  void set_password(const std::string &password) {
    password_ = password;
  }
  const std::string& password() const {
    return password_;
  }

  int transmode() const {
    return transmode_;
  }
  void set_transmode(const int transmode) {
    transmode_ = transmode;
  }

  /* ftp-data pid */
  int data_pid() const {
    return data_pid_;
  }
  void set_data_pid(const int pid) {
    data_pid_ = pid;
  }

  bool has_data_abort() const {
    return abort_flag_;
  }

  void set_data_abort_flag(bool flag) {
    abort_flag_ = flag;
  }

 private:
  int pasv_listen_sockfd_;
  unsigned short pasv_listen_port_;

  int port_connect_sockfd_;
  unsigned short port_connect_port_;

  int transmode_;

  /* for communicate between control processer and transfer processer */
  int ipc_ctrl_sockfd_;
  int ipc_data_sockfd_;

  /* ftp-ctrl */
  int ctrl_ip_address_;
  int ctrl_port_;
  int ctrl_sockfd_;

  /* ftp-data */
  int trans_ip_address_;
  int trans_port_;
  int trans_sockfd_;

  int data_pid_;

  std::string user_name_;
  std::string password_;
  std::string directory_;
  std::string root_path_;
  unsigned int interval_;

  bool abort_flag_;

};

#endif // FTPSESSION_H
