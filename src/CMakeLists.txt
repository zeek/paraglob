add_library(paraglob STATIC paraglob.cpp paraglob_serializer.cpp
                            ${CMAKE_CURRENT_SOURCE_DIR}/../auxil/libaca/aca.c)
target_include_directories(paraglob PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../auxil/libaca)
target_compile_features(paraglob PRIVATE cxx_std_20)
set_target_properties(paraglob PROPERTIES CXX_EXTENSIONS OFF OUTPUT_NAME paraglob)

install(TARGETS paraglob DESTINATION ${CMAKE_INSTALL_LIBDIR})
