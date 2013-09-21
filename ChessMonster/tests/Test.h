#pragma once

#include "../src/MinMaxAI.h"
#include "../src/GameState.h"
#include "../ttest/ttest.h"
#include <memory>

namespace cm {

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

	TTEST_CASE("GameState::makeMove changes board layout.")
	{
		GameState s = GameState("Kc3 Nd2", "Kf5 d7", Player::WHITE);
		s.makeMove("Kc3-c4");
		TTEST_EQUAL(s.board(), BitBoard("Kc4 Nd2", "Kf5 d7"));
	}

	TTEST_CASE("GameState::makeMove handles en passant correctly.")
	{
		GameState s = GameState("b2 Kh1", "c4", Player::WHITE);
		s.makeMove("b2-b4");
		TTEST_EQUAL(s.enPassantSqr(), Sqr("b3"));
		s.makeMove("c4xb3");
		TTEST_EQUAL(s.board(), BitBoard("Kh1", "b3"));
		TTEST_EQUAL(s.enPassantSqr(), Sqr::NONE);
	}

	TTEST_CASE("GameState::makeMove updates rook position when castling.")
	{
		GameState s("Ra1 Ke1 Rh1", "Ra8 Ke8 Rh8", Player::WHITE);
		s.makeMove("Ke1-c1");
		TTEST_EQUAL(s.board(), BitBoard("Kc1 Rd1 Rh1", "Ra8 Ke8 Rh8"));
		s.makeMove("Ke8-c8");
		TTEST_EQUAL(s.board(), BitBoard("Kc1 Rd1 Rh1", "Kc8 Rd8 Rh8"));

		s = GameState("Ra1 Ke1 Rh1", "Ra8 Ke8 Rh8", Player::WHITE);
		s.makeMove("Ke1-g1");
		TTEST_EQUAL(s.board(), BitBoard("Ra1 Rf1 Kg1", "Ra8 Ke8 Rh8"));
		s.makeMove("Ke8-g8");
		TTEST_EQUAL(s.board(), BitBoard("Ra1 Rf1 Kg1", "Ra8 Rf8 Kg8"));
	}

	TTEST_CASE("GameState::makeMove updates castling rights.")
	{
		GameState s("Ra1 Ke1 Rh1", "Ra8 Ke8 Rh8", Player::WHITE);
		s.makeMove("Ke1-e2");
		TTEST_EQUAL(s.castlingRights().toStr(), "a8 h8");
		s.makeMove("Ra8-b8");
		TTEST_EQUAL(s.castlingRights().toStr(), "h8");
	}

	TTEST_CASE("GameState::makeMove changes player.")
	{
		GameState s("Kc3 Nd2", "Kf5 d7", Player::WHITE);
		s.makeMove("Kc3-c4");
		TTEST_EQUAL(s.activePlayer(), Player::BLACK);
	}

	std::vector<Move> moves[32];

	uint64_t perft(GameState& state, unsigned depth)
	{
		moves[depth].clear();
		state.getLegalMoves(moves[depth]);
		if (depth == 1)
			return moves[depth].size();
		uint64_t count = 0;
		for (Move m : moves[depth]) {
			state.makeMove(m);
			count += perft(state, depth - 1);
			state.undoMove(m);
		}
		return count;
	}

	TTEST_CASE("Perft for initial position (depth 1-4).")
	{
		GameState s;
		for (int i = 0; i < 2; ++i) {
			TTEST_EQUAL(perft(s, 1), 20ull);
			TTEST_EQUAL(perft(s, 2), 400ull);
			TTEST_EQUAL(perft(s, 3), 8902ull);
			TTEST_EQUAL(perft(s, 4), 197281ull);
			//TTEST_EQUAL(perft(s, 5), 4865609ull);
			//TTEST_EQUAL(perft(s, 6), 119060324ull);
			//TTEST_EQUAL(perft(s, 7), 3195901860ull);
			//TTEST_EQUAL(perft(s, 8), 84998978956ull);
			s.makeNullMove();
		}
		TTEST_EQUAL(s, GameState());
	}

	TTEST_CASE("Perft in mid game position (depth 1-3).")
	{
		// http://chessprogramming.wikispaces.com/Perft+Results (Position 4)
		GameState s0("Kg1 Qd1 Ra1 Rf1 Ba4 Bb4 Nf3 Nh6 a2 a7 b5 c4 d2 e4 g2 h2",
				"Ke8 Qa3 Ra8 Rh8 Bb6 Bg6 Na5 Nf6 b7 b2 c7 d7 f7 g7 h7", Player::WHITE);
		GameState s(s0);
		for (int i = 0; i < 2; ++i) {
			TTEST_EQUAL(perft(s, 1), 6ull);
			TTEST_EQUAL(perft(s, 2), 264ull);
			TTEST_EQUAL(perft(s, 3), 9467ull);
			//TTEST_EQUAL(perft(s, 4), 422333ull);
			//TTEST_EQUAL(perft(s, 5), 15833292ull);
			TTEST_EQUAL(s, s0);
			s = s0 = GameState("Ke1 Qa6 Ra1 Rh1 Bb3 Bg3 Na4 Nf3 b2 b7 c2 d2 f2 g2 h2",
					"Kg8 Qd8 Ra8 Rf8 Ba5 Bb5 Nf6 Nh3 a7 a2 b4 c5 d7 e5 g7 h7", Player::BLACK);
		}

	}
};

}
