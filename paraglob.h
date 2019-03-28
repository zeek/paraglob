#ifndef PARAGLOB_H
#define PARAGLOB_H

#include "AhoCorasickPlus.h"
#include "paraglobNode.h"

#include <vector>
#include <string>
#include <unordered_map>
#include <set>
#include <cstddef>
#include <fnmatch.h>
#include <iostream>


class Paraglob {
private:
  AhoCorasickPlus my_ac;
  std::unordered_map<std::string, ParaglobNode> meta_to_node_map;
  std::vector<std::string> meta_words;
  std::vector<std::string> single_wildcards;
  /* Get a vector of the meta words in the pattern. */
  std::vector<std::string> get_meta_words(std::string pattern);
  /* Split a string on pairs of square brackets. */
  std::vector<std::string> split_on_brackets(std::string in);
public:
  Paraglob() = default;
  /* Initialize a paraglob from a vector of patterns */
  Paraglob(std::vector<std::string> patterns);
  /* Add a pattern to the paraglob */
  void add(std::string pattern);
  /* Compile the paraglob */
  void compile();
  /* Get a vector of the patterns that match the input string */
  std::vector<std::string> get(std::string text);
};

#endif
