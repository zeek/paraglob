
// External interface.

#ifndef PARAGLOB_H
#define PARAGLOB_H

#include <stdio.h>
#include <stdint.h>

struct __paraglob_t;
typedef struct __paraglob* paraglob_t;

enum paraglob_encoding { PARAGLOB_ASCII };
typedef void paraglob_match_callback(uint64_t pattern_len, const char* pattern, void* cookie);

paraglob_t paraglob_create(enum paraglob_encoding encoding);

int        paraglob_insert(paraglob_t pg, uint64_t len, const char* pattern, void* cookie);

int        paraglob_compile(paraglob_t pg);

uint64_t   paraglob_match(paraglob_t pg, uint64_t len, const char* needle, paraglob_match_callback* callback);

void       paraglob_delete(paraglob_t pg);

void       paraglob_dump_debug(paraglob_t pg, FILE* out);

#endif



