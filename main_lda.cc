#include <tagdict.h>

#include <algorithm>
#include <fstream>

#include <sys/stat.h>

#include "segment.h"
#include "dictionary.h"
#include "corpus.h"
#include "model.h"

using namespace sm;
using namespace std;

bool
mycmp(const pair<int, double>& a, const pair<int, double>& b){
  return a.second > b.second;
}

int
main(int argc, char **argv){
  if (argc != 2) {
    cout << "usage : ./train data" << endl; 
    return -1;
  } 

  ul_logstat_t logstat = {16, 0, UL_LOGTTY};
  ul_openlog("./log", "train", &logstat, 1024);

  FILE *fileptr;
  Corpus corpus;

  fileptr = fopen(argv[1], "r");
  int word, count, length, line = 0;
  while ((fscanf(fileptr, "%10d", &length) != EOF)) {
    line++;
    bow_t bow;
    for (int n = 0; n < length; n++) {
      bow_unit_t u;
      fscanf(fileptr, "%10d:%10d", &word, &count);
      u.id = word;
      u.weight = count;
      bow.push_back (u);
    }
    corpus.addDoc(bow);
    if (corpus.getNTerms()> 15000) {
      cout << "Error happens in reading line " << line << endl;
      return -1;
    }
  }

  fclose(fileptr);
  printf("number of docs    : %d\n", corpus.size());
  printf("number of terms   : %d\n", corpus.getNTerms());

  LDAModel *model = new LDAModel(&corpus, NULL);
  model->train();
  return 0;
}
