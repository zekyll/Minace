#pragma once

#include "GameState.h"
#include "Piece.h"
#include "BitBoard.h"
#include "Player.h"
#include <random>
#include <cstdint>

namespace mnc {

/**
 * Generates a game state from 64-bit seed according to the following rules:
 * - The game state is legal (each player has one king, no check mate or stale mate, player that
 * is next to move is not threatening opponent's king, no pawns on last row).
 * - White is next to move.
 * - En passant is not possible.
 * - No castling rights for either side.
 * - Half move clock starts at 0.
 * - Number of pieces of each type (except king) is between 0 and n, where n is the number in the
 * standard starting position.
 * - The position is not guaranteed to be balanced.
 */
class GameGenerator
{
public:

	static GameState createGame(uint64_t seed)
	{
		std::mt19937_64 rng(seed);
		GameState state;
		do {
			BitBoard board;
			addRandomizedPieces(board, 1, 1, Piece::KING, rng);
			addRandomizedPieces(board, 0, 1, Piece::QUEEN, rng);
			addRandomizedPieces(board, 0, 2, Piece::ROOK, rng);
			addRandomizedPieces(board, 0, 2, Piece::BISHOP, rng);
			addRandomizedPieces(board, 0, 2, Piece::KNIGHT, rng);
			addRandomizedPieces(board, 0, 8, Piece::PAWN, rng);
			state = GameState(board, Player::WHITE);
		} while (state.isCheckMate() || state.isStaleMate() || state.isKingChecked(Player::BLACK));

		return state;
	}

private:

	static void addRandomizedPieces(BitBoard& board, unsigned min, unsigned max, Piece pieceType,
			std::mt19937_64& rng)
	{
		std::uniform_int_distribution<unsigned> distr0_63(0, 63);
		std::uniform_int_distribution<unsigned> distr8_55(8, 55);

		for (unsigned player = 0; player < 2; ++player) {
			unsigned n = std::uniform_int_distribution<unsigned>(min, max)(rng);
			for (unsigned i = 0; i < n; ++i) {
				Sqr sqr;
				do {
					// No soldiers on first/last row.
					sqr = Sqr(pieceType != Piece::PAWN ? distr0_63(rng) : distr8_55(rng));
				} while (board(sqr));
				board.addPiece(Player(player), pieceType, sqr);
			}
		}
	}
};

}