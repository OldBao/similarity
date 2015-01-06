#ifndef SM_DICTIONARY_H_
#define SM_DICTIONARY_H_

#include <map>
#include <string>
#include <vector>
#include "document.h"

namespace sm {
  typedef struct bow_unit_s bow_unit_t;
  typedef std::vector<bow_unit_t> bow_t;

  struct bow_unit_s {
    int id;
    double weight;
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

    int length(){
      return _words.size();
    }

    int save();

    int getNDoc () { return _nDocs; }
    int getNnz () { return _nnz; }
    std::vector<int>& getDfs() {return _dfs;}
    std::string& operator [](size_t id);

  private:
    void updateDesc();
    std::string _desc;
    int _nDocs, _nPos, _nnz;
    std::vector<std::string> _words;
    std::map <std::string, int> _wordmap;
    std::vector<int> _dfs;
    uint64_t _mask;
  };
};
#endif
