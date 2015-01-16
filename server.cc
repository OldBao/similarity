
#include <iostream>
#include <fstream>
#include <signal.h>
#include "repo.h"
#include "segment.h"
using namespace std;
using namespace sm;

int
main(int argc, char **argv){
  signal(SIGPIPE, SIG_IGN);
  ul_logstat_t logstat;
  logstat.events = 4;
  logstat.spec= 0;
  ul_openlog("./log", "junk", &logstat, 1024);
  if (argc != 3) {
    cout << "./server url path" << endl;
    return 0;
  }

  fstream is(argv[1], ios::in);
  int i;

  if ( 0 != Segment::getInstance()->load ("worddict", "postag", "stopwords.utf8")){
    cout << "init segment error" << endl;
    return -1;
  }

  Repository repo("doccache", "conf/online", "mola.conf");
  if (0 != repo.open()){
    cout << "open repo error " << endl;
    return -1;
  }

  i = 0;
  while (1) {
    i++;
    if (i % 1000 == 0) {
      break;
      sleep (10);
    }
    string url;
    is >> url;
    if (is.eof()) break;
    repo.addUrl(url);
  }

  repo.waitAllJobDone();
  /*
  cout << "begin calculate tfidf" << endl;
  repo.tfidf();
  
  cout << "calculate done" << endl;
  */
  repo.save (argv[2]);

  return 0;
}
