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

uint64_t accept = SM_TOKEN_TYPE_DEFAULT | SM_TOKEN_TYPE_VERB | SM_TOKEN_TYPE_VD | SM_TOKEN_TYPE_VN;

static void get_content(const string &fn, string *url, string *title, string *content) {
  ifstream  fs;

  fs.open (fn.c_str());
  
  fs>>*url;     
  fs>>*title;

  stringstream buffer;
  buffer << fs.rdbuf();
  
  content->assign(buffer.str());

}

bool
mycmp(const pair<int, double>& a, const pair<int, double>& b){
  return a.second > b.second;
}

int
main(int argc, char **argv){
  if (argc != 4) {
    cout << "usage : ./train worddict postagdict dir" << endl; 
    return -1;
  } 

  ul_logstat_t logstat = {16, 0, UL_LOGTTY};
  ul_openlog("./log", "train", &logstat, 1024);

  if ( 0 != Segment::getInstance()->load (argv[1], argv[2])){
    cout << "init segment unit error!" << endl;
    return -1;
  }

  size_t max = 100000;

  DIR *dir = opendir(argv[3]);
  if (!dir) {
    cout << "opendir error" << endl;
    return -1;
  }

  Dictionary dict;
  Corpus corpus;
    
  while (corpus.size() <= max) {
    struct dirent *entry = readdir(dir);

    if (!entry) break;
    if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) continue;
    cout << "Processing : " << entry->d_name << endl; 

    string content, title, url;
    string path(argv[3]);
    path += "/" ;
    path += entry->d_name;
    get_content(path, &url, &content, &title);
    Document document(content, title, "utf8");

    if ( 0 != document.analysis(accept)){
      cout << "analysis document error" << endl;
      max+=1;
      continue;
    }

    bow_t bow;
    dict.doc2bow(&bow, document, true);
    corpus.addDoc(bow);
    /*
      cout << "Get " << document.getTokens().size() << " Tokens" << endl;
      for (vector<Token>::const_iterator iter = document.getTokens().begin();
      iter != document.getTokens().end();
      iter++)
      {
      cout << iter->toString() << endl;
      }
    */

    /*
      size_t i, j;
      for (i = 0; i < corpus.size(); i++) {
      const bow_t& b = corpus[i];
      
      for (j = 0; j < b.size(); j++) {
      const bow_unit_t &u =  b[j];
      cout << "[" << dict[u.id] << " : " << u.weight << "]" << endl;
      }
    }*/
  }
  /*
  TFIDFModel* model = new TFIDFModel(&corpus, &dict);
  model->train();
  
  int idx = 0;
  
  const vector<double>& v = model->idf();
 
  vector < pair<int, double> > m;
  m.resize(v.size());
  for (size_t i = 0; i < v.size(); i++) {
    m[i] = pair<int, double>(i, v[i]);
  }

  cout << "soring..." << endl;
  sort (m.begin(), m.end(), mycmp);
  cout << "sorted.." << endl;

  for (vector< pair<int, double> >::const_iterator iter = m.begin();
       iter != m.end();
       iter++)
    {
      cout << "[" << dict[iter->first] << " : " << iter->second << "]" << endl;
      idx++;
    }

  delete model;
  closedir(dir);
  */
  LDAModel *model = new LDAModel(&corpus, NULL);
  model->train();
  return 0;
}
