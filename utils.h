#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include <vector>

class Utils
{
public:
    Utils();

    static unsigned int getCurrentTime();

    static void ThreadSleep(unsigned int delay_usec);

    static std::string DeleteSpace(std::string &__before);

    static std::string DeleteCRLF(std::string &__before);

    static std::string GetFilePermission(struct stat &doc);

    static std::string GetFileInfo(struct stat &doc);

    static int GetFileAttribute(std::string &__path, struct stat *__fileStat);

    static std::string GetListString(std::string __currentPath);

    static int GetMaxValue(int *numberList, int listSize);

    static std::string GetLastDirPath(std::string CurrentPath);

    static int CheckSymbolExsit(std::string OriginString, char symbol);

    static std::string GetConvString(const char *from_type, const char *to_type, std::string str);
};

#endif // UTILS_H
