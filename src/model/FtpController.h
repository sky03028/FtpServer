/*
 * FtpController.h
 *
 *  Created on: 2019年1月16日
 *      Author: xueda
 */

#ifndef SRC_MODEL_FTPCONTROLLER_H_
#define SRC_MODEL_FTPCONTROLLER_H_

#include <memory>
#include <unordered_map>

#include "core/Controller.h"

class DefaultContext;
class DefaultSession;

class FtpController : public Controller {
 public:
  FtpController();
  virtual ~FtpController();

  bool Init();
  int ControlHandler(const std::shared_ptr<DefaultSession> &session);

 private:
  int CommandHandler(const std::shared_ptr<DefaultSession> &session,
                     DefaultContext* context);
  void ReplyHandler(const std::shared_ptr<DefaultSession> &session,
                    DefaultContext* context);

  int ReplyClient(const std::shared_ptr<DefaultSession> &session, int code,
                  const std::string& content);
  int NotifyTransfer(const std::shared_ptr<DefaultSession>& session,
                     DefaultContext* context);

  /* ftp command handler */
  int ftp_cwd(const std::shared_ptr<DefaultSession> &session,
              DefaultContext *context);
  int ftp_abor(const std::shared_ptr<DefaultSession> &session,
               DefaultContext *context);
  int ftp_list(const std::shared_ptr<DefaultSession> &session,
               DefaultContext *context);
  int ftp_pass(const std::shared_ptr<DefaultSession> &session,
               DefaultContext *context);
  int ftp_port(const std::shared_ptr<DefaultSession> &session,
               DefaultContext *context);
  int ftp_quit(const std::shared_ptr<DefaultSession> &session,
               DefaultContext *context);
  int ftp_retr(const std::shared_ptr<DefaultSession> &session,
               DefaultContext *context);
  int ftp_stor(const std::shared_ptr<DefaultSession> &session,
               DefaultContext *context);
  int ftp_syst(const std::shared_ptr<DefaultSession> &session,
               DefaultContext *context);
  int ftp_type(const std::shared_ptr<DefaultSession> &session,
               DefaultContext *context);
  int ftp_user(const std::shared_ptr<DefaultSession> &session,
               DefaultContext *context);
  int ftp_pasv(const std::shared_ptr<DefaultSession> &session,
               DefaultContext *context);
  int ftp_feat(const std::shared_ptr<DefaultSession> &session,
               DefaultContext *context);
  int ftp_rest(const std::shared_ptr<DefaultSession> &session,
               DefaultContext *context);
  int ftp_pwd(const std::shared_ptr<DefaultSession> &session,
              DefaultContext *context);
  int ftp_cdup(const std::shared_ptr<DefaultSession> &session,
               DefaultContext *context);

  typedef std::function<
      int(const std::shared_ptr<DefaultSession>&, DefaultContext*)> CmdHandler;
  std::unordered_map<std::string, CmdHandler> handlers_;
};

#endif /* SRC_MODEL_FTPCONTROLLER_H_ */
