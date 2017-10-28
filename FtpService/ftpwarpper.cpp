#include "ftpwarpper.h"
#include "socketsource.h"
#include "ftpcodes.h"
#include <unistd.h>
#include "utils.h"
#include <assert.h>
#include <dirent.h>
#include <sys/time.h>
#include <fstream>
#include <sys/sendfile.h>
#include <vector>

FtpWarpper::FtpWarpper() {
  CallBacksRegister();
}

FtpWarpper::~FtpWarpper() {
  CallBacksUnRegister();
}

void FtpWarpper::CallBacksRegister() {
  std::cout << __FUNCTION__ << std::endl;

  mProcessHandler.clear();

  mProcessHandler.insert(std::make_pair("ABOR", __FTP_ABOR));

  mProcessHandler.insert(std::make_pair("LIST", __FTP_LIST));

  mProcessHandler.insert(std::make_pair("PASS", __FTP_PASS));

  mProcessHandler.insert(std::make_pair("PORT", __FTP_PORT));

  mProcessHandler.insert(std::make_pair("PASV", __FTP_PASV));

  mProcessHandler.insert(std::make_pair("QUIT", __FTP_QUIT));

  mProcessHandler.insert(std::make_pair("RETR", __FTP_RETR));

  mProcessHandler.insert(std::make_pair("STOR", __FTP_STOR));

  mProcessHandler.insert(std::make_pair("SYST", __FTP_SYST));

  mProcessHandler.insert(std::make_pair("TYPE", __FTP_TYPE));

  mProcessHandler.insert(std::make_pair("USER", __FTP_USER));

  mProcessHandler.insert(std::make_pair("QUIT", __FTP_QUIT));

  mProcessHandler.insert(std::make_pair("CWD", __FTP_CWD));

  mProcessHandler.insert(std::make_pair("FEAT", __FTP_FEAT));

  mProcessHandler.insert(std::make_pair("REST", __FTP_REST));

  mProcessHandler.insert(std::make_pair("PWD", __FTP_PWD));

  mProcessHandler.insert(std::make_pair("CDUP", __FTP_CDUP));
}

void FtpWarpper::CallBacksUnRegister() {
  std::cout << __FUNCTION__ << std::endl;

  mProcessHandler.clear();
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
    return SocketSource::TcpSend(session.GetControlSocket(),
        (unsigned char *) PackedContent.c_str(), PackedContent.size());
  }
}

/* IPC operation */
int FtpWarpper::IPC_RecvInstruction(int __sockfrom,
    FtpInstruction &__instruction) {
  unsigned char *pInstruction = (unsigned char *) &__instruction;

  memset(pInstruction, 0, sizeof(__instruction));

  return SocketSource::TcpRecv(__sockfrom, pInstruction, sizeof(__instruction));

}

int FtpWarpper::IPC_SendInstruction(int __sockto,
    FtpInstruction &__instruction) {
  unsigned char *pInstruction = (unsigned char *) &__instruction;

  return SocketSource::TcpSend(__sockto, pInstruction, sizeof(__instruction));
}

/* ftp-data processer handler */
int FtpWarpper::PASV_FtpTransferStandby(FtpSession &session,
    FtpInstruction &__instruction) {
  int ListenSock;
  struct in_addr ip_addr;
  std::string ReplyContent;

  __instruction.clear();
  __instruction.setInsType(TRANSFER_PASV_STANDBY_RES);

  do {
    ip_addr.s_addr = SocketSource::GetLocalAddress();

    ListenSock = SocketSource::TcpServerCreate(inet_ntoa(ip_addr), 0);
    if (ListenSock == -1) {
      SocketSource::SocketClose(ListenSock);
      __instruction.setInsExecFlag(false);
      break;
    }

    SocketSource::TcpListen(ListenSock, 1);

    session.PASVListenSock = ListenSock;

    struct sockaddr_in local;
    socklen_t socklen = sizeof(struct sockaddr);
    getsockname(ListenSock, (struct sockaddr *) &local, &socklen);

    unsigned short port = local.sin_port << 8 | local.sin_port >> 8;
    unsigned char ipv4[4];
    ipv4[0] = (unsigned char) local.sin_addr.s_addr;
    ipv4[1] = (unsigned char) local.sin_addr.s_addr >> 8;
    ipv4[2] = (unsigned char) local.sin_addr.s_addr >> 16;
    ipv4[3] = (unsigned char) local.sin_addr.s_addr >> 24;

    ReplyContent = "Entering PASV mode (" + std::to_string(ipv4[0]) + ","
        + std::to_string(ipv4[1]) + "," + std::to_string(ipv4[2]) + ","
        + std::to_string(ipv4[3]) + "," + std::to_string(port / 256) + ","
        + std::to_string(port % 256) + ")";

    session.SetTransferIpAddress(ip_addr.s_addr);
    session.SetTransferPort(port);

    __instruction.setInsContent((char *) ReplyContent.c_str(),
        ReplyContent.size());
    __instruction.setInsExecFlag(true);
    __instruction.setInsContentLength(ReplyContent.size());

    session.SetTransferMode(PASV_MODE_ENABLE);

  } while (0);

  return IPC_SendInstruction(session.GetIPCTransferFD(), __instruction);
}

int FtpWarpper::PORT_FtpTransferStandby(FtpSession &session,
    FtpInstruction &__instruction) {
  unsigned short __port = 0;
  unsigned int __ipaddress = 0;

  unsigned int p_port[2];
  unsigned int p_ipaddress[4];

  std::string RequsetContent(__instruction.getInsContent());
  RequsetContent = Utils::DeleteCRLF(RequsetContent);
  RequsetContent = Utils::DeleteSpace(RequsetContent);

  std::sscanf(RequsetContent.c_str(), "%d,%d,%d,%d,%d,%d", &p_ipaddress[0],
      &p_ipaddress[1], &p_ipaddress[2], &p_ipaddress[3], &p_port[0],
      &p_port[1]);

  __port = p_port[0] | p_port[1] << 8;

  __ipaddress |= p_ipaddress[0];
  __ipaddress |= p_ipaddress[1] << 8;
  __ipaddress |= p_ipaddress[2] << 16;
  __ipaddress |= p_ipaddress[3] << 24;

  session.SetTransferPort(__port);
  session.SetTransferIpAddress(__ipaddress);

  std::string ReplyContent = "PORT SUCCESS";

  __instruction.clear();

  __instruction.setInsType(TRANSFER_PORT_STANDBY_RES);
  __instruction.setInsExecFlag(true);
  __instruction.setInsContent((char *) ReplyContent.c_str(),
      ReplyContent.size());
  __instruction.setInsContentLength(ReplyContent.size());

  session.SetTransferMode(PORT_MODE_ENABLE);

  return IPC_SendInstruction(session.GetIPCTransferFD(), __instruction);
}

int FtpWarpper::WORK_FtpTrySendCommand(FtpSession &session,
    FtpInstruction &__instruction) {
  int result = 0;

  if (-1 != session.GetTransferSocket()) {
    result = SocketSource::TcpSend(session.GetTransferSocket(),
        (unsigned char *) __instruction.getInsContent(),
        __instruction.getInsContentLength());
  }
  std::cout
      << strerror(SocketSource::CheckSockError(session.GetTransferSocket()))
      << std::endl;

  SocketSource::SocketClose(session.GetTransferSocket());

  session.SetTransferSocket(INVALID_SOCKET);

  __instruction.clear();
  __instruction.setInsType(TRANSFER_SENDCOMMAND_RES);

  if (result > 0) {
    __instruction.setInsExecFlag(true);
    __instruction.setInsContent("Transfer complete.",
        strlen("Transfer complete."));
    __instruction.setInsContentLength(strlen("Transfer complete."));
  } else {
    __instruction.setInsExecFlag(false);
    __instruction.setInsContentLength(0);
  }

  return IPC_SendInstruction(session.GetIPCTransferFD(), __instruction);
}

int FtpWarpper::WORK_FtpTryContact(FtpSession &session,
    FtpInstruction &__instruction) {
  int result;

  struct sockaddr_in remote;

  struct in_addr ip_addr;

  ip_addr.s_addr = session.GetTransferIpAddress();

  std::cout << __FUNCTION__ << " ip_addr = " << ip_addr.s_addr << std::endl;
  std::cout << __FUNCTION__ << " inet_ntoa(ip_addr) = " << inet_ntoa(ip_addr)
      << std::endl;
  std::cout << __FUNCTION__ << " session.GetTransferPort() = "
      << ntohs(session.GetTransferPort()) << std::endl;

  switch (session.GetTransferMode()) {
  case PASV_MODE_ENABLE: {
    result = SocketSource::TcpAccept(session.PASVListenSock,
        (struct sockaddr *) &remote, 3 * 1000);
  }
    break;

  case PORT_MODE_ENABLE: {
    result = SocketSource::TcpConnect(inet_ntoa(ip_addr),
        session.GetTransferPort(), 3 * 1000);
  }
    break;
  }

  __instruction.clear();

  if (-1 != result) {
    session.SetTransferSocket(result);

    __instruction.setInsExecFlag(true);
    __instruction.setInsContentLength(0);
  } else {

    session.SetTransferSocket(INVALID_SOCKET);

    __instruction.setInsExecFlag(false);
    __instruction.setInsContentLength(0);
  }

  return IPC_SendInstruction(session.GetIPCTransferFD(), __instruction);
}

int FtpWarpper::WORK_FtpTryFileDownload(FtpSession &session,
    FtpInstruction &__instruction) {
  int result;
  int nbytes;
  int totalBytes = 0;
  int fileSocket;
  std::string __filepath;
  struct stat fileStat;
  std::string ReplyContent;

  __filepath = std::string(__instruction.getInsContent());
  __filepath = Utils::DeleteSpace(__filepath);

  __instruction.clear();
  __instruction.setInsType(TRANSFER_FILEDOWNLOAD_RES);
  __instruction.setInsContentLength(0);
  __instruction.setInsExecFlag(false);

  //__filepath = Utils::GetConvString("gb2312", "utf-8", __filepath);

  __filepath = Utils::DeleteSpace(__filepath);

  do {

    result = Utils::GetFileAttribute(__filepath, &fileStat);
    if (-1 == result) {
      std::cout << __filepath << " is not exist!" << std::endl;
      ReplyContent = "Transfer fail.";
      __instruction.setInsContent(ReplyContent.c_str(), ReplyContent.size());
      __instruction.setInsContentLength(ReplyContent.size());
      break;
    }

    fileSocket = open(__filepath.c_str(), O_RDONLY);
    if (fileSocket == -1) {
      perror("open");
      break;
    }

    int TransferSocket = session.GetTransferSocket();
    fd_set fds;

    /* sendfile */
    do {
      if (fileStat.st_size == totalBytes) {
        __instruction.setInsExecFlag(true);
        std::cout << "Download : Transfer ok." << std::endl;
        ReplyContent = "Transfer complete. Total " + std::to_string(totalBytes)
            + " bytes.";
        break;
      }

      if (session.GetFtpDataAbort() == true) {
        session.SetFtpDataAbortFlag(false);
        std::cout << "Download : Transfer abort." << std::endl;
        ReplyContent = "Transfer fail.";
        break;
      }

      result = SocketSource::IOMonitor(&TransferSocket, 1, 100, WRITEFDS_TYPE,
          fds);
      if (result > 0) {
        if (FD_ISSET(TransferSocket, &fds)) {
          //std::cout << "Totalbytes = " << totalBytes << std::endl;

          nbytes = 4096;
          if (nbytes > fileStat.st_size - totalBytes)
            nbytes = fileStat.st_size - totalBytes;

          /* File stream : Server send to client */
          nbytes = sendfile(session.GetTransferSocket(), fileSocket, NULL,
              nbytes);
          if (nbytes > 0) {
            totalBytes += nbytes;
            continue;
          } else {
            result = SocketSource::CheckSockError(session.GetTransferSocket());
            if (result != 0) {
              if (result == EINTR)
                continue;
              else if (result == EAGAIN) //no data read, means sendfile get (EOF)
              {
                __instruction.setInsExecFlag(true);
                std::cout << "Download : Transfer ok." << std::endl;
                ReplyContent = "Transfer complete. Total "
                    + std::to_string(totalBytes) + " bytes.";
                break;
              } else {
                std::cout << "Download : Transfer fail." << std::endl;
                ReplyContent = "Transfer fail.";
                perror("sendfile");
                break;
              }
            }
          }
        }

      }

    } while (1);

    SocketSource::SocketClose(session.GetTransferSocket());
    session.SetTransferSocket(INVALID_SOCKET);

    __instruction.setInsContent(ReplyContent.c_str(), ReplyContent.size());
    __instruction.setInsContentLength(ReplyContent.size());

  } while (0);

  if (fileSocket >= 0) {
    close(fileSocket);
  }

  return IPC_SendInstruction(session.GetIPCTransferFD(), __instruction);

}

int FtpWarpper::WORK_FtpTryFileUpload(FtpSession &session,
    FtpInstruction &__instruction) {
  int result;
  int fileSocket;
  std::string __filepath;
  std::string ReplyContent;
  int TotalBytes = 0;

  __filepath = std::string(__instruction.getInsContent());

  __instruction.clear();
  __instruction.setInsType(TRANSFER_FILEUPLOAD_RES);
  __instruction.setInsContentLength(0);
  __instruction.setInsExecFlag(false);

  char out[CONVEROUTLEN];

  memset(out, 0, sizeof(out));

  CodeConverter cc = CodeConverter("gb2312", "utf-8");
  cc.convert((char *) __filepath.c_str(), strlen(__filepath.c_str()), out,
      CONVEROUTLEN);

  __filepath = std::string(out);

  __filepath = Utils::DeleteSpace(__filepath);

  std::cout << "upload file : " << __filepath << std::endl;

  int nbytes = 1;
  int TransferSocket = session.GetTransferSocket();
  fd_set fds;

  do {

    fileSocket = open(__filepath.c_str(), O_APPEND | O_CREAT);
    if (fileSocket == -1) {
      perror("open");
      ReplyContent = "Transfer fail.";
      __instruction.setInsContent(ReplyContent.c_str(), ReplyContent.size());
      __instruction.setInsContentLength(ReplyContent.size());
      break;
    }

    do {

      if (session.GetFtpDataAbort() == true) {
        session.SetFtpDataAbortFlag(false);
        std::cout << "Upload : Transfer abort." << std::endl;
        ReplyContent = "Transfer fail.";
        break;
      }

      result = SocketSource::IOMonitor(&TransferSocket, 1, 100, WRITEFDS_TYPE,
          fds);
      if (result > 0) {
        if (FD_ISSET(TransferSocket, &fds)) {
          /* File stream : client send to server */
          result = sendfile(fileSocket, session.GetTransferSocket(), NULL,
              nbytes);
          if (result > 0) {
            std::cout << "upload : result = " << result << std::endl;
            TotalBytes += result;
          } else if (result < 0) {

            if (result == EINTR)
              continue;
            else if (result == EAGAIN) {
              break;
            } else {
              perror("sendfile");
              std::cout << "Upload : Transfer fail." << std::endl;
              ReplyContent = "Transfer fail.";
              break;
            }

          } else if (result == 0) {
            std::cout << "Upload : Transfer ok." << std::endl;
            ReplyContent = "Transfer complete. Total "
                + std::to_string(TotalBytes) + " bytes.";
            __instruction.setInsExecFlag(true);
            break;
          }
        }

      }

    } while (1);

    SocketSource::SocketClose(session.GetTransferSocket());
    session.SetTransferSocket(INVALID_SOCKET);

    __instruction.setInsContent(ReplyContent.c_str(), ReplyContent.size());
    __instruction.setInsContentLength(ReplyContent.size());

  } while (0);

  if (fileSocket >= 0) {
    close(fileSocket);
  }

  return IPC_SendInstruction(session.GetIPCTransferFD(), __instruction);
}

/* ftp contorl processer handler */
int FtpWarpper::__FTP_CWD(FtpSession &session, char *context) {
  std::string ReplyContent;

  std::string __ftpPath = std::string(context);

  int errcode;
  struct stat fileStat;

  char out[CONVEROUTLEN];

  memset(out, 0, sizeof(out));

  CodeConverter cc("GB2312", "UTF-8");

  cc.convert((char *) __ftpPath.c_str(), strlen(__ftpPath.c_str()), out,
      CONVEROUTLEN);

  __ftpPath = std::string(out);

  __ftpPath = Utils::DeleteSpace(__ftpPath);
  __ftpPath = Utils::DeleteCRLF(__ftpPath);

  std::string temp = session.GetCurrentPath();

  if (__ftpPath.size() < session.GetRootPath().size()) {
    __ftpPath = session.GetRootPath();
  }

  if (Utils::CheckSymbolExsit(__ftpPath, '/') == -1) {
    if (temp.at(temp.size() - 1) != '/') {
      temp += "/";
    }
    __ftpPath = temp + __ftpPath;
  }

  session.SetCurrentPath(__ftpPath);

  std::cout << "__CWD : " << __ftpPath << std::endl;

  if (0 == stat(__ftpPath.c_str(), &fileStat)) {
    if (S_ISDIR(fileStat.st_mode)) {
      errcode = FTP_CWDOK;
      ReplyContent = "Directory changed to " + __ftpPath;
    } else {
      errcode = FTP_CWDOK;
    }
  } else {
    ReplyContent = "Not such file or directory.";
    errcode = FTP_NOPERM;
  }

  return FtpReply(session, errcode, ReplyContent);
}

int FtpWarpper::__FTP_ABOR(FtpSession &session, char *context) {
  FtpInstruction instruction;

  instruction.clear();

  instruction.setInsExecFlag(true);
  instruction.setInsType(TRANSFER_ABORT_REQ);

  IPC_SendInstruction(session.GetIPCContorlFD(), instruction);
  FtpReply(session, FTP_ABOROK, std::string("Abort ok."));

  return 0;
}

int FtpWarpper::__FTP_LIST(FtpSession &session, char *context) {
  FtpInstruction instruction;

  /* 1. try to notice the ftp-data processer to connect */
  instruction.clear();

  instruction.setInsExecFlag(true);
  instruction.setInsType(TRANSFER_TRY_CONNNECT_REQ);

  IPC_SendInstruction(session.GetIPCContorlFD(), instruction);
  FtpReply(session, FTP_DATACONN,
      std::string("Opening ASCII mode data connection for /bin/ls"));

  /* 2. try to send command back to client */
  std::string filePath;
  filePath = session.GetCurrentPath();

  std::cout << "__FTP_LIST :  filePath = " << filePath << std::endl;
  if (filePath.at(filePath.size() - 1) != '/')
    filePath += "/";

  std::string DirListString = Utils::GetListString(filePath);

  instruction.clear();

  instruction.setInsExecFlag(true);
  instruction.setInsType(TRANSFER_SENDCOMMAND_REQ);
  instruction.setInsContent(DirListString.c_str(), DirListString.size());
  instruction.setInsContentLength(DirListString.size());

  return IPC_SendInstruction(session.GetIPCContorlFD(), instruction);

}

int FtpWarpper::__FTP_PASS(FtpSession &session, char *context) {
  std::string __password(context);

  __password = Utils::DeleteSpace(__password);

  session.SetPassword(__password);

  std::string ReplyContent;

  ReplyContent = "Welcome to my ftpServer . Author: fengxueda531@163.com.";

  return FtpReply(session, FTP_LOGINOK, ReplyContent);
}

int FtpWarpper::__FTP_PORT(FtpSession &session, char *context) {
  FtpInstruction __instruction;

  __instruction.clear();

  __instruction.setInsType(TRANSFER_PORT_STANDBY_REQ);
  __instruction.setInsExecFlag(true);
  __instruction.setInsContent(context, strlen(context));
  __instruction.setInsContentLength(strlen(context));

  /* send connect request to ftp-data processer */
  IPC_SendInstruction(session.GetIPCContorlFD(), __instruction);

}

int FtpWarpper::__FTP_QUIT(FtpSession &session, char *context) {
  std::string ReplyContent = "Goodbye";

  FtpReply(session, FTP_GOODBYE, ReplyContent);

  //exit(EXIT_SUCCESS);

  return 0;
}

int FtpWarpper::__FTP_RETR(FtpSession &session, char *context) {
  FtpInstruction instruction;

  /* 1. try to notice the ftp-data processer to connect */
  instruction.clear();

  instruction.setInsExecFlag(true);
  instruction.setInsType(TRANSFER_TRY_CONNNECT_REQ);

  IPC_SendInstruction(session.GetIPCContorlFD(), instruction);

  /*2. try to send file to client */
  std::string temp;
  std::string filePath = std::string(context);

  instruction.clear();

  instruction.setInsExecFlag(true);
  instruction.setInsType(TRANSFER_FILEDOWNLOAD_REQ);
  filePath = Utils::DeleteSpace(filePath);
  filePath = Utils::DeleteCRLF(filePath);

  temp = session.GetCurrentPath();
  if (temp.at(temp.size() - 1) != '/') {
    filePath = temp + "/" + filePath;
  } else {
    filePath = temp + filePath;
  }

  instruction.setInsContent(filePath.c_str(), filePath.size());
  instruction.setInsContentLength(filePath.size());

  return IPC_SendInstruction(session.GetIPCContorlFD(), instruction);
}

int FtpWarpper::__FTP_STOR(FtpSession &session, char *context) {
  FtpInstruction instruction;

  /* 1. try to notice the ftp-data processer to connect */
  instruction.clear();

  instruction.setInsExecFlag(true);
  instruction.setInsType(TRANSFER_TRY_CONNNECT_REQ);

  IPC_SendInstruction(session.GetIPCContorlFD(), instruction);

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

  return IPC_SendInstruction(session.GetIPCContorlFD(), instruction);
}

int FtpWarpper::__FTP_SYST(FtpSession &session, char *context) {
  std::string ReplyContent;

  ReplyContent = "UNIX Type: L8";

  return FtpReply(session, FTP_SYSTOK, ReplyContent);
}

int FtpWarpper::__FTP_TYPE(FtpSession &session, char *context) {
  std::string __format(context);

  __format = Utils::DeleteSpace(__format);

  std::string ReplyContent;

  ReplyContent = "Type set to " + __format;

  return FtpReply(session, FTP_TYPEOK, ReplyContent);
}

int FtpWarpper::__FTP_USER(FtpSession &session, char *context) {
  std::string __username(context);

  __username = Utils::DeleteSpace(__username);

  session.SetUsername(__username);

  return FtpReply(session, FTP_GIVEPWORD, "User name okay, need password.");
}

int FtpWarpper::__FTP_PASV(FtpSession &session, char *context) {
  FtpInstruction __instruction;

  __instruction.clear();

  __instruction.setInsExecFlag(true);
  __instruction.setInsType(TRANSFER_PASV_STANDBY_REQ);
  __instruction.setInsContentLength(0);

  IPC_SendInstruction(session.GetIPCContorlFD(), __instruction);
}

int FtpWarpper::__FTP_FEAT(FtpSession &session, char *context) {
  std::string ReplyContent;

  ReplyContent = "Not support this command";

  return FtpReply(session, FTP_COMMANDNOTIMPL, ReplyContent);
}

int FtpWarpper::__FTP_REST(FtpSession &session, char *context) {
  std::string ReplyContent;

  ReplyContent = "Reset ok!";

  return FtpReply(session, FTP_RESTOK, ReplyContent);
}

int FtpWarpper::__FTP_PWD(FtpSession &session, char *context) {
  std::string ReplyContent;

  std::string CurrentPath;

  CurrentPath = "\"" + session.GetCurrentPath() + "\"";

  ReplyContent = CurrentPath + " is current directory.";

  return FtpReply(session, FTP_PWDOK, ReplyContent);
}

int FtpWarpper::__FTP_CDUP(FtpSession &session, char *context) {
  std::string ReplyContent;

  std::string CurrentPath;

  CurrentPath = session.GetCurrentPath();

  CurrentPath = Utils::GetLastDirPath(CurrentPath);

  if (CurrentPath.size() < session.GetRootPath().size())
    CurrentPath = session.GetRootPath();

  std::cout << "__FTP_CDUP : " << CurrentPath << std::endl;

  session.SetCurrentPath(CurrentPath);

  ReplyContent = "Directory changed to " + CurrentPath;

  return FtpReply(session, FTP_CWDOK, ReplyContent);
}

void FtpWarpper::SignalHandler(int __signalNo, siginfo_t* __signalInfo,
    void* __content) {
  if (__signalNo == SIGUSR1) {
    switch (2) {
    case 1:
      break;
    }
  }
}

int FtpWarpper::SignalRegister() {
  struct sigaction act;
  act.sa_sigaction = SignalHandler;
  sigemptyset(&act.sa_mask);  //清空信号集
  act.sa_flags = SA_SIGINFO; //带参数signal

  if (sigaction(SIGINT, &act, NULL) == -1) {
    perror("sigaction");
    exit(EXIT_FAILURE);
  }
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
    SocketSource::SocketClose(session.GetIPCContorlFD());
    SocketSource::SocketClose(session.getServerListenSocket());
    IPC_FTPTransferHandler(session);
  } else if (pid > 0) {
    /* ftp-control processer */
    std::cout << getpid() << std::endl;
    session.SetFtpDataPID(pid);
    SocketSource::SocketClose(session.GetIPCTransferFD());
    SocketSource::SocketClose(session.getClientSocketID());
    FTPControlHandler(session);
    waitpid(pid, NULL, 0);

    std::cout << "PID : " << pid << " FTP-DATA Processer exit." << std::endl;
    std::cout << "PID : " << getpid() << " FTP-CONTORL Processer exit."
        << std::endl;
  }

  return 0;
}

int FtpWarpper::MutiProcesserInit(FtpSession &session) {

#if defined(__linux__)

  int sockfd[2];

  assert(0 == socketpair(AF_UNIX, SOCK_STREAM, 0, sockfd));

  session.SetIPCControlFD(sockfd[0]);
  session.SetIPCTransferFD(sockfd[1]);

  SocketSource::SetNonBlock(session.GetIPCContorlFD());
  SocketSource::SetNonBlock(session.GetIPCTransferFD());

#endif

  return 0;
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
  std::map<std::string, std::function<int(FtpSession&, char *)> >::iterator iter;
  iter = mProcessHandler.find(ftpcommand);
  if (iter == mProcessHandler.end()) {
    FtpReply(session, FTP_COMMANDNOTIMPL, "Not Support Now.");
    return -1;
  }

  iter->second(session, context);

  return 0;
}

/* parent processer(ftp-control) */
int FtpWarpper::FTPControlHandler(FtpSession &session) {
  int nbytes;
  char buffer[2048];
  int SocketList[2];

  memset(buffer, 0, sizeof(buffer));

  FtpReply(session, FTP_GREET,
      "Environment: Linux system. Used UNIX BSD Socket. (FtpServer ver. 0.1)");

  fd_set fds;
  FtpInstruction instruction;
  SocketList[0] = session.GetControlSocket();
  SocketList[1] = session.GetIPCContorlFD();

  do {

    memset(buffer, 0, sizeof(buffer));
    nbytes = SocketSource::IOMonitor(SocketList,
        sizeof(SocketList) / sizeof(int), 100, READFDS_TYPE, fds);
    if (nbytes == 0)
      continue;
    else if (nbytes > 0) {
      if (FD_ISSET(session.GetControlSocket(), &fds)) {
        nbytes = SocketSource::TcpReadOneLine(session.GetControlSocket(),
            buffer, sizeof(buffer));
        if (nbytes <= 0) {
          if (SocketSource::CheckSockError(session.GetControlSocket()) == EAGAIN)
            continue;

          perror("FTPControlHandler ----> TcpReadOneLine");
          SocketSource::SocketClose(session.GetControlSocket());
          SocketSource::SocketClose(session.GetIPCContorlFD());

          sleep(1);

          /* kill ftp-data processer */
          kill(session.GetFtpDataPID(), SIGTERM);

          break;
        } else {
          FTPProtocolParser(session, buffer);
        }
      }

      /* IPC ftp-control process */
      if (FD_ISSET(session.GetIPCContorlFD(), &fds)) {
        nbytes = IPC_RecvInstruction(session.GetIPCContorlFD(), instruction);
        if (nbytes <= 0) {
          if (SocketSource::CheckSockError(session.GetIPCContorlFD()) == EAGAIN)
            continue;

          perror("FTPControlHandler ---> IPC_RecvInstruction");
          SocketSource::SocketClose(session.GetIPCContorlFD());
          SocketSource::SocketClose(session.GetControlSocket());

          sleep(1);

          /* kill ftp-data processer */
          kill(session.GetFtpDataPID(), SIGTERM);

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
int FtpWarpper::IPC_FTPTransferHandler(FtpSession &session) {
  FtpInstruction instruction;

  instruction.clear();

  fd_set fds;
  int result;
  int ipc_socket = session.GetIPCTransferFD();

  do {
    result = SocketSource::IOMonitor(&ipc_socket, 1, 100, READFDS_TYPE, fds);
    if (result == 0)
      continue;
    else if (result > 0) {
      if (FD_ISSET(ipc_socket, &fds)) {
        result = IPC_RecvInstruction(session.GetIPCTransferFD(), instruction);
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
        WORK_FtpTryContact(session, instruction);
      }
        break;

      case TRANSFER_SENDCOMMAND_REQ: {
        std::cout << "TRANSFER_SENDCOMMAND_REQ" << std::endl;
        WORK_FtpTrySendCommand(session, instruction);
      }
        break;

      case TRANSFER_FILEUPLOAD_REQ: {
        std::cout << "TRANSFER_FILEUPLOAD_REQ" << std::endl;
        WORK_FtpTryFileUpload(session, instruction);
      }
        break;

      case TRANSFER_FILEDOWNLOAD_REQ: {
        std::cout << "TRANSFER_FILEDOWNLOAD_REQ" << std::endl;
        WORK_FtpTryFileDownload(session, instruction);
      }
        break;

      case TRANSFER_ABORT_REQ: {
        std::cout << "TRANSFER_ABORT_REQ" << std::endl;
        session.SetFtpDataAbortFlag(true);
      }
        break;

      }
    }

    instruction.clear();

  } while (1);

  SocketSource::SocketClose(session.GetIPCTransferFD());
  perror("IPC_FTPTransferHandler");

  return 0;
}

int FtpWarpper::IPC_FTPControlHandler(FtpSession &session,
    FtpInstruction &__instruction) {
  int ResponseCode;
  std::string ReplyContent;

  switch (__instruction.getInsType()) {
  case TRANSFER_PORT_STANDBY_RES: {
    std::cout << "TRANSFER_PORT_STANDBY_RES" << std::endl;

    if (__instruction.IsInsExecSuccess()) {
      ReplyContent = std::string(__instruction.getInsContent());
      ResponseCode = FTP_PORTOK;
    } else {
      ReplyContent = "Fail to Enter Port Mode.";
      ResponseCode = FTP_IP_LIMIT;
    }
  }
    break;

  case TRANSFER_PASV_STANDBY_RES: {
    std::cout << "TRANSFER_PASV_STANDBY_RES" << std::endl;

    if (__instruction.IsInsExecSuccess()) {
      ReplyContent = std::string(__instruction.getInsContent());
      ResponseCode = FTP_PASVOK;
    } else {
      ReplyContent = "Fail to Enter Passive Mode.";
      ResponseCode = FTP_IP_LIMIT;
    }
  }
    break;

  case TRANSFER_SENDCOMMAND_RES: {
    std::cout << "TRANSFER_SENDCOMMAND_RES" << std::endl;
    if (__instruction.IsInsExecSuccess()) {
      ReplyContent = std::string(__instruction.getInsContent());
      ResponseCode = FTP_TRANSFEROK;
    } else {
      ReplyContent = "Transfer fail.";
      ResponseCode = FTP_BADSENDFILE;
    }
  }
    break;

  case TRANSFER_FILEDOWNLOAD_RES: {
    std::cout << "TRANSFER_FILEDOWNLOAD_RES" << std::endl;
    if (__instruction.IsInsExecSuccess()) {
      ReplyContent = std::string(__instruction.getInsContent());
      ResponseCode = FTP_TRANSFEROK;
    } else {
      ReplyContent = "Transfer fail.";
      ResponseCode = FTP_BADSENDFILE;
    }
  }
    break;

  case TRANSFER_FILEUPLOAD_RES: {
    std::cout << "TRANSFER_FILEUPLOAD_RES" << std::endl;
    if (__instruction.IsInsExecSuccess()) {
      ReplyContent = std::string(__instruction.getInsContent());
      ResponseCode = FTP_TRANSFEROK;
    } else {
      ReplyContent = "Transfer fail.";
      ResponseCode = FTP_BADSENDFILE;
    }
  }
    break;

  default: {
    ResponseCode = 0;
    ReplyContent.clear();
  }
    break;

  }

  if (ReplyContent.size() > 0) {
    ReplyContent = Utils::DeleteCRLF(ReplyContent);
    FtpReply(session, ResponseCode, ReplyContent);
  }

}

