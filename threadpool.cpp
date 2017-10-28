#include <assert.h>
#include "threadpool.h"

ThreadPool::ThreadPool() {
}

ThreadPool::~ThreadPool() {

}

int ThreadPool::ThreadsCreate(void *(*func)(void *), void *args,
    int thread_maxcnt) {
  for (int index = 0; index < thread_maxcnt; index++) {
    std::thread* thread = new std::thread(func);
    assert(thread != nullptr);
    threads_.push_back(thread);
  }
  return 0;
}

int ThreadPool::ThreadsAsyncJoin() {
  for (int index = 0; index < threads_.size(); index++) {
    if (threads_[index] != nullptr) {
      threads_[index]->join();
      delete threads_[index];
      threads_[index] = nullptr;
    }
  }
  return 0;
}

