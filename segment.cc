#include <fstream>

#include "log.h"
#include "segment.h"
#include "encoding.h"

using namespace sm;
using namespace std;

static const int scw_flags = SCW_OUT_ALL | SCW_OUT_PROP;
static const int MAX_TOKENS = 10240;

SM_IMP_SINGLETON(Segment);
typedef int (wtrans_func_t)(const string &src, wstring *dest);

static wtrans_func_t *get_wtrans(const std::string &encoding) {
  if (0==strncasecmp(encoding.c_str(), "utf8", 16) ||
      0==strncasecmp(encoding.c_str(), "utf-8",16)) {
    return encoding_utf8_to_wchar;
  } else if ( 0==strncasecmp(encoding.c_str(), "gbk", 16)) {
    return encoding_gbk_to_wchar;
  } else {
    return NULL;
  }
}


int
Segment::load (const std::string &path, const std::string& postag_path, const std::string& stopword_file){
  if (NULL == (_scw_dict = scw_load_worddict(path.c_str())) ){
    SM_LOG_FATAL ("loading segment dict error");
    return -1;
  }

  if (0 != tag_open (postag_path.c_str())) {
    SM_LOG_FATAL ("open tag error");
    return -1;
  }

  if (!stopword_file.empty()) {
    fstream sf(stopword_file.c_str(), ios::in | ios::binary);
    if (!sf.is_open()) {
      SM_LOG_FATAL ("open stop words file %s fail", stopword_file.c_str());
      return -1;
    }

    string encoding, line;
    wstring s;
    sf >> encoding;
    wtrans_func_t *trans = get_wtrans(encoding);
    if (!trans) {
      SM_LOG_FATAL ("stop word txt format error!, please set encoding header to file");
      return -1;
    }

    while (!sf.eof ()) {
      sf >> line;
      
      if (0 == trans(line, &s)) {
        _stop_words.insert (s);
      }
    }

    SM_LOG_DEBUG ("Add %zu stop words to list", _stop_words.size());
  }

  return 0;
}


Segment::~Segment(){
  if (_scw_dict) {
    scw_destroy_worddict (_scw_dict);
  }

  tag_close();
}

int
Segment::segment(vector<Token> *ret_tokens, const string &line, 
                 uint64_t mask,
                 const std::string& encoding, 
                 const std::string& token_encoding)
{
  scw_out_t *out = NULL;
  int ret;
  token_t tokens [MAX_TOKENS];
  memset (tokens, 0, MAX_TOKENS * sizeof (token_t));
  int i;

  out = scw_create_out (MAX_TOKENS, scw_flags);
  if (!out) {
    SM_LOG_WARNING ("create segment out error");
    goto error;
  }

  ret = scw_segment_words (_scw_dict, out, line.c_str(), line.length(), LANGTYPE_SIMP_CHINESE);
  if (ret < 0) {
    SM_LOG_WARNING ("segment error");
    goto error;
  }

  ret = scw_get_token_1 (out, SCW_OUT_WPCOMP, tokens, MAX_TOKENS);
  if (ret < 0) {
    SM_LOG_WARNING ("get tokens error");
    goto error;
  }

  tag_postag(tokens, (uint32_t) ret);
  
  for (i = 0; i < ret; i++) {
    if (SM_POS2TYPE(tokens[i].type) & mask) {
      Token token(tokens[i], token_encoding);
      if (!_token_stopped(token))
        ret_tokens->push_back(token);
    }

  }
  
  scw_destroy_out (out);
  return 0;

 error:
  if (out) {
    scw_destroy_out (out);
  }

  return -1;
}

bool
Segment::_token_stopped(const Token &t) {
  wtrans_func_t *trans = get_wtrans(t.encoding());
  if (trans) {
    wstring ret;
    trans (t.content, &ret);
    if (_stop_words.find(ret) != _stop_words.end()) {
      return true;
    }
  }

  return false;
}
