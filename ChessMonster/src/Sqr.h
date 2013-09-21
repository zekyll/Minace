#pragma once

#include <string>

namespace cm {

template<typename T>
class Sqr_t
{
public:
	static constexpr Sqr_t NONE = Sqr_t(-1);
	static constexpr unsigned COUNT = 64;

private:
	T mValue;

public:

	constexpr Sqr_t()
	: mValue(-1)
	{
		;
	}

	explicit constexpr Sqr_t(T value)
	: mValue(value)
	{
		;
	}

	constexpr Sqr_t(T row, T column)
	: Sqr_t(row * 8 + column)
	{
		;
	}

	Sqr_t(const std::string& str)
	{
		mValue = str[0] - 'a';
		mValue += ('8' - str[1]) * 8;
	}

	Sqr_t(const std::string& str, size_t& idx)
	{
		mValue = str[idx++] - 'a';
		mValue += ('8' - str[idx++]) * 8;
	}

	constexpr unsigned row() const
	{
		return mValue >> 3;
	}

	constexpr unsigned col() const
	{
		return mValue & 7;
	}

	constexpr operator const T() const
	{
		return mValue;
	}

	bool operator==(const Sqr_t& rhs) const
	{
		return mValue == rhs.mValue;
	}

	bool operator!=(const Sqr_t& rhs) const
	{
		return mValue != rhs.mValue;
	}

	explicit operator bool() const
	{
		return *this != NONE;
	}

	std::string toStr() const
	{
		const char s[] = {(char) ('a' + col()), (char) ('8' - row()), '\0'};
		return s;
	}
};

template<typename T>
constexpr Sqr_t<T> Sqr_t<T>::NONE;

typedef Sqr_t<unsigned> Sqr;

}
