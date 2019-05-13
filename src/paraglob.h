// See the file "COPYING" in the main distribution directory for copyright.

#ifndef PARAGLOB_H
#define PARAGLOB_H

#include <ahocorasick/AhoCorasickPlus.h>

#include "paraglobNode.h"

#include <algorithm> // sort
#include <string>
#include <unordered_map>
#include <vector>

namespace paraglob {

class Paraglob {
private:
  AhoCorasickPlus my_ac;
  std::unordered_map<std::string, paraglob::ParaglobNode> meta_to_node_map;
  std::vector<std::string> meta_words;
  /* Patterns with no meta words, ex: '*' & '?' */
  std::vector<std::string> single_wildcards;

  /* Get a vector of the meta words in the pattern. */
  std::vector<std::string> get_meta_words(const std::string& pattern);
  /* Split a string on pairs of square brackets. */
  std::vector<std::string> split_on_brackets(const std::string& in);

public:
  /* Create an empty paraglob to fill with add() and finalize with compile() */
  Paraglob() = default;
  /* Fully unitialize a paraglob from a (large) vector of patterns, including compilation */
  Paraglob(const std::vector<std::string>& patterns);
  /* Add a pattern to the paraglob & return true on success */
  bool add(const std::string& pattern);
  /* Compile the paraglob */
  void compile();
  /* Get a vector of the patterns that match the input string */
  std::vector<std::string> get(const std::string& text);
};

} // namespace paraglob

#endif
