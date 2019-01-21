/*
 * FtpMaster.cpp
 *
 *  Created on: 2019年1月17日
 *      Author: xueda
 */

#include "../ftp/FtpMaster.h"

#include <assert.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <iostream>

#include "core/FtpContext.h"
#include "core/FtpSession.h"
#include "middleware/Socket.h"

namespace ftp {

int FtpMaster::Setup(std::shared_ptr<FtpSession> &session) {

  std::shared_ptr<FtpSession> ctrl_session = session;
  std::shared_ptr<FtpSession> trans_session = std::make_shared<FtpSession>();
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
    Socket::Close(ctrl_session->sockfd());
    TransferHandler(trans_session);
  } else if (pid > 0) {
    /* ftp-control processer */
    std::cout << getpid() << std::endl;
    session->set_peer_pid(pid);
    Socket::Close(trans_session->ipc_sockfd());
    ControlHandler(ctrl_session);
    waitpid(pid, nullptr, 0);
    session->set_alive(false);

    std::cout << "PID : " << pid << " FTP-DATA Processer exit." << std::endl;
    std::cout << "PID : " << getpid() << " FTP-CONTORL Processer exit."
              << std::endl;
  }

  return 0;
}

int FtpMaster::RecvFrom(const std::shared_ptr<model::Session>& session,
                        model::Context* context) {
  std::shared_ptr<FtpSession> ftp_session;
  if (session->type() == model::SessionType::kTypeFTP) {
    ftp_session = std::static_pointer_cast<FtpSession>(session);
  }
  if (ftp_session == nullptr) {
    return 0;
  }

  int nbytes = 0;
  FtpContext* ftp_context = static_cast<FtpContext*>(context);
  switch (ftp_context->destination()) {
    case Destination::kDestClient:
      nbytes = ftp_session->RecvFrom(context);
      break;
    case Destination::kDestController:
    case Destination::kDestTransfer:
      nbytes = ftp_session->IpcRecv(context);
      break;
    default:
      break;
  }
  return nbytes;
}

int FtpMaster::SendTo(const std::shared_ptr<model::Session>& session,
                      model::Context* context) {
  std::shared_ptr<FtpSession> ftp_session;
  if (session->type() == model::SessionType::kTypeFTP) {
    ftp_session = std::static_pointer_cast<FtpSession>(session);
  }
  if (ftp_session == nullptr) {
    return 0;
  }
  int nbytes = 0;
  FtpContext* ftp_context = static_cast<FtpContext*>(context);
  switch (ftp_context->destination()) {
    case Destination::kDestClient:
      nbytes = ftp_session->SendTo(context);
      break;
    case Destination::kDestController:
    case Destination::kDestTransfer:
      nbytes = ftp_session->IpcSend(context);
      break;
    default:
      break;
  }
  return nbytes;
}

void FtpMaster::Reply(const std::shared_ptr<model::Session>& session,
                      const std::string& content) {
  std::unique_ptr<FtpContext> context(new FtpContext());
  context->set_content_type(model::ContentType::kString);
  context->set_content(content);
  context->set_destination(Destination::kDestClient);
  session->SendTo(context.get());
}

}

