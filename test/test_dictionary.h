#ifndef SM_TEST_DICTIONARY_H_
#define SM_TEST_DICTIONARY_H_

#include "test_main.h"
#include "dictionary.h"
#include <fstream>
#include <string>

class DictionaryTestCase : public ::testing::Test {
 protected:
  virtual ~DictionaryTestCase() {}
  
  void SetUp() {}
  void TearDown() {}
  
};
#endif
