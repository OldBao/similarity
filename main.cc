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
     
    char path[PATH_MAX];
    snprintf (path, PATH_MAX, "%s/docmap", argv[6]);
    FILE *docmap = fopen(path, "w");
    if (!docmap) {
        cout << "open doc map error" << endl;
        return -1;
    }

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
      Document document(content, title, entry->d_name, "utf8");
      
      if ( 0 != document.analysis(accept)){
        cout << "analysis document error" << endl;
        fail++;
        continue;
      }
      
      bow_t bow;
      dict.doc2bow(&bow, document, true);
      bow.pre_handle();
      if (bow.size() == 0) continue;
      corpus.addDoc(bow);
      
      success++;
      mycorpus << bow.size() << " ";
      for (int i = 0; i < bow.size(); i++) {
        mycorpus << bow[i].id << ":" << bow[i].weight << " ";
      }
      mycorpus << endl;

      fprintf (docmap, "%s %zu\n", entry->d_name, corpus.size()-1);
    }

    cout << "Handle Success: " << success << " Failed : " << fail << endl;

    mycorpus.close();

    dict.save (argv[6], "sim");
    Corpus tfidf_corpus;
    TFIDFModel model(&corpus, &dict);
    cout << "begin computing tfidf" << endl;
    model.train();
    model.inference(corpus, &tfidf_corpus, true);
    cout << "end computing tfidf" << endl;
    tfidf_corpus.save(argv[6], "tfidf");
    corpus.save (argv[6], "sim");
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

    if ( 0 != corpus.load(argv[2], "tfidf")){
        cout << "load model error" << endl;
        return -1;
    }

    LDAModel *model = new LDAModel(&corpus, NULL);
    model->train();
    model->save(argv[2], "lda");
  }

  else if (mode == "show") {
    int nwords = 5;
    if (argc < 3) {
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
    
    
    char fullpath[PATH_MAX];
    snprintf (fullpath, PATH_MAX, "%s/topwords", argv[2]);
    fstream tw(fullpath, ios::out);
    for (int i = 1; i <= model->getNTopics(); i++) {    
      string ret;
      model->getHotestWordsDesc (&ret, i, nwords);
      tw << "Topic [" << i <<"]" << ret << endl;
    }


    snprintf (fullpath, PATH_MAX, "%s/docmap", argv[2]);
    fstream docfile(fullpath, ios::in);
    map<int, string> docmap;
    while (!docfile.eof()) {
      int id; string docid;
      docfile >> id >>  docid;
      docmap[id] = docid;
    }
    tw.close();
    
    FILE **fps = (FILE **)malloc (sizeof (FILE *) * model->getNTopics());

    for (int i = 1; i <= model->getNTopics(); i++) {
      snprintf (fullpath, PATH_MAX, "%s/details/%d", argv[2], i);
      fps[i] = fopen(fullpath, "r");
    }

    for (int i = 0; i < corpus.size(); i++) {
      bow_t ret;
      cout << "computing " << i << " ";
      model->getMostLikelyTopicOfDoc (&ret, i, 0.0, 1);
      cout << "likely topic " << ret[0].id << " likelihood: " << ret[0].weight;

      if (ret.size() > 0) {
        fprintf (fps[ret[0].id], "%s\n", docmap[i].c_str());
      }
    }
    
    delete model;
  }else {
    cout << "invalid mode" << endl;
  }

  return 0;
}
