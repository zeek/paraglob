// See the file "COPYING" in the main distribution directory for copyright.
//
// Node class for paraglob. Holds a meta word and its associated patterns.

#ifndef PARAGLOBNODE_H
#define PARAGLOBNODE_H

#include <algorithm> // copy_if
#include <fnmatch.h>
#include <string>
#include <vector>
#include <iterator>

namespace paraglob {

class ParaglobNode {
public:
  ParaglobNode(std::string meta_word, std::string init_pattern)
    : meta_word(std::move(meta_word)), patterns({ std::move(init_pattern) }) { }

  std::string get_meta_word() const {
    return meta_word;
  }

  bool operator==(const ParaglobNode &other) const {
    return meta_word == other.meta_word;
  }

  void add_pattern(std::string pattern) {
    patterns.push_back(std::move(pattern));
  }

  /* Merges this nodes matching patterns into the input vector. */
  void merge_matches(std::vector<std::string>& target, const std::string& text) const {
    std::copy_if(patterns.begin(), patterns.end(),
                 std::back_inserter(target),
                 [text](const std::string& candidate) {
                   return (fnmatch(candidate.c_str(), text.c_str(), 0) == 0);
                 });
  }

  // Merges this nodes patterns into the input vector
  // Note: this could be done more efficently with a move iterator if we wanted
  // this to be destructive.
  void merge_patterns(std::vector<std::string>& target) const {
    target.insert(target.begin(), patterns.begin(), patterns.end());
  }

private:
    std::string meta_word;
    std::vector<std::string> patterns;
};

} // namespace paraglob

#endif