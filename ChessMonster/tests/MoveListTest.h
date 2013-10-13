#pragma once

#include "../src/MinMaxAI.h"
#include "../src/MoveList.h"
#include "../ttest/ttest.h"
#include <memory>
#include <cstddef>

namespace cm {

class MoveListTest : public ttest::TestBase
{
private:

	MoveList list;

	TTEST_BEFORE()
	{
		GameState state("Kh7 Nh5 Qf4 b7 c5", "Rc8 Kg7 h6 Qb6 Re3 d7", Player::BLACK);
		state.makeMove("d7-d5"); // Create possibility for en passant.
		list.populate(state, false);
	}

	TTEST_CASE("Move count is correct.")
	{
		TTEST_EQUAL(list.getCount(0), 2U);
		TTEST_EQUAL(list.getCount(1), 1U);
		TTEST_EQUAL(list.getCount(2), 4U);
		TTEST_EQUAL(list.getCount(10), 27U); // Kh7:3 Nh5:2 Qf4:21 b7:0 c5:1
		TTEST_EQUAL(list.getCount(11), 3U);
	}
};

}
