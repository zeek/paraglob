
set(SOURCES driver.cpp benchmark.cpp)
add_executable(paraglob-test ${SOURCES})

target_link_libraries(paraglob-test paraglob)

install(TARGETS paraglob-test DESTINATION bin)
