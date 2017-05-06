#include "threadpool.h"

ThreadPool::ThreadPool()
{
    ThreadVector.clear();
}

ThreadPool::~ThreadPool()
{

}



int ThreadPool::ThreadsCreate(void *(* func)(void *) , void *args, int thread_maxcnt)
{
    pthread_t thread_id;

    for(int index = 0; index < thread_maxcnt; index++)
    {
        assert(pthread_create(&thread_id, NULL, func, args) == 0);
        ThreadVector.push_back(thread_id);
    }

    return 0;
}


int ThreadPool::ThreadsAsyncJoin()
{
    int thread_maxcnt = ThreadVector.size();

    /* for release the resource */
    for(int index = 0; index < thread_maxcnt; index++)
    {
        pthread_join(ThreadVector[index], NULL);
    }

    return 0;
}




