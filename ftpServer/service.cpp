#include <iostream>
#include <stdlib.h>
#include "service.h"
#include "utils.h"
#include "ftpwarpper.h"

Service::Service() :
		mServerSocket(-1), mAcceptTimeout(3 * 1000), mMaxConnectCount(10), mMaxThreadCount(
				1), is_Service_exit(false) {
	L = NULL;

	assert((L = luaL_newstate()) != NULL);

	luaL_openlibs(L);

	//assert(0 == luaL_dofile(L, "./scripts/lua_main.lua"));

	mSessionQueue.clear();
}

Service::~Service() {
	lua_close(L);

	delete Threads;
}

void Service::FtpServiceStart(FtpSession ftpSession) {
	int pid;

	pid = fork();
	if (pid == -1) {
		perror("fork");
	} else if (pid == 0) {
		std::cout << "Ftp Running." << std::endl;
		FtpWarpper *ftpService;

		ftpService = new FtpWarpper;
		ftpService->ServiceStart(ftpSession);
		delete ftpService;

		exit (EXIT_SUCCESS);
	} else if (pid > 0) {
		std::cout << "Ftp Service Start." << std::endl;
		return;
	}
}

void *Service::ServiceThreadHandler(void *arg) {
	Service *service = (Service *) arg;
	Session session;
	FtpSession ftpsession;

	ftpsession.SetRootPath("/home/xueda/share/ftpServer");

	bool IsQueueEmpty = true;

	do {

		if (service->is_Service_exit)
			break;

		service->mThreadsMutex.lock();
		IsQueueEmpty = service->mSessionQueue.empty();
		if (!IsQueueEmpty) {
			session = service->mSessionQueue.front();
			service->mSessionQueue.pop_front();
		}
		service->mThreadsMutex.unlock();

		if (IsQueueEmpty) {
			Utils::ThreadSleep(50);
			continue;
		}

		ftpsession = session;
		ftpsession.SetCurrentPath("/home/xueda/share/ftpServer");
		FtpServiceStart(ftpsession);

	} while (1);

	pthread_exit(NULL);

}

void *Service::SeviceMonitor(void *arg) {
	Service *service = (Service *) arg;
	std::string command;

	while (1) {
		std::cin >> command;
		if (command == "exit") {
			break;
		}
	}

	service->is_Service_exit = true;

	std::cout << "Monitor thread exit." << std::endl;

	pthread_exit(NULL);
}

int Service::ServiceInit() {
	mServerSocket = SocketSource::TcpServerCreate(NULL, FTPSERVER_PORT);

	SocketSource::TcpListen(mServerSocket, mMaxConnectCount);

	Threads = new ThreadPool;

	Threads->ThreadsCreate(Service::SeviceMonitor, (void *) this, 1);
	Threads->ThreadsCreate(Service::ServiceThreadHandler, (void *) this,
			mMaxThreadCount);

	return 0;
}

int Service::ServiceStart() {
	struct sockaddr_in client;
	int s;
	Session session;

	session.setServerListenSockfd(mServerSocket);

	do {

		if (is_Service_exit)
			break;

		s = SocketSource::TcpAccept(mServerSocket, (struct sockaddr *) &client,
				mAcceptTimeout);
		if (s >= 0) {
			std::cout << "Get a client" << std::endl;

			/* To the limit count, refuse connect */
			if (mSessionQueue.size() == mMaxConnectCount) {
				SocketSource::SocketClose(s);
				Utils::ThreadSleep(1000);
			}

			session.setClientSocketID(s);
			session.setClientIpAddress(client.sin_addr.s_addr);
			session.setClientPort(client.sin_port);

			mSessionQueue.push_back(session);

		}

	} while (1);

	SocketSource::SocketClose(mServerSocket);

	Threads->ThreadsAsyncJoin();

	return 0;
}

