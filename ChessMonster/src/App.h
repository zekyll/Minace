#pragma once

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
#include <iostream>

namespace cm {

/**
 * Implementation of the command line interface.
 */
class App
{
private:

	StdOutLogger stdOutLogger;

public:

	void run()
	{
		for (;;) {
			std::cout << std::endl << "Choose:" << std::endl;
			std::cout << "1. Run unit tests" << std::endl;
			std::cout << "2. Run performance test" << std::endl;
			std::cout << "3. Exit" << std::endl;
			std::cout << "> ";

			int cmd;
			std::cin >> cmd;

			switch (cmd) {
			case 1:
				runUnitTests();
				break;
			case 2:
				runPerformanceTest();
				break;
			case 3:
				return;
			}
		}
	}

private:

	static void runUnitTests()
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
	}

	void runPerformanceTest()
	{
		StdOutLogger stdOutLogger;
		PerformanceTest pftest(stdOutLogger, 2, 5.0, true);
		pftest();
	}
};

}
