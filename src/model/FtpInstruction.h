/*
 * FtpInstructions.h
 *
 *  Created on: 2019年1月16日
 *      Author: xueda
 */

#ifndef SRC_MODEL_FTPINSTRUCTION_H_BAK_
#define SRC_MODEL_FTPINSTRUCTION_H_BAK_

#include "core/Instruction.h"

class FtpInstruction : public Instruction {
 public:
  FtpInstruction() {
    Clear();
  }
  virtual ~FtpInstruction() = default;

  FtpInstruction& operator=(FtpInstruction& ins) {
    length_ = ins.length_;
    type_ = ins.type_;
    has_success_ = ins.has_success_;
    memset(content_, 0, sizeof(content_));
    if (length_ != 0) {
      memcpy(content_, ins.content_, length_);
    } else {
      strcpy((char*) content_, (const char*) ins.content_);
    }

    return (*this);
  }

  int length() const {
    return length_;
  }
  void set_length(int length) {
    length_ = length;
  }

  int type() const {
    return type_;
  }
  void set_type(int type) {
    type_ = type;
  }

  bool has_success() {
    return has_success_;
  }
  void set_success_flag(bool flag) {
    has_success_ = flag;
  }

  void set_content(const char* content, int length) {
    memset(content_, 0, sizeof(content_));
    memcpy(content_, content, length);
  }

  const std::string SerializeAsString() {
    return std::string(content_);
  }

  void* GetBinaryData() {
    return content_;
  }

  void Clear() {
    length_ = 0;
    type_ = 0;
    has_success_ = false;
    memset(content_, 0, sizeof(content_));
  }

 private:
  int length_;
  int type_;
  bool has_success_;
  unsigned char content_[4096];
};

#endif /* SRC_MODEL_FTPINSTRUCTION_H_BAK_ */
