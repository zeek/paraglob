#include "paraglobNode.h"

ParaglobNode::ParaglobNode(std::string meta_word, std::string init_pattern) {
  this->meta_word = meta_word;
  this->addPattern(init_pattern);
}

bool ParaglobNode::operator==(const ParaglobNode &other) const {
  return (this->meta_word == other.meta_word);
}

void ParaglobNode::addPattern(std::string pattern) {
  this->patterns.insert(pattern);
}
