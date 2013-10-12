#pragma once

#include "BitBoard.h"
#include "Mask.h"
#include "Sqr.h"
#include "Player.h"
#include <string>

namespace cm {

/**
 * Parses an Extended Position Description from string. Currently only supports the initial data
 * fields and rest of the string is ignored.
 */
class Epd
{
private:

	std::string mString;

	BitBoard mBoard;

	Player mStartingPlayer;

	Mask mCastlingRights;

	Sqr mEnPassantSqr;

public:

	Epd(const std::string& s)
	: mString(s), mStartingPlayer(Player::WHITE), mCastlingRights(Mask()),
	mEnPassantSqr(Sqr::NONE)
	{
		size_t idx = 0;
		mBoard = BitBoard(s, &idx);
		parseSpace(s, idx);
		parseStartingPlayer(s, idx);
		parseSpace(s, idx);
		parseCastlingRights(s, idx);
		parseSpace(s, idx);
		parseEnPassantSquare(s, idx);
		//TODO check validity of castles / en passant?
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

private:

	void parseSpace(const std::string& s, size_t& idx)
	{
		if (s[idx] == ' ')
			++idx;
		else
			throw std::invalid_argument("Invalid EPD string. Expected space.");
	}

	void parseStartingPlayer(const std::string& s, size_t& idx)
	{
		if (idx < s.size()) {
			if (s[idx] == 'b')
				mStartingPlayer = Player::BLACK;
			else if (s[idx] != 'w')
				throw std::invalid_argument("Invalid starting player in EPD string.");
			++idx;
		} else
			throw std::invalid_argument("Missing starting player in EPD string.");
	}

	void parseCastlingRights(const std::string& s, size_t& idx)
	{
		if (idx < s.size()) {
			if (s[idx] == '-') {
				++idx;
			} else {
				parseCastlingRight(s, idx, 'K', Sqr(0));
				parseCastlingRight(s, idx, 'Q', Sqr(7));
				parseCastlingRight(s, idx, 'k', Sqr(56));
				parseCastlingRight(s, idx, 'q', Sqr(63));
			}
		} else
			throw std::invalid_argument("Missing castling rights in EPD string.");
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
		if (idx < s.size()) {
			if (s[idx] == '-')
				++idx;
			else if (idx < s.size() - 1)
				mEnPassantSqr = Sqr(s, idx);
			else
				throw std::invalid_argument("Missing en passant square in EPD string.");
		} else
			throw std::invalid_argument("Missing en passant square in EPD string.");
	}
};

}