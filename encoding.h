#ifndef ENCODING_H_
#define ENCODING_H_

#include <string>

namespace sm {
  int encoding_utf8_to_gbk (const std::string &utf, std::string *gbk);
  int encoding_utf8_to_gbk (const std::string &gbk, std::string *utf);
};
#endif
