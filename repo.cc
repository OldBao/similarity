#include <sstream>
#include <iostream>
#include <json/json.h>
#include <ul_sign.h>
#include "repo.h"
#include "log.h"
#include "encoding.h"

using namespace std;
using namespace sm;

Repository::Repository(const std::string &local, 
                       const std::string &mola_conf_path, 
                       const std::string &mola_conf_file, 
                       int nworkers)
  : _localpath(local), _mola_path(mola_conf_path), _mola_file (mola_conf_file)
{
  SM_ASSERT (nworkers < 12 * 1.5, "i suggest you to open at most [%d] workers", 12*1.5);

  for (int i = 0; i < nworkers; i++) {
    RepositoryWorker *worker = new RepositoryWorker(this);
    _workers.push_back(worker);
    worker->start();
  }
}

Repository::~Repository(){
  for (int i = 0; i < _workers.size(); i++) {
    delete _workers[i];
  }
}


int
Repository::open(){
  return 0;
}


void
Repository::waitAllJobDone(){
  for (vector<RepositoryWorker *>::iterator iter = _workers.begin();
       iter != _workers.end();
       iter++)
    {
      (*iter)->waitAllJobDone();
    }

}

int
Repository::addUrl (const std::string& url) {
  assert (0);
}

int
Repository::addUrls (const vector<string> &urls) {
  for (vector<string>::const_iterator iter = urls.begin();
       iter != urls.end();
       iter++)
    {
      _workers[random() % _workers.size()]->addJob(*iter + "|topic");
    }

}


int
Repository::doJob(const string &values) {
  json_tokener *tokener = json_tokener_new();
  const char *content,  *title, *docid;

  json_object *obj = json_tokener_parse_ex (tokener, values.c_str(), values.size());
  if (!obj) {
    SM_LOG_NOTICE ("json parse error : %s in %d", 
                   json_tokener_errors[tokener->err],
                   tokener->char_offset);
    return -1;
  }

  json_object_object_foreach (obj, key, value) {
    if (!strcmp (key, "ariticle")) {
      content = json_object_get_string (value);
    } else if (!strcmp (key, "docid")) {
      docid = json_object_get_string (value);
    } else {
      title = json_object_get_string (value);
    }
  }

  Document doc(content, title, docid);
  if (0 != document.analysis(ACCEPT)) {
    SM_LOG_NOTICE ("analysis error");
    return -1;
  }

  bow_t bow;
  _dict.doc2bow ()

  return 0;
}

uint64_t
Repository::_sign_doc (const std::string &doc) {
  uint64_t sign = 0;
  creat_sign_fs64((char *)doc.c_str(), (int) doc.size(), 
                  (uint32_t *) (&sign), (uint32_t *) (&sign) + 1);
  return sign;
}


RepositoryWorker::RepositoryWorker(Repository *repo) : _repo(repo){
  client.open();
}

RepositoryWorker::~RepositoryWorker(){

}

int
RepositoryWorker::doJob (const string &job) {
  string content;
  if ( 0 == client.get(job, &content) ) {
    _repo->doJob(content);
  } else {
    SM_LOG_WARNING ("get content %s error", job.c_str());
  }
  
  return 0;
}

