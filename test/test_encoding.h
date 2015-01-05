#ifndef SM_TEST_ENCODING_H_
#define SM_TEST_ENCODING_H_

#include "test_main.h"
#include "encoding.h"
#include <fstream>
#include <string>

class EncodingTestCase : public ::testing::Test {
 protected:
  virtual ~EncodingTestCase() {}
  
  void SetUp() {}
  void TearDown() {}
};
#endif
