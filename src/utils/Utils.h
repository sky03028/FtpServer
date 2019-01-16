#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include <vector>

class Utils {
 public:
  static unsigned long getCurrentTime();

  static void ThreadSleep(unsigned int delay_usec);

  static const std::string DeleteSpace(const std::string &src);

  static const std::string DeleteCRLF(const std::string &src);

  static const std::string GetFilePermission(const struct stat &doc);

  static const std::string GetFileInfo(const struct stat &doc);

  static const int GetFileAttribute(const std::string &, struct stat *);

  static const std::string GetListString(const std::string& path);

  static const int GetMaxValue(const int *numberList, const int listSize);

  static const std::string GetLastDirPath(const std::string& path);

  static const int CheckSymbolExsit(const std::string& src, const char symbol);

  static const std::string GetConvString(const char *from_type,
                                         const char *to_type,
                                         const std::string& contents);
};

#endif // UTILS_H
