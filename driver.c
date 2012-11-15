
// Test driver for paraglob matching.

#include <getopt.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "paraglob.h"

static const char *text = 0;

void usage(int rc)
{
    fprintf(stderr, "paraglob v" PARAGLOB_VERSION "\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "paraglob [options] <text> <patterns>\n");
    fprintf(stderr, "paraglob [options] <text>\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "The 2nd variant reads patterns as lines from stdin.\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Patterns can contain wildcards '*' and '?'.\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "  Options:\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "     -b a/b/c Benchmark mode. See below.\n");
    fprintf(stderr, "     -v       In benchmark mode, verify results.\n");
    fprintf(stderr, "     -d       Dump debugging output to stderr.\n");
    fprintf(stderr, "     -h       This help.\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "  Benchmark mode parameters:\n");
    fprintf(stderr, "     a: Number of patterns to generate.\n");
    fprintf(stderr, "     b: Number of queries to perform.\n");
    fprintf(stderr, "     c: Approx. percentage of queries passing the initial AC stage (integer only)\n");
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

    assert(text);
    fprintf(stderr, "%s matches %s\n", text, buffer);
}

static const char* benchmark_pattern_words[] = {
    "aaaaaa", "bb", "cccccccccccccccc", "ddddd", "eeeeeeeee", "fffffffffffff", "gggg"
};

static struct rusage timer_state;

struct timer_result {
    double time;
    long mem;
};

void timer_start()
{
    if ( getrusage(RUSAGE_SELF, &timer_state) < 0 )
        error("getrusage failed", strerror(errno));
}

struct timer_result timer_end()
{
    struct rusage r;

    if ( getrusage(RUSAGE_SELF, &r) < 0 )
        error("getrusage failed", strerror(errno));

    double t1 = (double)(timer_state.ru_utime.tv_sec) + (double)(timer_state.ru_utime.tv_usec) / 1e6;
    double t2 = (double)(r.ru_utime.tv_sec) + (double)(r.ru_utime.tv_usec) / 1e6;

    struct timer_result t;
    t.time = t2 - t1;
    t.mem = r.ru_maxrss - timer_state.ru_maxrss;

    return t;
}

const char* random_pattern_word()
{
    int idx = rand() % (sizeof(benchmark_pattern_words) / sizeof(const char*));
    return benchmark_pattern_words[idx];
}

const char* random_word()
{
    static char buffer[1024];

    int j;
    int rounds = (rand() % 25) + 5;
    for ( j = 0; j < rounds; j++ ) {
        buffer[j] = (char)((rand() % 26) + 'a');
    }

    buffer[rounds] = '\0';

    return buffer;
}

void benchmark(const char* params, int debug, int verify)
{
    srand(time(0));

    if ( ! (params && strlen(params)) )
        error("no benchmark parameters given", 0);

    // Parse parameters.
    long num_patterns = strtol(params, (char **)&params, 10);
    if ( ! *params++ )
        error("cannot parse benchmark parameter 1", 0);

    long num_queries = strtol(params, (char **)&params, 10);
    if ( ! *params++ )
        error("cannot parse benchmark parameter 2", 0);

    long match_prob = strtol(params, (char **)&params, 10);
    if ( *params )
        error("cannot parse benchmark parameter 3", 0);

    if ( debug )
        fprintf(stderr, "Benchmark parameters: patterns=%ld queries=%ld match_prob=%ld\n",
                num_patterns, num_queries, match_prob);

    fprintf(stderr, "Creating workload ...\n");

    // Create the patterns.
    const char* patterns[num_patterns];
    char buffer[1024];
    int i, j;

    for ( i = 0; i < num_patterns; i++ ) {

        buffer[0] = '\0';

        int rounds = (rand() % 10) + 2;
        for ( j = 0; j < rounds; j++ ) {

            if ( j != 0 )
                strcat(buffer, "*");

            if ( (rand() % 10) == 0 )
                strcat(buffer, random_pattern_word());
            else
                strcat(buffer, random_word());
        }

        patterns[i] = strdup(buffer);

        if ( debug )
            fprintf(stderr, "pattern %d: %s\n", i, patterns[i]);
    }

    // Create the queries.

    const char* queries[num_queries];

    for ( i = 0; i < num_queries; i++ ) {

        buffer[0] = '\0';

        if ( (rand() % 100) <= match_prob ) {
            // Create a likely match candidate.
            int rounds = (rand() % 5) + 1;
            for ( j = 0; j < rounds; j++ ) {
                strcat(buffer, random_pattern_word());
            }
        }

        else {
            // Create a mismatch.
            int rounds = (rand() % 50) + 5;
            for ( j = 0; j < rounds; j++ ) {
                buffer[j] = (char)((rand() % 26) + 'a');
            }

            buffer[rounds] = '\0';
        }

        queries[i] = strdup(buffer);
    }

    // Create the paraglob.

    fprintf(stderr, "Creating paraglob ...\n");

    paraglob_t pg = paraglob_create(PARAGLOB_ASCII, 0);

    if ( ! pg )
        error("cannot allocate paraglob structure", 0);

    for ( i = 0; i < num_patterns; i++ ) {
        if ( ! paraglob_insert(pg, strlen(patterns[i]), patterns[i], (void*)(intptr_t)i) )
            error("pattern error", buffer);
    }

    if ( ! paraglob_compile(pg) ) {
        // We ignore duplicate patterns, that can happen.
        if ( strstr(paraglob_strerror(pg), "duplicate") == 0 )
            error("compile error", paraglob_strerror(pg));
    }

    // Do the matching.

    fprintf(stderr, "Matching ...\n");

    int matching_queries[num_queries];

    for ( i = 0; i < num_queries; i++ )
        matching_queries[i] = 0;

    timer_start();

    uint64_t matches = 0;
    uint64_t fnmatches = 0;

    for ( i = 0; i < num_queries; i++ ) {
        uint64_t m = paraglob_match(pg, strlen(queries[i]), queries[i]);

        uint64_t fnm = 0;
        paraglob_stats(pg, &fnm);
        fnmatches += fnm;

        if ( debug )
            fprintf(stderr, "query: %s (%" PRIu64 " matches, %" PRIu64 " calls to fnmatch())\n", queries[i], m, fnm);

        if ( m ) {
            ++matches;
            matching_queries[i] = 1;
        }
    }

    struct timer_result t = timer_end();

    //

    fprintf(stderr, "time=%.2f mem=%ld matches=%" PRIu64 "/%.2f%% fnmatches=%" PRIu64 "/%.2f%% queries/sec=%.2f\n",
            t.time, t.mem, matches, (100.0 * matches / num_queries), fnmatches, 100.0 * fnmatches / (num_queries * num_patterns), num_queries / t.time);

    //

    if ( verify ) {
        int mismatches = 0;

        fprintf(stderr, "Verifying ...");

        timer_start();

        int brute_force_matches = 0;

        for ( j = 0; j < num_queries; j++ ) {

            int match = 0;

            for ( i = 0; i < num_patterns; i++ ) {
                if ( fnmatch(patterns[i], queries[j]) == 0 ) {
                    ++brute_force_matches;
                    match = 1;
                    break;
                }
            }

            if ( match && ! matching_queries[j] ) {
                if ( ! mismatches )
                    fprintf(stderr, "\n");

                fprintf(stderr, "   Mismatch for query |%s|\n", queries[j]);

                ++mismatches;
            }
        }

        t = timer_end();

        if ( matches == brute_force_matches )
            fprintf(stderr, " Ok\n");

        uint64_t fnmatches = 0;
        paraglob_stats(pg, &fnmatches);

        fprintf(stderr, "time=%.2f mem=%ld matches=%.2f%% queries/sec=%.2f (brute-force)\n",
                t.time, t.mem, (100.0 * brute_force_matches / num_queries), num_queries / t.time);
    }

    paraglob_delete(pg);
    return;
}

int main(int argc, char** argv)
{
    const char* bench_params = 0;
    int debug = 0;
    int verify = 0;

    char c;

    while ( (c = getopt(argc, argv, "dhvb:")) != -1 ) {
        switch ( c ) {
         case 'b':
            bench_params = optarg;
            break;

         case 'd':
            debug = 1;
            break;

         case 'v':
            verify = 1;
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

    if ( bench_params ) {
        if ( argc > 0 )
            usage(1);

        benchmark(bench_params, debug, verify);
        exit(0);
    }

    if ( argc < 1 )
        usage(1);

    text = argv[0];

    if ( debug )
        fprintf(stderr, "Text: |%s|\n", text);

    paraglob_t pg = paraglob_create(PARAGLOB_ASCII, match_callback);

    if ( ! pg )
        error("cannot allocate paraglob structure", 0);

    if ( debug )
        paraglob_enable_debug(pg, stderr);

    intptr_t cnt = 0;

    if ( argc > 1 ) {
        int i;
        for ( i = 1; i < argc; i++ ) {
            if ( ! paraglob_insert(pg, strlen(argv[i]), argv[i], (void*)++cnt) )
                error("pattern error", argv[i]);
        }
    }

    else {
        char buffer[1024];

        while ( 1 ) {
            if ( ! fgets(buffer, sizeof(buffer), stdin) ) {
                if ( feof(stdin) )
                    break;

                error("read error from standard input: ", strerror(errno));
            }

            if ( ! paraglob_insert(pg, strlen(buffer) - 1, buffer, (void*)++cnt) )
                error("pattern error", buffer);
        }
    }

    if ( debug )
        fprintf(stderr, "%" PRIiPTR " patterns\n", cnt);

    if ( ! paraglob_compile(pg) ) {
        if ( debug )
            paraglob_dump_debug(pg);

        error("compile error", paraglob_strerror(pg));
    }

    if ( debug ) {
        paraglob_dump_debug(pg);
        fprintf(stderr, "\n");
    }

    uint64_t matches = paraglob_match(pg, strlen(text), text);

    uint64_t fnmatches = 0;
    paraglob_stats(pg, &fnmatches);

    if ( debug )
        fprintf(stdout, "\n%" PRIu64 " matches found with %" PRIu64 " calls to fnmatch().\n", matches, fnmatches);

    paraglob_delete(pg);

    if ( matches > 0 )
        return 0;
    else
        return 1;
}
