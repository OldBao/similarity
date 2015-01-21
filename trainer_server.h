#ifndef TRAINER_SERVER_H_
#define TRAINER_SERVER_H_
#include <string>
#include <fstream>

#include "NsheadSvrEvent.h"
#include "UbAServer.h"
#include "event_log.h"
#include "concurrent.h"
#include "log.h"
#include "repo.h"
#include "model.h"


namespace sm {
  class TrainServer;

  class TrainData : public Thread<bool> {
  private:
    uint64_t _version;
    Repository *_repo;
    TopicModel *_model;
    TrainServer *_server;

  public:
    TrainData(uint64_t version, TrainServer *server);
    ~TrainData();

    virtual int doJob(const bool &);
    int addUrlFromIStream(istream &is);
    int train();
    int uploadToHdfs();
  };

  class TrainServerEvent : public ub::NsheadSvrEvent {
  public:
    virtual void read_done_callback();
    void event_error_callback();
  };

  class TrainServer : public ub::UbAServer {
  public:
    TrainServer();
    void on_accept (ub::UbEvent *event);
    void onTrainJobDone(uint64_t version, int status);
    const std::string getModelPath() {return _path;}
    uint64_t getCurrentVersion() {return _current_version; }
    uint64_t addNewTrainingJob (std::vector<std::string> files);
  private:
    string _path;
    std::map <uint64_t, TrainData *> _datas;
    uint64_t _current_version;
    RWLock _dataLock;
  };

typedef ub::SmartEvent<TrainServerEvent> TrainServerEventPtr;

  
};
#endif
