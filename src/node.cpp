// See the file "COPYING" in the main distribution directory for copyright.

#include "paraglob/node.h"

#ifndef _WIN32
#include <fnmatch.h>
#else
#include <Shlwapi.h>
#define FNM_NOMATCH 1

namespace {

int fnmatch(const char* pattern, const char* string, int flags) {
    BOOL match = PathMatchSpecA(string, pattern); // flags are not supported in Windows
    return match ? 0 : FNM_NOMATCH;
}

}
#endif

#include <algorithm> // copy_if
#include <iterator>

using namespace paraglob;

/* Merges this nodes matching patterns into the input vector. */
void ParaglobNode::merge_matches(std::vector<std::string>& target, const std::string& text) const {
    std::copy_if(patterns.begin(), patterns.end(), std::back_inserter(target),
                 [text](const std::string& candidate) { return (fnmatch(candidate.c_str(), text.c_str(), 0) == 0); });
}

// Merges this nodes patterns into the input vector
// Note: this could be done more efficiently with a move iterator if we wanted
// this to be destructive.
void ParaglobNode::merge_patterns(std::vector<std::string>& target) const {
    target.insert(target.begin(), patterns.begin(), patterns.end());
}
