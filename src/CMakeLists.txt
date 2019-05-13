
add_library(paraglob STATIC paraglob.cpp)
target_link_libraries(paraglob ahocorasick)
set_target_properties(paraglob PROPERTIES OUTPUT_NAME paraglob)

install(TARGETS paraglob DESTINATION lib)
install(FILES paraglob.h DESTINATION include)
