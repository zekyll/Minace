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

	MoveList moves[32];

	uint64_t perft(GameState& state, unsigned depth)
	{
		moves[depth].populate(state, false);
		if (depth == 1) {
			int n = 0;
			for (unsigned pr = 0; pr < MoveList::PRIORITIES; ++pr) {
				for (unsigned i = 0; i < moves[depth].getCount(pr); ++i)
					n += state.isLegalMove2(moves[depth].getMove(pr, i));
			}
			return n;
		}
		uint64_t count = 0;
		for (unsigned pr = 0; pr < MoveList::PRIORITIES; ++pr) {
			for (unsigned i = 0; i < moves[depth].getCount(pr); ++i) {
				Move m = moves[depth].getMove(pr, i);
				if (state.isLegalMove2(m)) {
					state.makeMove(m);
					count += perft(state, depth - 1);
					state.undoMove(m);
				}
			}
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
};

}
