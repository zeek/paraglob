// See the file "COPYING" in the main distribution directory for copyright.

#ifndef PARAGLOB_H
#define PARAGLOB_H

#include <ahocorasick/AhoCorasickPlus.h>

#include "paraglob_node.h"
#include "paraglob_serializer.h"

#include <algorithm> // sort
#include <string>
#include <sstream> // str() function
#include <unordered_map>
#include <vector>
#include <memory> // std::unique_ptr

namespace paraglob {

  class Paraglob {
  private:
    AhoCorasickPlus my_ac;
    std::unordered_map<std::string, paraglob::ParaglobNode> meta_to_node_map;
    std::vector<std::string> meta_words;
    /* Patterns with no meta words, ex: '*' & '?' */
    std::vector<std::string> single_wildcards;
    /* Is the paraglob well made and problem free? */
    bool good_standing;

    /* Get a vector of the meta words in the pattern. */
    std::vector<std::string> get_meta_words(const std::string& pattern);
    /* Split a string on pairs of square brackets. */
    std::vector<std::string> split_on_brackets(const std::string& in);

  public:
    /* Create an empty paraglob to fill and compile later */
    Paraglob() : good_standing (true) { }
    /* Initialize a paraglob from a (large) vector of patterns and compile */
    Paraglob(const std::vector<std::string>& patterns);
    /* Initialize and compile a paraglob from a serialized one */
    Paraglob(std::unique_ptr<std::vector<uint8_t>> serialized);
    /* Add a pattern to the paraglob. Mutate good_standing on failure. */
    void add(const std::string& pattern);
    /* Compile the paraglob. */
    void compile();
    /* Get a vector of the patterns that match the input string */
    std::vector<std::string> get(const std::string& text);
    /* Get a raw byte representation of the paraglob */
    std::unique_ptr<std::vector<uint8_t>> serialize();
    /* Returns true if the paraglob is well constructed and compiled. */
    bool in_good_standing() const;
    /* Get readable contents of the paraglob for debugging */
    std::string str() const;
    /* Two paraglobs are equal if they contain the same patterns */
    bool operator==(const Paraglob &other) const;
  };

} // namespace paraglob

#endif
