#include <errno.h>
#include <assert.h>
#include <iconv.h>
#include "encoding.h"
#include "log.h"

using namespace std;
using namespace sm;

#define BUFFER_SIZE 4096
const char * UTF8 = "UTF8//IGNORE";
const char * GBK = "GBK//IGNORE";
const char * WCHAR = "WCHAR_T//IGNORE";

static int encoding_iconv (const string& from, string *to, const char * fromencode, const char* toencode) {
  iconv_t cd = iconv_open (toencode, fromencode);
  size_t outlen, inlen, ret;
  char buffer[BUFFER_SIZE];

  if (cd == (iconv_t)(-1)) {
    return -1;
  }

  inlen = from.size();
  outlen = BUFFER_SIZE;
  char *inbuf = (char *) from.data();
  char *outbuf = buffer;

  while (inlen > 0) {
    ret = iconv (cd, &inbuf, &inlen, &outbuf, &outlen);
    if (ret == (size_t) (-1)) {
      if (errno == E2BIG)  {
        to->append(buffer, BUFFER_SIZE - outlen);
        outbuf = buffer;
        outlen = BUFFER_SIZE;
        continue;
      } else if (errno == EINVAL) {
        SM_LOG_NOTICE ("incomplete byte in %d", from.size() - inlen);
        break;
      }
      else {
        break;
      }
    } else {
      to->append(buffer, BUFFER_SIZE - outlen);
    }
  }
  
  iconv_close(cd);

  return inlen == 0 ? 0 : -1;
}


int 
sm::encoding_utf8_to_gbk (const std::string &utf, std::string *gbk) 
{
  if (!gbk) {
    return -1;
  }
  
  return encoding_iconv (utf, gbk, UTF8, GBK);
}


int 
sm::encoding_gbk_to_utf8 (const std::string &gbk, std::string *utf) {
  if (!utf) {
    return -1;
  }

  return encoding_iconv (gbk, utf, GBK, UTF8);
}


int
sm::encoding_wchar_to_utf8(const std::wstring &wide, std::string* utf8) {
  std::string buffer;
  const char *p = reinterpret_cast<const char *>(wide.data());
  const char *q = p + sizeof (wchar_t) * wide.size();
  buffer.assign (p, q);
  
  int ret =encoding_iconv(buffer, utf8, WCHAR, UTF8);
  if (ret) {
    return ret;
  }
  
  return 0;
  
}

int
sm::encoding_wchar_to_gbk(const std::wstring &wide, std::string* gbk) {
  std::string buffer;
  const char *p = reinterpret_cast<const char *>(wide.data());
  const char *q = p + sizeof (wchar_t) * wide.size();
  buffer.assign (p, q);
  
  int ret =encoding_iconv(buffer, gbk, WCHAR, GBK);
  if (ret) {
    return ret;
  }
  return 0;
}


int
sm::encoding_utf8_to_wchar(const std::string &utf, std::wstring* wide) {
  std::string buffer;

  int ret =encoding_iconv(utf, &buffer, UTF8, WCHAR);
  if (ret) {
    return ret;
  }

  if (buffer.empty()){
    return -1;
  }

  assert (buffer.size() % sizeof (wchar_t) == 0);

  const wchar_t *p = reinterpret_cast<const wchar_t *>(buffer.data());
  const wchar_t *q = p + buffer.size() / sizeof (wchar_t);
  wide->assign(p, q);

  return 0;
  
}

int
sm::encoding_gbk_to_wchar(const std::string &gbk, std::wstring* wide) {
  std::string buffer;

  int ret =encoding_iconv(gbk, &buffer, GBK, WCHAR);
  if (ret) {
    return ret;
  }

  if (buffer.empty()){
    return -1;
  }

  assert (buffer.size() % sizeof (wchar_t) == 0);

  const wchar_t *p = reinterpret_cast<const wchar_t *>(buffer.data());
  const wchar_t *q = p + buffer.size() / sizeof (wchar_t);
  wide->assign(p, q);

  return 0;
  
}

rwtrans_func_t *sm::get_rwtrans(const std::string &encoding){
  if (0==strncasecmp(encoding.c_str(), "utf8", 16) ||
      0==strncasecmp(encoding.c_str(), "utf-8",16)) {
    return encoding_wchar_to_utf8;
  } else if ( 0==strncasecmp(encoding.c_str(), "gbk", 16)) {
    return encoding_wchar_to_gbk;
  } else {
    return NULL;
  }
}

wtrans_func_t *sm::get_wtrans(const std::string &encoding) {
  if (0==strncasecmp(encoding.c_str(), "utf8", 16) ||
      0==strncasecmp(encoding.c_str(), "utf-8",16)) {
    return encoding_utf8_to_wchar;
  } else if ( 0==strncasecmp(encoding.c_str(), "gbk", 16)) {
    return encoding_gbk_to_wchar;
  } else {
    return NULL;
  }
}
