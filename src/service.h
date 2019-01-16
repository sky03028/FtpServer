#ifndef SERVICE_H
#define SERVICE_H

#include <deque>
#include <mutex>
#include <memory>
#include <condition_variable>
#include "socketsource.h"
#include "threadpool.h"
#include "ftpsession.h"

class Service {
 public:
  Service();
  ~Service();

  int ServiceInit();
  int ServiceStart();

  static void *ServiceThreadHandler(void *arg);
  static void *SeviceMonitor(void *arg);
  static void FtpServiceStart(FtpSession ftpSession);

 private:
  int listen_socket_;
  int accecpt_timeout_;
  int max_connected_cnt_;
  int max_thread_cnt_;

  std::mutex mutex_;
  std::condition_variable cond_var_;
  std::deque<Session> sessions_;
  std::unique_ptr<ThreadPool> thread_pool_;
  bool running_;

  static int kFtpServicePort = 21;
};

#endif // SERVICE_H
