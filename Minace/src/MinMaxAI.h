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

namespace mnc {

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

	static constexpr unsigned MAX_SEARCH_DEPTH = 100;

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
	mResults(MAX_SEARCH_DEPTH + 1),
	mTreeGenerator(treeGenerationDepth),
	mTrposTblCutoffs(0),
	mMoveLists(MAX_SEARCH_DEPTH + 1),
	mEvaluator(MAX_SEARCH_DEPTH),
	mEffectiveBranchingFactor(0.0),
	mBestMove(),
	mStopped(ATOMIC_FLAG_INIT),
	mScore(0),
	mKillerMoves(MAX_SEARCH_DEPTH + 1)
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
		mResults[0].bestMove = Move();
		mTreeGenerator.clear();
		mEvaluator.reset(state);

		try {
			createNodeAndSearch<false>(depth, -Scores::INF, Scores::INF, state, Move());
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
		assert(alpha < beta);
		assert(!Scores::isInf(alpha));
		assert(!Scores::isInf(-beta));
		assert(std::abs(mEvaluator.getScore()) < Scores::CHECK_MATE_THRESHOLD);
		assert(mPly > 0);

		// Check time limit periodically.
		if ((mNodeCount & 0xfff) == 0)
			checkTimeLimit();
		++mNodeCount;

		// Stalemate on first repetition.
		if (mRepetitionTable[state.id() & REP_TBL_MASK] && state.isRepeatedState() && mPly > 0)
			return Scores::DRAW;

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

		// Stand pat.
		alpha = std::max(alpha, mEvaluator.getScore());
		if (alpha >= beta || mPly >= MAX_SEARCH_DEPTH ||
				(unsigned) -depth >= mQuiescenceSearchDepth)
			return alpha; // No need to adjust; stand pat can't have mate score.

		// Adjust search window due to mate delay penalty.
		alpha += alpha > Scores::CHECK_MATE_THRESHOLD;
		beta += beta > Scores::CHECK_MATE_THRESHOLD;

		// Search captures.
		alpha = searchMoves<true>(depth, alpha, beta, state, bestMove, false /*(ignored)*/);

		// Delay penalty for mates.
		alpha -= alpha > Scores::CHECK_MATE_THRESHOLD;

		return alpha;
	}

	/* Normal search. */
	int search(int depth, int alpha, int beta, GameState& state)
	{
		assert(alpha < beta);
		assert(!Scores::isInf(alpha));
		assert(!Scores::isInf(-beta));
		assert(std::abs(mEvaluator.getScore()) < Scores::CHECK_MATE_THRESHOLD);

		if (depth <= 0)
			return quiescenceSearch(depth, alpha, beta, state);

		// Check time limit periodically.
		if ((mNodeCount & 0xfff) == 0)
			checkTimeLimit();
		++mNodeCount;

		// Stalemate on first repetition. (Allow in ply 0 because it is a real game position.)
		if (mRepetitionTable[state.id() & REP_TBL_MASK] && state.isRepeatedState() && mPly > 0)
			return Scores::DRAW;

		if (mPly >= MAX_SEARCH_DEPTH)
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
				if (mPly == 0)
					notifyNewPv(state, info->bestMove, depth, info->score);
				return info->score;
			}
		}
		Move bestMove = info ? info->bestMove : Move();

		// Check extension.
		bool checked = state.isKingChecked(state.activePlayer());
		if (checked)
			++depth;

		// Adjust search window due to mate delay penalty.
		alpha += alpha > Scores::CHECK_MATE_THRESHOLD;
		beta += beta > Scores::CHECK_MATE_THRESHOLD;

		// Initialize the result structure.
		mResults[mPly].id = state.id();
		mResults[mPly].nodeType = NodeType::UPPER_BOUND;
		mResults[mPly].score = -Scores::INF;
		mResults[mPly].bestMove = Move();

		// Mark state in order to check repetitions.
		++mRepetitionTable[state.id() & REP_TBL_MASK];

		// Reduce depth if we still get beta cutoff after null move.
		if (!checked)
			depth = applyNullMoveReduction(depth, beta, state);

		// Search all moves.
		searchMoves<false>(depth, alpha, beta, state, bestMove, checked);

		--mRepetitionTable[state.id() & REP_TBL_MASK];

		// Check mate & stale mate recognition.
		if (!mResults[mPly].bestMove)
			mResults[mPly].score = checked ? -Scores::MATE : Scores::DRAW;

		// Delay penalty for mates.
		mResults[mPly].score -= mResults[mPly].score > Scores::CHECK_MATE_THRESHOLD;

		// Only insert in transposition table after searching, because position may be repeated
		// during search. Stored depth must be the original depth without check extension.
		addTranspositionTableEntry(depth - checked, mResults[mPly]);

		assert(Scores::isValid(mResults[mPly].score));
		assert(mResults[mPly].bestMove || mResults[mPly].score == -Scores::MATE ||
				mResults[mPly].score == Scores::DRAW);

		return mResults[mPly].score;
	}

	template<bool tQs>
	int zeroWindowSearch(int depth, int beta, GameState& state, Move move)
	{
		assert(Scores::isValid(beta));
		return createNodeAndSearch<tQs>(depth, beta - 1, beta, state, move);
	}

	template<bool tQs>
	int searchMoves(int depth, int alpha, int beta, GameState& state, Move tpTblMove, bool checked)
	{
		assert(alpha < beta);
		assert(!Scores::isInf(alpha));
		assert(!Scores::isInf(-beta));

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

		return alpha;
	}

	template<bool tQs>
	int searchMove(int depth, int alpha, int beta, GameState& state, Move move)
	{
		assert(alpha < beta);
		assert(!Scores::isInf(alpha));
		assert(!Scores::isInf(-beta));
		assert(move);

		// Make move.
		++mPly;
		Player pl = state.activePlayer();
		state.makeMove(move);
		if (state.isKingChecked(pl)) {
			state.undoMove(move);
			--mPly;
			return -Scores::INF;
		}
		mEvaluator.makeMove(move);

		// Continue search recursively.
		int score;
		if (!tQs) {
			// For PV node a full search is made and zero window search for others.
			if (mResults[mPly - 1].nodeType == NodeType::UPPER_BOUND) {
				// Search normally until value is found in range ]alfa,beta[
				score = -createNodeAndSearch<tQs>(depth - 1, -beta, -alpha, state, move);
			} else {
				// For rest of the nodes it's only necessary to check that score is at most alpha
				// (or causes beta-cutoff). If not, then make normal search.
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
					if (mPly == 0) {
						int adjustedScore = score - (score > Scores::CHECK_MATE_THRESHOLD);
						notifyNewPv(state, move, depth, adjustedScore);
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
		if (depth >= (int) (NULL_MOVE_REDUCTION1 + 1) && !Scores::isInf(beta)) {
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

	void addTranspositionTableEntry(int depth, StateInfo& result)
	{
		if (depth > 0) {
			result.depth = depth;
			mTrposTbl.put(result);
		}
	}

	void checkTimeLimit()
	{
		auto dur = std::chrono::high_resolution_clock::now() - mStartTime;
		double t = std::chrono::duration_cast<std::chrono::microseconds>(dur).count() * 1e-6;
		if ((mStopped || (mTimeConstraint.time != 0 && t > mTimeConstraint.time)) && mBestMove)
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

	/* Called when found a new best move at ply 0. We can always store it as a the new best overall
	 * move because the best move from previous ID iteration is always searched first (thanks to
	 * transposition table). I.e. completing an iteration is not necessary.
	 * 
	 * Finds the principal variation for output from transposition table. This method is not very
	 * reliable but better than nothing. */
	void notifyNewPv(GameState& state, Move firstMove, int depth, int score)
	{
		assert(firstMove);
		assert(depth > 0);

		mBestMove = firstMove;
		mScore = score;

		if (mInfoCallback) {
			std::vector<Move> pv{firstMove};
			for (Move move = firstMove;;) {
				state.makeMove(move);
				const StateInfo* info = mTrposTbl.get(state.id());
				if (!info || info->nodeType != NodeType::EXACT)
					break;
				if (std::find(pv.begin(), pv.end(), info->bestMove) != pv.end())
					break;
				assert(info->bestMove);
				pv.push_back(info->bestMove);
				move = info->bestMove;
			}

			for (auto it = pv.rbegin(); it != pv.rend(); ++it)
				state.undoMove(*it);

			mInfoCallback->notifyPv(depth, score, pv);
		}
	}
};

}
