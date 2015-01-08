#ifndef ENCODING_H_
#define ENCODING_H_

#include <string>

namespace sm {
  extern int encoding_utf8_to_gbk (const std::string &utf, std::string *gbk);
  extern int encoding_gbk_to_utf8 (const std::string &gbk, std::string *utf);
  extern int encoding_utf8_to_wchar (const std::string &utf, std::wstring *wide);
  extern int encoding_gbk_to_wchar (const std::string &gbk, std::wstring *wide);
};
#endif
