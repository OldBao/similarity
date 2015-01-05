#include "segment.h"
#include "document.h"

using namespace std;
using namespace sm;

Document::Document(const std::string &content, const std::string &title) :
  _content (content),
  _title (title)
{

}

Document::~Document(){

}

int
Document::analysis(){
  if ( 0 != Segment::getInstance()->segment (&_tokens, _content)){
    return -1;
  }

  return 0;
}
