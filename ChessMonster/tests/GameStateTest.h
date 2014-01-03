#pragma once

#include "../src/MinMaxAI.h"
#include "../src/GameState.h"
#include "../ttest/ttest.h"
#include <memory>
#include <cstdint>

namespace cm {

class GameStateTest : public ttest::TestBase
{
private:

	TTEST_CASE("Reads board contents from EPD.")
	{
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

		GameState s("1KQ2R2/Pk1q3p/3NNB2/8/8/8/8/4rbbn w - -");
		TTEST_EQUAL(s.board(), bb);
		TTEST_EQUAL(s, GameState(bb, Player::WHITE));
	}

	TTEST_CASE("Reads starting player from EPD.")
	{
		BitBoard bb;
		bb.addPiece(Player::WHITE, Piece::KING,{7, 3});

		GameState s1("8/8/8/8/8/8/8/3K4 w - -");
		TTEST_EQUAL(s1.activePlayer(), Player::WHITE);
		TTEST_EQUAL(s1, GameState(bb, Player::WHITE));

		GameState s2("8/8/8/8/8/8/8/3K4 b - -");
		TTEST_EQUAL(s2.activePlayer(), Player::BLACK);
		TTEST_EQUAL(s2, GameState(bb, Player::BLACK));
	}

	TTEST_CASE("Reads castling rights from EPD.")
	{
		std::string epd = "r3k2r/8/8/8/8/8/8/R3K2R w - -";
		GameState s(epd);
		TTEST_EQUAL(s.castlingRights(), Mask());
		TTEST_EQUAL(s, GameState(BitBoard(epd), Player::WHITE));

		epd = "r3k2r/8/8/8/8/8/8/R3K2R w KQkq -";
		GameState s2(epd);
		TTEST_EQUAL(s2.castlingRights(), Mask({0, 7, 56, 63}));
		TTEST_EQUAL(s2, GameState(BitBoard(epd), Player::WHITE, ~Mask()));

		epd = "r3k2r/8/8/8/8/8/8/R3K2R b Qk -";
		GameState s3(epd);
		TTEST_EQUAL(s3.castlingRights(), Mask({7, 56}));
		TTEST_EQUAL(s3, GameState(BitBoard(epd), Player::BLACK, Mask({7, 56})));
	}

	TTEST_CASE("Reads en passant square from EPD.")
	{
		std::string epd = "8/8/8/8/8/8/8/8 w - -";
		GameState s(epd);
		TTEST_EQUAL(s.enPassantSqr(), Sqr::NONE);
		TTEST_EQUAL(s, GameState(BitBoard(epd), Player::WHITE));

		epd = "8/8/8/1p6/8/8/8/8 w - b6";
		GameState s2(epd);
		TTEST_EQUAL(s2.enPassantSqr(), Sqr("b6"));
		TTEST_EQUAL(s2, GameState(BitBoard(epd), Player::WHITE, Mask(), Sqr("b6")));

		epd = "8/8/8/8/7P/8/8/8 b - h3";
		GameState s3(epd);
		TTEST_EQUAL(s3.enPassantSqr(), Sqr("h3"));
		TTEST_EQUAL(s3, GameState(BitBoard(epd), Player::BLACK, Mask(), Sqr("h3")));
	}

	TTEST_CASE("Constructor throws if en passant square on wrong row.")
	{
		try {
			GameState("8/8/8/8/8/7P/8/8 b - h2");
			TTEST_EQUAL(true, false);
		} catch (std::invalid_argument& e) {
		}
	}

	TTEST_CASE("Constructor throws if invalid captured piece for en passant.")
	{
		try {
			GameState("8/8/8/8/7R/8/8/8 b - h3");
			TTEST_EQUAL(false, true);
		} catch (std::invalid_argument& e) {
		}
		try {
			GameState("8/8/8/8/7p/8/8/8 b - h3");
			TTEST_EQUAL(false, true);
		} catch (std::invalid_argument& e) {
		}
		try {
			GameState("8/8/8/8/8/8/8/8 b - h3");
			TTEST_EQUAL(false, true);
		} catch (std::invalid_argument& e) {
		}
	}

	TTEST_CASE("Reads half move clock from FEN.")
	{
		std::string epd = "8/8/8/8/8/8/8/8 w - - 7 22";
		GameState s(epd);
		TTEST_EQUAL(s.halfMoveClock(), 7);
		TTEST_EQUAL(s, GameState(BitBoard(epd), Player::WHITE, Mask(), Sqr::NONE, 7));
	}

	TTEST_CASE("MakeMove() changes board layout.")
	{
		GameState s("Kc3 Nd2", "Kf5 d7", Player::WHITE);
		s.makeMove("Kc3-c4");
		TTEST_EQUAL(s.board(), BitBoard("Kc4 Nd2", "Kf5 d7"));
	}

	TTEST_CASE("MakeMove() handles en passant correctly.")
	{
		GameState s("b2 Kh1", "c4", Player::WHITE);
		s.makeMove("b2-b4");
		TTEST_EQUAL(s.enPassantSqr(), Sqr("b3"));
		s.makeMove("c4xb3");
		TTEST_EQUAL(s.board(), BitBoard("Kh1", "b3"));
		TTEST_EQUAL(s.enPassantSqr(), Sqr::NONE);
	}

	TTEST_CASE("MakeMove() updates rook position when castling.")
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

	TTEST_CASE("MakeMove updates castling rights.")
	{
		GameState s("Ra1 Ke1 Rh1", "Ra8 Ke8 Rh8", Player::WHITE, ~Mask());
		s.makeMove("Ke1-e2");
		TTEST_EQUAL(s.castlingRights().toStr(), "a8 h8");
		s.makeMove("Ra8-b8");
		TTEST_EQUAL(s.castlingRights().toStr(), "h8");
	}

	TTEST_CASE("MakeMove changes player.")
	{
		GameState s("Kc3 Nd2", "Kf5 d7", Player::WHITE);
		s.makeMove("Kc3-c4");
		TTEST_EQUAL(s.activePlayer(), Player::BLACK);
	}

	TTEST_CASE("IsCheckMate() returns true when check mate.")
	{
		GameState s("Kb8", "Kb6 Rd8", Player::WHITE);
		TTEST_EQUAL(s.isCheckMate(), true);
	}

	TTEST_CASE("IsCheckMate() returns false if king can escape.")
	{
		GameState s("Ke5", "Kb6 Rc5", Player::WHITE);
		TTEST_EQUAL(s.isCheckMate(), false);
	}

	TTEST_CASE("IsCheckMate() returns false if king can capture aggressor.")
	{
		GameState s("Ka8", "Kb6 Rb8", Player::WHITE);
		TTEST_EQUAL(s.isCheckMate(), false);
	}

	TTEST_CASE("IsCheckMate() returns false when stale mate.")
	{
		GameState s("Kb8", "Kb6 Qc6", Player::WHITE);
		TTEST_EQUAL(s.isCheckMate(), false);
	}

	TTEST_CASE("IsStaleMate() returns true when stale mate.")
	{
		GameState s("Kb8", "Kb6 Qc6", Player::WHITE);
		TTEST_EQUAL(s.isStaleMate(), true);
	}

	TTEST_CASE("IsStaleMate() returns false when check mate.")
	{
		GameState s("Kb8", "Kb6 Rd8", Player::WHITE);
		TTEST_EQUAL(s.isStaleMate(), false);
	}

	TTEST_CASE("IsStaleMate() returns false if king can escape.")
	{
		GameState s("Ke5", "Kb6 Rc5", Player::WHITE);
		TTEST_EQUAL(s.isStaleMate(), false);
	}

	TTEST_CASE("IsStaleMate() returns false if king can capture aggressor.")
	{
		GameState s("Ka8", "Kb6 Rb8", Player::WHITE);
		TTEST_EQUAL(s.isStaleMate(), false);
	}

	TTEST_CASE("IsStaleMate() returns true when repeated position.")
	{
		GameState s("Ka1 Qb1", "Kg7 Qh8", Player::WHITE);
		s.makeMove("Qb1-c1");
		s.makeMove("Qh8-g8");
		s.makeMove("Qc1-b1");
		s.makeMove("Qg8-h8");
		TTEST_EQUAL(s.isStaleMate(), true);
	}

	TTEST_CASE("Position is not repeated if en passant square is different.")
	{
		GameState s("Ka1 Qb1 e2", "Kg7 Qh8 d4", Player::WHITE);
		s.makeMove("e2-e4");
		s.makeMove("Qh8-g8");
		s.makeMove("Qb1-c1");
		s.makeMove("Qg8-h8");
		s.makeMove("Qc1-b1");
		TTEST_EQUAL(s.isStaleMate(), false);
	}

	TTEST_CASE("Position is not repeated if castling rights are different.")
	{
		GameState s("Ke1 Ra1", "Ke8", Player::WHITE, ~Mask());
		s.makeMove("Ra1-a2");
		s.makeMove("Ke8-e7");
		s.makeMove("Ra2-a1");
		s.makeMove("Ke7-e8");
		TTEST_EQUAL(s.isStaleMate(), false);
	}

	TTEST_CASE("IsStaleMate() returns true when half move clock reaches 50.")
	{
		GameState s("Ke1 Qa1", "Ke8 Qa8", Player::WHITE);
		std::vector<std::string> moveCycles[]{
			{"Qa1-a2", "Qa2-a3", "Qa3-b3", "Qb3-c3", "Qc3-c2", "Qc2-b1", "Qb1-a1"}, // 7
			{"Qa8-a7", "Qa7-a6", "Qa6-b6", "Qb6-c6", "Qc6-c7", "Qc7-c8", "Qc8-b8", "Qb8-a8"} // 8
		};
		// 7 * 8 >= 50
		for (int i = 0; i < 50; ++i) {
			TTEST_EQUAL(s.isStaleMate(), false);
			s.makeMove(moveCycles[i % 2][i / 2 % moveCycles[i % 2].size()]);
		}
		TTEST_EQUAL(s.isStaleMate(), true);
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
			//TTEST_EQUAL(perft(s, 4), 197281ull);
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
				"Ke8 Qa3 Ra8 Rh8 Bb6 Bg6 Na5 Nf6 b7 b2 c7 d7 f7 g7 h7", Player::WHITE, ~Mask());
		GameState s(s0);
		for (int i = 0; i < 2; ++i) {
			TTEST_EQUAL(perft(s, 1), 6ull);
			TTEST_EQUAL(perft(s, 2), 264ull);
			TTEST_EQUAL(perft(s, 3), 9467ull);
			//TTEST_EQUAL(perft(s, 4), 422333ull);
			//TTEST_EQUAL(perft(s, 5), 15833292ull);
			TTEST_EQUAL(s, s0);
			s = s0 = GameState("Ke1 Qa6 Ra1 Rh1 Bb3 Bg3 Na4 Nf3 b2 b7 c2 d2 f2 g2 h2",
					"Kg8 Qd8 Ra8 Rf8 Ba5 Bb5 Nf6 Nh3 a7 a2 b4 c5 d7 e5 g7 h7",
					Player::BLACK, ~Mask());
		}
	}

	TTEST_CASE("Output to FEN string (starting position).")
	{
		GameState s;
		TTEST_EQUAL(s.toStr(), "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
	}

	TTEST_CASE("Output to FEN string (black to move, no castling rights).")
	{
		GameState s("8/8/8/8/8/8/8/8 b - - 0 1");
		TTEST_EQUAL(s.toStr(), "8/8/8/8/8/8/8/8 b - - 0 1");
	}

	TTEST_CASE("Output to FEN string (partial castling rights).")
	{
		GameState s("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w Kq - 0 1");
		TTEST_EQUAL(s.toStr(), "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w Kq - 0 1");
		GameState s2("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w Qk - 0 1");
		TTEST_EQUAL(s2.toStr(), "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w Qk - 0 1");
	}

	TTEST_CASE("Output to FEN string (en passant square, half move clock).")
	{
		GameState s("8/8/8/1p6/8/8/8/8 w - b6 4 1");
		TTEST_EQUAL(s.toStr(), "8/8/8/1p6/8/8/8/8 w - b6 4 1");
	}

	TTEST_CASE("Output to FEN string (full move number incremented every second move).")
	{
		GameState s;
		s.makeMove(Move("e2-e4"));
		TTEST_EQUAL(s.toStr(), "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1");
		s.makeMove(Move("c7-c5"));
		TTEST_EQUAL(s.toStr(), "rnbqkbnr/pp1ppppp/8/2p5/4P3/8/PPPP1PPP/RNBQKBNR w KQkq c6 0 2");
	}
};

}
