/**
 * @file   similarity.h
 * @author Gethin Zhang <zhangguanxing01@baidu.com>
 * @date   Mon Jan 12 11:33:50 2015
 * 
 * @brief  calcluate similarities from Model
 * note that this class will calculate sims from model and do inference; 
 * inferenced documents will not update to this model, you can use thirdparty cache (memcache | redis)
 * to store sim results. If you want to update the model, please recraete an new instance
 */

#ifndef SIMILARITY_H_
#define SIMILARITY_H_

#include "corpus.h"
#include "model.h"
#include "document.h"
#include "dictionary.h"
#include "concurrent.h"

namespace sm {
  class SimCalculator;

  class Similarity {
  public:
    Similarity(Model *model, Corpus *corpus, Dictionary *dict){}
    virtual ~Similarity() {}

    virtual int calculate(int64_t topicid) = 0;

    virtual int getSimilarities ( bow_t *ret, 
                                  int id, 
                                  double sim_threshold = 0.1, 
                                  int max_result=20) = 0;
    virtual int getSimilarities ( bow_t *ret, 
                                  const bow_t& bow,
                                  const std::vector<int>& comp_dest, 
                                  double sim_threshold = 0.1,
                                  int max_result=20) = 0;
    // inference
    //virtual int inference (std::vector<int> *ret, const bow_t& bow, double sim_theshold=0.75, int max_result = 50) = 0;
    //virtual int save(const std::string &path, const std::string &basename) = 0;
    //virtual int load(const std::string &path, const std::string &basename) = 0;
    virtual void waitAllJobDone() = 0;

  protected:
    Similarity(Similarity &) {}
    Similarity() {}
  };


  class TopicSimilarity : public Similarity {
  public:
    TopicSimilarity(TopicModel *model, Corpus *corpus, Dictionary *dict, uint64_t version = 0);
    virtual ~TopicSimilarity();

    virtual int calculate (int64_t topicid);

    virtual int getSimilarities ( bow_t * ret,
                                  int id, 
                                  double sim_threshold = 0.75, 
                                  int max_result=30 );

    virtual int getSimilarities ( bow_t *ret, 
                                  const bow_t& bow,
                                  const std::vector<int>& comp_dest, 
                                  double sim_threshold = 0.1,
                                  int max_result=30);

    virtual int doJob(int64_t topicid);
    // inference
   
    virtual void waitAllJobDone();
    
    int save();
    int load();

  private:
    std::string _model_path, _model_name;
    uint64_t _version;
    TopicModel *_model;
    Dictionary *_dict;
    Corpus *_corpus;
    int _maxSim;
    int _nworker;
    
    std::vector< bow_t > _sims;
    
    int _cal_within_topic  (int topicid);
    int _cal_within_topics  (const vector<int>& topics);
    TopicSimilarity(TopicSimilarity &);
    TopicSimilarity();

    vector <SimCalculator *> _calculators;
  };

  class SimCalculator : public Thread<int64_t> {
  public:
    SimCalculator(TopicSimilarity *sim);
    ~SimCalculator();
    virtual int doJob(const int64_t &topicid);

  private:
    TopicSimilarity *_sim;
  };

}



#endif
