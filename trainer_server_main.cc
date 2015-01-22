#include <iostream>

#include <signal.h>

#include "configurable.h"
#include "trainer_server.h"
#include "mola_wrapper.h"
#include "segment.h"
#include "cmdline.h"

using namespace std;
using namespace sm;

#define MYNAME "trainer"
static Cmdline cmdline(MYNAME);

void
show_version(){
  cout << cmdline.getDesc() << endl;
}


int
main(int argc, char **argv) {
  int ret;
  signal (SIGPIPE, SIG_IGN);
  comcfg::Flag flags(show_version);

  if ( 0 != flags.init(argc, argv)){
    cout << comcfg::DEFHELP << endl;
    return -1;
  }

  cmdline.change_proc_name(argc, argv);
  ret = Configurable::getInstance()->load (flags.cfpath(), flags.cffile(), flags.cfrange());
  if ( ret != 0 ) {
    if ( ret == comcfg::CONFIG_ERROR ) {
      cout << "load config " << flags.cfpath() << "/" << flags.cffile() << " fails" << endl;

    } else if ( ret == comcfg::CONSTRAINT_ERROR ) {
      cout << "load range file " << flags.cfrange() << "fails" << endl;
    }
    return -1;
  }

  comcfg::Configure conf = Configurable::getInstance()->getConf();
  ret = ub_load_comlog (flags.cfpath(), conf["global"]["log_conf"].to_cstr());
  if (ret != 0) {
    cout << "load comlog fails" << endl;
    return -1;
  }

  if (0 != MolaEngineManager::getInstance()->init(flags.cfpath(), conf["global"]["mola_conf"].to_cstr())){
    cout << "init mola error" << endl;
    return -1;
  }

  if (0 != Segment::getInstance()->load ()) {
    SM_LOG_FATAL ("load segment error");
    return -1;
  }

  comcfg::Configure trainer_server_conf;
  if (0 != trainer_server_conf.load(flags.cfpath(), conf["global"]["trainer_server_conf"].to_cstr())) {
    SM_LOG_FATAL ("load trainer server conf error");
    return -1;
  }

  ub::NetReactor *reactor = new ub::NetReactor;
  if (reactor->load (conf["reactor"]) != 0) {
    SM_LOG_FATAL ("load aserver reactor fails");
    delete reactor;
    return -1;
  }

  //TrainServer server(reactor);
  
}
