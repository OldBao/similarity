#ifndef SM_TEST_TFIDF_H_
#define SM_TEST_TFIDF_H_

#include "test_main.h"
#include "dictionary.h"
#include "corpus.h"
#include "model.h"
#include <fstream>
#include <string>

class TFIDFTestCase : public ::testing::Test {
 protected:
  virtual ~TFIDFTestCase() {}
  
  void SetUp() {}
  void TearDown() {}
  
};
#endif
