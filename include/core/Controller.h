/*
 * Controller.h
 *
 *  Created on: 2019年1月16日
 *      Author: xueda
 */

#ifndef INCLUDE_CORE_CONTROLLER_H_
#define INCLUDE_CORE_CONTROLLER_H_

#include "NetworkAdapter.h"

namespace model {

class Session;

class Controller : public NetworkAdapter {
 public:
  Controller() = default;
  virtual ~Controller() = default;
};

}

#endif /* INCLUDE_CORE_CONTROLLER_H_ */
