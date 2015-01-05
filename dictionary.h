#ifndef SM_DICTIONARY_H_
#define SM_DICTIONARY_H_

#include <map>
#include <string>
#include <vector>
#include "document.h"

namespace sm {

  class Dictionary {
  public:
    Dictionary ();
    virtual ~Dictionary();
    int addDocuments(const std::vector <Document> &documents);
    int addDocument(const Document &documents);

    int doc2bow(std::vector<std::pair<int, int> >* bow, 
                const Document &document, 
                bool update = false);

    const std::string &toString() const{
      return _desc;
    }

    int length(){
      return _words.size();
    }

    int save();

  private:
    void updateDesc();
    std::string _desc;
    int _nDocs, _nPos, _nnz;
    std::vector<std::string> _words;
    std::map <std::string, int> _wordmap;
    std::map <int, int> _dfs;
    uint64_t _mask;
  };
};
#endif
