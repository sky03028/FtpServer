/*
 * FtpDataTransfer.h
 *
 *  Created on: 2019年1月16日
 *      Author: xueda
 */

#ifndef SRC_MODEL_FTPDATATRANSFER_H_
#define SRC_MODEL_FTPDATATRANSFER_H_


#include <memory>
#include "core/Transfer.h"

class DefaultSession;
class DefaultContext;

class FtpDataTransfer : public Transfer {
 public:
  FtpDataTransfer();
  virtual ~FtpDataTransfer();

  void TransferHandler(const std::shared_ptr<DefaultSession> &session);

 private:
  int PasvModeStandby(const std::shared_ptr<DefaultSession> &session,
                      DefaultContext* context);
  int PortModeStandby(const std::shared_ptr<DefaultSession> &session,
                      DefaultContext* context);
  int TryContact(const std::shared_ptr<DefaultSession> &session,
                 DefaultContext* context);
  int TrySendCommand(const std::shared_ptr<DefaultSession> &session,
                     DefaultContext* context);
  int TryFileDownload(const std::shared_ptr<DefaultSession> &session,
                      DefaultContext* context);
  int TryFileUpload(const std::shared_ptr<DefaultSession> &session,
                    DefaultContext* context);

  void ReplyController(const std::shared_ptr<DefaultSession> & session,
                       DefaultContext* context);
  void ReplyClient(const std::shared_ptr<DefaultSession>& session,
                   DefaultContext* context);
};

#endif /* SRC_MODEL_FTPDATATRANSFER_H_ */
