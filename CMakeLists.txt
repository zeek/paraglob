project(paraglob)
cmake_minimum_required(VERSION 2.8.12)

include(RequireCXX11.cmake)

set(CMAKE_CXX_FLAGS "-Wall")
set(CMAKE_CXX_FLAGS_RELEASE "-O2")

include_directories(BEFORE ${CMAKE_CURRENT_SOURCE_DIR})
include_directories(BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/src)

if (NOT INSTALL_LIB_DIR)
  set(INSTALL_LIB_DIR ${CMAKE_INSTALL_PREFIX}/lib)
endif ()

add_subdirectory(ahocorasick)
add_subdirectory(src)
add_subdirectory(tools)
