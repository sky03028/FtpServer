/*
 * FtpDataTransfer.h
 *
 *  Created on: 2019年1月16日
 *      Author: xueda
 */

#ifndef SRC_FTP_FTPDATATRANSFER_H_
#define SRC_FTP_FTPDATATRANSFER_H_

#include <memory>
#include "core/Transfer.h"

namespace ftp {

class FtpSession;
class FtpContext;

class FtpDataTransfer : public model::Transfer {
 public:
  FtpDataTransfer();
  virtual ~FtpDataTransfer();

  void TransferHandler(const std::shared_ptr<FtpSession> &session);

 private:
  int PasvModeStandby(const std::shared_ptr<FtpSession> &session,
                      FtpContext* context);
  int PortModeStandby(const std::shared_ptr<FtpSession> &session,
                      FtpContext* context);
  int TryContact(const std::shared_ptr<FtpSession> &session,
                 FtpContext* context);
  int TrySendCommand(const std::shared_ptr<FtpSession> &session,
                     FtpContext* context);
  int TryFileDownload(const std::shared_ptr<FtpSession> &session,
                      FtpContext* context);
  int TryFileUpload(const std::shared_ptr<FtpSession> &session,
                    FtpContext* context);

  void ReplyController(const std::shared_ptr<FtpSession> & session,
                       FtpContext* context);
  void ReplyClient(const std::shared_ptr<FtpSession>& session,
                   FtpContext* context);
};

}

#endif /* SRC_FTP_FTPDATATRANSFER_H_ */
