#pragma once

#include "Sqr.h"
#include "Piece.h"
#include <string>
#include <stdexcept>
#include <cstdint>
#include <cctype>

namespace cm {

class Move
{
private:
	uint32_t mValue;
public:

	constexpr Move()
	: mValue(0)
	{
		;
	}

	constexpr Move(const Sqr fromSqr, const Sqr toSqr, const Piece pieceType,
			const Piece capturedType, const Piece newType)
	: mValue(fromSqr | toSqr << 8 | pieceType << 16 | (capturedType + 1) << 20 | newType << 24)
	{
		;
	}

	Move(const char* s)
	: Move(std::string(s))
	{
		;
	}

	Move(const std::string& s)
	{
		//TODO regex "[KQRBN]?[a-h][1-8](-|x[KQRBN]?)[a-h][1-8][QRBN]?"

		size_t i = 0;

		Piece pieceType = Piece::PAWN;
		if (isupper(s[i]))
			pieceType = s.substr(i++, 1);

		Sqr fromSqr(s, i);

		Piece capturedType = Piece::NONE;
		if (s[i++] == 'x') {
			capturedType = Piece::PAWN;
			if (isupper(s[i]))
				capturedType = s.substr(i++, 1);
		}

		Sqr toSqr(s, i);

		Piece newType = pieceType;
		if (i != s.length()) {
			if (pieceType != Piece::PAWN)
				throw std::invalid_argument("Invalid move format.");
			newType = s.substr(i++, 1);
		}

		*this = Move(fromSqr, toSqr, pieceType, capturedType, newType);
	}

	constexpr Sqr fromSqr() const
	{
		return Sqr(mValue & 0xff);
	}

	constexpr Sqr toSqr() const
	{
		return Sqr(mValue >> 8 & 0xff);
	}

	constexpr Piece pieceType() const
	{
		return Piece(mValue >> 16 & 0x7);
	}

	constexpr Piece capturedType() const
	{
		return Piece((mValue >> 20 & 0x7) - 1);
	}

	constexpr Piece newType()
	{
		return Piece(mValue >> 24 & 0x7);
	}

	constexpr bool isPromotion()
	{
		return newType() != pieceType();
	}

	constexpr bool isCapture()
	{
		return mValue >> 20 & 0x7;
	}

	constexpr bool operator==(const Move& rhs) const
	{
		return mValue == rhs.mValue;
	}

	constexpr bool operator!=(const Move& rhs) const
	{
		return mValue != rhs.mValue;
	}

	explicit constexpr operator bool() const
	{
		return !!mValue;
	}

	std::string toStr()
	{
		std::string ret = pieceType().toStr() + fromSqr().toStr();
		ret += capturedType() ? "x" + capturedType().toStr() : "-";
		ret += toSqr().toStr();
		if (isPromotion())
			ret += newType().toStr();
		return ret;
	}
};

}
