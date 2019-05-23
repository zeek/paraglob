// See the file "COPYING" in the main distribution directory for copyright.

#include "paraglob_serializer.h"

std::unique_ptr<std::vector<char>>
  paraglob::ParaglobSerializer::serialize(const std::vector<std::string>& v) {
    std::unique_ptr<std::vector<char>> ret (new std::vector<char>);
    add_int(v.size(), *ret);

    for (const std::string s: v) {
      add_int(s.length(), *ret);
      for (const char &c : s) {
        ret->push_back(c);
      }
    }

    return ret;
  }

// ret -> [<n_strings><len_1><str_1>, <len_2><str_2>, ... <len_n><str_n>]
std::vector<std::string> paraglob::ParaglobSerializer::unserialize
  (const std::unique_ptr<std::vector<char>>& vsp) {
    std::vector<std::string> ret;
    // Serialized empty vector.
    if (vsp->size() == 0) {
      return ret;
    }

    std::vector<char>::iterator vsp_it = vsp->begin();
    int n_strings = get_int_and_move(vsp_it);
    // Reserve space ahead of time rather than resizing in loop.
    ret.reserve(n_strings);
    std::vector<std::string>::iterator ret_it = ret.begin();

    int added = 0;
    while (added < n_strings) {
      int l = get_int_and_move(vsp_it);
      ret.emplace_back(vsp_it, vsp_it + l);
      std::advance(ret_it, 1);
      std::advance(vsp_it, l);
      ++added;
    }
    
    return ret;
  }

void paraglob::ParaglobSerializer::add_int(int a, std::vector<char> &target) {
  char* chars = reinterpret_cast<char*>(&a);
  target.insert(target.end(), chars, chars+sizeof(int));
}

int paraglob::ParaglobSerializer::get_int_and_move(std::vector<char>::iterator &start) {
  int ret = (int)*start;
  std::advance(start, sizeof(int));
  return ret;
}
