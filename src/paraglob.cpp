// See the file "COPYING" in the main distribution directory for copyright.

#include "paraglob.h"

paraglob::Paraglob::Paraglob(const std::vector<std::string>& patterns): Paraglob() {
  for (const std::string& pattern : patterns) {
    this->add(pattern);
  }
  this->compile();
}

bool paraglob::Paraglob::add(const std::string& pattern) {
  AhoCorasickPlus::EnumReturnStatus status;

  for (const std::string& meta_word : this->get_meta_words(pattern)) {
    AhoCorasickPlus::PatternId patId = this->meta_words.size();
    status = this->my_ac.addPattern(meta_word, patId);

    if (status == AhoCorasickPlus::RETURNSTATUS_SUCCESS) {
      this->meta_words.push_back(meta_word);
      this->meta_to_node_map.emplace(meta_word, paraglob::ParaglobNode(meta_word, pattern));
    } else if (status == AhoCorasickPlus::RETURNSTATUS_DUPLICATE_PATTERN) {
      this->meta_to_node_map.at(meta_word).add_pattern(pattern);
    } else { // Failed to add
      return false;
    }
  }

  return true;
}

void paraglob::Paraglob::compile() {
  this->my_ac.finalize();
}

std::vector<std::string> paraglob::Paraglob::get(const std::string& text) {
  // Narrow to the meta-word matches
  std::vector<std::string> patterns;
  for (int id : this->my_ac.findAll(text, false))
    this->meta_to_node_map.at(this->meta_words.at(id)).merge_matches(patterns, text);

  // Single wildcards always need to be checked
  if (this->single_wildcards.size() > 0)
    patterns.insert(patterns.end(), this->single_wildcards.begin(), this->single_wildcards.end());

  // Remove duplicates
  std::sort(patterns.begin(), patterns.end());
  patterns.erase(unique(patterns.begin(), patterns.end()), patterns.end());
  return patterns;
}

std::vector<std::string> paraglob::Paraglob::split_on_brackets(const std::string &in) {
  std::vector<std::string> out;
  size_t pos;
  size_t prev = 0;

  while ((pos = in.find_first_of('[', prev)) != std::string::npos) {
    size_t end_bracket = in.find_first_of(']', pos);
    if (end_bracket != std::string::npos) {
      out.push_back(in.substr(prev, pos-prev));
      prev = end_bracket + 1;
    } else {
      break;
    }
  }

  // There are no more opening / closing brackets
  // Append the rest of the string
  out.push_back(in.substr(prev, in.length()-prev));
  return out;
}


std::vector<std::string> paraglob::Paraglob::get_meta_words(const std::string &pattern) {
  std::vector<std::string> meta_words;

  // Split the pattern by brackets
  for (std::string word : split_on_brackets(pattern)) {
    // Parse each bracket section
    std::size_t prev = 0, pos;

    while ((pos = word.find_first_of("*?", prev)) != std::string::npos) {
      if (pos > prev) {
        meta_words.push_back(word.substr(prev, pos-prev));
      }
      prev = pos+1;
    }
    if (prev < word.length()) {
      meta_words.push_back(word.substr(prev, std::string::npos));
    }
  }

  if (meta_words.size() == 0 && pattern != "") {
    this->single_wildcards.push_back(pattern);
  }
  return meta_words;
}
