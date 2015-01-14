#include <iostream>
#include <fstream>
#include "repo.h"

using namespace std;
using namespace sm;

int
main(int argc, char **argv){
  if (argc != 2) {
    cout << "./server url" << endl;
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
    cout << "open repo error" << endl;
    return -1;
  }

  vector<string> urls;

  for (i = 0; i < 1; i++) {
    string url;
    is >> url;
    urls.push_back (url);
  }

  repo.addUrls(urls);

  repo.waitAllJobDone();
}
