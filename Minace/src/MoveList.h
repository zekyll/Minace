#pragma once

#include "GameState.h"
#include <array>

namespace cm {

/**
 * Manages a list of pseudo-legal moves in specific game state. The list is divided in multiple
 * priority classes to allow simple and efficient move ordering for the search algorithm.
 * Queen promotions: 3
 * Captures: 0-9 (0 is PxK and 9 is KxP)
 * Killer moves: 10
 * Normal (quiet) moves: 11
 * Promotions to rook/bishop/knight: 12
 */
template<typename T = void>
class MoveList_t
{
public:
	static constexpr unsigned PRIORITIES = 13;

private:
	static constexpr unsigned CAPTURE_PRIORITIES[][6]{
		{0, 8, 8, 9, 9, 9},
		{0, 4, 5, 6, 6, 7},
		{0, 3, 4, 5, 5, 6},
		{0, 2, 3, 4, 4, 5},
		{0, 2, 3, 4, 4, 5},
		{0, 1, 2, 3, 3, 4}
	};

	static constexpr unsigned PROMOTION_PRIORITIES[]{~0U, 3, 12, 12, 12, ~0U};

	static constexpr unsigned KILLER_MOVE_PRIORITY = 10;

	static constexpr unsigned QUIET_MOVE_PRIORITY = 11;

	Move moves[PRIORITIES][256];

	std::array<unsigned, PRIORITIES> moveCounts;

public:

	void populate(const GameState& state, bool excludeQuietMoves,
			const std::array<Move, 2>& killerMoves = std::array<Move, 2>())
	{
		Player player = state.activePlayer();
		clear();

		// Other pieces except pawns.
		for (unsigned piece = 0; piece < Piece::COUNT - 1; ++piece) {
			Mask pieces = state.board()(player, Piece(piece));
			addMoves(state, Piece(piece), pieces, Piece(piece), excludeQuietMoves, killerMoves);
		}

		// Promotable pawns.
		Mask pieces = state.board()(player, Piece::PAWN) & MoveMasks::PROMOTABLE[player];
		if (pieces) {
			for (unsigned promoType = Piece::QUEEN; promoType <= Piece::KNIGHT; ++promoType) {
				addMoves(state, Piece::PAWN, pieces, Piece(promoType), excludeQuietMoves,
						killerMoves);
			}
		}

		// Non-promotable pawns.
		pieces = state.board()(player, Piece::PAWN) & ~MoveMasks::PROMOTABLE[player];
		addMoves(state, Piece::PAWN, pieces, Piece::PAWN, excludeQuietMoves, killerMoves);
	}

	unsigned getCount(int priority) const
	{
		return moveCounts[priority];
	}

	Move getMove(unsigned priority, unsigned idx) const
	{
		return moves[priority][idx];
	}

	void clear()
	{
		moveCounts.fill(0);
	}

private:

	void addMoves(const GameState& state, Piece pieceType, Mask pieces, Piece newType,
			bool excludeQuietMoves, const std::array<Move, 2>& killerMoves)
	{
		Player player = state.activePlayer();
		Mask enPassantMask = getEnPassantMask(state);

		for (Sqr fromSqr : pieces) {
			Mask moves = state.getPseudoLegalMoves(player, pieceType, fromSqr);

			if ((moves & state.board()(~player)) || state.enPassantSqr()) {
				for (unsigned capturedType = 0; capturedType < Piece::COUNT; ++capturedType) {
					Mask captureTargets = state.board()(~player, Piece(capturedType));
					if (pieceType == Piece::PAWN && capturedType == Piece::PAWN)
						captureTargets |= enPassantMask;
					Mask captureMoves = moves & captureTargets;
					for (Sqr toSqr : captureMoves)
						add(pieceType, fromSqr, toSqr, Piece(capturedType), newType, killerMoves);
				}
			}

			if (!excludeQuietMoves) {
				Mask allCaptureTargets = state.board()(~player);
				if (pieceType == Piece::PAWN)
					allCaptureTargets |= enPassantMask;
				Mask quietMoves = moves & ~allCaptureTargets;
				for (Sqr toSqr : quietMoves)
					add(pieceType, fromSqr, toSqr, Piece::NONE, newType, killerMoves);
			}
		}
	}

	Mask getEnPassantMask(const GameState& state) const
	{
		Sqr enPassantSqr = state.enPassantSqr();
		return enPassantSqr ? enPassantSqr : Mask();
	}

	void add(Piece pieceType, Sqr fromSqr, Sqr toSqr, Piece capturedType, Piece newType,
			const std::array<Move, 2>& killerMoves)
	{
		Move move = Move(fromSqr, toSqr, pieceType, capturedType, newType);
		unsigned priority = QUIET_MOVE_PRIORITY;
		if (capturedType)
			priority = CAPTURE_PRIORITIES[pieceType][capturedType];
		else if (newType != pieceType)
			priority = PROMOTION_PRIORITIES[newType];
		else if (move == killerMoves[0] || move == killerMoves[1])
			priority = KILLER_MOVE_PRIORITY;
		unsigned idx = moveCounts[priority]++;
		moves[priority][idx] = move;
	}
};

template<typename T>
constexpr unsigned MoveList_t<T>::CAPTURE_PRIORITIES[][6];

template<typename T>
constexpr unsigned MoveList_t<T>::PROMOTION_PRIORITIES[];

typedef MoveList_t<> MoveList;

}
