#include <iostream>
#include <stdlib.h>
#include "FtpService.h"

#include "service/FtpWarpper.h"
#include "utils/Utils.h"

extern "C" int fork();

FtpService::FtpService()
    : listen_socket_(-1),
      accecpt_timeout_(3 * 1000),
      max_connected_cnt_(10),
      max_thread_cnt_(1),
      running_(true) {
}

FtpService::~FtpService() {
}

void FtpService::SplitProcessor(std::shared_ptr<FtpSession>& session) {
  int pid;

  pid = fork();
  if (pid == -1) {
    perror("fork");
  } else if (pid == 0) {
    std::cout << "Ftp Running." << std::endl;
    FtpWarpper *warpper;
    warpper = new FtpWarpper();
    warpper->Setup(*session);
    delete warpper;

    exit(EXIT_SUCCESS);
  } else if (pid > 0) {
    std::cout << "Ftp Service Start." << std::endl;
    return;
  }
}

void FtpService::Handler(void *arg) {
  do {
    std::shared_ptr<FtpSession> session;
    {
      std::unique_lock<std::mutex> lock(mutex_);
      cond_var_.wait(lock, [this] {return !sessions_.empty();});
      session = sessions_.front();
      sessions_.pop_front();
    }

    if (session == nullptr) {
      continue;
    }
    session->set_root_path("/home/xueda/share/ftpServer");
    session->set_directory("/home/xueda/share/ftpServer");
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
  listen_socket_ = Socket::TcpServerCreate(nullptr, kFtpServicePort);
  Socket::TcpListen(listen_socket_, max_connected_cnt_);
  thread_pool_.reset(new ThreadPool());
  thread_pool_->ThreadsCreate(&FtpService::Monitor, this, nullptr, 1);
  thread_pool_->ThreadsCreate(&FtpService::Handler, this, nullptr,
                              max_thread_cnt_);

  return 0;
}

int FtpService::Start() {
  struct sockaddr_in client;
  int sd;

  do {
    sd = Socket::TcpAccept(listen_socket_, (struct sockaddr *) &client,
                           accecpt_timeout_);
    if (sd >= 0) {
      std::cout << "Get a client" << std::endl;
      /* To the limit count, refuse connect */
      if (sessions_.size() == (unsigned int) max_connected_cnt_) {
        Socket::SocketClose(sd);
        Utils::ThreadSleep(1000);
        continue;
      }
      std::shared_ptr<FtpSession> session(
          new FtpSession(SessionType::kTypeFTP));
      session->set_listen_sockfd(listen_socket_);
      session->set_sockfd(sd);
      session->set_ip_address(client.sin_addr.s_addr);
      session->set_port(client.sin_port);
      std::lock_guard<std::mutex> lock(mutex_);
      sessions_.push_back(session);
      cond_var_.notify_one();
    }
  } while (running_);
  Socket::SocketClose(listen_socket_);
  thread_pool_->ThreadsAsyncJoin();

  return 0;
}

