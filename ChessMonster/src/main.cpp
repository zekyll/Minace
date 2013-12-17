
#include "MinMaxAI.h"
#include "TranspositionTable.h"
#include "GameState.h"
#include "MoveList.h"
#include "Scores.h"
#include "Evaluator.h"
#include "PerformanceTest.h"
#include "StdOutLogger.h"
#include "../tests/Test.h"
#include "../tests/EpdTest.h"
#include "../tests/GameStateTest.h"
#include "../tests/MinMaxAITest.h"
#include "../tests/EvaluatorTest.h"
#include "../tests/ScoresTest.h"
#include "../tests/MoveListTest.h"
#include "../tests/TreeGeneratorTest.h"
#include "../tests/UtilTest.h"
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
	EpdTest().run();
	GameStateTest().run();
	ScoresTest().run();
	MoveListTest().run();
	EvaluatorTest().run();
	MinMaxAITest().run();
	TreeGeneratorTest().run();
	UtilTest().run();

	StdOutLogger stdOutLogger;
	PerformanceTest pftest(stdOutLogger, 2, 5.0, true);
	pftest();

	return 0;
}
