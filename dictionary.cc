#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/io/gzip_stream.h>
#include <fstream>
#include <algorithm>
#include "log.h"
#include "dictionary.h"
#include "encoding.h"
#include "interface/dict.pb.h"

using namespace sm;
using namespace std;

Dictionary::Dictionary (uint64_t version) :
  _nPos(0), _nnz(0), _version(version)
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
Dictionary::addRawDoc (bow_t *bow, const vector< pair<string, double> >&doc) {
  assert (bow);

  for (vector< pair<string, double> >::const_iterator iter = doc.begin();
       iter != doc.end();
       iter++) 
    {
      std::wstring wcontent;
      int id = -1;
      map<wstring, int>::iterator tmp_iter;
      encoding_utf8_to_wchar (iter->first, &wcontent);

      _wordmapLock.AcquireRead();
      if ((tmp_iter = _wordmap.find(wcontent)) != _wordmap.end()) {
        id = tmp_iter->second;
      } 
      _wordmapLock.Release();
      
      if (id == -1) {
        _wordsLock.AcquireWrite();
        id = _words.size();
        _words.push_back (wcontent);
        _wordsLock.Release();
        _wordmapLock.AcquireWrite();
        _wordmap[wcontent] = id;
        _wordmapLock.Release();
      }

      bow_unit_t unit;
      unit.id = id;
      unit.weight = iter->second;
      bow->push_back(unit);
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

      _wordmapLock.AcquireRead();
      if (_wordmap.find(iter->content) != _wordmap.end()) {
        id = _wordmap[iter->content];
        _wordmapLock.Release();
      } else {
        if (update) {
          _wordsLock.AcquireWrite();
          id = _words.size();
          _words.push_back (iter->content);
          _dfs.resize(_words.size());
          _wordsLock.Release();
          
          _wordmapLock.Release();
          _wordmapLock.AcquireWrite();
          _wordmap[iter->content] = id;
        }
        _wordmapLock.Release();
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



  /// update document frequence stat and bow
  for (map<int, int>::iterator iter = frequencies.begin();
       iter != frequencies.end();
       iter++)
    {
      if (update) {  
        _wordsLock.AcquireWrite();
        _dfs[iter->first]++;
        _wordsLock.Release();
      }

      if (bow) {
        bow_unit_t unit;
        unit.id = iter->first;
        unit.weight = iter->second;
        bow->push_back(unit);
      }
    }

  if (update) {
    _nnzLock.Acquire();
    _nPos += new_pos;
    _nnz += frequencies.size();
    _nnzLock.Release();
  }

  return 0;
}


const std::wstring & 
Dictionary::operator [](size_t id) const{
  return at(id);
}


const std::string
Dictionary::at(size_t id, const std::string &encoding) const {
  string buffer;

  rwtrans_func_t *w = get_rwtrans(encoding);

  Dictionary *d = const_cast<Dictionary *>(this);
  d->_wordsLock.AcquireRead();
  w(_words[id], &buffer);
  d->_wordsLock.Release();

  return buffer;
}


const std::wstring & 
Dictionary::at(size_t id) const {
  Dictionary *d = const_cast<Dictionary *>(this);
  d->_wordsLock.AcquireRead();
  return _words[id];
  d->_wordsLock.Release();
}


int
Dictionary::save(const std::string& path, const std::string &basename, const std::string &encoding){
  rwtrans_func_t *w;
  w = get_rwtrans(encoding);
  assert (w);

  char fullpath[PATH_MAX];
  if (_version != 0) {
    snprintf (fullpath, PATH_MAX, "%s/%s.dict.%lu", path.c_str(), basename.c_str(), _version);
  } else {
    snprintf (fullpath, PATH_MAX, "%s/%s.dict", path.c_str(), basename.c_str());
  }

  ofstream os(fullpath);
  if (!os.is_open()) return -1;
  google::protobuf::io::OstreamOutputStream oos(&os);
  google::protobuf::io::GzipOutputStream gzips(&oos);

  smpb::Dictionary serial_dict;

  for (size_t i = 0; i < _words.size(); i++) {
    string buffer;
    smpb::Word *word = serial_dict.add_words();
    assert ( 0 == w(_words[i], &buffer));
    word->set_content(buffer);
    word->set_dfs(_dfs[i]);
  }

  serial_dict.set_nnz(_nnz);
  serial_dict.set_npos(_nPos);
  serial_dict.set_version(_version);

  if (!serial_dict.SerializeToZeroCopyStream(&gzips)){
    SM_LOG_WARNING ("serialize to os error");
    return -1;
  }

  SM_LOG_NOTICE ("save dictionary %s success", fullpath);
  return 0;
}

int
Dictionary::load (const std::string &path, const std::string &base) {
  char fullpath[PATH_MAX];
  if (_version != 0) {
    snprintf (fullpath, PATH_MAX, "%s/%s.dict.%lu", path.c_str(), base.c_str(), _version);
  } else {
    snprintf (fullpath, PATH_MAX, "%s/%s.dict", path.c_str(), base.c_str());
  }

  ifstream is(fullpath);
  if (!is.is_open()) {
    SM_LOG_WARNING ("open dict file %s error", fullpath);
    return -1;
  }
  google::protobuf::io::IstreamInputStream iis(&is);
  google::protobuf::io::GzipInputStream gzips(&iis);

  smpb::Dictionary deserial_dict;
  if (!deserial_dict.ParseFromZeroCopyStream(&gzips)) {
    SM_LOG_WARNING ("parse dict %s error", fullpath);
    return -1;
  }

  if (_version != deserial_dict.version()){
    SM_LOG_WARNING ("expect dict version is %lu, file is %lu", _version, deserial_dict.version());
    return -1;
  }
  
  for (int i = 0; i < deserial_dict.words_size(); i++) {
    const smpb::Word &word = deserial_dict.words(i);
    wstring buffer;
    encoding_utf8_to_wchar(word.content(), &buffer);
    if (word.has_dfs())
      _dfs.push_back (word.dfs());
    _words.push_back (buffer);
    _wordmap[buffer] = i;
  }

  if (deserial_dict.has_npos()) {
    _nPos = deserial_dict.npos();
  }

  if (deserial_dict.has_nnz()){
    _nnz = deserial_dict.nnz();
  }

  SM_LOG_NOTICE ("load dictionary [%s] version %lu success, %zu words", 
                 fullpath, _version, _words.size());
  return 0;
}
