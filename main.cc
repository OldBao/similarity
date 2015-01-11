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
  /*
  cout << "url : " << *url << endl;
  cout << "title : " << *title << endl;
  cout << "content : " << *content << endl;
*/
}


int
main(int argc, char **argv){
  if (argc < 2) {
    cout << "usage: ./train [build|train|show]" << endl;
  }

  string mode(argv[1]);

  ul_logstat_t logstat = {16, 0, UL_LOGTTY};
  ul_openlog("./log", "train", &logstat, 1024);

  Dictionary dict;
  Corpus corpus;

  fstream mycorpus("mycorpus.txt", ios::out);

  if (mode == "build") {
    if (argc < 6) {
      cout << "usage : ./train build worddict postagdict stopwords dir outputdir [max_url_count]" << endl; 
      cout << "egg : ./train worddict postag stopwords.utf8 gallery model 50" << endl;
      return -1;
    }
    int max = 10;
    if (argc == 8) {
      max = atoi(argv[7]);
    }

    if ( 0 != Segment::getInstance()->load (argv[2], argv[3], argv[4])){
      cout << "init segment unit error!" << endl;
      return -1;
    }

    DIR *dir = opendir(argv[5]);
    if (!dir) {
      cout << "opendir error" << endl;
      return -1;
    }

    int fail = 0, success = 0;
     
    while (corpus.size() <= max) {
      struct dirent *entry = readdir(dir);

      if (!entry) break;
      if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) continue;
      cout << "Processing : " << entry->d_name << endl; 
      
      string content, title, url;
      string path(argv[5]);
      path += "/" ;
      path += entry->d_name;
      get_content(path, &url, &title, &content);
      Document document(content, title, "utf8");
      
      if ( 0 != document.analysis(accept)){
        cout << "analysis document error" << endl;
        fail++;
        continue;
      }
      
      bow_t bow;
      dict.doc2bow(&bow, document, true);
      bow.sort();
      corpus.addDoc(bow);
      
      success++;
      mycorpus << bow.size() << " ";
      for (int i = 0; i < bow.size(); i++) {
        mycorpus << bow[i].id << ":" << bow[i].weight << " ";
      }
      mycorpus << endl;
    }

    cout << "Handle Success: " << success << " Failed : " << fail << endl;

    mycorpus.close();

  
    dict.save (argv[6], "sim");
    corpus.save (argv[6], "sim");

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
  }
  
  else if (mode == "train") {
    if (argc != 3) {
        cout << "usage : ./train train modeldir" << endl;
        cout << "eg : ./train train model" << endl;
        return -1;
    }
    if ( 0 != dict.load(argv[2], "sim")){
        cout << "load dict error" << endl;
        return -1;
    }

    if ( 0 != corpus.load(argv[2], "sim")){
        cout << "load model error" << endl;
        return -1;
    }
    LDAModel *model = new LDAModel(&corpus, NULL);
    model->train();
    model->save(argv[2], "lda");
  }

  else if (mode == "show") {
    int nwords;
    if (argc < 4) {
        cout << "usage : ./train show modeldir" << endl;
        cout << "eg : ./train show model" << endl;
        return -1;
    }

    if (argc == 4) {
      nwords = atoi(argv[3]);
    }
    if ( 0 != dict.load(argv[2], "sim") ) {
      cout << "load dict error" << endl;
      return -1;
    }
    if ( 0 != corpus.load(argv[2], "sim")) {
      cout << "load corpus error" << endl;
      return -1;
    }
   

    LDAModel *model = new LDAModel(&corpus, &dict);

    if (-1 == model->load(argv[2], "lda")){
      return -1;
    }

    for (int i = 1; i <= 100; i++) {
      string ret;
      model->getHotestWordsDesc(&ret, i, nwords);
      cout << ret << endl;
    }
    delete model;
  } else {
    cout << "invalid mode" << endl;
  }

  return 0;
}
