// See the file "COPYING" in the main distribution directory for copyright.

#include "string_vector_serializer.h"

namespace paraglob {
  // Serializes a vector of strings into a string.
  // Prepends the length of each string followed by a seperator to the output
  // string and then appends all the strings in the vector
  std::string serialize_string_vec(const std::vector<std::string>& in) {
    std::string delimiter ("~~~~~/////~~~~~");
    std::string nums ("");
    std::string out ("");

    for (std::string s : in) {
      nums = nums + std::to_string(s.length()) + ",";
      out = out + s;
    }

    return (nums + delimiter + out);
  }

  std::vector<std::string> unserialize_string_vec(std::string serialized) {
    std::vector<std::string> out;
    std::string delimiter ("~~~~~/////~~~~~");
    // Serialized an empty vector:
    if (serialized == delimiter) {
      return out;
    }

    size_t delim_begin = serialized.find(delimiter);
    if (delim_begin == std::string::npos) {
      throw std::invalid_argument(
        "Attempted to parse a string without a delimiter.\nString: " + serialized);
    }

    std::string nums = serialized.substr(0, delim_begin);
    serialized.erase(0, delim_begin + delimiter.length());
    std::vector<int> nums_vec;

    size_t pos;
    while((pos = nums.find(",")) != std::string::npos) {
      nums_vec.push_back(std::stoi(nums.substr(0, pos)));
      nums.erase(0, pos + 1); // Comma has length 1
    }

    for (int l : nums_vec) {
      out.push_back(serialized.substr(0, l));
      serialized.erase(0, l);
    }
    return out;
  }
} // namespace paraglob
