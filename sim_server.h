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
  typedef struct sim_unit_s {
    uint64_t docid;
    double sim;
  } sim_unit_t;

  typedef std::vector<sim_unit_t> sim_t;

  class SimServerData {
  private:
    uint64_t _version;
    Corpus* _corpus;
    Dictionary *_dict;
    TopicModel *_model;
    TopicSimilarity *_sim;

  public :
    SimServerData(uint64_t version);
    int load();
    uint64_t getVersion();
    ~SimServerData();

    int getSimilarity(sim_t *sim, uint64_t docid, float threshold, int max_result);
  };


  class SimServerEvent : public ub::HttpSvrEvent {
  public:
    virtual void read_done_callback();
    void session_begin();
    void http_read_header_done();
    void http_read_section_done();
    void event_error_callback();

  private:
    int _get_request(const std::string &req,
                     uint64_t *docid, float* threshold, int *max_result,
                     std::string *ret);

    void _sims_to_json(const sim_t &sims, std::string *ret);
  };


  class SimServer : public ub::UbAServer {
  public:
    SimServer(ub::NetReactor *reactor);
    virtual ~SimServer();

    int updateServerData (SimServerData *data);
    uint64_t getCurrentDataVersion();

    int getSimilarities (sim_t *sim, uint64_t docid, float threshold, int max_result);
    void on_accept (ub::UbEvent *event);

    //TODO change this to config
    int getDefaultMaxResult() {return 20; }
    float getDefaultThreshold() {return 0.0; }
  private:
    int seek_from_cache(sim_t* sim, uint64_t docid, float threshold, int max_result);
    int save_to_cache(const sim_t& sim, uint64_t docid);
    SimServerData *_server_data;
    RWLock _dataLock;
    int _cache_expire;
  };

  class SimServerDataManager {
    SM_DECLARE_SINGLETON (SimServerDataManager);

  public:
    int init();
    int registerSimServer(SimServer *server);

    int checkVersion();
    int force_update (uint64_t version = -1); //update to newest

  private:
    uint64_t _local_version;
    std::map<uint64_t, SimServerData *> _datas;
    SimServer * _server;
  };

};
#endif
