#pragma once

#include "Player.h"
#include "Piece.h"
#include "Sqr.h"
#include "Mask.h"
#include <sstream>
#include <cstdint>
#include <string>
#include <cctype>
#include <cstring>
#include <cassert>
#include <array>
#include <iosfwd>

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

	BitBoard(const std::string& epd, size_t* idx = nullptr)
	: BitBoard()
	{
		unsigned i = idx ? *idx : 0;
		for (unsigned row = 0, col = 0; row < 7 || col < 8; ++i) {
			if (i >= epd.size())
				throw std::invalid_argument("Invalid position description.");
			if (epd[i] >= '1' && epd[i] <= '8') {
				col += epd[i] - '0';
			} else if (epd[i] == '/') {
				if (col != 8)
					throw std::invalid_argument("Invalid position description.");
				++row;
				col = 0;
			} else if (isalpha(epd[i])) {
				Piece piece(epd.substr(i, 1));
				if (!piece)
					throw std::invalid_argument("Invalid position description.");
				Player player(!!islower(epd[i]));
				addPiece(player, piece, Sqr(row, col));
				++col;
			} else {
				throw std::invalid_argument("Invalid position description.");
			}
		}
		if (idx)
			*idx = i;
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

	Mask operator()(Piece piece, Sqr sqr) const
	{
		return mPieces[piece] & sqr;
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
		assert(player);
		assert(sqr);
		for (unsigned piece = 0; piece < Piece::COUNT; ++piece) {
			if ((*this)(player, Piece(piece), sqr))
				return Piece(piece);
		}
		return Piece::NONE;
	}

	Piece getPieceType(Sqr sqr) const
	{
		assert(sqr);
		for (unsigned piece = 0; piece < Piece::COUNT; ++piece) {
			if ((*this)(Piece(piece), sqr))
				return Piece(piece);
		}
		return Piece::NONE;
	}

	Player getPlayer(Sqr sqr) const
	{
		assert(sqr);
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
		for (unsigned i = Piece::COUNT; i-- > 0;) {
			if (mPieces[i] != rhs.mPieces[i])
				return false;
		}

		return true;
	}

	bool operator!=(const BitBoard& rhs) const
	{
		return !(*this == rhs);
	}

	std::string toStr(bool fen = false) const
	{
		std::stringstream ss;
		if (fen) {
			for (unsigned row = 0; row < 8; ++row) {
				unsigned emptyCount = 0;
				for (unsigned col = 0; col < 8; ++col) {
					Sqr sqr(row, col);
					if ((*this)(sqr)) {
						if (emptyCount)
							ss << emptyCount;
							ss << getPieceType(sqr).toStr(getPlayer(sqr), true);
						emptyCount = 0;
					} else {
						++emptyCount;
					}
				}
				if (emptyCount)
					ss << emptyCount;
				if (row < 7)
					ss << "/";
			}
		} else {
			ss << *this;
		}
		return ss.str();
	}

	friend std::ostream& operator<<(std::ostream& os, const BitBoard& board)
	{
		for (unsigned row = 0; row < 8; ++row) {
			for (unsigned col = 0; col < 8; ++col) {
				Player player = board.getPlayer(Sqr(row, col));
				Piece piece = player ? board.getPieceType(player, Sqr(row, col)) : Piece::NONE;
				os << piece.toStr(player, true);
			}
			os << '\n';
		}
		return os;
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
