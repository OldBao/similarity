#ifndef MODEL_H_
#define MODEL_H_

#include "corpus.h"
#include "dictionary.h"

namespace sm {
  class Model {
  public:
    Model (Corpus *corpus, Dictionary *dict);
    virtual ~Model ();

    virtual int train() = 0;
    virtual int inference (const bow_t &src, bow_t *ret, bool normalized=false) = 0;
    virtual int inference (const Corpus& corpus, Corpus *ret, bool normalized=false) = 0;
    virtual int save (const std::string &name) = 0;
    virtual int load (const std::string &name) = 0;

  protected:
    Dictionary *_dict;
    Corpus *_corpus;
  };

  class TFIDFModel : public Model{
  public:
    TFIDFModel (Corpus *corpus, Dictionary *dict);
    ~TFIDFModel();

    int train();
    int inference (const bow_t &src, bow_t *ret, bool normalized=false);
    int inference (const Corpus& corpus, Corpus *ret, bool normalized=false);
    
    const std::vector<double> idf() {return _idf;}

    virtual int save(const std::string &fname);
    virtual int load(const std::string &fname);
    
  private:
    std::vector<int> _dfs; 
    std::vector<double> _idf;
    int _ndoc, _nnz;
  };
};
#endif
