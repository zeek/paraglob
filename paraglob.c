
#include <ctype.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

#include "paraglob.h"
#include "set.h"
#include "vector.h"

struct _pg_word {
    uint64_t len;
    char data[];
};

typedef struct _pg_word pg_word;

DECLARE_SET(word, pg_word*, uint64_t, SET_STD_EQUAL)

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
    vec_pattern* patterns;
    set_word* words;  // All words; owns them.
};

static set_word* _computeWords(paraglob_t pg, uint64_t len, const char* data)
{
    assert(pg->encoding == PARAGLOB_ASCII);

    set_word* words = set_word_create(0);

    const char* s = data;
    const char* t = data;

    while ( t < (data + len + 1) ) {

        if ( *t != '*' && t < (data + len) ) {
            ++t;
            continue;
        }

        if ( s != t ) {
            // Extract word.
            uint64_t len = (t -s);
            pg_word* word = malloc(sizeof(pg_word) + len);
            word->len = len;
            memcpy(&word->data, s, t - s);
            set_word_insert(words, word);
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

paraglob_t paraglob_create(enum paraglob_encoding encoding)
{
    paraglob_t pg = calloc(sizeof(struct __paraglob), 1);

    if ( ! pg )
        return 0;

    pg->encoding = encoding;
    pg->patterns = vec_pattern_create(0);
    pg->words = set_word_create(0);

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

    set_word_join(pg->words, words);

    return (p->words != 0);
}

int paraglob_compile(paraglob_t pg)
{
    return 1;
}

uint64_t paraglob_match(paraglob_t pg, uint64_t len, const char* needle, paraglob_match_callback* callback)
{
    return 0;
}

void paraglob_dump_debug(paraglob_t pg, FILE* out)
{
    vec_for_each(pattern, pg->patterns, p) {
        fprintf(out, "Pattern: |");
        _safe_print(out, p->len, p->data);
        fprintf(out, "|");

        fprintf(out, " ->");

        set_for_each(word, p->words, w) {
            fprintf(out, " |");
            _safe_print(out, w->len, w->data);
            fprintf(out, "|");
        }

        fprintf(out, " (Cookie %p) ", p->cookie);

        fprintf(out, "\n");
    }

    fprintf(out, "Global word set:");

    set_for_each(word, pg->words, w) {
        fprintf(out, " |");
        _safe_print(out, w->len, w->data);
        fprintf(out, "|");
    }

    fprintf(out, "\n");
}
