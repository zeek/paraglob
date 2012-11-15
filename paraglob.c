
#include <ctype.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <fnmatch.h>

#include "paraglob.h"
#include "set.h"
#include "vector.h"

#include "multifast-ac/ahocorasick.h"

struct _pg_word;
struct _pg_meta_word;
struct _pg_pattern;

static int _compare_meta_words(struct _pg_meta_word* w1, struct _pg_meta_word* w2);
static int _compare_uint64(const void* u1, const void* u2);
static int _compare_words(struct _pg_word* w1, struct _pg_word* w2);

DECLARE_SET(meta_word, struct _pg_meta_word*, uint64_t, _compare_meta_words)
DECLARE_SET(pattern, struct _pg_pattern*, uint64_t,  SET_STD_EQUAL);
DECLARE_SET(word, struct _pg_word*, uint64_t, _compare_words)
DECLARE_VECTOR(pattern, struct _pg_pattern*, uint64_t);
DECLARE_VECTOR(word, struct _pg_word*, uint64_t)

struct _pg_meta_word {
    set_word* words;
    vec_pattern* patterns;
};

struct _pg_pattern {
    set_word* words;
    void *cookie;
    uint64_t len;
    char data[];
};

struct _pg_word {
    set_meta_word* meta_words; // Meta words that contain this word.
    uint64_t len;
    char data[];
};

struct __paraglob {
    // Configuration.
    enum paraglob_encoding encoding;
    paraglob_match_callback* callback;
    FILE* debug;

    // Compile-time state.
    vec_pattern* patterns;
    set_word* words;
    set_meta_word* meta_words;
    AC_AUTOMATA_t* ac;
    AC_ERROR_t ac_error;

    // Matching state.
    enum paraglob_error error;
    uint64_t matches;
    uint64_t fnmatches;
    set_word* matching_words;
    set_pattern* matching_patterns;
    const char* needle;
    uint64_t needle_len;
};

typedef struct _pg_meta_word pg_meta_word;
typedef struct _pg_pattern pg_pattern;
typedef struct _pg_word pg_word;

static int _compare_uint64(const void* u1, const void* u2)
{
    return (uint64_t)u1 - (uint64_t)u2;
}

static int _compare_words(pg_word* w1, pg_word* w2)
{
    int min = (w1->len <= w2->len) ? w1->len : w2->len;
    int n = memcmp(&w1->data, &w2->data, min);

    if ( n != 0 )
        return n;

    return w1->len - w2->len;
}

static int _compare_meta_words(pg_meta_word* w1, pg_meta_word* w2)
{
    return w1 - w2; // Pointer comparision.
}

static void _print_meta_word(FILE* out, pg_meta_word* mw);
static void _print_word(FILE* out, pg_word* mw);

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
            pg_word* word = calloc(sizeof(pg_word), len);
            word->len = len;
            word->meta_words = set_meta_word_create(0);
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

static void _print_word(FILE* out, pg_word* w)
{
    _safe_print(out, w->len, w->data);
}

static void _print_meta_word(FILE* out, pg_meta_word* mw)
{
    fprintf(out, "(");

    int first = 1;
    set_for_each(word, mw->words, w) {
        if ( ! first )
            fprintf(out, "|");

        _print_word(out, w);
        first = 0;
    }

    fprintf(out, ")");
}

static void _verify_match(paraglob_t pg, pg_pattern* p)
{
    if ( pg->debug ) {
        fprintf(pg->debug, "Match candidate: |");
        _safe_print(pg->debug, p->len, p->data);
        fprintf(pg->debug, "|\n");
    }

    // FIXME: We need a glob function that can deal with a length specifier.
    char data[p->len + 1];
    char needle[pg->needle_len + 1];

    memcpy(data, p->data, p->len);
    memcpy(needle, pg->needle, pg->needle_len);

    data[p->len] = '\0';
    needle[pg->needle_len] = '\0';

    ++pg->fnmatches;

    if ( fnmatch(data, needle, 0) == 0 ) {
        if ( pg->debug )
            fprintf(stderr, "==> Match found!\n");

        if ( pg->callback )
            pg->callback(p->len, p->data, p->cookie);

        ++pg->matches;
    }
}

static int _ac_match_callback(AC_MATCH_t *match, void * cookie)
{
    paraglob_t pg = (paraglob_t) cookie;

    int i;

    for ( i = 0; i < match->match_num; i++ ) {
        pg_word* w = (pg_word*)match->patterns[i].rep.stringy;

        if ( pg->debug ) {
            fprintf(pg->debug, "Word match: |");
            _safe_print(pg->debug, w->len, w->data);
            fprintf(pg->debug, "| (%p)\n", cookie);
        }

        set_word_insert(pg->matching_words, w);
    }

    return 0;
}

int _ac_meta_words_match_callback(AC_MATCH_t *match, void * cookie)
{
    paraglob_t pg = (paraglob_t) cookie;

    int i, j;
    for ( i = 0; i < match->match_num; i++ ) {
        pg_meta_word* mw = (pg_meta_word*) match->patterns[i].rep.stringy;

        if ( pg->debug ) {
            fprintf(pg->debug, "Meta word match: ");
            _print_meta_word(pg->debug, mw);
            fprintf(pg->debug, "\n");
        }

        vec_for_each(pattern, mw->patterns, p)
            _verify_match(pg, p);
    }

    return 0;
}

static int _ac_build(paraglob_t pg)
{
    pg->ac = ac_automata_init(_ac_match_callback);

    set_for_each(word, pg->words, w) {
        AC_ALPHABET_t* data = calloc(w->len, sizeof(AC_ALPHABET_t));
        int i;
        for ( i = 0; i < w->len; i++ )
            data[i] = (AC_ALPHABET_t)(intptr_t)w->data[i];

        AC_PATTERN_t p;
        p.astring = data;
        p.length = w->len;
        p.rep.stringy = (char *)w;
        pg->ac_error = ac_automata_add(pg->ac, &p);

        if ( pg->ac_error != ACERR_SUCCESS ) {
            pg->error = PARAGLOB_ERROR_AC;
            return 0;
        }

        free(data);
    }

    ac_automata_finalize(pg->ac);

    ///

    vec_for_each(pattern, pg->patterns, p1) {
        uint64_t mw_len = set_word_size(p1->words);
        pg_meta_word* mw = calloc(sizeof(pg_meta_word) + mw_len * sizeof(pg_word*), 1);
        mw->words = p1->words;

        pg_meta_word* cur_mw = set_meta_word_get(pg->meta_words, mw);

        if ( ! cur_mw ) {
            mw->patterns = vec_pattern_create(0);
            set_meta_word_insert(pg->meta_words, mw);

            set_for_each(word, p1->words, w)
                set_meta_word_insert(w->meta_words, mw);
        }

        else
            free(mw);
    }

    ///

    vec_for_each(pattern, pg->patterns, p2) {
        set_for_each(meta_word, pg->meta_words, mw) {
            if ( set_word_is_subset(p2->words, mw->words) )
                vec_pattern_append(mw->patterns, p2);
        }
    }

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
    pg->meta_words = set_meta_word_create(0);
    pg->error = PARAGLOB_ERROR_NONE;
    pg->debug = 0;
    pg->ac = 0;
    pg->ac_error = ACERR_SUCCESS;
    pg->matches = 0;
    pg->fnmatches = 0;
    pg->matching_words = set_word_create(0);
    pg->matching_patterns = set_pattern_create(0);
    pg->needle = 0;
    pg->needle_len = 0;

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

    set_for_each(meta_word, pg->meta_words, mw) {
        free(mw);
    }

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
    if ( ! pg->ac ) {
        pg->error = PARAGLOB_ERROR_NOT_COMPILED;
        return 0;
    }

    // Rest parse state.
    pg->error = PARAGLOB_ERROR_NONE;
    pg->matches = 0;
    pg->fnmatches = 0;
    pg->needle = needle;
    pg->needle_len = len;

    set_word_clear(pg->matching_words, 0);
    ac_automata_reset(pg->ac);

    // Send data to AC matcher.

    while ( len-- ) {
        intptr_t i = *needle++;

        AC_TEXT_t t;
        AC_ALPHABET_t n = (AC_ALPHABET_t)i;
        t.astring = &n;
        t.length = 1;

        if ( ac_automata_search (pg->ac, &t, pg) < 0 ) {
            pg->error = PARAGLOB_ERROR_NOT_COMPILED;
            return 0;
        }
    }

    // Find the match candidates.

    int m = set_word_size(pg->matching_words);

    if ( pg->debug ) {
        fprintf(pg->debug, "All matching words:");

        set_for_each(word, pg->matching_words, w) {
            fprintf(pg->debug, " ");
            _print_word(pg->debug, w);
        }

        fprintf(pg->debug, "\n");
    }

    set_for_each(word, pg->matching_words, w) {
        set_for_each(meta_word, w->meta_words, mw) {
            if ( ! set_word_is_subset(pg->matching_words, mw->words) )
                continue;
            if ( pg->debug ) {
                fprintf(pg->debug, "Meta word match: ");
                _print_meta_word(pg->debug, mw);
                fprintf(pg->debug, "\n");
            }

            vec_for_each(pattern, mw->patterns, p) {
                set_pattern_insert(pg->matching_patterns, p);
            }
        }
    }

    set_for_each(pattern, pg->matching_patterns, p)
        _verify_match(pg, p);

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

void paraglob_stats(paraglob_t pg, uint64_t* fnmatches)
{
    *fnmatches = pg->fnmatches;
}

void paraglob_enable_debug(paraglob_t pg, FILE* debug)
{
    pg->debug = debug;
}

void paraglob_dump_debug(paraglob_t pg)
{
    if ( ! pg->debug )
        return;

    fprintf(stderr, "\n");
    fprintf(stderr, "Patterns:\n");

    vec_for_each(pattern, pg->patterns, p) {
        fprintf(stderr, "  ");
        _safe_print(pg->debug, p->len, p->data);

        fprintf(pg->debug, " ->");

        set_for_each(word, p->words, w) {
            fprintf(pg->debug, " |");
            _safe_print(pg->debug, w->len, w->data);
            fprintf(pg->debug, "|");
        }

        fprintf(pg->debug, " (Cookie %p) ", p->cookie);

        fprintf(pg->debug, "\n");
    }

    fprintf(stderr, "\n");
    fprintf(stderr, "Words:\n");

    int i = 0;
    set_for_each(word, pg->words, w) {
        fprintf(stderr, "  ");
        _print_word(pg->debug, w);
        fprintf(stderr, "\n");

        set_for_each(meta_word, w->meta_words, mw) {
            fprintf(stderr, "    + ");
            _print_meta_word(pg->debug, mw);
            fprintf(stderr, "\n");
        }
    }

    fprintf(stderr, "\n");
    fprintf(stderr, "Meta words:\n");

    set_for_each(meta_word, pg->meta_words, mw) {
        fprintf(stderr, "  ");
        _print_meta_word(pg->debug, mw);
        fprintf(stderr, "\n");
    }

    fprintf(stderr, "\n");

    if ( pg->ac ) {
        fprintf(pg->debug, "Aho-Corasick for chars: compiled\n");
        //ac_automata_display(pg->ac, 'n');
    }

    else
        fprintf(pg->debug, "Aho-Corasick for chars: not compiled yet\n");
}

