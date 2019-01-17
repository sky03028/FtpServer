/*
 * FtpMaster.h
 *
 *  Created on: 2019年1月17日
 *      Author: xueda
 */

#ifndef SRC_FTP_FTPMASTER_H_
#define SRC_FTP_FTPMASTER_H_

#include <memory>

#include "../ftp/FtpController.h"
#include "../ftp/FtpDataTransfer.h"

class Session;
class Context;
class FtpSession;

class FtpMaster : public FtpController, public FtpDataTransfer {
 public:
  FtpMaster() = default;
  virtual ~FtpMaster() = default;

  int Setup(std::shared_ptr<FtpSession> &session);

  virtual int RecvFrom(const std::shared_ptr<Session>& session,
                       Context* context);
  virtual int SendTo(const std::shared_ptr<Session> & session,
                     Context* context);
  virtual void Reply(const std::shared_ptr<Session>& session,
                     const std::string& content);
};

#endif /* SRC_FTP_FTPMASTER_H_ */
