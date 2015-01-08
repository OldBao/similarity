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
    size_t size() const {return _v.size(); }
    const bow_unit_t &operator[](size_t i) const { return _v[i]; }
    void reserve(size_t size) { _v.reserve(size); }
    void resize(size_t size) { _v.resize(size); }
    void clear() { _v.clear(); }
    double total () const{return _total;}
    double cossim (const bow_t & other) const;
    double norm () const;
  private:
    std::vector<bow_unit_t> _v;
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

    int save();

    int getNDoc () { return _nDocs; }
    int getNnz () { return _nnz; }
    std::vector<int>& getDfs() {return _dfs;}
    std::string& operator [](size_t id);

  private:
    void _update();
    std::string _desc;
    int _nDocs, _nPos, _nnz;
    std::vector<std::string> _words;
    std::map <std::string, int> _wordmap;
    std::vector<int> _dfs;
    uint64_t _mask;
  };
};
#endif
