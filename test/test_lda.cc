#include "corpus.h"
#include "model.h"
#include "test_lda.h"

using namespace std;

namespace sm {
TEST_F (LDATestCase, TestTopkHotwords) {
  Corpus corpus;
  corpus.getBows().resize(100);

  LDAModel model(&corpus, NULL);
  model._ntopics = 1;
  model._nterms = 10000;
  model._init_prob();
  
  const int VERY_BIG = 1000000;
  int i;
  for (i = 0; i < 5000; i++) {
    model._log_prob_w[0][i] = i;
  }
  model._log_prob_w[0][i++] = VERY_BIG;
  for (;i < 10000; i++) {
    model._log_prob_w[0][i] = i;
  }

  bow_t b;
  model.getHotestWords(&b, 1, 2);
  ASSERT_EQ(b.size(), 2);
  ASSERT_EQ(b[0].id, 5000);
  ASSERT_EQ(b[0].weight, VERY_BIG);

  ASSERT_EQ(b[1].id, 9999);
  ASSERT_EQ(b[1].weight, 9999);
}
}
