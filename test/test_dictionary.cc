#include "test_dictionary.h"

using namespace std;
using namespace sm;

TEST_F(DictionaryTestCase, TestDupToken) {
  Dictionary dict;
  string line = UTF8_SHORT_LINE;
  Document doc(line, "", "utf8");

  ASSERT_EQ(0, doc.analysis());
  ASSERT_EQ(0, dict.addDocument(doc));
  ASSERT_EQ (dict.size(), 1);
  ASSERT_EQ (0, dict.addDocument(doc));
  ASSERT_EQ (dict.size(), 1);
}

TEST_F (DictionaryTestCase, TestTwoDifferenctToken){
  Dictionary dict;
  Document doc1(UTF8_SHORT_LINE, "", "utf8");
  Document doc2(UTF8_SHORT_LINE2, "", "utf8");

  ASSERT_EQ(0, doc1.analysis());
  ASSERT_EQ(0, doc2.analysis());
  ASSERT_EQ(0, dict.addDocument(doc1));
  ASSERT_EQ (dict.size(), 1);
  ASSERT_EQ (0, dict.addDocument(doc2));
  ASSERT_EQ (dict.size(), 2);
}
