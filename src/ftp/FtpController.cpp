/*
 * FtpController.cpp
 *
 *  Created on: 2019年1月16日
 *      Author: xueda
 */

#include "../ftp/FtpController.h"

#include <sys/stat.h>
#include <signal.h>
#include <assert.h>
#include <unordered_map>
#include <thread>

#include "../model/FtpContext.h"
#include "../model/FtpSession.h"
#include "defines/FtpCodes.h"

#include "utils/CodeConverter.h"
#include "utils/JsonParser.h"
#include "utils/Utils.h"
#include "utils/JsonCreator.h"
#include "middleware/Socket.h"

FtpController::FtpController() {

}

FtpController::~FtpController() {
}

bool FtpController::Init() {
  std::cout << __FUNCTION__ << std::endl;
  handlers_["ABOR"] = std::bind(&FtpController::ftp_abor, this,
                                std::placeholders::_1, std::placeholders::_2);
  handlers_["LIST"] = std::bind(&FtpController::ftp_list, this,
                                std::placeholders::_1, std::placeholders::_2);
  handlers_["PASS"] = std::bind(&FtpController::ftp_pass, this,
                                std::placeholders::_1, std::placeholders::_2);
  handlers_["PORT"] = std::bind(&FtpController::ftp_port, this,
                                std::placeholders::_1, std::placeholders::_2);
  handlers_["PASV"] = std::bind(&FtpController::ftp_pasv, this,
                                std::placeholders::_1, std::placeholders::_2);
  handlers_["QUIT"] = std::bind(&FtpController::ftp_quit, this,
                                std::placeholders::_1, std::placeholders::_2);
  handlers_["RETR"] = std::bind(&FtpController::ftp_retr, this,
                                std::placeholders::_1, std::placeholders::_2);
  handlers_["STOR"] = std::bind(&FtpController::ftp_stor, this,
                                std::placeholders::_1, std::placeholders::_2);
  handlers_["SYST"] = std::bind(&FtpController::ftp_syst, this,
                                std::placeholders::_1, std::placeholders::_2);
  handlers_["TYPE"] = std::bind(&FtpController::ftp_type, this,
                                std::placeholders::_1, std::placeholders::_2);
  handlers_["USER"] = std::bind(&FtpController::ftp_user, this,
                                std::placeholders::_1, std::placeholders::_2);
  handlers_["CWD"] = std::bind(&FtpController::ftp_cwd, this,
                               std::placeholders::_1, std::placeholders::_2);
  handlers_["FEAT"] = std::bind(&FtpController::ftp_feat, this,
                                std::placeholders::_1, std::placeholders::_2);
  handlers_["REST"] = std::bind(&FtpController::ftp_rest, this,
                                std::placeholders::_1, std::placeholders::_2);
  handlers_["PWD"] = std::bind(&FtpController::ftp_pwd, this,
                               std::placeholders::_1, std::placeholders::_2);
  handlers_["CDUP"] = std::bind(&FtpController::ftp_cdup, this,
                                std::placeholders::_1, std::placeholders::_2);
  return true;
}

/* parent processer(ftp-control) */
int FtpController::ControlHandler(
    const std::shared_ptr<FtpSession> &session) {
  int nbytes;
  char buffer[2048];
  int sockfd_list[2];

  memset(buffer, 0, sizeof(buffer));

  ReplyClient(
      session, FTP_GREET,
      "Environment: Linux system. Used UNIX BSD Socket. (FtpServer ver. 0.1)");

  fd_set fds;
  sockfd_list[0] = session->sockfd();
  sockfd_list[1] = session->ipc_sockfd();

  std::unique_ptr<FtpContext> context(new FtpContext());
  do {

    memset(buffer, 0, sizeof(buffer));
    nbytes = Socket::Select(sockfd_list, sizeof(sockfd_list) / sizeof(int), 100,
                            READFDS_TYPE, fds);
    if (nbytes == 0) {
      continue;
    } else if (nbytes > 0) {
      // clients request
      if (FD_ISSET(session->sockfd(), &fds)) {
//        nbytes = Socket::TcpReadLine(session->sockfd(), buffer, sizeof(buffer));
        context->set_source(Source::kSrcClient);
        nbytes = RecvFrom(session, context.get());
        if (nbytes <= 0) {
          if (Socket::CheckSockError(session->sockfd()) == EAGAIN) {
            continue;
          }

          perror("ControlHandler ----> TcpReadOneLine");
          Socket::Close(session->sockfd());
          Socket::Close(session->ipc_sockfd());

          std::this_thread::sleep_for(std::chrono::seconds(1));
          /* kill ftp-data processer */
          kill(session->transfer_pid(), SIGTERM);

          break;
        } else {
          std::unique_ptr<FtpContext> ctx(new FtpContext());
          ctx->set_content(std::string(buffer));
          CommandHandler(session, ctx.get());
        }
      }

      /* IPC ftp-control process */
      if (FD_ISSET(session->ipc_sockfd(), &fds)) {
//        nbytes = RecvInstruction(session.ipc_ctrl_sockfd(), instruction);
        context->set_source(Source::kSrcTransfer);
        nbytes = RecvFrom(session, context.get());
        if (nbytes <= 0) {
          if (Socket::CheckSockError(session->ipc_sockfd()) == EAGAIN) {
            continue;
          }

          perror("FTPControlHandler ---> IPC_RecvInstruction");
          Socket::Close(session->ipc_sockfd());
          Socket::Close(session->sockfd());

          std::this_thread::sleep_for(std::chrono::seconds(1));

          /* kill ftp-data processer */
          kill(session->transfer_pid(), SIGTERM);

          break;
        } else {
          ReplyHandler(session, context.get());
        }
      }

    }

  } while (1);

  return 0;
}

int FtpController::CommandHandler(
    const std::shared_ptr<FtpSession> &session, FtpContext* context) {
  char command[5];
  char content[2048];

  memset(command, 0, sizeof(command));
  memset(content, 0, sizeof(content));

  memcpy(command, context->content().c_str(), sizeof(command) - 1);
  memcpy(content, context->content().c_str() + 4,
         context->content().size() - 4);

  std::string ftpcommand = std::string(command);
  ftpcommand = Utils::DeleteSpace(ftpcommand);
  ftpcommand = Utils::DeleteCRLF(ftpcommand);

  std::cout << context->content() << std::endl;
  if (handlers_.count(ftpcommand) == 0) {
    ReplyClient(session, FTP_COMMANDNOTIMPL, "Not Support Now.");
    return -1;
  }
  auto& function = handlers_[ftpcommand];
  function(session, context);

  return 0;
}

void FtpController::ReplyHandler(const std::shared_ptr<FtpSession> &session,
                                 FtpContext* context) {
  int resp_code;
  std::string reply_content;

  JsonParser parser(context->content());
  int cmd_type = parser.GetInt("cmdtype");
  bool success = parser.GetBool("status");

  switch (cmd_type) {
    case TRANSFER_PORT_STANDBY_RES: {
      std::cout << "TRANSFER_PORT_STANDBY_RES" << std::endl;

      if (success) {
        reply_content = std::string(context->content());
        resp_code = FTP_PORTOK;
      } else {
        reply_content = "Fail to Enter Port Mode.";
        resp_code = FTP_IP_LIMIT;
      }
    }
      break;

    case TRANSFER_PASV_STANDBY_RES: {
      std::cout << "TRANSFER_PASV_STANDBY_RES" << std::endl;

      if (success) {
        reply_content = std::string(context->content());
        resp_code = FTP_PASVOK;
      } else {
        reply_content = "Fail to Enter Passive Mode.";
        resp_code = FTP_IP_LIMIT;
      }
    }
      break;

    case TRANSFER_SENDCOMMAND_RES: {
      std::cout << "TRANSFER_SENDCOMMAND_RES" << std::endl;
      if (success) {
        reply_content = std::string(context->content());
        resp_code = FTP_TRANSFEROK;
      } else {
        reply_content = "Transfer fail.";
        resp_code = FTP_BADSENDFILE;
      }
    }
      break;

    case TRANSFER_FILEDOWNLOAD_RES: {
      std::cout << "TRANSFER_FILEDOWNLOAD_RES" << std::endl;
      if (success) {
        reply_content = std::string(context->content());
        resp_code = FTP_TRANSFEROK;
      } else {
        reply_content = "Transfer fail.";
        resp_code = FTP_BADSENDFILE;
      }
    }
      break;

    case TRANSFER_FILEUPLOAD_RES: {
      std::cout << "TRANSFER_FILEUPLOAD_RES" << std::endl;
      if (success) {
        reply_content = std::string(context->content());
        resp_code = FTP_TRANSFEROK;
      } else {
        reply_content = "Transfer fail.";
        resp_code = FTP_BADSENDFILE;
      }
    }
      break;

    default: {
      resp_code = 0;
      reply_content.clear();
    }
      break;

  }

  if (reply_content.size() > 0) {
    reply_content = Utils::DeleteCRLF(reply_content);
    ReplyClient(session, resp_code, reply_content);
  }

}

int FtpController::ReplyClient(const std::shared_ptr<FtpSession> &session,
                               int code, const std::string& content) {
  std::string packed_content;
  if (code > 0) {
    packed_content = std::to_string(code) + " " + content + "\r\n";
  } else {
    if (content.size() > 0) {
      packed_content = std::string(content) + "\r\n";
    }
  }

  if (packed_content.size() > 0) {
    Reply(session, packed_content);
  }
  return 0;
}

int FtpController::NotifyTransfer(
    const std::shared_ptr<FtpSession>& session, FtpContext* context) {
  SendTo(session, context);
  return 0;
}

/* ftp contorl processer handler */
int FtpController::ftp_cwd(const std::shared_ptr<FtpSession> &session,
                           FtpContext *context) {
  std::string reply_content;

  assert(context->content_type() == ContentType::kString);
  std::string path = context->content();

  int errcode;
  struct stat file_stat;

  char out[CodeConverter::kMaxConvertSize];

  memset(out, 0, sizeof(out));

  CodeConverter cc("GB2312", "UTF-8");
  cc.convert((char *) path.c_str(), path.length(), out,
             CodeConverter::kMaxConvertSize);

  path = std::string(out);
  path = Utils::DeleteSpace(path);
  path = Utils::DeleteCRLF(path);

  std::string directory = session->directory();
  if (path.size() < session->root_directory().size()) {
    path = session->root_directory();
  }

  if (Utils::CheckSymbolExsit(path, '/') == -1) {
    if (directory.at(directory.size() - 1) != '/') {
      directory += "/";
    }
    path = directory + path;
  }
  session->set_directory(path);

  std::cout << "CWD : " << path << std::endl;

  if (0 == stat(path.c_str(), &file_stat)) {
    if (S_ISDIR(file_stat.st_mode)) {
      errcode = FTP_CWDOK;
      reply_content = "Directory changed to " + path;
    } else {
      errcode = FTP_CWDOK;
    }
  } else {
    reply_content = "Not such file or directory.";
    errcode = FTP_NOPERM;
  }

  return ReplyClient(session, errcode, reply_content);
}

int FtpController::ftp_abor(const std::shared_ptr<FtpSession> &session,
                            FtpContext *context) {
  JsonCreator creator;
  creator.SetInt("cmdtype", TRANSFER_ABORT_REQ);
  creator.SetBool("status", true);
  creator.SerializeAsString();

  context->set_destination(Destination::kDestTransfer);
  context->set_content_type(ContentType::kJson);
  context->set_content(creator.SerializeAsString());
  NotifyTransfer(session, context);
  ReplyClient(session, FTP_ABOROK, std::string("Abort ok."));
  return 0;
}

int FtpController::ftp_list(const std::shared_ptr<FtpSession> &session,
                            FtpContext *context) {
  /* 1. try to notice the ftp-data processer to connect */
  {
    JsonCreator creator;
    creator.SetInt("cmdtype", TRANSFER_TRY_CONNNECT_REQ);
    creator.SetBool("status", true);
    creator.SerializeAsString();

    context->set_destination(Destination::kDestTransfer);
    context->set_content_type(ContentType::kJson);
    context->set_content(creator.SerializeAsString());
    SendTo(session, context);
  }
  ReplyClient(session, FTP_DATACONN,
              std::string("Opening ASCII mode data connection for /bin/ls"));

  /* 2. try to send command back to client */
  std::string directory;
  directory = session->directory();

  std::cout << "FTP_LIST :  directory = " << directory << std::endl;
  if (directory.at(directory.size() - 1) != '/') {
    directory += "/";
  }
  std::string dirlist = Utils::GetListString(directory);

  {
    JsonCreator creator;
    creator.SetBool("status", true);
    creator.SetInt("cmdtype", TRANSFER_SENDCOMMAND_REQ);
    creator.SetString("dirlist", dirlist);

    context->set_content_type(ContentType::kJson);
    context->set_content(creator.SerializeAsString());
    context->set_destination(Destination::kDestTransfer);
    SendTo(session, context);
  }
  return 0;
}

int FtpController::ftp_pass(const std::shared_ptr<FtpSession> &session,
                            FtpContext *context) {
  // ... 校验密码

  std::string reply_content =
      "Welcome to my ftpServer . Author: fengxueda531@163.com.";

  return ReplyClient(session, FTP_LOGINOK, reply_content);
}

int FtpController::ftp_port(const std::shared_ptr<FtpSession> &session,
                            FtpContext *context) {
  std::string content;
  if (context->content_type() == ContentType::kString) {
    content = context->content();
  }

  {
    JsonCreator creator;
    creator.SetBool("status", true);
    creator.SetInt("cmdtype", TRANSFER_PORT_STANDBY_REQ);
    creator.SetString("content", content);

    context->set_destination(Destination::kDestTransfer);
    context->set_content_type(ContentType::kJson);
    context->set_content(creator.SerializeAsString());
    NotifyTransfer(session, context);
  }
  return 0;
}

int FtpController::ftp_quit(const std::shared_ptr<FtpSession> &session,
                            FtpContext *context) {
  std::string ReplyContent = "Goodbye";
  ReplyClient(session, FTP_GOODBYE, ReplyContent);
  //exit(EXIT_SUCCESS);

  return 0;
}

int FtpController::ftp_retr(const std::shared_ptr<FtpSession> &session,
                            FtpContext *context) {
  std::string cur_directory;
  std::string directory;
  if (context->content_type() == ContentType::kString) {
    directory = context->content();
    directory = Utils::DeleteSpace(directory);
    directory = Utils::DeleteCRLF(directory);
  }

  cur_directory = session->directory();
  if (cur_directory.at(cur_directory.size() - 1) != '/') {
    directory = cur_directory + "/" + directory;
  } else {
    directory = cur_directory + directory;
  }

  /* 1. try to notice the ftp-data processer to connect */
  {
    JsonCreator creator;
    creator.SetBool("status", true);
    creator.SetInt("cmdtype", TRANSFER_TRY_CONNNECT_REQ);

    context->set_destination(Destination::kDestTransfer);
    context->set_content_type(ContentType::kJson);
    context->set_content(creator.SerializeAsString());
    NotifyTransfer(session, context);
  }

  /*2. try to send file to client */
  {
    JsonCreator creator;
    creator.SetBool("status", true);
    creator.SetInt("cmdtype", TRANSFER_FILEDOWNLOAD_REQ);
    creator.SetString("content", directory);

    context->set_destination(Destination::kDestTransfer);
    context->set_content_type(ContentType::kJson);
    context->set_content(creator.SerializeAsString());
    NotifyTransfer(session, context);
  }

  return 0;
}

int FtpController::ftp_stor(const std::shared_ptr<FtpSession> &session,
                            FtpContext *context) {
  std::string directory;
  if (context->content_type() == ContentType::kString) {
    directory = context->content();
    directory = Utils::DeleteSpace(directory);
    directory = Utils::DeleteCRLF(directory);
    directory = "./" + directory;
  }

  /* 1. try to notice the ftp-data processer to connect */
  {
    JsonCreator creator;
    creator.SetBool("status", true);
    creator.SetInt("cmdtype", TRANSFER_TRY_CONNNECT_REQ);

    context->set_destination(Destination::kDestTransfer);
    context->set_content_type(ContentType::kJson);
    context->set_content(creator.SerializeAsString());
    NotifyTransfer(session, context);
  }

  /*2. try to send file to client */
  {
    JsonCreator creator;
    creator.SetBool("status", true);
    creator.SetInt("cmdtype", TRANSFER_FILEUPLOAD_REQ);
    creator.SetString("directory", directory);

    context->set_destination(Destination::kDestTransfer);
    context->set_content_type(ContentType::kJson);
    context->set_content(creator.SerializeAsString());
    NotifyTransfer(session, context);
  }

  return 0;
}

int FtpController::ftp_syst(const std::shared_ptr<FtpSession> &session,
                            FtpContext *context) {
  const std::string reply_content = "UNIX Type: L8";
  ReplyClient(session, FTP_SYSTOK, reply_content);
  return 0;
}

int FtpController::ftp_type(const std::shared_ptr<FtpSession> &session,
                            FtpContext *context) {
  std::string format;
  if (context->content_type() == ContentType::kString) {
    format = context->content();
    format = Utils::DeleteSpace(format);
  }
  std::string reply_content;
  reply_content = "Type set to " + format;
  ReplyClient(session, FTP_TYPEOK, reply_content);
  return 0;
}

int FtpController::ftp_user(const std::shared_ptr<FtpSession> &session,
                            FtpContext *context) {
  // 校验用户名 ...
  return ReplyClient(session, FTP_GIVEPWORD, "User name okay, need password.");
}

int FtpController::ftp_pasv(const std::shared_ptr<FtpSession> &session,
                            FtpContext *context) {
  JsonCreator creator;
  creator.SetBool("status", true);
  creator.SetInt("cmdtype", TRANSFER_PASV_STANDBY_REQ);

  context->set_destination(Destination::kDestTransfer);
  context->set_content_type(ContentType::kJson);
  context->set_content(creator.SerializeAsString());
  NotifyTransfer(session, context);
  return 0;
}

int FtpController::ftp_feat(const std::shared_ptr<FtpSession> &session,
                            FtpContext *context) {
  const std::string reply_content = "Not support this command";
  ReplyClient(session, FTP_COMMANDNOTIMPL, reply_content);
  return 0;
}

int FtpController::ftp_rest(const std::shared_ptr<FtpSession> &session,
                            FtpContext *context) {
  const std::string ReplyContent = "Reset ok!";
  ReplyClient(session, FTP_RESTOK, ReplyContent);
  return 0;
}

int FtpController::ftp_pwd(const std::shared_ptr<FtpSession> &session,
                           FtpContext *context) {
  std::string reply_content;
  std::string directory = "\"" + session->directory() + "\"";
  reply_content = directory + " is current directory.";
  ReplyClient(session, FTP_PWDOK, reply_content);
  return 0;
}

int FtpController::ftp_cdup(const std::shared_ptr<FtpSession> &session,
                            FtpContext *context) {
  std::string reply_content;
  std::string directory;

  directory = session->directory();
  directory = Utils::GetLastDirPath(directory);
  if (directory.size() < session->root_directory().size()) {
    directory = session->root_directory();
  }
  session->set_directory(directory);
  std::cout << "FTP_CDUP : " << directory << std::endl;
  reply_content = "Directory changed to " + directory;

  ReplyClient(session, FTP_CWDOK, reply_content);
  return 0;
}
