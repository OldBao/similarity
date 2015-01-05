#ifndef SEGMENT_H_
#define SEGMENT_H_

#include <isegment.h>
#include <ipostag.h>
#include "token.h"
#include "singleton.h"

namespace sm{

  class Segment {
    SM_DECLARE_SINGLETON(Segment, const char*);

 public:
    int load (const std::string &path,
              const std::string &postag_path);

    int segment (std::vector <Token>* tokens, const std::string &line, uint64_t mask = 0, const std::string &encoding = "gbk", const std::string& token_encoding="utf8");

 private:
  scw_worddict_t *_scw_dict;
  };
 
};
#endif
