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
  Document doc(line, "",  "", "utf8");

  ASSERT_EQ(0, doc.analysis());
  ASSERT_EQ(1, doc.getTokens().size());

  wstring buffer;
  encoding_utf8_to_wchar(UTF8_SHORT_LINE_WITHOUT_NEWLINE, &buffer);
  ASSERT_EQ(buffer, doc.getTokens().at(0).content);
}


TEST_F(DictionaryTestCase, TestDupToken) {
  Dictionary dict;
  string line = UTF8_SHORT_LINE;
  Document doc(line, "", "", "utf8");

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
  Document doc1(GBK_SHORT_LINE, "", "", "GBK");
  Document doc2(GBK_SHORT_LINE2, "", "", "GBK");
  int id;
  string word;
  string encoding;
  int dfs, nnz, wn;

  ASSERT_EQ(0, doc1.analysis());
  ASSERT_EQ(0, doc2.analysis());

  dict.addDocument(doc1);
  dict.addDocument(doc2);

  ASSERT_EQ(0, dict.save("/tmp", "__tmp_gbk", "GBK"));

  const char *expect_fp = "/tmp/__tmp_gbk.dict";
  const char *expect_fp_meta = "/tmp/__tmp_gbk.dict.meta";
  struct stat s;

  ASSERT_EQ(0, stat(expect_fp, &s));
  ASSERT_NE(0, S_ISREG(s.st_mode));

  ASSERT_EQ(0, stat(expect_fp_meta, &s));
  ASSERT_NE(0, S_ISREG(s.st_mode));
  
  fstream is(expect_fp, ios::in);

  is >> encoding;
  ASSERT_STREQ (encoding.c_str(), "GBK");

  is >> dfs >> word;
  ASSERT_STREQ(word.c_str(), GBK_SHORT_TOKEN);
  ASSERT_EQ (dfs, 1);
  is >> dfs >> word;
  ASSERT_EQ (dfs, 1);
  ASSERT_STREQ(word.c_str(), GBK_SHORT_TOKEN2);
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

