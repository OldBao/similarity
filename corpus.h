#ifndef CORPUS_H_
#define CORPUS_H_

#include "dictionary.h"
namespace sm {

  class Corpus {
  public:
    Corpus(Dictionary * _dict = NULL, uint64_t version = 0);
    virtual ~Corpus();
    
    int save(const std::string &base, const std::string& basename);
    int load(const std::string &path, const std::string& basename);
    
    int truncate(int num_features = 12);
    int addDoc(uint64_t docid, const bow_t& bow);
    
    size_t size() const;
    size_t maxDocLen() const {return _mdl;}
    int getNTerms() const {return _nterms;}
    const bow_t& operator[] (size_t index) const;
    const bow_t& at(size_t index) const;
    std::vector <bow_t> & getBows() {return _docs;}
    int getIdFromDocid (uint64_t docid);
    uint64_t getDocid(size_t id) const;
  protected:
    std::vector <bow_t> _docs;

    std::vector <uint64_t> _docids;
    std::map<uint64_t, int> _docmap;

    Dictionary *_dict;
    int _nterms;
    size_t _mdl;

    RWLock _docsLock, _docmapLock;
    Lock   _coreLock;
    uint64_t _version;
  };

};
#endif
