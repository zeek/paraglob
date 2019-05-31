// See the file "COPYING" in the main distribution directory for copyright.

#include "benchmark.h"

static const char* benchmark_pattern_words[] = {
    "aaaaaa", "bb", "cccccccccccccccc", "ddddd", "eeeeeeeee", "fffffffffffff", "gggg"
};

const char* random_pattern_word(){
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

double benchmark(char* a, char* b, char* c, bool silent) {
  srand(time(0));
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
  // Use a vector, otherwise will run into limit on call stack
  // for large number of queries
  std::vector<std::string> patterns;
  patterns.reserve(num_patterns);
  char buffer[1024];
  int i, j;

  for ( i = 0; i < num_patterns; i++ ) {

      buffer[0] = '\0';

      int rounds = (rand() % 10) + 2;
      for ( j = 0; j < rounds; j++ ) {

          if ( j != 0 )
              strcat(buffer, "*");

          if ( (rand() % 10) == 0 ) {
              strcat(buffer, random_pattern_word());
            }
          else {
              strcat(buffer, random_word());
            }
      }

      std::string s(strdup(buffer));
      patterns.push_back(s);
    }

  // Create the queries.
  // Use a vector, otherwise will run into limit on call stack
  // for large number of queries
  std::vector<std::string> queries;
  queries.reserve(num_queries);
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

      queries.push_back(std::string(strdup(buffer)));
  }

  if (!silent) {
    std::cout << "creating paraglob \n";
  }
  auto build_start = std::chrono::high_resolution_clock::now();
  paraglob::Paraglob myGlob;
  for (std::string p : patterns) {
    myGlob.add(p);
  }
  myGlob.compile();
  auto build_finish = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> build_time = build_finish - build_start;

  auto start = std::chrono::high_resolution_clock::now();

  if (!silent) {
    std::cout << "making queries \n";
  }
  for (std::string q : queries) {
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
