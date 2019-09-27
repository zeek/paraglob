project(paraglob)
cmake_minimum_required(VERSION 2.8.12)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif(NOT CMAKE_BUILD_TYPE)
string(TOUPPER ${CMAKE_BUILD_TYPE} build_affix)

# Modifies CXX_FLAGS && CXX_FLAGS_RELEASE
include(RequireCXX11.cmake)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

include_directories(BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/src)

if (NOT INSTALL_LIB_DIR)
  set(INSTALL_LIB_DIR ${CMAKE_INSTALL_PREFIX}/lib)
endif ()

add_subdirectory(src)
add_subdirectory(tools)

install(DIRECTORY include/paraglob DESTINATION include FILES_MATCHING PATTERN "*.h")

set(summary
    "=================|  Paraglob Config Summary  |==================="
    "\nBUILD_TYPE:          ${build_affix}"
    "\nCXX_FLAGS:           ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_${build_affix}}"
    "\n================================================================="
    )

message("\n" ${summary} "\n")
