/**
 * Node class for paraglob. Holds a meta word and its associated patterns.
 */

#ifndef PARAGLOBNODE_H
#define PARAGLOBNODE_H

#include <vector>
#include <string>

class ParaglobNode {
private:
    std::vector<std::string> patterns;
    std::string meta_word;
public:
  ParaglobNode(const std::string meta_word, const std::string init_pattern);
  std::string get_meta_word() const;
  bool operator==(const ParaglobNode &other) const;
  void add_pattern(const std::string pattern);
  /* Merges this nodes patterns into the input vector. */
  void merge_into(std::vector<std::string> &s);
};

/*
Hash function for ParaglobNode. Makes ParaglobNode compatable with
std::unordered_map. Note that this is being added to namespace std.
 */
namespace std {
  template <>
  struct hash<ParaglobNode>
  {
    std::size_t operator()(const ParaglobNode& p) const
    {
      return std::hash<std::string>()(p.get_meta_word());
    }
  };
}

#endif
