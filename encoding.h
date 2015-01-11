#ifndef ENCODING_H_
#define ENCODING_H_

#include <string>

namespace sm {
  typedef int (wtrans_func_t)(const std::string &src, std::wstring *dest);
  typedef int (rwtrans_func_t)(const std::wstring &wide, std::string *dest);

  rwtrans_func_t *get_rwtrans(const std::string &encoding);
  wtrans_func_t *get_wtrans(const std::string &encoding);

  extern int encoding_utf8_to_gbk (const std::string &utf, std::string *gbk);
  extern int encoding_gbk_to_utf8 (const std::string &gbk, std::string *utf);
  extern int encoding_utf8_to_wchar (const std::string &utf, std::wstring *wide);
  extern int encoding_gbk_to_wchar (const std::string &gbk, std::wstring *wide);
  extern int encoding_wchar_to_utf8 (const std::wstring &wide, std::string *utf8);
  extern int encoding_wchar_to_gbk (const std::wstring &wide, std::string *gbk);
};
#endif
