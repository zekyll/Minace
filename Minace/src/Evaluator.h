#pragma once

#include "Scores.h"
#include "Player.h"
#include "GameState.h"

namespace mnc {

/**
 * Keeps track of score (aka static evaluation) for the game state. The score is updated
 * incrementally by notifying about each move (makeMove/makeNullMove/undoMove). Scores is
 * calculated based on piece values and locations, and it is symmetric in the sense that the score
 * for one player is the additive inverse of the other player's score.
 */
class Evaluator
{
private:
	unsigned mPly;

	std::vector<int> mScores;

	Player mPlayer;
public:

	Evaluator(size_t maxDepth)
	: mPly(0), mScores(maxDepth + 1), mPlayer(Player::NONE)
	{
	}

	void reset(const GameState& state)
	{
		mPly = 0;
		mScores[mPly] = 0;
		mPlayer = state.activePlayer();

		for (unsigned playerOffset = 0; playerOffset < 2; ++playerOffset) {
			for (unsigned pieceType = 0; pieceType < Piece::COUNT; ++pieceType) {
				Player player((mPlayer + playerOffset) % 2);
				Mask pieces = state.board()(player, Piece(pieceType));
				for (Sqr sqr : pieces) {
					int value = Scores::POSITIONAL_PIECE_VALUES[player][pieceType][sqr];
					mScores[mPly] += (1 - 2 * playerOffset) * value;
				}
			}
		}
	}

	int getScore()
	{
		assert(mPlayer);
		return mScores[mPly];
	}

	int getRelativeScore()
	{
		assert(mPlayer);
		return mScores[mPly] - (1 - 2 * (mPly & 1)) * mScores[0];
	}

	void makeMove(Move move)
	{
		assert(mPlayer);
		assert(mPly < mScores.size());

		int score = mScores[mPly];

		Sqr fromSqr = move.fromSqr();
		Sqr toSqr = move.toSqr();
		Piece pieceType = move.pieceType();
		Piece capturedType = move.capturedType();
		Piece newType = move.newType();

		score -= Scores::POSITIONAL_PIECE_VALUES[mPlayer][pieceType][fromSqr];
		score += Scores::POSITIONAL_PIECE_VALUES[mPlayer][newType][toSqr];
		if (capturedType)
			score += Scores::POSITIONAL_PIECE_VALUES[~mPlayer][capturedType][toSqr];

		//TODO handle castling and en passant

		mScores[++mPly] = -score;
		mPlayer = ~mPlayer;
	}

	void makeNullMove()
	{
		assert(mPlayer);
		int score = mScores[mPly];
		mScores[++mPly] = -score;
		mPlayer = ~mPlayer;
	}

	void undoMove()
	{
		assert(mPly >= 0);
		--mPly;
		mPlayer = ~mPlayer;
	}
};

}
