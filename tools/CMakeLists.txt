# Builds an executable instead of a library for testing
cmake_minimum_required (VERSION 3.14)

set(CMAKE_CXX_FLAGS "-Wall -O2")
set(CMAKE_CXX_STANDARD 17)

add_subdirectory(../ahocorasick build)

set(SOURCES driver.cpp benchmark.cpp ../src/paraglob.cpp ../ahocorasick/AhoCorasickPlus.cpp)
add_executable(paraglob.tools ${SOURCES})

target_link_libraries(paraglob.tools ahocorasick)

install(TARGETS paraglob.tools DESTINATION bin)
