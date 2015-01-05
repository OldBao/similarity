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
    int doc2bow(std::vector<std::pair<int, int> >* bow, 
                const Document &document, 
                bool update = false);
    int save();

  private:
    int _nDocs, _nPos, _nnz;
    std::vector<std::string> _words;
    std::map <std::string, int> _wordmap;
    std::map <int, int> _dfs;
  };
};
#endif
