cmake_minimum_required(VERSION 3.15.0 FATAL_ERROR)
project(paraglob)

include(GNUInstallDirs)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif(NOT CMAKE_BUILD_TYPE)
string(TOUPPER ${CMAKE_BUILD_TYPE} build_affix)

# Modifies CXX_FLAGS && CXX_FLAGS_RELEASE
include(RequireCXX17.cmake)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_subdirectory(src)

if ( NOT MSVC ) 
    # The tools aren't currently supported on windows.
    add_subdirectory(tools)
endif()

install(DIRECTORY include/paraglob DESTINATION include FILES_MATCHING PATTERN "*.h")

set(summary
    "=================|  Paraglob Config Summary  |==================="
    "\nBUILD_TYPE:          ${build_affix}"
    "\nCXX_FLAGS:           ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_${build_affix}}"
    "\n================================================================="
    )

message("\n" ${summary} "\n")
