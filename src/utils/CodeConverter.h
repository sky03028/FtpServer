#ifndef CODECONVERTER_H
#define CODECONVERTER_H

#include <iconv.h>
#include <iostream>

class CodeConverter {
 public:
  static const int kMaxConvertSize = 255;

  CodeConverter(const char *from_charset, const char *to_charset) {
    cd = iconv_open(to_charset, from_charset);
  }
  ~CodeConverter() {
    iconv_close(cd);
  }

  int convert(char *inbuf, int inlen, char *outbuf, int outlen) {
    char **pin = &inbuf;
    char **pout = &outbuf;

    memset(outbuf, 0, outlen);
    return iconv(cd, pin, (size_t *) &inlen, pout, (size_t *) &outlen);
  }

 private:
  iconv_t cd;
};

#endif // CODECONVERTER_H
