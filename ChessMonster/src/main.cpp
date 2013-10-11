
#include "MinMaxAI.h"
#include "TranspositionTable.h"
#include "GameState.h"
#include "MoveList.h"
#include "Scores.h"
#include "Evaluator.h"
#include "../tests/Test.h"
#include "../tests/TreeGeneratorTest.h"
#include <cstdlib>
#include <iostream>

using namespace std;
using namespace cm;

template class Scores_t<int>;
template class MoveMasks_t<Mask>;
template class Zobrist_t<uint64_t>;

int main(int argc, char** argv)
{

	Test().run();
	TreeGeneratorTest().run();

	return 0;
}
