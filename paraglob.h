#ifndef PARAGLOB_H
#define PARAGLOB_H

#include "AhoCorasickPlus.h"
#include "paraglobNode.h"

#include <vector>
#include <string>
#include <unordered_map>
#include <set>
#include <fnmatch.h>
#include <iostream>
#include <algorithm> //remove_if, sort


class Paraglob {
private:
  AhoCorasickPlus my_ac;
  std::unordered_map<std::string, ParaglobNode> meta_to_node_map;
  std::vector<std::string> meta_words;
  /* Patterns with no meta words, ex: '*' & '?' */
  std::vector<std::string> single_wildcards;
  /* Get a vector of the meta words in the pattern. */
  std::vector<std::string> get_meta_words(const std::string &pattern);
  /* Split a string on pairs of square brackets. */
  std::vector<std::string> split_on_brackets(const std::string &in);
public:
  Paraglob() = default;
  /* Initialize a paraglob from a (large) vector of patterns. */
  Paraglob(const std::vector<std::string> &patterns);
  /* Add a pattern to the paraglob */
  void add(std::string pattern);
  /* Compile the paraglob */
  void compile();
  /* Get a vector of the patterns that match the input string */
  std::vector<std::string> get(std::string text);
};

#endif
