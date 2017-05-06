#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <vector>
#include <pthread.h>
#include <assert.h>


class ThreadPool
{
public:
    ThreadPool();
    ~ThreadPool();

    int ThreadsCreate(void *(* func)(void *), void *args, int thread_maxcnt);

    int ThreadsAsyncJoin();

    //std::mutex mutex;

private:
    std::vector<pthread_t> ThreadVector;


};

#endif // THREADPOOL_H
