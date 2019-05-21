// See the file "COPYING" in the main distribution directory for copyright.
// Functions for serializing and unserializing a vector of strings.

#ifndef STRING_VECTOR_SERIALIZER_H
#define STRING_VECTOR_SERIALIZER_H

#include <string>
#include <vector>
#include <stdexcept>

namespace paraglob {
  /* Serializes a vector of strings into a string. */
  std::string serialize_string_vec(const std::vector<std::string>& in);
  /* Unserialize a string into a vector. Throws an exception for bad input. */
  std::vector<std::string> unserialize_string_vec(std::string serialized);
} // namespace paraglob

#endif
