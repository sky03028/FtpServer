#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include <vector>

#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/time.h>
#include <string.h>
#include "CodeConverter.h"
#include <thread>

class Utils {
 public:
  static unsigned long getCurrentTime() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000 + tv.tv_usec / 1000);
  }

  static void ThreadSleep(unsigned int interval) {
    std::this_thread::sleep_for(std::chrono::milliseconds(interval));
  }

  static const std::string DeleteSpace(const std::string &src) {
    int begin;

    std::string dst(src);
    while (-1 != (begin = src.find(" "))) {
      dst.replace(begin, 1, "");
    }

    return src;
  }

  static const std::string DeleteCRLF(const std::string &src) {
    int begin;

    std::string dst(src);
    while (-1 != (begin = src.find('\r')) || -1 != (begin = src.find('\n'))) {
      dst.replace(begin, 1, "");
    }

    return src;
  }

  static const std::string GetFilePermission(const struct stat& doc) {
    char perms[] = "----------";
    perms[0] = '?';

    mode_t mode = doc.st_mode;

    switch (mode & S_IFMT) {
      case S_IFREG:
        perms[0] = '-';
        break;
      case S_IFDIR:
        perms[0] = 'd';
        break;
      case S_IFLNK:
        perms[0] = 'l';
        break;
      case S_IFIFO:
        perms[0] = 'p';
        break;
      case S_IFSOCK:
        perms[0] = 's';
        break;
      case S_IFCHR:
        perms[0] = 'c';
        break;
      case S_IFBLK:
        perms[0] = 'b';
        break;
    }

    if (mode & S_IRUSR) {
      perms[1] = 'r';
    }
    if (mode & S_IWUSR) {
      perms[2] = 'w';
    }
    if (mode & S_IXUSR) {
      perms[3] = 'x';
    }
    if (mode & S_IRGRP) {
      perms[4] = 'r';
    }
    if (mode & S_IWGRP) {
      perms[5] = 'w';
    }
    if (mode & S_IXGRP) {
      perms[6] = 'x';
    }
    if (mode & S_IROTH) {
      perms[7] = 'r';
    }
    if (mode & S_IWOTH) {
      perms[8] = 'w';
    }
    if (mode & S_IXOTH) {
      perms[9] = 'x';
    }
    if (mode & S_ISUID) {
      perms[3] = (perms[3] == 'x') ? 's' : 'S';
    }
    if (mode & S_ISGID) {
      perms[6] = (perms[6] == 'x') ? 's' : 'S';
    }
    if (mode & S_ISVTX) {
      perms[9] = (perms[9] == 'x') ? 't' : 'T';
    }

    return std::string(perms);
  }

  static const std::string GetFileInfo(const struct stat &doc) {
    std::string content;

    char datebuf[64] = { 0 };
    const char *p_date_format = "%b %e %H:%M";
    struct timeval tv;

    gettimeofday(&tv, NULL);
    time_t local_time = tv.tv_sec;
    if (doc.st_mtime > local_time
        || (local_time - doc.st_mtime) > 60 * 60 * 24 * 182) {
      p_date_format = "%b %e  %Y";
    }

    struct tm* p_tm = localtime(&local_time);
    strftime(datebuf, sizeof(datebuf), p_date_format, p_tm);

    content.clear();

    content += std::to_string(doc.st_size) + " ";
    content += std::string(datebuf) + " ";

    return content;
  }

  static const int GetFileAttribute(const std::string &path,
                                    struct stat *fileStat) {
    char out[CodeConverter::kMaxConvertSize];

    memset(out, 0, sizeof(out));
    CodeConverter cc = CodeConverter("gb2312", "utf-8");
    cc.convert((char *) path.c_str(), strlen(path.c_str()), out,
               CodeConverter::kMaxConvertSize);

    std::cout << "filePath : " << out << std::endl;

    if (-1 == stat(out, fileStat)) {
      return -1;
    }

    return 0;
  }

  static const std::string GetListString(const std::string& path) {
    struct stat doc;
    struct dirent * pDocument;

    std::string reply_content;
    std::string content;

    reply_content.clear();
    content.clear();

    std::string docPath;

    DIR *dir = opendir(path.c_str());
    while ((pDocument = readdir(dir)) != NULL) {
      if (strcmp(pDocument->d_name, ".") == 0
          && strcmp(pDocument->d_name, "..") == 0)
        continue;

      docPath = path + std::string(pDocument->d_name);
      //if(lstat(pDocument->d_name, &doc) == -1)
      //    break;

      if (lstat(docPath.c_str(), &doc) == -1)
        break;

      content = GetFilePermission(doc) + " 1 root  root ";
      content += GetFileInfo(doc);
      content += std::string(pDocument->d_name) + "\r\n";
      reply_content += content;
    }

    closedir(dir);

    return reply_content;
  }

  static const int GetMaxValue(const int *numberList, const int listSize) {
    int maxValue = 0;
    for (int index = 0; index < listSize; index++) {
      if (maxValue < numberList[index]) {
        maxValue = numberList[index];
      }
    }

    return maxValue;
  }

  /* 获取上一层目录路径 */
  static const std::string GetLastDirPath(const std::string& src_path) {
    std::vector<int> symbolList;

    symbolList.clear();

    std::string dst_path(src_path);
    if (dst_path != "/" && dst_path != "./" && dst_path != "") {
      for (unsigned int index = 0; index < dst_path.size(); index++) {
        const auto pos = dst_path.find('/', index);
        if (pos == std::string::npos)
          continue;
        symbolList.push_back(pos);
        index = pos;
      }

      if (dst_path.at(dst_path.size() - 1) != '/') {
        dst_path.replace(symbolList.at(symbolList.size() - 1),
                         dst_path.size() - 1, "");
      } else {
        dst_path.replace(symbolList.at(symbolList.size() - 2),
                         dst_path.size() - 1, "");
      }
    }

    return dst_path;
  }

  static const int CheckSymbolExsit(const std::string& origin, char symbol) {
    for (unsigned int index = 0; index < origin.size(); index++) {
      if (origin.at(index) == symbol)
        return 0;
    }

    return -1;
  }

  static const std::string GetConvString(const char *from_type,
                                         const char *to_type,
                                         const std::string& str) {
    char out[CodeConverter::kMaxConvertSize];

    memset(out, 0, sizeof(out));
    CodeConverter cc = CodeConverter(from_type, to_type);
    cc.convert((char *) str.c_str(), strlen(str.c_str()), out,
               CodeConverter::kMaxConvertSize);

    return std::string(out);
  }
};

#endif // UTILS_H
