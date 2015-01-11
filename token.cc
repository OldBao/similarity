#include <fstream>
#include <assert.h>
#include "token.h"
#include "encoding.h"
#include "log.h"

using namespace std;
using namespace sm;

extern const char *get_pos_str (unsigned int pos);

static void
replace_all(std::string* str, const std::string &from, const std::string &to) {
  size_t start_pos = 0;

  while (1) {
  start_pos = str->find(from, start_pos);
    if (start_pos == std::string::npos){
        break;
    }
    str->replace(start_pos, from.length(), to);
    start_pos += to.length(); 
  }
}


Token::Token(){
}

Token::Token(token_t t, const std::string &encoding) {
  string buffer(t.buffer);
  replace_all(&buffer, "\n", "");

  wtrans_func_t *w = get_wtrans(encoding);
  assert (w);
  if (0 != w(buffer, &content)){
    SM_LOG_WARNING ("encoding token [%s] error", t.buffer);
  }

  weight = t.weight & ~0x800000;
  type = t.type;
  lprop = t.prop.m_lprop;
  hprop = t.prop.m_hprop;
  
  format();
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
            content.length());
  _desc.assign(buffer);
}
