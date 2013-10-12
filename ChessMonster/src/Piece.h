#pragma once

#include "Player.h"
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
	static std::string SYMBOLS[2][Player::COUNT][COUNT + 1];

private:
	T mValue;

public:

	explicit constexpr Piece_t(T value)
	: mValue(value)
	{
	}

	Piece_t(const std::string& s)
	{
		for (unsigned i = 0; i < 2; ++i) {
			for (unsigned player = 0; player < Player::COUNT; ++player) {
				std::string(&symbols)[COUNT + 1] = SYMBOLS[i][player];
				auto b = std::begin(symbols);
				auto e = std::end(symbols);
				auto r = std::find(b + 1, e, s);
				if (r != e) {
					mValue = r - b - 1;
					return;
				}
			}
		}
		mValue = NONE.mValue;
	}

	std::string toStr(Player player = Player::WHITE, bool displayPawnAndEmpty = false) const
	{
		if (!player)
			player = Player::WHITE;
		return SYMBOLS[displayPawnAndEmpty][player][mValue + 1];
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
std::string Piece_t<T>::SYMBOLS[2][Player::COUNT][Piece_t<T>::COUNT + 1] = {
	{
		{"", "K", "Q", "R", "B", "N", ""},
		{"", "k", "q", "r", "b", "n", ""}
	},
	{
		{".", "K", "Q", "R", "B", "N", "P"},
		{".", "k", "q", "r", "b", "n", "p"}
	}
};

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
