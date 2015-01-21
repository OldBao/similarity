#include <iostream>
#include <fstream>
#include <signal.h>
#include "repo.h"
#include "segment.h"
#include "model.h"
#include "similarity.h"
#include "configurable.h"
#include "cfgflag.h"
#include "ub.h"
#include "mola_wrapper.h"

using namespace std;
using namespace sm;

int
main(int argc, char **argv){
  comcfg::Flag flags;

  if ( 0 != flags.init(argc, argv, "m:")){
    cout << comcfg::DEFHELP << endl;
    cout << " -m max documents" << endl;
    return -1;
  }

  if (0 != Configurable::getInstance()->load (flags.cfpath(), flags.cffile(), flags.cfrange())){
    cout << "get config error" << endl;
    return -1;
  }

  const comcfg::Configure& conf = Configurable::getInstance()->getConf();
  if( 0 != ub_load_comlog (flags.cfpath(), conf["global"]["log_conf"].to_cstr())) {
    cout << "load comlog fails" << endl;
    return -1;
  }

  if ( 0 != Segment::getInstance()->load ()){
    cout << "init segment error" << endl;
    return -1;
  }

  if (0 != MolaEngineManager::getInstance()->init(flags.cfpath(), conf["global"]["mola_conf"].to_cstr())){
    cout << "init mola error" << endl;
    return -1;
  }


  Repository repo;
  comcfg::CmdOption &option = flags.opt();
  int max;
  if (option.hasOption("m"))
    max = option["m"].to_int32();
  else
    max = 10;

  int i = 0;
  string url;
  while (cin >> url) {
    i++;
    if (i == max) break;
    repo.addUrl(url);
  }

  repo.waitAllJobDone();
  repo.save ();

  LDAModel ldaModel (&repo.corpus(), &repo.dict());
  ldaModel.train();
  ldaModel.save();
  TopicSimilarity sim (&ldaModel, &repo.corpus(), &repo.dict());
  for (int i = 0; i < ldaModel.getNTopics(); i++) {
    sim.calculate(i);
  }
  sim.waitAllJobDone();
  sim.save();
  return 0;
}
