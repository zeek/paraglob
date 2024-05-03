include(../RequireCXXStd.cmake)

set(SOURCES driver.cpp benchmark.cpp)
add_executable(paraglob-test ${SOURCES})
target_compile_features(paraglob-test PRIVATE ${ZEEK_CXX_STD})
set_target_properties(paraglob-test PROPERTIES CXX_EXTENSIONS OFF)

target_link_libraries(paraglob-test paraglob)

install(TARGETS paraglob-test DESTINATION bin)
