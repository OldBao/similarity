#ifndef _SM_REPO_H_
#define _SM_REPO_H_

#include <vector>
#include <string>

#include <sys/types.h>
#include <map>
#include "concurrent.h"
#include "dictionary.h"
#include "kvproxy_client.h"
#include "corpus.h"

namespace sm {
  class Repository;
  typedef int (ItemCallback)(const std::string &item, const std::string& content, void *context);

  class RepositoryWorker : public Thread<std::string> {
  public:
    RepositoryWorker(Repository *repo);
    virtual ~RepositoryWorker();
    virtual int doJob(const std::string &job);

  private:
    Repository* _repo;
    KvProxyClient client;
  };

  class Repository {
  public:
    Repository(const std::string &local, 
               const std::string &mola_conf_path, 
               const std::string &mola_conf_file,
               int nworkers = 12);

    virtual ~Repository();

    int open ();
    int addUrl (const std::string& url);
    int addUrls (const std::vector<std::string> &urls);
    int registerItemHandler(ItemCallback);
    int size();
    int doJob(const std::string &doc);
    
    void waitAllJobDone();
    
    int tfidf ();
    int save(const std::string &basepath);
   private:
    std::string _localpath, _mola_path, _mola_file;
    uint64_t _sign_doc(const std::string &doc);
    std::map<int, uint64_t> _docmap;

    std::vector<RepositoryWorker *> _workers;

    Lock _docmapLock;
    Dictionary _dict;
    Corpus _corpus, _tfidf;
  };
};
#endif
