#include <inttypes.h>
#include <fstream>
#include <iostream>
#include <json/json.h>
#include <ul_sign.h>
#include "repo.h"
#include "log.h"
#include "encoding.h"
#include "model.h"

using namespace std;
using namespace sm;

const uint64_t ACCEPT = SM_TOKEN_TYPE_DEFAULT | SM_TOKEN_TYPE_VERB | SM_TOKEN_TYPE_VD | SM_TOKEN_TYPE_VN;

static uint64_t _sign_doc (const std::string &doc) {
  uint64_t sign = 0;
  creat_sign_fs64( (char *)doc.data(), (int) doc.size(),
                   (uint32_t *) (&sign), (uint32_t *) (&sign) + 1);
  return sign;
}



Repository::Repository(int nworkers, const std::string &local)
  : _localpath(local)
{
  SM_ASSERT (nworkers < 12 * 1.5, "i suggest you to open at most [%lf] workers", 12*1.5);

  for (int i = 0; i < nworkers; i++) {
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
    }

}

int
Repository::addUrl (const std::string& url) {
  _workers[random() % _workers.size()]->addJob(url + "|topic");
  return 0;
}

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

/*
int
Repository::doJob (const string &values) {
  json_tokener *tokener = json_tokener_new();
  json_object *obj = json_tokener_parse_ex (tokener, values.c_str(), values.size());
  vector< pair<string, double> > doc;
  uint64_t docid;
  bow_t bow;

  if (!obj || !json_object_is_type(obj, json_type_object)) {
    SM_LOG_NOTICE ("json parse error : %s in %d", 
                   json_tokener_errors[tokener->err],
                   tokener->char_offset);
    goto error;
  }


  json_object_object_foreach (obj, key, value) {
    if (!strcmp (key, "atomic_keyword")) {
      if (!json_object_is_type (value, json_type_array)) {
        SM_LOG_WARNING ("invalid type of atomic keyword");
        goto error;
      }
      array_list *arr = json_object_get_array(value);
      for (int i = 0; i < array_list_length(arr); i++) {
        json_object *kw = (json_object *) array_list_get_idx(arr, i);
        json_object_object_foreach (kw, awkey, awvalue) {
          pair<string, float> p;
          p.first = awkey;
          p.second = json_object_get_double(awvalue);
          doc.push_back (p);
        }
      }
    } else if (!strcmp(key, "docid")) {
      docid = strtoull(json_object_get_string(value), NULL, 10);
    }
  }

  if (0 != _dict.addRawDoc(&bow, doc)) {
    SM_LOG_WARNING ("add raw doc error");
    goto error;
  }

  if (bow.size() == 0) {
    SM_LOG_DEBUG ("didn't get any words in %lu", docid);
    return -2;
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
Repository::doJob(const string &values) {
  json_tokener *tokener = json_tokener_new();
  const char *content = NULL,  *title = NULL, *docidstr;
  bow_t bow;
  uint64_t docid = -1;
  int ret;
  
  json_object *obj = json_tokener_parse_ex (tokener, values.c_str(), values.size());
  if (!obj) {
    SM_LOG_NOTICE ("json parse error : %s in %d", 
                   json_tokener_errors[tokener->err],
                   tokener->char_offset);
    json_tokener_free(tokener);
    return -1;
  }

  json_object_object_foreach (obj, key, value) {
    if (!strcmp (key, "article")) {
      content = json_object_get_string (value);
    } else if (!strcmp (key, "docid")) {
      docidstr = json_object_get_string (value);
    } else if (!strcmp (key, "title")){
      title = json_object_get_string (value);
    }
  }


  if (!content || !title || !docidstr) {
     SM_LOG_WARNING ("can't get keys of content or title");
     json_tokener_free(tokener);
     return -1;
  }

  ret =  sscanf (docidstr, "%" PRIu64, &docid);
  if (ret != 1) {
     SM_LOG_WARNING ("docid invalid");
     json_tokener_free(tokener);
     return -1;
  }

  Document doc(content, title, docid);
  if (0 != doc.analysis(ACCEPT)) {
    SM_LOG_NOTICE ("analysis error");
    goto error;
  }


  _dict.doc2bow (&bow, doc, true);
  bow.pre_handle();
  if (bow.size() == 0) {
    SM_LOG_DEBUG ("didn't get any words in %lu: [contents:%s]", docid, values.c_str());
    goto error;
  }
  _corpus.addDoc(docid, bow);
  
  json_tokener_free(tokener);

  return 0;
 error:
  json_tokener_free(tokener);
  return -1;
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
Repository::save(const std::string &basepath) {
  int ret;
  ret = _dict.save (basepath, "similarity");
  if (0 != ret) {
    SM_LOG_WARNING("save dictionary error");
    return -1;
  }
  
  ret = _corpus.save(basepath, "similarity");
  
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
