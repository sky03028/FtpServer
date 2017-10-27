#include "utils.h"
#include <sys/time.h>
#if defined(__WIN32__)
#include <windows.h>
#elif defined(__linux__)
#include <unistd.h>
#endif
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/time.h>
#include <string.h>
#include "CodeConverter.h"

Utils::Utils()
{

}


unsigned int Utils::getCurrentTime()
{
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return (tv.tv_sec * 1000 + tv.tv_usec / 1000);
}



void Utils::ThreadSleep(unsigned int delay_usec)
{
#if defined(__WIN32__)
    Sleep(delay_usec);
#elif defined(__linux__)
    usleep(delay_usec * 1000);
#endif
}



std::string Utils::DeleteSpace(std::string &__before)
{
    int begin;

    while( -1 != (begin = __before.find(" ")) )
    {
        __before.replace(begin, 1, "");
    }

    return __before;
}

std::string Utils::DeleteCRLF(std::string &__before)
{
    int begin;

    while( -1 != (begin = __before.find('\r')) || -1 != (begin = __before.find('\n')))
    {
        __before.replace(begin, 1, "");
    }

    return __before;
}


std::string Utils::GetFilePermission(struct stat &doc)
{
    char perms[] = "----------";
    perms[0] = '?';

    mode_t mode = doc.st_mode;

    switch (mode & S_IFMT)
    {
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

    if (mode & S_IRUSR)
    {
        perms[1] = 'r';
    }
    if (mode & S_IWUSR)
    {
        perms[2] = 'w';
    }
    if (mode & S_IXUSR)
    {
        perms[3] = 'x';
    }
    if (mode & S_IRGRP)
    {
        perms[4] = 'r';
    }
    if (mode & S_IWGRP)
    {
        perms[5] = 'w';
    }
    if (mode & S_IXGRP)
    {
        perms[6] = 'x';
    }
    if (mode & S_IROTH)
    {
        perms[7] = 'r';
    }
    if (mode & S_IWOTH)
    {
        perms[8] = 'w';
    }
    if (mode & S_IXOTH)
    {
        perms[9] = 'x';
    }
    if (mode & S_ISUID)
    {
        perms[3] = (perms[3] == 'x') ? 's' : 'S';
    }
    if (mode & S_ISGID)
    {
        perms[6] = (perms[6] == 'x') ? 's' : 'S';
    }
    if (mode & S_ISVTX)
    {
        perms[9] = (perms[9] == 'x') ? 't' : 'T';
    }

    return std::string(perms);
}


std::string Utils::GetFileInfo(struct stat &doc)
{
    std::string content;

    char datebuf[64] = {0};
    const char *p_date_format = "%b %e %H:%M";
    struct timeval tv;

    gettimeofday(&tv, NULL);
    time_t local_time = tv.tv_sec;
    if (doc.st_mtime > local_time || (local_time - doc.st_mtime) > 60*60*24*182)
    {
        p_date_format = "%b %e  %Y";
    }

    struct tm* p_tm = localtime(&local_time);
    strftime(datebuf, sizeof(datebuf), p_date_format, p_tm);

    content.clear();

    content += std::to_string(doc.st_size) + " ";
    content += std::string(datebuf) + " ";

    return content;
}



int Utils::GetFileAttribute(std::string &__path, struct stat *__fileStat)
{
    char out[CONVEROUTLEN];

    memset(out, 0, sizeof(out));
    CodeConverter cc = CodeConverter("gb2312", "utf-8");
    cc.convert((char *)__path.c_str(), strlen(__path.c_str()),out,CONVEROUTLEN);

    __path = std::string(out);

    std::cout << "filePath : " << __path << std::endl;

    if(-1 == stat(__path.c_str(), __fileStat))
        return -1;

    return 0;
}




std::string Utils::GetListString(std::string __currentPath)
{
    struct stat doc;
    struct dirent * pDocument;

    std::string ReplyContent;
    std::string content;

    ReplyContent.clear();
    content.clear();

    std::string docPath;

    DIR *dir = opendir(__currentPath.c_str());
    while((pDocument = readdir(dir)) != NULL)
    {
        if(strcmp(pDocument->d_name, ".") == 0 && strcmp(pDocument->d_name, "..") == 0)
            continue;

        docPath = __currentPath + std::string(pDocument->d_name);
        //if(lstat(pDocument->d_name, &doc) == -1)
        //    break;

        if(lstat(docPath.c_str(), &doc) == -1)
            break;

        content = GetFilePermission(doc) + " 1 root  root ";
        content += GetFileInfo(doc);
        content += std::string(pDocument->d_name) + "\r\n";
        ReplyContent += content;
    }

    closedir(dir);

    return ReplyContent;
}


int Utils::GetMaxValue(int *numberList, int listSize)
{
    int maxValue = 0;

    for(int index = 0; index < listSize; index++)
    {
         if(maxValue < numberList[index])
         {
             maxValue = numberList[index];
         }
    }

    return maxValue;
}


/* 获取上一层目录路径 */
std::string Utils::GetLastDirPath(std::string CurrentPath)
{
    int pos;

     std::vector<int> symbolList;

     symbolList.clear();

    if(CurrentPath != "/" && CurrentPath != "./" && CurrentPath != "")
    {
        for(int index = 0; index < CurrentPath.size(); index++)
        {
            pos = CurrentPath.find('/', index);
            if(pos == std::string::npos)
                continue;
            symbolList.push_back(pos);
            index = pos;
        }

        if(CurrentPath.at(CurrentPath.size() - 1) != '/')
        {
            CurrentPath.replace(symbolList.at(symbolList.size() - 1), CurrentPath.size() - 1, "");
        }
        else{
            CurrentPath.replace(symbolList.at(symbolList.size() - 2), CurrentPath.size() - 1, "");
        }
    }

    return CurrentPath;
}


int Utils::CheckSymbolExsit(std::string OriginString, char symbol)
{
    for(int index = 0; index < OriginString.size(); index++)
    {
        if(OriginString.at(index) == symbol)
            return 0;
    }

    return -1;
}


std::string Utils::GetConvString(const char *from_type, const char *to_type, std::string str)
{
    char out[CONVEROUTLEN];

    memset(out, 0, sizeof(out));

    CodeConverter cc = CodeConverter(from_type, to_type);
    cc.convert((char *)str.c_str(), strlen(str.c_str()),out,CONVEROUTLEN);

    str = std::string(out);

    return str;
}


