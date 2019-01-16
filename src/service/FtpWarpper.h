#ifndef FTPWARPPER_H
#define FTPWARPPER_H

#include <unordered_map>
#include <functional>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include "utils/CodeConverter.h"
#include "service/FtpSession.h"

class FtpInstruction;

class FtpWarpper {
 public:
  FtpWarpper();

  ~FtpWarpper();

  void RegisterCallback();

  int ServiceStart(FtpSession &session);

  void MutiProcesserInit(FtpSession &session);

  int FTPControlHandler(FtpSession &session);

  int FTPProtocolParser(FtpSession &session, char *data);

  void IPC_FTPControlHandler(FtpSession &session,
                             FtpInstruction &__instruction);

  void IPC_FTPTransferHandler(FtpSession &session);

  /* ftp-data transfer processer handler */
  int PASV_FtpTransferStandby(FtpSession &session,
                              FtpInstruction &__instruction);

  int PORT_FtpTransferStandby(FtpSession &session,
                              FtpInstruction &__instruction);

  int WORK_FtpTryContact(FtpSession &session, FtpInstruction &__instruction);

  int WORK_FtpTrySendCommand(FtpSession &session,
                             FtpInstruction &__instruction);

  int WORK_FtpTryFileDownload(FtpSession &session,
                              FtpInstruction &__instruction);

  int WORK_FtpTryFileUpload(FtpSession &session, FtpInstruction &__instruction);

 private:
  typedef std::function<int(FtpSession&, char *)> InstructionHandler;

  int FtpReply(FtpSession &session, int code, std::string content);

  /* ftp command handler */
  int __FTP_CWD(FtpSession &session, char *context);

  int __FTP_ABOR(FtpSession &session, char *context);

  int __FTP_LIST(FtpSession &session, char *context);

  int __FTP_PASS(FtpSession &session, char *context);

  int __FTP_PORT(FtpSession &session, char *context);

  int __FTP_QUIT(FtpSession &session, char *context);

  int __FTP_RETR(FtpSession &session, char *context);

  int __FTP_STOR(FtpSession &session, char *context);

  int __FTP_SYST(FtpSession &session, char *context);

  int __FTP_TYPE(FtpSession &session, char *context);

  int __FTP_USER(FtpSession &session, char *context);

  int __FTP_PASV(FtpSession &session, char *context);

  int __FTP_FEAT(FtpSession &session, char *context);

  int __FTP_REST(FtpSession &session, char *context);

  int __FTP_PWD(FtpSession &session, char *context);

  int __FTP_CDUP(FtpSession &session, char *context);

  /* IPC */
  int IPC_RecvInstruction(int sockfrom, FtpInstruction &instruction);

  int IPC_SendInstruction(int sockto, FtpInstruction &instruction);

  std::unordered_map<std::string, InstructionHandler> handlers_;
};

class FtpInstruction {
 public:

  FtpInstruction& operator=(FtpInstruction& ins) {
    InsLength = ins.InsLength;
    InsType = ins.InsType;
    InsExecOK = ins.InsExecOK;
    memset(InsContent, 0, sizeof(InsContent));
    if (InsLength != 0)
      memcpy(InsContent, ins.InsContent, InsLength);
    else
      strcpy(InsContent, ins.InsContent);

    return (*this);
  }

  void clear() {
    InsLength = 0;
    InsType = 0;
    InsExecOK = false;
    memset(InsContent, 0, sizeof(InsContent));
  }

  int getInsContentLength() {
    return InsLength;
  }

  void setInsContentLength(int __insLength) {
    InsLength = __insLength;
  }

  int getInsType() {
    return InsType;
  }

  void setInsType(int __insType) {
    InsType = __insType;
  }

  bool IsInsExecSuccess() {
    return InsExecOK;
  }

  void setInsExecFlag(bool __insExecOK) {
    InsExecOK = __insExecOK;
  }

  char* getInsContent() {
    return InsContent;
  }

  void setInsContent(const char* __insContent, int ContentLength) {
    memset(InsContent, 0, sizeof(InsContent));
    memcpy(InsContent, __insContent, ContentLength);
  }

 private:
  int InsLength;
  int InsType;
  bool InsExecOK;
  char InsContent[4096];

};

#endif // FTPWARPPER_H
