#include <inttypes.h>
#include <fstream>
#include <iostream>
#include <json/json.h>
#include <ul_sign.h>
#include "repo.h"
#include "log.h"
#include "encoding.h"
#include "model.h"
#include "configurable.h"

using namespace std;
using namespace sm;

static uint64_t _sign_doc (const std::string &doc) {
  uint64_t sign = 0;
  creat_sign_fs64( (char *)doc.data(), (int) doc.size(),
                   (uint32_t *) (&sign), (uint32_t *) (&sign) + 1);
  return sign;
}



Repository::Repository()
{
  SM_CONFIG_BEGIN(GLOBAL)
  SM_CONFIG_PROP_STR(model_path, "model");
  SM_CONFIG_END

  SM_CONFIG_BEGIN(REPO)
  SM_CONFIG_PROP_STR(cache_path, "doccache");
  SM_CONFIG_PROP_STR(corpus_name, "similarity");
  SM_CONFIG_PROP_STR(dict_name, "similarity");
  SM_CONFIG_PROP(nworker, int32, 12);
  SM_CONFIG_END

  for (int i = 0; i < _nworker; i++) {
    RepositoryWorker *worker = new RepositoryWorker(this);
    _workers.push_back(worker);
    worker->start();
  }
}

Repository::~Repository(){
  for (size_t i = 0; i < _workers.size(); i++) {
    delete _workers[i];
  }
}


void
Repository::waitAllJobDone(){
  for (vector<RepositoryWorker *>::iterator iter = _workers.begin();
       iter != _workers.end();
       iter++)
    {
      (*iter)->waitAllJobDone();
      (*iter)->stop();
    }

    
}

int
Repository::addUrl (const std::string& url) {
  _workers[random() % _workers.size()]->addJob(url + "|topic");
  return 0;
}


/*
int
Repository::addUrls (const vector<string> &urls) {
  for (vector<string>::const_iterator iter = urls.begin();
       iter != urls.end();
       iter++)
    {
      _workers[random() % _workers.size()]->addJob(*iter + "|topic");
    }

  return 0;
}

int
Repository::doJob (const string &values) {
  Json::Reader reader;
  Json::Value artObj;
  vector< pair<string, double> > doc;
  uint64_t docid;
  bow_t bow;

  if (!reader.parse (values, artObj)) {
    SM_LOG_NOTICE ("json parse error : %s in %d", 
                   json_tokener_errors[tokener->err],
                   tokener->char_offset);
    return -1;
  }

  }

  if (0 != _dict.addRawDoc(&bow, doc)) {
    SM_LOG_WARNING ("add raw doc error");
    goto error;
  }

  bow.pre_handle();
  _corpus.addDoc(docid, bow);
  
  json_tokener_free(tokener);
  return 0;

 error:
  json_tokener_free(tokener);
  return -1;
}
*/

int
Repository::doJob(const string &raw) {
  bow_t bow;

  Document doc;
  if (0 != doc.parseFromJsonRaw(raw)){
    SM_LOG_NOTICE ("get document content from json error");
    return -1;
  }

  if (0 != doc.analysis()) {
    SM_LOG_NOTICE ("analysis error");
    return -1;
  }

  _dict.doc2bow (&bow, doc, true);
  bow.pre_handle();
  if (bow.size() == 0) {
    SM_LOG_DEBUG ("didn't get any words in %lu", doc.getId());
    return -1;
  }

  return _corpus.addDoc(doc.getId(), bow);
}


RepositoryWorker::RepositoryWorker(Repository *repo) : _repo(repo){
}

RepositoryWorker::~RepositoryWorker(){

}

int
RepositoryWorker::doJob (const string &job) {
  string content;

  uint64_t docid = _sign_doc (job);
  if (0 != _read_from_local(docid, &content))  {
    if ( 0 != client.get(job, &content) )  {
      SM_LOG_WARNING ("get content %s error", job.c_str());
      return -1;
    } else {
       _save_to_local(docid, content); 
    }
  }


  if (content.empty()) {
    SM_LOG_NOTICE ("get empty content from url [%s]", job.c_str());
    return -1;
  }

  return _repo->doJob(content);
}

int
Repository::tfidf (){
  int ret;

  TFIDFModel model (&_corpus, &_dict);
  ret = model.train();
  if (ret != 0) {
    SM_LOG_WARNING ("train idf fail");
    return -1;
  }

  ret = model.inference(_corpus, &_tfidf);
  if (ret != 0) {
    SM_LOG_WARNING ("train idf fail");
    return -1;
  }

  return 0;
  
}


int
Repository::save() {
  int ret;
  ret = _dict.save (_dict_name);
  if (0 != ret) {
    SM_LOG_WARNING("save dictionary error");
    return -1;
  }
  
  ret = _corpus.save(_corpus_name);
  
  if (0 != ret) {
    SM_LOG_WARNING ("save corpus error");
    return -1;
  }

  return 0;
}


int
RepositoryWorker::_read_from_local(uint64_t id, string *content){
  if (_repo->getLocalCachePath().empty()) return -1;
  char fullpath[PATH_MAX];
  snprintf (fullpath, PATH_MAX, "%s/%" PRIu64, _repo->getLocalCachePath().c_str(), id);

  struct stat statbuf;
  if (-1 == stat (fullpath, &statbuf)){
    return -1;
  }

  if (S_ISREG(statbuf.st_mode)) {
    ifstream is(fullpath);
    if (is.is_open()) {
      is.seekg(0, ios::end);
      ifstream::pos_type pos = is.tellg();
      char *buffer = (char *) malloc (pos);
      bzero (buffer, pos);
      is.seekg(0, ios::beg);
      is.read (buffer, pos);
      content->assign (buffer, pos);
      free (buffer);
      return 0;
    }
  }

  return -1;
}

int
RepositoryWorker::_save_to_local(uint64_t id, const string& content) {
  if (_repo->getLocalCachePath().empty()) return -1;

  char fullpath[PATH_MAX];
  snprintf (fullpath, PATH_MAX, "%s/%" PRIu64, _repo->getLocalCachePath().c_str(), id);

  ofstream os(fullpath);
  if (!os.is_open()) return -1;

  os.write(content.data(), content.size());

  return 0;
}
