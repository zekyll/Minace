#pragma once

#include "Piece.h"
#include "Sqr.h"
#include "Player.h"
#include "Piece.h"
#include <climits>
#include <iomanip>
#include <cmath>

namespace mnc {

/**
 * Constants used for static evaluation of game states.
 */
template<typename TScore>
class Scores_t
{
public:
	static constexpr TScore INF = INT_MAX - 10000;

	static constexpr TScore CHECK_MATE_THRESHOLD = 1000 * 100;

	static constexpr TScore DRAW = 0;

	static constexpr TScore MATE = 1000000 * 100;

	static constexpr TScore PIECE_VALUES[Piece::COUNT]{
		MATE,
		9 * 100,
		5 * 100,
		3 * 100,
		3 * 100,
		1 * 100,
	};

	static TScore POSITIONAL_PIECE_VALUES[Player::COUNT][Piece::COUNT][Sqr::COUNT];

	/* Checks that score is not infinite. */
	static int isValid(int score)
	{
		return !isInf(score) && !isInf(-score);
	}

	/* Checks if score is infinite (may not be exactly INF due to window adjustments). */
	static int isInf(int score)
	{
		return score > INF - 10000;
	}

	/* Get score for mate in given number of moves. (Negative if player gets mated. ) */
	static int getCheckMateScore(int moves)
	{
		int score = PIECE_VALUES[Piece::KING] - std::abs(moves);
		return moves >= 0 ? score : -score;
	}

	/* Show score in floating point form and with human readable mate scores. */
	static std::string toStr(int score)
	{
		if (score > CHECK_MATE_THRESHOLD) {
			int moves = PIECE_VALUES[Piece::KING] - score;
			return "mate " + std::to_string(moves);
		} else if (score < -CHECK_MATE_THRESHOLD) {
			int moves = -(score - (-PIECE_VALUES[Piece::KING]));
			return "mate " + std::to_string(moves);
		} else {
			std::stringstream ss;
			ss << std::setprecision(2) << std::fixed << 0.01 * score;
			return ss.str();
		}
	}

private:

	Scores_t()
	{
		for (unsigned pieceType = 0; pieceType < Piece::COUNT; ++pieceType) {
			for (unsigned sqr = 0; sqr < 64; ++sqr) {
				POSITIONAL_PIECE_VALUES[Player::WHITE][pieceType][sqr] += PIECE_VALUES[pieceType];
				unsigned blackSqr = (7 - sqr / 8) * 8 + sqr % 8;
				POSITIONAL_PIECE_VALUES[Player::BLACK][pieceType][blackSqr] =
						POSITIONAL_PIECE_VALUES[Player::WHITE][pieceType][sqr];
			}
		}
	}

	static Scores_t sStaticInit;
};

typedef Scores_t<int_least32_t> Scores;

template<typename TScore>
constexpr TScore Scores_t<TScore>::PIECE_VALUES[Piece::COUNT];

template<typename TScore>
TScore Scores_t<TScore>::POSITIONAL_PIECE_VALUES[Player::COUNT][Piece::COUNT][Sqr::COUNT]{
	{
		// King
		{
			0, 2, 3, 4, 4, 3, 2, 0,
			2, 4, 5, 6, 6, 5, 4, 2,
			3, 5, 7, 8, 8, 7, 5, 3,
			3, 5, 8, 9, 9, 8, 5, 3,
			3, 5, 8, 9, 9, 8, 5, 3,
			3, 5, 7, 8, 8, 7, 5, 3,
			2, 4, 5, 6, 6, 5, 4, 2,
			0, 2, 3, 4, 5, 4, 3, 0
		},
		// Queen
		{
			0, 2, 3, 4, 4, 3, 2, 0,
			2, 4, 5, 6, 6, 5, 4, 2,
			3, 5, 7, 8, 8, 7, 5, 3,
			3, 5, 8, 9, 9, 8, 5, 3,
			3, 5, 8, 9, 9, 8, 5, 3,
			3, 5, 7, 8, 8, 7, 5, 3,
			2, 4, 5, 6, 6, 5, 4, 2,
			0, 2, 3, 4, 4, 3, 3, 0
		},
		// Rook
		{
			0, 2, 2, 2, 2, 2, 2, 0,
			0, 2, 2, 2, 2, 2, 2, 0,
			0, 2, 2, 2, 2, 2, 2, 0,
			0, 2, 2, 2, 2, 2, 2, 0,
			0, 2, 2, 2, 2, 2, 2, 0,
			0, 2, 2, 2, 2, 2, 2, 0,
			0, 2, 2, 2, 2, 2, 2, 0,
			1, 2, 3, 4, 3, 3, 2, 1
		},
		// Bishop
		{
			0, 1, 1, 1, 1, 1, 1, 0,
			1, 2, 3, 3, 3, 3, 2, 1,
			1, 3, 4, 6, 6, 4, 3, 1,
			1, 3, 7, 8, 8, 7, 3, 1,
			1, 3, 7, 9, 9, 7, 3, 1,
			2, 4, 5, 7, 7, 5, 4, 2,
			1, 4, 3, 3, 3, 3, 4, 1,
			0, 1, 1, 1, 1, 1, 1, 0
		},
		// Knight
		{
			0, 1, 1, 2, 2, 1, 1, 0,
			1, 2, 3, 5, 5, 3, 2, 1,
			3, 4, 6, 7, 7, 6, 4, 3,
			3, 5, 7, 9, 9, 7, 5, 3,
			3, 5, 7, 9, 9, 7, 5, 3,
			3, 4, 6, 8, 8, 6, 4, 3,
			1, 2, 3, 5, 5, 3, 2, 1,
			0, 1, 1, 2, 2, 1, 1, 0
		},
		// Pawn
		{
			0, 0, 0, 0, 0, 0, 0, 0,
			7, 8, 9, 9, 9, 9, 8, 7,
			6, 7, 8, 8, 8, 8, 7, 6,
			5, 6, 7, 7, 7, 7, 6, 5,
			4, 5, 6, 6, 6, 6, 5, 4,
			3, 4, 4, 3, 3, 4, 4, 3,
			2, 2, 2, 0, 0, 2, 2, 2,
			0, 0, 0, 0, 0, 0, 0, 0
		}
	}
};

template<typename TScore>
Scores_t<TScore> Scores_t<TScore>::sStaticInit;

}
