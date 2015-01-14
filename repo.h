#ifndef _SM_REPO_H_
#define _SM_REPO_H_

#include <vector>
#include <string>

#include <sys/types.h>
#include <dirent.h>
#include "concurrent.h"
#include "mco/MolaEngine.h"

namespace sm {
  class Repository;
  typedef int (ItemCallback)(const std::string &item, const std::string& content, void *context);

  class RepositoryWorker : public Thread< std::vector<baidu::mco::Slice>* > {
  public:
    RepositoryWorker(Repository *repo);
    virtual ~RepositoryWorker();
    virtual int doJob(std::vector<baidu::mco::Slice>* const &job);

  private:
    Repository* _repo;
  };

  class Repository {
  public:
    Repository(const std::string &local, 
               const std::string &mola_conf_path, 
               const std::string &mola_conf_file,
               int nworkers = 5);

    virtual ~Repository();

    int open ();
    int addUrl (const std::string& url);
    int addUrls (const std::vector<std::string> &urls);
    int registerItemHandler(ItemCallback);
    int size();
    int doJob(const std::vector<baidu::mco::Slice> &docs);
    
    void waitAllJobDone();
   private:
    std::string _localpath, _mola_path, _mola_file;
    uint64_t _sign_doc(const std::string &doc);
    std::map<int64_t, std::string> _docmap;
    DIR *_dir;
    baidu::mco::MolaEngine _engine;

    std::vector<RepositoryWorker *> _workers;
  };
};
#endif
