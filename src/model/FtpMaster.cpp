/*
 * FtpMaster.cpp
 *
 *  Created on: 2019年1月17日
 *      Author: xueda
 */

#include <assert.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <iostream>

#include "FtpMaster.h"
#include "model/DefaultSession.h"
#include "model/DefaultContext.h"
#include "middleware/Socket.h"

int FtpMaster::Setup(std::shared_ptr<DefaultSession> &session) {

  std::shared_ptr<DefaultSession> ctrl_session = session;
  std::shared_ptr<DefaultSession> trans_session = std::make_shared<
      DefaultSession>((int(SessionType::kTypeFTP)));
#if defined(__linux__)
  int sockfd[2];
  assert(0 == socketpair(AF_UNIX, SOCK_STREAM, 0, sockfd));
  ctrl_session->set_ipc_sockfd(sockfd[0]);
  trans_session->set_ipc_sockfd(sockfd[1]);
  Socket::SetNonBlock(ctrl_session->ipc_sockfd());
  Socket::SetNonBlock(trans_session->ipc_sockfd());
#endif

  int pid;

  pid = fork();
  if (pid == -1) {
    perror("fork");
  } else if (pid == 0) {
    /* ftp-data processer */
    std::cout << getpid() << std::endl;
    Socket::Close(ctrl_session->ipc_sockfd());
    Socket::Close(ctrl_session->listen_sockfd());
    TransferHandler(trans_session);
  } else if (pid > 0) {
    /* ftp-control processer */
    std::cout << getpid() << std::endl;
    session->set_transfer_pid(pid);
    Socket::Close(trans_session->ipc_sockfd());
    ControlHandler(ctrl_session);
    waitpid(pid, nullptr, 0);

    std::cout << "PID : " << pid << " FTP-DATA Processer exit." << std::endl;
    std::cout << "PID : " << getpid() << " FTP-CONTORL Processer exit."
              << std::endl;
  }

  return 0;
}

int FtpMaster::RecvFrom(const std::shared_ptr<Session>& session,
                        Context* context) {
  return session->RecvFrom(context);
}

void FtpMaster::SendTo(const std::shared_ptr<Session>& session,
                       Context* context) {
  session->SendTo(context);
}

void FtpMaster::Reply(const std::shared_ptr<Session>& session,
                      const std::string& content) {
  std::unique_ptr<DefaultContext> context(new DefaultContext());
  context->set_content_type(ContentType::kString);
  context->set_content(content);
  context->set_destination(Destination::kDestClient);
  session->SendTo(context.get());
}
