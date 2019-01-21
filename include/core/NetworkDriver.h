/*
 * NetworkDriver.h
 *
 *  Created on: 2019年1月17日
 *      Author: xueda
 */

#ifndef INCLUDE_CORE_NETWORKDRIVER_H_
#define INCLUDE_CORE_NETWORKDRIVER_H_

namespace model {

class Context;

class ConnectionType {
 public:
  static const int kServer = 1;
  static const int kClient = 2;
};

class NetworkDriver {
 public:
  NetworkDriver() = default;
  virtual ~NetworkDriver() = default;

  virtual int SendTo(Context* context) = 0;

  virtual int RecvFrom(Context* context) = 0;

  virtual bool Create(const int type) = 0;

  virtual bool Contact() = 0;

  virtual void Destory() = 0;
};

}

#endif /* INCLUDE_CORE_NETWORKDRIVER_H_ */
