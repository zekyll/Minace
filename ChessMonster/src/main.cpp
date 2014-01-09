#include "App.h"
#include "Uci.h"
#include "Tournament.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <memory>

// Explicit instantiation for classes that have static initialization code.
namespace cm {
template class Scores_t<int>;
template class MoveMasks_t<Mask>;
template class Zobrist_t<uint64_t>;
}

int main(int argc, char** argv)
{
	int mode = 0;
	std::unique_ptr<std::ostream> log(new std::stringstream);
	std::string tournamentFile;

	// Parse command line arguments.
	for (int i = 1; i < argc; ++i) {
		if (strcmp(argv[i], "-t") == 0)
			mode = 1;
		else if (strcmp(argv[i], "-l") == 0 && ++i < argc)
			log.reset(new std::ofstream(argv[i]));
		if (strcmp(argv[i], "-o") == 0 && ++i < argc) {
			mode = 2;
			tournamentFile = argv[i];
		}
	}

	if (mode == 0) {
		// UCI mode
		cm::Uci uci(std::cin, std::cout, *log);
		uci.run();
	} else if (mode == 1) {
		// Testing mode
		cm::App app;
		app.run();
	} else if (mode == 2) {
#ifdef __unix__
		// Tournament
		cm::Tournament tournament(tournamentFile, std::cout);
		tournament.run();
#endif
	}

	return 0;
}
