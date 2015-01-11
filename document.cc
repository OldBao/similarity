#include "segment.h"
#include "document.h"
#include "encoding.h"

using namespace std;
using namespace sm;

Document::Document(const std::string &content, const std::string &title, const std::string& encoding) :
  _content (content),
  _title (title),
  _encoding(encoding)
{
  
}

Document::~Document(){

}

int
Document::analysis(uint64_t mask){
  int ret;
  string gbk_content;

  if (strcasecmp(_encoding.c_str(), "utf8") == 0 || 
      strcasecmp(_encoding.c_str(), "utf-8") == 0) {
    ret = encoding_utf8_to_gbk(_content, &gbk_content);
    if (ret)
      return ret;
  } else {
    gbk_content = _content;
  }

  if ( 0 != Segment::getInstance()->segment (&_tokens, gbk_content, mask)){
    return -1;
  }

  return 0;
}
