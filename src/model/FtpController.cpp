/*
 * FtpController.cpp
 *
 *  Created on: 2019年1月16日
 *      Author: xueda
 */

#include "model/FtpController.h"
#include "defines/FtpCodes.h"

FtpController::FtpController() {

}

FtpController::~FtpController() {
}

/* ftp contorl processer handler */
int FtpController::ftp_cwd(const std::shared_ptr<Session> &session,
                           Context *context) {
  std::string reply_content;

  std::string ftp_path = std::string(context);

  int errcode;
  struct stat fileStat;

  char out[CONVEROUTLEN];

  memset(out, 0, sizeof(out));

  CodeConverter cc("GB2312", "UTF-8");

  cc.convert((Context *) ftp_path.c_str(), strlen(ftp_path.c_str()), out,
             CONVEROUTLEN);

  ftp_path = std::string(out);

  ftp_path = Utils::DeleteSpace(ftp_path);
  ftp_path = Utils::DeleteCRLF(ftp_path);

  std::string temp = session.directory();

  if (ftp_path.size() < session.root_path().size()) {
    ftp_path = session.root_path();
  }

  if (Utils::CheckSymbolExsit(ftp_path, '/') == -1) {
    if (temp.at(temp.size() - 1) != '/') {
      temp += "/";
    }
    ftp_path = temp + ftp_path;
  }

  session.set_directory(ftp_path);

  std::cout << "__CWD : " << ftp_path << std::endl;

  if (0 == stat(ftp_path.c_str(), &fileStat)) {
    if (S_ISDIR(fileStat.st_mode)) {
      errcode = FTP_CWDOK;
      reply_content = "Directory changed to " + ftp_path;
    } else {
      errcode = FTP_CWDOK;
    }
  } else {
    reply_content = "Not such file or directory.";
    errcode = FTP_NOPERM;
  }

  return Reply(session, errcode, reply_content);
}

int FtpController::ftp_abor(const std::shared_ptr<Session> &session,
                            Context *context) {
  FtpInstruction instruction;

  instruction.clear();

  instruction.setInsExecFlag(true);
  instruction.setInsType(TRANSFER_ABORT_REQ);

  SendInstruction(session->ipc_ctrl_sockfd(), instruction);
  if (session->type() == SessionType::kTypeFTP) {
    std::shared_ptr<FtpSession> ftp_session = std::static_pointer_cast<
        FtpSession>(session);
  }
  Reply(session, FTP_ABOROK, std::string("Abort ok."));

  return 0;
}

int FtpController::ftp_list(const std::shared_ptr<Session> &session,
                            Context *context) {
  FtpInstruction instruction;

  /* 1. try to notice the ftp-data processer to connect */
  instruction.clear();

  instruction.setInsExecFlag(true);
  instruction.setInsType(TRANSFER_TRY_CONNNECT_REQ);

  SendInstruction(session.ipc_ctrl_sockfd(), instruction);
  Reply(session, FTP_DATACONN,
        std::string("Opening ASCII mode data connection for /bin/ls"));

  /* 2. try to send command back to client */
  std::string filePath;
  filePath = session.directory();

  std::cout << "__FTP_LIST :  filePath = " << filePath << std::endl;
  if (filePath.at(filePath.size() - 1) != '/')
    filePath += "/";

  std::string DirListString = Utils::GetListString(filePath);

  instruction.clear();

  instruction.setInsExecFlag(true);
  instruction.setInsType(TRANSFER_SENDCOMMAND_REQ);
  instruction.setInsContent(DirListString.c_str(), DirListString.size());
  instruction.setInsContentLength(DirListString.size());

  return SendInstruction(session.ipc_ctrl_sockfd(), instruction);

}

int FtpController::ftp_pass(const std::shared_ptr<Session> &session,
                            Context *context) {
  std::string password(context);
  password = Utils::DeleteSpace(password);
  session.set_password(password);
  std::string reply_content =
      "Welcome to my ftpServer . Author: fengxueda531@163.com.";

  return Reply(session, FTP_LOGINOK, reply_content);
}

int FtpController::ftp_port(const std::shared_ptr<Session> &session,
                            Context *context) {
  FtpInstruction instruction;

  instruction.clear();

  instruction.setInsType(TRANSFER_PORT_STANDBY_REQ);
  instruction.setInsExecFlag(true);
  instruction.setInsContent(context, strlen(context));
  instruction.setInsContentLength(strlen(context));

  /* send connect request to ftp-data processer */
  SendInstruction(session.ipc_ctrl_sockfd(), instruction);
  return 0;
}

int FtpController::ftp_quit(const std::shared_ptr<Session> &session,
                            Context *context) {
  std::string ReplyContent = "Goodbye";

  Reply(session, FTP_GOODBYE, ReplyContent);

  //exit(EXIT_SUCCESS);

  return 0;
}

int FtpController::ftp_retr(const std::shared_ptr<Session> &session,
                            Context *context) {
  FtpInstruction instruction;

  /* 1. try to notice the ftp-data processer to connect */
  instruction.clear();

  instruction.setInsExecFlag(true);
  instruction.setInsType(TRANSFER_TRY_CONNNECT_REQ);

  SendInstruction(session.ipc_ctrl_sockfd(), instruction);

  /*2. try to send file to client */
  std::string temp;
  std::string filePath = std::string(context);

  instruction.clear();

  instruction.setInsExecFlag(true);
  instruction.setInsType(TRANSFER_FILEDOWNLOAD_REQ);
  filePath = Utils::DeleteSpace(filePath);
  filePath = Utils::DeleteCRLF(filePath);

  temp = session.directory();
  if (temp.at(temp.size() - 1) != '/') {
    filePath = temp + "/" + filePath;
  } else {
    filePath = temp + filePath;
  }

  instruction.setInsContent(filePath.c_str(), filePath.size());
  instruction.setInsContentLength(filePath.size());

  return SendInstruction(session.ipc_ctrl_sockfd(), instruction);
}

int FtpController::ftp_stor(const std::shared_ptr<Session> &session,
                            Context *context) {
  FtpInstruction instruction;

  /* 1. try to notice the ftp-data processer to connect */
  instruction.clear();

  instruction.setInsExecFlag(true);
  instruction.setInsType(TRANSFER_TRY_CONNNECT_REQ);

  SendInstruction(session.ipc_ctrl_sockfd(), instruction);

  /*2. try to send file to client */

  std::string filePath = std::string(context);

  instruction.clear();

  instruction.setInsExecFlag(true);
  instruction.setInsType(TRANSFER_FILEUPLOAD_REQ);
  filePath = Utils::DeleteSpace(filePath);
  filePath = Utils::DeleteCRLF(filePath);
  filePath = "./" + filePath;
  instruction.setInsContent(filePath.c_str(), filePath.size());
  instruction.setInsContentLength(filePath.size());

  return SendInstruction(session.ipc_ctrl_sockfd(), instruction);
}

int FtpController::ftp_syst(const std::shared_ptr<Session> &session,
                            Context *context) {
  const std::string reply_content = "UNIX Type: L8";
  return Reply(session, FTP_SYSTOK, reply_content);
}

int FtpController::ftp_type(const std::shared_ptr<Session> &session,
                            Context *context) {
  std::string format(context);
  format = Utils::DeleteSpace(format);

  std::string reply_content;
  reply_content = "Type set to " + format;

  return Reply(session, FTP_TYPEOK, reply_content);
}

int FtpController::ftp_user(const std::shared_ptr<Session> &session,
                            Context *context) {
  std::string username(context);

  username = Utils::DeleteSpace(username);

  session.set_user_name(username);

  return Reply(session, FTP_GIVEPWORD, "User name okay, need password.");
}

int FtpController::ftp_pasv(const std::shared_ptr<Session> &session,
                            Context *context) {
  FtpInstruction instruction;

  instruction.clear();

  instruction.setInsExecFlag(true);
  instruction.setInsType(TRANSFER_PASV_STANDBY_REQ);
  instruction.setInsContentLength(0);

  SendInstruction(session.ipc_ctrl_sockfd(), instruction);

  return 0;
}

int FtpController::ftp_feat(const std::shared_ptr<Session> &session,
                            Context *context) {
  const std::string reply_content = "Not support this command";
  return Reply(session, FTP_COMMANDNOTIMPL, reply_content);
}

int FtpController::ftp_rest(const std::shared_ptr<Session> &session,
                            Context *context) {
  const std::string ReplyContent = "Reset ok!";
  return Reply(session, FTP_RESTOK, ReplyContent);
}

int FtpController::ftp_pwd(const std::shared_ptr<Session> &session,
                           Context *context) {
  std::string reply_content;
  std::string directory;

  directory = "\"" + session.directory() + "\"";
  reply_content = directory + " is current directory.";
  return Reply(session, FTP_PWDOK, reply_content);
}

int FtpController::ftp_cdup(const std::shared_ptr<Session> &session,
                            Context *context) {
  std::string reply_content;
  std::string directory;

  directory = session->directory();
  directory = Utils::GetLastDirPath(directory);

  if (directory.size() < session.root_path().size())
    directory = session.root_path();

  std::cout << "__FTP_CDUP : " << directory << std::endl;

  session.set_directory(directory);

  reply_content = "Directory changed to " + directory;

  return Reply(session, FTP_CWDOK, reply_content);
}
