#pragma once

#include "Piece.h"
#include "Sqr.h"
#include "Player.h"
#include "Piece.h"
#include <climits>

namespace cm {

/**
 * Constants used for static evaluation of game states.
 */
template<typename TScore>
class Scores_t
{
public:
	static constexpr TScore MIN = -INT_MAX;

	static constexpr TScore MAX = -MIN;

	static constexpr TScore CHECK_MATE_DEPTH_ADJUSTMENT = 1000 * 100;

	static constexpr TScore CHECK_MATE_THRESHOLD = 1000 * 100;

	static constexpr TScore DRAW = 0;

	static constexpr TScore PIECE_VALUES[Piece::COUNT]{
		1000000 * 100,
		9 * 100,
		5 * 100,
		3 * 100,
		3 * 100,
		1 * 100,
	};

	static TScore POSITIONAL_PIECE_VALUES[Player::COUNT][Piece::COUNT][Sqr::COUNT];

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

template<typename TMask>
Scores_t<TMask> Scores_t<TMask>::sStaticInit;

}
