#ifndef SEGMENT_H_
#define SEGMENT_H_
#include <set>

#include <isegment.h>
#include <ipostag.h>
#include "token.h"
#include "singleton.h"

namespace sm{

  class Segment {
    SM_DECLARE_SINGLETON(Segment, const char*);

 public:
    int load (const std::string &path,
              const std::string &postag_path,
              const std::string &stopwords = "");

    int segment (std::vector <Token>* tokens, const std::string &line, uint64_t mask = 0, const std::string &encoding = "gbk", const std::string& token_encoding="utf8");

 private:
    bool _token_stopped(const Token &token);
  scw_worddict_t *_scw_dict;
  std::set<std::wstring> _stop_words;
  };
 
};
#endif
