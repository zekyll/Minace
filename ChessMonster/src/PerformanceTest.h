#pragma once

#include "MinMaxAI.h"
#include "Logger.h"
#include "GameGenerator.h"
#include "Util.h"
#include "TimeConstraint.h"
#include <random>
#include <cstdint>
#include <cmath>

namespace cm {

/**
 * Tests performance of the MinMaxAI in randomly generated positions with different search depths.
 */
class PerformanceTest
{
private:
	Logger& mLogger;

	unsigned mStartDepth;

	double mLength;

	bool mQs;

	uint64_t mTotalNodes;

	double mTotalEbf;

public:

	PerformanceTest(Logger& logger, unsigned startDepth, double length, bool qs)
	: mLogger(logger), mStartDepth(startDepth), mLength(length), mQs(qs), mTotalNodes(0)
	{
	}

	void operator ()()
	{
		mLogger.logMessage("Running performance test...");

		unsigned depth = mStartDepth;
		unsigned n;
		do {
			std::mt19937_64 rng(1234567);

			double totalTime = 0;
			mTotalNodes = 0;
			mTotalEbf = 0;

			MinMaxAI ai(nullptr, mQs * 30, 0);
			TimeConstraint tc(depth);
			n = 0;
			while (totalTime < mLength) {
				totalTime += runSingleTest(ai, rng(), tc);
				++n;
			}

			double avgTime = totalTime * 1e3 / n;
			printStatistics(depth, n, avgTime);

			++depth;
		} while (n > 10);

		mLogger.logMessage("Test done.");
	}

	double runSingleTest(MinMaxAI& ai, uint64_t seed, const TimeConstraint& tc)
	{
		GameState state = GameGenerator::createGame(seed);
		
		auto start = std::chrono::high_resolution_clock::now();
		if (!ai.getMove(state, tc))
			throw 0;
		mTotalNodes += ai.nodeCount();
		mTotalEbf += ai.effectiveBranchingFactor();
		auto dur = std::chrono::high_resolution_clock::now() - start;
		return std::chrono::duration_cast<std::chrono::nanoseconds>(dur).count() * 1e-9;
	}

	void printStatistics(unsigned depth, unsigned n, double avgTime)
	{
		double eubf = calculateEffectiveUniformBranchingFactor((double)mTotalNodes / n, depth);
		mLogger.logMessage(strFormat(200,
				"depth=%d n=%d avgTime=%.3fms avgNodes=%.3g eubf=%.3g idebf=%.3g",
				depth, n, avgTime, (double) mTotalNodes / n, eubf, mTotalEbf / n));
	}
};

}
