#include <inttypes.h>
#include "trainer_server.h"
#include "interface/trainer.pb.h"

using namespace std;
using namespace sm;

TrainData::TrainData (uint64_t version, TrainServer *server) :
  _version(version), _repo(NULL), _model (NULL), _server(server)
{
  _repo = new Repository ();

}

TrainData::~TrainData() {
  if (_model) {
    delete _model;
    _model = NULL;
  }
  if (_repo) {
    delete _repo;
    _repo = NULL;
  }
}

int
TrainData::addUrlFromIStream(istream &is){
  string url;

  while (!is.eof()) {
    is >> url;
    if (url.empty()) continue;
    SM_LOG_NOTICE ("add %s to repo", url.c_str());
    _repo->addUrl(url);
  }
  return 0;
}

int
TrainData::doJob(const bool &) {
  int ret = train();
  if (ret != 0) {
    _server->onTrainJobDone(_version, ret);
  }
  else {
    ret = uploadToHdfs();
    _server->onTrainJobDone(_version, ret);
  }
  return 0;
}

int
TrainData::train(){
  SM_ASSERT (!_model, "needn't train twice, is wasteful");

  _model = new LDAModel (&_repo->corpus(), &_repo->dict(), _version);
  SM_LOG_NOTICE ("begin train lda, version %" PRIu64 ", topics : %d, terms:%d", 
                 _version ,_model->getNTopics(), _repo->corpus().getNTerms());

  if (0 != _model->train()) {
    SM_LOG_WARNING ("train lda error");
    return -1;
  }

  return _model->save (_server->getModelPath(), "similarity");
}



void
TrainServerEvent::read_done_callback(){
  nshead_t *req_head = (nshead_t *) this->get_read_buffer();
  char *reqbuf = (char *)req_head+1;
  
  smpb::TrainRequest req;
  smpb::TrainResponse res;
  TrainServer *server = (TrainServer *) _fserver;

  string raw;
  raw.assign (reqbuf, req_head->body_len);
  if (!req.ParseFromString(raw)){
    res.set_ret(smpb::TrainResponse::INVALID);
  } else {
    if (req.cmd() == smpb::TrainRequest::CHECK_VERSION) {
      res.set_ret(smpb::TrainResponse::OK);
      res.set_version(server->getCurrentVersion());
    } else if (req.cmd() == smpb::TrainRequest::UPDATE) {
      if (req.files_size() == 0) {
        res.set_ret(smpb::TrainResponse::INVALID);
      } else {
        vector<string> urlfiles;
        for (int i = 0; i < req.files_size(); i++) urlfiles.push_back(req.files(i));
        uint64_t newid = server->addNewTrainingJob(urlfiles);
        if (newid != (uint64_t) -1) {
          res.set_ret(smpb::TrainResponse::INTERNAL);
        } else {
          res.set_ret(smpb::TrainResponse::OK);
          res.set_version(newid);
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
  else new_version = _datas.rbegin()->first;
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
      ifstream is(iter->c_str());
      if (!is.is_open()) {
        SM_LOG_WARNING ("open %s file fails", iter->c_str());
        return -1;
      } else {
        if ( 0 != data->addUrlFromIStream(is)){
          return -1;
        }
      }
    }

  data->addJob(true);
  return 0;
}

void
TrainServer::onTrainJobDone(uint64_t version, int status) {
  if (status == 0) {
    SM_LOG_NOTICE ("new version %" PRIu64 "done, trasfering [%"PRIu64"=>%"PRIu64"]",
                   _current_version,version);
    _dataLock.AcquireWrite();
    TrainData *data = _datas[_current_version];
    _datas.erase(_current_version);
    delete data;

    if (version > _current_version) {
      _current_version = version;
    }
    _dataLock.Release();
  } else {
    _dataLock.AcquireWrite();
    TrainData *data = _datas[version];
    _datas.erase(_current_version);
    _dataLock.Release();
  }
}


void
TrainServer::on_accept(ub::UbEvent *event) {
  TrainServerEventPtr mev;
  session_begin(&mev);
}
