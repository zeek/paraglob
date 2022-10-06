// See the file "COPYING" in the main distribution directory for copyright.

#include "paraglob/paraglob.h"

#include "ahocorasick/AhoCorasickPlus.h"

paraglob::Paraglob::Paraglob()
  : my_ac(new AhoCorasickPlus) {}

paraglob::Paraglob::Paraglob(const std::vector<std::string>& patterns)
  : my_ac(new AhoCorasickPlus) {
  for (const std::string& pattern : patterns) {
    if ( !(this->add(pattern)) ) {
      throw paraglob::add_error("Failed to add pattern: " + pattern);
    }
  }
  this->compile();
}

paraglob::Paraglob::Paraglob(std::unique_ptr<std::vector<uint8_t>> serialized)
  : Paraglob(paraglob::ParaglobSerializer::unserialize(serialized)) {}

paraglob::Paraglob::~Paraglob() = default;

bool paraglob::Paraglob::add(const std::string& pattern) {
  AhoCorasickPlus::EnumReturnStatus status;

  for (const std::string& meta_word : this->get_meta_words(pattern)) {
    AhoCorasickPlus::PatternId patId = this->meta_words.size();
    status = this->my_ac->addPattern(meta_word, patId, true);

    if (status == AhoCorasickPlus::RETURNSTATUS_SUCCESS) {
      this->meta_words.push_back(meta_word);
      // Build the new paraglobNode in place.
      this->meta_to_node_map.emplace(
        std::piecewise_construct, std::forward_as_tuple(meta_word),
        std::forward_as_tuple(meta_word, pattern)
      );
    } else if (status == AhoCorasickPlus::RETURNSTATUS_DUPLICATE_PATTERN) {
      this->meta_to_node_map.at(meta_word).add_pattern(pattern);
    } else { // Failed to add
      return false;
    }
  }

  return true;
}

void paraglob::Paraglob::compile() {
  this->my_ac->finalize();
}

std::vector<std::string> paraglob::Paraglob::get(const std::string& text) {
  // Narrow to the meta-word matches
  std::vector<std::string> patterns;
  for (int id : this->my_ac->findAll(text, false))
    this->meta_to_node_map.at(this->meta_words.at(id)).merge_matches(patterns, text);

  // Single wildcards always need to be checked
  if (this->single_wildcards.size() > 0)
    patterns.insert(patterns.end(), this->single_wildcards.begin(), this->single_wildcards.end());

  // Remove duplicates
  std::sort(patterns.begin(), patterns.end());
  patterns.erase(unique(patterns.begin(), patterns.end()), patterns.end());
  return patterns;
}

std::vector<std::string> paraglob::Paraglob::split_on_brackets(const std::string &in) const {
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
  for (const std::string& word : split_on_brackets(pattern)) {
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

std::vector<std::string> paraglob::Paraglob::get_patterns() const {
  std::vector<std::string> patterns;
  // Merge in all of the nodes patterns
  for (const auto& it : this->meta_to_node_map) {
    it.second.merge_patterns(patterns);
  }
  if (this->single_wildcards.size() > 0)
    patterns.insert(patterns.end(), this->single_wildcards.begin(), this->single_wildcards.end());

  // Remove the duplicate patterns. Duplicates don't effect the state.
  std::sort(patterns.begin(), patterns.end());
  patterns.erase(unique(patterns.begin(), patterns.end()), patterns.end());

  return patterns;
}

// Returns a string representation of the paraglob that it can rebuild
// itself from. A paraglobs state is completely defined by the vector of patterns
// that it contains.
//
// NOTE: Ideally, we'd like to serialize a paraglob in such a way that it can be
// unserialized without having to compile itself, but this proves to be very
// non-trivial. While its surely possible, the multifast data structure
// maintains a complex system of nodes, pointers to nodes, and doesn't store
// itself in memory contiguously. Without a pressing use case for this
// functionality, right now we're choosing not to do this. Instead, paraglob
// serializes its vector of patterns, and rebuilds itself when unserialized.
std::unique_ptr<std::vector<uint8_t>> paraglob::Paraglob::serialize() const {
  return paraglob::ParaglobSerializer::serialize(this->get_patterns());
}

std::string paraglob::Paraglob::str() const {
  std::stringstream ss;

  auto add_string = [&ss](const std::string& p){ ss << p << " "; };
  auto pretty_add = [add_string](const std::vector<std::string>& v) {
    add_string("[");
    if (v.size() > 6) {
      std::for_each(v.begin(), v.begin() + 3, add_string);
      add_string("...");
      std::for_each(v.rbegin(), v.rbegin() + 3, add_string);
    } else {
      std::for_each(v.begin(), v.end(), add_string);
    }
    add_string("]\n");
  };

  add_string("paraglob:\nmeta words: ");

  pretty_add(this->meta_words);
  add_string("patterns:");
  pretty_add(this->get_patterns());

  return ss.str();
}

bool paraglob::Paraglob::operator==(const Paraglob &other) const {
  return (this->meta_to_node_map == other.meta_to_node_map);
}
