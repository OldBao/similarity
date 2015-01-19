#include <iostream>
#include <fstream>
#include <signal.h>
#include "repo.h"
#include "segment.h"
#include "model.h"

using namespace std;
using namespace sm;

int
main(int argc, char **argv){
  fstream f("docmap", ios::out | ios::trunc);
  f.close();
  signal(SIGPIPE, SIG_IGN);
  ul_logstat_t logstat;
  logstat.events = 4;
  logstat.spec= 0;
  ul_openlog("./log", "junk", &logstat, 1024);
  if (argc != 4) {
    cout << "./server url path count" << endl;
    return 0;
  }

  int max = atoi(argv[3]);
  fstream is(argv[1], ios::in);
  int i;

  if ( 0 != Segment::getInstance()->load ("worddict", "postag", "stopwords.utf8")){
    cout << "init segment error" << endl;
    return -1;
  }

  Repository repo(12, "doccache");

  i = 0;
  while (1) {
    i++;
    if (i % 10000 == 0) {
      sleep (10);
    }
    if (i == max) break;
    string url;
    is >> url;
    if (is.eof()) break;
    repo.addUrl(url);
  }

  repo.waitAllJobDone();
  repo.save (argv[2]);

  LDAModel ldaModel (&repo.corpus(), &repo.dict());
  ldaModel.train();
  ldaModel.save(argv[2], "similarity");
  return 0;
}
