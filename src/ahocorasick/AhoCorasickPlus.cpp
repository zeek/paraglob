/*
 * AhoCorasickPlus.cpp: A sample C++ wrapper for Aho-Corasick C library
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

 * Modified by Jon Siwek: add "copy" flag to addPattern() methods
 * Modified by Jon Siwek: fix addPattern() to set pattern ID type to "number"
*/

#include "ahocorasick.h"
#include "AhoCorasickPlus.h"

AhoCorasickPlus::AhoCorasickPlus ()
{
    m_automata = ac_trie_create ();
    m_acText = new AC_TEXT_t;
}

AhoCorasickPlus::~AhoCorasickPlus ()
{
    ac_trie_release (m_automata);
    delete m_acText;
}


AhoCorasickPlus::EnumReturnStatus AhoCorasickPlus::addPattern
    (std::string_view pattern, PatternId id, bool copy)
{
    // Adds zero-terminating string

    EnumReturnStatus rv = RETURNSTATUS_FAILED;

    AC_PATTERN_t patt;
    patt.ptext.astring = (AC_ALPHABET_t*) pattern.data();
    patt.ptext.length = pattern.size();
    patt.id.u.number = id;
    patt.id.type = AC_PATTID_TYPE_NUMBER;
    patt.rtext.astring = NULL;
    patt.rtext.length = 0;

    AC_STATUS_t status = ac_trie_add (m_automata, &patt, copy);

    switch (status)
    {
        case ACERR_SUCCESS:
            rv = RETURNSTATUS_SUCCESS;
            break;
        case ACERR_DUPLICATE_PATTERN:
            rv = RETURNSTATUS_DUPLICATE_PATTERN;
            break;
        case ACERR_LONG_PATTERN:
            rv = RETURNSTATUS_LONG_PATTERN;
            break;
        case ACERR_ZERO_PATTERN:
            rv = RETURNSTATUS_ZERO_PATTERN;
            break;
        case ACERR_TRIE_CLOSED:
            rv = RETURNSTATUS_AUTOMATA_CLOSED;
            break;
    }
    return rv;
}

void AhoCorasickPlus::finalize () {
    ac_trie_finalize (m_automata);
}

void AhoCorasickPlus::search (const std::string &text, bool keep)
{
    m_acText->astring = text.c_str();
    m_acText->length = text.size();
    ac_trie_settext (m_automata, m_acText, (int)keep);
}

std::vector<int> AhoCorasickPlus::findAll (const std::string& text, bool keep)
{
  this->search(text, keep);

  std::vector<int> IDs;
  AC_MATCH_t matchp;
  unsigned int j;

  while ((matchp = ac_trie_findnext (m_automata)).size)
  {
      for (j = 0; j < matchp.size; j++)
      {
          // Add the id to our vector
          IDs.push_back(matchp.patterns[j].id.u.number);
      }
  }

  return IDs;
}
