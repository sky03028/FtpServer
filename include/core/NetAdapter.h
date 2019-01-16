/*
 * NetworkAdapter.h
 *
 *  Created on: 2019年1月16日
 *      Author: xueda
 */

#ifndef INCLUDE_CORE_NETADAPTER_H_
#define INCLUDE_CORE_NETADAPTER_H_

#include <memory>

class Session;
class Context;

class NetAdapter {
 public:
  NetAdapter() = default;
  virtual ~NetAdapter() = default;

  virtual std::shared_ptr<Session> CreateSession(const std::string& type) = 0;
  virtual void RecvFrom(const std::shared_ptr<Session>& session,
                        Context* context) = 0;
  virtual void SendTo(const std::shared_ptr<Session>& session,
                      Context* context) = 0;
  virtual void Reply(const std::shared_ptr<Session>& session, const int code,
                     std::string& content) = 0;
};

#endif /* INCLUDE_CORE_NETADAPTER_H_ */
