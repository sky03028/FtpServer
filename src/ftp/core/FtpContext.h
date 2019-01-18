/*
 * InstructionContext.h
 *
 *  Created on: 2019年1月16日
 *      Author: xueda
 */

#ifndef SRC_FTP_CORE_FTPCONTEXT_H_
#define SRC_FTP_CORE_FTPCONTEXT_H_

#include <assert.h>
#include <string.h>
#include <mutex>
#include <memory>

#include "core/Context.h"

namespace ftp {

enum TransferMode {
  PASV_MODE_ENABLE = 1,
  PORT_MODE_ENABLE = 2,
};

enum TransferCmdType {
  TRANSFER_PASV_STANDBY_REQ = 1,
  TRANSFER_PASV_STANDBY_RES,

  TRANSFER_PORT_STANDBY_REQ,
  TRANSFER_PORT_STANDBY_RES,

  TRANSFER_TRY_CONNNECT_REQ,
  TRANSFER_TRY_CONNNECT_RES,

  TRANSFER_SENDCOMMAND_REQ,
  TRANSFER_SENDCOMMAND_RES,

  TRANSFER_FILEUPLOAD_REQ,
  TRANSFER_FILEUPLOAD_RES,

  TRANSFER_FILEDOWNLOAD_REQ,
  TRANSFER_FILEDOWNLOAD_RES,

  TRANSFER_ABORT_REQ,
  TRANSFER_ABORT_RES,
};

class Destination {
 public:
  static const int kDestUndefined = 0;
  static const int kDestClient = 1;
  static const int kDestController = 2;
  static const int kDestTransfer = 3;
};

class Source {
 public:
  static const int kSrcUndefined = 0;
  static const int kSrcClient = 1;
  static const int kSrcController = 2;
  static const int kSrcTransfer = 3;
};

class BinaryObject {
 public:
  BinaryObject()
      : length_(0),
        capacity_(0),
        data_(nullptr) {
  }
  BinaryObject(void* data, const int length) {
    assert(data != nullptr && length >= 0);
    data_ = (unsigned char*) data;
    length_ = length;
    capacity_ = length;
  }

  virtual ~BinaryObject() {
    if (data_ != nullptr) {
      delete data_;
      length_ = 0;
      capacity_ = 0;
    }
  }

  void CopyFrom(void* src, const int length) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (capacity_ - length_ < length) {
      Expand(length);
    }
    memcpy(data_ + length_, src, length);
    length_ = length;
  }
  void CopyTo(void *dst, const int length) {
    std::lock_guard<std::mutex> lock(mutex_);
    memcpy(dst, data_, length);
  }

  void* data() const {
    return data_;
  }
  int length() const {
    return length_;
  }

  BinaryObject& operator=(const BinaryObject& object) = delete;
  BinaryObject& operator()(const BinaryObject& object) = delete;

 private:
  void Expand(const int length) {
    if (length_ + length > capacity_) {
      capacity_ += kBlockSize;
      unsigned char* temp_data = new unsigned char[capacity_];
      memset(temp_data, 0, capacity_);
      if (data_ != nullptr) {
        memcpy(temp_data, data_, length_);
        delete data_;
      }
      data_ = temp_data;
    }
  }

  static const int kBlockSize = 4096;

  int length_;
  int capacity_;
  unsigned char* data_;
  std::mutex mutex_;
};

class FtpContext : public model::Context {
 public:
  FtpContext()
      : source_(Source::kSrcUndefined),
        destination_(Destination::kDestUndefined) {
    object_ = std::make_shared<BinaryObject>();
  }
  virtual ~FtpContext() = default;

  void set_source(const int source) {
    source_ = source;
  }
  int source() const {
    return source_;
  }

  void set_destination(const int destination) {
    destination_ = destination;
  }
  int destination() const {
    return destination_;
  }

  void set_object(const std::shared_ptr<BinaryObject>& object) {
    object_ = object;
  }
  const std::shared_ptr<BinaryObject>& object() const {
    return object_;
  }

 private:
  int source_;
  int destination_;
  std::shared_ptr<BinaryObject> object_;
};

}

#endif /* SRC_FTP_CORE_FTPCONTEXT_H_ */
