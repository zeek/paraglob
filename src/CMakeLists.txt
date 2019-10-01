
add_subdirectory(ahocorasick)

add_library(paraglob STATIC paraglob.cpp paraglob_serializer.cpp)
target_link_libraries(paraglob ahocorasick)
set_target_properties(paraglob PROPERTIES OUTPUT_NAME paraglob)

install(TARGETS paraglob DESTINATION lib)
