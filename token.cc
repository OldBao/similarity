#include <fstream>
#include <assert.h>
#include "token.h"
#include "encoding.h"

using namespace std;
using namespace sm;

extern const char *get_pos_str (unsigned int pos);


Token::Token(const string& encoding){
  _encoding = encoding;
}

Token::Token(token_t t, const string &encoding) {
  _encoding = encoding;
  if(encoding == "utf8") {
    encoding_gbk_to_utf8(t.buffer, &content);
  }
  weight = t.weight & ~0x800000;
  type = t.type;
  lprop = t.prop.m_lprop;
  hprop = t.prop.m_hprop;
  
  format();
}

int
Token::length() const{
  return _length;
}

void
Token::format(){
  char buffer[4096];
  snprintf (buffer, 4096, "[Content:%s] [Type:%s] [Weight:%d] [Prop:%d-%d] [Length:%d]",
            content.c_str(),
            get_pos_str(type),
            weight,
            lprop,
            hprop,
            length());
  _desc.assign(buffer);
  std::wstring wide;
  encoding_utf8_to_wchar(content, &wide);
  _length = wide.size();
}
