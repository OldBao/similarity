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

  Dictionary dict;
  Corpus corpus;
  if ( 0 != dict.load(argv[1], "sim")){
    cout << "load dict error" << endl;
    return -1;
  }

  if ( 0 != corpus.load(argv[1], "sim")){
    cout << "load model error" << endl;
    return -1;
  }

  TopicModel *model = new LDAModel(&corpus, &dict);
  if ( 0 != model->load (argv[1], "lda")){
    cout << "load lda error" << endl;
    return -1;
  }

  corpus.truncate();

  Similarity *sim = new TopicSimilarity (model, &corpus, &dict, 3);
  for (int i = 1; i <= model->getNTopics(); i++) {
    sim->calculate(i);
  }
  
  sim->waitAllJobDone();
  return 0;
}
