#ifndef MODEL_H_
#define MODEL_H_

namespace sm {
  class Model {
  public:
    Model (Corpus *corpus, Dictionary *dict);
    virtual ~Model ();

    virtual int train() = 0;
    virtual int save (const std::string &name) = 0;
    virtual int load (const std::string &name) = 0;

  private:
    Corpus *_corpus;
    Dictionary *_dict
  };

  class TFIDFModel {
    TFIDFModel (const Corpus &corpus, Dictionary *dict);
    virtual ~TFIDFModel ();

    virtual int train();
    virtual int save(const std::string &fname);
    virtual int load(const std::string &fname);
    
  };
    
  private:
  int _ndoc, nnz;
  std::vector<int> _dfs; 
  };
};
#endif
