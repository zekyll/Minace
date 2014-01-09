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

	// Size of the hash table for repeated positions.
	static constexpr unsigned REP_TBL_SIZE = 256;

	static constexpr unsigned REP_TBL_MASK = REP_TBL_SIZE - 1;

	unsigned mQuiescenceSearchDepth;

	TranspositionTable<StateInfo> mTrposTbl;

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

	// Simple hash table for doing a quick preliminary check of repeated positions. Each entry
	// holds the number of positions in that bucket. If zero, it is not necessary to call
	// GameState::isRepeatedState().
	uint8_t mRepetitionTable[REP_TBL_SIZE];

public:

	MinMaxAI(InfoCallback* infoCallback = nullptr, size_t transpositionTableBytes = 32 * (1 << 20),
			unsigned quiescenceSearchDepth = 30, unsigned treeGenerationDepth = 0)
	: mQuiescenceSearchDepth(quiescenceSearchDepth),
	mTrposTbl(transpositionTableBytes),
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
		initRepetitionTable(state);
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
			createNodeAndSearch<false>(depth, Scores::MIN, Scores::MAX, state, Move());
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

	template<bool tQs>
	int createNodeAndSearch(int depth, int alpha, int beta, GameState& state, Move move)
	{
#if CM_EXTRA_INFO
		mTreeGenerator.startNode(alpha, beta, state.activePlayer(), move);
#endif

		int score;
		if (tQs)
			score = quiescenceSearch(depth, alpha, beta, state);
		else
			score = search(depth, alpha, beta, state);

#if CM_EXTRA_INFO
		//NodeType nodeType = mResults[mPly] != null ? mResults[mPly].nodeType : -1; //TODO;
		NodeType nodeType = NodeType::NONE;
		mTreeGenerator.endNode(score, nodeType);
#endif

		return score;
	}

	/* Searches only winning captures. */
	int quiescenceSearch(int depth, int alpha, int beta, GameState& state)
	{
		// Check time limit periodically.
		if ((mNodeCount & 0xfff) == 0)
			checkTimeLimit();
		++mNodeCount;

		// Stalemate on first repetition.
		if (mRepetitionTable[state.id() & REP_TBL_MASK] && state.isRepeatedState() && mPly > 0)
			return Scores::DRAW;

		// King "captured".
		if (mEvaluator.getScore() < -Scores::CHECK_MATE_THRESHOLD) //TODO correct?
			return mEvaluator.getScore();

		// Stand pat.
		alpha = std::max(alpha, mEvaluator.getScore());
		if ((unsigned) -depth >= mQuiescenceSearchDepth || alpha >= beta)
			return alpha; //TODO beta?

		// Check if we can get cutoff from transposition table. If not then we can still use
		// the best stored move.
		const StateInfo* info = mTrposTbl.get(state.id());
		if (info && (info->nodeType == NodeType::EXACT
				|| (info->nodeType == NodeType::LOWER_BOUND && info->score >= beta)
				|| (info->nodeType == NodeType::UPPER_BOUND && info->score <= alpha))) {
#if CM_EXTRA_INFO
			++mTrposTblCutoffs;
#endif
			return info->score;
		}
		Move bestMove = info && info->bestMove.isCapture() ? info->bestMove : Move();

		// Search captures.
		alpha = searchMoves<true>(depth, alpha, beta, state, bestMove);

		// Prioritize faster mates.
		alpha = applyScoreDepthAdjustment(alpha, state);

		return alpha;
	}

	/* Normal search. */
	int search(int depth, int alpha, int beta, GameState& state)
	{
		if (depth <= 0)
			return quiescenceSearch(depth, alpha, beta, state);

		// Check time limit periodically.
		if ((mNodeCount & 0xfff) == 0)
			checkTimeLimit();
		++mNodeCount;

		// Stalemate on first repetition.
		if (mRepetitionTable[state.id() & REP_TBL_MASK] && state.isRepeatedState() && mPly > 0)
			return Scores::DRAW;

		// King "captured".
		if (mEvaluator.getScore() < -Scores::CHECK_MATE_THRESHOLD) //TODO correct?
			return mEvaluator.getScore();

		// Check if we can get the result from transposition table. If not then we can still used
		// the best stored move.
		const StateInfo* info = mTrposTbl.get(state.id());
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
		mResults[mPly].id = state.id();
		mResults[mPly].nodeType = NodeType::UPPER_BOUND;
		mResults[mPly].score = Scores::MIN;
		mResults[mPly].bestMove = Move();

		// Mark state in order to check repetitions.
		++mRepetitionTable[state.id() & REP_TBL_MASK];

		// Reduce depth if we still get beta cutoff after null move.
		depth = applyNullMoveReduction(depth, beta, state);

		// Search all moves.
		searchMoves<false>(depth, alpha, beta, state, bestMove);

		--mRepetitionTable[state.id() & REP_TBL_MASK];

		// Prioritize faster mates.
		mResults[mPly].score = applyScoreDepthAdjustment(mResults[mPly].score, state);

		// Only insert in transposition table after searching, because same position might be
		// encountered during search.
		addTranspositionTableEntry(depth, mResults[mPly]);

		return mResults[mPly].score;
	}

	template<bool tQs>
	int zeroWindowSearch(int depth, int beta, GameState& state, Move move)
	{
		return createNodeAndSearch<tQs>(depth, beta - 1, beta, state, move);
	}

	template<bool tQs>
	int searchMoves(int depth, int alpha, int beta, GameState& state, Move tpTblMove)
	{
		// If earlier best move was found in transposition table, try it first.
		if (tpTblMove) {
			alpha = std::max(alpha, searchMove<tQs>(depth, alpha, beta, state, tpTblMove));
			if (alpha >= beta)
				return alpha;
		}

		// Create prioritized move list. In normal search goes through all moves; in quiescence
		// search only captures.
		mMoveLists[mPly].populate(state, tQs, mKillerMoves[mPly]);

		// Iterate over all moves in prioritized order.
		for (unsigned pri = 0; pri < MoveList::PRIORITIES; ++pri) {
			unsigned count = mMoveLists[mPly].getCount(pri);
			for (unsigned i = 0; i < count; ++i) {
				Move move = mMoveLists[mPly].getMove(pri, i);
				if (move == tpTblMove) // Already searched.
					continue;
				alpha = std::max(alpha, searchMove<tQs>(depth, alpha, beta, state, move));
				if (alpha >= beta)
					return alpha;
			}
		}

		// Stale mate recognition.
		if (!tQs && mResults[mPly].score < -Scores::CHECK_MATE_THRESHOLD && state.isStaleMate())
			mResults[mPly].score = 0;

		return alpha;
	}

	template<bool tQs>
	int searchMove(int depth, int alpha, int beta, GameState& state, Move move)
	{
		// Make move.
		++mPly;
		state.makeMove(move);
		mEvaluator.makeMove(move);

		// Continue search recursively.
		int score;
		if (!tQs) {
			// For PV node a full search is made and zero window search for others.
			if (mResults[mPly - 1].nodeType == NodeType::UPPER_BOUND) {
				// Search normally until value is found in range ]alfa,beta[
				score = -createNodeAndSearch<tQs>(depth - 1, -beta, -alpha, state, move);
			} else {
				// For rest of the nodes it's only necessary to check that score is at most alpha (or
				// causes beta-cutoff). If not, then make normal search.
				score = -zeroWindowSearch<tQs>(depth - 1, -alpha, state, move);
				if (score > alpha && score < beta)
					score = -createNodeAndSearch<tQs>(depth - 1, -beta, -alpha, state, move);
			}
		} else {
			// Normal search in Qs
			score = -createNodeAndSearch<tQs>(depth - 1, -beta, -alpha, state, move);
		}

		// Undo move.
		mEvaluator.undoMove();
		state.undoMove(move);
		--mPly;

		// Found better move.
		if (!tQs) {
			// In normal search keep track of best score even if it's lower than alpha.
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
		} else {
			if (score > alpha)
				alpha = score;
		}

		return alpha;
	}

	/* Reduces depth if beta cutoff can still be achieved with null move search. */
	int applyNullMoveReduction(int depth, int beta, GameState& state)
	{
		if (depth >= (int) (NULL_MOVE_REDUCTION1 + 1)) {
			state.makeNullMove();
			mEvaluator.makeNullMove();
			++mPly;
			int zwsDepth = depth - NULL_MOVE_REDUCTION1 - 1;
			int score = -zeroWindowSearch<false>(zwsDepth, 1 - beta, state, Move());
			--mPly;
			mEvaluator.undoMove();
			state.undoNullMove();
			if (score >= beta)
				depth = std::max<int>(depth - NULL_MOVE_REDUCTION2, 1);
		}
		return depth;
	}

	void initRepetitionTable(const GameState& state)
	{
		std::fill(std::begin(mRepetitionTable), std::end(mRepetitionTable), 0);
		unsigned start = state.ply() - std::min(state.halfMoveClock(), state.ply());
		for (unsigned i = start; i < state.ply(); ++i)
			++mRepetitionTable[state.id(i) & REP_TBL_MASK];
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
			timeSlot = std::min(timeSlot, mTimeConstraint.clock[player] - CLOCK_SAFETY_MARGIN);

			if (mTimeConstraint.time)
				mTimeConstraint.time = std::min(mTimeConstraint.time, timeSlot);
			else
				mTimeConstraint.time = timeSlot;
		}

		//mInfoCallback->notifyString("Time limit: " + std::to_string(mTimeConstraint.time));
	}
};

}
