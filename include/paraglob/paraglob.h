// See the file "COPYING" in the main distribution directory for copyright.

#pragma once

#include <cstdint>
#include <memory> // std::unique_ptr
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "paraglob/node.h"

class aca_handle;

namespace paraglob {

class Paraglob {
public:
    /* Create an empty paraglob to fill with add and finalize with compile */
    Paraglob(size_t max_tree_size = 2048);

    /* Initialize a paraglob from a (large) vector of patterns and compile */
    Paraglob(const std::vector<std::string>& patterns, size_t max_tree_size = 2048);

    /* Initialize and compile a paraglob from a serialized one */
    Paraglob(std::unique_ptr<std::vector<uint8_t>> serialized, size_t max_tree_size = 2048);

    /* Destructor */
    ~Paraglob();

    /* Add a pattern to the paraglob & return true on success */
    bool add(const std::string& pattern);

    /* Compile the paraglob */
    void compile();

    /* Get a vector of the patterns that match the input string */
    std::vector<std::string> get(const std::string& text);

    /* Get a raw byte representation of the paraglob */
    std::unique_ptr<std::vector<uint8_t>> serialize() const;

    /* Get readable contents of the paraglob for debugging */
    std::string str() const;

    /* Two paraglobs are equal if they contain the same patterns */
    bool operator==(const Paraglob& other) const;

private:
    /* Get a vector of the meta words in the pattern. */
    std::vector<std::string> get_meta_words(const std::string& pattern);

    /* Split a string on pairs of square brackets. */
    std::vector<std::string> split_on_brackets(const std::string& in) const;

    /* Get a vector of all the patterns in the paraglob */
    std::vector<std::string> get_patterns() const;

    std::unique_ptr<aca_handle> handle;
    std::unordered_map<std::string, paraglob::ParaglobNode> meta_to_node_map;
    std::vector<std::string> meta_words;

    /* Patterns with no meta words, ex: '*' & '?' */
    std::vector<std::string> single_wildcards;

    /* All unique patterns in insertion order, for deterministic serialization */
    std::vector<std::string> all_patterns;
    std::unordered_set<std::string> seen_patterns;
};

} // namespace paraglob
