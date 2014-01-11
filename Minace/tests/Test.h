#pragma once

#include "../src/MinMaxAI.h"
#include "../src/GameState.h"
#include "../ttest/ttest.h"

namespace mnc {

class Test : public ttest::TestBase
{
private:

	TTEST_CASE("Piece::Piece(const string&)")
	{
		TTEST_EQUAL(Piece("K"), Piece::KING);
		TTEST_EQUAL(Piece("N"), Piece::KNIGHT);
		TTEST_EQUAL(Piece(""), Piece::PAWN);
	}

	TTEST_CASE("Piece::toStr()")
	{
		TTEST_EQUAL(Piece::KING.toStr(), "K");
		TTEST_EQUAL(Piece::BISHOP.toStr(), "B");
		TTEST_EQUAL(Piece::PAWN.toStr(), "");
		TTEST_EQUAL(Piece::NONE.toStr(), "");
		TTEST_EQUAL(Piece::KING.toStr(Player::BLACK), "k");
		TTEST_EQUAL(Piece::ROOK.toStr(Player::BLACK), "r");
		TTEST_EQUAL(Piece::PAWN.toStr(Player::BLACK), "");
		TTEST_EQUAL(Piece::NONE.toStr(Player::BLACK), "");
		TTEST_EQUAL(Piece::KING.toStr(Player::WHITE, true), "K");
		TTEST_EQUAL(Piece::KNIGHT.toStr(Player::WHITE, true), "N");
		TTEST_EQUAL(Piece::PAWN.toStr(Player::WHITE, true), "P");
		TTEST_EQUAL(Piece::NONE.toStr(Player::WHITE, true), ".");
		TTEST_EQUAL(Piece::KING.toStr(Player::BLACK, true), "k");
		TTEST_EQUAL(Piece::QUEEN.toStr(Player::BLACK, true), "q");
		TTEST_EQUAL(Piece::PAWN.toStr(Player::BLACK, true), "p");
		TTEST_EQUAL(Piece::NONE.toStr(Player::BLACK, true), ".");
	}
};

}
