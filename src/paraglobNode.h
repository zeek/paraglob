/**
 * Node class for paraglob. Holds a meta word and its associated patterns.
 */

#ifndef PARAGLOBNODE_H
#define PARAGLOBNODE_H

#include <vector>
#include <string>
#include <algorithm> // copy_if
#include <fnmatch.h>

namespace paraglob {
  class ParaglobNode;
}

class paraglob::ParaglobNode {
public:
  ParaglobNode(const std::string meta_word, const std::string init_pattern) {
    this->meta_word = meta_word;
    this->add_pattern(init_pattern);
  }

  std::string get_meta_word() const {
    return this->meta_word;
  }

  bool operator==(const ParaglobNode &other) const {
    return (this->meta_word == other.get_meta_word());
  }

  void add_pattern(const std::string pattern) {
    this->patterns.push_back(pattern);
  }

  /* Merges this nodes matching patterns into the input vector. */
  void merge_matches(std::vector<std::string> &target, const std::string &text) {
    std::copy_if(this->patterns.begin(), this->patterns.end(),
                std::back_inserter(target),
                [text](const std::string& cannidate) {
                  return (fnmatch(cannidate.c_str(), text.c_str(), 0) == 0);
                });
  }

private:
    std::vector<std::string> patterns;
    std::string meta_word;
};

#endif
