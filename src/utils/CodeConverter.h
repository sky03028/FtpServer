#ifndef CODECONVERTER_H
#define CODECONVERTER_H

#include <iconv.h>
#include <iostream>

class CodeConverter {
 public:
  static const int kMaxConvertSize = 4096;

  CodeConverter(const char *from_charset, const char *to_charset) {
    cd = iconv_open(to_charset, from_charset);
  }
  virtual ~CodeConverter() {
    iconv_close(cd);
  }

  const std::string Make(const std::string& src) {
    char* inbuf = (char *) src.c_str();
    char outbuf[kMaxConvertSize];
    int inlen = src.length();
    int outlen = sizeof(outbuf);
    memset(outbuf, 0, sizeof(outbuf));

    char *poutbuf = outbuf;

    char **pin = &inbuf;
    char **pout = &poutbuf;
    iconv(cd, pin, (size_t *) &inlen, pout, (size_t *) &outlen);
    return std::string(outbuf);
  }

 private:
  iconv_t cd;
};

#endif // CODECONVERTER_H
