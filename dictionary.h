#ifndef SM_DICTIONARY_H_
#define SM_DICTIONARY_H_

#include <map>
#include <string>
#include <vector>
#include <list>
#include "document.h"
#include "bow.h"
#include "concurrent.h"

namespace sm {
  class Dictionary {
  public:
    Dictionary (uint64_t version = 0);
    virtual ~Dictionary();
    int addDocuments(const std::vector <Document> &documents);
    int addDocument(const Document &documents);

    int doc2bow(bow_t* bow, 
                const Document &document, 
                bool update = false);

    int addRawDoc(bow_t* bow, const std::vector< std::pair<std::string, double> >& doc);
    int size(){
      return _words.size();
    }

    int save(const std::string &path, const std::string& name, const std::string &encoding="UTF8");
    int load(const std::string &path, const std::string& name);

    int getNnz () { return _nnz; }
    std::vector<int>& getDfs() {return _dfs;}
    const std::wstring& operator [](size_t id) const;
    const std::wstring& at(size_t id) const;
    const std::string at(size_t id, const std::string &encoding) const;

    const std::vector<std::wstring> getWords() {return _words;} 
    const std::map<std::wstring, int> getWordsMap() {return _wordmap;} 
  private:
    int  _nPos, _nnz;
    uint64_t _version;
    std::vector<std::wstring> _words;
    std::map <std::wstring, int> _wordmap;
    std::vector<int> _dfs;
    uint64_t _mask;

    RWLock _wordmapLock;
    Lock _nnzLock;
    RWLock _wordsLock;
  };
};
#endif
