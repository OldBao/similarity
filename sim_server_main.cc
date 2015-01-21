#include <iostream>

#include <signal.h>

#include "ub.h"
#include "configurable.h"
#include "mola_wrapper.h"
#include "segment.h"
#include "cmdline.h"
#include "sim_server.h"
using namespace sm;

#define MYNAME "sim_server"
static Cmdline *cmdline;


void
show_version(){
  cout << cmdline->getDesc() << endl;
}


static
int check(void *){
  SimServerDataManager::getInstance()->checkVersion ();
  
  return 0;
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

  cmdline = new Cmdline(argc, argv, MYNAME);
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

  if (0 != SimServerDataManager::getInstance()->init()) {
    SM_LOG_FATAL ("init data manager error");
    return -1;
  }

  comcfg::Configure sim_server_conf;
  if (0 != sim_server_conf.load(flags.cfpath(), conf["global"]["sim_server_conf"].to_cstr())) {
    SM_LOG_FATAL ("load sim server conf error");
    return -1;
  }

  ub::NetReactor *reactor = new ub::NetReactor;
  if (reactor->load (sim_server_conf["reactor"]) != 0) {
    SM_LOG_FATAL ("load reactor failed");
    return -1;
  }
  reactor->run();

  SimServer *simserver = new SimServer (reactor);
  if (simserver->load (sim_server_conf["server"]["sim"]) != 0) {
    SM_LOG_WARNING ("load aserver failed");
    delete simserver;
    return -1;
  }

  SimServerDataManager::getInstance()->registerSimServer (simserver);
  ub_timer_task_t *timer = ub_create_timer_task();
  ub_add_timer_task(timer, check, NULL, 3000);
  ub_run_timer_task(timer);
  ub_join_timer_task(timer);
  ub_destroy_timer_task(timer);
  reactor->join();
  return 0;
}
