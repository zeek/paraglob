// See the file "COPYING" in the main distribution directory for copyright.

#include "paraglob/serializer.h"

std::unique_ptr<std::vector<uint8_t>>
  paraglob::ParaglobSerializer::serialize(const std::vector<std::string>& v) {
    std::unique_ptr<std::vector<uint8_t>> ret (new std::vector<uint8_t>);
    add_int(v.size(), *ret);

    for (const std::string &s: v) {
      add_int(s.length(), *ret);
      for (uint8_t c : s) { // copy here because of type change
        ret->push_back(c);
      }
    }

    return ret;
  }

// ret -> [<n_strings><len_1><str_1>, <len_2><str_2>, ... <len_n><str_n>]
std::vector<std::string> paraglob::ParaglobSerializer::unserialize
  (const std::unique_ptr<std::vector<uint8_t>>& vsp) {
    std::vector<std::string> ret;

    std::vector<uint8_t>::iterator vsp_it = vsp->begin();
    uint64_t n_strings = get_int_and_move(vsp_it);

    // If n_strings is zero vsp_it will equal vsp->end
    if (vsp_it > vsp->end()){
      throw paraglob::underflow_error("Serialization data ended unexpectedly.");
    }
    // Reserve space ahead of time rather than resizing in loop.
    ret.reserve(n_strings);

    while (vsp_it < vsp->end()) {
      uint64_t l = get_int_and_move(vsp_it);
      ret.emplace_back(vsp_it, vsp_it + l);
      std::advance(vsp_it, l);
    }

    // If the read was successful, we have advanced our iterator exactly to the
    // end, and we have read exactly n_strings.
    if (vsp_it > vsp->end()) {
      throw paraglob::underflow_error("Serialization data ended unexpectedly.");
    } else if (ret.size() > n_strings) {
      throw paraglob::overflow_error("Read more patterns than expected.");
    } else if (ret.size() < n_strings) {
      throw paraglob::underflow_error("Read fewer patterns than expected.");
    }

    return ret;
  }

inline void paraglob::ParaglobSerializer::add_int
  (uint64_t a, std::vector<uint8_t> &target) {
    uint8_t* chars = reinterpret_cast<uint8_t*>(&a);
    target.insert(target.end(), chars, chars + sizeof(uint64_t));
}

inline uint64_t paraglob::ParaglobSerializer::get_int_and_move
  (std::vector<uint8_t>::iterator &start) {
    uint64_t ret = static_cast<uint64_t>(*start);
    std::advance(start, sizeof(uint64_t));
    return ret;
}
