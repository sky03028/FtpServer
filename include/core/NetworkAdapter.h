/*
 * NetworkAdapter.h
 *
 *  Created on: 2019年1月16日
 *      Author: xueda
 */

#ifndef INCLUDE_CORE_NETWORKADAPTER_H_
#define INCLUDE_CORE_NETWORKADAPTER_H_

#include <memory>

namespace model {

class Session;
class Context;

class NetworkAdapter {
 public:
  NetworkAdapter() = default;
  virtual ~NetworkAdapter() = default;

  virtual int RecvFrom(const std::shared_ptr<Session>& session,
                       Context* context) = 0;
  virtual int SendTo(const std::shared_ptr<Session>& session,
                     Context* context) = 0;
  virtual void Reply(const std::shared_ptr<Session>& session,
                     const std::string& content) = 0;
};

}

#endif /* INCLUDE_CORE_NETWORKADAPTER_H_ */
