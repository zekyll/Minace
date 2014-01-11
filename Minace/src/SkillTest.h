#pragma once

#include "GamePlayer.h"
#include "Logger.h"
#include "Epd.h"
#include "Move.h"
#include "Util.h"
#include <vector>
#include <string>
#include <initializer_list>

namespace mnc {

/**
 * Allows testing the skill level of an AI by using a set of positions where the best move is known.
 */
template<typename T = void>
class SkillTest_t
{
public:
	static SkillTest_t ZUGZWANG, EASY;

private:

	std::vector<std::pair<std::string, std::string >> mTestPositions;

public:

	SkillTest_t(std::initializer_list<std::pair<std::string, std::string >> testPositions)
	: mTestPositions(testPositions)
	{
	}

	void run(GamePlayer& ai, const TimeConstraint& tc, Logger& logger) const
	{
		logger.logMessage("Running skill test...");

		unsigned count[2] = {};

		for (const auto& tp : mTestPositions) {
			bool r = testPos(tp.first, tp.second, ai, tc, logger);
			++count[r];
		}

		logger.logMessage("Tests passed: "
				+ std::to_string(count[1]) + "/" + std::to_string(count[0] + count[1]));
	}

private:

	bool testPos(const Epd& epd, Move bestMove, GamePlayer& ai, const TimeConstraint& tc,
			Logger& logger) const
	{
		GameState state(epd);
		Move move = ai.getMove(state, tc);
		if (move == bestMove) {
			logger.logMessage(epd.string() + " [OK]");
			return true;
		} else {
			logger.logMessage(epd.string() + " [FAILED]");
			std::string brd = state.board().toStr(true) + "#";
			brd = "  " + stringReplace(brd, "\n", "\n  ");
			brd = stringReplace(brd, "\n  #", "");
			logger.logMessage(brd);
			logger.logMessage("  Expected: " + bestMove.toStr());
			logger.logMessage("  Chose: " + move.toStr());
			return false;
		}
	}
};

template<typename T>
SkillTest_t<T> SkillTest_t<T>::ZUGZWANG {
	{"8/8/p1p5/1p5p/1P5p/8/PPP2K1p/4R1rk w - - 0 1 bm Rf1; id \"zugzwang.001\";", "a2-a3"},
	{"1q1k4/2Rr4/8/2Q3K1/8/8/8/8 w - - 0 1 bm Kh6;  id \"zugzwang.002\";", "a2-a3"},
	{"7k/5K2/5P1p/3p4/6P1/3p4/8/8 w - - 0 1 bm g5; id \"zugzwang.003\";", "a2-a3"},
	{"8/6B1/p5p1/Pp4kp/1P5r/5P1Q/4q1PK/8 w - - 0 32 bm Qxh4; id \"zugzwang.004\";", "a2-a3"},
	{"8/8/1p1r1k2/p1pPN1p1/P3KnP1/1P6/8/3R4 b - - 0 1 bm Nxd5; id \"zugzwang.005\";", "a2-a3"},
};

// http://www.stmintz.com/ccc/index.php?id=122312
template<typename T>
SkillTest_t<T> SkillTest_t<T>::EASY
{
	{"2kr3r/ppp3pp/8/2b1pp2/P1Pn3P/3P1qP1/2PQ1P1R/R3KB2 b Q - bm Nxc2+;", "Nd4xc2"},
	{"4r3/4P1p1/4k3/2B3pp/1R2K3/r6P/1p4P1/8 b - - bm Ra4;", "Ra3-a4"},
	{"r6r/2p2k2/ppp5/6Q1/5P2/3Pq3/PPP3PP/4RR1K b - - bm Rxh2+;", "Rh8xh2"},
	{"6r1/1k6/N1b5/2P4r/4n3/7R/7K/1N6 b - - bm Nf2;", "Ne4-f2"},
	{"3r4/4kp2/pq2p2P/1p2Q3/1P1NN1pb/P7/4b1PK/3R4 w - - bm Nf5+;", "Nd4-f5"},
	{"4rk2/Q7/6p1/2R1p3/8/5qP1/8/4K3 b - - bm Rd8;", "Re8-d8"},
	{"8/8/5p2/4r2k/5K1P/6QP/8/8 w - - bm Qg7;", "Qg3-g7"},
};

typedef SkillTest_t<> SkillTest;

}
