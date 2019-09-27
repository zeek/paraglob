// See the file "COPYING" in the main distribution directory for copyright.

/*
A simple driver for testing paraglob's performance and functionality.

Supports the following arguments:
	-b <a> <b> <c> <time>	-> Benchmark paraglob.  See below.
	-n <text> <patterns>	-> Print the number of matching patterns in the text.

Benchmarking:
	a	-> number of patterns to generate
	b -> number of queries to perform
	c -> percentage of generated patterns that match queries

	<time> -> optional. If provided, rather than printing detailed results,
						prints 1 or 0 corresponding to rather or not the benchmark was
						completed in under <time> seconds

Note that this script is just for testing and as such if you give it bad
arguments it will ungracefully break.
*/

#include <iostream>
#include <cstring>
#include <vector>
#include <fstream>

#include "paraglob/paraglob.h"
#include "benchmark.h"

int main(int argc, char* argv[]) {
	double max_time = 0;

	if (argc <= 1) {
		std::cerr << "usage: " << argv[0] << " -n <text> <patterns>\n";
		std::cerr << "       " <<
			"Prints the number of patterns that match the text.\n";
		std::cerr << "       " << argv[0] << " -b <a> <b> <c> <time>\n";
		std::cerr << "       " <<
			"Benchmark. a - n patterns. b - n queries. c - % matches.\n";
		std::cerr << "       " << argv[0] << " -s <patterns>\n";
		std::cerr << "       " <<
			"Prints a a paraglob with **patterns** serialization\n";
		exit(1);
	}

	if (strcmp(argv[1], "-b") == 0) {
		if (argc == 6) {
			// there is a time argument
			max_time = atof(argv[5]);
		}

		char* a = argv[2];
		char* b = argv[3];
		char* c = argv[4];
		double elapsed = benchmark(a, b, c, max_time);

		if (max_time > 0) {
			if (elapsed <= max_time) {
				std::cout << "Passed.\n";
			} else {
				std::cout << "Failed.\n";
			}
		}
	} else if (strcmp(argv[1], "-n") == 0) {
		std::vector<std::string> v;
		for (int i = 3 ; i < argc ; i++) {
			v.push_back(std::string(argv[i]));
		}
		paraglob::Paraglob p(v);
		std::cout << p.get(std::string(argv[2])).size() << "\n";
		std::cout << p.str();
	} else if (strcmp(argv[1], "-s") == 0) {
		std::vector<std::string> v;
		for (int i = 3 ; i < argc ; i++) {
			v.push_back(std::string(argv[i]));
		}
		try
			{
			paraglob::Paraglob p(v);
			paraglob::Paraglob sp(p.serialize());
			if (p == sp)
				std::cout << "passed" << std::endl;
			else
				std::cout << "failed" << std::endl;
			}
		catch(const paraglob::overflow_error &e)
			{
			std::cerr << e.what() << '\n';
			}
	}
	else if (strcmp(argv[1], "-str") == 0) {
		std::vector<std::string> v;
		for (int i = 3 ; i < argc ; i++) {
			v.push_back(std::string(argv[i]));
		}
		paraglob::Paraglob p(v);
		std::cout << p.str();
	}
	else {
		std::cout << "Unrecognized first param\n";
	}
}
