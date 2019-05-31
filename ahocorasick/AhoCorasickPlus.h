/*
 * AhoCorasickPlus.h: This is the header file for a sample
 * C++ wrapper for Aho-Corasick C library
 *
 * This file is part of multifast.
 *
    Copyright 2010-2015 Kamiar Kanani <kamiar.kanani@gmail.com>

    multifast is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    multifast is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with multifast.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef AHOCORASICKPPW_H_
#define AHOCORASICKPPW_H_

#include <string>
#include <queue>
#include <vector>

// Forward declarations
struct ac_trie;
struct ac_text;


class AhoCorasickPlus
{
public:

    enum EnumReturnStatus
    {
        RETURNSTATUS_SUCCESS = 0,       // No error occurred
        RETURNSTATUS_DUPLICATE_PATTERN, // Duplicate patterns
        RETURNSTATUS_LONG_PATTERN,      // Long pattern
        RETURNSTATUS_ZERO_PATTERN,      // Empty pattern (zero length)
        RETURNSTATUS_AUTOMATA_CLOSED,   // Automata is closed
        RETURNSTATUS_FAILED,            // General unknown failure
    };

    typedef unsigned int PatternId;

    struct Match
    {
        unsigned int    position;
        PatternId       id;
    };

public:

    AhoCorasickPlus();
    ~AhoCorasickPlus();

    EnumReturnStatus addPattern (const std::string &pattern, PatternId id);
    EnumReturnStatus addPattern (const char pattern[], PatternId id);
    void             finalize   ();

    std::vector<int> findAll (const std::string& text, bool keep);

    /* Returns true if the automata is compiled. */
    bool in_good_standing () const;

private:
    void search   (const std::string &text, bool keep);
    struct ac_trie      *m_automata;
    struct ac_text      *m_acText;
};

#endif /* AHOCORASICKPPW_H_ */
