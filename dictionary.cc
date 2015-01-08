#include "log.h"
#include "dictionary.h"

using namespace sm;
using namespace std;

void
bow_t::push_back(const bow_unit_t &u){
  if (u.weight > 1e-12) {
    _total += u.weight;
  }

  vector<bow_unit_t>::iterator iter;
  for (iter = _v.begin();
       iter != _v.end();
       iter++) {
    if (iter->id > u.id) {
      break;
    }
  }

  _v.insert(iter, u);
}

double
bow_t::norm () const{
  double total = 0.0;
  
  for (vector<bow_unit_t>::const_iterator iter = _v.begin();
       iter != _v.end();
       iter++)
    {
      total += iter->weight * iter->weight;
    }

  return sqrt (total);
}


double 
bow_t::cossim(const bow_t &other) const {
  if (size() == 0 || other.size() == 0) return 0.0;
  double mynorm = norm(), othernorm = other.norm();
  double sim = 0.0;
  
  size_t i, j;
  i = j = 0;
  while (i < size() && j < other.size()) {
    const bow_unit_t &u1 = _v[i];
    const bow_unit_t &u2 = other[j];
      
    if (u1.id > u2.id) {
      j++;
    } else if (u1.id < u2.id){
      i++;
    } else {
      sim += u1.weight * u2.weight;
      i++;
      j++;
    }
  }

  return sim / (mynorm * othernorm);
}


Dictionary::Dictionary () :
  _nDocs(0), _nPos(0), _nnz(0)
{

}

Dictionary::~Dictionary(){

}

int
Dictionary::addDocument(const Document &document) {
  return doc2bow(NULL, document, true);
}


int
Dictionary::addDocuments(const vector<Document> &documents){
  int ret;

  for (vector<Document>::const_iterator iter = documents.begin();
       iter != documents.end();
       iter++)
    {
      if (0 != (ret = doc2bow(NULL, *iter, true))){
        return ret;
      }
    }

  return 0;
}


int
Dictionary::doc2bow (bow_t *bow, const Document& document, bool update) {
  int id = -1;
  map <int, int> frequencies;
  int new_pos;

  for (vector <Token>::const_iterator iter = document.getTokens().begin();
       iter != document.getTokens().end();
       iter++)
    {
      if (iter->length() == 1) {
        continue;
      }
      if (_wordmap.find(iter->content) != _wordmap.end()) {
        id = _wordmap[iter->content];
      } else {
        if (update) {
          id = _words.size();
          _words.push_back (iter->content);
          _wordmap[iter->content] = id;
        }
      }

      new_pos++;

      if (id != -1) {
        if (frequencies.find (id) != frequencies.end()) {
          frequencies[id]++;
        } else {
          frequencies[id] = 1;
        }
      }
    }


  if (update)
    _dfs.resize (_words.size());

  /// update document frequence stat and bow
  for (map<int, int>::iterator iter = frequencies.begin();
       iter != frequencies.end();
       iter++)
    {
      if (update) {
        _dfs[iter->first]++;
      }

      if (bow) {
        bow_unit_t unit;
        unit.id = iter->first;
        unit.weight = iter->second;
        bow->push_back(unit);
      }
    }

  if (update) {
    _nDocs++;
    _nPos += new_pos;
    _nnz += frequencies.size();
    
    _update();
    SM_LOG_DEBUG ("ADD [%zu] new tokens to dict, current [%s]", frequencies.size(),
                  toString().c_str());
  }

  return 0;
}


void
Dictionary::_update(){
  char buffer[4096];
  snprintf (buffer, 4096, "Dictionary (%zu unique tokens)", _words.size());
  _desc.assign(buffer);
}


std::string & 
Dictionary::operator [](size_t id) {
  //assert (id <= _words.size() && id > 0);

  return _words[id];
}
