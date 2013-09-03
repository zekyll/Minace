#pragma once

#include <string>
#include <algorithm>
#include <iterator>

namespace cm {

template<typename T>
class Piece_t
{
public:
	static constexpr Piece_t NONE = Piece_t(-1);
	static constexpr Piece_t KING = Piece_t(0);
	static constexpr Piece_t QUEEN = Piece_t(1);
	static constexpr Piece_t ROOK = Piece_t(2);
	static constexpr Piece_t BISHOP = Piece_t(3);
	static constexpr Piece_t KNIGHT = Piece_t(4);
	static constexpr Piece_t PAWN = Piece_t(5);
	static constexpr unsigned COUNT = 6;

private:
	static std::string SYMBOLS[COUNT];

private:
	T mValue;

public:

	explicit constexpr Piece_t(T value)
	: mValue(value)
	{
	}

	Piece_t(const std::string& s)
	{
		mValue = std::find(std::begin(SYMBOLS), std::end(SYMBOLS), s) - std::begin(SYMBOLS);
		for (mValue = 0; mValue < COUNT; ++mValue) {
			if (SYMBOLS[mValue] == s)
				break;
		}
	}

	std::string toStr() const
	{
		return SYMBOLS[mValue];
	}

	constexpr bool operator==(const Piece_t& rhs) const
	{
		return mValue == rhs.mValue;
	}

	constexpr bool operator!=(const Piece_t& rhs) const
	{
		return mValue != rhs.mValue;
	}

	explicit constexpr operator bool() const
	{
		return *this != NONE;
	}

	constexpr operator T() const
	{
		return mValue;
	}
};

template<typename T>
		std::string Piece_t<T>::SYMBOLS[Piece_t<T>::COUNT] = {"K", "Q", "R", "B", "N", ""};

template<typename T>
constexpr Piece_t<T> Piece_t<T>::NONE;

template<typename T>
constexpr Piece_t<T> Piece_t<T>::KING;

template<typename T>
constexpr Piece_t<T> Piece_t<T>::QUEEN;

template<typename T>
constexpr Piece_t<T> Piece_t<T>::ROOK;

template<typename T>
constexpr Piece_t<T> Piece_t<T>::BISHOP;

template<typename T>
constexpr Piece_t<T> Piece_t<T>::KNIGHT;

template<typename T>
constexpr Piece_t<T> Piece_t<T>::PAWN;

typedef Piece_t<unsigned> Piece;

}