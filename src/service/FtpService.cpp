#include <iostream>
#include <stdlib.h>
#include "FtpService.h"

#include "ftp/core/FtpSession.h"
#include "ftp/FtpMaster.h"
#include "middleware/Socket.h"
#include "utils/Utils.h"

extern "C" int fork();

namespace service {

FtpService::FtpService()
    : max_connected_cnt_(1024),
      running_(true) {
}

FtpService::~FtpService() {
}

void FtpService::SplitProcessor(std::shared_ptr<ftp::FtpSession>& session) {
  int pid;

  pid = fork();
  if (pid == -1) {
    perror("fork");
  } else if (pid == 0) {
    std::cout << "Ftp Running." << std::endl;
    ftp::FtpMaster *master;
    master = new ftp::FtpMaster();
    master->Setup(session);
    delete master;

    exit(EXIT_SUCCESS);
  } else if (pid > 0) {
    std::cout << "Ftp Service Start." << std::endl;
    return;
  }
}

void FtpService::Handler(void *arg) {
  do {
    std::shared_ptr<ftp::FtpSession> session;
    {
      std::unique_lock<std::mutex> lock(mutex_);
      cond_var_.wait(lock, [this] {return !queue_.empty();});
      session = queue_.front();
      queue_.pop();
    }

    if (session == nullptr) {
      continue;
    }
    SplitProcessor(session);
  } while (running_);
}

void FtpService::Monitor(void *arg) {
  std::string command;

  while (1) {
    std::cin >> command;
    if (command == "exit") {
      break;
    }
  }

  running_ = false;
  std::cout << "Monitor thread exit." << std::endl;
}

int FtpService::Init() {
  thread_pool_.reset(new ThreadPool());
  thread_pool_->ThreadsCreate(&FtpService::Monitor, this, nullptr, 1);
  thread_pool_->ThreadsCreate(&FtpService::Handler, this, nullptr, 1);

  return 0;
}

int FtpService::Start() {
  int listen_sockfd = Socket::TcpServerCreate(nullptr, kFtpServicePort);
  Socket::TcpListen(listen_sockfd, max_connected_cnt_);
  do {
    unsigned int ip_address = 0;
    unsigned short port = 0;
    int sockfd = Socket::ServerContact(listen_sockfd, &ip_address, &port);
    if (sockfd >= 0) {
      std::cout << "Get a client" << std::endl;
      std::shared_ptr<ftp::FtpSession> session(new ftp::FtpSession());
      session->set_sockfd(sockfd);
      session->set_ip_address(ip_address);
      session->set_port(port);
      std::lock_guard<std::mutex> lock(mutex_);
      queue_.push(session);
      cond_var_.notify_one();
    }
  } while (running_);
  Socket::Close(listen_sockfd);
  thread_pool_->ThreadsAsyncJoin();

  return 0;
}

}

