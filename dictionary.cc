#include <algorithm>
#include "log.h"
#include "dictionary.h"

using namespace sm;
using namespace std;

static const int DICT_MAX_WORD_LEN = 1024;
static const int CHECK_ID = 1000;

static inline bool
_bow_cmp (const bow_unit_t&a, const bow_unit_t &b) {
  return a.weight > b.weight;
}

void
bow_t::sort(){
  std::sort(v.begin(), v.end(), _bow_cmp);
}

void
bow_t::push_back(const bow_unit_t &u){
  if (u.weight > 1e-12) {
    _total += u.weight;
  }

  vector<bow_unit_t>::iterator iter;
  for (iter = v.begin();
       iter != v.end();
       iter++) {
    if (iter->id > u.id) {
      break;
    }
  }

  v.insert(iter, u);
}

double
bow_t::norm () const{
  double total = 0.0;
  
  for (vector<bow_unit_t>::const_iterator iter = v.begin();
       iter != v.end();
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
    const bow_unit_t &u1 = v[i];
    const bow_unit_t &u2 = other.v[j];
      
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
   _nPos(0), _nnz(0)
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

  for (vector<Token>::const_iterator iter = document.getTokens().begin();
       iter != document.getTokens().end();
       iter++){
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
    _nPos += new_pos;
    _nnz += frequencies.size();
    
    _update();
    SM_LOG_DEBUG ("ADD [%zu] new tokens to dict, current [%s]", frequencies.size(),
                  toString().c_str());
  }

  return 0;
}


void
Dictionary::_update() {
  char buffer[4096];
  snprintf (buffer, 4096, "Dictionary (%zu unique tokens)", _words.size());
  _desc.assign(buffer);
}


const std::string & 
Dictionary::operator [](size_t id) const{
  return at(id);
}


const std::string & 
Dictionary::at(size_t id) const{
  //assert (id <= _words.size() && id > 0);

  return _words[id];
}


int
Dictionary::save(const std::string& path, const std::string &basename){
  char filename[PATH_MAX];
  FILE *fp = NULL;
  int ret;

  snprintf (filename, PATH_MAX, "%s/%s.dict.meta", path.c_str(), basename.c_str());
  fp = fopen(filename, "w");
  if (!fp){
    SM_LOG_WARNING ("open dictionary file [%s] error", filename);
    goto error;
  }
  ret = fprintf (fp, "non-zero-entry %d\nword-num %zu\n", _nnz, _words.size());
  if (ret < 0) {
    SM_LOG_WARNING ("write meta content error");
    goto error;
  }

  fclose(fp);

  snprintf (filename, PATH_MAX, "%s/%s.dict", path.c_str(), basename.c_str());
  fp = fopen(filename, "w");
  if (!fp) {
    SM_LOG_WARNING ("open dictionary file [%s] error", filename);
    goto error;
  }

  SM_LOG_DEBUG ("writing %zu word to file [%s]", _words.size(), filename);
  for (size_t i = 0; i < _words.size(); i++)
    {
      if (i % CHECK_ID == 0) {
        fprintf (fp, "%d\n", i);
      }

      ret = fprintf (fp, "%s %d\n", _words[i].c_str(), _dfs[i]);
      if (ret < 0) {
        SM_LOG_WARNING ("write dictionary error");
        goto error;
      }
    }
  
  fclose(fp);
  
  return 0;
 error:
  if (fp) fclose(fp);
  return -1;
}

int
Dictionary::load (const std::string &path, const std::string &base) {
  char filename[PATH_MAX];
  int ret, id, check_id;
  char word[MAX_WORD_LEN];
  vector<string> contents;
  int lineno, wordcount;
  map<int, int> frequencies;
  FILE *fp;
  int dfs;

  snprintf (filename, PATH_MAX, "%s/%s.dict.meta", path.c_str(), base.c_str());
  fp = fopen(filename, "r");
  if (!fp){
    SM_LOG_WARNING ("open dict meta [%s] for load error", filename);
    goto error;
  }
  if ( 2 != fscanf( fp, "non-zero-entry %d\nword-num %d\n", &_nnz, &wordcount) ) {
    SM_LOG_WARNING ("read dict meta content error");
    goto error;
  }
  fclose(fp);


  snprintf (filename, PATH_MAX, "%s/%s.dict", path.c_str(), base.c_str());
  fp = fopen (filename, "r");
  if (!fp) {
    SM_LOG_WARNING ("open dict [%s] for load error", filename);
    goto error;
  }

  id = 0;
  _dfs.resize(wordcount);
  while (1) {
    if (id % CHECK_ID == 0) {
      fscanf (fp, "%d", &check_id);
      if (check_id != id) {
        SM_LOG_WARNING ("check dict id error");
        goto error;
      }
    }

    if (id > wordcount) break;

    ret = fscanf (fp, "%s %d", word, &dfs);
    if (ret == EOF) break;
    if (ret != 2) {
      SM_LOG_WARNING ("dict format error");
      goto error;
    }
    
    _words.push_back(word);
    _wordmap[word] = id;
    _dfs[id] = dfs;
    id++;
  }

  SM_LOG_NOTICE ("loaded dictionary %zu dict", _words.size());

  fclose(fp);
  return 0;

 error:
  if (fp) fclose(fp);
  return -1;
}
