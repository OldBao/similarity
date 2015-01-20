#include "trainer_server.h"

using namespace std;
using namespace sm;

TrainData::TrainData (uint64_t version, TrainerServer *server) :
  _version(version), _repo(NULL), _model (NULL), _server(server)
{
  repo = new Repository ();

}

TrainData::~TrainData() {
  if (_model) {
    delete _model;
    _model = NULL;
  }
}

int
addUrlFromIStream(istream &is){
  string url;
  if (!is.is_open()) {
    SM_LOG_WARNING ("invalid istream!");
    return -1;
  }

  while (!is.eof()) {
    is >> url;
    if (url.empty()) continue
    SM_LOG_NOTICE ("add %s to repo", url.c_str());
    _repo->addUrl(url);
  }
  return 0;
}

int
doJob(int) {
  int ret = train();
  if (ret != 0) {
    onTrainJobDone(_version, ret);
  }
  else {
    ret = uploadToHdfs();
    onTrainJobDone(_version, ret);
  }
  return 0;
}

int
train(){
  SM_ASSERT (!_model, "needn't train twice, is wasteful");

  _model = new LDAModel (_repo->corpus(), _repo->dict(), _version);
  SM_LOG_NOTICE ("begin train lda, version %" PRIu64 ", topics : %d, terms:%d", 
                 _model->getNTopics(), _repo->corpus().getNTerms());

  if (0 != ldaModel.train()) {
    SM_LOG_WARNING ("train lda error");
    return -1;
  }

  return ldaModel.save (_model_path, "similarity");
}


int
TrainData::~TrainData(){
  if (_model)
    delete _model;
}


void
TrainServerEvent::read_done_callback(){
  nshead_t *req_head = (nshead_t *) this->get_read_buffer();
  char *reqbuf = (char *)req_head+1;
  
  smpb::TrainReuqest req;
  smpb::TrainResponse res;
  TrainServer *server = (TrainServer *) _fserver;

  if (!request.parseFromString(reqbuf, req_head->body_len)){
    res.ret = INVALID;
  } else {
    if (request.cmd == CHECK_VERSION) {
      res.ret = OK;
      res.version = server->getCurrentVersion();
    } else if (request.cmd == UPDATE) {
      if (request.files_size() == 0) {
        res.ret = INVALID;
      } else {
        vector<string> urlfiles;
        for (int i = 0; i < request.files_size(); i++) urlfiles.push_back(request.files(i));
        uint64_t newid = server->addNewTrainingJob(urlfiles);
        if (newid != (uint64) -1) {
          res.ret = INTERNAL;
        } else {
          res.ret = OK;
          res.newid = newid;
        }
      }
    }
  }
}


uint64_t
TrainServer::addNewTrainingJob (vector<string> files) {
  uint64_t new_version;
  _dataLock.AcquireRead();
  if (_datas.empty()) new_version = 0;
  else new_version = _datas.rbegin()->first();
  _dataLock.Release();
  
  TrainData *data = new TrainData(new_version, this);
  _dataLock.AcquireWrite();
  _datas[new_version] = data;
  _dataLock.Release();

  vector<istream> iss;
  for (vector<string>::iterator iter = files.begin();
       iter != files.end();
       iter++)
    {
      ifstream is(*iter, ios::in);
      if (!is.is_open()) {
        SM_LOG_WARNING ("open %s file fails", iter->c_str());
        return -1;
      } else {
        if ( 0 != data->addUrlFromStream(is)){
          return -1
        }
      }
    }

  data->addJob();
  return 0;
}

void
TrainServer::onTrainJobDone(unint64_t version, int status) {
  if (status == 0) {
    SM_LOG_NOTICE ("new version %" PRIu64 "done, trasfering [%"PRIu64"=>%"PRIu64"]",
                   _current_version,version);
    _dataLock.AcquireWrite();
    TrainData *data = _datas[_current_version];
    _datas.remove(_current_verion);
    delete data;

    if (version > _current_version) {
      _current_version = version;
    }
    _dataLock.Release();
  } else {
    _dataLock.AcquireWrite();
    TrainData *data = _datas[version];
    _datas.remove(_current_version);
    _dataLock.Release();
  }
}


void
on_accpet(ub::UbEvent __attribute__((unused))) {
  MyNsheadEventPtr mev;
  session_begin(&mev);
}
