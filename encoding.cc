#include <errno.h>
#include <iconv.h>
#include "encoding.h"

using namespace std;
using namespace sm;

#define BUFFER_INC 4096

static int encoding_iconv (const string& from, string *to, const char * fromencode, const char* toencode) {
  iconv_t cd = iconv_open (toencode, fromencode);
  size_t outlen, inlen, ret;

  if (cd == (iconv_t)(-1)) {
    return -1;
  }

  inlen = from.size();
  to->resize(BUFFER_INC);
  outlen = BUFFER_INC;
  char *inbuf = (char *) from.c_str();
  char *outbuf = (char *) to->c_str();

  while (inlen > 0) {
    ret = iconv (cd, &inbuf, &inlen, &outbuf, &outlen);
    if (ret == (size_t) (-1)) {
      if (errno == E2BIG)  {
        to->resize (to->size() + BUFFER_INC);
        outlen += BUFFER_INC;
        continue;
      }
      else {
        break;
      }
    }
  }
  
  iconv_close(cd);

  return inlen == 0 ? 0 : -1;
}


int 
encoding_utf8_to_gbk (const string &utf, string *gbk) 
{
  if (!gbk) {
    return -1;
  }
  
  return encoding_iconv (utf, gbk, "UTF8", "GBK");
}


int 
encoding_gbk_to_utf8 (const std::string &gbk, std::string *utf) {
  if (!utf) {
    return -1;
  }

  return encoding_iconv (gbk, utf, "GBK", "UTF8");
}
