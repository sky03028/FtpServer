#ifndef SERVICE_H
#define SERVICE_H

#include <deque>
#include <mutex>
#include <memory>
#include <condition_variable>

#include "core/Service.h"
#include "utils/Threadpool.h"

namespace ftp {
class FtpSession;
}

namespace service {

class FtpService : public model::Service {
 public:
  FtpService();
  ~FtpService();

  int Init();
  int Start();

 private:
  void Handler(void *arg);
  void Monitor(void *arg);
  void SplitProcessor(std::shared_ptr<ftp::FtpSession>& session);

  int listen_socket_;
  int accecpt_timeout_;
  int max_connected_cnt_;
  int max_thread_cnt_;

  std::mutex mutex_;
  std::condition_variable cond_var_;
  std::deque<std::shared_ptr<ftp::FtpSession>> sessions_;
  std::unique_ptr<ThreadPool> thread_pool_;
  bool running_;

  static const int kFtpServicePort = 21;
};

}

#endif // SERVICE_H
