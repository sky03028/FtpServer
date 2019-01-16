#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <vector>
#include <thread>

class ThreadPool {
 public:
  ThreadPool();
  ~ThreadPool();

  int ThreadsCreate(void *(*func)(void *), void *args, int thread_maxcnt);
  int ThreadsAsyncJoin();

 private:
  std::vector<std::thread*> threads_;

};

#endif // THREADPOOL_H
