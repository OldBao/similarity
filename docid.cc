#include <iostream>
#include <stdint.h>
#include <ul_sign.h>

int
main(int argc, char **argv){
  uint64_t sign1, sign2;
  creat_sign_fs64 (argv[1], strlen(argv[1]), (uint32_t *) (&sign1), (uint32_t *) (&sign1) + 1);
  creat_sign_fs64 (argv[1], strlen(argv[1]), (uint32_t *) (&sign2)+1, (uint32_t *) (&sign2));

  std::cout  << "Sign is : " << sign1 << std::endl;
  std::cout  << "Sign is LD: " << sign2 << std::endl;

  return 0;
}
