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

	static const int MATE = Scores::PIECE_VALUES[Piece::KING];

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

	TTEST_CASE("getCheckMateScore()")
	{
		TTEST_EQUAL(Scores::getCheckMateScore(1), MATE - 1);
		TTEST_EQUAL(Scores::getCheckMateScore(-3), -MATE + 3);
	}

	TTEST_CASE("Score in floating point form has two decimals.")
	{
		TTEST_EQUAL(Scores::toStr(1510), "15.10");
		TTEST_EQUAL(Scores::toStr(-12), "-0.12");
	}

	TTEST_CASE("Mate score has correct string.")
	{
		TTEST_EQUAL(Scores::toStr(MATE - 1), "mate 1");
		TTEST_EQUAL(Scores::toStr(MATE - 5), "mate 5");
		TTEST_EQUAL(Scores::toStr(-MATE), "mate 0");
		TTEST_EQUAL(Scores::toStr(-MATE + 3), "mate -3");
	}
};

}
