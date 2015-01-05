#include "token.h"

using namespace std;
using namespace sm;

extern const char *get_pos_str (unsigned int pos);


Token::Token(const string& encoding){
  _encoding = encoding;
}

Token::Token(token_t t) {
    content = t.buffer;
    weight = t.weight & ~0x800000;
    type = get_pos_str(t.type);
    lprop = t.prop.m_lprop;
    hprop = t.prop.m_hprop;
}

