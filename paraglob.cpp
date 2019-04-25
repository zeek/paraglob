#include "paraglob.h"

Paraglob::Paraglob(const std::vector<std::string> &patterns): Paraglob() {
  for (std::string pattern : patterns) {
    this->add(pattern);
  }
  this->compile();
}

void Paraglob::add(std::string pattern) {
  // Get the meta words
  std::vector<std::string> m_words = get_meta_words(pattern);
  // Put them away
  if (m_words.size() == 0) {
    this->single_wildcards.push_back(pattern);
  }
  for (std::string meta_word : m_words) {
    AhoCorasickPlus::EnumReturnStatus status;
    AhoCorasickPlus::PatternId patId = this->meta_words.size();

    status = this->my_ac.addPattern(meta_word, patId);
    if (status != AhoCorasickPlus::RETURNSTATUS_SUCCESS &&
      status != AhoCorasickPlus::RETURNSTATUS_DUPLICATE_PATTERN) {
      std::cout << "Failed to add: " << meta_word << std::endl;
    } else {
      if (status == AhoCorasickPlus::RETURNSTATUS_DUPLICATE_PATTERN) {
        this->meta_to_node_map.at(meta_word).add_pattern(pattern);
      } else {
        this->meta_words.push_back(meta_word);
        this->meta_to_node_map.emplace(meta_word, ParaglobNode(meta_word, pattern));
      }
    }
  }
}

void Paraglob::compile() {
  this->my_ac.finalize();
}

std::vector<std::string> Paraglob::get(std::string text) {
  // Narrow to the meta-word matches
  AhoCorasickPlus::Match aMatch;
  this->my_ac.search(text, false);
  std::vector<std::string> meta_matches;
  while (this->my_ac.findNext(aMatch)) {
    meta_matches.push_back(this->meta_words.at(aMatch.id));
  }

  // Get the relevant patterns
  std::vector<std::string> patterns;
  for (std::string meta_word : meta_matches) {
    this->meta_to_node_map.at(meta_word).merge_into(patterns);
  }
  patterns.insert(patterns.end(), single_wildcards.begin(), single_wildcards.end());

  // Remove non-matches
  // Faster to remove in place rather than building new vector.
  patterns.erase(std::remove_if(
    patterns.begin(), patterns.end(),
    [text](const std::string& pattern) {
        return (fnmatch(pattern.c_str(), text.c_str(), 0) != 0); // No match
    }), patterns.end());

  // Remove duplicates
  // Faster to remove duplicates after we've narrowed down to matching patterns.
  std::sort(patterns.begin(), patterns.end());
  patterns.erase(unique(patterns.begin(), patterns.end()), patterns.end());
  return patterns;
}

std::vector<std::string> Paraglob::split_on_brackets(const std::string &in) {
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
  // There are no more opening/ closing brackets
  // Append the rest of the string
  out.push_back(in.substr(prev, in.length()-prev));
  return out;
}


std::vector<std::string> Paraglob::get_meta_words(const std::string &pattern) {
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
