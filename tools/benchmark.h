// See the file "COPYING" in the main distribution directory for copyright.

#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <iostream>
#include <cstring>
#include <chrono>

#include "paraglob/paraglob.h"

/* A set of benchmark functions exercising paraglob with different workloads. */
double benchmark(char* a, char* b, char* c, bool silent);
double benchmark_n(long num_patterns, long num_queries, long match_prob, bool silent);

void makeGraphData();

#endif
