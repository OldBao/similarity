#ifndef DOCUMENT_H_
#define DOCUMENT_H_
#include <vector>

#include "token.h"

namespace sm {

class Document {
 public:
  Document (const std::string &content, const std::string &title, const std::string& encoding = "utf8");

  int analysis(uint64_t mask = ~0); //segment
  const std::vector<Token>& getTokens() const{
    return _tokens;
  }

  virtual ~Document ();
 private:
  std::string _content, _title;
  std::vector<Token> _tokens;
  std::string _encoding;
};

};
#endif
