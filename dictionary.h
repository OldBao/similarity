#ifndef SM_DICTIONARY_H_
#define SM_DICTIONARY_H_

#include <map>
#include <string>
#include <vector>
#include <list>
#include "document.h"

namespace sm {
  typedef struct bow_unit_s bow_unit_t;
  typedef struct bow_s bow_t;


  struct bow_unit_s {
    int id;
    double weight;
  };

  class bow_s {
  public:
    void push_back(const bow_unit_t &u);

    //this function is for convenient
    size_t size() const {return v.size(); }
    const bow_unit_t &operator[](size_t i) const { return v[i]; }
    const bow_unit_t &at(size_t i) const { return v[i]; }
    void reserve(size_t size) { v.reserve(size); }
    void resize(size_t size) { v.resize(size); }
    void clear() { v.clear(); }

    double total () const{return _total;}
    double cossim (const bow_t & other) const;
    double norm () const;
    void sort();

    std::vector<bow_unit_t> v;
  private:
    double _total; //total weight of unit
  };


  class Dictionary {
  public:
    Dictionary ();
    virtual ~Dictionary();
    int addDocuments(const std::vector <Document> &documents);
    int addDocument(const Document &documents);

    int doc2bow(bow_t* bow, 
                const Document &document, 
                bool update = false);

    const std::string &toString() const{
      return _desc;
    }

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
    void _update();
    std::string _desc;
    int  _nPos, _nnz;
    std::vector<std::wstring> _words;
    std::map <std::wstring, int> _wordmap;
    std::vector<int> _dfs;
    uint64_t _mask;
  };
};
#endif
