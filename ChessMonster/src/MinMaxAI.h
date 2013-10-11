#pragma once

#include "TranspositionTable.h"
#include "StateInfo.h"
#include "Evaluator.h"
#include "MoveList.h"
#include "TreeGenerator.h"
#include "Scores.h"
#include "GamePlayer.h"
#include "GameState.h"
#include "Move.h"
#include <algorithm>
#include <string>
#include <vector>
#include <chrono>
#include <exception>
#include <cmath>

namespace cm {

class Logger; //TODO

/**
 * AI based on minmax/negamax and alpha-beta pruning.
 */
class MinMaxAI : public GamePlayer
{
private:

	class TimeLimitException : public std::exception
	{
	};

	static constexpr unsigned NULL_MOVE_REDUCTION1 = 2;

	static constexpr unsigned NULL_MOVE_REDUCTION2 = 4;

	static constexpr size_t MAX_TRANSPOSITION_TABLE_SIZE = 1024 * 1024;

	unsigned mSearchDepth;

	unsigned mQuiescenceSearchDepth;

	TranspositionTable<StateInfo> mTrposTbl;

	TranspositionTable<uint64_t> mEarlierStates;

	double mTimeLimit;

	Logger* mLogger;

	unsigned mPly;

	int mRootScore;

	std::vector<StateInfo> mResults;

	TreeGenerator mTreeGenerator;

	SearchTreeNode mTree;

	unsigned mNodeCount; //TODO 64-bit?

	unsigned mTrposTblHitCount;

	std::vector<MoveList> mMoveLists;

	std::chrono::high_resolution_clock::time_point mStartTime;

	Evaluator mEvaluator;

public:

	MinMaxAI(Logger* logger, unsigned searchDepth, unsigned quiescenceSearchDepth, double timeLimit,
			unsigned treeGenerationDepth)
	: mSearchDepth(searchDepth),
	mQuiescenceSearchDepth(quiescenceSearchDepth),
	mEarlierStates(512),
	mTimeLimit(timeLimit),
	mLogger(logger),
	mPly(0),
	mResults(searchDepth + 1 + quiescenceSearchDepth),
	mTreeGenerator(treeGenerationDepth),
	mMoveLists(searchDepth + 1 + quiescenceSearchDepth),
	mEvaluator(searchDepth + quiescenceSearchDepth)
	{
		if (searchDepth < 2)
			throw std::invalid_argument("Search depth too small.");
	}

	virtual Move getMove(const GameState& state) override
	{
		//mTree = nullptr;
		mEvaluator.reset(state);
		mRootScore = mEvaluator.getScore();
		setEarlierStates(state);
		mStartTime = std::chrono::high_resolution_clock::now();
		mTrposTbl.clear();
		Move bestMove; // none
		GameState stateCopy = state;
		//		unsigned lastIterNodeCount = 0, lastIterTrPosTblHitCount = 0, lastIterTrposTblSize = 0;
		//		double lastIterBranchingFactor = 0.0;

		for (int depth = 2; (unsigned) depth <= mSearchDepth; ++depth) {
			if (!findMove(stateCopy, depth))
				break;

			//			lastIterNodeCount = mNodeCount;
			//			lastIterTrPosTblHitCount = mTrposTblHitCount;
			//			lastIterTrposTblSize = mTrposTbl.size();
			//			lastIterBranchingFactor = std::pow(mNodeCount, 1.0 / depth);
			bestMove = mResults[0].bestMove;
		}

		//		log("nodeCount=" + lastIterNodeCount);
		//		log("trposTblHitCount=" + lastIterTrPosTblHitCount);
		//		log("trposTblSize=" + lastIterTrposTblSize);
		//		log(String.format("t=%.3fms", (System.nanoTime() - mStartTime) * 1e-6));
		//		log(String.format("branchingFactor=%.3g", lastIterBranchingFactor));

		return bestMove;
	}

	void setLogger(Logger* logger)
	{
		mLogger = logger;
	}

	//	Node* getSearchTree()
	//	{
	//		return mTree;
	//	}

private:

	bool findMove(GameState state, int depth)
	{
		log("depth=" + std::to_string(depth));

		mNodeCount = 0;
		mTrposTblHitCount = 0;
		mPly = 0;
		mTreeGenerator.clear();
		mEvaluator.reset(state);

		try {
			createNodeAndSearch(depth, Scores::MIN, Scores::MAX, state, Move());
		} catch (TimeLimitException e) {
			return false;
		}

		mTree = mTreeGenerator.getTree();

		return true;
	}

	int createNodeAndSearch(int depth, int alpha, int beta, GameState& state, Move move)
	{
		mTreeGenerator.startNode(alpha, beta, state.activePlayer(), move);

		int score = search(depth, alpha, beta, state);

		//int nodeType = mResults[mPly] != null ? mResults[mPly].nodeType : -1;
		int nodeType = -1; //TODO;
		mTreeGenerator.endNode(score, nodeType);

		return score;
	}

	int search(int depth, int alpha, int beta, GameState& state)
	{
		checkTimeLimit();

		++mNodeCount;

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
			++mTrposTblHitCount;
			if (info->nodeType == NodeType::EXACT
					|| (info->nodeType == NodeType::LOWER_BOUND && info->score >= beta)
					|| (info->nodeType == NodeType::UPPER_BOUND && info->score <= alpha))
				return info->score;
		}

		// Initialize the result structure.
		mResults[mPly].id = state.getId();
		mResults[mPly].nodeType = NodeType::UPPER_BOUND;
		mResults[mPly].score = Scores::MIN;
		mEarlierStates.put(state.getId());

		// Search all moves.
		depth = applyNullMoveReduction(depth, beta, state);
		searchAllMoves(depth, alpha, beta, state, info && depth > 0 ? info->bestMove : Move());

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
		mMoveLists[mPly].populate(state, depth <= 0);

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

	int searchMove(int depth, int alpha, int beta, GameState state, Move move)
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

		// Parannus edelliseeen parhaimpaan siirtoon verrattuna.
		if (score > mResults[mPly].score) {
			mResults[mPly].score = score;
			mResults[mPly].bestMove = move;
			if (score > alpha) {
				//				if (mPly == 0 && mLogger)
				//					log("  " + Move.toString(move) + " " + (score - rootScore));
				if (score >= beta)
					mResults[mPly].nodeType = NodeType::LOWER_BOUND;
				else
					mResults[mPly].nodeType = NodeType::EXACT;
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

	void log(const std::string& msg)
	{
		//		if (mLoggingEnabled)
		//			logger.logMessage(msg);
	}

	decltype(mNodeCount) nodeCount()
	{
		return mNodeCount;
	}

	void setEarlierStates(const GameState& state)
	{
		mEarlierStates.clear();
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

	void addTranspositionTableEntry(int depth, StateInfo result)
	{
		if (depth > 0 && mTrposTbl.size() < MAX_TRANSPOSITION_TABLE_SIZE) {
			result.depth = depth;
			mTrposTbl.put(result);
		}
	}

	void checkTimeLimit()
	{
		if ((mNodeCount & 0xfff) == 0) {
			//			if (Thread.interrupted())
			//				throw InterruptedException();
			auto dur = std::chrono::high_resolution_clock::now() - mStartTime;
			double t = std::chrono::duration_cast<std::chrono::microseconds>(dur).count() * 1e-6;
			if (mTimeLimit != 0 && t > mTimeLimit /*&& mtree*/) { //TODO ensure at least 1 iter
				//log(String.format("  time limit (%.1fms)", t * 1e3));
				throw TimeLimitException();
			}
		}
	}
};

}
