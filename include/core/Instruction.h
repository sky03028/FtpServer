/*
 * Instruction.h
 *
 *  Created on: 2019年1月16日
 *      Author: xueda
 */

#ifndef INCLUDE_CORE_INSTRUCTION_H_
#define INCLUDE_CORE_INSTRUCTION_H_

class Instruction {
 public:
  Instruction() = default;
  virtual ~Instruction() = default;

  virtual const std::string SerializeAsString() = 0;
  virtual void* GetBinaryData() = 0;
};

#endif /* INCLUDE_CORE_INSTRUCTION_H_ */
