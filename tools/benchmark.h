// See the file "COPYING" in the main distribution directory for copyright.

#pragma once

#include <cstring>

/* A set of benchmark functions exercising paraglob with different workloads. */
double benchmark(char* a, char* b, char* c, bool silent);
double benchmark_n(long num_patterns, long num_queries, long match_prob, bool silent);

void makeGraphData();
