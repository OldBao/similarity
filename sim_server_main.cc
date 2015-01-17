#include "sim_server.h"
#include "log.h"

using namespace sm;

int
main(int argc, char **argv) {
  int ret = ub_load_comlog ("./conf", "comlog.conf");
  if (ret != 0) {
    SM_LOG_FATAL ("open configure failed");
    return -1;
  }

  if (0 != SimServerDataManager::getInstance()->init(argv[1])) {
    SM_LOG_FATAL ("init sim server manager error");
    return -1;
  }

  comcfg::Configure conf;
  if (0 != conf.load ("./conf", "sim_server.conf")) {
    SM_LOG_FATAL ("open configure failed");
    return -1;
  }

  ub::NetReactor *reactor = new ub::NetReactor;
  if (reactor->load (conf["reactor"]) != 0) {
    SM_LOG_FATAL ("load reactor failed");
    return -1;
  }

  reactor->run ();

  SimServer *simserver = new SimServer (reactor);
  if (simserver->load (conf["server"]["sim"]) != 0) {
    SM_LOG_WARNING ("load aserver failed");
    delete simserver;
    return -1;
  }

  //simserver->set_sock_opt (UBSVR_)
  
  SimServerDataManager::getInstance()->registerSimServer (simserver);
  
  simserver->getready ();
  reactor->join();

  delete simserver;
  delete reactor;

  return 0;
}
