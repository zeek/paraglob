#ifndef BENCHMARK_H
#define BENCHMARK_H

#include "../src/paraglob.h"
#include <iostream>

double benchmark(char* a, char* b, char* c, bool silent);
double benchmark_n(long num_patterns, long num_queries, long match_prob, bool silent);
void makeGraphData();

#endif
