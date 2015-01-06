#include "model.h"

static const int NUM_INIT 1

LDAModel::LDAModel (Corpus *corpus, Dictionary *dict) :
  _corpus(corpus),
  _dict(dict)
  _alpha(1.0)
{
  assert (corpus);
  assert (dict);

  _log_prop_w = malloc (sizeof (double *) * _ntopics);
  
  for (int i = 0; i < _ntopics; i++) {
    _log_prop_w[i] = malloc (sizeof (double) * dict->size());
    for (int j = 0; j < dict->size(); j++) {
      _log_prop_w[i][j] = 0.0;
    } 
  }
}

LDAModel::~LDAModel(){
  for (int i = 0; i < _ntopics; i++) {
      free (_log_prop_w[i]);
  }
  free (_log_prop_w);
}


int 
LDAModel::train(){
  int d, n;
  lda_model *model = NULL;
  double **var_gamma, **phi;

  // allocate variational parameters
  var_gamma = malloc(sizeof(double*)*(corpus->size()));
  for (d = 0; d < corpus->num_docs; d++)
    var_gamma[d] = malloc(sizeof(double) * _ntopics);

  int max_length = corpus->maxDocLen();
  phi = malloc( sizeof(double*) * max_length);
  for (n = 0; n < max_length; n++)
    phi[n] = malloc(sizeof(double) * _ntopics);
  
  // initialize model
  int ndocs = _corpus.size(), nterms = _dict.size();
  model = load_lda_model(start);
  LDAState *ss = new LDAState(_ntopics, nterms);

  // run expectation maximization
  int i = 0;
  double likelihood, likelihood_old = 0, converged = 1;

  while (((converged < 0) || (converged > _convergence) || (i <= 2)) && (i <= _iter)) {
    i++; 
    SM_DEBUG_LOG("**** em iteration %d ****\n", i);
    likelihood = 0;
    
    ss->zero (ntopics, nterms);
    // e-step
    for (d = 0; d < _ntopics; d++) {
      if ((d % 1000) == 0) {
        SM_DEBUG_LOG("document %d\n",d);
      }

      likelihood += do_e_step(corpus[d],
                              var_gamma[d],
                              phi,
                              ss);
    }

    // m-step
    mle(ss);

    // check for convergence
    converged = (likelihood_old - likelihood) / (likelihood_old);
    if (converged < 0) _max_iter = _max_iter * 2;
    likelihood_old = likelihood;
  }
}


double 
LDAModel::do_e_step(const bow_t& doc, 
                     double* gamma, 
                     double** phi,
                     LDAState* ss)
{
    double likelihood;
    int n, k;
    // posterior inference
    likelihood = inference(doc, gamma, phi);
    // update sufficient statistics
    double gamma_sum = 0;
    for (k = 0; k < model->_ntopics; k++) {
        gamma_sum += gamma[k];
        ss->alpha_suffstats += digamma(gamma[k]);
    }
    ss->alpha_suffstats -= _ntopics * digamma(gamma_sum);

    for (n = 0; n < doc->length; n++) {
        for (k = 0; k < _ntopics; k++) {
          ss->class_word[k][doc[n].id] += doc[n].weight*phi[n][k];
          ss->class_total[k] += doc[n].weight*phi[n][k];
        }
    }

    ss->ndocs +=  1;
    return(likelihood);
}

double
LDAModel::mle (LDAState* ss) {
  int k; int w;
  int nterms = _dict->size(), ndocs = _corpus->size();
  for (k = 0; k < model->_ntopics; k++) {
    for (w = 0; w < nterms; w++) {
      if (ss->class_word[k][w] > 0) {
        _log_prob_w[k][w] =
          log(ss->class_word[k][w]) -
          log(ss->class_total[k]);
      } else {
        _log_prob_w[k][w] = -100;
      }
    }

    if (_estimate_alpha == 1)
    {
      _alpha = opt_alpha(ss->alpha_suffstats,
                         ss->num_docs,
                         _ntopics);

      SM_DEBUG_LOG("new alpha = %5.5f\n", model->alpha);
    }
  }
}


LDAState::LDAState(int ntopics, int nterms) {
  class_word =  (double *)  malloc (sizeof (double) * ntopics);
  class_total = (double **) malloc (sizeof (double*) * ntopics);

  for (int i = 0; i < ntopics; i++) {
    class_total[i] = 0.0;
    class_word[i] = malloc (sizeof (double) * nterms);
    for (int j = 0; j < nterms; j++) {
      class_word[i][j] = 0.0;
    }
  }
}


LDAState::zero(int ntopics, int nterms) {
  int k, w;
  for (k = 0; k < ntopics; k++) {
    class_total[k] = 0;
    for (w = 0; w < nterms; w++) {
      class_word[k][w] = 0;
    }
  }

  nDocs = 0;
  alpha_suffstats = 0;
}
