#pragma once

#include "BitBoard.h"
#include "Mask.h"
#include "Sqr.h"
#include "Player.h"
#include <string>

namespace cm {

/**
 * Parses an Extended Position Description from string. Currently only supports the initial data
 * fields and rest of the string is ignored. Also recognizes Forsyth-Edwards Notation (FEN).
 */
class Epd
{
private:

	std::string mString;

	BitBoard mBoard;

	Player mStartingPlayer;

	Mask mCastlingRights;

	Sqr mEnPassantSqr;

	unsigned mHalfMoveClock, mFullMoveNumber;

public:

	Epd(const std::string& s)
	: Epd()
	{
		size_t idx = 0;
		init(s, idx);
	}

	Epd(const std::string& s, size_t& idx)
	: Epd()
	{
		init(s, idx);
	}

	const std::string& string() const
	{
		return mString;
	}

	const BitBoard& board() const
	{
		return mBoard;
	}

	Player startingPlayer() const
	{
		return mStartingPlayer;
	}

	Mask castlingRights() const
	{
		return mCastlingRights;
	}

	Sqr enpassantSqr() const
	{
		return mEnPassantSqr;
	}

	unsigned halfMoveClock() const
	{
		return mHalfMoveClock;
	}

	unsigned fullMoveNumber() const
	{
		return mFullMoveNumber;
	}

private:

	Epd()
	: mString(), mBoard(), mStartingPlayer(Player::WHITE), mCastlingRights(Mask()),
	mEnPassantSqr(Sqr::NONE), mHalfMoveClock(0), mFullMoveNumber(1)
	{
	}

	void init(const std::string& s, size_t& idx)
	{
		mBoard = BitBoard(s, &idx);
		parseSpace(s, idx);
		parseStartingPlayer(s, idx);
		parseSpace(s, idx);
		parseCastlingRights(s, idx);
		parseSpace(s, idx);
		parseEnPassantSquare(s, idx);
		//TODO check validity of castles / en passant?

		// FEN
		parseFenMoveCounters(s, idx);

		//TODO parse EPD operations
	}

	bool parseFenMoveCounters(const std::string& s, size_t& idx)
	{
		size_t startIdx = idx;
		bool r = parseSpace(s, idx, false) && parseNumber(s, idx, mHalfMoveClock)
				&& parseSpace(s, idx, false) && parseNumber(s, idx, mFullMoveNumber);
		if (!r)
			idx = startIdx;
		return r;
	}

	bool parseNumber(const std::string& s, size_t& idx, unsigned& num)
	{
		if (s[idx] < '0' || s[idx] > '9')
			return false;
		char* e;
		num = strtol(&s[idx], &e, 10);
		idx += e - &s[idx];
		return true;
	}

	bool parseSpace(const std::string& s, size_t& idx, bool throwOnFail = true)
	{
		if (s[idx] == ' ') {
			++idx;
			return true;
		}
		if (throwOnFail)
			throw std::invalid_argument("Invalid position description. Expected space.");

		return false;
	}

	void parseStartingPlayer(const std::string& s, size_t& idx)
	{
		if (s[idx] == 'b')
			mStartingPlayer = Player::BLACK;
		else

			if (s[idx] != 'w')
			throw std::invalid_argument("Invalid starting player in position description.");
		++idx;
	}

	void parseCastlingRights(const std::string& s, size_t& idx)
	{
		if (s[idx] == '-') {
			++idx;
		} else {
			parseCastlingRight(s, idx, 'K', Sqr(56));
			parseCastlingRight(s, idx, 'Q', Sqr(63));
			parseCastlingRight(s, idx, 'k', Sqr(0));
			parseCastlingRight(s, idx, 'q', Sqr(7));

			if (!mCastlingRights)
				throw std::invalid_argument("Invalid castling rights in position description.");
		}
	}

	void parseCastlingRight(const std::string& s, size_t& idx, char c, Sqr sqr)
	{
		if (s[idx] == c) {

			mCastlingRights |= sqr;
			++idx;
		}
	}

	void parseEnPassantSquare(const std::string& s, size_t& idx)
	{
		if (s[idx] == '-') {
			++idx;
		} else {
			try {
				mEnPassantSqr = Sqr(s, idx);
			} catch (std::invalid_argument& e) {
				throw std::invalid_argument("Invalid en passant square in position description.");
			}
		}
	}
};

}