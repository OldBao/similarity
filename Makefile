#COMAKE2 edit-mode: -*- Makefile -*-
####################64Bit Mode####################
ifeq ($(shell uname -m),x86_64)
CC=gcc
CXX=g++
CXXFLAGS=-g \
  -pipe \
  -W \
  -Wall \
  -fPIC
CFLAGS=-g \
  -pipe \
  -W \
  -Wall \
  -fPIC
CPPFLAGS=-D_GNU_SOURCE \
  -D__STDC_LIMIT_MACROS \
  -DVERSION=\"1.9.8.7\"
INCPATH=-I. \
  -I./include \
  -I./output \
  -I./output/include
DEP_INCPATH=-I../../../../../../lib2-64/ccode \
  -I../../../../../../lib2-64/ccode/include \
  -I../../../../../../lib2-64/ccode/output \
  -I../../../../../../lib2-64/ccode/output/include \
  -I../../../../../../lib2-64/dict \
  -I../../../../../../lib2-64/dict/include \
  -I../../../../../../lib2-64/dict/output \
  -I../../../../../../lib2-64/dict/output/include \
  -I../../../../../../lib2-64/libcrf \
  -I../../../../../../lib2-64/libcrf/include \
  -I../../../../../../lib2-64/libcrf/output \
  -I../../../../../../lib2-64/libcrf/output/include \
  -I../../../../../../lib2-64/others-ex \
  -I../../../../../../lib2-64/others-ex/include \
  -I../../../../../../lib2-64/others-ex/output \
  -I../../../../../../lib2-64/others-ex/output/include \
  -I../../../../../../lib2-64/postag \
  -I../../../../../../lib2-64/postag/include \
  -I../../../../../../lib2-64/postag/output \
  -I../../../../../../lib2-64/postag/output/include \
  -I../../../../../../lib2-64/ullib \
  -I../../../../../../lib2-64/ullib/include \
  -I../../../../../../lib2-64/ullib/output \
  -I../../../../../../lib2-64/ullib/output/include \
  -I../../../../../../lib2-64/wordseg \
  -I../../../../../../lib2-64/wordseg/include \
  -I../../../../../../lib2-64/wordseg/output \
  -I../../../../../../lib2-64/wordseg/output/include \
  -I../../../../../../public/odict \
  -I../../../../../../public/odict/include \
  -I../../../../../../public/odict/output \
  -I../../../../../../public/odict/output/include

#============ CCP vars ============
CCHECK=@ccheck.py
CCHECK_FLAGS=
PCLINT=@pclint
PCLINT_FLAGS=
CCP=@ccp.py
CCP_FLAGS=


#COMAKE UUID
COMAKE_MD5=bdf2a0b66042d7998cf5ff083534723a  COMAKE


.PHONY:all
all:comake2_makefile_check train 
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mall[0m']"
	@echo "make all done"

.PHONY:comake2_makefile_check
comake2_makefile_check:
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mcomake2_makefile_check[0m']"
	#in case of error, update 'Makefile' by 'comake2'
	@echo "$(COMAKE_MD5)">comake2.md5
	@md5sum -c --status comake2.md5
	@rm -f comake2.md5

.PHONY:ccpclean
ccpclean:
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mccpclean[0m']"
	@echo "make ccpclean done"

.PHONY:clean
clean:ccpclean
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mclean[0m']"
	rm -rf train
	rm -rf ./output/bin/train
	rm -rf train_lock.o
	rm -rf train_main.o
	rm -rf train_dictionary.o
	rm -rf train_segment.o
	rm -rf train_token.o
	rm -rf train_document.o
	rm -rf train_encoding.o

.PHONY:dist
dist:
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mdist[0m']"
	tar czvf output.tar.gz output
	@echo "make dist done"

.PHONY:distclean
distclean:clean
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mdistclean[0m']"
	rm -f output.tar.gz
	@echo "make distclean done"

.PHONY:love
love:
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mlove[0m']"
	@echo "make love done"

train:train_lock.o \
  train_main.o \
  train_dictionary.o \
  train_segment.o \
  train_token.o \
  train_document.o \
  train_encoding.o
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mtrain[0m']"
	$(CXX) train_lock.o \
  train_main.o \
  train_dictionary.o \
  train_segment.o \
  train_token.o \
  train_document.o \
  train_encoding.o -Xlinker "-("  ../../../../../../lib2-64/ccode/lib/libulccode.a \
  ../../../../../../lib2-64/dict/lib/libuldict.a \
  ../../../../../../lib2-64/libcrf/lib/libcrf.a \
  ../../../../../../lib2-64/others-ex/lib/libullib_ex.a \
  ../../../../../../lib2-64/postag/lib/libpostag.a \
  ../../../../../../lib2-64/ullib/lib/libullib.a \
  ../../../../../../lib2-64/wordseg/libsegment.a \
  ../../../../../../public/odict/libodict.a -lpthread \
  -lcrypto \
  -lrt -Xlinker "-)" -o train
	mkdir -p ./output/bin
	cp -f --link train ./output/bin

train_lock.o:lock.cc \
  lock.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mtrain_lock.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o train_lock.o lock.cc

train_main.o:main.cc \
  segment.h \
  token.h \
  singleton.h \
  lock.h \
  dictionary.h \
  document.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mtrain_main.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o train_main.o main.cc

train_dictionary.o:dictionary.cc \
  dictionary.h \
  document.h \
  token.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mtrain_dictionary.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o train_dictionary.o dictionary.cc

train_segment.o:segment.cc \
  log.h \
  segment.h \
  token.h \
  singleton.h \
  lock.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mtrain_segment.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o train_segment.o segment.cc

train_token.o:token.cc \
  token.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mtrain_token.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o train_token.o token.cc

train_document.o:document.cc \
  segment.h \
  token.h \
  singleton.h \
  lock.h \
  document.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mtrain_document.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o train_document.o document.cc

train_encoding.o:encoding.cc \
  encoding.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mtrain_encoding.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o train_encoding.o encoding.cc

endif #ifeq ($(shell uname -m),x86_64)


