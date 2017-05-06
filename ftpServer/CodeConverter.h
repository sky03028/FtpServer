#ifndef CODECONVERTER_H
#define CODECONVERTER_H

#include <iconv.h>
#include <iostream>

#define CONVEROUTLEN 255

class CodeConverter
{
    public:
    // 构造
    CodeConverter(const char *from_charset,const char *to_charset) {
    cd = iconv_open(to_charset,from_charset);
    }

    // 析构
    ~CodeConverter() {
    iconv_close(cd);
    }

    // 转换输出
    int convert(char *inbuf,int inlen,char *outbuf,int outlen) {
    char **pin = &inbuf;
    char **pout = &outbuf;

    memset(outbuf,0,outlen);
    return iconv(cd,pin,(size_t *)&inlen,pout,(size_t *)&outlen);
    }

private:
    iconv_t cd;
};

#endif // CODECONVERTER_H
