#include "log.h"
#include "segment.h"

using namespace sm;
using namespace std;

static const int scw_flags = SCW_OUT_ALL | SCW_OUT_PROP;
static const int MAX_TOKENS = 10240;

SM_IMP_SINGLETON(Segment);

int
Segment::load (const std::string &path, const std::string& postag_path){
  if (NULL == (_scw_dict = scw_load_worddict(path.c_str())) ){
    SM_LOG_FATAL ("loading segment dict error");
    return -1;
  }

  if (0 != tag_open (postag_path.c_str())) {
    SM_LOG_FATAL ("open tag error");
    return -1;
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
Segment::segment(vector<Token> *ret_tokens, const string &line){
  scw_out_t *out = NULL;
  int ret;
  token_t tokens [MAX_TOKENS];
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

  tag_postag(tokens, ret);
  
  for (i = 0; i < ret; i++) {
    Token token(tokens[i]);
    ret_tokens->push_back(token);
  }
  
  return 0;
 error:
  if (out) {
    scw_destroy_out (out);
  }

  return -1;
}
