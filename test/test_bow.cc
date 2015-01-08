#include "test_bow.h"

using namespace std;
using namespace sm;


TEST_F (BowTestCase, TestOrder) {
  bow_unit_t u1 = {3, 10}, u2 = {1, 20}, u3 = {2, 30};
  bow_t bow;
  bow.push_back(u1);
  bow.push_back(u2);
  bow.push_back(u3);

  ASSERT_EQ (bow[0].id, 1);
  ASSERT_EQ (bow[1].id, 2);
  ASSERT_EQ (bow[2].id, 3);
}


TEST_F (BowTestCase, TestNormNormal) {
  bow_unit_t u1 = {1, 10}, u2 = {2, 20};
  bow_t bow;
  bow.push_back(u1);
  bow.push_back(u2);

  ASSERT_NEAR (bow.norm(), 22.3, 1e-1);
}



TEST_F (BowTestCase, TestCosSim) {
  bow_unit_t u1 = {1, 10}, u2 = {1, 20}, u3 = {3, 10}, u4 = {4, 10};
  bow_t bow1, bow2, bow3;
  bow1.push_back (u1);
  bow1.push_back (u2);

  bow2.push_back (u3);
  bow2.push_back (u4);

  bow3.push_back (u1);
  bow3.push_back (u3);
  
  ASSERT_DOUBLE_EQ (bow1.cossim(bow2), 0.0);

  ASSERT_NEAR (bow1.cossim(bow3), 0.32, 5e-1);
  
  ASSERT_NEAR (bow1.cossim(bow1), 1.0, 1e-10);
}
