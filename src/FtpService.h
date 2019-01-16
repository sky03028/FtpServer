#ifndef SERVICE_H
#define SERVICE_H

#include <deque>
#include <mutex>
#include <memory>
#include <condition_variable>

#include "service/FtpSession.h"
#include "middleware/Socket.h"
#include "utils/Threadpool.h"

class FtpService {
 public:
  FtpService();
  ~FtpService();

  int Init();
  int Start();

 private:
  void ServiceThreadHandler(void *arg);
  void SeviceMonitor(void *arg);
  void FtpServiceStart(FtpSession& ftpSession);

  int listen_socket_;
  int accecpt_timeout_;
  int max_connected_cnt_;
  int max_thread_cnt_;

  std::mutex mutex_;
  std::condition_variable cond_var_;
  std::deque<std::shared_ptr<FtpSession>> sessions_;
  std::unique_ptr<ThreadPool> thread_pool_;
  bool running_;

  static const int kFtpServicePort = 21;
};

#endif // SERVICE_H
