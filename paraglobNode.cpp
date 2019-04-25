#include "paraglobNode.h"

ParaglobNode::ParaglobNode(const std::string meta_word, const std::string init_pattern) {
  this->meta_word = meta_word;
  this->add_pattern(init_pattern);
}

std::string ParaglobNode::get_meta_word() const {
  return this->meta_word;
}

bool ParaglobNode::operator==(const ParaglobNode &other) const {
  return (this->meta_word == other.meta_word);
}

void ParaglobNode::add_pattern(const std::string pattern) {
  this->patterns.push_back(pattern);
}

void ParaglobNode::merge_into(std::vector<std::string> &s) {
  s.insert(s.end(), this->patterns.begin(), this->patterns.end());
}
