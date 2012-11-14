
#include <ctype.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <fnmatch.h>

#include "paraglob.h"
#include "set.h"
#include "vector.h"

#include "multifast-ac/ahocorasick.h"

struct _pg_word {
    uint64_t len;
    char data[];
};

typedef struct _pg_word pg_word;

static int _compare_words(pg_word* w1, pg_word* w2)
{
    int min = (w1->len <= w2->len) ? w1->len : w2->len;
    int n = memcmp(&w1->data, &w2->data, min);

    if ( n != 0 )
        return n;

    return w1->len - w2->len;
}

DECLARE_SET(word, pg_word*, uint64_t, _compare_words)

struct _pg_pattern {
    set_word* words;
    void *cookie;
    uint64_t len;
    char data[];
};

typedef struct _pg_pattern pg_pattern;

DECLARE_VECTOR(pattern, pg_pattern*, uint64_t);

struct __paraglob {
    enum paraglob_encoding encoding;
    paraglob_match_callback* callback;
    FILE* debug;

    vec_pattern* patterns;
    set_word* words;  // All words; this structure owns them.
    enum paraglob_error error;
    AC_AUTOMATA_t* ac_chars;
    AC_AUTOMATA_t* ac_words;
    AC_ERROR_t ac_error;

    uint64_t matches;
    set_word* matching_words;
    char* needle;
    uint64_t needle_len;
    int needle_free;
};

static set_word* _computeWords(paraglob_t pg, uint64_t len, const char* data)
{
    assert(pg->encoding == PARAGLOB_ASCII);

    set_word* words = set_word_create(0);

    const char* s = data;
    const char* t = data;

    const char* separators = "*?";

    while ( t < (data + len + 1) ) {

        if ( t < (data + len) ) {
            if ( (strchr(separators, *t) == 0) && (t == data || t[-1] != '\\') ) {
                ++t;
                continue;
            }
        }

        if ( s != t ) {
            // Extract word.
            uint64_t len = (t -s);
            pg_word* word = malloc(sizeof(pg_word) + len);
            word->len = len;
            memcpy(&word->data, s, t - s);

            pg_word* cur_word = set_word_get(pg->words, word);
            if ( cur_word ) {
                set_word_insert(words, cur_word);
                free(word);
            }

            else {
                set_word_insert(words, word);
                set_word_insert(pg->words, word);
            }

        }

        ++t;
        s = t;
    }

    return words;
}

static void _safe_print(FILE* out, uint64_t len, const char* data)
{
    while ( len-- ) {
        if ( isprint(*data) )
            fputc(*data++, out);
        else
            fprintf(out, "\\%02x", *data++);
    }
}

static void _verify_match(paraglob_t pg, pg_pattern* p)
{
    if ( pg->debug ) {
        fprintf(pg->debug, "Match candidate: |");
        _safe_print(pg->debug, p->len, p->data);
        fprintf(pg->debug, "|\n");
    }

    // FIXME: We need a glob function that can deal with a length specifier.
    if ( fnmatch(p->data, pg->needle, 0) == 0 ) {
        if ( pg->debug )
            fprintf(stderr, "==> Match found!\n");

        if ( pg->callback )
            pg->callback(p->len, p->data, p->cookie);

        ++pg->matches;
    }
}

static int _ac_chars_match_callback(AC_MATCH_t *match, void * cookie)
{
    paraglob_t pg = (paraglob_t) cookie;

    int i;

    for ( i = 0; i < match->match_num; i++ ) {

        pg_word* w = (pg_word*)match->patterns[i].rep.stringy;
        AC_ALPHABET_t a = (AC_ALPHABET_t)w;

        if ( pg->debug ) {
            fprintf(pg->debug, "Word match: |");
            _safe_print(pg->debug, w->len, w->data);
            fprintf(pg->debug, "| (%p)\n", a);
        }

        set_word_insert(pg->matching_words, w);
    }

    return 0;
}

int _ac_words_match_callback(AC_MATCH_t *match, void * cookie)
{
    paraglob_t pg = (paraglob_t) cookie;

    int i;
    for ( i = 0; i < match->match_num; i++ )
        _verify_match(pg, match->patterns[i].rep.stringy);

    return 0;
}

static int _ac_build(paraglob_t pg)
{
    pg->ac_chars = ac_automata_init(_ac_chars_match_callback);

    set_for_each(word, pg->words, w) {

        AC_ALPHABET_t* data = malloc(w->len * sizeof(AC_ALPHABET_t));
        int i;
        for ( i = 0; i < w->len; i++ )
            data[i] = (AC_ALPHABET_t)(intptr_t)w->data[i];

        AC_PATTERN_t p;
        p.astring = data;
        p.length = w->len;
        p.rep.stringy = w;
        pg->ac_error = ac_automata_add(pg->ac_chars, &p);

        if ( pg->ac_error != ACERR_SUCCESS ) {
            pg->error = PARAGLOB_ERROR_AC;
            return 0;
        }
    }

    ac_automata_finalize(pg->ac_chars);

    ///

    pg->ac_words = ac_automata_init(_ac_words_match_callback);

    vec_for_each(pattern, pg->patterns, pat) {

        AC_ALPHABET_t* data = malloc(set_word_size(pat->words) * sizeof(AC_ALPHABET_t));
        AC_ALPHABET_t* dst = data;

        set_for_each(word, pat->words, w)
            *dst++ = (AC_ALPHABET_t)(w);

        AC_PATTERN_t p;
        p.astring = data;
        p.length = set_word_size(pat->words);
        p.rep.stringy = pat;
        pg->ac_error = ac_automata_add(pg->ac_words, &p);

        if ( pg->ac_error != ACERR_SUCCESS ) {
            pg->error = PARAGLOB_ERROR_AC;
            return 0;
        }
    }

    ac_automata_finalize(pg->ac_words);

    return 1;
}

paraglob_t paraglob_create(enum paraglob_encoding encoding, paraglob_match_callback* callback)
{
    paraglob_t pg = calloc(sizeof(struct __paraglob), 1);

    if ( ! pg )
        return 0;

    pg->encoding = encoding;
    pg->callback = callback;
    pg->patterns = vec_pattern_create(0);
    pg->words = set_word_create(0);
    pg->error = PARAGLOB_ERROR_NONE;
    pg->debug = 0;
    pg->ac_chars = 0;
    pg->ac_words = 0;
    pg->ac_error = ACERR_SUCCESS;
    pg->matches = 0;
    pg->matching_words = 0;
    pg->needle = 0;
    pg->needle_len = 0;
    pg->needle_free = 0;

    return pg;
}

void paraglob_delete(paraglob_t pg)
{
    vec_for_each(pattern, pg->patterns, p) {
        free(p);
    }

    set_for_each(word, pg->words, w) {
        free(w);
    }

    if ( pg->needle && pg->needle_free )
        free(pg->needle);

    free(pg);
}

int paraglob_insert(paraglob_t pg, uint64_t len, const char* data, void* cookie)
{
    pg_pattern* p = calloc(sizeof(pg_pattern) + len, 1);

    if ( ! p )
        return 0;

    set_word* words = _computeWords(pg, len, data);

    if ( ! words )
        return 0;

    p->words = words;
    p->cookie = cookie;
    p->len = len;
    memcpy(&p->data, data, len);

    vec_pattern_append(pg->patterns, p);

    return (p->words != 0);
}

int paraglob_compile(paraglob_t pg)
{
    if ( ! _ac_build(pg) )
        return 0;

    return 1;
}

uint64_t paraglob_match(paraglob_t pg, uint64_t len, const char* needle)
{
    if ( ! paraglob_match_begin(pg) )
        return 0;

    if ( ! paraglob_match_next(pg, len, needle) )
        return 0;

    return paraglob_match_end(pg);
}

int paraglob_match_begin(paraglob_t pg)
{
    if ( ! (pg->ac_chars && pg->ac_words) ) {
        pg->error = PARAGLOB_ERROR_NOT_COMPILED;
        return 0;
    }

    ac_automata_reset(pg->ac_chars);
    ac_automata_reset(pg->ac_words);

    pg->matches = 0;
    pg->needle = 0;
    pg->needle_len = 0;
    pg->needle_free = 0;

    if ( ! pg->matching_words )
        pg->matching_words = set_word_create(0);
    else
        set_word_clear(pg->matching_words);

    return 1;
}

int paraglob_match_next(paraglob_t pg, uint64_t len, const char* needle)
{
    pg->needle = realloc(pg->needle, pg->needle_len + len);
    memcpy(pg->needle + pg->needle_len, needle, len);
    pg->needle_free = 1;

    while ( len-- ) {
        intptr_t i = *needle++;

        AC_TEXT_t t;
        AC_ALPHABET_t n = (AC_ALPHABET_t)i;
        t.astring = (AC_ALPHABET_t) &n;
        t.length = 1;

        if ( ac_automata_search (pg->ac_chars, &t, pg) < 0 ) {
            pg->error = PARAGLOB_ERROR_NOT_COMPILED;
            return 0;
        }
    }

    return 1;
}

uint64_t paraglob_match_end(paraglob_t pg)
{
    int m = set_word_size(pg->matching_words);
    pg_word* wmatches[m];

    int i = 0;
    set_for_each(word, pg->matching_words, w)
        wmatches[i++] = w;

    AC_TEXT_t t;
    t.astring = (AC_ALPHABET_t*) &wmatches;
    t.length = m;

    if ( ac_automata_search (pg->ac_words, &t, pg) < 0 ) {
        pg->error = PARAGLOB_ERROR_NOT_COMPILED;
        return 1;
    }

    return pg->matches;
}

const char* paraglob_strerror(paraglob_t pg)
{
    switch ( pg->error ) {
     case PARAGLOB_ERROR_NONE:
            return "no error";

     case PARAGLOB_ERROR_NOT_COMPILED:
            return "paraglob not compiled";

     case PARAGLOB_ERROR_AC: {
         switch ( pg->ac_error ) {
             case ACERR_SUCCESS:
                return "[aho-corasick] success";

             case ACERR_DUPLICATE_PATTERN:
                return "[aho-corasick] duplicate pattern";

             case ACERR_LONG_PATTERN:
                return "[aho-corasick] overlong pattern";

             case ACERR_ZERO_PATTERN:
                    return "[aho-corasick] zero-length pattern";

             case  ACERR_AUTOMATA_CLOSED:
                return "[aho-corasick] use after close";

             default:
                return "[aho-corasick] unknown error";
            }
     }

     default:
        return "unknown error";
    }
}

void paraglob_enable_debug(paraglob_t pg, FILE* debug)
{
    pg->debug = debug;
}

void paraglob_dump_debug(paraglob_t pg)
{
    if ( ! pg->debug )
        return;

    vec_for_each(pattern, pg->patterns, p) {
        fprintf(pg->debug, "Pattern: |");
        _safe_print(pg->debug, p->len, p->data);
        fprintf(pg->debug, "|");

        fprintf(pg->debug, " ->");

        set_for_each(word, p->words, w) {
            fprintf(pg->debug, " |");
            _safe_print(pg->debug, w->len, w->data);
            fprintf(pg->debug, "|");
        }

        fprintf(pg->debug, " (Cookie %p) ", p->cookie);

        fprintf(pg->debug, "\n");
    }

    fprintf(pg->debug, "Global word set:");

    set_for_each(word, pg->words, w) {
        fprintf(pg->debug, " |");
        _safe_print(pg->debug, w->len, w->data);
        fprintf(pg->debug, "|");
    }

    fprintf(pg->debug, "\n");

    if ( pg->ac_chars ) {
        fprintf(pg->debug, "Aho-Corasick for chars: compiled\n");
        //ac_automata_display(pg->ac_chars, 'n');
    }

    else
        fprintf(pg->debug, "Aho-Corasick for chars: not compiled yet\n");

    if ( pg->ac_words ) {
        fprintf(pg->debug, "Aho-Corasick for words: compiled\n");
        //ac_automata_display(pg->ac_chars, 'n');
    }

    else
        fprintf(pg->debug, "Aho-Corasick: not compiled yet\n");
}

