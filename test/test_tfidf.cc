#include "test_tfidf.h"

using namespace std;
using namespace sm;

static void add_w(bow_t *b, int id, double weight) {
  bow_unit_t tmpu;

  tmpu.id = id;
  tmpu.weight = weight;
  b->push_back(tmpu);
}


TEST_F (TFIDFTestCase, TestNormal){
  bow_t bow;

  Corpus* corpus = new Corpus();

  add_w(&bow, 1, 1);
  add_w(&bow, 2, 1);
  add_w(&bow, 3, 1);

  corpus->addDoc (1, bow);
  
  bow.clear();
  add_w(&bow, 1, 2);
  corpus->addDoc (2, bow);

  bow.clear();
  add_w(&bow, 2, 2);
  corpus->addDoc (3, bow);

  ASSERT_EQ (corpus->size(), 3);

  bow.clear();
  add_w(&bow, 1, 2);
  add_w(&bow, 3, 2);
  //1 : tf: 2 idf log2(3/2) 
  //3 : tf: 2 idf log2(3/1)

  TFIDFModel *model = new TFIDFModel(corpus, NULL);

  model->train();

  const vector<double> &idf = model->idf();
  ASSERT_DOUBLE_EQ (idf[1], log2(1.5));
  ASSERT_DOUBLE_EQ (idf[2], log2(1.5));
  ASSERT_DOUBLE_EQ (idf[3], log2(3.0));

  bow_t ret;
  model->inference(bow, &ret);
  ASSERT_EQ (ret.size(), 2);

  ASSERT_EQ (ret[1].id, 1);
  ASSERT_DOUBLE_EQ (2.0 * log2(3.0/2), ret[1].weight);

  ASSERT_EQ (ret[0].id, 3);
  ASSERT_DOUBLE_EQ (2.0 * log2(3.0/1), ret[0].weight);
  
}
