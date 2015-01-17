#include <tagdict.h>

#include <algorithm>
#include <fstream>

#include <sys/stat.h>

#include "segment.h"
#include "dictionary.h"
#include "corpus.h"
#include "model.h"
#include "encoding.h"
#include "similarity.h"

using namespace sm;
using namespace std;

int
main(int argc, char **argv){
  if (argc != 2) {
    cout << "usage ./sim model_dict" << endl;
    return -1;
  }

  Corpus corpus;

  Dictionary dict;
  if ( 0 != dict.load(argv[1], "similarity")){
    cout << "load dict error" << endl;
    return -1;
  }

  if ( 0 != corpus.load(argv[1], "similarity")){
    cout << "load model error" << endl;
    return -1;
  }

  TopicModel *model = new LDAModel(&corpus, &dict);
  if ( 0 != model->load (argv[1], "similarity")){
    cout << "load lda error" << endl;
    return -1;
  }

  //corpus.truncate();

  Similarity *sim = new TopicSimilarity (model, &corpus, &dict, 5);

  for (int i = 1; i <= model->getNTopics(); i++) {
    sim->calculate(i);
  }

  sim->waitAllJobDone();

  cout << "all job done" << endl;
  for (int i = 0; i < corpus.size(); i++) {
    bow_t ret;
    cout << "Sim of [ " << corpus.getDocid(i)  << ":";
    sim->getSimilarities(&ret, i, 0.0, 5);
    cout << ret.size() << "]\t";

    for (int j = 0; j < ret.size(); j++) {
      cout << "[" << corpus.getDocid(ret[j].id) << ":" << ret[j].weight << "]";
    }
    cout << endl;
  }
  return 0;
}
