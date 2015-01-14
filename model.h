#ifndef MODEL_H_
#define MODEL_H_

#include "corpus.h"
#include "dictionary.h"

#include "test/test_lda.h"

#include <vector>
namespace sm {
  class Model {
  public:
    Model (Corpus *corpus, Dictionary *dict);
    virtual ~Model ();

    virtual int train() = 0;
    virtual int inference (const bow_t &src, bow_t *ret, bool normalized=false) = 0;
    virtual int inference (const Corpus& corpus, Corpus *ret, bool normalized=false) = 0;
    virtual int save (const std::string &path, const std::string &name) = 0;
    virtual int load (const std::string &path, const std::string &name) = 0;

  protected:
    Dictionary *_dict;
    Corpus *_corpus;
  };

  class TopicModel : public Model {
  public:
    TopicModel (Corpus *corpus, Dictionary *dict) : Model(corpus, dict){}
    virtual ~TopicModel(){}

    virtual int getMostLikelyTopicOfDoc (bow_t *ret, int docid, double threshold=0.5, int max_result=3) = 0;

    virtual int getNTopics() = 0;
    virtual int getDocsOfTopic (vector<int> *, int topicid = -1) = 0;
  };


  class TFIDFModel : public Model{
  private:
    std::vector<int> _dfs; 
    std::vector<double> _idf;
    int _ndoc, _nnz;

  public:
    TFIDFModel (Corpus *corpus, Dictionary *dict);
    virtual ~TFIDFModel();

    int train();
    int inference (const bow_t &src, bow_t *ret, bool normalized=false);
    int inference (const Corpus& corpus, Corpus *ret, bool normalized=false);
    
    const std::vector<double> idf() {return _idf;}

    virtual int save(const std::string &path, const std::string &basename) { return 0;}
    virtual int load(const std::string &path, const std::string &basename) {return 0;}
  };


  class LDAState {
  public:
    LDAState (const Corpus &corpus, int topics, int num_init = 1);
    virtual ~LDAState();

    void zero();
    double **class_word;
    double  *class_total;
    double   alpha_suffstats;
    int ndocs;
  private:
    int _nterms, _ntopics;
    LDAState();
    LDAState(LDAState &);
  };

  class LDAModel : public TopicModel {
  public:
    LDAModel (Corpus *corpus, Dictionary *dict);
    virtual ~LDAModel ();
    
    int train();
    int inference (const bow_t &src, bow_t *ret, bool normalized=false);
    int inference (const Corpus& corpus, Corpus *ret, bool normalized=false);

    virtual int getNTopics() {return _ntopics;}
    virtual int save(const std::string &path, const std::string &name);
    virtual int load(const std::string &path, const std::string &name);

    virtual int getMostLikelyTopicOfDoc (bow_t *ret, int docid, double threshold=0.5, int max_result=3);
    virtual int getDocsOfTopic (vector<int> *ret, int topicid = -1);
    void getHotestWordsDesc(std::string *desc, int topicid, int nwords = 10, const std::string &encoding="utf8");
    void getHotestWords(bow_t *bow, int topicid, int nwords = 10);

  private:
    void _init_prob();
    void _em (LDAState *ss);
    void _mle (LDAState *s, int estimate_alpha);
    double _e_step (const bow_t &doc, double *gamma, double **phi, LDAState *ss);
    double _infer(const bow_t& doc, double* var_gamma, double** phi);
    double _compute_likelihood(const bow_t& doc, double** phi, double* var_gamma);
    double _opt_alpha(double ss, int D, int K);
    int _cluster();
    double _alpha;
    double _init_alpha;
    int _estimate_alpha;
    int _var_max_iter;
    double _var_converged;
    int _em_max_iter;
    float _em_converged;
    int _max_alpha_iter;
    double _newton_threshold;
    int _ntopics, _nterms;
    double **_log_prob_w;
    double **_var_gamma;
    double **_phi;

    vector<vector<int> > _topics; 

    //test 
    FRIEND_TEST (LDATestCase, TestTopkHotwords);
  };
};
#endif
