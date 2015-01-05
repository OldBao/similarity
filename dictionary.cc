#include "log.h"
#include "dictionary.h"

using namespace sm;
using namespace std;

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
    
    updateDesc();
    SM_LOG_DEBUG ("ADD [%zu] new tokens to dict, current [%s]", frequencies.size(),
                  toString().c_str());
  }

  return 0;
}


void
Dictionary::updateDesc(){
  char buffer[4096];
  snprintf (buffer, 4096, "Dictionary (%zu unique tokens)", _words.size());
  _desc.assign(buffer);
}


std::string & 
Dictionary::operator [](size_t id) {
  //assert (id <= _words.size() && id > 0);

  return _words[id];
}
