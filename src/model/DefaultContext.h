/*
 * InstructionContext.h
 *
 *  Created on: 2019年1月16日
 *      Author: xueda
 */

#ifndef SRC_MODEL_DEFAULTCONTEXT_H_
#define SRC_MODEL_DEFAULTCONTEXT_H_

#include "core/Instruction.h"

class Destination {
 public:
  static const int kDestUndefined = 0;
  static const int kDestClient = 1;
  static const int kDestController = 2;
  static const int kDestTransfer = 3;
};

class DefaultContext : public Context {
 public:
  DefaultContext();
  virtual ~DefaultContext();

  void set_destination(const int destination) {
    destination_ = destination;
  }
  int destination() const {
    return destination_;
  }

 private:
  int destination_;
};

#endif /* SRC_MODEL_DEFAULTCONTEXT_H_ */
