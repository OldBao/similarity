#ifndef SIM_SERVER_H_
#define SIM_SERVER_H_

#include <stdint.h>
#include <string>

#include "HttpSvrEvent.h"
#include "UbAServer.h"
#include "event_log.h"

#include "singleton.h"
#include "dictionary.h"
#include "corpus.h"
#include "model.h"
#include "similarity.h"

namespace sm {
  class SimServerData {
  private:
    uint64_t _version;
    Corpus* _corpus;
    Dictionary *_dict;
    TopicModel *_model;
    TopicSimilarity *_sim;

  public :
    SimServerData(uint64_t version);
    int load(const std::string& path);
    ~SimServerData();

    int getSimilarity(bow_t *bow, uint64_t docid, float threshold, int max_result);
  };


  class SimServerEvent : public ub::HttpSvrEvent {
  public:
    virtual void read_done_callback();
    void session_begin();
    void http_read_header_done();
    void http_read_section_done();
    void event_error_callback();
  };


  class SimServer : public ub::UbAServer {
  public:
    SimServer(ub::NetReactor *reactor);
    virtual ~SimServer();

    int updateServerData (SimServerData *data);
    uint64_t getCurrentDataVersion();
    int getSimilarities (bow_t *bow, uint64_t docid, float threshold=0.5, int max_result=50);
    void on_accept (ub::UbEvent *event);
  private:
    SimServerData *_server_data;
  };

  class SimServerDataManager {
    SM_DECLARE_SINGLETON (SimServerDataManager);

  public:
    int init(const std::string &basepath);
    int registerSimServer(SimServer *server);

    int check_version();
    int force_update (uint64_t version = -1); //update to newest

  private:
    std::map<uint64_t, SimServerData *> _datas;
    std::vector<SimServer *> _servers;
  };

};
#endif
