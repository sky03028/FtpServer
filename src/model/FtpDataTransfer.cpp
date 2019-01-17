/*
 * FtpDataTransfer.cpp
 *
 *  Created on: 2019年1月16日
 *      Author: xueda
 */

#include <assert.h>
#include <sys/sendfile.h>

#include "model/DefaultSession.h"
#include "model/DefaultContext.h"

#include "utils/JsonParser.h"
#include "utils/JsonCreator.h"
#include "utils/Utils.h"
#include "middleware/Socket.h"
#include "FtpDataTransfer.h"

FtpDataTransfer::FtpDataTransfer() {

}

FtpDataTransfer::~FtpDataTransfer() {
}

/* child processer(ftp-data) */
void FtpDataTransfer::TransferHandler(
    const std::shared_ptr<DefaultSession> &session) {
  fd_set fds;
  int ipc_socket = session->ipc_sockfd();

  std::unique_ptr<DefaultContext> context(new DefaultContext());
  do {
    int result = Socket::Select(&ipc_socket, 1, 100, READFDS_TYPE, fds);
    if (result == 0) {
      continue;
    } else if (result > 0) {
      if (FD_ISSET(ipc_socket, &fds)) {
        context->set_source(Source::kSrcController);
        result = RecvFrom(session, context.get());
      }
    }

    if (result > 0) {
      assert(context->content_type() == ContentType::kJson);
      int cmdtype;
      {
        JsonParser parser(context->content());
        cmdtype = parser.GetInt("cmdtype");
      }
      switch (cmdtype) {
        case TRANSFER_PASV_STANDBY_REQ: {
          std::cout << "TRANSFER_PASV_STANDBY_REQ" << std::endl;
          PasvModeStandby(session, context.get());
        }
          break;
        case TRANSFER_PORT_STANDBY_REQ: {
          std::cout << "TRANSFER_PORT_STANDBY_REQ" << std::endl;
          PortModeStandby(session, context.get());
        }
          break;
        case TRANSFER_TRY_CONNNECT_REQ: {
          std::cout << "TRANSFER_TRY_CONNNECT_REQ" << std::endl;
          TryContact(session, context.get());
        }
          break;
        case TRANSFER_SENDCOMMAND_REQ: {
          std::cout << "TRANSFER_SENDCOMMAND_REQ" << std::endl;
          TrySendCommand(session, context.get());
        }
          break;
        case TRANSFER_FILEUPLOAD_REQ: {
          std::cout << "TRANSFER_FILEUPLOAD_REQ" << std::endl;
          TryFileUpload(session, context.get());
        }
          break;
        case TRANSFER_FILEDOWNLOAD_REQ: {
          std::cout << "TRANSFER_FILEDOWNLOAD_REQ" << std::endl;
          TryFileDownload(session, context.get());
        }
          break;
        case TRANSFER_ABORT_REQ: {
          std::cout << "TRANSFER_ABORT_REQ" << std::endl;
          session->set_abort_flag(true);
        }
          break;
      }
    }

  } while (1);

  Socket::Close(session->ipc_sockfd());
  perror("IPC_FTPTransferHandler");
}

/* ftp-data processer handler */
int FtpDataTransfer::PasvModeStandby(
    const std::shared_ptr<DefaultSession> &session, DefaultContext* context) {
  JsonCreator creator;
  creator.SetInt("cmdtype", TRANSFER_PASV_STANDBY_RES);

  do {
    struct in_addr ip_addr;
    ip_addr.s_addr = Socket::GetLocalAddress();

    int listen_sockfd = Socket::TcpServerCreate(inet_ntoa(ip_addr), 0);
    if (listen_sockfd == -1) {
      Socket::Close(listen_sockfd);
      creator.SetBool("status", false);
      break;
    }
    Socket::TcpListen(listen_sockfd, 1);
    session->set_listen_sockfd(listen_sockfd);

    struct sockaddr_in local;
    socklen_t socklen = sizeof(struct sockaddr);
    getsockname(listen_sockfd, (struct sockaddr *) &local, &socklen);

    unsigned short port = local.sin_port << 8 | local.sin_port >> 8;
    unsigned char ipv4[4];
    ipv4[0] = (unsigned char) local.sin_addr.s_addr;
    ipv4[1] = (unsigned char) local.sin_addr.s_addr >> 8;
    ipv4[2] = (unsigned char) local.sin_addr.s_addr >> 16;
    ipv4[3] = (unsigned char) local.sin_addr.s_addr >> 24;

    std::string reply_content = "Entering PASV mode (" + std::to_string(ipv4[0])
        + "," + std::to_string(ipv4[1]) + "," + std::to_string(ipv4[2]) + ","
        + std::to_string(ipv4[3]) + "," + std::to_string(port / 256) + ","
        + std::to_string(port % 256) + ")";

    session->set_ip_address(ip_addr.s_addr);
    session->set_port(port);

    creator.SetBool("status", true);
    creator.SetString("content", reply_content);
    session->set_transfer_mode(PASV_MODE_ENABLE);

  } while (0);

  context->set_destination(Destination::kDestController);
  context->set_content_type(ContentType::kJson);
  context->set_content(creator.SerializeAsString());
  ReplyController(session, context);
  return 0;
}

int FtpDataTransfer::PortModeStandby(
    const std::shared_ptr<DefaultSession> &session, DefaultContext* context) {
  unsigned short port = 0;
  unsigned int ip_address = 0;

  unsigned int p_port[2];
  unsigned int p_ipaddress[4];

  assert(context->content_type() == ContentType::kJson);
  JsonParser parser(context->content());

  std::string request_content(parser.GetString("content"));
  request_content = Utils::DeleteCRLF(request_content);
  request_content = Utils::DeleteSpace(request_content);

  std::sscanf(request_content.c_str(), "%d,%d,%d,%d,%d,%d", &p_ipaddress[0],
              &p_ipaddress[1], &p_ipaddress[2], &p_ipaddress[3], &p_port[0],
              &p_port[1]);

  port = p_port[0] | p_port[1] << 8;

  ip_address |= p_ipaddress[0];
  ip_address |= p_ipaddress[1] << 8;
  ip_address |= p_ipaddress[2] << 16;
  ip_address |= p_ipaddress[3] << 24;

  session->set_port(port);
  session->set_ip_address(ip_address);

  std::string reply_content = "PORT SUCCESS";

  JsonCreator creator;
  creator.SetBool("status", true);
  creator.SetInt("cmdtype", TRANSFER_PORT_STANDBY_RES);
  creator.SetString("content", reply_content);
  session->set_transfer_mode(PORT_MODE_ENABLE);

  context->set_destination(Destination::kDestController);
  context->set_content_type(ContentType::kJson);
  context->set_content(creator.SerializeAsString());
  ReplyController(session, context);
  return 0;
}

int FtpDataTransfer::TrySendCommand(
    const std::shared_ptr<DefaultSession> &session, DefaultContext* context) {
  int result = 0;
  if (session->sockfd() > 0) {
    context->set_destination(Destination::kDestClient);
    ReplyClient(session, context);
  }
  std::cout << strerror(Socket::CheckSockError(session->sockfd())) << std::endl;
  Socket::Close(session->sockfd());
  session->set_sockfd(INVALID_SOCKET);

  JsonCreator creator;
  creator.SetInt("cmdtype", TRANSFER_SENDCOMMAND_RES);
  if (result > 0) {
    creator.SetBool("status", true);
    creator.SetString("content", "Transfer complete.");
  } else {
    creator.SetBool("status", false);
  }

  context->set_destination(Destination::kDestController);
  context->set_content_type(ContentType::kJson);
  context->set_content(creator.SerializeAsString());
  ReplyController(session, context);
  return 0;
}

int FtpDataTransfer::TryContact(const std::shared_ptr<DefaultSession> &session,
                                DefaultContext* context) {
  int result;

  struct sockaddr_in remote;

  struct in_addr ip_addr;

  ip_addr.s_addr = session->ip_address();

  std::cout << __FUNCTION__ << " ip_addr = " << ip_addr.s_addr << std::endl;
  std::cout << __FUNCTION__ << " inet_ntoa(ip_addr) = " << inet_ntoa(ip_addr)
            << std::endl;
  std::cout << __FUNCTION__ << " session.GetTransferPort() = "
            << ntohs(session->port()) << std::endl;

  switch (session->transfer_mode()) {
    case PASV_MODE_ENABLE: {
      result = Socket::TcpAccept(session->listen_sockfd(),
                                 (struct sockaddr *) &remote, 3 * 1000);
    }
      break;

    case PORT_MODE_ENABLE: {
      result = Socket::TcpConnect(inet_ntoa(ip_addr), session->port(),
                                  3 * 1000);
    }
      break;
  }

  JsonCreator creator;
  creator.SetInt("cmdtype", TRANSFER_TRY_CONNNECT_RES);
  if (result > 0) {
    session->set_sockfd(result);
    creator.SetBool("status", true);
    creator.SetString("content", "Transfer Connected.");
  } else {
    session->set_sockfd(INVALID_SOCKET);
    creator.SetBool("status", false);

  }

  ReplyController(session, context);
  return 0;
}

int FtpDataTransfer::TryFileDownload(
    const std::shared_ptr<DefaultSession> &session, DefaultContext* context) {
  int result;
  int total_bytes = 0;
  int file_sockfd;
  std::string filepath;
  struct stat fileStat;
  std::string reply_content;
  bool success = false;

  assert(context->content_type() == ContentType::kJson);
  {
    JsonParser parser(context->content());
    filepath = parser.GetString("content");
    filepath = Utils::DeleteSpace(filepath);
  }

  do {
    result = Utils::GetFileAttribute(filepath, &fileStat);
    if (-1 == result) {
      std::cout << filepath << " is not exist!" << std::endl;
      reply_content = "Transfer fail.";
      break;
    }

    file_sockfd = open(filepath.c_str(), O_RDONLY);
    if (file_sockfd == -1) {
      perror("open");
      break;
    }

    int sockfd = session->sockfd();
    fd_set fds;
    /* sendfile */
    do {
      if (fileStat.st_size == total_bytes) {
        success = true;
        std::cout << "Download : Transfer ok." << std::endl;
        reply_content = "Transfer complete. Total "
            + std::to_string(total_bytes) + " bytes.";
        break;
      }

      if (session->has_abort()) {
        session->set_abort_flag(false);
        std::cout << "Download : Transfer abort." << std::endl;
        reply_content = "Transfer fail.";
        success = false;
        break;
      }

      result = Socket::Select(&sockfd, 1, 100, WRITEFDS_TYPE, fds);
      if (result > 0) {
        if (FD_ISSET(sockfd, &fds)) {
          //std::cout << "Totalbytes = " << totalBytes << std::endl;

          int nbytes = 4096;
          if (nbytes > fileStat.st_size - total_bytes) {
            nbytes = fileStat.st_size - total_bytes;
          }

          /* File stream : Server send to client */
          nbytes = sendfile(session->sockfd(), file_sockfd, nullptr, nbytes);
          if (nbytes > 0) {
            total_bytes += nbytes;
            continue;
          } else {
            result = Socket::CheckSockError(session->sockfd());
            if (result != 0) {
              if (result == EINTR) {
                continue;
              } else if (result == EAGAIN) {  //no data read, means sendfile get (EOF)
                success = true;
                std::cout << "Download : Transfer ok." << std::endl;
                reply_content = "Transfer complete. Total "
                    + std::to_string(total_bytes) + " bytes.";
                break;
              } else {
                std::cout << "Download : Transfer fail." << std::endl;
                reply_content = "Transfer fail.";
                perror("sendfile");
                break;
              }
            }
          }
        }

      }

    } while (1);

    Socket::Close(session->sockfd());
    session->set_sockfd(INVALID_SOCKET);
  } while (0);

  if (file_sockfd >= 0) {
    close(file_sockfd);
  }

  JsonCreator creator;
  creator.SetInt("cmdtype", TRANSFER_FILEDOWNLOAD_RES);
  creator.SetBool("status", success);
  creator.SetString("content", reply_content);

  context->set_destination(Destination::kDestController);
  context->set_content_type(ContentType::kJson);
  context->set_content(creator.SerializeAsString());
  ReplyController(session, context);
  return 0;
}

int FtpDataTransfer::TryFileUpload(
    const std::shared_ptr<DefaultSession> &session, DefaultContext* context) {
  int result;
  int file_sockfd;
  std::string filepath;
  std::string reply_content;
  int total_bytes = 0;
  int success = false;

  assert(context->content_type() == ContentType::kJson);
  {
    JsonParser parser(context->content());
    filepath = parser.GetString("content");
    filepath = Utils::DeleteSpace(filepath);
  }

//  char out[CodeConverter::kMaxConvertSize];
//  memset(out, 0, sizeof(out));
//  CodeConverter cc = CodeConverter("gb2312", "utf-8");
//  cc.convert((char *) filepath.c_str(), filepath.length(), out,
//             CodeConverter::kMaxConvertSize);
//  filepath = std::string(out);

  std::cout << "upload file : " << filepath << std::endl;
  int nbytes = 1;
  int sockfd = session->sockfd();
  fd_set fds;

  do {

    file_sockfd = open(filepath.c_str(), O_APPEND | O_CREAT);
    if (file_sockfd == -1) {
      perror("open");
      reply_content = "Transfer fail.";
      success = false;
      break;
    }

    do {

      if (session->has_abort()) {
        session->set_abort_flag(false);
        std::cout << "Upload : Transfer abort." << std::endl;
        reply_content = "Transfer fail.";
        success = false;
        break;
      }

      result = Socket::Select(&sockfd, 1, 100, WRITEFDS_TYPE, fds);
      if (result > 0) {
        if (FD_ISSET(sockfd, &fds)) {
          /* File stream : client send to server */
          result = sendfile(file_sockfd, session->sockfd(), nullptr, nbytes);
          if (result > 0) {
            std::cout << "upload : result = " << result << std::endl;
            total_bytes += result;
          } else if (result < 0) {

            if (result == EINTR)
              continue;
            else if (result == EAGAIN) {
              break;
            } else {
              perror("sendfile");
              std::cout << "Upload : Transfer fail." << std::endl;
              reply_content = "Transfer fail.";
              break;
            }

          } else if (result == 0) {
            std::cout << "Upload : Transfer ok." << std::endl;
            reply_content = "Transfer complete. Total "
                + std::to_string(total_bytes) + " bytes.";
            success = true;
            break;
          }
        }

      }

    } while (1);

    Socket::Close(session->sockfd());
    session->set_sockfd(INVALID_SOCKET);
  } while (0);

  if (file_sockfd >= 0) {
    close(file_sockfd);
  }

  JsonCreator creator;
  creator.SetInt("cmdtype", TRANSFER_FILEUPLOAD_RES);
  creator.SetBool("status", success);
  creator.SetString("content", reply_content);

  context->set_destination(Destination::kDestController);
  context->set_content_type(ContentType::kJson);
  context->set_content(creator.SerializeAsString());
  ReplyController(session, context);
  return 0;
}

void FtpDataTransfer::ReplyController(
    const std::shared_ptr<DefaultSession> & session, DefaultContext* context) {
  context->set_destination(Destination::kDestController);
  SendTo(session, context);
}

void FtpDataTransfer::ReplyClient(
    const std::shared_ptr<DefaultSession>& session, DefaultContext* context) {
  context->set_destination(Destination::kDestClient);
  SendTo(session, context);
}

