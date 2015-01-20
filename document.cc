#include <json/json.h>
#include <inttypes.h>
#include "segment.h"
#include "document.h"
#include "encoding.h"

using namespace std;
using namespace sm;


Document::Document(const string& encoding) : 
  _content (""), 
  _title(""),
  _id((uint64_t)-1), 
  _encoding(encoding)
{

}

Document::Document(const std::string &content, const std::string &title, uint64_t id, const std::string& encoding) :
  _content (content),
  _title (title),
  _id (id), 
  _encoding(encoding)
{
  
}

Document::~Document(){

}

int
Document::parseFromJsonRaw(const std::string &raw) {
  Json::Reader reader;
  Json::Value value;
  
  if (!reader.parse(raw, value)) {
    SM_LOG_WARNING ("json parse error : %s", raw.c_str());
    return -1;
  }

  if (value.isMember("article") && value["article"].isString()){
    _content = value["article"].asString();
  } else {
    SM_LOG_WARNING ("'ariticle' keyword not found in cache");
    return -1;
  }

  if (value.isMember("docid")) {
    if (value["docid"].isNumeric()) {
      _id = value["docid"].asUInt64();
    } else if (value["docid"].isString()) {
      if (!value["docid"].isString() ||
          1 != sscanf (value["docid"].asString().c_str(), "%" PRIu64, &_id)) 
        {
          SM_LOG_WARNING("'docid' format invalid");
          return -1;
        }
    } else {
      SM_LOG_WARNING ("'docid' not set");
      return -1;
    }
  }

  if (value.isMember("title") && value["title"].isString()) {
   _title = value["title"].asString();
  }

#ifdef DEBUG
  FILE *fp = fopen("docmap", "a+");
  fprintf (fp, "%" PRIu64 "\t%s\n", _id, value["url"].asString().c_str());
  fclose(fp);
#endif

  return 0;
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
