#include <sstream>
#include <iostream>
#include <ul_sign.h>
#include "repo.h"

using namespace baidu::mco;

using namespace std;
using namespace baidu::mco;
using namespace sm;

Repository::Repository(const std::string &local, 
                        const std::string &mola_conf_path, 
                       const std::string &mola_conf_file, 
                       int nworkers)
  : _localpath(local), _mola_path(mola_conf_path), _mola_file (mola_conf_file)
{

  assert (nworkers < 12 * 1.5);

  for (int i = 0; i < nworkers; i++) {
    RepositoryWorker *worker = new RepositoryWorker(this);
    _workers.push_back(worker);
    worker->start();
  }
}

Repository::~Repository(){
  if (_dir) closedir (_dir);
  
  for (int i = 0; i < _workers.size(); i++) {
    delete _workers[i];
  }
}
int
Repository::open(){
  if (!_engine.init (_mola_path, _mola_file) ){
    SM_LOG_WARNING ("open mola [%s/%s] file", _mola_path.c_str(), _mola_file.c_str());
    return -1;
  }

  _dir = opendir (_localpath.c_str());
  if (!_dir) {
    SM_LOG_WARNING ("open local path [%s] ", _localpath.c_str());
      return -1;
  }

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
  vector<KvIdKey> ids;
  //this content is so large, so i use heap to store
  vector<RetStatus> status;

  for (int i = 0; i < urls.size(); i++) {
    vector<Slice>* values = new vector<Slice>;
    ids.push_back(_sign_doc(urls[i]+"|topic"));
    if ( ids.size() % _workers.size()  == 0) {
      _engine.multiGet ((KvIdKey *)&ids[0],         
                        ids.size(), 
                        *values,
                        CbsTransPage,
                        status, 
                        (uint32_t) 16, 
                        "");

      for(int j = 0; j < ids.size(); j++) {
        switch (status[i].m_retcode_) {
        case retThroughputExceed:
          SM_LOG_WARNING ("Getted a very large packet, i will temporary ignore this url [%s] ", 
                          urls[i].c_str());
        case retFail:
          SM_LOG_WARNING ("get url [%s] failed", urls[i].c_str());
        case retNoHit:
          SM_LOG_WARNING ("can't geturl [%s]", urls[i].c_str());
          values->erase(values->begin()+i);
          break;
        case retSuccess:
          _workers[ random() % _workers.size() ]->addJob(values);
        default:
          SM_LOG_WARNING ("unknown error");
          break;
        }
      }
      ids.clear();
    }

  }
}

int
Repository::doJob(const vector<Slice> &values) {
  for (vector<Slice>::const_iterator iter = values.begin();
       iter != values.end();
       iter++)
    {
      
      Document document ()
      iter->data()
    }
}

uint64_t
Repository::_sign_doc (const std::string &doc) {
  uint64_t sign = 0;
  creat_sign_fs64((char *)doc.c_str(), (int) doc.size(), 
                  (uint32_t *) (&sign), (uint32_t *) (&sign) + 1);
  return sign;
}


RepositoryWorker::RepositoryWorker(Repository *repo) : _repo(repo){

}

RepositoryWorker::~RepositoryWorker(){

}

int
RepositoryWorker::doJob (vector<Slice>* const &job) {
  return _repo->doJob(*job);
}

