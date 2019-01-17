/*
 * Transfer.h
 *
 *  Created on: 2019年1月16日
 *      Author: xueda
 */

#ifndef INCLUDE_CORE_TRANSFER_H_
#define INCLUDE_CORE_TRANSFER_H_

#include "NetAdapter.h"

class Transfer : public NetAdapter {
 public:
  Transfer() = default;
  virtual ~Transfer() = default;
};

#endif /* INCLUDE_CORE_TRANSFER_H_ */
