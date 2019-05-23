// See the file "COPYING" in the main distribution directory for copyright.
// Class for performing serialization and deserialization of paraglob.

#ifndef PARAGLOB_SERIALIZER_H
#define PARAGLOB_SERIALIZER_H

#include <vector>
#include <memory> // std::unique_ptr

#include <paraglob.h>

namespace paraglob {

  class ParaglobSerializer {
  public:
    // ret -> [<n_strings><len_1><str_1>, <len_2><str_2>, ... <len_n><str_n>]
    // TODO: When Zeek supports C++17 char should be replaced by std::byte.
    static std::unique_ptr<std::vector<char>> serialize(const std::vector<std::string>& v);
    static std::vector<std::string> unserialize (const std::unique_ptr<std::vector<char>>& vsp);
  private:
    static void add_int(int a, std::vector<char> &target);
    static int get_int_and_move(std::vector<char>::iterator &start);
  };

} // namespace paraglob

#endif
