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
  string gbk_content;
  int ret;
  if (_encoding == "utf8") {
    ret = encoding_utf8_to_gbk(_content, &gbk_content);
    if (ret)
      return ret;
  }

  if ( 0 != Segment::getInstance()->segment (&_tokens, gbk_content, mask)){
    return -1;
  }

  return 0;
}
