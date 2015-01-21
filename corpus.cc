#include <inttypes.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/io/gzip_stream.h>
#include <fstream>
#include "log.h"
#include "corpus.h"
#include "model.h"

#include "interface/corpus.pb.h"

using namespace std;
using namespace sm;

static int getMaxIdFromBow (const bow_t & bow);

Corpus::Corpus(Dictionary *dict, uint64_t version) : 
  _dict(dict), _nterms(0), _mdl(0), _version(version){

}

bool
Corpus::hasDoc (uint64_t docid) {
  bool has = false;
  _docmapLock.AcquireRead();
  if (_docmap.find(docid) != _docmap.end()) {
    SM_LOG_DEBUG ("docid %d" "already add to corpus", _docmap[docid]);
    has = true;
  }
  _docmapLock.Release();
  return has;
}

int
Corpus::addDoc( uint64_t docid, const bow_t &bow ) {
  if (hasDoc(docid)) return 0;
  int newid;
  _docsLock.AcquireWrite();
  _docs.push_back(bow);
  _docids.push_back(docid);
  newid = _docs.size() - 1;
  _docsLock.Release();

  _docmapLock.AcquireWrite();
  /*
  SM_LOG_DEBUG ("Adding docid %" PRIu64 " to map", docid);
  */

  SM_ASSERT (_docmap.find(docid) == _docmap.end(), 
             "docid %" PRIu64 " should not add twice", docid);

  _docmap[docid] = newid;
  _docmapLock.Release();
  
  if (bow.size() > _mdl) {
    _mdl = bow.size();
  }

  int ret = getMaxIdFromBow (bow);
  _coreLock.Acquire();
  if (ret+1 > _nterms) {
    _nterms = ret+1;
  }
  _coreLock.Release();

  return newid;
}


Corpus::~Corpus(){

}

size_t
Corpus::size() const {
  int s;
  Corpus *d = const_cast<Corpus *>(this);
  d->_docsLock.AcquireRead();
  s = _docs.size();
  d->_docsLock.Release();
  return s;
}


const bow_t &
Corpus::at(size_t i) const{
  assert (i < _docs.size());

  Corpus *d = const_cast<Corpus *>(this);
  d->_docsLock.AcquireRead();
  const bow_t &ref =  _docs[i];
  d->_docsLock.Release();
  return ref;
}

const bow_t &
Corpus::operator [](size_t i) const{
  return at(i);
}


int
Corpus::truncate (int num_features) {
  _docsLock.AcquireRead();
  for (size_t i = 0; i < _docs.size(); i++) {
    if ((int)_docs[i].size() > num_features) {
      _docs[i].resize(num_features);
      _docs[i].pre_handle();
    }
  }
  _docsLock.Release();

  return 0;
}


int
Corpus::save(const std::string& path, const std::string &basename){
  smpb::Corpus serial_corpus;
  char fullpath[PATH_MAX];

  if (_version) {
    snprintf (fullpath, PATH_MAX, "%s/%s.corpus.%lu", path.c_str(), basename.c_str(), _version);
  } else {
    snprintf (fullpath, PATH_MAX, "%s/%s.corpus", path.c_str(), basename.c_str());
  }


  ofstream os(fullpath);
  if (!os.is_open()){
    SM_LOG_WARNING ("open out file %s fail", fullpath);
    return -1;
  }
  google::protobuf::io::OstreamOutputStream oos(&os);
  google::protobuf::io::CodedOutputStream cos(&oos);

  serial_corpus.set_version(_version);
  serial_corpus.set_nterms(_nterms);

  for (size_t i = 0; i < _docs.size(); i++) {
    smpb::Doc *serial_doc = serial_corpus.add_docs();
    smpb::Bow *serial_bow = serial_doc->mutable_bow();
    const bow_t& doc = _docs[i];
    serial_doc->set_docid(_docids[i]);

    if (doc.total() != NAN) {
      serial_bow->set_total(doc.total());      
    } 
    if (doc.norm() != NAN) {
      serial_bow->set_norm (doc.norm());
    }

    for(size_t j = 0; j < doc.size(); j++) {
      smpb::BowUnit *serial_bowunit = serial_bow->add_units();
      serial_bowunit->set_id (doc[j].id);
      serial_bowunit->set_weight (doc[j].weight);
      }
  }

  if (!serial_corpus.SerializeToCodedStream(&cos)) {
    SM_LOG_WARNING ("serialize to %s error", fullpath);
    return -1;
  }

  SM_LOG_NOTICE ("save corpus [%s] success, %zu docs, %d terms", 
                 fullpath, _docs.size(), _nterms);
  return 0;
}


int
Corpus::load(const std::string &path, const std::string &basename){
  char fullpath[PATH_MAX];
  
  snprintf (fullpath, PATH_MAX, "%s/%s.corpus", path.c_str(), basename.c_str());
  ifstream is(fullpath);
  if (!is.is_open()) {
    SM_LOG_WARNING ("load file [%s] for reading error", fullpath);
    return -1;
  }

  google::protobuf::io::IstreamInputStream iis(&is);
  google::protobuf::io::CodedInputStream cis(&iis);
  smpb::Corpus deserial_corpus;

  cis.SetTotalBytesLimit(1024*1024*1024, 1024*1024*1024);
  if (!deserial_corpus.ParseFromCodedStream(&cis)){
    SM_LOG_WARNING ("parse corpus file %s error", fullpath);
    return -1;
  }

  if (_version != deserial_corpus.version()){
    SM_LOG_WARNING ("expect corpus version is %lu, file is %lu", _version, deserial_corpus.version());
    return -1;
  }
  _nterms = deserial_corpus.nterms();

  for (int i = 0; i < deserial_corpus.docs_size(); i++) {
    const smpb::Doc &deserial_doc = deserial_corpus.docs(i);
    _docmap[deserial_doc.docid()] = i;

    const smpb::Bow &dbow = deserial_doc.bow();
    bow_t bow;
    for (int j = 0; j < dbow.units_size(); j++) {
      bow_unit_t u;
      u.id = dbow.units(j).id();
      u.weight = dbow.units(j).weight();
      bow.push_back(u);
    }

    if (dbow.has_total()) {
      bow.setTotal (dbow.total());
    } else {
      bow._cal_total();
    }
    if (dbow.norm()) {
      bow.setNorm (dbow.norm());
    } else {
      bow._cal_norm();
    }
    bow.setPreHandled(true);

    if (bow.size() > _mdl) {
      _mdl = bow.size();
    }

    _docs.push_back(bow);
    _docids.push_back(deserial_doc.docid());
  }


  SM_LOG_NOTICE ("Load coprus %s version %" PRIu64 " success: %zu docs, %d terms",
                 fullpath, _version, _docs.size(), _nterms);
  return 0;
}

int
Corpus::getIdFromDocid(uint64_t docid) {
  int id = -1;
  _docmapLock.AcquireRead ();
  if (_docmap.find(docid) != _docmap.end())
    id = _docmap[docid];
  _docmapLock.Release();
  return id;
}


uint64_t
Corpus::getDocid(size_t id) const{
  Corpus *c = const_cast <Corpus *> (this);
  uint64_t docid;
  c->_docsLock.AcquireRead();
  assert (id < _docids.size());
  docid = _docids[id];
  c->_docsLock.Release();
  return docid;
}


static int getMaxIdFromBow (const bow_t & bow) {
  int max = -1;
  for (size_t i = 0; i < bow.size(); i++) {
    const bow_unit_t& u = bow[i];
      if (u.id > max) max = u.id;
    }

  return max;
}
