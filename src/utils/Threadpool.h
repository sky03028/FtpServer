#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <vector>
#include <thread>
#include <assert.h>

class ThreadPool {
 public:
  ThreadPool() = default;
  virtual ~ThreadPool() {
    for (std::thread* thread : threads_) {
      if (thread == nullptr) {
        continue;
      }
      thread->join();
      delete thread;
      thread = nullptr;
    }
  }

  template<typename T>
  int ThreadsCreate(void (T::*function)(void*), void *parent, void *args,
                    int thread_maxcnt) {
    for (int index = 0; index < thread_maxcnt; index++) {
      std::thread* thread = new std::thread(function, (T*) parent, args);
      assert(thread != nullptr);
      threads_.push_back(thread);
    }
    return 0;
  }

  void ThreadsAsyncJoin() {
    for (unsigned int index = 0; index < threads_.size(); index++) {
      if (threads_[index] != nullptr) {
        threads_[index]->join();
        delete threads_[index];
        threads_[index] = nullptr;
      }
    }
  }

 private:
  std::vector<std::thread*> threads_;

};

#endif // THREADPOOL_H
