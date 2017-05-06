#ifndef SERVICE_H
#define SERVICE_H

#include "socketsource.h"
#include "threadpool.h"
#include <deque>
#include <mutex>
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "ftpsession.h"

#define FTPSERVER_PORT  21

class Service
{
public:
    Service();

    ~Service();


    int ServiceInit();

    int ServiceStart();

    static void *ServiceThreadHandler(void *arg);

    static void *SeviceMonitor(void *arg);

    static void FtpServiceStart(FtpSession ftpSession);

private:
    int mServerSocket;
    int mAcceptTimeout;
    int mMaxConnectCount;
    int mMaxThreadCount;

    std::deque<Session> mSessionQueue;
    std::mutex  mThreadsMutex;

    ThreadPool *Threads;

    bool is_Service_exit;

    lua_State *L;

};

#endif // SERVICE_H
