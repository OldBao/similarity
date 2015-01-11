#include <cmath>
#include "model.h"
#include "log.h"

using namespace std;
using namespace sm;


Model::Model(Corpus *corpus, Dictionary *dict) :
  _dict(dict), _corpus(corpus)
{
  
}

Model::~Model() {}



TFIDFModel::TFIDFModel (Corpus *corpus, Dictionary *dict) :
  Model(corpus, dict) 
{ 
  assert (corpus);

  if (dict) {
    _ndoc = corpus->size();
    _nnz = dict->getNnz();
    _dfs = dict->getDfs();
  } else {
    _ndoc = corpus->size();

    for (size_t i = 0; i < corpus->size(); i++) {
      const bow_t &b = corpus->at(i);
      _nnz += b.size();

      for (size_t i = 0; i < b.size(); i++) {
        const bow_unit_t &u = b[i];
        if (u.id >= _dfs.size()) _dfs.resize(u.id+1);

        _dfs[u.id]++;

      }
    }
  }

  SM_LOG_DEBUG ("TFIDF Model [%d] documents, [%d] non-zero entries", _ndoc, _nnz);
}

TFIDFModel::~TFIDFModel(){

}

int
TFIDFModel::train (){
  _idf.resize(_dfs.size());

  for (size_t i  = 0 ; i < _dfs.size(); i++) {
    if (_dfs[i] == 0) {
      _idf[i] = 0.0;
    } else {
      _idf[i] =  log2 ((double) _ndoc / _dfs[i]);
    }
  }  

  return 0;
}


int
TFIDFModel::inference (const Corpus  &corpus, Corpus *dest, bool normalize) {
  bow_t ret;

  for (size_t i = 0; i < corpus.size(); i++) {
    if (0 == i % 1000) {
        SM_LOG_DEBUG ("computed %d", i);
    }
    assert ( 0 == inference (corpus[i], &ret, normalize) );
    assert ( 0 == dest->addDoc (ret)) ;
  }

  return 0;
  
}

int
TFIDFModel::inference (const bow_t &src, bow_t *dest, bool normalize) {
  dest->clear();
  dest->reserve (src.size());

  for (size_t i = 0; i < src.size(); i++) {
    bow_unit_t tmpu;
    if (_idf[src[i].id] == 0.0) {
      continue;
    }
      
    tmpu.id = src[i].id;
    tmpu.weight = _idf[src[i].id] * src[i].weight; 
    if (tmpu.weight >= 1e-12) {
      //precesion threshold
      dest->push_back(tmpu);
    }
    }

  if (normalize) {
    dest->unitvec();
  }

  return 0;
}
