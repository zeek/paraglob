project(paraglob)
cmake_minimum_required(VERSION 2.8.12)

set(CMAKE_CXX_FLAGS "-Wall -O2")
set(SOURCES src/paraglob.cpp ahocorasick/AhoCorasickPlus.cpp )
if (NOT INSTALL_LIB_DIR)
  set(INSTALL_LIB_DIR ${CMAKE_INSTALL_PREFIX}/lib)
endif ()

add_subdirectory(ahocorasick)

add_library(paraglob STATIC ${SOURCES})
set_target_properties(paraglob PROPERTIES OUTPUT_NAME paraglob)
target_link_libraries(paraglob ahocorasick)

install(TARGETS paraglob DESTINATION lib)
install(FILES src/paraglob.h DESTINATION include)
