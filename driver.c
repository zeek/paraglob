
// Test driver for paraglob matching.

#include <getopt.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "paraglob.h"

void usage(int rc)
{
    fprintf(stderr, "\n");
    fprintf(stderr, "paraglob [options] <text> <patterns\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "  Options:\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "     -d Dump debugging output to stderr.\n");
    fprintf(stderr, "     -h This help.\n");
    fprintf(stderr, "\n");
    exit(rc);
}

void error(const char* msg, const char* arg)
{
    if ( arg )
        fprintf(stderr, "error: %s (%s)\n", msg, arg);
    else
        fprintf(stderr, "error: %s\n", msg);
    exit(1);
}

void match_callback(uint64_t pattern_len, const char* pattern, void *cookie)
{
    char buffer[pattern_len + 1];
    memcpy(buffer, pattern, pattern_len);
    buffer[pattern_len] = '\0';

    fprintf(stderr, "Match with |%s| (cookie %" PRIu64 ")", buffer, (uint64_t)cookie);
}

int main(int argc, char** argv)
{
    int debug = 0;

    char c;

    while ( (c = getopt(argc, argv, "d")) != -1 ) {
        switch ( c ) {
         case 'd':
            debug = 1;
            break;

         case 'h':
            usage(0);
            break;

         default:
            usage(1);
        }
    }

    argc -= optind;
    argv += optind;

    if ( argc != 1 )
        usage(1);

    const char* text = argv[0];

    fprintf(stderr, "Text: |%s|\n", text);

    paraglob_t pg = paraglob_create(PARAGLOB_ASCII, match_callback, (debug ? stderr : 0));

    if ( ! pg )
        error("cannot allocate paraglob structure", 0);

    char buffer[1024];
    int cnt = 0;

    while ( 1 ) {
        if ( ! fgets(buffer, sizeof(buffer), stdin) ) {
            if ( feof(stdin) )
                break;

            error("read error from standard input: ", strerror(errno));
        }

        if ( ! paraglob_insert(pg, strlen(buffer) - 1, buffer, (void*)++cnt) )
            error("pattern error", buffer);
    }

    fprintf(stderr, "%d patterns read\n", cnt);

    if ( ! paraglob_compile(pg) )
        error("compile error", paraglob_strerror(pg));

    if ( debug )
        paraglob_dump_debug(pg);

    uint64_t matches = paraglob_match(pg, strlen(text), text);

    fprintf(stdout, "\n%" PRIu64 " matches found\n", matches);

    paraglob_delete(pg);

    exit(0);
}
