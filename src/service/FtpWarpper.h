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
#include "model/FtpSession.h"

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

  void IPC_FTPControlHandler(FtpSession &session, FtpInstruction &instruction);

  void IPC_FTPTransferHandler(FtpSession &session);

  /* ftp-data transfer processer handler */
  int PASV_FtpTransferStandby(FtpSession &session, FtpInstruction &instruction);
  int PORT_FtpTransferStandby(FtpSession &session, FtpInstruction &instruction);

 private:
  int TryContact(FtpSession &session, FtpInstruction &instruction);
  int TrySendCommand(FtpSession &session, FtpInstruction &instruction);
  int TryFileDownload(FtpSession &session, FtpInstruction &instruction);
  int TryFileUpload(FtpSession &session, FtpInstruction &instruction);

  typedef std::function<int(FtpSession&, char *)> InstructionHandler;

  int FtpReply(FtpSession &session, int code, std::string content);

  /* ftp command handler */
  int ftp_cwd(FtpSession &session, char *context);
  int ftp_abor(FtpSession &session, char *context);
  int ftp_list(FtpSession &session, char *context);
  int ftp_pass(FtpSession &session, char *context);
  int ftp_port(FtpSession &session, char *context);
  int ftp_quit(FtpSession &session, char *context);
  int ftp_retr(FtpSession &session, char *context);
  int ftp_stor(FtpSession &session, char *context);
  int ftp_syst(FtpSession &session, char *context);
  int ftp_type(FtpSession &session, char *context);
  int ftp_user(FtpSession &session, char *context);
  int ftp_pasv(FtpSession &session, char *context);
  int ftp_feat(FtpSession &session, char *context);
  int ftp_rest(FtpSession &session, char *context);
  int ftp_pwd(FtpSession &session, char *context);
  int ftp_cdup(FtpSession &session, char *context);

  /* IPC */
  int RecvInstruction(int sockfrom, FtpInstruction &instruction);
  int SendInstruction(int sockto, FtpInstruction &instruction);

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
