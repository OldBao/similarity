#ifndef TRAINER_SERVER_H_
#define TRAINER_SERVER_H_

#include <fstream>

namespace sm {
  typedef ub::SmartEvent<TrainerServerEvent> TrainerServerEventPtr;
  class TrainerData {
  private:
    uint64_t _version;
    Repository *_repo;
    TopicModel *_model;
  public:
    TrainerData(uint64_t version, Repository *repo);
    ~TrainerData();

    int train(std::istream &is);
    int uploadToHdfs();
  };

  class TrainerServerEvent : public ub::NsheadEvent {
  public:
    virtual void read_done_callback();
    void event_error_callback();
  };

  class TrainerServer : public UbAServer {
  public:
    TrainerServer();
    void on_accept (ub::UbEvent *event);
    
  };

  
};
#endif
