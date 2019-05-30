// See the file "COPYING" in the main distribution directory for copyright.

#include "paraglob_serializer.h"

std::unique_ptr<std::vector<uint8_t>>
  paraglob::ParaglobSerializer::serialize
    (const std::vector<std::string>& v, bool* good_standing) {
      std::unique_ptr<std::vector<uint8_t>> ret (new std::vector<uint8_t>);
      add_int(v.size(), *ret);

      for (const std::string &s: v) {
        add_int(s.length(), *ret);
        for (const uint8_t c : s) {
          ret->push_back(c);
        }
      }

    // TODO: How can we check to see if serialization was successful & change
    // good_standing if not?

    return ret;
  }

// ret -> [<n_strings><len_1><str_1>, <len_2><str_2>, ... <len_n><str_n>]
std::vector<std::string> paraglob::ParaglobSerializer::unserialize
  (const std::unique_ptr<std::vector<uint8_t>>& vsp, bool* good_standing) {
    std::vector<std::string> ret;
    // Serialized empty vector.
    if (vsp->size() == 0) {
      return ret;
    }

    std::vector<uint8_t>::iterator vsp_it = vsp->begin();
    uint64_t n_strings = get_int_and_move(vsp_it);
    if (vsp_it > vsp->end()) { // Equality possible if first number is 0
      *good_standing = false;
      return ret;
    }
    // Reserve space ahead of time rather than resizing in loop.
    ret.reserve(n_strings);

    uint64_t l;
    while (vsp_it < vsp->end()) {
      l = get_int_and_move(vsp_it);
      // Calls std::string(begin*, end*)
      ret.emplace_back(vsp_it, vsp_it + l);
      std::advance(vsp_it, l);
    }

    // If the read was successful we have advanced our iterator exactly to the
    // end and we have read exactly n_strings strings.
    if (vsp_it > vsp->end()) {
      *good_standing = false;
    } else if (ret.size() != n_strings){ // No need to set to false twice
      *good_standing = false;
    }

    return ret;
  }

void paraglob::ParaglobSerializer::add_int
  (uint64_t a, std::vector<uint8_t> &target) {
    uint8_t* chars = reinterpret_cast<uint8_t*>(&a);
    target.insert(target.end(), chars, chars + sizeof(uint64_t));
}

uint64_t paraglob::ParaglobSerializer::get_int_and_move
  (std::vector<uint8_t>::iterator &start) {
    uint64_t ret = static_cast<uint64_t>(*start);
    std::advance(start, sizeof(uint64_t));
    return ret;
}
