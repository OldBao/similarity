#include "log.h"
#include "corpus.h"
#include "model.h"

using namespace std;
using namespace sm;

static int getMaxIdFromBow (const bow_t & bow);

Corpus::Corpus(Dictionary *dict) : _dict(dict), _nterms(0), _mdl(0){
}

int
Corpus::addDoc(const bow_t &bow) {
  _docs.push_back(bow);
  if (bow.size() > _mdl) {
    _mdl = bow.size();
  }

  int ret = getMaxIdFromBow (bow);
  if (ret+1 > _nterms) {
    _nterms = ret+1;
  }

  _update();
  return 0;
}

Corpus::~Corpus(){

}

int
Corpus::addDocs (const vector<bow_t> &bows) {
  _docs.insert (_docs.end(), bows.begin(), bows.end());
  for (vector<bow_t>::const_iterator iter = bows.begin(); iter != bows.end(); iter++) {
    if (iter->size() > _mdl) _mdl = iter->size();
  }
  

  _update();
  return 0;
}

size_t
Corpus::size() const {
  return _docs.size();
}


const bow_t &
Corpus::at(size_t i) const{
  assert (i < _docs.size());

  return _docs[i];
}

const bow_t &
Corpus::operator [](size_t i) const{
  return at(i);
}

void
Corpus::_update(){
  char buffer[4096];
  snprintf (buffer, 4096, "%zu documents", _docs.size());
  _desc.assign (buffer);
}

int
Corpus::save(const std::string& path, const std::string &basename){
  char filename[PATH_MAX];
  FILE *fp = NULL;
  int ret;
  bow_t bow;

  snprintf (filename, PATH_MAX, "%s/%s.corpus", path.c_str(), basename.c_str());
  fp = fopen(filename, "w");
  if (!fp) {
    SM_LOG_WARNING ("OPEN corpus file [%s] for writing error", filename);
    goto error;
  }

  for (size_t i = 0; i < _docs.size(); i++) {
    const bow_t& b = _docs[i];
    if (b.size() == 0) continue;
    ret = fprintf (fp, "%zu ", b.size());
    
    if (ret < 0){
      SM_LOG_WARNING ("write corpus file error");
      goto error;
    }

    for (size_t j = 0; j < b.size(); j++) {
      ret = fprintf (fp, "%d:%.10lf ", b.v[j].id, b.v[j].weight);
      if (ret < 0) {
        SM_LOG_WARNING ("write corpus file error");
        goto error;
      }
    }
    fprintf (fp, "\n");
  }

  fclose(fp);
  return  0;

 error:
  if (fp) fclose (fp);
  return -1;
}


int
Corpus::load(const std::string &path, const std::string &basename){
  char filename[PATH_MAX];
  FILE *fp = NULL;
  int ret;
  int length;
  int line = 0;
  int id;
  double weight;
  bow_t bow;
  bow_unit_t u;

  
  snprintf (filename, PATH_MAX, "%s/%s.corpus", path.c_str(), basename.c_str());
  fp = fopen(filename, "r");
  if (!fp) {
    SM_LOG_WARNING ("OPEN file [%s] for reading error", filename);
    goto error;
  }

  line = 0;
  while (1) {
    ret = fscanf (fp, "%10d", &length);
    line++;
    if (ret == EOF) break;
    else if (ret != 1) {
      SM_LOG_WARNING ("file [%s] format error", filename);
      goto error;
    }

    if (length == 0) continue;

    bow.clear();
    for (int n = 0; n < length; n++) {
      ret = fscanf (fp, "%d:%lf", &id, &weight);
      if (ret != 2) {
        SM_LOG_WARNING("file [%s:%d:%d] format error", filename, line, n);
        goto error;
      }
      u.id = id;
      u.weight = weight;
      bow.push_back(u);
    }

    assert(bow.size() == length);

    addDoc(bow);
  }
  
  fclose(fp);

  SM_LOG_NOTICE ("load corpus: %d docs, %d terms", size(), _nterms);
  
  return 0;
  
 error:
  if (fp) fclose(fp);
  return -1;
}

static int getMaxIdFromBow (const bow_t & bow) {
  int max = -1;
  for (size_t i = 0; i < bow.size(); i++) {
    const bow_unit_t& u = bow.v[i];
      if (u.id > max) max = u.id;
    }

  return max;
}
