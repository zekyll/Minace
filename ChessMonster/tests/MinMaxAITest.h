#pragma once

#include "../src/MinMaxAI.h"
#include "../src/GameState.h"
#include "../ttest/ttest.h"
#include <memory>
#include <cstddef>

namespace cm {

class MinMaxAITest : public ttest::TestBase
{
private:

	std::unique_ptr<MinMaxAI> ai;
	TimeConstraint tc;

	TTEST_BEFORE()
	{
		ai.reset(new MinMaxAI());
		tc = TimeConstraint(3);
	}

	TTEST_CASE("Avoids check by escaping.")
	{
		GameState s("Ka8", "Kb6 Bc6", Player::WHITE);
		TTEST_EQUAL(ai->getMove(s, tc).toStr(), "Ka8-b8");
	}

	TTEST_CASE("Avoids check by capturing.")
	{
		GameState s("Ke8", "Qe7 Kc3", Player::WHITE);
		TTEST_EQUAL(ai->getMove(s, tc).toStr(), "Ke8xQe7");
	}

	TTEST_CASE("Avoids check by blocking.")
	{
		GameState s("Ka8 Bb3", "Kb6 Be4 Bf4", Player::WHITE);
		TTEST_EQUAL(ai->getMove(s, tc).toStr(), "Bb3-d5");
	}

	TTEST_CASE("Can check mate.")
	{
		GameState s("Ke2 Rb4 Rd7", "Kf8", Player::WHITE);
		TTEST_EQUAL(ai->getMove(s, tc).toStr(), "Rb4-b8");
	}

	TTEST_CASE("Can fork.")
	{
		GameState s("Ka1 Qc1", "Kf8 Rb8", Player::WHITE);
		TTEST_EQUAL(ai->getMove(s, tc).toStr(), "Qc1-f4");
	}

	TTEST_CASE("Doesn't stalemate when has material advantage.")
	{
		GameState s("Ka8 Qb8", "Kh1", Player::BLACK);
		TTEST_EQUAL("Qb8-g3" == ai->getMove(s, 5).toStr(), false); // depth 5
	}

	TTEST_CASE("Stalemates when has material advantage.")
	{
		GameState s("Ka3 Qa1 Bb1 b3 a2 c2 Rb2", "Kh1 Bh3 c5 c3 Nb4", Player::BLACK);
		TTEST_EQUAL(ai->getMove(s, tc).toStr(), "Bh3-d7");
	}

	TTEST_CASE("Doesn't make illegal move when check mate inevitable.")
	{
		GameState s("Kb8", "Rc7 Kb6", Player::WHITE);
		TTEST_EQUAL(ai->getMove(s, 4).toStr(), "Kb8-a8"); // depth 4
	}

	TTEST_CASE("Promotes to knight for faster check mate.")
	{
		GameState s("Kc6 b7", "Ka6 a7 a5", Player::WHITE);
		TTEST_EQUAL(ai->getMove(s, tc).toStr(), "b7-b8N");
	}

	TTEST_CASE("Promotes to rook to avoid stalemate.")
	{
		GameState s("Ka7 g7 Qc5", "Kd7 Qd3", Player::WHITE);
		TTEST_EQUAL(ai->getMove(s, 7).toStr(), "g7-g8R"); // depth 7
	}

	TTEST_CASE("Can do en passant.")
	{
		GameState s("Ka7 d2", "Kh1 e4", Player::WHITE);
		s.makeMove("d2-d4");
		TTEST_EQUAL(ai->getMove(s, tc).toStr(), "e4xd3");
	}

	TTEST_CASE("Bugfix test #1.")
	{
		GameState s("Ka7 Qg8 Qc5", "Kd7 Qd3", Player::BLACK);
		TTEST_EQUAL(ai->getMove(s, 6).toStr(), "Qd3-a6"); // depth 6
	}

	TTEST_CASE("Bugfix test #2.")
	{
		// Illegal move if CHECK_MATE_DEPTH_ADJUSTMENT too small.
		GameState s("Kb4 Qa1", "b2 Qc4 Rc5 Kd4", Player::WHITE);
		TTEST_EQUAL(ai->getMove(s, 6).toStr(), "Kb4-a3"); // depth 6
	}
};

}
