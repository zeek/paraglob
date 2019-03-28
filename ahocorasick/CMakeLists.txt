cmake_minimum_required(VERSION 3.14)

project(ahocorasick)

set(CMAKE_CXX_FLAGS "-Wall -O2")

add_library(ahocorasick STATIC ahocorasick.c node.c mpool.c replace.c)

install(TARGETS ahocorasick DESTINATION lib)
install(FILES ahocorasick.h DESTINATION include)
