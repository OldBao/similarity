#include <vector>
#include <algorithm>
#include "similarity.h"
#include "log.h"
using namespace std;
using namespace sm;

#define THREAD_COUNT 5

SimCalculator::SimCalculator(TopicSimilarity *sim) : _sim(sim){}
SimCalculator::~SimCalculator() {}

int SimCalculator::doJob (int64_t* const &id) {
  _sim->doJob((int64_t)id);
}

TopicSimilarity::TopicSimilarity(TopicModel *model, Corpus *corpus, Dictionary *dict, int maxSim) : 
  _model(model), _dict(dict), _corpus(corpus), _maxSim(maxSim)
{
  _sims.resize (_corpus->size());

  for (int i = 0; i < THREAD_COUNT; i++) {
    SimCalculator *cal = new SimCalculator(this);
    cal->start();
    _calculators.push_back(cal);
  }
}



TopicSimilarity::~TopicSimilarity() {
  for (int i = 0; i < _calculators.size(); i++) {
    delete _calculators[i];
  }

}


void
TopicSimilarity::waitAllJobDone(){
  for (vector<SimCalculator *>::iterator iter = _calculators.begin();
       iter != _calculators.end();
       iter++)
    {
      (*iter)->waitAllJobDone();
    }
}

int
TopicSimilarity::_cal_within_topic  (int id) {
  vector<int> v;
  v.push_back(id);

  return _cal_within_topics(v);
}


int
TopicSimilarity::_cal_within_topics(const vector<int> &topics) {
  vector<int> all;
  
  for (vector<int>::const_iterator iter = topics.begin();
       iter != topics.end();
       iter++)
    {
      vector<int> tmp;
      _model->getDocsOfTopic(&tmp, *iter);
      
      if (tmp.size() == 0) continue;
      for (vector<int>::const_iterator iter = tmp.begin(); iter != tmp.end(); iter++) {
        all.push_back(*iter);
      }
    }

  int an = all.size();
  if (an == 0) return 0;

  SM_LOG_DEBUG ( "[%d] docs in this series topic", all.size() );
  
  for (int i = 0 ; i < an; i++) {
    const bow_t& src = _corpus->at(all[i]);
    for (int j = i+1; j < an; j++) {
      bow_unit_t u;
      const bow_t &dest = _corpus->at(all[j]);
      float distance = src.cossim(dest);
      if (distance == 0.0) continue;
      u.id = all[j];
      u.weight = distance;
      _sims[ all[i] ].push_topk (u, _maxSim);
      u.id = all[i];
      _sims[ all[j] ].push_topk (u, _maxSim);
    }
  }

  for (int i = 0; i < an; i++) {
    //TODO delete this
    stringstream tmp;
    tmp << "Sim of [ " << all[i]  << "]:  ";
    _sims[ all[i] ].sort();

    for (int j = 0; j < _sims[ all[i] ].size(); j++) {
      tmp << "[" << _sims[all[i]][j].id << ":" << _sims[ all[i] ][j].weight << "]";
    }
    SM_LOG_DEBUG ("%s", tmp.str().c_str());
  }
  
  return 0;
}

int
TopicSimilarity::doJob(int64_t topicid) {
  return _cal_within_topic(topicid);
}

int
TopicSimilarity::calculate(int64_t topicid){
  _sims.resize (_corpus->size());
  
  // this is a little trick
  // because i want implement "auto delete", but 'delete int' is invalid
  // so i just send this scala as integer
  // the best choice is use type_trait, but it's seems too complicated ;-(
  this->_calculators[topicid%THREAD_COUNT]->addJob ((int64_t *) topicid);
  return 0;
}


int
TopicSimilarity::getSimilarities (bow_t *ret, int id, double sim_threshold, int max_result)
{
  SM_CHECK_RET_ERR (id < _sims.size(), "sim size is too small");
  SM_CHECK_RET_ERR (ret->size() == 0, "return container should be empty");

  const bow_t &src = _sims[id];

  for (int i = 0; i < src.size(); i++) {
    const bow_unit_t u = src[i];
    if (u.weight < sim_threshold) continue;
    else {
      ret->push_back (u);
      if (ret->size() > max_result) break;
    }
  }

  return 0;
}
