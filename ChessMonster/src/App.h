#pragma once

#include "MinMaxAI.h"
#include "TranspositionTable.h"
#include "GameState.h"
#include "MoveList.h"
#include "Scores.h"
#include "Evaluator.h"
#include "PerformanceTest.h"
#include "SkillTest.h"
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
#include "../tests/ProcessTest.h"
#include <iostream>

namespace cm {

/**
 * Implementation of the command line interface.
 */
class App
{
private:

	StdOutLogger mStdOutLogger;

public:

	void run()
	{
		for (;;) {
			std::cout << std::endl << "Choose:" << std::endl;
			std::cout << "1. Unit tests" << std::endl;
			std::cout << "2. Performance test" << std::endl;
			std::cout << "3. Skill test: Easy positions" << std::endl;
			std::cout << "4. Skill test: Zugzwang positions" << std::endl;
			std::cout << "5. Exit" << std::endl;
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
				runSkillTest(SkillTest::EASY);
				break;
			case 4:
				runSkillTest(SkillTest::ZUGZWANG);
				break;
			case 5:
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
		PstreamTest().run();
	}

	void runPerformanceTest()
	{
		PerformanceTest pftest(mStdOutLogger, 2, 5.0, true);
		pftest();
	}

	void runSkillTest(const SkillTest& skillTest)
	{
		MinMaxAI ai;
		skillTest.run(ai, TimeConstraint(10), mStdOutLogger);
	}
};

}
