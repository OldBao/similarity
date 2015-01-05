#ifndef SEGMENT_H_
#define SEGMENT_H_

#include <isegment.h>
#include <ipostag.h>
#include "token.h"
#include "singleton.h"

namespace sm{

  class Segment {
    SM_DECLARE_SINGLETON(Segment);

 public:
    int load (const std::string &path,
              const std::string &postag_path);

    int segment (std::vector <Token>* tokens, const std::string &line);

 private:
  scw_worddict_t *_scw_dict;
  };
 
};
#endif
