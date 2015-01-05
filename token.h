#ifndef TOKEN_H_
#define TOKEN_H_
#include "isegment.h"

namespace sm {
class Token {
 
 public:
  Token(const std::string &encoding = "utf8");
  Token(token_t scw_token);
  
  std::string type, content;
  int weight, lprop, hprop;

 private:
  std::string _encoding;
};
}
#endif
