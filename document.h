#ifndef DOCUMENT_H_
#define DOCUMENT_H_
#include <vector>

#include "token.h"

namespace sm {

class Document {
 public:
  Document (const std::string &content, const std::string &title, 
            uint64_t id, const std::string& encoding = "utf8");

  Document(const std::string& encoding = "utf8");
  int parseFromJsonRaw(const std::string &raw);
  int analysis(uint64_t mask = 
    SM_TOKEN_TYPE_DEFAULT | SM_TOKEN_TYPE_VERB | SM_TOKEN_TYPE_VD | SM_TOKEN_TYPE_VN);
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
