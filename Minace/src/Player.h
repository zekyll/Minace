#pragma once

#include <string>

namespace mnc {

template<typename T>
class Player_t
{
public:
	static constexpr Player_t NONE = Player_t(-1);
	static constexpr Player_t WHITE = Player_t(0);
	static constexpr Player_t BLACK = Player_t(1);
	static constexpr unsigned COUNT = 2;

private:
	T mValue;

public:

	explicit constexpr Player_t(T value)
	: mValue(value)
	{
	}

	constexpr operator T() const
	{
		return mValue;
	}

	constexpr explicit operator bool() const
	{
		return *this != NONE;
	}

	constexpr Player_t operator~() const
	{
		return Player_t(1 ^ mValue);
	}

	std::string toStr(bool longName = false) const
	{
		if (longName)
			return *this ? (*this == WHITE ? "white" : "black") : "n/a";
		else
			return *this ? (*this == WHITE ? "w" : "b") : "-";
	}
};

template<typename T>
constexpr Player_t<T> Player_t<T>::NONE;

template<typename T>
constexpr Player_t<T> Player_t<T>::WHITE;

template<typename T>
constexpr Player_t<T> Player_t<T>::BLACK;

typedef Player_t<unsigned> Player;

}
