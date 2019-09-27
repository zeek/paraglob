// See the file "COPYING" in the main distribution directory for copyright.
// Class for performing serialization and deserialization of paraglob.

#ifndef PARAGLOB_SERIALIZER_H
#define PARAGLOB_SERIALIZER_H

#include <vector>
#include <memory> // std::unique_ptr

#include "paraglob/paraglob.h"
#include "paraglob/exceptions.h"

namespace paraglob {

  class ParaglobSerializer {
  public:
    /* Returns serialized version of vector in form:
       [<n_strings><len_1><str_1>, <len_2><str_2>, ... <len_n><str_n>] */
    // TODO: When Zeek supports C++17 char should be replaced by std::byte.
    static std::unique_ptr<std::vector<uint8_t>> serialize
      (const std::vector<std::string>  &v);
    /* Loads a serialized vector and returns it. */
    static std::vector<std::string> unserialize
      (const std::unique_ptr<std::vector<uint8_t>> &vsp);
  private:
    /* Divides up and adds a large integer to the input vector. */
    static void add_int (uint64_t a, std::vector<uint8_t> &target);
    /* Gets the large integer beginning at the iterator and moves it forward. */
    static uint64_t get_int_and_move (std::vector<uint8_t>::iterator &start);
  };

} // namespace paraglob

#endif
