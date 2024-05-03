add_library(ahocorasick STATIC ahocorasick.c node.c mpool.c replace.c AhoCorasickPlus.cpp)
target_compile_features(ahocorasick PRIVATE ${ZEEK_CXX_STD})
set_target_properties(ahocorasick PROPERTIES CXX_EXTENSIONS OFF)
