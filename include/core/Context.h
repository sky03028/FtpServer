/*
 * Context.h
 *
 *  Created on: 2019年1月16日
 *      Author: xueda
 */

#ifndef INCLUDE_CORE_CONTEXT_H_
#define INCLUDE_CORE_CONTEXT_H_

namespace model {

class ContentType {
 public:
  static const int kUndefined = 0;
  static const int kString = 1;
  static const int kJson = 2;
  static const int kBinary = 3;
};

class Context {
 public:
  Context()
      : content_type_(ContentType::kUndefined) {
  }
  virtual ~Context() = default;

  void set_content_type(const int type) {
    content_type_ = type;
  }
  int content_type() const {
    return content_type_;
  }

  void set_content(const std::string& content) {
    content_ = content;
  }
  const std::string& content() const {
    return content_;
  }

 private:
  int content_type_;
  std::string content_;
};

}

#endif /* INCLUDE_CORE_CONTEXT_H_ */
