#include "model.h"
#include "log.h"

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

static inline bool
_bow_cmp (const bow_unit_t&a, const bow_unit_t &b) {
  return a.weight > b.weight;
}

LDAModel::LDAModel (Corpus *corpus, Dictionary *dict) :
  Model(corpus, dict),
  _alpha(0.01), 
  _init_alpha(1.0),
  _estimate_alpha(1), //TODO 1 or 0
  _var_max_iter(20),
  _var_converged(1e-6),
  _em_max_iter(100),
  _em_converged(1e-6),
  _max_alpha_iter(1000),
  _newton_threshold(1e-10),
  _ntopics(100),
  _nterms(corpus->getNTerms())
{
  assert (corpus);

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

}


LDAModel::~LDAModel(){
  if (_log_prob_w){ 
    for (int i = 0; i < _ntopics; i++) {
      free (_log_prob_w[i]);
    }
    free (_log_prob_w);
  }
}


int 
LDAModel::train(){
  // initialize model
  LDAState *ss = new LDAState(*_corpus, _ntopics);
  _mle(ss, 0);
  _alpha = _init_alpha;

  _em (ss);
  return 0;
}


void
LDAModel::_em(LDAState *ss){
  size_t d, n, max_length;
  int i;
  double **var_gamma, **phi;
  double likelihood, likelihood_old = 0, converged = 1;

  // allocate variational parameters
  var_gamma = (double **) malloc(sizeof(double*)*(_corpus->size()));
  for (d = 0; d < _corpus->size(); d++)
    var_gamma[d] = (double *) malloc(sizeof(double) * _ntopics);

  max_length = _corpus->maxDocLen();
  phi = (double **) malloc( sizeof(double*) * max_length);
  for (n = 0; n < max_length; n++)
    phi[n] = (double *) malloc(sizeof(double) * _ntopics);


  // run expectation maximization
  i = 0; 
  likelihood = likelihood_old = 0.0;
  converged = 1;
  while (((converged < 0) || (converged > _em_converged) || (i <= 2)) && (i <= _em_max_iter)) {
    i++; 
    SM_LOG_DEBUG("**** em iteration %zu ****", i);
    likelihood = 0;
    
    ss->zero ();
    // e-step
    for (d = 0; d < _corpus->size(); d++) {
      likelihood += _e_step(_corpus->at(d),
                            var_gamma[d],
                            phi,
                            ss);
    }

    // m-step
    _mle(ss, _estimate_alpha);

    // check for convergence
    converged = (likelihood_old - likelihood) / (likelihood_old);
    if (converged < 0) _var_max_iter = _var_max_iter * 2;
    likelihood_old = likelihood;
  }

#ifdef DEBUG
  //output final visual 
  FILE *fp = fopen("ws.txt", "w+");
  for (d = 0; d < _corpus->size(); d++) {
    const bow_t& doc = _corpus->at(d);
    likelihood += _infer (_corpus->at(d), var_gamma[d], phi);
    fprintf (fp, "%03zu", doc.size());
    for (n = 0; n < doc.size(); n++){
      fprintf (fp, " %04d:%02d", doc[n].id, argmax(phi[n], _ntopics));
    }
    fprintf (fp, "\n");
    fflush(fp);
  }
#endif

  for (i = 0; i < _corpus->size(); i++) free (var_gamma[i]);
  free (var_gamma);
  for (i = 0; i < max_length; i++) free (phi[i]);
  free (phi);

}

int LDAModel::inference (const bow_t &src, bow_t *ret, bool normalized){
  double *var_gamma[1], likelihood, **phi;

  var_gamma[0] =  (double *)malloc (sizeof (double) * _ntopics);
  phi = (double **) malloc (sizeof (double *) * src.size());
  for (int n = 0; n < src.size(); n++) {
    phi[n] = (double *) malloc (sizeof (double) * _ntopics);
  }
  likelihood = _infer (src, var_gamma[0], phi);

  return 0;
}
int LDAModel::inference (const Corpus& corpus, Corpus *ret, bool normalized) {
  return 0;
}

int LDAModel::save (const std::string &path, const std::string &basename) { 
  char filename[PATH_MAX];
  int ret;
  snprintf (filename, PATH_MAX, "%s/%s.meta", path.c_str(), basename.c_str());
  
  FILE *fp = fopen(filename, "w");
  if (!fp) {
    SM_LOG_WARNING ("open  meta file %s error", filename);
    goto error;
  }
  
  ret = fprintf (fp, "num-topics %d\nnum-terms%d\nalpha %f\n", _ntopics, _corpus->getNTerms(), _alpha);
  if (ret < 0) {
    SM_LOG_WARNING ("write error");
    goto error;
  }

  fclose(fp);

  snprintf(filename, PATH_MAX, "%s/%s.beta", path.c_str(), basename.c_str());
  fp = fopen(filename, "w");
  if (!fp) {
    SM_LOG_WARNING ("opening beta file [%s] for writting error", filename);
    goto error;
  }

  for (int i = 0; i < _ntopics; i++) {
    for (int j = 0; j < _nterms; j++) {
      fprintf (fp, " %5.10f", _log_prob_w[i][j]);
    }
    fprintf (fp, "\n");
  }
  fclose(fp);
  
  return 0;
 error:
  if (fp) fclose(fp);
  return -1;
}


int LDAModel::load (const std::string &path, const std::string &basename) {
  char filename[PATH_MAX];
  int nterms;

  snprintf (filename, PATH_MAX, "%s/%s.meta", path.c_str(), basename.c_str());

  FILE *fp = fopen (filename, "r");
  if (!fp) {
    SM_LOG_WARNING ("open meta file %s error", filename);
    goto error;
  }
  if (3 != fscanf (fp, "num-topics %d\nnum-terms%d\nalpha %f\n", 
                   &_ntopics, &nterms, &_alpha) )
    {
      SM_LOG_WARNING ("meta file format error!");
      goto error;
    }

  SM_LOG_DEBUG ("opening meta file %s success", filename);
  fclose(fp);

  assert (nterms == _nterms);
  _init_prob();

  snprintf (filename, PATH_MAX, "%s/%s.beta", path.c_str(), basename.c_str());
  fp = fopen(filename, "r");
  if (!fp) {
    SM_LOG_WARNING ("open beta file %s error", filename);
    goto error;
  }

  for (int i = 0; i < _ntopics; i++) {
    for (int j = 0; j < nterms; j++) {
      float x;
      if ( 1 != fscanf (fp, "%f", &x)){
        SM_LOG_WARNING ("beta file format error");
        goto error;
      }

      _log_prob_w[i][j] = x;
        
    }
  }

  fclose(fp);
  return 0;
 error:
  if (fp) {
    fclose(fp);
  }
  if (_log_prob_w){
    for (int i = 0; i < _ntopics; i++) {
      free (_log_prob_w[i]);
      }
    free(_log_prob_w);
  }

  return -1;
}

void
LDAModel::getHotestWords (bow_t *bow, int topicid, int nwords) {
  assert (topicid >=1 && topicid <= _ntopics);
  bow_unit_t tmp;
  int i;
  bow->clear();

  for (i = 0; i < _nterms; i++) {
    tmp.id = i;
    tmp.weight = _log_prob_w[topicid-1][i];
    bow->push_back(tmp);
    push_heap (bow->v.begin(), bow->v.end(), _bow_cmp);
    if (bow->size() > nwords) {
      pop_heap (bow->v.begin(), bow->v.end(), _bow_cmp);
      bow->v.pop_back();
    }
  }
  bow->sort();

}

void
LDAModel::getHotestWordsDesc(string *desc, int topicid, int nwords, const std::string& encoding){
  bow_t bow;
  getHotestWords(&bow, topicid, nwords);
  
  string buffer;
  stringstream ss;
  ss << "Topic [" << topicid-1 << "]: ";

  for (size_t i = 0; i < bow.size(); i++) {
    if (_dict) {

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


LDAState::LDAState(const Corpus &corpus, int topics, int num_init):
  ndocs(0),
  _nterms (corpus.getNTerms()),
  _ntopics(topics)
{
  int i, j, k, d, n;

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




