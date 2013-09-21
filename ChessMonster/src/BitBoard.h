#pragma once

#include "Player.h"
#include "Piece.h"
#include "Sqr.h"
#include "Mask.h"
#include <cstdint>
#include <string>
#include <cctype>
#include <cstring>
#include <cassert>
#include <array>

namespace cm {

class BitBoard
{
private:
	std::array<Mask, Piece::COUNT> mPieces;

	std::array<Mask, Player::COUNT> mPlayerPieces;

public:

	BitBoard()
	{
	}

	BitBoard(const std::string& whitePieces, const std::string& blackPieces)
	: BitBoard()
	{
		addPieces(Player::WHITE, whitePieces);
		addPieces(Player::BLACK, blackPieces);
	}

	void clear()
	{
		mPieces.fill(0);
		mPlayerPieces.fill(0);
	}

	void addPiece(Player player, Piece piece, Sqr sqr)
	{
		assert(!(*this)(sqr));
		Mask sqrMask(sqr);
		mPieces[piece] |= sqrMask;
		mPlayerPieces[player] |= sqrMask;
	}

	void removePiece(Player player, Piece piece, Sqr sqr)
	{
		assert((*this)(player, piece, sqr));
		Mask sqrMask(sqr);
		mPieces[piece] &= ~sqrMask;
		mPlayerPieces[player] &= ~sqrMask;
	}

	Mask operator()(Player player, Sqr sqr) const
	{
		return mPlayerPieces[player] & sqr;
	}

	Mask operator()(Player player, Piece piece, Sqr sqr) const
	{
		return mPlayerPieces[player] & mPieces[piece] & sqr;
	}

	Mask operator()(Player player, Piece piece) const
	{
		return mPlayerPieces[player] & mPieces[piece];
	}

	Mask operator()(Player player) const
	{
		return mPlayerPieces[player];
	}

	Mask operator()(Sqr sqr) const
	{
		return (mPlayerPieces[Player::WHITE] | mPlayerPieces[Player::BLACK]) & sqr;
	}

	Mask operator()() const
	{
		return mPlayerPieces[Player::WHITE] | mPlayerPieces[Player::BLACK];
	}

	Piece getPieceType(Player player, Sqr sqr) const
	{
		for (unsigned piece = 0; piece < Piece::COUNT; ++piece) {
			if ((*this)(player, Piece(piece), sqr))
				return Piece(piece);
		}
		return Piece::NONE;
	}

	Player getPlayer(Sqr sqr) const
	{
		for (unsigned player = 0; player < Player::COUNT; ++player) {
			if ((*this)(Player(player), sqr))
				return Player(player);
		}
		return Player::NONE;
	}

	bool operator==(const BitBoard& rhs) const
	{
		if (mPlayerPieces[Player::WHITE] != rhs.mPlayerPieces[Player::WHITE]
				|| mPlayerPieces[Player::BLACK] != rhs.mPlayerPieces[Player::BLACK])
			return false;
		for (unsigned i = Piece::COUNT; i-- > 0; ) {
			if (mPieces[i] != rhs.mPieces[i])
				return false;
		}

		return true;
	}
private:

	void addPieces(Player player, const std::string& pieceStr)
	{
		for (size_t i = 0; i < pieceStr.size(); ++i) {
			Piece piece = Piece::PAWN;
			if (isupper(pieceStr[i]))
				piece = Piece(std::string(1, pieceStr[i++]));
			Sqr sqr(pieceStr, i);
			addPiece(player, piece, sqr);
		}
	}
};

}