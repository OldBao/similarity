#include "test_dictionary.h"

#include <fstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

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


TEST_F(DictionaryTestCase, TestSave) {
  Dictionary dict;
  Document doc1(UTF8_SHORT_LINE, "", "utf8");
  Document doc2(UTF8_SHORT_LINE2, "", "utf8");
  int id;
  string word;
  int dfs, nnz, wn;

  ASSERT_EQ(0, doc1.analysis());
  ASSERT_EQ(0, doc2.analysis());

  dict.addDocument(doc1);
  dict.addDocument(doc2);

  ASSERT_EQ(0, dict.save("/tmp", "__tmp"));

  const char *expect_fp = "/tmp/__tmp.dict";
  const char *expect_fp_meta = "/tmp/__tmp.dict.meta";
  struct stat s;

  ASSERT_EQ(0, stat(expect_fp, &s));
  ASSERT_NE(0, S_ISREG(s.st_mode));

  ASSERT_EQ(0, stat(expect_fp_meta, &s));
  ASSERT_NE(0, S_ISREG(s.st_mode));
  
  fstream is(expect_fp, ios::in);

  is >> id;
  ASSERT_EQ(id, 0);
  is >> word >> dfs;
  ASSERT_STREQ(word.c_str(), UTF8_SHORT_TOKEN);
  ASSERT_EQ (dfs, 1);
  is >> word >> dfs;
  ASSERT_EQ (dfs, 1);
  ASSERT_STREQ(word.c_str(), UTF8_SHORT_TOKEN2);
  is >> word;
  ASSERT_TRUE(is.eof());

  is.close();

  is.open(expect_fp_meta, ios::in);
  is >> word >> nnz;
  ASSERT_STREQ (word.c_str(), "non-zero-entry");
  ASSERT_EQ (nnz, 2);
  is >> word >> wn;
  ASSERT_STREQ (word.c_str(), "word-num");
  ASSERT_EQ (wn, 2);
  is >> word;
  ASSERT_TRUE(is.eof());
  is.close();

  unlink(expect_fp);

}

TEST_F(DictionaryTestCase, TestLoad) {
  Dictionary dict, otherdict;
  Document doc1(UTF8_SHORT_LINE, "", "utf8");
  Document doc2(UTF8_SHORT_LINE2, "", "utf8");
  string word;

  ASSERT_EQ(0, doc1.analysis());
  ASSERT_EQ(0, doc2.analysis());

  dict.addDocument(doc1);
  dict.addDocument(doc2);

  ASSERT_EQ(0, dict.save("/tmp", "__tmp"));
  ASSERT_EQ(0, otherdict.load("/tmp", "__tmp"));
  ASSERT_EQ(otherdict.size(), 2);

  ASSERT_EQ(dict.getWords(), otherdict.getWords());
  ASSERT_EQ(dict.getNnz(), otherdict.getNnz());
  ASSERT_EQ(dict.getWordsMap(), otherdict.getWordsMap());
  ASSERT_EQ(dict.getDfs(), otherdict.getDfs());
}

