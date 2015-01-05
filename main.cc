#include "segment.h"
#include "dictionary.h"
#include <fstream>

using namespace sm;
using namespace std;


int
main(int argc, char **argv){
  if (argc != 4) {
    cout << "usage : ./train worddict postagdict file" << endl; 
    return -1;
  } 

  if ( 0 != Segment::getInstance()->load (argv[1], argv[2])){
    cout << "init segment unit error!" << endl;
    return -1;
  }

  int fd;
  ifstream  fs;
  string url, title, content;
  fs.open (argv[3]);
  
  fs>>url;     
  fs>>title;

  stringstream buffer;
  buffer << fs.rdbuf();
  
  content.assign(buffer.str());
  cout << "read line : " << content << endl;

  Document document(content, title);
  document.analysis();

  return 0;
  


}
