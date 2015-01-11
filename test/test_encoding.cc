#include <iostream>
#include "test_encoding.h"

using namespace std;
using namespace sm;

TEST_F (EncodingTestCase, TestNormal) {
  string gbk, utf, tmp_utf, tmp_gbk; 
  gbk.assign(GBK_SHORT_LINE);
  utf.assign(UTF8_SHORT_LINE);
  
  ASSERT_EQ (0, encoding_gbk_to_utf8(gbk, &tmp_utf));
  ASSERT_EQ (0, encoding_utf8_to_gbk(utf, &tmp_gbk));

  ASSERT_EQ (gbk, tmp_gbk);
  ASSERT_EQ (utf, tmp_utf);
}


TEST_F (EncodingTestCase, TestWChar) {
  string gbk, utf, tmp_gbk, tmp_utf8;
  wstring wide;

  gbk.assign(GBK_SHORT_LINE);
  utf.assign(UTF8_SHORT_LINE);

  ASSERT_EQ (0, encoding_gbk_to_wchar(gbk, &wide));
  ASSERT_EQ (6, wide.size());
  ASSERT_EQ (0, encoding_wchar_to_gbk(wide, &tmp_gbk));
  ASSERT_EQ (tmp_gbk, gbk);

  wide.clear();
  ASSERT_EQ (0, encoding_utf8_to_wchar(utf, &wide));
  ASSERT_EQ (6, wide.size());
  ASSERT_EQ (0, encoding_wchar_to_utf8(wide, &tmp_utf8));
  ASSERT_EQ (tmp_utf8, utf);
}
