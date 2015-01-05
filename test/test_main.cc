#include <string>
#include <iostream>
#include "test_main.h"
#include "log.h"
#include "segment.h"

using namespace std;
using namespace sm;

int
main(int argc, char **argv) {
  //ul_logstat_t logstat = {16, 0, UL_LOGTTY};
  //ul_openlog("./log", "test", &logstat, 1024);

  if ( 0 != Segment::getInstance()->load (argv[1], argv[2])){
    cout << "init segment unit error!" << endl;
    return -1;
  }

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
