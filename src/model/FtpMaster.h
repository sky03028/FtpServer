/*
 * FtpMaster.h
 *
 *  Created on: 2019年1月17日
 *      Author: xueda
 */

#ifndef SRC_MODEL_FTPMASTER_H_
#define SRC_MODEL_FTPMASTER_H_

#include <memory>

#include "FtpController.h"
#include "FtpDataTransfer.h"

class Session;
class Context;
class DefaultSession;

class FtpMaster : public FtpController, public FtpDataTransfer {
 public:
  FtpMaster() = default;
  virtual ~FtpMaster() = default;

  int Setup(std::shared_ptr<DefaultSession> &session);

  virtual int RecvFrom(const std::shared_ptr<Session>& session,
                       Context* context);
  virtual void SendTo(const std::shared_ptr<Session>& session,
                      Context* context);
  virtual void Reply(const std::shared_ptr<Session>& session,
                     const std::string& content);
};

#endif /* SRC_MODEL_FTPMASTER_H_ */
