#include <fstream>

#include "log.h"
#include "segment.h"
#include "encoding.h"

using namespace sm;
using namespace std;

static const int scw_flags = SCW_OUT_ALL | SCW_OUT_PROP;
static const int MAX_TOKENS = 10240;
static const int MAX_WORD_SIZE = 1024;
SM_IMP_SINGLETON(Segment);


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

  FILE *fp = NULL;
  if (!stopword_file.empty()) {
    fp = fopen(stopword_file.c_str(), "r");
    if (!fp) {
      SM_LOG_FATAL ("open stop words file %s fail", stopword_file.c_str());
      return -1;
    }

    char encoding[MAX_WORD_SIZE];
    char line[MAX_WORD_SIZE];
    fscanf (fp, "%s", encoding);
    wtrans_func_t *trans = get_wtrans(encoding);
    if (!trans) {
      SM_LOG_FATAL ("stop word txt format error!, please set encoding header to file");
      return -1;
    }

    while (true) {
      int ret = fscanf (fp, "%s", line);
      if (ret == EOF) break;
      if (ret != 1) {
        SM_LOG_WARNING ("something wrong happens");
        continue;
      }
      wstring tmp;
      trans(line, &tmp);
      _stop_words.insert (tmp);
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
                 const std::string& encoding)
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
      Token token(tokens[i]);
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
  if (_stop_words.find(t.content) != _stop_words.end()) {
    return true;
  }

  return false;
}
