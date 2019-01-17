/*
 * JsonParser.h
 *
 *  Created on: 2019年1月17日
 *      Author: xueda
 */

#ifndef SRC_UTILS_JSONPARSER_H_
#define SRC_UTILS_JSONPARSER_H_

#include <cjson/cJSON.h>
#include <string>
#include <iostream>

class JsonParser {
 public:
  JsonParser(const std::string& content)
      : root_(nullptr),
        item_(nullptr),
        content_(content) {
    root_ = cJSON_Parse(content.c_str());
    if (root_ == nullptr) {
      std::cout << "Parse json failed!" << std::endl;
    }
  }

  virtual ~JsonParser() {
    if (root_ != nullptr) {
      cJSON_Delete(root_);
    }
  }

  const std::string GetString(const std::string& key) {
    if (root_ == nullptr) {
      return "";
    }
    item_ = cJSON_GetObjectItem(item_, key.c_str());
    if (item_ != nullptr) {
      return std::string(item_->valuestring);
    }
    return "";
  }

  int GetInt(const std::string& key) {
    if (root_ == nullptr) {
      return 0;
    }
    item_ = cJSON_GetObjectItem(item_, key.c_str());
    if (item_ != nullptr) {
      return item_->valueint;
    }
    return 0;
  }

  bool GetBool(const std::string& key) {
    if (root_ == nullptr) {
      return false;
    }
    item_ = cJSON_GetObjectItem(item_, key.c_str());
    if (item_ != nullptr) {
      return (bool) item_->valueint;
    }
    return false;
  }

 private:
  cJSON* root_;
  cJSON* item_;
  std::string content_;
};

#endif /* SRC_UTILS_JSONPARSER_H_ */
