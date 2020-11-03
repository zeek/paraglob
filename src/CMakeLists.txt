
set(AHOCORASICK_SRCS
  ahocorasick/ahocorasick.c
  ahocorasick/node.c
  ahocorasick/mpool.c
  ahocorasick/replace.c
  ahocorasick/AhoCorasickPlus.cpp
)

add_subdirectory(ahocorasick)

add_library(paraglob STATIC
  paraglob.cpp
  paraglob_serializer.cpp
  ${AHOCORASICK_SRCS}
)
set_target_properties(paraglob PROPERTIES OUTPUT_NAME paraglob)

install(TARGETS paraglob DESTINATION ${CMAKE_INSTALL_LIBDIR})
