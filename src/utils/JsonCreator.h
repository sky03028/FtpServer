/*
 * JsonCreator.h
 *
 *  Created on: 2019年1月17日
 *      Author: xueda
 */

#ifndef SRC_UTILS_JSONCREATOR_H_
#define SRC_UTILS_JSONCREATOR_H_

#include <assert.h>
#include <cjson/cJSON.h>
#include <string>

class JsonCreator {
 public:
  JsonCreator()
      : root_(nullptr),
        item_(nullptr) {
    root_ = cJSON_CreateObject();
    assert(root_ != nullptr);
  }
  virtual ~JsonCreator() {
    if (root_ != nullptr) {
      cJSON_Delete(root_);
    }
  }

  void SetString(const std::string& key, const std::string& value) {
    cJSON_AddItemToObject(root_, key.c_str(),
                          cJSON_CreateString(value.c_str()));
  }

  void SetInt(const std::string& key, const int value) {
    cJSON_AddItemToObject(root_, key.c_str(), cJSON_CreateNumber(value));
  }

  void SetBool(const std::string& key, const bool value) {
    cJSON_AddItemToObject(root_, key.c_str(), cJSON_CreateBool(value));
  }

  const std::string SerializeAsString() {
    return std::string(cJSON_Print(root_));
  }

 private:
  cJSON* root_;
  cJSON* item_;
};

#endif /* SRC_UTILS_JSONCREATOR_H_ */
