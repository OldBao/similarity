#COMAKE2 edit-mode: -*- Makefile -*-
####################64Bit Mode####################
ifeq ($(shell uname -m),x86_64)
CC=gcc
CXX=g++
CXXFLAGS=-g \
  -O0 \
  -pipe \
  -W \
  -Wall \
  -fPIC \
  -DDEBUG
CFLAGS=
CPPFLAGS=-D_GNU_SOURCE \
  -D__STDC_LIMIT_MACROS \
  -DVERSION=\"1.0.0.0\"
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
  -I../../../../../../public/nshead \
  -I../../../../../../public/nshead/include \
  -I../../../../../../public/nshead/output \
  -I../../../../../../public/nshead/output/include \
  -I../../../../../../public/odict \
  -I../../../../../../public/odict/include \
  -I../../../../../../public/odict/output \
  -I../../../../../../public/odict/output/include \
  -I../../../../../../third-64/gtest \
  -I../../../../../../third-64/gtest/include \
  -I../../../../../../third-64/gtest/output \
  -I../../../../../../third-64/gtest/output/include \
  -I../../../../../../third-64/json-c \
  -I../../../../../../third-64/json-c/include \
  -I../../../../../../third-64/json-c/output \
  -I../../../../../../third-64/json-c/output/include \
  -I../../../../../../third-64/protobuf \
  -I../../../../../../third-64/protobuf/include \
  -I../../../../../../third-64/protobuf/output \
  -I../../../../../../third-64/protobuf/output/include \
  -I../../../../../../third-64/zlib \
  -I../../../../../../third-64/zlib/include \
  -I../../../../../../third-64/zlib/output \
  -I../../../../../../third-64/zlib/output/include

#============ CCP vars ============
CCHECK=@ccheck.py
CCHECK_FLAGS=
PCLINT=@pclint
PCLINT_FLAGS=
CCP=@ccp.py
CCP_FLAGS=


#COMAKE UUID
COMAKE_MD5=ce177675aa53570613d8a944c59e825e  COMAKE


.PHONY:all
all:comake2_makefile_check libsimilarity.a train sim server test 
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
	rm -rf libsimilarity.a
	rm -rf ./output/lib/libsimilarity.a
	rm -rf ./output/include/dictionary.h
	rm -rf ./output/include/document.h
	rm -rf ./output/include/token.h
	rm -rf ./output/include/segment.h
	rm -rf ./output/include/encoding.h
	rm -rf ./output/include/corpus.h
	rm -rf ./output/include/model.h
	rm -rf ./output/include/concurrent.h
	rm -rf ./output/include/kvproxy_client.h
	rm -rf train
	rm -rf ./output/bin/train
	rm -rf sim
	rm -rf ./output/bin/sim
	rm -rf server
	rm -rf ./output/bin/server
	$(MAKE) -C test clean
	rm -rf similarity_dictionary.o
	rm -rf similarity_segment.o
	rm -rf similarity_token.o
	rm -rf similarity_document.o
	rm -rf similarity_encoding.o
	rm -rf similarity_corpus.o
	rm -rf similarity_tfidf.o
	rm -rf similarity_lda.o
	rm -rf similarity_lda_utils.o
	rm -rf similarity_cokus.o
	rm -rf similarity_similarity.o
	rm -rf similarity_bow.o
	rm -rf similarity_concurrent.o
	rm -rf similarity_repo.o
	rm -rf similarity_kvproxy_client.o
	rm -rf interface/dict.pb.cc
	rm -rf interface/dict.pb.h
	rm -rf interface/similarity_dict.pb.o
	rm -rf interface/corpus.pb.cc
	rm -rf interface/corpus.pb.h
	rm -rf interface/similarity_corpus.pb.o
	rm -rf interface/bow.pb.cc
	rm -rf interface/bow.pb.h
	rm -rf interface/similarity_bow.pb.o
	rm -rf interface/lda.pb.cc
	rm -rf interface/lda.pb.h
	rm -rf interface/similarity_lda.pb.o
	rm -rf train_main.o
	rm -rf sim_sim_main.o
	rm -rf server_server.o

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

libsimilarity.a:similarity_dictionary.o \
  similarity_segment.o \
  similarity_token.o \
  similarity_document.o \
  similarity_encoding.o \
  similarity_corpus.o \
  similarity_tfidf.o \
  similarity_lda.o \
  similarity_lda_utils.o \
  similarity_cokus.o \
  similarity_similarity.o \
  similarity_bow.o \
  similarity_concurrent.o \
  similarity_repo.o \
  similarity_kvproxy_client.o \
  interface/similarity_dict.pb.o \
  interface/similarity_corpus.pb.o \
  interface/similarity_bow.pb.o \
  interface/similarity_lda.pb.o \
  dictionary.h \
  document.h \
  token.h \
  segment.h \
  encoding.h \
  corpus.h \
  model.h \
  concurrent.h \
  kvproxy_client.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mlibsimilarity.a[0m']"
	ar crs libsimilarity.a similarity_dictionary.o \
  similarity_segment.o \
  similarity_token.o \
  similarity_document.o \
  similarity_encoding.o \
  similarity_corpus.o \
  similarity_tfidf.o \
  similarity_lda.o \
  similarity_lda_utils.o \
  similarity_cokus.o \
  similarity_similarity.o \
  similarity_bow.o \
  similarity_concurrent.o \
  similarity_repo.o \
  similarity_kvproxy_client.o \
  interface/similarity_dict.pb.o \
  interface/similarity_corpus.pb.o \
  interface/similarity_bow.pb.o \
  interface/similarity_lda.pb.o
	mkdir -p ./output/lib
	cp -f --link libsimilarity.a ./output/lib
	mkdir -p ./output/include
	cp -f --link dictionary.h document.h token.h segment.h encoding.h corpus.h model.h concurrent.h kvproxy_client.h ./output/include

train:train_main.o \
  -lsimilarity
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mtrain[0m']"
	$(CXX) train_main.o -Xlinker "-(" -lsimilarity ../../../../../../lib2-64/ccode/lib/libulccode.a \
  ../../../../../../lib2-64/dict/lib/libuldict.a \
  ../../../../../../lib2-64/libcrf/lib/libcrf.a \
  ../../../../../../lib2-64/others-ex/lib/libullib_ex.a \
  ../../../../../../lib2-64/postag/lib/libpostag.a \
  ../../../../../../lib2-64/ullib/lib/libullib.a \
  ../../../../../../lib2-64/wordseg/libsegment.a \
  ../../../../../../public/nshead/libnshead.a \
  ../../../../../../public/odict/libodict.a \
  ../../../../../../third-64/gtest/lib/libgtest.a \
  ../../../../../../third-64/gtest/lib/libgtest_main.a \
  ../../../../../../third-64/json-c/lib/libjson.a \
  ../../../../../../third-64/protobuf/lib/libprotobuf-lite.a \
  ../../../../../../third-64/protobuf/lib/libprotobuf.a \
  ../../../../../../third-64/protobuf/lib/libprotoc.a \
  ../../../../../../third-64/zlib/lib/libz.a -lpthread \
  -lcrypto \
  -lrt \
  -L. -Xlinker "-)" -o train
	mkdir -p ./output/bin
	cp -f --link train ./output/bin

sim:sim_sim_main.o \
  -lsimilarity
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msim[0m']"
	$(CXX) sim_sim_main.o -Xlinker "-(" -lsimilarity ../../../../../../lib2-64/ccode/lib/libulccode.a \
  ../../../../../../lib2-64/dict/lib/libuldict.a \
  ../../../../../../lib2-64/libcrf/lib/libcrf.a \
  ../../../../../../lib2-64/others-ex/lib/libullib_ex.a \
  ../../../../../../lib2-64/postag/lib/libpostag.a \
  ../../../../../../lib2-64/ullib/lib/libullib.a \
  ../../../../../../lib2-64/wordseg/libsegment.a \
  ../../../../../../public/nshead/libnshead.a \
  ../../../../../../public/odict/libodict.a \
  ../../../../../../third-64/gtest/lib/libgtest.a \
  ../../../../../../third-64/gtest/lib/libgtest_main.a \
  ../../../../../../third-64/json-c/lib/libjson.a \
  ../../../../../../third-64/protobuf/lib/libprotobuf-lite.a \
  ../../../../../../third-64/protobuf/lib/libprotobuf.a \
  ../../../../../../third-64/protobuf/lib/libprotoc.a \
  ../../../../../../third-64/zlib/lib/libz.a -lpthread \
  -lcrypto \
  -lrt \
  -L. -Xlinker "-)" -o sim
	mkdir -p ./output/bin
	cp -f --link sim ./output/bin

server:server_server.o \
  -lsimilarity
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mserver[0m']"
	$(CXX) server_server.o -Xlinker "-(" -lsimilarity ../../../../../../lib2-64/ccode/lib/libulccode.a \
  ../../../../../../lib2-64/dict/lib/libuldict.a \
  ../../../../../../lib2-64/libcrf/lib/libcrf.a \
  ../../../../../../lib2-64/others-ex/lib/libullib_ex.a \
  ../../../../../../lib2-64/postag/lib/libpostag.a \
  ../../../../../../lib2-64/ullib/lib/libullib.a \
  ../../../../../../lib2-64/wordseg/libsegment.a \
  ../../../../../../public/nshead/libnshead.a \
  ../../../../../../public/odict/libodict.a \
  ../../../../../../third-64/gtest/lib/libgtest.a \
  ../../../../../../third-64/gtest/lib/libgtest_main.a \
  ../../../../../../third-64/json-c/lib/libjson.a \
  ../../../../../../third-64/protobuf/lib/libprotobuf-lite.a \
  ../../../../../../third-64/protobuf/lib/libprotobuf.a \
  ../../../../../../third-64/protobuf/lib/libprotoc.a \
  ../../../../../../third-64/zlib/lib/libz.a -lpthread \
  -lcrypto \
  -lrt \
  -L. -Xlinker "-)" -o server
	mkdir -p ./output/bin
	cp -f --link server ./output/bin

.PHONY:test
test:
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mtest[0m']"
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Entering directory:'[1;32;40mtest[0m']"
	$(MAKE) -C test
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Leaving directory:'[1;32;40mtest[0m']"

similarity_dictionary.o:dictionary.cc \
  log.h \
  dictionary.h \
  document.h \
  token.h \
  bow.h \
  concurrent.h \
  concurrent.hpp \
  encoding.h \
  interface/dict.pb.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msimilarity_dictionary.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o similarity_dictionary.o dictionary.cc

similarity_segment.o:segment.cc \
  log.h \
  segment.h \
  token.h \
  singleton.h \
  concurrent.h \
  concurrent.hpp \
  encoding.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msimilarity_segment.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o similarity_segment.o segment.cc

similarity_token.o:token.cc \
  token.h \
  encoding.h \
  log.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msimilarity_token.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o similarity_token.o token.cc

similarity_document.o:document.cc \
  segment.h \
  token.h \
  singleton.h \
  concurrent.h \
  concurrent.hpp \
  log.h \
  document.h \
  encoding.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msimilarity_document.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o similarity_document.o document.cc

similarity_encoding.o:encoding.cc \
  encoding.h \
  log.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msimilarity_encoding.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o similarity_encoding.o encoding.cc

similarity_corpus.o:corpus.cc \
  log.h \
  corpus.h \
  dictionary.h \
  document.h \
  token.h \
  bow.h \
  concurrent.h \
  concurrent.hpp \
  model.h \
  test/test_lda.h \
  test/test_main.h \
  interface/corpus.pb.h \
  interface/bow.pb.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msimilarity_corpus.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o similarity_corpus.o corpus.cc

similarity_tfidf.o:tfidf.cc \
  model.h \
  corpus.h \
  dictionary.h \
  document.h \
  token.h \
  bow.h \
  concurrent.h \
  concurrent.hpp \
  log.h \
  test/test_lda.h \
  test/test_main.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msimilarity_tfidf.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o similarity_tfidf.o tfidf.cc

similarity_lda.o:lda.cc \
  model.h \
  corpus.h \
  dictionary.h \
  document.h \
  token.h \
  bow.h \
  concurrent.h \
  concurrent.hpp \
  log.h \
  test/test_lda.h \
  test/test_main.h \
  encoding.h \
  interface/lda.pb.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msimilarity_lda.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o similarity_lda.o lda.cc

similarity_lda_utils.o:lda_utils.cc
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msimilarity_lda_utils.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o similarity_lda_utils.o lda_utils.cc

similarity_cokus.o:cokus.cc \
  cokus.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msimilarity_cokus.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o similarity_cokus.o cokus.cc

similarity_similarity.o:similarity.cc \
  similarity.h \
  corpus.h \
  dictionary.h \
  document.h \
  token.h \
  bow.h \
  concurrent.h \
  concurrent.hpp \
  log.h \
  model.h \
  test/test_lda.h \
  test/test_main.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msimilarity_similarity.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o similarity_similarity.o similarity.cc

similarity_bow.o:bow.cc \
  bow.h \
  log.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msimilarity_bow.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o similarity_bow.o bow.cc

similarity_concurrent.o:concurrent.cc \
  concurrent.h \
  concurrent.hpp \
  log.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msimilarity_concurrent.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o similarity_concurrent.o concurrent.cc

similarity_repo.o:repo.cc \
  repo.h \
  concurrent.h \
  concurrent.hpp \
  log.h \
  dictionary.h \
  document.h \
  token.h \
  bow.h \
  kvproxy_client.h \
  corpus.h \
  encoding.h \
  model.h \
  test/test_lda.h \
  test/test_main.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msimilarity_repo.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o similarity_repo.o repo.cc

similarity_kvproxy_client.o:kvproxy_client.cc \
  log.h \
  kvproxy_client.h \
  interface.h \
  cache_interface.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msimilarity_kvproxy_client.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o similarity_kvproxy_client.o kvproxy_client.cc

interface/dict.pb.cc \
  interface/dict.pb.h:interface/dict.proto
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40minterface/dict.pb.cc \
  interface/dict.pb.h[0m']"
	../../../../../../third-64/protobuf/bin/protoc --cpp_out=interface --proto_path=interface  interface/dict.proto

interface/dict.proto:
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40minterface/dict.proto[0m']"
	@echo "ALREADY BUILT"

interface/similarity_dict.pb.o:interface/dict.pb.cc \
  interface/dict.pb.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40minterface/similarity_dict.pb.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o interface/similarity_dict.pb.o interface/dict.pb.cc

interface/corpus.pb.cc \
  interface/corpus.pb.h:interface/corpus.proto
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40minterface/corpus.pb.cc \
  interface/corpus.pb.h[0m']"
	../../../../../../third-64/protobuf/bin/protoc --cpp_out=interface --proto_path=interface  interface/corpus.proto

interface/corpus.proto:
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40minterface/corpus.proto[0m']"
	@echo "ALREADY BUILT"

interface/similarity_corpus.pb.o:interface/corpus.pb.cc \
  interface/corpus.pb.h \
  interface/bow.pb.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40minterface/similarity_corpus.pb.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o interface/similarity_corpus.pb.o interface/corpus.pb.cc

interface/bow.pb.cc \
  interface/bow.pb.h:interface/bow.proto
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40minterface/bow.pb.cc \
  interface/bow.pb.h[0m']"
	../../../../../../third-64/protobuf/bin/protoc --cpp_out=interface --proto_path=interface  interface/bow.proto

interface/bow.proto:
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40minterface/bow.proto[0m']"
	@echo "ALREADY BUILT"

interface/similarity_bow.pb.o:interface/bow.pb.cc \
  interface/bow.pb.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40minterface/similarity_bow.pb.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o interface/similarity_bow.pb.o interface/bow.pb.cc

interface/lda.pb.cc \
  interface/lda.pb.h:interface/lda.proto
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40minterface/lda.pb.cc \
  interface/lda.pb.h[0m']"
	../../../../../../third-64/protobuf/bin/protoc --cpp_out=interface --proto_path=interface  interface/lda.proto

interface/lda.proto:
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40minterface/lda.proto[0m']"
	@echo "ALREADY BUILT"

interface/similarity_lda.pb.o:interface/lda.pb.cc \
  interface/lda.pb.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40minterface/similarity_lda.pb.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o interface/similarity_lda.pb.o interface/lda.pb.cc

train_main.o:main.cc \
  segment.h \
  token.h \
  singleton.h \
  concurrent.h \
  concurrent.hpp \
  log.h \
  dictionary.h \
  document.h \
  bow.h \
  corpus.h \
  model.h \
  test/test_lda.h \
  test/test_main.h \
  encoding.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mtrain_main.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o train_main.o main.cc

sim_sim_main.o:sim_main.cc \
  segment.h \
  token.h \
  singleton.h \
  concurrent.h \
  concurrent.hpp \
  log.h \
  dictionary.h \
  document.h \
  bow.h \
  corpus.h \
  model.h \
  test/test_lda.h \
  test/test_main.h \
  encoding.h \
  similarity.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40msim_sim_main.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o sim_sim_main.o sim_main.cc

server_server.o:server.cc \
  repo.h \
  concurrent.h \
  concurrent.hpp \
  log.h \
  dictionary.h \
  document.h \
  token.h \
  bow.h \
  kvproxy_client.h \
  corpus.h \
  segment.h \
  singleton.h
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mserver_server.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o server_server.o server.cc

endif #ifeq ($(shell uname -m),x86_64)


