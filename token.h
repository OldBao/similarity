#ifndef TOKEN_H_
#define TOKEN_H_
#include "isegment.h"
#include "tagdict.h"

namespace sm {

#if POS_TAG_COUNT > 64
#error "postag has toomuch, so please do not use bitmap"
#endif

#define SM_POS2TYPE(pos) ( (uint64_t)( 1ULL <<(pos) ))
#define INC_HID_TYPE(s, n) const uint64_t SM_TOKEN_TYPE_##s = SM_POS2TYPE(n);

#define INC_TYPE(s) const uint64_t SM_TOKEN_TYPE_##s = SM_POS2TYPE(POS_##s)
  INC_HID_TYPE(AG, 1); //Ag
  INC_HID_TYPE(DG, 2); //Dg
  INC_TYPE(NG); // 3 Ng
  INC_HID_TYPE(TG, 4); // TG
  INC_TYPE(VG); //5 Vg
  INC_TYPE(ADJ); //6 a
  INC_HID_TYPE(AD, 7); //7 ad
  INC_HID_TYPE(AN, 8); //8 an
  INC_TYPE(B);   // 9 b
  INC_TYPE(CONJ); //10 c
  INC_TYPE(ADV); //11 d
  INC_TYPE(EXCL); //12 e
  INC_TYPE(F); //13 f
  INC_HID_TYPE(G, 14); //14 g
  INC_HID_TYPE(H, 15); //15 h
  INC_HID_TYPE(I, 16); //16 i
  INC_HID_TYPE(J, 17); //17 j
  INC_HID_TYPE(K, 18); //18 k
  INC_HID_TYPE(L, 19); //19 l
  INC_TYPE(NUMBER); //20 m
  INC_TYPE(DEFAULT); //21 n
  INC_TYPE(PER); // 22 nr
  INC_TYPE(LOC); // 23 ns
  INC_TYPE(ORG); // 24 nt
  INC_TYPE(NX); // 25 nx
  INC_TYPE(NZ); // 26 nz
  INC_TYPE(ONO); //27 o
  INC_TYPE(PREP); // 28 p
  INC_TYPE(QUAN); // 29 q
  INC_TYPE(PRON); // 30 r
  INC_TYPE(S); // 31 s
  INC_TYPE(TIME); //32 t
  INC_TYPE(AUX); //33 u
  INC_TYPE(VERB); //34 v
  INC_HID_TYPE(VD, 35); //35 vd
  INC_HID_TYPE(VN, 36); //36 vn
  INC_TYPE(DELIM); //37 w
  INC_TYPE(MOOD); // 38 y
  INC_TYPE(ZHUANG); //39 z
#undef INC_TYPE
#undef INC_HID_TYPE

class Token {
 public:
  Token(const std::string &encoding="utf8");
  Token(token_t scw_token, const std::string &encoding="utf8");

  const std::string& toString() const{
    return _desc;
  }
  int length() const;

  std::string content;
  int type;
  int weight, lprop, hprop;

 private:
  std::string _desc;
  std::string _encoding;
 
  int _length;
  void format();
};
}
#endif
