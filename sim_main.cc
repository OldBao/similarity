#include <tagdict.h>

#include <algorithm>
#include <fstream>

#include <sys/stat.h>

#include "segment.h"
#include "dictionary.h"
#include "corpus.h"
#include "model.h"
#include "encoding.h"

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


  double max_sim = 0.0;
  int max;
  for (int i = 0; i < corpus.size()-1; i++) {
    const bow_t &src = corpus[i];
    max_sim = max = -1;
    for (int j = 0; j < corpus.size()-1; j++) {
      if (j == i) continue;
      const bow_t &dest = corpus[j];
      double sim = src.cossim(dest);
      if (sim > max_sim) {
        max_sim = sim;
        max = j;
      }
    }
    cout << i << " " << max << " " << max_sim << endl;
  }
  return 0;
}
