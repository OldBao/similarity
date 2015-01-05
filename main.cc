#include "segment.h"
#include "dictionary.h"
#include <fstream>
#include <tagdict.h>
#include <stdio.h>
using namespace sm;
using namespace std;

uint64_t accept = SM_TOKEN_TYPE_DEFAULT | SM_TOKEN_TYPE_VERB | SM_TOKEN_TYPE_VD | SM_TOKEN_TYPE_VN;

int
main(int argc, char **argv){
  if (argc != 4) {
    cout << "usage : ./train worddict postagdict file" << endl; 
    return -1;
  } 

  ul_logstat_t logstat = {16, 0, UL_LOGTTY};
  ul_openlog("./log", "train", &logstat, 1024);

  if ( 0 != Segment::getInstance()->load (argv[1], argv[2])){
    cout << "init segment unit error!" << endl;
    return -1;
  }

  ifstream  fs;
  string url, title, content;
  fs.open (argv[3]);
  
  fs>>url;     
  fs>>title;

  stringstream buffer;
  buffer << fs.rdbuf();
  
  content.assign(buffer.str());

  Document document(content, title, "utf8");

  if ( 0 != document.analysis(accept)){
    cout << "analysis document error" << endl;
    return -1;
  }

  Dictionary dict;
  dict.addDocument(document);

  /*
  cout << "Get " << document.getTokens().size() << " Tokens" << endl;
  for (vector<Token>::const_iterator iter = document.getTokens().begin();
       iter != document.getTokens().end();
       iter++)
    {
      cout << iter->toString() << endl;
    }
  */

  return 0;
}
