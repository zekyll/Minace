#pragma once

#include "TimeConstraint.h"
#include "TranspositionTable.h"
#include "StateInfo.h"
#include "Evaluator.h"
#include "MoveList.h"
#include "TreeGenerator.h"
#include "Scores.h"
#include "GamePlayer.h"
#include "GameState.h"
#include "Move.h"
#include "Config.h"
#include <algorithm>
#include <string>
#include <vector>
#include <chrono>
#include <exception>
#include <cmath>
#include <atomic>

namespace cm {

class InfoCallback
{
public:

	virtual void notifyPv(unsigned depth, int score, const std::vector<Move>& pv)
	{
	}

	virtual void notifyIterDone(unsigned depth, int score, uint64_t nodes, size_t hashEntries,
			size_t hashCapacity)
	{
	}

	virtual void notifyString(const std::string& s)
	{
	}
};

/**
 * AI based on minmax/negamax and alpha-beta pruning.
 */
class MinMaxAI : public GamePlayer
{
private:

	class StoppedException : public std::exception
	{
	};

	static constexpr unsigned NULL_MOVE_REDUCTION1 = 2;

	static constexpr unsigned NULL_MOVE_REDUCTION2 = 4;

	static constexpr unsigned MAX_SEARCH_DEPTH = 30;

	// Don't let clock run lower than this because of timing inaccuracies, random delays etc.
	static constexpr double CLOCK_SAFETY_MARGIN = 0.1;

	unsigned mQuiescenceSearchDepth;

	TranspositionTable<StateInfo> mTrposTbl;

	HashTable<uint64_t> mEarlierStates;

	TimeConstraint mTimeConstraint;

	InfoCallback* mInfoCallback;

	unsigned mPly;

	std::vector<StateInfo> mResults;

	TreeGenerator mTreeGenerator;

	SearchTreeNode mTree;

	uint64_t mTotalNodeCount;

	unsigned mNodeCount, mTrposTblCutoffs; //TODO 64-bit?

	std::vector<MoveList> mMoveLists;

	std::chrono::high_resolution_clock::time_point mStartTime;

	Evaluator mEvaluator;

	double mEffectiveBranchingFactor;

	Move mBestMove;

	std::atomic_bool mStopped;

	// Evaluation score for previous search.
	int mScore;

	// Moves from sibling nodes that cause beta cutoff.
	std::vector<std::array<Move, 2 >> mKillerMoves;

public:

	MinMaxAI(InfoCallback* infoCallback = nullptr, size_t transpositionTableBytes = 32 * (1 << 20),
			unsigned quiescenceSearchDepth = 30, unsigned treeGenerationDepth = 0)
	: mQuiescenceSearchDepth(quiescenceSearchDepth),
	mTrposTbl(transpositionTableBytes),
	mEarlierStates(8196),
	mInfoCallback(infoCallback),
	mPly(0),
	mResults(MAX_SEARCH_DEPTH + 1 + quiescenceSearchDepth),
	mTreeGenerator(treeGenerationDepth),
	mTrposTblCutoffs(0),
	mMoveLists(MAX_SEARCH_DEPTH + 1 + quiescenceSearchDepth),
	mEvaluator(MAX_SEARCH_DEPTH + quiescenceSearchDepth),
	mEffectiveBranchingFactor(0.0),
	mBestMove(),
	mStopped(ATOMIC_FLAG_INIT),
	mScore(0),
	mKillerMoves(MAX_SEARCH_DEPTH + 1 + quiescenceSearchDepth)
	{
	}

	virtual Move getMove(const GameState& state, const TimeConstraint& tc) override
	{
		mTree = SearchTreeNode();
		mEvaluator.reset(state);
		setEarlierStates(state);
		mStartTime = std::chrono::high_resolution_clock::now();
		mTrposTbl.startNewSearch();
		mNodeCount = 0;
		mEffectiveBranchingFactor = 0.0;
		mBestMove = Move(); // none
		GameState stateCopy = state;
		mStopped = false;
		mTotalNodeCount = 0;
		mScore = 0;
		setupTimeConstraint(tc, state.activePlayer());

		unsigned maxDepth = std::min((unsigned) MAX_SEARCH_DEPTH,
				mTimeConstraint.depth ? mTimeConstraint.depth : (unsigned) - 1);
		maxDepth = std::max(1u + (mQuiescenceSearchDepth == 0), maxDepth);
		for (int depth = 1; (unsigned) depth <= maxDepth; ++depth) {
			if (!findMove(stateCopy, depth))
				break;

			// We need depth>=2 or quiescence search to make sure that king is not left in check.
			if (mQuiescenceSearchDepth > 0 || depth >= 2) {
				mBestMove = mResults[0].bestMove;
				mScore = mResults[0].score;
			}
		}

		return mBestMove;
	}

	SearchTreeNode getSearchTree()
	{
		return std::move(mTree);
	}

	decltype(mNodeCount) nodeCount() const
	{
		return mNodeCount;
	}

	double effectiveBranchingFactor() const
	{
		return mEffectiveBranchingFactor;
	}

	virtual int getScore() override
	{
		return mScore;
	}

	virtual void stop() override
	{
		mStopped = true;
	}

	virtual bool cmd(const std::string& c)
	{
		if (c == "hashinfo") {
			if (mInfoCallback) {
				std::stringstream ss;
				ss << "debug"
						<< " hashused " << mTrposTbl.size()
						<< " hashlimit " << mTrposTbl.limit()
						<< " hashcap " << mTrposTbl.capacity()
						<< " hashcutoffs " << mTrposTblCutoffs
						<< " hashlookups " << mTrposTbl.lookups()
						<< " hashwrites " << mTrposTbl.writes();
				mInfoCallback->notifyString(ss.str());
			}
			return true;
		}
		return false;
	};

private:

	bool findMove(GameState& state, int depth)
	{
		unsigned prevNodeCount = mNodeCount;
		mNodeCount = 0;
		mPly = 0;
		mTreeGenerator.clear();
		mEvaluator.reset(state);

		try {
			createNodeAndSearch(depth, Scores::MIN, Scores::MAX, state, Move());
		} catch (StoppedException& e) {
			return false;
		}

		mTotalNodeCount += mNodeCount;
		mTree = mTreeGenerator.getTree();
		mEffectiveBranchingFactor = (double) mNodeCount / (prevNodeCount ? prevNodeCount : 1);

		if (mInfoCallback) {
			mInfoCallback->notifyIterDone(depth, mResults[0].score, mTotalNodeCount,
					mTrposTbl.size(), mTrposTbl.capacity());
		}

		return true;
	}

	int createNodeAndSearch(int depth, int alpha, int beta, GameState& state, Move move)
	{
#if CM_EXTRA_INFO
		mTreeGenerator.startNode(alpha, beta, state.activePlayer(), move);

		int score = search(depth, alpha, beta, state);

		//NodeType nodeType = mResults[mPly] != null ? mResults[mPly].nodeType : -1; //TODO;
		NodeType nodeType = NodeType::NONE;
		mTreeGenerator.endNode(score, nodeType);

		return score;
#else
		return search(depth, alpha, beta, state);
#endif
	}

	int search(int depth, int alpha, int beta, GameState& state)
	{
		++mNodeCount;
		if ((mNodeCount & 0xfff) == 0)
			checkTimeLimit();

		// Stalemate on first repetition.
		if (mEarlierStates.get(state.getId()) && mPly > 0)
			return Scores::DRAW;

		if (mEvaluator.getScore() < -Scores::CHECK_MATE_THRESHOLD) //TODO correct?
			return mEvaluator.getScore();

		// Quiescence search, when depth <= 0.
		if (depth <= 0) {
			int score = mEvaluator.getScore();
			if ((unsigned) -depth >= mQuiescenceSearchDepth || score >= beta)
				return score;
			if (score > alpha)
				alpha = score;
		}

		// Check if we can get the result from transposition table. If not then we can still used
		// the best stored move.
		const StateInfo* info = mTrposTbl.get(state.getId());
		if (info && info->depth >= depth) {
			if (info->nodeType == NodeType::EXACT
					|| (info->nodeType == NodeType::LOWER_BOUND && info->score >= beta)
					|| (info->nodeType == NodeType::UPPER_BOUND && info->score <= alpha)) {
				mResults[mPly].bestMove = info->bestMove;
				mResults[mPly].score = info->score;
#if CM_EXTRA_INFO
				++mTrposTblCutoffs;
#endif
				return info->score;
			}
		}
		Move bestMove = info && depth > 0 ? info->bestMove : Move();

		// Initialize the result structure.
		mResults[mPly].id = state.getId();
		mResults[mPly].nodeType = NodeType::UPPER_BOUND;
		mResults[mPly].score = Scores::MIN;
		mResults[mPly].bestMove = Move();
		mEarlierStates.put(state.getId());

		// Search all moves.
		depth = applyNullMoveReduction(depth, beta, state);
		searchAllMoves(depth, alpha, beta, state, bestMove);

		mEarlierStates.remove(state.getId());

		// If quiescence search did not find any captures, set score to lower bound.
		if (depth <= 0 && mResults[mPly].score == Scores::MIN)
			mResults[mPly].score = alpha;

		mResults[mPly].score = applyScoreDepthAdjustment(mResults[mPly].score, state);

		// Only insert in transposition table after searching, because same position might be
		// encountered during search.
		addTranspositionTableEntry(depth, mResults[mPly]);

		return mResults[mPly].score;
	}

	int zeroWindowSearch(int depth, int beta, GameState& state, Move move)
	{
		return createNodeAndSearch(depth, beta - 1, beta, state, move);
	}

	void searchAllMoves(int depth, int alpha, int beta, GameState& state, Move tpTblMove)
	{
		// If earlier best move was found in transposition table, try it first.
		if (tpTblMove) {
			alpha = searchMove(depth, alpha, beta, state, tpTblMove);
			if (alpha >= beta)
				return;
		}

		// Create prioritized move list. In normal search goes through all moves; in quiescence
		// search (depth <= 0) only captures.
		mMoveLists[mPly].populate(state, depth <= 0, mKillerMoves[mPly]);

		// Iterate over all moves in prioritized order.
		for (unsigned pri = 0; pri < MoveList::PRIORITIES; ++pri) {
			unsigned count = mMoveLists[mPly].getCount(pri);
			for (unsigned i = 0; i < count; ++i) {
				Move move = mMoveLists[mPly].getMove(pri, i);
				if (move == tpTblMove) // Already searched.
					continue;
				alpha = searchMove(depth, alpha, beta, state, move);
				if (alpha >= beta)
					return;
			}
		}

		// Stale mate recognition.
		if (mResults[mPly].score < -Scores::CHECK_MATE_THRESHOLD && state.isStaleMate())
			mResults[mPly].score = 0;
	}

	int searchMove(int depth, int alpha, int beta, GameState& state, Move move)
	{
		// Make move.
		++mPly;
		state.makeMove(move);
		mEvaluator.makeMove(move);

		// Continue search recursively. For PV node a full search is made and zero window search
		// for others.
		int score;
		if (mResults[mPly - 1].nodeType == NodeType::UPPER_BOUND) {
			// Search normally until value is found in range ]alfa,beta[
			score = -createNodeAndSearch(depth - 1, -beta, -alpha, state, move);
		} else {
			// For rest of the nodes it's only necessary to check that score is at most alpha (or
			// causes beta-cutoff). If not, then make normal search.
			score = -zeroWindowSearch(depth - 1, -alpha, state, move);
			if (score > alpha && score < beta)
				score = -createNodeAndSearch(depth - 1, -beta, -alpha, state, move);
		}

		// Undo move.
		mEvaluator.undoMove();
		state.undoMove(move);
		--mPly;

		// Found better move.
		if (score > mResults[mPly].score) {
			mResults[mPly].score = score;
			mResults[mPly].bestMove = move;
			if (score > alpha) {
				if (mPly == 0 && mInfoCallback) {
					std::vector<Move> mvs{move};
					mInfoCallback->notifyPv(depth, score, mvs);
				}
				if (score >= beta) {
					mResults[mPly].nodeType = NodeType::LOWER_BOUND;
					if (!move.isCapture() && !move.isPromotion()) {
						mKillerMoves[mPly][1] = mKillerMoves[mPly][0];
						mKillerMoves[mPly][0] = move;
					}
				} else {
					mResults[mPly].nodeType = NodeType::EXACT;
				}
				alpha = score;
			}
		}

		return alpha;
	}

	int applyNullMoveReduction(int depth, int beta, GameState& state)
	{
		if (depth >= (int) (NULL_MOVE_REDUCTION1 + 1)) {
			state.makeNullMove();
			mEvaluator.makeNullMove();
			++mPly;
			int zwsDepth = depth - NULL_MOVE_REDUCTION1 - 1;
			int score = -zeroWindowSearch(zwsDepth, 1 - beta, state, Move());
			--mPly;
			mEvaluator.undoMove();
			state.undoNullMove();
			if (score >= beta)
				depth = std::max<int>(depth - NULL_MOVE_REDUCTION2, 1);
		}
		return depth;
	}

	void setEarlierStates(const GameState& state)
	{
		mEarlierStates.clear(8196);
		std::vector<uint64_t> states = state.getEarlierStates();
		for (uint64_t state : states)
			mEarlierStates.put(state);
	}

	int applyScoreDepthAdjustment(int score, const GameState& state)
	{
		if (score > Scores::CHECK_MATE_THRESHOLD)
			score -= Scores::CHECK_MATE_DEPTH_ADJUSTMENT;
		//		else {
		//			int currentScore = getScore(state);
		//			if (score > currentScore)
		//				score -= SCORE_DEPTH_ADJUSTMENT;
		//		}
		return score;
	}

	void addTranspositionTableEntry(int depth, StateInfo& result)
	{
		if (depth > 0) {
			result.depth = depth;
			mTrposTbl.put(result);
		}
	}

	void checkTimeLimit()
	{
		if (mStopped)
			throw StoppedException();
		auto dur = std::chrono::high_resolution_clock::now() - mStartTime;
		double t = std::chrono::duration_cast<std::chrono::microseconds>(dur).count() * 1e-6;
		if (mTimeConstraint.time != 0 && t > mTimeConstraint.time && mBestMove)
			throw StoppedException();
	}

	void setupTimeConstraint(const TimeConstraint& tc, Player player)
	{
		mTimeConstraint = tc;

		// If clock is used then allocate a time slot based on remaining time. The actual time
		// limit is then the minimum of the time slot and the hard limit (tc.time).
		if (mTimeConstraint.clock[player]) {
			double timeSlot = mTimeConstraint.clock[player] - CLOCK_SAFETY_MARGIN;
			int moves = mTimeConstraint.clockMovesLeft ? mTimeConstraint.clockMovesLeft : 30;
			timeSlot += moves * mTimeConstraint.clockIncrement[player];
			timeSlot /= moves + 1;

			if (mTimeConstraint.time)
				mTimeConstraint.time = std::min(mTimeConstraint.time, timeSlot);
			else
				mTimeConstraint.time = timeSlot;
		}

		//mInfoCallback->notifyString("Time limit: " + std::to_string(mTimeConstraint.time));
	}
};

}
