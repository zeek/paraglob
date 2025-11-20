// See the file "COPYING" in the main distribution directory for copyright.

#include "paraglob/paraglob.h"

#include <cstdint>
#include <sstream>

extern "C" {
#include "aca.h"
}

#include "paraglob/exceptions.h"
#include "paraglob/serializer.h"

class aca_handle : public aca {};

using namespace paraglob;

Paraglob::Paraglob() : handle(std::make_unique<aca_handle>()) { aca_init(static_cast<aca*>(handle.get()), 256); }

Paraglob::Paraglob(const std::vector<std::string>& patterns) : handle(std::make_unique<aca_handle>()) {
    aca_init(static_cast<aca*>(handle.get()), 256);

    for ( const std::string& pattern : patterns ) {
        if ( ! (add(pattern)) ) {
            throw paraglob::add_error("Failed to add pattern: " + pattern);
        }
    }
    compile();
}

Paraglob::Paraglob(std::unique_ptr<std::vector<uint8_t>> serialized)
    : Paraglob(ParaglobSerializer::unserialize(serialized)) {}

Paraglob::~Paraglob() { aca_destroy(handle.get()); }

bool Paraglob::add(const std::string& pattern) {
    for ( const std::string& meta_word : get_meta_words(pattern) ) {
        if ( ! meta_to_node_map.contains(meta_word) ) {
            aca_add(static_cast<aca*>(handle.get()), const_cast<char*>(meta_word.c_str()),
                    static_cast<int>(meta_word.size()));
            meta_words.push_back(meta_word);
            // Build the new paraglobNode in place.
            meta_to_node_map.emplace(std::piecewise_construct, std::forward_as_tuple(meta_word),
                                     std::forward_as_tuple(meta_word, pattern));
        }
        else {
            meta_to_node_map.at(meta_word).add_pattern(pattern);
        }
    }

    return true;
}

void Paraglob::compile() { aca_build(static_cast<aca*>(handle.get())); }

static std::set<int> hits;

static int aca_hit(int pat) {
    hits.insert(pat);
    return 0;
}

std::vector<std::string> Paraglob::get(const std::string& text) {
    // Narrow to the meta-word matches
    std::vector<std::string> patterns;

    hits.clear();
    aca_iter it = aca_root(static_cast<aca*>(handle.get()));
    for ( char ch : text ) {
        it = aca_next(it, ch, aca_hit);
    }

    for ( int hit : hits )
        meta_to_node_map.at(meta_words.at(hit)).merge_matches(patterns, text);

    // Single wildcards always need to be checked
    if ( ! single_wildcards.empty() )
        patterns.insert(patterns.end(), single_wildcards.begin(), single_wildcards.end());

    // Remove duplicates
    std::ranges::sort(patterns);
    patterns.erase(std::unique(patterns.begin(), patterns.end()), patterns.end());
    return patterns;
}

std::vector<std::string> Paraglob::split_on_brackets(const std::string& in) const {
    std::vector<std::string> out;
    size_t pos;
    size_t prev = 0;

    while ( (pos = in.find_first_of('[', prev)) != std::string::npos ) {
        size_t end_bracket = in.find_first_of(']', pos);
        if ( end_bracket != std::string::npos ) {
            out.push_back(in.substr(prev, pos - prev));
            prev = end_bracket + 1;
        }
        else {
            break;
        }
    }

    // There are no more opening / closing brackets
    // Append the rest of the string
    out.push_back(in.substr(prev, in.length() - prev));
    return out;
}


std::vector<std::string> Paraglob::get_meta_words(const std::string& pattern) {
    std::vector<std::string> meta_words;

    // Split the pattern by brackets
    for ( const std::string& word : split_on_brackets(pattern) ) {
        // Parse each bracket section
        std::size_t prev = 0, pos;

        while ( (pos = word.find_first_of("*?", prev)) != std::string::npos ) {
            if ( pos > prev ) {
                meta_words.push_back(word.substr(prev, pos - prev));
            }
            prev = pos + 1;
        }
        if ( prev < word.length() ) {
            meta_words.push_back(word.substr(prev, std::string::npos));
        }
    }

    if ( meta_words.size() == 0 && pattern != "" ) {
        single_wildcards.push_back(pattern);
    }
    return meta_words;
}

std::vector<std::string> Paraglob::get_patterns() const {
    std::vector<std::string> patterns;
    // Merge in all of the nodes patterns
    for ( const auto& it : meta_to_node_map ) {
        it.second.merge_patterns(patterns);
    }
    if ( single_wildcards.size() > 0 )
        patterns.insert(patterns.end(), single_wildcards.begin(), single_wildcards.end());

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
std::unique_ptr<std::vector<uint8_t>> Paraglob::serialize() const {
    return ParaglobSerializer::serialize(get_patterns());
}

std::string Paraglob::str() const {
    std::stringstream ss;

    auto add_string = [&ss](const std::string& p) { ss << p << " "; };
    auto pretty_add = [add_string](const std::vector<std::string>& v) {
        add_string("[");
        if ( v.size() > 6 ) {
            std::for_each(v.begin(), v.begin() + 3, add_string);
            add_string("...");
            std::for_each(v.rbegin(), v.rbegin() + 3, add_string);
        }
        else {
            std::for_each(v.begin(), v.end(), add_string);
        }
        add_string("]\n");
    };

    add_string("paraglob:\nmeta words: ");

    pretty_add(meta_words);
    add_string("patterns:");
    pretty_add(get_patterns());

    return ss.str();
}

bool Paraglob::operator==(const Paraglob& other) const { return (meta_to_node_map == other.meta_to_node_map); }
