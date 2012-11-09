
// External interface.

#ifndef PARAGLOB_H
#define PARAGLOB_H

#include <stdio.h>
#include <stdint.h>

struct __paraglob_t;
typedef struct __paraglob* paraglob_t;

enum paraglob_encoding { PARAGLOB_ASCII };
enum paraglob_error    { PARAGLOB_ERROR_NONE = 0, PARAGLOB_ERROR_AC, PARAGLOB_ERROR_NOT_COMPILED };

typedef void paraglob_match_callback(uint64_t pattern_len, const char* pattern, void* cookie);

paraglob_t paraglob_create(enum paraglob_encoding encoding, paraglob_match_callback* callback, FILE* debug);

int        paraglob_insert(paraglob_t pg, uint64_t len, const char* pattern, void* cookie);

int        paraglob_compile(paraglob_t pg);

uint64_t   paraglob_match(paraglob_t pg, uint64_t len, const char* needle);

int        paraglob_match_begin(paraglob_t pg);
int        paraglob_match_next(paraglob_t pg, uint64_t len, const char* needle);
uint64_t   paraglob_match_end(paraglob_t pg);

void       paraglob_delete(paraglob_t pg);

void       paraglob_dump_debug(paraglob_t pg);

const char* paraglob_strerror(paraglob_t pg);

#endif



