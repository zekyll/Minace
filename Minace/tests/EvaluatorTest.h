#pragma once

#include "../src/MinMaxAI.h"
#include "../src/GameState.h"
#include "../ttest/ttest.h"
#include <memory>
#include <cstddef>

namespace mnc {

class EvaluatorTest : public ttest::TestBase
{
private:

	Evaluator e{2};
	GameState state{"Kf1 Nd2 b7", "Ke8 Qd8 Re4 Bf8 Ng8 g2 h2", Player::WHITE};
	std::unique_ptr<MinMaxAI> ai;

	TTEST_BEFORE()
	{
		e.reset(state);
	}

	TTEST_CASE("New state score is correct.")
	{
		TTEST_EQUAL(e.getScore(), -100000005 - 904 - 502 - 301 - 301 - 108 - 107
				+ 100000004 + 305 + 108);
	}

	TTEST_CASE("Multiple moves.")
	{
		int s = e.getScore();

		e.makeMove("Bc6xa4");
		int d1 = 1 - 4 + 105;
		TTEST_EQUAL(e.getScore(), -(s + d1));
		TTEST_EQUAL(e.getRelativeScore(), -d1);

		e.makeMove("b2xQc1B");
		int d2 = 301 - 108 + 903;
		TTEST_EQUAL(e.getScore(), s + d1 - d2);
		TTEST_EQUAL(e.getRelativeScore(), d1 - d2);

		e.undoMove();
		TTEST_EQUAL(e.getScore(), -(s + d1));
		TTEST_EQUAL(e.getRelativeScore(), -d1);

		e.undoMove();
		TTEST_EQUAL(e.getScore(), s);
		TTEST_EQUAL(e.getRelativeScore(), 0);
	}
};

}
