cmake_minimum_required (VERSION 3.14)

project(paraglob)

set(CMAKE_CXX_FLAGS "-Wall -O2")
set(CMAKE_CXX_STANDARD 17)

add_subdirectory(ahocorasick)

set(SOURCES driver.cpp benchmark.cpp paraglob.cpp AhoCorasickPlus.cpp paraglobNode.cpp)
add_executable(paraglob.out ${SOURCES})

target_link_libraries(paraglob.out ahocorasick)

install(TARGETS paraglob.out DESTINATION bin)
install(FILES ahocorasick.h DESTINATION include)
