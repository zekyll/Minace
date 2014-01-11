#pragma once

#include <string>
#include <stdexcept>

namespace mnc {

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
	}

	explicit constexpr Sqr_t(T value)
	: mValue(value)
	{
	}

	constexpr Sqr_t(T row, T column)
	: Sqr_t(row * 8 + column)
	{
	}

	Sqr_t(const std::string& str)
	{
		size_t idx = 0;
		init(str, idx);
	}

	Sqr_t(const std::string& str, size_t& idx)
	{
		init(str, idx);
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

private:

	void init(const std::string& str, size_t& idx)
	{
		if (idx >= str.size() - 1 || str[idx] < 'a' || str[idx] > 'h' || str[idx + 1] < '1'
				|| str[idx + 1] > '8')
			throw std::invalid_argument("Invalid square descriptor.");
		mValue = str[idx++] - 'a';
		mValue += ('8' - str[idx++]) * 8;
	}
};

template<typename T>
constexpr Sqr_t<T> Sqr_t<T>::NONE;

typedef Sqr_t<unsigned> Sqr;

}
