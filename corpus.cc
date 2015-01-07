#include "corpus.h"

using namespace std;
using namespace sm;

static int getMaxIdFromBow (const bow_t & bow);

Corpus::Corpus(Dictionary *dict) : _rw(NULL), _dict(dict){
  if (dict) {
    _nterms = dict->size();
  }
}

int
Corpus::setRWType (int type){
  if (_rw) {
    delete _rw;
  }

  if (type == CORPUS_TYPE_BLEI) {
    _rw = new BleiCorpusRW();
  } else {
    return -1;
  }

  return 0;
}

int
Corpus::addDoc(const bow_t &bow) {
  _docs.push_back(bow);
  if (bow.size() > _mdl) {
    _mdl = bow.size();
  }

  int ret = getMaxIdFromBow (bow);
  if (ret+1 > _nterms) {
    _nterms = ret+1;
  }
  _update();
  return 0;
}

Corpus::~Corpus(){

}

int
Corpus::addDocs (const vector<bow_t> &bows) {
  _docs.insert (_docs.end(), bows.begin(), bows.end());
  for (vector<bow_t>::const_iterator iter = bows.begin(); iter != bows.end(); iter++) {
    if (iter->size() > _mdl) _mdl = iter->size();
  }
  

  _update();
  return 0;
}

size_t
Corpus::size() const {
  return _docs.size();
}


const bow_t &
Corpus::at(size_t i) const{
  assert (i >= 0 && i < _docs.size());

  return _docs[i];
}

const bow_t &
Corpus::operator [](size_t i) const{
  return at(i);
}

void
Corpus::_update(){
  char buffer[4096];
  snprintf (buffer, 4096, "%zu documents", _docs.size());
  _desc.assign (buffer);
}


CorpusRW::CorpusRW(){}
CorpusRW::~CorpusRW(){}


BleiCorpusRW::BleiCorpusRW(){

}

BleiCorpusRW::~BleiCorpusRW(){

}
int
BleiCorpusRW::serialize(const std::vector<bow_t> &bows, std::string *dest) {

}


int 
BleiCorpusRW::deserialize(const std::string &src, std::vector <bow_t> *bows ) {

}


static int getMaxIdFromBow (const bow_t & bow) {
  int max = -1;
  for (size_t i = 0; i < bow.size(); i++) {
    const bow_unit_t & u = bow[i];
      if (u.id > max) max = u.id;
    }

  return max;
}
