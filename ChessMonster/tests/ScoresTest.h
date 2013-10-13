#pragma once

#include "../src/MinMaxAI.h"
#include "../src/GameState.h"
#include "../ttest/ttest.h"
#include <memory>
#include <cstddef>

namespace cm {

class ScoresTest : public ttest::TestBase
{
private:

	TTEST_CASE("Positional values for players are mirrored.")
	{
		TTEST_EQUAL(Scores::POSITIONAL_PIECE_VALUES[Player::BLACK][Piece::PAWN][6 * 8 + 3],
				Scores::POSITIONAL_PIECE_VALUES[Player::WHITE][Piece::PAWN][1 * 8 + 3]);
		TTEST_EQUAL(Scores::POSITIONAL_PIECE_VALUES[Player::BLACK][Piece::BISHOP][5 * 8 + 6],
				Scores::POSITIONAL_PIECE_VALUES[Player::WHITE][Piece::BISHOP][2 * 8 + 6]);
	}

	TTEST_CASE("Positional value is increased by piece value.")
	{
		TTEST_EQUAL(Scores::POSITIONAL_PIECE_VALUES[Player::BLACK][Piece::KING][0 * 8 + 4],
				100000005);
	}
};

}
