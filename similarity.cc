#include <fstream>
#include <vector>
#include <algorithm>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/io/gzip_stream.h>
#include "similarity.h"
#include "log.h"
#include "interface/similarity.pb.h"
using namespace std;
using namespace sm;

SimCalculator::SimCalculator(TopicSimilarity *sim) : _sim(sim){}
SimCalculator::~SimCalculator() {}

int SimCalculator::doJob (const int64_t &id) {
  _sim->doJob(id);
  return 0;
}

TopicSimilarity::TopicSimilarity(TopicModel *model, Corpus *corpus, Dictionary *dict, 
                                 uint64_t version) : 
  _version (version), _model(model), 
  _dict(dict), _corpus(corpus), _maxSim(50), _nworker(12)
{
  _sims.resize (_corpus->size());

  for (int i = 0; i < _nworker ; i++) {
    SimCalculator *cal = new SimCalculator(this);
    cal->start();
    _calculators.push_back(cal);
  }
}



TopicSimilarity::~TopicSimilarity() {
  for (size_t i = 0; i < _calculators.size(); i++) {
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

  return 0;
}


int
TopicSimilarity::doJob(int64_t topicid) {
  return _cal_within_topic(topicid);
}


int
TopicSimilarity::calculate(int64_t topicid){
  _sims.resize (_corpus->size());
  
  this->_calculators[topicid%_calculators.size()]->addJob (topicid);
  return 0;
}


int
TopicSimilarity::getSimilarities(bow_t *ret, const bow_t& bow, const vector<int> &dest, 
                                 double threshold, int max_result)
{
  SM_ASSERT (ret && ret->size() == 0, "ret should be empty");
  if (max_result <= 0) return 0;
  
  for (vector<int>::const_iterator iter = dest.begin();
       iter != dest.end();
       iter++)
    {
      SM_ASSERT (*iter < (int)_corpus->size() && *iter >0, 
                 "id [%d] not with corpus", *iter);
      bow_unit_t u;
      float distance = bow.cossim(_corpus->at(*iter));

      if (distance <= threshold) {
        continue;
      }
      u.id = *iter;
      u.weight = distance;
      
      ret->push_topk (u, max_result);
    }
  
  ret->sort();
  return 0;
}


int
TopicSimilarity::getSimilarities (bow_t *ret, int id, double sim_threshold, int max_result)
{
  if (max_result <= 0) return 0;
  SM_CHECK_RET_ERR (id < (int)_sims.size(), "sim size is too small");
  SM_CHECK_RET_ERR (ret->size() == 0, "return container should be empty");

  const bow_t &src = _sims[id];
  _sims[id].sort();

  for (size_t i = 0; i < src.size(); i++) {
    const bow_unit_t u = src[i];
    if (u.weight < sim_threshold) continue;
    else {
      ret->push_back (u);
      if ((int)ret->size() >= max_result) break;
    }
  }

  return 0;
}


int
TopicSimilarity::save(const string &path, const string &basename) {
  char filename[PATH_MAX];
  if (_version != 0) {
    snprintf (filename, PATH_MAX, "%s/%s.sim.%lu", path.c_str(), basename.c_str(), _version);
  } else {
    snprintf (filename, PATH_MAX, "%s/%s.sim", path.c_str(), basename.c_str());
  }

  ofstream os(filename);
  if (!os.is_open()){
    SM_LOG_WARNING ("open %s error store lda", filename);
    return -1;
  }

  google::protobuf::io::OstreamOutputStream oos(&os);
  google::protobuf::io::CodedOutputStream cos(&oos);
  smpb::Similarity ssim;

  ssim.set_version(_version);

  SM_ASSERT (_sims.size() == _corpus->size(), "similarity of doc must be equal");
  for (size_t i = 0; i < _sims.size(); i++) {
    smpb::Sim* simOfDoc = ssim.add_sims ();
    for (size_t j = 0; j < _sims[i].size(); j++) {
      smpb::SimUnit *unit = simOfDoc->add_units();
      unit->set_id(_sims[i][j].id);
      unit->set_sim(_sims[i][j].weight);
    }
  }

  if (!ssim.SerializeToCodedStream(&cos)){
    SM_LOG_WARNING ("serialize to %s error", filename);
    return -1;
  }

  SM_LOG_NOTICE ("save sim model %s success", filename);
  return 0;
}

int
TopicSimilarity::load(const string &path, const string &basename) {
  char filename[PATH_MAX];

  if (_version != 0) {
    snprintf (filename, PATH_MAX, "%s/%s.sim.%lu", path.c_str(), basename.c_str(), _version);
  } else {
    snprintf (filename, PATH_MAX, "%s/%s.sim", path.c_str(), basename.c_str());
  }

  ifstream is(filename);
  if (!is.is_open()) {
    SM_LOG_WARNING ("open sim file %s error", filename);
    return -1;
  }

  google::protobuf::io::IstreamInputStream iis(&is);
  google::protobuf::io::CodedInputStream cis(&iis);
  cis.SetTotalBytesLimit(1024*1024*1024, 1024*1024*1024); //TODO change to soft code

  smpb::Similarity dsim;
  if (!dsim.ParseFromCodedStream(&cis)) {
    SM_LOG_WARNING ("parse sim model [%s] error", filename);
    return -1;
  }

  if (_version != dsim.version()) {
    SM_LOG_WARNING ("expect sim model version is %lu, file is %lu", 
                    _version, dsim.version());
    return -1;
  }

  SM_CHECK_RET_ERR (dsim.sims_size() == (int)_corpus->size(), 
                    "sim size [%d] not equals to corpus size [%zu]",
                    dsim.sims_size(), _corpus->size());

  _sims.resize(dsim.sims_size());
  for (int i = 0; i < dsim.sims_size(); i++) {
    const smpb::Sim &dsimOfDoc = dsim.sims(i);
    for (int j = 0; j < dsimOfDoc.units_size(); j++) {
      bow_unit_t u;
      u.id = dsimOfDoc.units(j).id();
      u.weight = dsimOfDoc.units(j).sim();

      _sims[i].push_back (u);
    }
  }

  SM_LOG_NOTICE ("load sim model [%s] success", filename);

  return 0;
}
