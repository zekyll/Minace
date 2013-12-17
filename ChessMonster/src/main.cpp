
#include "App.h"

// Explicit instantiation for classes that have static initialization code.
namespace cm {
template class Scores_t<int>;
template class MoveMasks_t<Mask>;
template class Zobrist_t<uint64_t>;
}

int main(int argc, char** argv)
{
	cm::App app;
	app.run();

	return 0;
}
