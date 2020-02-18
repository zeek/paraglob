// See the file "COPYING" in the main distribution directory for copyright.

#include "benchmark.h"
#include <random>
#include <memory>

std::random_device dev;
std::mt19937 rng(dev());
std::uniform_int_distribution<std::mt19937::result_type> dist(0,RAND_MAX);

int rand_int()
  {
  return dist(rng);
  }

static const char* benchmark_pattern_words[] = {
    "aaaaaa", "bb", "cccccccccccccccc", "ddddd", "eeeeeeeee", "fffffffffffff", "gggg"
};

const char* random_pattern_word(){
    int idx = rand_int() % (sizeof(benchmark_pattern_words) / sizeof(const char*));
    return benchmark_pattern_words[idx];
}

const char* random_word()
{
    static char buffer[1024];

    int j;
    int rounds = (rand_int() % 25) + 5;
    for ( j = 0; j < rounds; j++ ) {
        buffer[j] = (char)((rand_int() % 26) + 'a');
    }

    buffer[rounds] = '\0';

    return buffer;
}

double benchmark(char* a, char* b, char* c, bool silent) {
  long num_patterns = atol(a);
  long num_queries = atol(b);
  long match_prob = atol(c);
  return benchmark_n(num_patterns, num_queries, match_prob, silent);
}

double benchmark_n(long num_patterns, long num_queries, long match_prob, bool silent) {
  if (!silent) {
    std::cout << "creating workload:\n";
    std::cout << "\t# patterns: " << num_patterns << "\n";
    std::cout << "\t# queries: " << num_queries << "\n";
    std::cout << "\t% matches: " << match_prob << "\n";
  }

  // Create the patterns.
  std::unique_ptr<std::string[]> patterns(new std::string[num_patterns]);
  char buffer[1024];
  int i, j;

  for ( i = 0; i < num_patterns; i++ ) {

      buffer[0] = '\0';

      int rounds = (rand_int() % 10) + 2;
      for ( j = 0; j < rounds; j++ ) {

          if ( j != 0 )
              strcat(buffer, "*");

          if ( (rand_int() % 10) == 0 ) {
              strcat(buffer, random_pattern_word());
            }
          else {
              strcat(buffer, random_word());
            }
      }

      std::string s(buffer);
      patterns[i] = s;
    }

    // Create the queries.

  std::unique_ptr<std::string[]> queries(new std::string[num_queries]);

  for ( i = 0; i < num_queries; i++ ) {

      buffer[0] = '\0';

      if ( (rand_int() % 100) <= match_prob ) {
          // Create a likely match candidate.
          int rounds = (rand_int() % 5) + 1;
          for ( j = 0; j < rounds; j++ ) {
              strcat(buffer, random_pattern_word());
          }
      }

      else {
          // Create a mismatch.
          int rounds = (rand_int() % 50) + 5;
          for ( j = 0; j < rounds; j++ ) {
              buffer[j] = (char)((rand_int() % 26) + 'a');
          }

          buffer[rounds] = '\0';
      }

      queries[i] = std::string(strdup(buffer));
  }

  if (!silent) {
    std::cout << "creating paraglob \n";
  }
  auto build_start = std::chrono::high_resolution_clock::now();
  paraglob::Paraglob myGlob;
  for ( i = 0; i < num_patterns; ++i ) {
    const auto& p = patterns[i];
    myGlob.add(p);
  }
  myGlob.compile();
  auto build_finish = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> build_time = build_finish - build_start;

  auto start = std::chrono::high_resolution_clock::now();

  if (!silent) {
    std::cout << "making queries \n";
  }
  for ( i = 0; i < num_queries; ++i ) {
    const auto& q = queries[i];
    myGlob.get(q);
  }

  auto finish = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed = finish - start;

  if (!silent) {
    std::cout << "Build time: " << build_time.count() << "s\n";
    std::cout << "Search time: " << elapsed.count() << " s\n";
    std::cout << "Queries/second: " << num_queries/elapsed.count() << "\n";
  }

  return elapsed.count() + build_time.count();
}

void makeGraphData() {
  /*
  prints data to the console for generation of 3d plot
  of paraglob performance.
  x axis is number of patterns
  y axis is number of queries
  z axis is the time taken to build and run the queries
  */
  for(long patterns = 500; patterns <= 10000; patterns += 500) {
    std::cout << "{ ";
    for(long queries = 1000; queries <= 20000; queries += 1000) {
      std::cout << benchmark_n(patterns, queries, 10, true);
      if (queries != 20000) {
        std::cout << ", ";
      }
    }
    std::cout << "},\n";
  }
}
