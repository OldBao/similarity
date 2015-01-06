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
    _ndoc = dict->getNDoc();
    _nnz = dict->getNnz();
    _dfs = dict->getDfs();
  } else {
    _ndoc = corpus->size();

    for (size_t i = 0; i < corpus->size(); i++) {
      const bow_t &b = corpus->at(i);
      _nnz += b.size();

      for (bow_t::const_iterator iter = b.begin(); iter != b.end(); iter++) {
        if (iter->id >= _dfs.size()) _dfs.resize(iter->id+1);

        _dfs[iter->id]++;

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
TFIDFModel::inference (const Corpus  &corpus, Corpus *cor, bool normalize) {
  return 0;
}

int
TFIDFModel::inference (const bow_t &src, bow_t *dest, bool normalize) {
  dest->reserve (src.size());

  for (bow_t::const_iterator iter = src.begin();
       iter != src.end();
       iter++)
    {
      bow_unit_t tmpu;
      if (_idf[iter->id] == 0.0) {
        continue;
      }
      
      tmpu.id = iter->id;
      tmpu.weight = _idf[iter->id] * iter->weight; 
      if (tmpu.weight >= 1e-12) {
        //precesion threshold
        dest->push_back(tmpu);
      }
    }

  if (normalize) {
    //TODO
  }

  return 0;
}


int TFIDFModel::save (const std::string &name){
  return 0;
}
int TFIDFModel::load (const std::string &name){
  return 0;
}
