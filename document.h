#ifndef DOCUMENT_H_
#define DOCUMENT_H_
#include <vector>

#include "token.h"

namespace sm {

class Document {
 public:
  Document (const std::string &content, const std::string &title, 
            uint64_t id, const std::string& encoding = "utf8");

  int analysis(uint64_t mask = ~0); //segment
  const std::vector<Token>& getTokens() const{
    return _tokens;
  }

  virtual ~Document ();

  void getContent(std::string *ret, const std::string &encoding);
  uint64_t getId() {return _id;}
  void getTitle(std::string *ret, const std::string &encoding);

 private:
  std::string _content, _title;
  uint64_t _id;
  std::vector<Token> _tokens;
  std::string _encoding;
};

};
#endif
