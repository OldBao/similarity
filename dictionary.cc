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
Dictionary::doc2bow (vector <pair<int, int> > *bow, const Document& document, bool update) {
  int id = -1;
  map <int, int> frequencies;

  for (vector <Token>::const_iterator iter = document.getTokens().begin();
       iter != document.getTokens().end();
       iter++)
    {
      if (_wordmap.find(iter->content) != _wordmap.end()) {
        id = _wordmap[iter->content];
      } else {
        if (update) {
          id = _words.size();
          _words.push_back (iter->content);
          _wordmap[iter->content] = id;
        }
      }    
  
      if (frequencies.find (id) != frequencies.end()) {
        frequencies[id]++;
      } else {
        frequencies[id] = 1;
      }
    }

  if (update) {
    _nDocs++;
    _nPos += document.getTokens().size();
    _nnz += frequencies.size();
    for (map<int, int>::iterator iter = frequencies.begin();
         iter != frequencies.end();
         iter++)
      {
        if (_dfs.find(iter->first) != _dfs.end()){
          _dfs[iter->first]++;
        } else {
          _dfs[iter->first] = 1;
        }
      }
  }

  return 0;
}
