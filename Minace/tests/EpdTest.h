#pragma once

#include "../src/BitBoard.h"
#include "../src/Epd.h"
#include "../ttest/ttest.h"

namespace mnc {

class EpdTest : public ttest::TestBase
{
private:

	TTEST_CASE("Reads an empty board correctly.")
	{
		Epd epd("8/8/8/8/8/8/8/8 w - -");
		TTEST_EQUAL(epd.board(), BitBoard());
	}

	TTEST_CASE("Reads a complex board correctly.")
	{
		Epd epd("1KQ2R2/Pk1q3p/3NNB2/8/8/8/8/4rbbn w - -");
		BitBoard bb;
		bb.addPiece(Player::WHITE, Piece::KING,{0, 1});
		bb.addPiece(Player::WHITE, Piece::QUEEN,{0, 2});
		bb.addPiece(Player::WHITE, Piece::ROOK,{0, 5});
		bb.addPiece(Player::WHITE, Piece::PAWN,{1, 0});
		bb.addPiece(Player::BLACK, Piece::KING,{1, 1});
		bb.addPiece(Player::BLACK, Piece::QUEEN,{1, 3});
		bb.addPiece(Player::BLACK, Piece::PAWN,{1, 7});
		bb.addPiece(Player::WHITE, Piece::KNIGHT,{2, 3});
		bb.addPiece(Player::WHITE, Piece::KNIGHT,{2, 4});
		bb.addPiece(Player::WHITE, Piece::BISHOP,{2, 5});
		bb.addPiece(Player::BLACK, Piece::ROOK,{7, 4});
		bb.addPiece(Player::BLACK, Piece::BISHOP,{7, 5});
		bb.addPiece(Player::BLACK, Piece::BISHOP,{7, 6});
		bb.addPiece(Player::BLACK, Piece::KNIGHT,{7, 7});
		TTEST_EQUAL(epd.board(), bb);
	}

	TTEST_CASE("Reads starting player correctly.")
	{
		TTEST_EQUAL(Epd("8/8/8/8/8/8/8/3K4 w - -").startingPlayer(), Player::WHITE);
		TTEST_EQUAL(Epd("8/8/8/8/8/8/8/3K4 b - -").startingPlayer(), Player::BLACK);
	}

	TTEST_CASE("Reads castling rights correctly.")
	{
		TTEST_EQUAL(Epd("8/8/8/8/8/8/8/3K4 w - -").castlingRights(), Mask());
		TTEST_EQUAL(Epd("8/8/8/8/8/8/8/3K4 w KQkq -").castlingRights(), Mask({0, 7, 56, 63}));
		TTEST_EQUAL(Epd("8/8/8/8/8/8/8/3K4 w Qk -").castlingRights(), Mask({7, 56}));
		TTEST_EQUAL(Epd("8/8/8/8/8/8/8/3K4 w Kq -").castlingRights(), Mask({0, 63}));
	}

	TTEST_CASE("Reads en passant square correctly.")
	{
		TTEST_EQUAL(Epd("8/8/8/8/1P6/8/8/3K4 b - b3").enpassantSqr(), Sqr("b3"));
		TTEST_EQUAL(Epd("8/8/8/7p/8/8/8/3K4 w KQkq h6").enpassantSqr(), Sqr("h6"));
	}

	TTEST_CASE("Handles a FEN string with move counts.")
	{
		TTEST_EQUAL(Epd("8/8/8/8/1P6/8/8/3K4 b - b3 7 22").halfMoveClock(), 7);
		TTEST_EQUAL(Epd("8/8/8/8/1P6/8/8/3K4 b - b3 7 22").fullMoveNumber(), 22);
	}
};

}
