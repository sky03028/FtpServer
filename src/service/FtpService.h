#ifndef SERVICE_H
#define SERVICE_H

#include <queue>
#include <mutex>
#include <memory>
#include <list>
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

  int max_connected_cnt_;

  std::mutex mutex_;
  std::condition_variable cond_var_;

  std::unique_ptr<ThreadPool> thread_pool_;
  std::queue<std::shared_ptr<ftp::FtpSession>> queue_;
  bool running_;

  static const int kFtpServicePort = 21;
};

}

#endif // SERVICE_H
