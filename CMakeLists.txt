cmake_minimum_required (VERSION 2.8)

project(paraglob)

set(srcs driver.c paraglob.c multifast-ac/ahocorasick.c multifast-ac/node.c)
add_executable(paraglob ${srcs})
