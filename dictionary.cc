#include <algorithm>
#include "log.h"
#include "dictionary.h"
#include "encoding.h"

using namespace sm;
using namespace std;

static const int DICT_MAX_WORD_LEN = 1024;
#define ESCAPE  ' '


char *
myfgets(char *buf, size_t len, FILE *fp) {
  char *f;
  f = fgets(buf, len, fp);
  if (!f) return f;

  if ((f = strchr(buf, '\n')) != NULL) *f = '\0';
  return f;
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
  assert (!bow || bow->size() == 0);

  int id = -1;
  map <int, int> frequencies;
  int new_pos;

  for (vector<Token>::const_iterator iter = document.getTokens().begin();
       iter != document.getTokens().end();
       iter++){
    {
      //filter words only contains one character
      if (iter->content.length() == 1) {
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


const std::wstring & 
Dictionary::operator [](size_t id) const{
  return at(id);
}


const std::string
Dictionary::at(size_t id, const std::string &encoding) const {
  string buffer;

  rwtrans_func_t *w = get_rwtrans(encoding);
  w(_words[id], &buffer);
  return buffer;
}

const std::wstring & 
Dictionary::at(size_t id) const {
  //assert (id <= _words.size() && id > 0);
  return _words[id];
}


int
Dictionary::save(const std::string& path, const std::string &basename, const std::string &encoding){
  char filename[PATH_MAX];
  FILE *fp = NULL;
  int ret;
  rwtrans_func_t *w;

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

  ret = fprintf (fp, "%s\n", encoding.c_str());
  if (ret < 0) {
    SM_LOG_WARNING ("write to file error");
    goto error;
  }

  SM_LOG_DEBUG ("writing %zu word to file [%s]", _words.size(), filename);
  w = get_rwtrans(encoding);
  assert (w);
  for (size_t i = 0; i < _words.size(); i++)
    {
      string buffer;
      w(_words[i], &buffer);
      ret = fprintf (fp, "%d%c%s\n", _dfs[i], ESCAPE, buffer.c_str());
      if (-1 == ret) {
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
  char filename[PATH_MAX], word[MAX_WORD_LEN];
  int id, line, dfs, wordcount;
  vector<string> contents;
  map<int, int> frequencies;
  FILE *fp;
  wtrans_func_t *w;

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

  id = 0, line = 0;
  _dfs.resize(wordcount);

  //get file encoding
  if (NULL == myfgets (word, MAX_WORD_LEN, fp)) {
    SM_LOG_WARNING("get encoding line error");
    goto error;
  }

  w = get_wtrans(word);
  if (!w) {
    SM_LOG_WARNING("invalid encoding : %s", word);
    goto error;
  }

  while (1) {
    wstring buffer;
    if (id > wordcount) break;
    memset (word, 0, sizeof word);
    if (myfgets (word, MAX_WORD_LEN, fp) == NULL ){
      break;
    }
    char *sep = strchr(word, ESCAPE);
    if (!sep) {
      SM_LOG_WARNING("dict format error in line %d", line);
      goto error;
    }
    *sep = '\0';

    if (1 != sscanf (word, "%d", &dfs)){
      SM_LOG_WARNING("dict format error in line %d", line);
      goto error;
    }

    w(sep+1, &buffer);
    line++;
    
    _words.push_back(buffer);
    _wordmap[buffer] = id;
    _dfs[id] = dfs;
    id++;
  }

  fclose(fp);
  return 0;

 error:
  if (fp) fclose(fp);
  return -1;
}
