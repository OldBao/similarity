#ifndef TRAINER_SERVER_H_
#define TRAINER_SERVER_H_
#include <string>
#include <fstream>

#include "HttpSvrEvent.h"
#include "UbAServer.h"
#include "event_log.h"
#include "concurrent.h"
#include "log.h"
#include "repo.h"


namespace sm {

  class TrainData : public Thread<void> {
  private:
    uint64_t _version;
    Repository *_repo;
    TopicModel *_model;

  public:
    TrainData(uint64_t version, TrainServer *server);
    ~TrainData();

    virtual int doJob(void);
    int addUrlFromIStream(istream &is);
    int train();
    int uploadToHdfs();
  };

  class TrainServerEvent : public ub::NsheadEvent {
  public:
    virtual void read_done_callback();
    void event_error_callback();
  };

  class TrainServer : public UbAServer {
  public:
    TrainServer();
    void on_accept (ub::UbEvent *event);
    void onTrainJobDone(uint64_t version, int status);

  private:
    std::map <uint64_t, TrainData *> _datas;
    uint64_t _current_version;
    RWLock _dataLock;
  };

typedef ub::SmartEvent<TrainServerEvent> TrainServerEventPtr;

  
};
#endif
