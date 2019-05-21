
add_library(paraglob STATIC paraglob.cpp)
add_library(string_vector_serializer STATIC string_vector_serializer.cpp)
target_link_libraries(paraglob ahocorasick)
target_link_libraries(paraglob string_vector_serializer)
set_target_properties(paraglob PROPERTIES OUTPUT_NAME paraglob)

install(TARGETS paraglob DESTINATION lib)
install(FILES paraglob.h DESTINATION include)
