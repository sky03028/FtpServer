#include <iostream>
#include <stdlib.h>
#include "service.h"
#include "utils.h"
#include "ftpwarpper.h"

extern "C" int fork();

Service::Service()
    : listen_socket_(-1),
      accecpt_timeout_(3 * 1000),
      max_connected_cnt_(10),
      max_thread_cnt_(1),
      running_(true) {
}

Service::~Service() {
}

void Service::FtpServiceStart(FtpSession ftpSession) {
  int pid;

  pid = fork();
  if (pid == -1) {
    perror("fork");
  } else if (pid == 0) {
    std::cout << "Ftp Running." << std::endl;
    FtpWarpper *ftpService;
    ftpService = new FtpWarpper();
    ftpService->ServiceStart(ftpSession);
    delete ftpService;

    exit(EXIT_SUCCESS);
  } else if (pid > 0) {
    std::cout << "Ftp Service Start." << std::endl;
    return;
  }
}

void *Service::ServiceThreadHandler(void *arg) {
  Service *service = (Service *) arg;

  FtpSession ftpsession;
  ftpsession.SetRootPath("/home/xueda/share/ftpServer");
  do {
    FtpSession ftpsession;
    {
      std::unique_lock<std::mutex> lock(mutex_);
      cond_var_.wait(lock, [this] {return !service->sessions_.empty();});
      ftpsession = service->sessions_.front();
      service->sessions_.pop_front();
    }
    ftpsession.SetCurrentPath("/home/xueda/share/ftpServer");
    FtpServiceStart(ftpsession);
  } while (service->running_);

  return nullptr;
}

void *Service::SeviceMonitor(void *arg) {
  Service *service = (Service *) arg;
  std::string command;

  while (1) {
    std::cin >> command;
    if (command == "exit") {
      break;
    }
  }

  service->running_ = false;
  std::cout << "Monitor thread exit." << std::endl;

  return nullptr;
}

int Service::ServiceInit() {
  listen_socket_ = SocketSource::TcpServerCreate(NULL, kFtpServicePort);
  SocketSource::TcpListen(listen_socket_, max_connected_cnt_);
  thread_pool_.reset(new ThreadPool());
  thread_pool_->ThreadsCreate(Service::SeviceMonitor, (void *) this, 1);
  thread_pool_->ThreadsCreate(Service::ServiceThreadHandler, (void *) this,
                              max_thread_cnt_);

  return 0;
}

int Service::ServiceStart() {
  struct sockaddr_in client;
  int sd;
  Session session;

  session.setServerListenSockfd(listen_socket_);
  do {
    sd = SocketSource::TcpAccept(listen_socket_, (struct sockaddr *) &client,
                                 accecpt_timeout_);
    if (sd >= 0) {
      std::cout << "Get a client" << std::endl;
      /* To the limit count, refuse connect */
      if (sessions_.size() == max_connected_cnt_) {
        SocketSource::SocketClose (s);
        Utils::ThreadSleep(1000);
      }
      session.setClientSocketID(sd);
      session.setClientIpAddress(client.sin_addr.s_addr);
      session.setClientPort(client.sin_port);
      std::lock_guard<std::mutex> lock(mutex_);
      sessions_.push_back(session);
    }
  } while (running_);
  SocketSource::SocketClose(listen_socket_);
  thread_pool_->ThreadsAsyncJoin();

  return 0;
}

