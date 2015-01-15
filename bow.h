#ifndef BOW_H_
#define BOW_H_

#include <vector>
#include <string>
namespace sm {
  typedef struct bow_unit_s bow_unit_t;

  struct bow_unit_s {
    int id;
    double weight;
  };

  class bow_t {
  public:
    bow_t ();
    void push_topk(const bow_unit_t &u, int k);
    void push_back(const bow_unit_t &u);

    //this function is for convenient
    size_t size() const;
    const bow_unit_t &operator[](size_t i) const;
    const bow_unit_t &at(size_t i) const;
    void reserve(size_t size);
    void resize(size_t size);
    void clear();
    void pre_handle();
    void setPreHandled(bool t) {_pre_handled = true;}

    void setTotal(double total) { _total = total;}
    double total () const;
    double cossim ( const bow_t & other) const;
    void setNorm(double norm){_norm = norm;}
    double norm () const;
    void unitvec();
    void sort();

  private:
    double _cal_total();
    double _cal_norm();

    std::vector<bow_unit_t> _v;
    double _norm, _total;

    bool _pre_handled;
  };



};
#endif
