/*
 * FtpController.h
 *
 *  Created on: 2019年1月16日
 *      Author: xueda
 */

#ifndef SRC_MODEL_FTPCONTROLLER_H_
#define SRC_MODEL_FTPCONTROLLER_H_

class Context;

#include "core/Controller.h"
#include "core/Session.h"

class FtpController : public Controller {
 public:
  FtpController();
  virtual ~FtpController();

  int SendInstuction(const std::shared_ptr<FtpSession>& session, Context* context) {
    return SendTo(session, context);
  }

 private:
  /* ftp command handler */
  int ftp_cwd(const std::shared_ptr<Session> &session, Context *context);
  int ftp_abor(const std::shared_ptr<Session> &session, Context *context);
  int ftp_list(const std::shared_ptr<Session> &session, Context *context);
  int ftp_pass(const std::shared_ptr<Session> &session, Context *context);
  int ftp_port(const std::shared_ptr<Session> &session, Context *context);
  int ftp_quit(const std::shared_ptr<Session> &session, Context *context);
  int ftp_retr(const std::shared_ptr<Session> &session, Context *context);
  int ftp_stor(const std::shared_ptr<Session> &session, Context *context);
  int ftp_syst(const std::shared_ptr<Session> &session, Context *context);
  int ftp_type(const std::shared_ptr<Session> &session, Context *context);
  int ftp_user(const std::shared_ptr<Session> &session, Context *context);
  int ftp_pasv(const std::shared_ptr<Session> &session, Context *context);
  int ftp_feat(const std::shared_ptr<Session> &session, Context *context);
  int ftp_rest(const std::shared_ptr<Session> &session, Context *context);
  int ftp_pwd(const std::shared_ptr<Session> &session, Context *context);
  int ftp_cdup(const std::shared_ptr<Session> &session, Context *context);
};

#endif /* SRC_MODEL_FTPCONTROLLER_H_ */
