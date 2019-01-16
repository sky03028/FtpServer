#include "../service/FtpWarpper.h"

#include <unistd.h>
#include <assert.h>
#include <dirent.h>
#include <sys/time.h>
#include <fstream>
#include <sys/sendfile.h>
#include <vector>

#include "utils/Utils.h"
#include "defines/FtpCodes.h"
#include "middleware/Socket.h"

FtpWarpper::FtpWarpper() {
  RegisterCallback();
}

FtpWarpper::~FtpWarpper() {
}

void FtpWarpper::RegisterCallback() {
  std::cout << __FUNCTION__ << std::endl;

  handlers_.clear();

  handlers_["ABOR"] = std::bind(&FtpWarpper::ftp_abor, this,
                                std::placeholders::_1, std::placeholders::_2);
  handlers_["LIST"] = std::bind(&FtpWarpper::ftp_list, this,
                                std::placeholders::_1, std::placeholders::_2);
  handlers_["PASS"] = std::bind(&FtpWarpper::ftp_pass, this,
                                std::placeholders::_1, std::placeholders::_2);
  handlers_["PORT"] = std::bind(&FtpWarpper::ftp_port, this,
                                std::placeholders::_1, std::placeholders::_2);
  handlers_["PASV"] = std::bind(&FtpWarpper::ftp_pasv, this,
                                std::placeholders::_1, std::placeholders::_2);
  handlers_["QUIT"] = std::bind(&FtpWarpper::ftp_quit, this,
                                std::placeholders::_1, std::placeholders::_2);
  handlers_["RETR"] = std::bind(&FtpWarpper::ftp_retr, this,
                                std::placeholders::_1, std::placeholders::_2);
  handlers_["STOR"] = std::bind(&FtpWarpper::ftp_stor, this,
                                std::placeholders::_1, std::placeholders::_2);
  handlers_["SYST"] = std::bind(&FtpWarpper::ftp_syst, this,
                                std::placeholders::_1, std::placeholders::_2);
  handlers_["TYPE"] = std::bind(&FtpWarpper::ftp_type, this,
                                std::placeholders::_1, std::placeholders::_2);
  handlers_["USER"] = std::bind(&FtpWarpper::ftp_user, this,
                                std::placeholders::_1, std::placeholders::_2);
  handlers_["CWD"] = std::bind(&FtpWarpper::ftp_cwd, this,
                               std::placeholders::_1, std::placeholders::_2);
  handlers_["FEAT"] = std::bind(&FtpWarpper::ftp_feat, this,
                                std::placeholders::_1, std::placeholders::_2);
  handlers_["REST"] = std::bind(&FtpWarpper::ftp_rest, this,
                                std::placeholders::_1, std::placeholders::_2);
  handlers_["PWD"] = std::bind(&FtpWarpper::ftp_pwd, this,
                               std::placeholders::_1, std::placeholders::_2);
  handlers_["CDUP"] = std::bind(&FtpWarpper::ftp_cdup, this,
                                std::placeholders::_1, std::placeholders::_2);
}

int FtpWarpper::FtpReply(FtpSession &session, int code, std::string content) {
  std::string PackedContent;

  PackedContent.clear();

  if (code > 0) {
    PackedContent = std::to_string(code) + " " + std::string(content) + "\r\n";
  } else {
    if (content.size() > 0) {
      PackedContent = std::string(content) + "\r\n";
    }
  }

  if (PackedContent.size() > 0) {
    return Socket::TcpSend(session.ctrl_sockfd(),
                           (unsigned char *) PackedContent.c_str(),
                           PackedContent.size());
  }
  return 0;
}

/* IPC operation */
int FtpWarpper::RecvInstruction(int sockfrom, FtpInstruction &instruction) {
  unsigned char *pInstruction = (unsigned char *) &instruction;
  memset(pInstruction, 0, sizeof(instruction));
  return Socket::TcpRecv(sockfrom, pInstruction, sizeof(instruction));
}

int FtpWarpper::SendInstruction(int sockto, FtpInstruction &instruction) {
  unsigned char *pInstruction = (unsigned char *) &instruction;
  return Socket::TcpSend(sockto, pInstruction, sizeof(instruction));
}

/* ftp-data processer handler */
int FtpWarpper::PASV_FtpTransferStandby(FtpSession &session,
                                        FtpInstruction &instruction) {
  int listen_sockfd;
  struct in_addr ip_addr;
  std::string reply_content;

  instruction.clear();
  instruction.setInsType(TRANSFER_PASV_STANDBY_RES);

  do {
    ip_addr.s_addr = Socket::GetLocalAddress();

    listen_sockfd = Socket::TcpServerCreate(inet_ntoa(ip_addr), 0);
    if (listen_sockfd == -1) {
      Socket::SocketClose(listen_sockfd);
      instruction.setInsExecFlag(false);
      break;
    }

    Socket::TcpListen(listen_sockfd, 1);

    session.set_pasv_listen_sockfd(listen_sockfd);

    struct sockaddr_in local;
    socklen_t socklen = sizeof(struct sockaddr);
    getsockname(listen_sockfd, (struct sockaddr *) &local, &socklen);

    unsigned short port = local.sin_port << 8 | local.sin_port >> 8;
    unsigned char ipv4[4];
    ipv4[0] = (unsigned char) local.sin_addr.s_addr;
    ipv4[1] = (unsigned char) local.sin_addr.s_addr >> 8;
    ipv4[2] = (unsigned char) local.sin_addr.s_addr >> 16;
    ipv4[3] = (unsigned char) local.sin_addr.s_addr >> 24;

    reply_content = "Entering PASV mode (" + std::to_string(ipv4[0]) + ","
        + std::to_string(ipv4[1]) + "," + std::to_string(ipv4[2]) + ","
        + std::to_string(ipv4[3]) + "," + std::to_string(port / 256) + ","
        + std::to_string(port % 256) + ")";

    session.set_trans_ip_address(ip_addr.s_addr);
    session.set_trans_port(port);

    instruction.setInsContent((char *) reply_content.c_str(),
                              reply_content.size());
    instruction.setInsExecFlag(true);
    instruction.setInsContentLength(reply_content.size());

    session.set_transmode(PASV_MODE_ENABLE);

  } while (0);

  return SendInstruction(session.ipc_data_sockfd(), instruction);
}

int FtpWarpper::PORT_FtpTransferStandby(FtpSession &session,
                                        FtpInstruction &instruction) {
  unsigned short port = 0;
  unsigned int ip_address = 0;

  unsigned int p_port[2];
  unsigned int p_ipaddress[4];

  std::string request_content(instruction.getInsContent());
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

  session.set_trans_port(port);
  session.set_trans_ip_address(ip_address);

  std::string reply_content = "PORT SUCCESS";

  instruction.clear();
  instruction.setInsType(TRANSFER_PORT_STANDBY_RES);
  instruction.setInsExecFlag(true);
  instruction.setInsContent((char *) reply_content.c_str(),
                            reply_content.size());
  instruction.setInsContentLength(reply_content.size());

  session.set_transmode(PORT_MODE_ENABLE);
  return SendInstruction(session.ipc_data_sockfd(), instruction);
}

int FtpWarpper::TrySendCommand(FtpSession &session,
                               FtpInstruction &instruction) {
  int result = 0;

  if (-1 != session.trans_sockfd()) {
    result = Socket::TcpSend(session.trans_sockfd(),
                             (unsigned char *) instruction.getInsContent(),
                             instruction.getInsContentLength());
  }
  std::cout << strerror(Socket::CheckSockError(session.trans_sockfd()))
            << std::endl;

  Socket::SocketClose(session.trans_sockfd());

  session.set_trans_sockfd(INVALID_SOCKET);

  instruction.clear();
  instruction.setInsType(TRANSFER_SENDCOMMAND_RES);

  if (result > 0) {
    instruction.setInsExecFlag(true);
    instruction.setInsContent("Transfer complete.",
                              strlen("Transfer complete."));
    instruction.setInsContentLength(strlen("Transfer complete."));
  } else {
    instruction.setInsExecFlag(false);
    instruction.setInsContentLength(0);
  }

  return SendInstruction(session.ipc_data_sockfd(), instruction);
}

int FtpWarpper::TryContact(FtpSession &session, FtpInstruction &instruction) {
  int result;

  struct sockaddr_in remote;

  struct in_addr ip_addr;

  ip_addr.s_addr = session.trans_ip_address();

  std::cout << __FUNCTION__ << " ip_addr = " << ip_addr.s_addr << std::endl;
  std::cout << __FUNCTION__ << " inet_ntoa(ip_addr) = " << inet_ntoa(ip_addr)
            << std::endl;
  std::cout << __FUNCTION__ << " session.GetTransferPort() = "
            << ntohs(session.trans_port()) << std::endl;

  switch (session.transmode()) {
    case PASV_MODE_ENABLE: {
      result = Socket::TcpAccept(session.pasv_listen_sockfd(),
                                 (struct sockaddr *) &remote, 3 * 1000);
    }
      break;

    case PORT_MODE_ENABLE: {
      result = Socket::TcpConnect(inet_ntoa(ip_addr), session.trans_port(),
                                  3 * 1000);
    }
      break;
  }

  instruction.clear();

  if (-1 != result) {
    session.set_trans_sockfd(result);

    instruction.setInsExecFlag(true);
    instruction.setInsContentLength(0);
  } else {

    session.set_trans_sockfd(INVALID_SOCKET);

    instruction.setInsExecFlag(false);
    instruction.setInsContentLength(0);
  }

  return SendInstruction(session.ipc_data_sockfd(), instruction);
}

int FtpWarpper::TryFileDownload(FtpSession &session,
                                FtpInstruction &instruction) {
  int result;
  int nbytes;
  int total_bytes = 0;
  int file_sockfd;
  std::string filepath;
  struct stat fileStat;
  std::string reply_content;

  filepath = std::string(instruction.getInsContent());
  filepath = Utils::DeleteSpace(filepath);

  instruction.clear();
  instruction.setInsType(TRANSFER_FILEDOWNLOAD_RES);
  instruction.setInsContentLength(0);
  instruction.setInsExecFlag(false);

  //__filepath = Utils::GetConvString("gb2312", "utf-8", __filepath);

  filepath = Utils::DeleteSpace(filepath);

  do {

    result = Utils::GetFileAttribute(filepath, &fileStat);
    if (-1 == result) {
      std::cout << filepath << " is not exist!" << std::endl;
      reply_content = "Transfer fail.";
      instruction.setInsContent(reply_content.c_str(), reply_content.size());
      instruction.setInsContentLength(reply_content.size());
      break;
    }

    file_sockfd = open(filepath.c_str(), O_RDONLY);
    if (file_sockfd == -1) {
      perror("open");
      break;
    }

    int TransferSocket = session.trans_sockfd();
    fd_set fds;

    /* sendfile */
    do {
      if (fileStat.st_size == total_bytes) {
        instruction.setInsExecFlag(true);
        std::cout << "Download : Transfer ok." << std::endl;
        reply_content = "Transfer complete. Total "
            + std::to_string(total_bytes) + " bytes.";
        break;
      }

      if (session.has_data_abort()) {
        session.set_data_abort_flag(false);
        std::cout << "Download : Transfer abort." << std::endl;
        reply_content = "Transfer fail.";
        break;
      }

      result = Socket::IOMonitor(&TransferSocket, 1, 100, WRITEFDS_TYPE, fds);
      if (result > 0) {
        if (FD_ISSET(TransferSocket, &fds)) {
          //std::cout << "Totalbytes = " << totalBytes << std::endl;

          nbytes = 4096;
          if (nbytes > fileStat.st_size - total_bytes)
            nbytes = fileStat.st_size - total_bytes;

          /* File stream : Server send to client */
          nbytes = sendfile(session.trans_sockfd(), file_sockfd, NULL, nbytes);
          if (nbytes > 0) {
            total_bytes += nbytes;
            continue;
          } else {
            result = Socket::CheckSockError(session.trans_sockfd());
            if (result != 0) {
              if (result == EINTR)
                continue;
              else if (result == EAGAIN)  //no data read, means sendfile get (EOF)
              {
                instruction.setInsExecFlag(true);
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

    Socket::SocketClose(session.trans_sockfd());
    session.set_trans_sockfd(INVALID_SOCKET);

    instruction.setInsContent(reply_content.c_str(), reply_content.size());
    instruction.setInsContentLength(reply_content.size());

  } while (0);

  if (file_sockfd >= 0) {
    close(file_sockfd);
  }

  return SendInstruction(session.ipc_data_sockfd(), instruction);

}

int FtpWarpper::TryFileUpload(FtpSession &session,
                              FtpInstruction &instruction) {
  int result;
  int file_sockfd;
  std::string filepath;
  std::string reply_content;
  int total_bytes = 0;

  filepath = std::string(instruction.getInsContent());

  instruction.clear();
  instruction.setInsType(TRANSFER_FILEUPLOAD_RES);
  instruction.setInsContentLength(0);
  instruction.setInsExecFlag(false);

  char out[CONVEROUTLEN];

  memset(out, 0, sizeof(out));

  CodeConverter cc = CodeConverter("gb2312", "utf-8");
  cc.convert((char *) filepath.c_str(), strlen(filepath.c_str()), out,
  CONVEROUTLEN);

  filepath = std::string(out);

  filepath = Utils::DeleteSpace(filepath);

  std::cout << "upload file : " << filepath << std::endl;

  int nbytes = 1;
  int TransferSocket = session.trans_sockfd();
  fd_set fds;

  do {

    file_sockfd = open(filepath.c_str(), O_APPEND | O_CREAT);
    if (file_sockfd == -1) {
      perror("open");
      reply_content = "Transfer fail.";
      instruction.setInsContent(reply_content.c_str(), reply_content.size());
      instruction.setInsContentLength(reply_content.size());
      break;
    }

    do {

      if (session.has_data_abort()) {
        session.set_data_abort_flag(false);
        std::cout << "Upload : Transfer abort." << std::endl;
        reply_content = "Transfer fail.";
        break;
      }

      result = Socket::IOMonitor(&TransferSocket, 1, 100, WRITEFDS_TYPE, fds);
      if (result > 0) {
        if (FD_ISSET(TransferSocket, &fds)) {
          /* File stream : client send to server */
          result = sendfile(file_sockfd, session.trans_sockfd(), nullptr,
                            nbytes);
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
            instruction.setInsExecFlag(true);
            break;
          }
        }

      }

    } while (1);

    Socket::SocketClose(session.trans_sockfd());
    session.set_trans_sockfd(INVALID_SOCKET);

    instruction.setInsContent(reply_content.c_str(), reply_content.size());
    instruction.setInsContentLength(reply_content.size());

  } while (0);

  if (file_sockfd >= 0) {
    close(file_sockfd);
  }

  return SendInstruction(session.ipc_data_sockfd(), instruction);
}

/* ftp contorl processer handler */
int FtpWarpper::ftp_cwd(FtpSession &session, char *context) {
  std::string reply_content;

  std::string ftp_path = std::string(context);

  int errcode;
  struct stat fileStat;

  char out[CONVEROUTLEN];

  memset(out, 0, sizeof(out));

  CodeConverter cc("GB2312", "UTF-8");

  cc.convert((char *) ftp_path.c_str(), strlen(ftp_path.c_str()), out,
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

  return FtpReply(session, errcode, reply_content);
}

int FtpWarpper::ftp_abor(FtpSession &session, char *context) {
  FtpInstruction instruction;

  instruction.clear();

  instruction.setInsExecFlag(true);
  instruction.setInsType(TRANSFER_ABORT_REQ);

  SendInstruction(session.ipc_ctrl_sockfd(), instruction);
  FtpReply(session, FTP_ABOROK, std::string("Abort ok."));

  return 0;
}

int FtpWarpper::ftp_list(FtpSession &session, char *context) {
  FtpInstruction instruction;

  /* 1. try to notice the ftp-data processer to connect */
  instruction.clear();

  instruction.setInsExecFlag(true);
  instruction.setInsType(TRANSFER_TRY_CONNNECT_REQ);

  SendInstruction(session.ipc_ctrl_sockfd(), instruction);
  FtpReply(session, FTP_DATACONN,
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

int FtpWarpper::ftp_pass(FtpSession &session, char *context) {
  std::string password(context);
  password = Utils::DeleteSpace(password);
  session.set_password(password);
  std::string reply_content =
      "Welcome to my ftpServer . Author: fengxueda531@163.com.";

  return FtpReply(session, FTP_LOGINOK, reply_content);
}

int FtpWarpper::ftp_port(FtpSession &session, char *context) {
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

int FtpWarpper::ftp_quit(FtpSession &session, char *context) {
  std::string ReplyContent = "Goodbye";

  FtpReply(session, FTP_GOODBYE, ReplyContent);

  //exit(EXIT_SUCCESS);

  return 0;
}

int FtpWarpper::ftp_retr(FtpSession &session, char *context) {
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

int FtpWarpper::ftp_stor(FtpSession &session, char *context) {
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

int FtpWarpper::ftp_syst(FtpSession &session, char *context) {
  const std::string reply_content = "UNIX Type: L8";
  return FtpReply(session, FTP_SYSTOK, reply_content);
}

int FtpWarpper::ftp_type(FtpSession &session, char *context) {
  std::string format(context);
  format = Utils::DeleteSpace(format);

  std::string reply_content;
  reply_content = "Type set to " + format;

  return FtpReply(session, FTP_TYPEOK, reply_content);
}

int FtpWarpper::ftp_user(FtpSession &session, char *context) {
  std::string username(context);

  username = Utils::DeleteSpace(username);

  session.set_user_name(username);

  return FtpReply(session, FTP_GIVEPWORD, "User name okay, need password.");
}

int FtpWarpper::ftp_pasv(FtpSession &session, char *context) {
  FtpInstruction instruction;

  instruction.clear();

  instruction.setInsExecFlag(true);
  instruction.setInsType(TRANSFER_PASV_STANDBY_REQ);
  instruction.setInsContentLength(0);

  SendInstruction(session.ipc_ctrl_sockfd(), instruction);

  return 0;
}

int FtpWarpper::ftp_feat(FtpSession &session, char *context) {
  const std::string reply_content = "Not support this command";
  return FtpReply(session, FTP_COMMANDNOTIMPL, reply_content);
}

int FtpWarpper::ftp_rest(FtpSession &session, char *context) {
  const std::string ReplyContent = "Reset ok!";
  return FtpReply(session, FTP_RESTOK, ReplyContent);
}

int FtpWarpper::ftp_pwd(FtpSession &session, char *context) {
  std::string reply_content;
  std::string directory;

  directory = "\"" + session.directory() + "\"";
  reply_content = directory + " is current directory.";
  return FtpReply(session, FTP_PWDOK, reply_content);
}

int FtpWarpper::ftp_cdup(FtpSession &session, char *context) {
  std::string reply_content;
  std::string directory;

  directory = session.directory();
  directory = Utils::GetLastDirPath(directory);

  if (directory.size() < session.root_path().size())
    directory = session.root_path();

  std::cout << "__FTP_CDUP : " << directory << std::endl;

  session.set_directory(directory);

  reply_content = "Directory changed to " + directory;

  return FtpReply(session, FTP_CWDOK, reply_content);
}

int FtpWarpper::ServiceStart(FtpSession &session) {
  int pid;

  MutiProcesserInit(session);

  pid = fork();
  if (pid == -1) {
    perror("fork");
  } else if (pid == 0) {
    /* ftp-data processer */
    std::cout << getpid() << std::endl;
    Socket::SocketClose(session.ipc_ctrl_sockfd());
    Socket::SocketClose(session.listen_sockfd());
    IPC_FTPTransferHandler(session);
  } else if (pid > 0) {
    /* ftp-control processer */
    std::cout << getpid() << std::endl;
    session.set_data_pid(pid);
    Socket::SocketClose(session.ipc_data_sockfd());
    Socket::SocketClose(session.sockfd());
    FTPControlHandler(session);
    waitpid(pid, NULL, 0);

    std::cout << "PID : " << pid << " FTP-DATA Processer exit." << std::endl;
    std::cout << "PID : " << getpid() << " FTP-CONTORL Processer exit."
              << std::endl;
  }

  return 0;
}

void FtpWarpper::MutiProcesserInit(FtpSession &session) {

#if defined(__linux__)

  int sockfd[2];

  assert(0 == socketpair(AF_UNIX, SOCK_STREAM, 0, sockfd));

  session.set_ipc_ctrl_sockfd(sockfd[0]);
  session.set_ipc_data_sockfd(sockfd[1]);

  Socket::SetNonBlock(session.ipc_ctrl_sockfd());
  Socket::SetNonBlock(session.ipc_data_sockfd());

#endif
}

int FtpWarpper::FTPProtocolParser(FtpSession &session, char *data) {
  char command[5];
  char context[2048];

  memset(command, 0, sizeof(command));
  memset(context, 0, sizeof(context));

  memcpy(command, data, sizeof(command) - 1);
  memcpy(context, data + 4, (strlen(data)) - 4);

  std::string ftpcommand = std::string(command);
  ftpcommand = Utils::DeleteSpace(ftpcommand);
  ftpcommand = Utils::DeleteCRLF(ftpcommand);

  std::cout << data << std::endl;
  if (handlers_.count(ftpcommand) == 0) {
    FtpReply(session, FTP_COMMANDNOTIMPL, "Not Support Now.");
    return -1;
  }
  handlers_[ftpcommand](session, context);

  return 0;
}

/* parent processer(ftp-control) */
int FtpWarpper::FTPControlHandler(FtpSession &session) {
  int nbytes;
  char buffer[2048];
  int SocketList[2];

  memset(buffer, 0, sizeof(buffer));

  FtpReply(
      session, FTP_GREET,
      "Environment: Linux system. Used UNIX BSD Socket. (FtpServer ver. 0.1)");

  fd_set fds;
  FtpInstruction instruction;
  SocketList[0] = session.ctrl_sockfd();
  SocketList[1] = session.ipc_ctrl_sockfd();

  do {

    memset(buffer, 0, sizeof(buffer));
    nbytes = Socket::IOMonitor(SocketList, sizeof(SocketList) / sizeof(int),
                               100, READFDS_TYPE, fds);
    if (nbytes == 0)
      continue;
    else if (nbytes > 0) {
      if (FD_ISSET(session.ctrl_sockfd(), &fds)) {
        nbytes = Socket::TcpReadLine(session.ctrl_sockfd(), buffer,
                                     sizeof(buffer));
        if (nbytes <= 0) {
          if (Socket::CheckSockError(session.ctrl_sockfd()) == EAGAIN)
            continue;

          perror("FTPControlHandler ----> TcpReadOneLine");
          Socket::SocketClose(session.ctrl_sockfd());
          Socket::SocketClose(session.ipc_ctrl_sockfd());

          sleep(1);

          /* kill ftp-data processer */
          kill(session.data_pid(), SIGTERM);

          break;
        } else {
          FTPProtocolParser(session, buffer);
        }
      }

      /* IPC ftp-control process */
      if (FD_ISSET(session.ipc_ctrl_sockfd(), &fds)) {
        nbytes = RecvInstruction(session.ipc_ctrl_sockfd(), instruction);
        if (nbytes <= 0) {
          if (Socket::CheckSockError(session.ipc_ctrl_sockfd()) == EAGAIN)
            continue;

          perror("FTPControlHandler ---> IPC_RecvInstruction");
          Socket::SocketClose(session.ipc_ctrl_sockfd());
          Socket::SocketClose(session.ctrl_sockfd());

          sleep(1);

          /* kill ftp-data processer */
          kill(session.data_pid(), SIGTERM);

          break;
        } else {
          IPC_FTPControlHandler(session, instruction);
        }
      }

    }

  } while (1);

  return 0;
}

/* child processer(ftp-data) */
void FtpWarpper::IPC_FTPTransferHandler(FtpSession &session) {
  FtpInstruction instruction;

  instruction.clear();

  fd_set fds;
  int result;
  int ipc_socket = session.ipc_data_sockfd();

  do {
    result = Socket::IOMonitor(&ipc_socket, 1, 100, READFDS_TYPE, fds);
    if (result == 0)
      continue;
    else if (result > 0) {
      if (FD_ISSET(ipc_socket, &fds)) {
        result = RecvInstruction(session.ipc_data_sockfd(), instruction);
      }
    }

    if (result > 0) {
      switch (instruction.getInsType()) {
        case TRANSFER_PASV_STANDBY_REQ: {
          std::cout << "TRANSFER_PASV_STANDBY_REQ" << std::endl;
          PASV_FtpTransferStandby(session, instruction);
        }
          break;

        case TRANSFER_PORT_STANDBY_REQ: {
          std::cout << "TRANSFER_PORT_STANDBY_REQ" << std::endl;
          PORT_FtpTransferStandby(session, instruction);
        }
          break;

        case TRANSFER_TRY_CONNNECT_REQ: {
          std::cout << "TRANSFER_TRY_CONNNECT_REQ" << std::endl;
          TryContact(session, instruction);
        }
          break;

        case TRANSFER_SENDCOMMAND_REQ: {
          std::cout << "TRANSFER_SENDCOMMAND_REQ" << std::endl;
          TrySendCommand(session, instruction);
        }
          break;

        case TRANSFER_FILEUPLOAD_REQ: {
          std::cout << "TRANSFER_FILEUPLOAD_REQ" << std::endl;
          TryFileUpload(session, instruction);
        }
          break;

        case TRANSFER_FILEDOWNLOAD_REQ: {
          std::cout << "TRANSFER_FILEDOWNLOAD_REQ" << std::endl;
          TryFileDownload(session, instruction);
        }
          break;

        case TRANSFER_ABORT_REQ: {
          std::cout << "TRANSFER_ABORT_REQ" << std::endl;
          session.set_data_abort_flag(true);
        }
          break;

      }
    }

    instruction.clear();

  } while (1);

  Socket::SocketClose(session.ipc_data_sockfd());
  perror("IPC_FTPTransferHandler");
}

void FtpWarpper::IPC_FTPControlHandler(FtpSession &session,
                                       FtpInstruction &instruction) {
  int resp_code;
  std::string reply_content;

  switch (instruction.getInsType()) {
    case TRANSFER_PORT_STANDBY_RES: {
      std::cout << "TRANSFER_PORT_STANDBY_RES" << std::endl;

      if (instruction.IsInsExecSuccess()) {
        reply_content = std::string(instruction.getInsContent());
        resp_code = FTP_PORTOK;
      } else {
        reply_content = "Fail to Enter Port Mode.";
        resp_code = FTP_IP_LIMIT;
      }
    }
      break;

    case TRANSFER_PASV_STANDBY_RES: {
      std::cout << "TRANSFER_PASV_STANDBY_RES" << std::endl;

      if (instruction.IsInsExecSuccess()) {
        reply_content = std::string(instruction.getInsContent());
        resp_code = FTP_PASVOK;
      } else {
        reply_content = "Fail to Enter Passive Mode.";
        resp_code = FTP_IP_LIMIT;
      }
    }
      break;

    case TRANSFER_SENDCOMMAND_RES: {
      std::cout << "TRANSFER_SENDCOMMAND_RES" << std::endl;
      if (instruction.IsInsExecSuccess()) {
        reply_content = std::string(instruction.getInsContent());
        resp_code = FTP_TRANSFEROK;
      } else {
        reply_content = "Transfer fail.";
        resp_code = FTP_BADSENDFILE;
      }
    }
      break;

    case TRANSFER_FILEDOWNLOAD_RES: {
      std::cout << "TRANSFER_FILEDOWNLOAD_RES" << std::endl;
      if (instruction.IsInsExecSuccess()) {
        reply_content = std::string(instruction.getInsContent());
        resp_code = FTP_TRANSFEROK;
      } else {
        reply_content = "Transfer fail.";
        resp_code = FTP_BADSENDFILE;
      }
    }
      break;

    case TRANSFER_FILEUPLOAD_RES: {
      std::cout << "TRANSFER_FILEUPLOAD_RES" << std::endl;
      if (instruction.IsInsExecSuccess()) {
        reply_content = std::string(instruction.getInsContent());
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
    FtpReply(session, resp_code, reply_content);
  }

}

