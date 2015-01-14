#include <algorithm>
#include <cmath>
#include "bow.h"
#include "log.h"

using namespace std;
using namespace sm;


static inline bool
__ge (const bow_unit_t&a, const bow_unit_t &b) {
  return a.weight > b.weight;
}


bow_t::bow_t() {

}

size_t
bow_t::size() const{
  return _v.size();
}

void
bow_t::sort(){
  std::sort(_v.begin(), _v.end(), __ge);
}


double
bow_t::_cal_total(){
  double ret = 0.0;
  
  for (vector<bow_unit_t>::const_iterator iter = _v.begin();
       iter != _v.end();
       iter++)
    {
      ret += iter->weight;
    }
  
  _total = ret;

  return _total;
}

double
bow_t::total() const {
  SM_CHECK_RET (NAN, _total != NAN, "total is nan");
  return _total;
}


const bow_unit_t &
bow_t::operator[](size_t s) const{
  return _v[s];
}

const bow_unit_t &
bow_t::at(size_t s) const{
  return _v.at(s);
}

void
bow_t::resize(size_t s) {
  _v.resize(s);
}

void
bow_t::clear() {
  _v.clear();
}

void 
bow_t::reserve(size_t s) { 
  _v.reserve(s); 
}

void
bow_t::pre_handle() {
  sort();
  _cal_total();
  _cal_norm();
}


void
bow_t::push_topk (const bow_unit_t &u, int k) {
  if (size() <= k) {
    _v.push_back(u);
    push_heap (_v.begin(), _v.end(), __ge);
  } else {
    if ( __ge( _v[0], u ) ) {
      _v[0] = u;
      make_heap (_v.begin(), _v.end(), __ge);
    }
  }
}


void
bow_t::push_back(const bow_unit_t &u){
  _v.push_back(u);
}

void
bow_t::unitvec(){
  double n = norm();

  for (vector<bow_unit_t>::iterator iter = _v.begin();
       iter != _v.end();
       iter++){
    iter->weight *= 1.0 / n;
  }
}


double
bow_t::norm() const {
  SM_CHECK_RET (NAN, _norm != NAN, "norm is NAN");
  return _norm;
}

double
bow_t::_cal_norm (){
  double tmp = 0.0;
  for (vector<bow_unit_t>::const_iterator iter = _v.begin();
       iter != _v.end();
       iter++)
    {
      tmp += iter->weight * iter->weight;
    }
  _norm = sqrt (tmp);
  
  return _norm;
}


double 
bow_t::cossim(const bow_t &other, int num_features) const {
  if (size() == 0 || other.size() == 0) return 0.0;
  double mynorm = norm(), othernorm = other.norm();
  double sim = 0.0;
  
  size_t i, j;
  i = j = 0;
  while (i < size() && j < other.size()) {
    const bow_unit_t &u1 = _v[i];
    const bow_unit_t &u2 = other._v[j];
      
    if (u1.id > u2.id) {
      j++;
    } else if (u1.id < u2.id){
      i++;
    } else {
      sim += u1.weight * u2.weight;
      i++;
      j++;
    }
  }

  return sim / (mynorm * othernorm);
}
