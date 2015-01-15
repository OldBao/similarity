#include <algorithm>
#include <cmath>
#include "bow.h"
#include "log.h"

using namespace std;
using namespace sm;


static inline bool
__gt (const bow_unit_t&a, const bow_unit_t &b) {
  return a.weight > b.weight;
}

static inline bool
__lt (const bow_unit_t&a, const bow_unit_t &b) {
  return a.weight < b.weight;
}


bow_t::bow_t() : _total(NAN), _norm(NAN), _pre_handled(false){

}

size_t
bow_t::size() const{
  return _v.size();
}

void
bow_t::sort(){
  std::sort(_v.begin(), _v.end(), __gt);
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
  SM_ASSERT(_pre_handled, "please call pre_handle function ;-)");
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
  _pre_handled = true;
}


void
bow_t::push_topk (const bow_unit_t &u, int k) {
  if (size() < k) {
    _v.push_back(u);
    push_heap ( _v.begin(), _v.end(), __gt);
  } else {
    if ( __gt( u, _v[0]) ) {
      _v[0] = u;
      make_heap (_v.begin(), _v.end(), __gt);
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
  SM_ASSERT(_pre_handled, "please call pre_handle function ;-)");
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
bow_t::cossim(const bow_t &other) const {
  if (size() == 0 || other.size() == 0) return 0.0;
  double mynorm = norm(), othernorm = other.norm();
  double sim = 0.0;
  
  for (int i = 0; i < size(); i++) {
    const bow_unit_t& u = _v[i];

    for (int j = 0; j < other.size(); j++) {
      if (other[j].id == u.id) {
        sim += u.weight * other[j].weight;
        break;
      }
    }
  }

  return sim / (mynorm * othernorm);
}
