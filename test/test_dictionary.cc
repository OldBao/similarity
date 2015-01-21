#include "test_dictionary.h"

#include <fstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "encoding.h"

using namespace std;
using namespace sm;


TEST_F(DictionaryTestCase, TestLineWithNewline) {
  string line = UTF8_SHORT_LINE_WITH_NEWLINE;
  Document doc(line, "",  -1, "utf8");

  ASSERT_EQ(0, doc.analysis());
  ASSERT_EQ(1, doc.getTokens().size());

  wstring buffer;
  encoding_utf8_to_wchar(UTF8_SHORT_LINE_WITHOUT_NEWLINE, &buffer);
  ASSERT_EQ(buffer, doc.getTokens().at(0).content);
}


TEST_F(DictionaryTestCase, TestDupToken) {
  Dictionary dict;
  string line = UTF8_SHORT_LINE;
  Document doc(line, "", -1, "utf8");

  ASSERT_EQ(0, doc.analysis());
  ASSERT_EQ(0, dict.addDocument(doc));
  ASSERT_EQ (dict.size(), 1);
  ASSERT_EQ (0, dict.addDocument(doc));
  ASSERT_EQ (dict.size(), 1);
}

TEST_F (DictionaryTestCase, TestTwoDifferenctToken){
  Dictionary dict;
  Document doc1(UTF8_SHORT_LINE, "", -1, "utf8");
  Document doc2(UTF8_SHORT_LINE2, "", -1, "utf8");

  ASSERT_EQ(0, doc1.analysis());
  ASSERT_EQ(0, doc2.analysis());
  ASSERT_EQ(0, dict.addDocument(doc1));
  ASSERT_EQ (dict.size(), 1);
  ASSERT_EQ (0, dict.addDocument(doc2));
  ASSERT_EQ (dict.size(), 2);
}


