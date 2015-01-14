#ifndef CORPUS_H_
#define CORPUS_H_

#include "dictionary.h"
namespace sm {

  class Corpus {
  public:
    Corpus(Dictionary * _dict = NULL);
    virtual ~Corpus();
    
    int save(const std::string &base, const std::string& basename);
    int load(const std::string &path, const std::string& basename);
    
    const std::string &toString () const {
      return _desc;
    }
    
    int truncate(int num_features = 12);
    int addDoc(const bow_t& bow);
    
    size_t size() const;
    size_t maxDocLen() const {return _mdl;}
    int getNTerms() const {return _nterms;}
    const bow_t& operator[] (size_t index) const;
    const bow_t& at(size_t index) const;
    std::vector <bow_t> & getBows() {return _docs;}
    
  protected:
    void _update();
    std::vector <bow_t> _docs;
    std::string _desc;
    Dictionary *_dict;
    int _nterms;
    size_t _mdl;
  };

};
#endif
