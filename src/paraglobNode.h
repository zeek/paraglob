// See the file "COPYING" in the main distribution directory for copyright.
//
// Node class for paraglob. Holds a meta word and its associated patterns.

#ifndef PARAGLOBNODE_H
#define PARAGLOBNODE_H

#include <algorithm> // copy_if
#include <fnmatch.h>
#include <string>
#include <vector>

namespace paraglob {

class ParaglobNode {
public:
  ParaglobNode(std::string meta_word, std::string init_pattern)
    : meta_word(std::move(meta_word)), patterns({ std::move(init_pattern) }) {
  }

  std::string get_meta_word() const {
    return this->meta_word;
  }

  bool operator==(const ParaglobNode &other) const {
    return (this->meta_word == other.get_meta_word());
  }

  void add_pattern(std::string pattern) {
    this->patterns.push_back(std::move(pattern));
  }

  /* Merges this nodes matching patterns into the input vector. */
  void merge_matches(std::vector<std::string>& target, const std::string& text) {
    std::copy_if(this->patterns.begin(), this->patterns.end(),
                 std::back_inserter(target),
                 [text](const std::string& candidate) {
                   return (fnmatch(candidate.c_str(), text.c_str(), 0) == 0);
                 });
  }

private:
    std::string meta_word;
    std::vector<std::string> patterns;
};

} // namespace paraglob

#endif
