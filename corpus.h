#ifndef CORPUS_H_
#define CORPUS_H_

#include "dictionary.h"
namespace sm {

  const int CORPUS_TYPE_BLEI = 1;
  class CorpusRW;

  class Corpus {
  public:
    Corpus(Dictionary * _dict = NULL);
    virtual ~Corpus();
    
    int setRWType (int type);
    static int save(const std::string& fname);
    static int load(const std::string& fname);
    
    const std::string &toString () const {
      return _desc;
    }
    
    int addDoc(const bow_t& bow);
    int addDocs (const std::vector<bow_t>& bows);
    
    size_t size() const;
    size_t maxDocLen() const {return _mdl;}
    int getNTerms() const {return _nterms;}
    const bow_t& operator[] (size_t index) const;
    const bow_t& at(size_t index) const;
    std::vector <bow_t> & getBows() {return _docs;}
    
  protected:
    void _update();
    CorpusRW *_rw;
    std::vector <bow_t> _docs;
    std::string _desc;
    Dictionary *_dict;
    int _nterms;
    int _mdl;
  };
  
  class CorpusRW {
  public:
    CorpusRW();
    virtual ~CorpusRW();
    virtual int serialize(const std::vector<bow_t>& bows, std::string* dest) = 0;
    virtual int deserialize(const std::string &src, std::vector <bow_t> *bows ) = 0;
  };
  
  
  class BleiCorpusRW : public CorpusRW {
  public:
    BleiCorpusRW();
    virtual ~BleiCorpusRW();
    virtual int serialize(const std::vector<bow_t> &bows, std::string *dest);
    virtual int deserialize(const std::string &src, std::vector <bow_t> *bows );
  };
  
};
#endif
