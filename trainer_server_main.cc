#include "trainer_server.h"

int
main(int argc, char **argv) {
  if (0 != Segment::getInstance()->load ("worddict", "postag", "stopwords.utf8")) {
    SM_LOG_FATAL ("load segment dicts error");
    return -1;
  }

  
}
