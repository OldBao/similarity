#ifndef DOCUMENT_H_
#define DOCUMENT_H_
#include <vector>

#include "token.h"

namespace sm {

class Document {
 public:
  Document (const std::string &content);
  Document (const std::string &content, const std::string &title);

  int analysis(); //segment
  const std::vector<Token>& getTokens() const{
    return _tokens;
  }

  virtual ~Document ();
 private:
  std::string _content, _title;
  std::vector<Token> _tokens;
};

};
#endif
