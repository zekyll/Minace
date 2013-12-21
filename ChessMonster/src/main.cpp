#include "App.h"
#include "Uci.h"
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
	bool testMode = false;
	std::unique_ptr<std::ostream> log(new std::stringstream);

	// Parse command line arguments.
	for (int i = 1; i < argc; ++i) {
		if (strcmp(argv[i], "-t") == 0)
			testMode = true;
		else if (strcmp(argv[i], "-l") == 0 && ++i < argc)
			log.reset(new std::ofstream(argv[i]));
	}

	if (testMode) {
		// Testing mode
		cm::App app;
		app.run();
	} else {
		// UCI mode
		cm::Uci uci(std::cin, std::cout, *log);
		uci.run();
	}

	return 0;
}
