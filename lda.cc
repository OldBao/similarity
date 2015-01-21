#include <fstream>
#include "model.h"
#include "log.h"
#include "encoding.h"
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/io/gzip_stream.h>
#include "interface/lda.pb.h"
#include "configurable.h"

using namespace std;
using namespace sm;

/* extern functions is from lda_utils.c */
extern double alhood(double a, double ss, int D, int K);
extern double d_alhood(double a, double ss, int D, int K);
extern double d2_alhood(double a, int D, int K);
extern double log_sum(double log_a, double log_b);
extern double trigamma(double x);
extern double digamma(double x);
extern double log_gamma(double x);
extern int argmax(double* x, int n);
extern uint32_t randomMT();

#define myrand() (double) (((unsigned long) randomMT()) / 4294967296.)


LDAModel::LDAModel (Corpus *corpus, Dictionary *dict, uint64_t version, int nworkers) :
  TopicModel(corpus, dict),
  _ndocs(_corpus->size()),
  _nterms(corpus->getNTerms()),
  _version(version),
  _log_prob_w(NULL),
  _var_gamma(NULL)
{
  assert (corpus);
  assert (corpus->size() > 0);

  SM_CONFIG_BEGIN(global)
  SM_CONFIG_PROP_STR(model_path, "model");
  SM_CONFIG_END

  SM_CONFIG_BEGIN (lda)
  SM_CONFIG_PROP_STR(model_name, "similarity");
  SM_CONFIG_PROP(alpha, double, 0.01);
  SM_CONFIG_PROP(init_alpha, double, 1.0);
  SM_CONFIG_PROP(estimate_alpha, int32, 1);
  SM_CONFIG_PROP(var_max_iter, int32, 20);
  SM_CONFIG_PROP(var_converged, double, 1e-6);
  SM_CONFIG_PROP(em_max_iter, int32, 100);
  SM_CONFIG_PROP(em_converged, float, 1e-4);
  SM_CONFIG_PROP(max_alpha_iter, int32, 1000);
  SM_CONFIG_PROP(newton_threshold, double, 1e-4);
  SM_CONFIG_PROP(ntopics, int32, 100);
  SM_CONFIG_END
  
  _e_workers.resize(std::min (nworkers, (int) corpus->size()));
  
  _init_prob();
}


void
LDAModel::_init_prob(){
  _log_prob_w = (double **) malloc (sizeof (double *) * _ntopics);
  
  for (int i = 0; i < _ntopics; i++) {
    _log_prob_w[i] = (double *) malloc (sizeof (double) * _nterms);
    for (int j = 0; j < _nterms; j++) {
      _log_prob_w[i][j] = 0.0;
    } 
  }

  // allocate variational parameters
  _var_gamma = (double **) malloc(sizeof(double*) * (_ndocs));
  for (int i = 0; i < _ndocs; i++)
    _var_gamma[i] = (double *) malloc(sizeof(double) * _ntopics);

}


LDAModel::~LDAModel(){
  if (_var_gamma) {
    for (int i = 0; i < _ndocs; i++) free (_var_gamma[i]);
    free (_var_gamma);
  }

  if (_log_prob_w){ 
    for (int i = 0; i < _ntopics; i++) {
      free (_log_prob_w[i]);
    }
    free (_log_prob_w);
  }

  for (size_t i = 0; i < _e_workers.size(); i++) {
    delete _e_workers[i];
  }
}


int
LDAModel::_cluster(){
  _topics.resize(_ntopics);

  for (int i = 0; i < _ndocs; i++) {
    bow_t ret;
    getMostLikelyTopicOfDoc (&ret, i, 0, 1);
    if (ret.size() != 0) {
      //SM_LOG_DEBUG ("doc [%d] belongs to topic [%d:%lf]", i, ret[0].id, ret[0].weight);
      assert (ret[0].id >= 0 && ret[0].id < _ntopics);
      _topics[ret[0].id].push_back (i);
    } else {
      SM_LOG_WARNING ("wtf");
    }
  }

  SM_LOG_DEBUG ("cluster docs %zu done", _topics.size());

  return 0;
}

double
LDAModel::_merge_ss(LDAState *ss) {
  double likelihood = 0.0;

  for (size_t i = 0; i < _e_workers.size(); i++) {
    LDAEWorker* worker = _e_workers[i];
    likelihood += worker->likelihood;
    ss->alpha_suffstats += worker->ss->alpha_suffstats;
    ss->ndocs += worker->ss->ndocs;
    for (int j = 0; j < _ntopics; j++) {
      ss->class_total[j] += worker->ss->class_total[j];
      for (int n = 0; n < _nterms; n++) {
        ss->class_word[j][n] += worker->ss->class_word[j][n];
      }
    }
  }
  
  SM_LOG_DEBUG ("merged likelihood %lf, state : docs %d alpha: %lf", 
                likelihood, ss->ndocs, ss->alpha_suffstats);
  return likelihood;
}

int 
LDAModel::train(){
  for (size_t i = 0; i < _e_workers.size(); i++) {
    LDAEWorker *worker = new LDAEWorker(this);
    _e_workers[i] = worker;
    worker->start();
  }

  // initialize model
  LDAState *ss = new LDAState(*_corpus, _ntopics);
  _mle(ss, 0);
  _alpha = _init_alpha;

  _em (ss);
  
  delete ss;
  _cluster();
  return 0;
}


void
LDAModel::_em(LDAState *ss){
  int i;
  double likelihood, likelihood_old = 0, converged = 1;

  // run expectation maximization
  i = 0; 
  likelihood = likelihood_old = 0.0;
  converged = 1;
  while (((converged < 0) || (converged > _em_converged) || (i <= 2)) && (i <= _em_max_iter)) {
    i++; 
    SM_LOG_DEBUG("**** em iteration %d ****", i);
    likelihood = 0;
    
    ss->zero ();

    // e-step
    for (size_t t = 0; t < _e_workers.size(); t++) {
      _e_workers[t]->ss->zero();
      _e_workers[t]->likelihood = 0.0;
    }

    int range = _corpus->size() / _e_workers.size() + 1;
    int t = 0;
    while (t < (int) _e_workers.size()-1 &&
           ((t+1)*range-1) < (int)_corpus->size() ) 
      {
        pair<int,int> job(t*range, (t+1)*range-1);
        _e_workers[t]->addJob(job);
        t++;
      }

    if ( t * range < (int) _corpus->size() ) {
      pair<int, int> job(t*range, _corpus->size()-1);
      _e_workers[t]->addJob(job);
    }

    for (size_t t = 0; t < _e_workers.size(); t++) {
      _e_workers[t]->waitAllJobDone();
    }

    likelihood = _merge_ss(ss);

    // m-step
    _mle(ss, _estimate_alpha);

    // check for convergence
    converged = (likelihood_old - likelihood) / (likelihood_old);
    if (converged < 0) _var_max_iter = _var_max_iter * 2;
    likelihood_old = likelihood;
  }

}


int 
LDAModel::inference (const bow_t &src, bow_t *ret, bool normalized){
  assert (ret->size() == 0);
  double *var_gamma[1], **phi;

  var_gamma[0] =  (double *) malloc (sizeof (double) * _ntopics);
  phi = (double **) malloc (sizeof (double *) * src.size());
  for (size_t n = 0; n < src.size(); n++) {
    phi[n] = (double *) malloc (sizeof (double) * _ntopics);
  }

  _infer (src, var_gamma[0], phi);

  for (int i = 0; i < _ntopics; i++) {
    bow_unit_t u;
    u.id = i;
    u.weight = _var_gamma[0][i];
    ret->push_back (u);
  }
  ret->sort();

  if (normalized) {
    ret->unitvec();
  }

  return 0;
}


int LDAModel::inference (const Corpus& __attribute__((unused)), 
                         Corpus *__attribute__((unused)), 
                         bool __attribute__((unused))) {
  return 0;
}

int LDAModel::save () {
  char filename[PATH_MAX];
  if (_version != 0) {
    snprintf (filename, PATH_MAX, "%s/%s.lda.%lu",
     _model_path.c_str(), _model_name.c_str(), _version);
  } else {
    snprintf (filename, PATH_MAX, "%s/%s.lda", 
    _model_path.c_str(), _model_name.c_str());
  }
  
  ofstream os(filename);
  if (!os.is_open()){
    SM_LOG_WARNING ("open %s error store lda", filename);
    return -1;
  }
  google::protobuf::io::OstreamOutputStream oos(&os);
  google::protobuf::io::CodedOutputStream cos(&oos);
  
  smpb::LDA slda;

  slda.set_version(_version);
  slda.set_topics(_ntopics);
  slda.set_terms(_nterms);
  slda.set_docs(_ndocs);
  slda.set_alpha(_alpha);

  for (int i = 0; i < _ntopics; i++) {
    for (int j = 0; j < _nterms; j++) {
      slda.add_beta(_log_prob_w[i][j]);
    }
  }
  
  for (int i = 0; i < _ndocs; i++){
    for (int j = 0; j < _ntopics; j++) {
      slda.add_gamma(_var_gamma[i][j]);
    }
  }

  if (!slda.SerializeToCodedStream(&cos)){
    SM_LOG_WARNING ("serialize to %s error", filename);
    return -1;
  }

  SM_LOG_NOTICE ("save model %s success", filename);
  return 0;
}


int 
LDAModel::load() {
  char filename[PATH_MAX];

  if (_version != 0) {
    snprintf (filename, PATH_MAX, "%s/%s.lda.%lu", 
    _model_path.c_str(), _model_name.c_str(), _version);
  } else {
    snprintf (filename, PATH_MAX, "%s/%s.lda", 
    _model_path.c_str(), _model_name.c_str());
  }

  ifstream is(filename);
  if (!is.is_open()) {
    SM_LOG_WARNING ("open dict file %s error", filename);
    return -1;
  }

  google::protobuf::io::IstreamInputStream iis(&is);
  google::protobuf::io::CodedInputStream cis(&iis);
  cis.SetTotalBytesLimit(1024*1024*1024, 1024*1024*1024);

  smpb::LDA dlda;
  if (!dlda.ParseFromCodedStream(&cis)) {
    SM_LOG_WARNING ("parse lda model [%s] error", filename);
    return -1;
  }

  if (_version != dlda.version()) {
    SM_LOG_WARNING ("expect lda model version is %lu, file is %lu", 
                    _version, dlda.version());
    return -1;
  }


  _nterms = dlda.terms();
  _ntopics = dlda.topics();
  _ndocs = dlda.docs();
  _init_prob();

  for (int i = 0; i < _ntopics; i++) {
    for (int j = 0; j < _nterms; j++) {
      _log_prob_w[i][j] = dlda.beta(i*_nterms + j);
    }
  }

  for (int i = 0; i < _ndocs; i++) {
    for (int j = 0; j < _ntopics; j++) {
      _var_gamma[i][j] = dlda.gamma(i*_ntopics+j);
    }
  }

  SM_LOG_NOTICE ("load lda model [%s] success", filename);

  _cluster();
  return 0;
}


int
LDAModel::getMostLikelyTopicOfDoc (bow_t *ret, int docid, double threshold, int max_result) {
  assert (ret && ret->size() == 0);
  assert (docid < (int) _ndocs);

  for (int i = 0; i < _ntopics; i++) {
    bow_unit_t u;
    u.id = i;
    u.weight = _var_gamma[docid][i];
    if (u.weight < threshold)
      continue;

    ret->push_topk (u, max_result);
  }

  ret->sort();
  return 0;
}


int
LDAModel::getDocsOfTopic (vector<int> *tmp, int topicid) {
  assert (topicid < _ntopics && topicid >= 0);

  *tmp =  _topics[topicid];
  return 0;

}


void
LDAModel::getHotestWords (bow_t *bow, int topicid, int nwords) {
  assert (topicid >=1 && topicid <= _ntopics);
  assert (bow->size() == 0);

  bow_unit_t tmp;
  int i;

  for (i = 0; i < _nterms; i++) {
    tmp.id = i;
    tmp.weight = _log_prob_w[topicid-1][i];
    bow->push_topk (tmp, nwords);
  }
  bow->sort();
}

void
LDAModel::getHotestWordsDesc(string *desc, int topicid, int nwords, const std::string& encoding){
  assert (desc->size() == 0);
  bow_t bow;
  getHotestWords(&bow, topicid, nwords);
  
  stringstream ss;
  rwtrans_func_t *w = get_rwtrans(encoding);
  assert(w);

  ss << "Topic [" << topicid-1 << "]: ";

  for (size_t i = 0; i < bow.size(); i++) {
    string buffer;
    if (_dict) {
      assert (bow[i].id < _dict->size());
      const wstring &d = _dict->at(bow[i].id);
      assert (0 == w (d, &buffer) );
      ss << buffer;
    } else {
      ss << bow[i].id;
    }
    ss << "*" << bow[i].weight << " ";
  }
  
  desc->assign (ss.str());
}


double 
LDAModel::_e_step(const bow_t& doc, 
                  double* gamma, 
                  double** phi,
                  LDAState* ss)
{
  double likelihood;
  size_t n;
  int k;

  // posterior inference
  likelihood = _infer(doc, gamma, phi);
  // update sufficient statistics
  double gamma_sum = 0;
  for (k = 0; k < _ntopics; k++) {
        gamma_sum += gamma[k];
        ss->alpha_suffstats += digamma(gamma[k]);
    }
    ss->alpha_suffstats -= _ntopics * digamma(gamma_sum);

    for (n = 0; n < doc.size(); n++) {
        for (k = 0; k < _ntopics; k++) {
          ss->class_word[k][doc[n].id] += doc[n].weight*phi[n][k];
          ss->class_total[k] += doc[n].weight*phi[n][k];
        }
    }

    ss->ndocs +=  1;
    return likelihood;
}


void
LDAModel::_mle (LDAState* ss, int estimate_alpha) {
  int k; int w;

  for (k = 0; k < _ntopics; k++) {
    for (w = 0; w < _nterms; w++) {
      if (ss->class_word[k][w] > 0) {
        _log_prob_w[k][w] =
          log(ss->class_word[k][w]) -
          log(ss->class_total[k]);
      } else {
        _log_prob_w[k][w] = -100;
      }
    }
  }

  if (estimate_alpha == 1) {
    _alpha = _opt_alpha(ss->alpha_suffstats,
                        ss->ndocs,
                        _ntopics);

    SM_LOG_DEBUG ("new alpha = %5.5f", _alpha);
  }
}

double 
LDAModel::_infer(const bow_t& doc, double* var_gamma, double** phi) {
    double converged = 1;
    double phisum = 0, likelihood = 0;
    double likelihood_old = 0, oldphi[_ntopics];
    size_t n;
    int k;
    int var_iter;
    double digamma_gam[_ntopics];

    // compute posterior dirichlet
    for (k = 0; k < _ntopics; k++) {
      assert (doc.total() != NAN);
      var_gamma[k] = _alpha + (doc.total()/((double) _ntopics));
      digamma_gam[k] = digamma(var_gamma[k]);
      for (n = 0; n < doc.size(); n++)
        phi[n][k] = 1.0/_ntopics;
    }
    var_iter = 0;

    while ((converged > _var_converged) &&
           ((var_iter < _var_max_iter) || (_var_max_iter == -1))) {
	var_iter++;
	for (n = 0; n < doc.size(); n++) {
            phisum = 0;
            for (k = 0; k < _ntopics; k++) {
              oldphi[k] = phi[n][k];
              phi[n][k] =
                digamma_gam[k] +
                _log_prob_w[k][doc[n].id];

              if (k > 0)
                phisum = log_sum(phisum, phi[n][k]);
              else
                phisum = phi[n][k]; // note, phi is in log space
            }

            for (k = 0; k < _ntopics; k++) {
              phi[n][k] = exp(phi[n][k] - phisum);
              var_gamma[k] =
                var_gamma[k] + doc[n].weight*(phi[n][k] - oldphi[k]);
              // !!! a lot of extra digamma's here because of how we're computing it
              // !!! but its more automatically updated too.
              digamma_gam[k] = digamma(var_gamma[k]);
            }
        }

        likelihood = _compute_likelihood(doc, phi, var_gamma);
        assert(!isnan(likelihood));
        converged = (likelihood_old - likelihood) / likelihood_old;
        likelihood_old = likelihood;

        //SM_LOG_TRACE ("[LDA INF] %8.5f %1.3e", likelihood, converged);
    }
    return likelihood;
}


double 
LDAModel::_opt_alpha(double ss, int D, int K) {
  double a, log_a, init_a = 100;
  double f, df, d2f;
  int iter = 0;

  log_a = log(init_a);
  do {
    iter++;
    a = exp(log_a);
    if (isnan(a)) {
      init_a = init_a * 10;
      SM_LOG_DEBUG ("warning : alpha is nan; new init = %5.5f", init_a);
      a = init_a;
      log_a = log(a);
    }
    f = alhood(a, ss, D, K);
    df = d_alhood(a, ss, D, K);
    d2f = d2_alhood(a, D, K);
    log_a = log_a - df/(d2f * a + df);
    SM_LOG_DEBUG ("alpha maximization : %5.5f   %5.5f", f, df);
  }
  while ((fabs(df) > _newton_threshold) && (iter < _max_alpha_iter));
  return exp(log_a) ;
}



double
LDAModel::_compute_likelihood(const bow_t& doc, double** phi, double* var_gamma) {
  double likelihood = 0, digsum = 0, var_gamma_sum = 0, dig[_ntopics];
  int k;
  size_t n;

  for (k = 0; k < _ntopics; k++) {
    dig[k] = digamma(var_gamma[k]);
    var_gamma_sum += var_gamma[k];
  }
  digsum = digamma(var_gamma_sum);

  likelihood =
    lgamma(_alpha * _ntopics)
    - _ntopics * lgamma(_alpha)
    - (lgamma(var_gamma_sum));

  for (k = 0; k < _ntopics; k++) {
    likelihood +=
      (_alpha - 1)*(dig[k] - digsum) + lgamma(var_gamma[k])
      - (var_gamma[k] - 1)*(dig[k] - digsum);

    for (n = 0; n < doc.size(); n++) {
      if (phi[n][k] > 0) {
        likelihood += doc[n].weight*
          (phi[n][k]*((dig[k] - digsum) - log(phi[n][k])
                      + _log_prob_w[k][doc[n].id]));
      }
    }
  }
  return likelihood;
}


LDAState::LDAState(const Corpus &corpus, int topics):
  ndocs(0),
  _nterms (corpus.getNTerms()),
  _ntopics(topics)
{
  int i, j, k, n;

  class_word =  (double **)  malloc (sizeof (double) * _ntopics);
  class_total = (double *)  malloc (sizeof (double*) * _ntopics);


  for (i = 0; i < _ntopics; i++) {
    class_total[i] = 0.0;
    class_word[i] = (double *) malloc (sizeof (double) * _nterms);
    for (j = 0; j < _nterms; j++) {
      class_word[i][j] = 0.0;
    }
  }

  /*
  for (k = 0; k < _ntopics; k++) {
    for (i = 0; i < num_init; i++) {
      d = floor(myrand() * corpus.size());
      SM_LOG_DEBUG ("initialized with document %d", d);
      const bow_t& doc = corpus[d];
      for (n = 0; n < doc.size(); n++) {
        class_word[k][doc[n].id] += doc[n].weight;
      }
    }

    for (n = 0; n < _nterms; n++) {
      class_word[k][n] += 1.0;
      class_total[k] = class_total[k] + class_word[k][n];
    }
    }*/

  for (k = 0; k < _ntopics; k++)
    for (n = 0; n < _nterms; n++) {
      class_word[k][n] += 1.0/_nterms + myrand();
      class_total[k] += class_word[k][n];
    }
}

LDAState::~LDAState(){
  for (int i = 0; i < _ntopics; i++) {
    free(class_word[i]);
  }

  free(class_total);
  free(class_word);
}


void
LDAState::zero() {
  int k, w;
  for (k = 0; k < _ntopics; k++) {
    class_total[k] = 0;
    for (w = 0; w < _nterms; w++) {
      class_word[k][w] = 0;
    }
  }

  ndocs = 0;
  alpha_suffstats = 0;
}


LDAEWorker::LDAEWorker(LDAModel *model)
  : likelihood(0.0),  model(model), ss(NULL)
{
  phi = (double **) malloc( sizeof(double*) * model->_corpus->maxDocLen());
  for (size_t i = 0; i < model->_corpus->maxDocLen(); i++)
    phi[i] = (double *) malloc(sizeof(double) * model->_ntopics);

  ss = new LDAState (*model->_corpus, model->_ntopics);
}


LDAEWorker::~LDAEWorker() {
  if (ss) delete ss;
  if (phi) {
    for (size_t i = 0; i < model->_corpus->maxDocLen(); i++)
      free(phi[i]);
    free(phi);
  }
}

int
LDAEWorker::doJob(const pair<int, int>& range){
  SM_ASSERT (range.first <= range.second, "range should be [l, g]");

  /*
  SM_LOG_NOTICE ("Trainning [%d-%d]",
                 range.first, range.second);
  */
  for (int i = range.first; i <= range.second; i++) {
    likelihood += model->_e_step(model->_corpus->at(i),
                                 model->_var_gamma[i],
                                 phi,
                                 ss);
  }
  /*
  SM_LOG_NOTICE ("[%d-%d] likelihood %lf", 
                 range.first, range.second, likelihood);
  */
  return 0;
}
