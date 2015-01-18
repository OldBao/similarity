#include "trainer_server.h"

using namespace std;
using namespace sm;

TrainerData::TrainerData (uint64_t version, Repository *repo) :
  _version(version), _repo(repo), _model (NULL)
{
  _model = new LDAModel (repo.corpus, repo.dict());
}

TrainerData::~TrainerData() {
  if (_model) {
    delete _model;
    _model = NULL;
  }
}


int
train(istream &url_stream){
  string url;
  while (!url_stream.eof()) {
    url_stream >> url;
    _repo.addUrl(url);
  }
}
