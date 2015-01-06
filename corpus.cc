#include "corpus.h"

using namespace std;
using namespace sm;

Corpus::Corpus(Dictionary *dict) : _rw(NULL), _dict(dict){

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
  updateDesc();
  return 0;
}

Corpus::~Corpus(){

}

int
Corpus::addDocs (const vector<bow_t> &bows) {
  _docs.insert (_docs.end(), bows.begin(), bows.end());
  updateDesc();
  return 0;
}

size_t
Corpus::size(){
  return _docs.size();
}


bow_t &
Corpus::at(size_t i) {
  assert (i >= 0 && i < _docs.size());

  return _docs[i];
}

bow_t &
Corpus::operator [](size_t i) {
  return at(i);
}

void
Corpus::updateDesc(){
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
