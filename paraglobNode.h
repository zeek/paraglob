/**
 * Node class for paraglob. Holds a meta word and its associated patterns.
 */

#ifndef PARAGLOBNODE_H
#define PARAGLOBNODE_H

#include <set>
#include <string>

class ParaglobNode {
public:
  std::set<std::string> patterns;
  std::string meta_word;
  ParaglobNode(std::string meta_word, std::string init_pattern);
  bool operator==(const ParaglobNode &other) const;
  void addPattern(std::string pattern);
};

/**
 * Hash function for ParaglobNode. Makes ParaglobNode compatable with
 * std::unordered_map. Note that this is being added to namespace std.
 */
namespace std {
  template <>
  struct hash<ParaglobNode>
  {
    std::size_t operator()(const ParaglobNode& p) const
    {
      return std::hash<std::string>()(p.meta_word);
    }
  };
}

#endif
